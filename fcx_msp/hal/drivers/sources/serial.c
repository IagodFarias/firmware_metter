/***************************************************************************//**
 * @file    serial.c
 * @brief   This module provides serial communication for the application.
 * @details It uses two ring buffers (receive and transmit) to provide serial
 *          data for the main application.
 * @date    11/08/2014
 * @author  Robson Cardoso
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "msp430.h"
#include "eusci_a_uart.h"
#include "gpio.h"
#include "timeout.h"
#include "serial.h"
#include "hal.h"

#if DEBUG_SERIAL
#include <stdio.h>
#include <string.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define SERIAL_PARITY                           EUSCI_A_UART_NO_PARITY
#define SERIAL_MSB_LSB_FIRST                    EUSCI_A_UART_LSB_FIRST
#define SERIAL_MSB_STOP_BIT                     EUSCI_A_UART_ONE_STOP_BIT
#define SERIAL_UART_MODE                        EUSCI_A_UART_MODE
#define SERIAL_UART_ADDR                        (EUSCI_A1_BASE)
#define SERIAL_UART_BAUDRATE                    (57600)

/** Serial receive timeout in milliseconds. */
#if DEBUG
#define SERIAL_RECEIVE_TIMEOUT_MS               2000
#else
#define SERIAL_RECEIVE_TIMEOUT_MS               200
#endif
#define SERIAL_TIMEOUT_RX                       HAL_SYS_MSDELAY_ACLK(SERIAL_RECEIVE_TIMEOUT_MS)
#define SERIAL_TIMER_MAX_DELAY                  65535

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void TxByte(uint8_t Data);
static bool RxPeekByte(uint8_t *byte);
static void setTimeoutSerial(uint64_t ull_timeout);
static bool timeoutSerialCallback(void);

#if DEBUG_SERIAL
int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);
#endif

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static volatile bool gb_timeout = false;
static tFunctionTimer g_timeoutSerial;
static volatile uint64_t gull_timeoutTime = (uint64_t) SERIAL_TIMEOUT_RX;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void serial_init(void)
{
    // Configure timeout serial callback
    g_timeoutSerial.pbFunction_A = &timeoutSerialCallback;
    g_timeoutSerial.ui16FunctionDelay_A = 0;

    /*
    * Select Port 1
    * Set Pin 2, 3 to input Primary Module Function or PySEL1,0 = [01],
    *  (UCA1TXD/UCA1SIMO, UCA1RXD/UCA1SOMI).
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
           GPIO_PORT_P1,
           GPIO_PIN2 + GPIO_PIN3,
           GPIO_PRIMARY_MODULE_FUNCTION
           );

    // Configure UART
    EUSCI_A_UART_initParam param = {0};
    param.parity = SERIAL_PARITY;
    param.msborLsbFirst = SERIAL_MSB_LSB_FIRST;
    param.numberofStopBits = SERIAL_UART_MODE;
    param.uartMode = EUSCI_A_UART_MODE;

#if ((HAL_SYS_SMCLK_FREQ_HZ ==8000000) && (SERIAL_UART_BAUDRATE == 57600))
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 8;           // 8000000/16/57600 = 8.68
    param.firstModReg = 10;              // UCOS16=1, BRFx = 10, BRSx = 0xF7
    param.secondModReg = 0xF7;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
#elif ((HAL_SYS_SMCLK_FREQ_HZ==8000000) && (SERIAL_UART_BAUDRATE == 115200))
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 4;           // 8000000/16/115200 = 4.34
    param.firstModReg = 5;              // UCOS16=1, BRFx = 5, BRSx = 0x55
    param.secondModReg = 0x55;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
#elif ((SERIAL_UART_BAUDRATE == 9600) && (HAL_SYS_ACLK_FREQ_HZ==32768))
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_ACLK;
    param.clockPrescalar = 3;           // 32768/9600 = 3.41
    param.firstModReg = 0;              // UCOS16=0 BRFx = 0, BRSx = 0x92
    param.secondModReg = 0x92;
    param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;
#else
    #error "UART Baudrate Configuration not supported "
#endif

    if(STATUS_FAIL == EUSCI_A_UART_init(SERIAL_UART_ADDR, &param))
    {
        return;
    }

    EUSCI_A_UART_enable(SERIAL_UART_ADDR);
}

uint16_t serial_transmitBytes(uint8_t *puc_data, uint16_t ui_size)
{
    uint16_t ui_idx;

    for(ui_idx = 0; ui_idx < ui_size; ui_idx++)
    {
        TxByte(*(puc_data++));
    }

    return ui_idx;
}

bool serial_receiveByte(uint8_t *puc_data)
{
    bool b_dataToRx = false;

    b_dataToRx = RxPeekByte(puc_data);

    return b_dataToRx;
}

bool serial_receiveBytes(uint8_t *puc_data, uint16_t ui_size)
{
    uint16_t ui_idx = 0;
    bool b_dataToRx = false;

    for (ui_idx = 0; ui_idx < ui_size; ui_idx++)
    {
        // Reset flag for next byte
        b_dataToRx = false;

        // Start timeout from serial
        setTimeoutSerial((uint64_t) SERIAL_TIMEOUT_RX);

        while (!b_dataToRx && !gb_timeout)
        {
            b_dataToRx = RxPeekByte(&puc_data[ui_idx]);
        }

        // Disable timeout
        Timer_cancelDelayedFunction(eTimerDelayedFunction_A);

        // Break loop if no byte received
        if(!b_dataToRx)
        {
            break;
        }
    }

    return b_dataToRx;
}

bool serial_stillTransmitting(void)
{
    return !EUSCI_A_UART_getInterruptStatus(SERIAL_UART_ADDR,
                    EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
}

void serial_enable(void)
{
    EUSCI_A_UART_enable(SERIAL_UART_ADDR);
}

void serial_disable(void)
{
    EUSCI_A_UART_clearInterrupt(SERIAL_UART_ADDR,
                                EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    EUSCI_A_UART_disable(SERIAL_UART_ADDR);
}

static void TxByte(uint8_t Data)
{
    EUSCI_A_UART_transmitData(SERIAL_UART_ADDR, Data);
}

static bool RxPeekByte(uint8_t *byte)
{
    bool b_status = false;

    b_status = EUSCI_A_UART_getInterruptStatus(SERIAL_UART_ADDR,
                                   EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

    if (b_status)
    {
        *byte = EUSCI_A_UART_receiveData(SERIAL_UART_ADDR);
        EUSCI_A_UART_clearInterrupt(SERIAL_UART_ADDR,
                                    EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    }

    return b_status;
}

static void setTimeoutSerial(uint64_t ull_timeout)
{
    gull_timeoutTime = ull_timeout;
    gb_timeout = false;

    if(ull_timeout > SERIAL_TIMER_MAX_DELAY)
    {
        gull_timeoutTime -= SERIAL_TIMER_MAX_DELAY;
        g_timeoutSerial.ui16FunctionDelay_A = SERIAL_TIMER_MAX_DELAY;
    }
    else
    {
        gull_timeoutTime = 0;
        g_timeoutSerial.ui16FunctionDelay_A = (uint16_t) ull_timeout;
    }

    // Start timer serial
    Timer_startDelayedFunctionTimer(&g_timeoutSerial);
    Timer_scheduleDelayedFunction(eTimerDelayedFunction_A);
}

#if DEBUG_SERIAL
int fputc(int _c, register FILE *_fp)
{
    return serial_transmitBytes((uint8_t*) &_c, 1);
}

int fputs(const char *_ptr, register FILE *_fp)
{
  unsigned int i, len;

  len = strlen(_ptr);

  for(i=0 ; i<len ; i++)
  {
      serial_transmitBytes((uint8_t*) _ptr++, 1);
  }

  return len;
}
#endif

static bool timeoutSerialCallback(void)
{
    if(gull_timeoutTime)
    {
        setTimeoutSerial(gull_timeoutTime);
        return false;
    }

    gb_timeout = true;
    return true;
}
