/***************************************************************************//**
 * @brief
 * @file    optint.c
 * @author  Robson Cardoso
 * @date    17/03/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "general_settings.h"
#include "optint.h"
#include "anasens.h"
#include "eusci_a_uart.h"
#include "hal.h"
#include "timeout.h"
#include "queue.h"
#include "utils_fcx.h"
#include "optcomm.h"


#if DEBUG
#include <stdio.h>
#endif

#if DEBUG_OPTINT
#include <stdio.h>
#include "systick.h"
#include "opmodemng.h"
#endif /* DEBUG_OPTINT */

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Size of the threshold lookup table. */
#define OPTINT_THRES_LUT_SIZE                   8
#define OPTINT_PERC_THRES_LUT_SIZE              8

#define OPTINT_UART_PORT                        GPIO_PORT_P2
#define OPTINT_UART_TX_PIN                      GPIO_PIN0
#define OPTINT_UART_RX_PIN                      GPIO_PIN1

#define OPTINT_LIGHT_SENSOR_PORT                GPIO_PORT_P1
#define OPTINT_LIGHT_SENSOR_PIN                 GPIO_PIN0
#define OPTINT_ENABLE1_PIN                      GPIO_PIN6
#define OPTINT_ENABLE2_PIN                      GPIO_PIN7

#define OPTINT_MIN_ADC_RESOLUTION               _IQ19(0.4)

#define OPTINT_NORMAL_TIMEOUT_TRIES             5
#define OPTINT_RESET_TIMEOUT_TRIES              10

#define OPTINT_PARITY                           EUSCI_A_UART_NO_PARITY
#define OPTINT_MSB_LSB_FIRST                    EUSCI_A_UART_LSB_FIRST
#define OPTINT_MSB_STOP_BIT                     EUSCI_A_UART_ONE_STOP_BIT
#define OPTINT_UART_MODE                        EUSCI_A_UART_MODE
#define OPTINT_UART_ADDR                        (EUSCI_A0_BASE)
#define OPTINT_UART_BAUDRATE                    (9600)

/** Optint receive timeout in milliseconds. */
#define OPTINT_RECEIVE_TIMEOUT_MS               10
#define OPTINT_TIMEOUT_RX                       HAL_SYS_MSDELAY_ACLK(OPTINT_RECEIVE_TIMEOUT_MS)
#define OPTINT_TIMER_MAX_DELAY                  65535

#define OPTINT_QUEUE_SIZE                       5

#define     Bit_time    833     // 9600 Baud, SMCLK=8MHz (8MHz/9600)=833

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if FIXED_POINT_CALC
STATIC void getMinMax(uint32_t *piq19_max, uint32_t *piq19_min);
STATIC void optint_detect(uint32_t iq19_currentVoltage);
#endif
STATIC INLINE void enableOpticalCircuit(bool b_enable);
#if CONFIG_OPTCOMM_ENABLED
STATIC INLINE void enableTxCircuit(bool b_enable);
//void enableRxCircuit(bool b_enable);
STATIC INLINE bool isTransmitting();
STATIC void configUsart(void);
STATIC void enableUsart(void);
STATIC void disableUsart(void);
static void setTimeoutOptInt(uint64_t ull_timeout);
static bool timeoutOptIntCallback(void);

#endif
/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Low threshold lookup table. */
static const s_lut_coordinate_iq_t gas_percThreshold[OPTINT_PERC_THRES_LUT_SIZE] =
{
     //TABELA com valores na abcissa referente à tensão que o fototransistor mandar de acordo com a intensidade da luz
    {_IQ19(2300.0), _IQ19(0.995)},
    {_IQ19(2500.0), _IQ19(0.95)},
    {_IQ19(2800.0), _IQ19(0.9)},
    {_IQ19(3000.0), _IQ19(0.8)},
    {_IQ19(3200.0), _IQ19(0.7)},
    {_IQ19(3400.0), _IQ19(0.9)},
    {_IQ19(3750.0), _IQ19(0.95)},
    {_IQ19(4000.0), _IQ19(0.995)}
};

/** Current optical interface's state. */
STATIC E_OPTINT_STATE_t ge_optIntState;
STATIC uint8_t guc_timeoutReq = 0;
STATIC bool gb_timeoutReq = 0;
STATIC uint32_t gaul_queue[OPTINT_QUEUE_SIZE];
STATIC s_queue_t gs_queue;
STATIC s_queue_t *gps_queue = &gs_queue;
volatile uint8_t gauc_optint_TXByte;        // Value received once hasReceived is set
volatile uint8_t gauc_optint_RXByte;        // Value received once hasReceived is set
volatile bool gb_OPTCOMM_has_received = false;
volatile bool gb_OPTCOMM_has_sent = false;

//Start timer in continuous mode sourced by SMCLK
Timer_A_initContinuousModeParam initContParam = {
    TIMER_A_CLOCKSOURCE_SMCLK,    //! Selects Clock source.
    TIMER_A_CLOCKSOURCE_DIVIDER_1,   //! Is the desired divider for the clock source
    TIMER_A_TAIE_INTERRUPT_DISABLE,  //! Is to enable or disable Timer_A interrupt
    TIMER_A_DO_CLEAR, //! Decides if Timer_A clock divider, count direction, count need to be reset.
    false //! Whether to start the timer immediately
};

//Set up compare
Timer_A_initCompareModeParam initCompParam = {
    TIMER_A_CAPTURECOMPARE_REGISTER_0, // compareRegister
    TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE, // compareInterruptEnable
    TIMER_A_OUTPUTMODE_OUTBITVALUE, // compareOutputMode
    120 // compareValue
};

#if CONFIG_OPTCOMM_ENABLED
static volatile bool gb_timeout = false;
static tFunctionTimer g_timeoutOptInt;
static volatile uint64_t gull_timeoutTime = (uint64_t) OPTINT_TIMEOUT_RX;
#endif

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize optical interface.
 */
void optint_init(void)
{

#if CONFIG_OPTCOMM_ENABLED

    // Configure timeout optint callback
    g_timeoutOptInt.pbFunction_A = &timeoutOptIntCallback;
    g_timeoutOptInt.ui16FunctionDelay_A = 0;

#endif

    // Configure output Light Sensor pins functions
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            OPTINT_LIGHT_SENSOR_PORT,
            OPTINT_ENABLE1_PIN + OPTINT_ENABLE2_PIN,
            GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsPeripheralModuleFunctionInputPin(
            OPTINT_LIGHT_SENSOR_PORT,
            OPTINT_LIGHT_SENSOR_PIN,
            GPIO_PRIMARY_MODULE_FUNCTION);

    // Configure pins directions
    GPIO_setAsOutputPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);
    GPIO_setAsOutputPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);

    // Set pins to default value to save energy
    GPIO_setOutputLowOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);
    GPIO_setOutputLowOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);
    
    // Queue initialize to interface
    queue_init((uint32_t *)&gaul_queue, OPTINT_QUEUE_SIZE, gps_queue);

#if CONFIG_OPTCOMM_ENABLED
    // Config UART
    configUsart();
#endif

    // Initialize state variable
    ge_optIntState = E_OPTINT_STATE_RELEASED;
}

/**
 * @brief
 *      Start optical interface.
 *
 * @details
 *      Enable/Disable optical circuit for finger detection. The optical
 *      circuit shall be disabled as soon as possible for save power.
 */
void optint_start(void)
{
    enableOpticalCircuit(true);
}

/**
 * @brief
 *      Update optical interface's state.
 *
 * @details
 *      Disable the optical circuit and try to detect the pressed state.
 */
void optint_update(void)
{
    enableOpticalCircuit(false);
    optint_detect(anasens_getOpticalInputVoltage());
}

/**
 * @brief
 *      Get optical interface's state.
 *
 * @return
 *      Returns optical interface's state. For more information, please refer
 *      to @ref E_OPTINT_STATE_t.
 */
E_OPTINT_STATE_t optint_getState(void)
{
    return ge_optIntState;
}

#if CONFIG_OPTCOMM_ENABLED
/**
 * @brief
 *      Enable/Disable optical interface for reception.
 *
 * @param[in]   b_enable
 *      Pass "true" to enable the reception or "false" to disable the reception.
 */
void optint_enableRx(bool b_enable)
{
    // Enable/Disable the reception circuit
    enableRxCircuit(b_enable);
}

/**
 * @brief
 *      Receive message through the optical interface.
 *
 * @details
 *      Receive a number of bytes by optical interface. Note that the reception
 *      by optical interface must be started by @ref optint_enableRx prior to
 *      this function be called. This function is blocked until the number of
 *      bytes has been received or a timeout has been occured.
 *
 * @param[out]  puc_msg
 *      Buffer where the received message shall be stored.
 *
 * @param[in]   uc_len
 *      Number of bytes to receive.
 *
 * @return
 *      Returns 'true' in case of success or 'false' otherwise.
 */
bool optint_receive(uint8_t *puc_data, uint8_t uc_len)
{
    bool b_dataToRx = false;
    uint8_t uc_idx = 0;

    // Initialize total bits count
    guc_optint_bit_count = 11 * uc_len;
    gb_optint_isReceiving = true;
    gb_OPTCOMM_has_received = false;

    setTimeoutOptInt((uint64_t) OPTINT_TIMEOUT_RX);


    GPIO_clearInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);  // Clear RXD IFG (interrupt flag)
    GPIO_enableInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

    // Main loop condition checks if there are still bits to be received
    while (guc_optint_bit_count > 0 && !gb_timeout)
    {
        if (gb_OPTCOMM_has_received)
        {
            // Process received byte here
            b_dataToRx = true;
            puc_data[uc_idx] = gauc_optint_RXByte;
            uc_idx++;

            // Prepare for next byte
            gb_OPTCOMM_has_received = false;
            gauc_optint_RXByte = 0;
        }
    }

    if(gb_timeout)
        b_dataToRx = false;
    else
        Timer_cancelDelayedFunction(eTimerDelayedFunction_A);

    return b_dataToRx;
}

/**
 * @brief
 *      Enable/Disable optical interface for transmission.
 *
 * @param[in]   b_enable
 *      Pass "true" to enable the transmission or "false" to disable the
 *      transmission.
 */
void optint_enableTx(bool b_enable)
{
    if(!b_enable)
    {
        // Wait for the transmission to finish
        while(isTransmitting());
    }

    // Enable/Disable the transmission circuit
    enableTxCircuit(b_enable);
}

/**
 * @brief
 *      Send message through the optical interface.
 *
 * @param[in]   puc_msg
 *      Message to be sent.
 *
 * @param[in]   uc_len
 *      Number of bytes to be sent.
 */

void optint_send(uint8_t *puc_msg, uint8_t uc_len)
{
    uint8_t uc_idx;

    for(uc_idx = 0; uc_idx < uc_len; uc_idx++)
    {

        gb_optint_isReceiving = false; // vou enviar e puxar a interrupt de timer
        gb_OPTCOMM_has_sent = false;

        guc_optint_bit_count = 12;

        gauc_optint_TXByte = puc_msg[uc_idx];

        Timer_A_initContinuousMode(TIMER_A4_BASE, &initContParam);

        //Initialize compare mode
        Timer_A_clearCaptureCompareInterrupt(TIMER_A4_BASE,
                                             TIMER_A_CAPTURECOMPARE_REGISTER_0
                                             );

        Timer_A_initCompareMode(TIMER_A4_BASE, &initCompParam);

        Timer_A_startCounter(TIMER_A4_BASE,
                             TIMER_A_CONTINUOUS_MODE
                             );

        // Send a byte

        // Wait for any byte which is sending to finish
        while(!gb_OPTCOMM_has_sent);
    }
}

/**
 * @brief
 *      Check if there is any byte been transmitted through the optical
 *      interface.
 *
 * @return
 *      Returns 'true' if the optical interface is transmitting or 'false'
 *      otherwise.
 */
bool optint_isTransmitting()
{
    return isTransmitting();
}
#endif

void optint_reset()
{
    gb_timeoutReq = true;
    guc_timeoutReq = OPTINT_RESET_TIMEOUT_TRIES;
    ge_optIntState = E_OPTINT_STATE_RELEASED;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
#if FIXED_POINT_CALC
/**
 * \fn void optint_detect(uint32_t)
 * @brief
 *      Detect user pressing of the optical interface.
 *
 * @note
 *      To learn about the DC blocking filter used below, please refer to the
 *      following links:
 *      - http://www.embedded.com/design/configurable-systems/4007653/DSP-Tricks-DC-Removal
 *      - http://peabody.sapp.org/class/dmp2/lab/dcblock/
 *
 * @param[in]   iq19_currentVoltage
 *      Current voltage level measured by the optical circuit.
 */
STATIC void optint_detect(uint32_t iq19_currentVoltage)
{
    static uint32_t iq19_previousVoltage = 0;
    uint8_t uc_threshIdx = 0;
    uint32_t iq19_lowLimit = 0;
    uint32_t iq19_max = 0;
    uint32_t iq19_min = 0;
    uint32_t iq19_dist = 0;

    if(queue_isFull(gps_queue))
    {

        // Get maximum and minimum from queue
        getMinMax(&iq19_max, &iq19_min);

        // Get the better percent threshold by mean
        uc_threshIdx = utils_lutSearchIQ(iq19_currentVoltage,
                                       (s_lut_coordinate_iq_t *)gas_percThreshold,
                                       OPTINT_PERC_THRES_LUT_SIZE);

        // Get the minimum resolution between measures
        iq19_dist = OPTINT_MIN_ADC_RESOLUTION;

        // Get distance between max and min
        if (iq19_max != iq19_min)
        {
            iq19_dist = iq19_max - iq19_min;
        }


        if(uc_threshIdx == 0){
            ge_optIntState = E_OPTINT_STATE_PRESSED;
        }else if(uc_threshIdx == 1){
            ge_optIntState = E_OPTINT_STATE_RELEASED;
        }else {
            // Calculate the threshold to detect pressed interface
            iq19_lowLimit = iq19_previousVoltage;
            iq19_lowLimit -= iq19_dist;
            iq19_lowLimit = _IQ19mpy(iq19_lowLimit,
                                 gas_percThreshold[uc_threshIdx - 1].iq_ordinate);

            // Check if the current voltage is less than threshold
            if(iq19_currentVoltage < iq19_lowLimit)
            {
                if(!gb_timeoutReq)
                {
                    guc_timeoutReq = OPTINT_NORMAL_TIMEOUT_TRIES;
                }

                ge_optIntState = E_OPTINT_STATE_PRESSED;
            }
            else
            {
                gb_timeoutReq = false;
                ge_optIntState = E_OPTINT_STATE_RELEASED;
            }
        }


#if DEBUG_OPTINT
        bool b_printEnable = (E_OPMODE_NORMAL == opmodemng_getCurrentOpMode()) ||
                             (E_OPMODE_VERIFICATION == opmodemng_getCurrentOpMode());

        if (b_printEnable)
        {
            printf("d_dist=%.0f, low_limit=%.3f, d_V=%.0f, per=%2.2f, uc_th=%2.2f, guc_timeoutReq=%d, gb_reset=%d, pressed=%d\n",
                   _IQ19toF(iq19_dist),
                   _IQ19toF(iq19_lowLimit),
                   _IQ19toF(iq19_currentVoltage),
                   _IQ19toF(gas_percThreshold[uc_threshIdx - 1].iq_ordinate),
                   _IQ19toF(gas_percThreshold[uc_threshIdx - 1].iq_abscissa),
                   guc_timeoutReq,
                   gb_timeoutReq,
                   (E_OPTINT_STATE_PRESSED == ge_optIntState));
        }
#endif /* DEBUG_OPTINT */

    }

    // Check if a new measure should be inserted into queue
    if (!guc_timeoutReq)
    {
        // Reset queue if reset was required
        if(gb_timeoutReq)
        {
            gb_timeoutReq = false;
            queue_reset(gps_queue);
        }

        // Insert into queue
        queue_insertRear(gps_queue, iq19_currentVoltage);
        iq19_previousVoltage = iq19_currentVoltage;
    }
    else
    {
        guc_timeoutReq--;
    }
}

/**
 * \fn void getMinMax(uint32_t*, uint32_t*)
 * \brief
 *
 * \param[in] piq19_max, piq19_min
 */
STATIC void getMinMax(uint32_t *piq19_max, uint32_t *piq19_min)
{
    *piq19_max = 0;
    *piq19_min = _IQ19(3999.0);

    for (int uc_idx = 0; uc_idx < queue_size(gps_queue); ++uc_idx)
    {
        if(*piq19_max < queue_getIndex(gps_queue, uc_idx))
        {
            *piq19_max = queue_getIndex(gps_queue, uc_idx);
        }

        if(*piq19_min > queue_getIndex(gps_queue, uc_idx))
        {
            *piq19_min = queue_getIndex(gps_queue, uc_idx);
        }
    }
}
#endif
/**
 * @brief
 *      Enable/Disable optical circuit for finger detection.
 *
 * @param[in]   b_enable
 *      Pass "true" to enable the optical circuit for finger detection or
 *      "false" to disable it.
 */
STATIC inline void enableOpticalCircuit(bool b_enable)
{
    if(b_enable)
    {
        // If it's to enable the circuit, pull the enable pin to HIGH level
        GPIO_setOutputHighOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);
    }
    else
    {
        // If it's to disable the circuit, pull the enable pin to LOW level
        GPIO_setOutputLowOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);
    }
}

#if CONFIG_OPTCOMM_ENABLED
/**
 * @brief
 *      Enable/Disable optical circuit for transmission.
 *
 * @param[in]   b_enable
 *      Pass "true" to enable the circuit for transmission or "false" to
 *      disable the circuit for transmission.
 */
STATIC INLINE void enableTxCircuit(bool b_enable)
{
    if(b_enable)
    {
        // Enable TX LED
        // GPIO_setOutputHighOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

        // Enable USART
        enableUsart();
    }
    else
    {
        // Disable TX LED
        //GPIO_setOutputLowOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

        // Disable USART
        disableUsart();
    }
}

/**
 * @brief
 *      Enable/Disable optical circuit for reception.
 *
 * @param[in]   b_enable
 *      Pass "true" to enable the circuit for reception or "false" to
 *      disable the circuit for reception.
 */
void enableRxCircuit(bool b_enable)
{
    if(b_enable)
    {
        GPIO_clearInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);  // Clear RXD IFG (interrupt flag)
        GPIO_selectInterruptEdge(OPTINT_UART_PORT, OPTINT_UART_RX_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
        GPIO_setAsInputPin(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

        // Set IR_EN2 to High Level
        //GPIO_setAsOutputPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);
        GPIO_setOutputHighOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);

        // Set IR_EN1 to High Level
        //GPIO_setAsOutputPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);
        GPIO_setOutputHighOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);


    }
    else
    {
       GPIO_disableInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

       // Set IR_EN1 to LOW Level
       GPIO_setOutputLowOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);

       // Set IR_EN2 to LOW Level
       GPIO_setOutputLowOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);
    }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    GPIO_disableInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN); // Disable RXD interrupt
    GPIO_clearInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);  // Clear RXD IFG (interrupt flag)


    Timer_A_initContinuousMode(TIMER_A4_BASE, &initContParam);

    //Initialize compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A4_BASE,
                                         TIMER_A_CAPTURECOMPARE_REGISTER_0
                                         );

    Timer_A_initCompareMode(TIMER_A4_BASE, &initCompParam);

    Timer_A_startCounter(TIMER_A4_BASE,
                         TIMER_A_CONTINUOUS_MODE
                         );

}

#pragma vector=TIMER4_A0_VECTOR
__interrupt void Timer4_A0 (void)
{
    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A4_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + Bit_time;

    if(gb_optint_isReceiving){

        switch(guc_optint_bit_count % 11)
        {
            case 0: // startbit
                if(guc_optint_bit_count == 0)  // Finished Entire Message
                {
                   Timer_A_stop(TIMER_A4_BASE); // SMCLK, timer off (for power consumption)
                   Timer_A_disableInterrupt(TIMER_A4_BASE);  // Disable interrupt
                }
                else
                {
                    guc_optint_bit_count--;
                }
                break;
            case 1:
                gb_OPTCOMM_has_received = true;
                guc_optint_bit_count--;
                break;
            case 2:
                guc_optint_bit_count--;
                break;
            default:
                gauc_optint_RXByte = gauc_optint_RXByte >> 1; // Shift the bits down
                guc_optint_bit_count--;
                if (GPIO_getInputPinValue(OPTINT_UART_PORT, OPTINT_UART_RX_PIN) == GPIO_INPUT_PIN_HIGH)       // If bit is set?
                {
                    gauc_optint_RXByte |= 0x80;        // Set the value in the RXByte variable

                }
                break;
        }
    }
    else
    { // software uart for tx as well
        switch(guc_optint_bit_count)
        {
            case 0:
                Timer_A_stop(TIMER_A4_BASE); // SMCLK, timer off (for power consumption)
                Timer_A_disableInterrupt(TIMER_A4_BASE);  // Disable interrupt
                gb_OPTCOMM_has_sent = true;
                break;
            case 1 ... 3:
                GPIO_setOutputLowOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);
                guc_optint_bit_count--;
                break;
            case 12:
                GPIO_setOutputHighOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);
                guc_optint_bit_count--;
                break;
            default:
                if (gauc_optint_TXByte & BIT0)
                {
                    GPIO_setOutputLowOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);
                }
                else
                {
                    GPIO_setOutputHighOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);
                }

                gauc_optint_TXByte = gauc_optint_TXByte >> 1;           // Shift the bits down
                guc_optint_bit_count--;
                break;
        }
    }

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A4_BASE,
                            TIMER_A_CAPTURECOMPARE_REGISTER_0,
                            compVal
                            );
}



/**
 * @brief
 *      Check if there is any byte been transmitted through the optical
 *      interface.
 *
 * @return
 *      Returns 'true' if the optical interface is transmitting or 'false'
 *      otherwise.
 */
STATIC inline bool isTransmitting()
{
    return !EUSCI_A_UART_getInterruptStatus(OPTINT_UART_ADDR,
                               EUSCI_A_UART_TRANSMIT_COMPLETE_INTERRUPT_FLAG);
}


bool optcomm_RxPeekByte(uint8_t *byte)
{

    bool b_status = 0;

    gauc_optint_RXByte = 0;         // Initialize RXByte
    guc_optint_bit_count = 9;           // Load Bit counter, 8 bits + ST
    gb_optint_isReceiving = true;
    gb_OPTCOMM_has_received = false;

    // Start timeout from optint
    setTimeoutOptInt((uint64_t) OPTINT_TIMEOUT_RX);

    GPIO_clearInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);  // Clear RXD IFG (interrupt flag)
    GPIO_enableInterrupt(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

    while (!gb_OPTCOMM_has_received && !gb_timeout) // !b_status &&
    {
        if (gb_OPTCOMM_has_received)
        {
            b_status = 1;
            *byte = gauc_optint_RXByte  ;
        }
    }


    // Disable timeout
    Timer_cancelDelayedFunction(eTimerDelayedFunction_A);

    return b_status;
}


STATIC void configUsart(void)
{
    // Configure UART
    EUSCI_A_UART_initParam param = { 0 };
    param.parity = OPTINT_PARITY;
    param.msborLsbFirst = OPTINT_MSB_LSB_FIRST;
    param.numberofStopBits = OPTINT_UART_MODE;
    param.uartMode = OPTINT_UART_MODE;

#if ((HAL_SYS_SMCLK_FREQ_HZ ==8000000) && (OPTINT_UART_BAUDRATE == 57600))
                param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
                param.clockPrescalar = 8;           // 8000000/16/57600 = 8.68
                param.firstModReg = 10;              // UCOS16=1, BRFx = 10, BRSx = 0xF7
                param.secondModReg = 0xF7;
                param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
#elif ((HAL_SYS_SMCLK_FREQ_HZ==8000000) && (OPTINT_UART_BAUDRATE == 115200))
                param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
                param.clockPrescalar = 4;           // 8000000/16/115200 = 4.34
                param.firstModReg = 5;              // UCOS16=1, BRFx = 5, BRSx = 0x55
                param.secondModReg = 0x55;
                param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
#elif ((HAL_SYS_SMCLK_FREQ_HZ==8000000) && (OPTINT_UART_BAUDRATE == 9600))
                param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
                param.clockPrescalar = 52;           // 32768/9600 = 3.41
                param.firstModReg = 1;              // UCOS16=0 BRFx = 0, BRSx = 0x92
                param.secondModReg = 0x49;
                param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
#elif ((OPTINT_UART_BAUDRATE == 9600) && (HAL_SYS_ACLK_FREQ_HZ==32768))
                param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_ACLK;
                param.clockPrescalar = 3;           // 32768/9600 = 3.41
                param.firstModReg = 0;              // UCOS16=0 BRFx = 0, BRSx = 0x92
                param.secondModReg = 0x92;
                param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;
#else
#error "UART Baudrate Configuration not supported "
#endif


    EUSCI_A_UART_enableInterrupt(OPTINT_UART_ADDR,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT);

    EUSCI_A_UART_enableInterrupt(OPTINT_UART_ADDR,
                                 EUSCI_A_UART_STARTBIT_INTERRUPT);

    if (STATUS_FAIL == EUSCI_A_UART_init(OPTINT_UART_ADDR, &param))
    {
        return;
    }

    // Enable transmit complete interrupt
    EUSCI_A_UART_clearInterrupt(OPTINT_UART_ADDR,
                                EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);

    EUSCI_A_UART_enable(OPTINT_UART_ADDR);



    // Disable USART and GPIO's
    disableUsart();
}


STATIC void enableUsart(void)
{
    //GPIO_setAsInputPin(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);
    //GPIO_setAsInputPinWithPullDownResistor(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

    //GPIO_setAsOutputPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

    // Set IR_EN1 to High Level
    GPIO_setOutputHighOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE1_PIN);

    // Set IR_EN2 to High Level
    GPIO_setOutputHighOnPin(OPTINT_LIGHT_SENSOR_PORT, OPTINT_ENABLE2_PIN);


    /*
     * Select Port 2
     * Set Pin 0, 1 to input secondary Module fFunction or PySEL1,0 = [01],
     *  (UCA0TXD/UCA0SIMO, UCA0RXD/UCA0SOMI).
     */

    //GPIO_setAsInputPin(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);

    //P2IN |= BIT1;

    //P2IN &= ~BIT0;
    //P2DIR |= BIT0;
    //P2DIR &= ~BIT1;

    //PJSEL0 |= BIT4 | BIT5;
    //P2SEL1 |= BIT0 | BIT1;
    //P2SEL0  &= ~(BIT0 | BIT1);

    //GPIO_setAsInputPinWithPullDownResistor(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);


    //GPIO_setAsPeripheralModuleFunctionInputPin(
    //        OPTINT_UART_PORT,
    //        OPTINT_UART_TX_PIN,
    //        GPIO_SECONDARY_MODULE_FUNCTION);

    //EUSCI_A_UART_enable(OPTINT_UART_ADDR);



    GPIO_setAsOutputPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

    //EUSCI_A_UART_selectDeglitchTime(OPTINT_UART_ADDR, EUSCI_A_UART_DEGLITCH_TIME_50ns);



}

STATIC void disableUsart(void)
{
    /*
     * Select Port 2
     * Set Pin 0, 1 to output Primary Module Function or PySEL1,0 = [01],
     *  (UCA0TXD/UCA0SIMO, UCA0RXD/UCA0SOMI).
     */
    //GPIO_setAsPeripheralModuleFunctionOutputPin(
    //        OPTINT_UART_PORT,
    //        OPTINT_UART_TX_PIN + OPTINT_UART_RX_PIN,
    //        GPIO_PRIMARY_MODULE_FUNCTION);

    //GPIO_setAsOutputPin(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);
    //GPIO_setAsOutputPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

    // Put RX as output high to LS circuit and TX low to disable LED
    //GPIO_setOutputHighOnPin(OPTINT_UART_PORT, OPTINT_UART_RX_PIN);
    GPIO_setOutputLowOnPin(OPTINT_UART_PORT, OPTINT_UART_TX_PIN);

    EUSCI_A_UART_clearInterrupt(OPTINT_UART_ADDR,
                                EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);

    EUSCI_A_UART_disable (OPTINT_UART_ADDR);
}

static void setTimeoutOptInt(uint64_t ull_timeout)
{
    gull_timeoutTime = ull_timeout;
    gb_timeout = false;

    if(ull_timeout > OPTINT_TIMER_MAX_DELAY)
    {
        gull_timeoutTime -= OPTINT_TIMER_MAX_DELAY;
        g_timeoutOptInt.ui16FunctionDelay_A = OPTINT_TIMER_MAX_DELAY;
    }
    else
    {
        gull_timeoutTime = 0;
        g_timeoutOptInt.ui16FunctionDelay_A = (uint16_t) ull_timeout;
    }

    // Start timeout timer
    Timer_startDelayedFunctionTimer(&g_timeoutOptInt);
    Timer_scheduleDelayedFunction(eTimerDelayedFunction_A);
}

static bool timeoutOptIntCallback(void)
{
    if(gull_timeoutTime)
    {
        setTimeoutOptInt(gull_timeoutTime);
        return false;
    }

    gb_timeout = true;
    return true;
}
#endif /* CONFIG_OPTCOMM_ENABLED */

