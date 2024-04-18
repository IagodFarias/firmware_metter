/***************************************************************************//**
 * @brief
 * @file    display.c
 * @author  Robson Cardoso
 * @date    17/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "general_settings.h"
#include "display.h"
#include "lcddriver.h"
#include "utils_fcx.h"
#include <string.h>

#if UNIT_TEST
#include <stdio.h>
#endif /* UNIT_TEST */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Resolution used for printing temperature. */
#define DISPLAY_TEMP_RESOLUTION                 1

/** Offset required to properly print temperature with the required resolution. */
#define DISPLAY_TEMP_NUM_DEC_PLACES_OFFSET      (3 + DISPLAY_TEMP_RESOLUTION)

/*******************************************************************************
 * Structures
 ******************************************************************************/

/**
 * @brief
 *      Represents text symbol's segment in terms of COM and SIGNAL numbers,
 *      in a way that we can enumerate each bit for each text segment in the
 *      following bit pattern:
 *
 * @verbatim
 *  ----0----
 * |         |
 * |5        |1
 *  ----6----
 * |         |
 * |4        |2
 *  ----3----
 * @endverbatim
 */
typedef struct S_DIGIT
{
    uint8_t uc_com[DISPLAY_NUM_SEGMENTS];
    uint8_t uc_signal[DISPLAY_NUM_SEGMENTS];
} s_digit_t;

typedef struct S_ICON
{
    uint8_t uc_com;
    uint8_t uc_signal;
} s_icon_t;

typedef struct S_DOT
{
    uint8_t uc_com;
    uint8_t uc_signal;
} s_dot_t;

typedef struct S_DISPLAY
{
    s_icon_t as_icons[DISPLAY_NUM_ICONS];
    s_digit_t as_digits[DISPLAY_NUM_DIGITS];
    s_dot_t as_dots[DISPLAY_NUM_DOTS];
} s_display_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void send(void);
static double_t quickPow10(uint8_t uc_power);
static void clearSegments(uint8_t uc_digit, uint8_t uc_bitpattern);

/*******************************************************************************
 * Global variables
 ******************************************************************************/

static const s_display_t gs_display = DISPLAY_UNCLTNP01395601_CONFIG;

static const uint8_t gauc_alphabet[] = DISPLAY_ALPHABET;

static const uint8_t gauc_numbers[] = DISPLAY_NUMBERS;

/** LCDDRIVER instance */
STATIC s_lcddriver_t gs_lcdDriver;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

#ifndef UNIT_TEST_HOST
/**
 * @brief
 *      Initialize display module.
 */
void display_init(void)
{
    // Enable required peripherals
    display_enable();

    // Initialize LCD driver
    lcddriver_init(&gs_lcdDriver);

    // Reset the display
    display_reset();

    // Clear all segments
    display_eraseAllSegments();

    // Turn on display
    display_turnOn();
}

/**
 * @brief
 *      Reset the display.
 */
void display_reset(void)
{
    lcddriver_packSwResetCmd(&gs_lcdDriver);
    send();
}

/**
 * @brief
 *      Turn on the display.
 */
void display_turnOn(void)
{
    lcddriver_turnOnLcd();
}

/**
 * @brief
 *      Turn off the display.
 */
void display_turnOff(void)
{
    lcddriver_turnOffLcd();
}

/**
 * @brief
 *      Print whatever has been set in the internal buffer.
 *
 * @details
 *      This method commits the data stored in the internal buffer of the LCD
 *      driver module in RAM to the physical driver via I2C.
 *
 * @note
 *      The user should initialize the internal buffer of the LCD driver module
 *      before sending a command. Otherwise, the result may be unpredictable.
 *      All methods with prefix 'display_set' are intended to change the state
 *      of the LCD driver's internal buffer.
 */
void display_print(void)
{
    static uint8_t auc_lastBuffer[LCDDRIVER_BUFFER_SIZE];

    if(memcmp(auc_lastBuffer, gs_lcdDriver.auc_buffer, LCDDRIVER_BUFFER_SIZE))
    {
        memcpy(auc_lastBuffer, gs_lcdDriver.auc_buffer, LCDDRIVER_BUFFER_SIZE);

        lcddriver_packDataCmd(&gs_lcdDriver);
        send();
    }
}
#endif /* UNIT_TEST_HOST */

/**
 * @brief
 *      Set all segments.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 */
void display_setAllSegments(void)
{
    lcddriver_setAllSegments(&gs_lcdDriver);
}

/**
 * @brief
 *      Clear all segments.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 */
void display_clearAllSegments(void)
{
    lcddriver_clearAllSegments(&gs_lcdDriver);
}

/**
 * @brief
 *      Set an icon on the display.
 *
 * @details
 *      This method sets the LCD driver's internal buffer with the encoded
 *      representation of the icon @p E_icon.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   E_icon
 *      Icon selection. For more details, please refer to the enumeration
 *      definition (@see E_ICON_t).
 */
void display_setIcon(E_ICON_t E_icon)
{
    ASSERT(E_icon < DISPLAY_NUM_ICONS);

    lcddriver_setSegment(&gs_lcdDriver,
        gs_display.as_icons[E_icon].uc_com,
        gs_display.as_icons[E_icon].uc_signal);
}

/**
 * @brief
 *      Set a dot on the display.
 *
 * @details
 *      This method sets the LCD driver's internal buffer with the encoded
 *      representation of the dot @p E_dot.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   E_dot
 *      Dot selection. For more details, please refer to the enumeration
 *      definition (@see E_DOT_t).
 */
void display_setDot(E_DOT_t E_dot)
{
    ASSERT(E_dot < DISPLAY_NUM_DOTS);

    lcddriver_setSegment(&gs_lcdDriver,
        gs_display.as_dots[E_dot].uc_com,
        gs_display.as_dots[E_dot].uc_signal);
}

/**
 * @brief
 *      Set segments corresponding to a digit at a given position.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   uc_digit
 *      Digit position.
 *
 * @param[in]   uc_bitpattern
 *      Bit pattern to be set on display.
 */
void display_setSegments(uint8_t uc_digit, uint8_t uc_bitpattern)
{
    uint8_t uc_segIdx;

    // Sanity check
    ASSERT(uc_digit < DISPLAY_NUM_DIGITS);

    // Set each segment according to the bit pattern
    for(uc_segIdx = 0; uc_segIdx < DISPLAY_NUM_SEGMENTS; uc_segIdx++)
    {
        if(uc_bitpattern & (1 << uc_segIdx))
        {
            lcddriver_setSegment(&gs_lcdDriver,
                (uint8_t) gs_display.as_digits[uc_digit].uc_com[uc_segIdx],
                (uint8_t) gs_display.as_digits[uc_digit].uc_signal[uc_segIdx]);
        }
    }
}

/**
 * @brief
 *      Set a digit at a given position.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   uc_digit
 *      Digit position.
 *
 * @param[in]   uc_number
 *      Number to be set. It can assume values in the range [0-11]:
 *      - 0-9: represents numbers from 0 to 9
 *      - 10: represents a minus sign
 */
void display_setDigit(uint8_t uc_digit, uint8_t uc_number)
{
    // Sanity check
    ASSERT(uc_number < sizeof(gauc_numbers));

    // Get number layout and set segments
    display_setSegments(uc_digit, gauc_numbers[uc_number]);
}

/**
 * @brief
 *      Set an integer number.
 *
 * @details
 *      This method sets the LCD driver's internal buffer with the encoded
 *      representation of the number @p ll_value. The value of @p l_value should
 *      range from -99,999,999 to 999,999,999. Otherwise, there will be not
 *      enough digits on the display to represent it, and, consequently, the
 *      leftmost digits will not be printed.
 *
 *      Additionally, it is possible to pad the number with leading zeros. The
 *      parameter @p uc_posPaddingZeros controls the position of the leftmost
 *      leading zero. Due to the display limitations, the maximum value
 *      allowed for @p uc_posPaddingZeros is 8.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   ll_value
 *      Signed integer ranging from -99,999,999 to 999,999,999.
 *
 * @param[in]   uc_numPaddingZeros
 *      Position of the leftmost zero used for padding.
 */
void display_setNumber(int64_t ll_value, uint8_t uc_numPaddingZeros)
{
    uint8_t uc_digit = 0;
    uint8_t uc_number;
    bool b_neg;

    // Make sure the number of leading zeros will fit appropriately on the
    // screen
    if(uc_numPaddingZeros > (DISPLAY_NUM_DIGITS - 1))
    {
        uc_numPaddingZeros = DISPLAY_NUM_DIGITS - 1;
    }

    // If l_value is a negative number get its absolute value and set b_neg flag
    // to indicate the minus sign should be set. Otherwise, clear b_neg flag.
    // Make sure the value of l_value is printable on the screen, i. e., ranges
    // between -99,999,999 and 999,999,999
    if(ll_value < 0)
    {
        ll_value = -ll_value;

        while(ll_value > 99999999L)
        {
            ll_value -= 100000000L;
        }

        b_neg = (ll_value) ? true : false;
    }
    else
    {
        b_neg = false;

        while(ll_value > 999999999L)
        {
            ll_value -= 1000000000L;
        }
    }

    // Extract useful digits
    do
    {
        uc_number = ll_value % 10;
        display_setDigit(uc_digit, uc_number);
        uc_digit++;
    } while((ll_value /= 10) > 0);

    // Pad with leading zeros if necessary
    while(uc_digit <= uc_numPaddingZeros)
    {
        display_setDigit(uc_digit, 0);
        uc_digit++;
    }

    // Set minus sign if the number is negative
    if(b_neg)
    {
        display_setDigit(DISPLAY_NUM_DIGITS-1, DISPLAY_NUMBER_SIGN_IDX);
    }
}

/**
 * @brief
 *      Set a double number.
 *
 * @details
 *      This method sets the LCD driver's internal buffer with the encoded
 *      representation of the number @p d_value in double format with the
 *      precision determined by @p uc_numDecimalPlaces.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   d_value
 *      Number in double format.
 *
 * @param[in]   uc_numDecimalPlaces
 *      Number of decimal places that should be used.
 */
void display_setDouble(double_t d_value, uint8_t uc_numDecimalPlaces)
{
    int64_t ll_value = 0;

    // Make sure the number will fit appropriately on the screen
    if(uc_numDecimalPlaces > (DISPLAY_NUM_DIGITS - 1))
    {
        uc_numDecimalPlaces = DISPLAY_NUM_DIGITS - 1;
    }

    // Cast value
    ll_value = (int64_t) (d_value * quickPow10(uc_numDecimalPlaces));

    if(ll_value < 0)
    {
        ll_value *= -1;
    }

    // Set LCD driver's internal buffer
    display_setNumber(ll_value, uc_numDecimalPlaces);

    // Set dot according to the required precision
    if(uc_numDecimalPlaces)
    {
        display_setDot((E_DOT_t) (uc_numDecimalPlaces - 1));
    }
}

/**
 * @brief
 *      Set a character on the display.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   uc_digit
 *      Digit position.
 *
 * @param[in]   uc_data
 *      Character in ASCII format.
 */
void display_setCharacter(uint8_t uc_digit, uint8_t uc_data)
{
    // Sanity check
    ASSERT((uc_data - 0x20) < sizeof(gauc_alphabet));

    // Get character layout and set segments
    display_setSegments(uc_digit, gauc_alphabet[uc_data - 0x20]);
}

/**
 * @brief
 *      Set a string on the display.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 *      Additionally, for instance, it is not possible to print two dots, one
 *      dot following another.
 *
 * @param[in]   *puc_string
 *      Pointer to the string.
 */
void display_setString(uint8_t *puc_string)
{
    uint8_t uc_digitIdx = DISPLAY_NUM_DIGITS;
    uint8_t uc_data = *puc_string;
    bool b_isLastCharADot = false;

    while(uc_data && uc_digitIdx)
    {
        if(uc_data != 0x2E)
        {
            display_setCharacter(--uc_digitIdx, uc_data);
            b_isLastCharADot = false;
        }
        else
        {
            if(b_isLastCharADot || (DISPLAY_NUM_DIGITS == uc_digitIdx))
            {
                uc_digitIdx--;
            }

            if(uc_digitIdx)
            {
                display_setDot((E_DOT_t) (uc_digitIdx - 1));
            }

            b_isLastCharADot = true;
        }

        uc_data = *(++puc_string);
    }
}

/**
 * @brief
 *      Set temperature.
 *
 * @details
 *      This method sets the LCD driver's internal buffer with the encoded
 *      representation of the number @p d_value in double format with the
 *      precision determined by @p uc_numDecimalPlaces.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 *      Additionally, the @ref display_setDouble method has been used because it
 *      is well tested. However, we pay with a loss of performance as we need to
 *      first clear digits 0, 1, and 2  before setting the degree Celsius
 *      symbol.
 *
 * @param[in]   d_temperature
 *      Temperature in double format.
 */
void display_setTemperature(double_t d_temperature)
{
    // Set temperature
    display_setDouble(d_temperature, DISPLAY_TEMP_NUM_DEC_PLACES_OFFSET);

    // Clear digits 0, 1, and 2
    clearSegments(2, 0x7F);
    clearSegments(1, 0x7F);
    clearSegments(0, 0x7F);

    // Then set Celsius symbol
    display_setSegments(1, DISPLAY_SYMBOL_DEGREE_CELSIUS);
    display_setCharacter(0, (uint8_t) 'C');
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
#ifndef UNIT_TEST_HOST
/**
 * @brief
 *      Send a command to LCD driver through I2C interface.
 */
static void send(void)
{
    lcddriver_print(&gs_lcdDriver);
}
#endif /* UNIT_TEST_HOST */

/**
 * @brief
 *      Calculate
 *
 * @param[in]  uc_power  The uc power
 *
 * @return     { description_of_the_return_value }
 */
static double_t quickPow10(uint8_t uc_power)
{
    const double_t d_pow10[] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0,
                                1000000.0, 10000000.0, 100000000.0,
                                1000000000.0};

    return d_pow10[uc_power];
}

/**
 * @brief
 *      Clear segments corresponding to a digit at a given position.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 *
 * @param[in]   uc_digit
 *      Digit position.
 *
 * @param[in]   uc_bitpattern
 *      Bit pattern to clear on display.
 */
static void clearSegments(uint8_t uc_digit, uint8_t uc_bitpattern)
{
    uint8_t uc_segIdx;

    // Sanity check
    ASSERT(uc_digit < DISPLAY_NUM_DIGITS);

    // Set each segment according to the bit pattern
    for(uc_segIdx = 0; uc_segIdx < DISPLAY_NUM_SEGMENTS; uc_segIdx++)
    {
        if(uc_bitpattern & (1 << uc_segIdx))
        {
            lcddriver_clearSegment(&gs_lcdDriver,
                (uint8_t) gs_display.as_digits[uc_digit].uc_com[uc_segIdx],
                (uint8_t) gs_display.as_digits[uc_digit].uc_signal[uc_segIdx]);
        }
    }
}
