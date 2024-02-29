/***************************************************************************//**
 * @brief
 * @file    display.h
 * @author  Robson Cardoso
 * @date    20/02/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef DISPLAY_H_
#define DISPLAY_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"
#include "uncltnp01395601_config.h"
#include "gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define I2C_SCL_PIN                 IOMUX_IO_8  /** I2C clock pin mapping. */
#define I2C_SDA_PIN                 IOMUX_IO_9  /** I2C data pin mapping. */

/** Encoded characters. */
#define DISPLAY_ALPHABET                                                        \
{                                                                               \
    0x00, /* space */                                                           \
    0x06, /* ! */                                                               \
    0x00, /* " */                                                               \
    0x00, /* # */                                                               \
    0x00, /* $ */                                                               \
    0x00, /* % */                                                               \
    0x00, /* & */                                                               \
    0x00, /* ' */                                                               \
    0x00, /* ( */                                                               \
    0x00, /* ) */                                                               \
    0x00, /* * */                                                               \
    0x00, /* + */                                                               \
    0x00, /* , */                                                               \
    0x40, /* - */                                                               \
    0x00, /* . */                                                               \
    0x00, /* / */                                                               \
                                                                                \
    0x3F, /* 0 */                                                               \
    0x06, /* 1 */                                                               \
    0x5B, /* 2 */                                                               \
    0x4F, /* 3 */                                                               \
    0x66, /* 4 */                                                               \
    0x6D, /* 5 */                                                               \
    0x7D, /* 6 */                                                               \
    0x07, /* 7 */                                                               \
    0x7F, /* 8 */                                                               \
    0x6F, /* 9 */                                                               \
                                                                                \
    0x00, /* : */                                                               \
    0x00, /* ; */                                                               \
    0x00, /* < */                                                               \
    0x00, /* = */                                                               \
    0x00, /* > */                                                               \
    0x00, /* ? */                                                               \
    0x00, /* @ */                                                               \
                                                                                \
    0x77, /* A */                                                               \
    0x7C, /* B */                                                               \
    0x39, /* C */                                                               \
    0x5E, /* D */                                                               \
    0x79, /* E */                                                               \
    0x71, /* F */                                                               \
    0x7D, /* G */                                                               \
    0x76, /* H */                                                               \
    0x30, /* I */                                                               \
    0x0E, /* J */                                                               \
    0x76, /* K */                                                               \
    0x38, /* L */                                                               \
    0x37, /* M */                                                               \
    0x37, /* N */                                                               \
    0x3F, /* O */                                                               \
    0x73, /* P */                                                               \
    0x3F, /* Q */                                                               \
    0x50, /* R */                                                               \
    0x6D, /* S */                                                               \
    0x78, /* T */                                                               \
    0x3E, /* U */                                                               \
    0x3E, /* V */                                                               \
    0x3E, /* W */                                                               \
    0x76, /* X */                                                               \
    0x66, /* Y */                                                               \
    0x5B, /* Z */                                                               \
                                                                                \
    0x39, /* [ */                                                               \
    0x00, /* backslash */                                                       \
    0x0F, /* ] */                                                               \
    0x00, /* ^ */                                                               \
    0x08, /* _ */                                                               \
    0x00, /* ` */                                                               \
                                                                                \
    0x77, /* a */                                                               \
    0x7C, /* b */                                                               \
    0x58, /* c */                                                               \
    0x5E, /* d */                                                               \
    0x79, /* e */                                                               \
    0x71, /* f */                                                               \
    0x6F, /* g */                                                               \
    0x74, /* h */                                                               \
    0x10, /* i */                                                               \
    0x0C, /* j */                                                               \
    0x76, /* k */                                                               \
    0x30, /* l */                                                               \
    0x54, /* m */                                                               \
    0x54, /* n */                                                               \
    0x5C, /* o */                                                               \
    0x73, /* p */                                                               \
    0x67, /* q */                                                               \
    0x50, /* r */                                                               \
    0x6D, /* s */                                                               \
    0x78, /* t */                                                               \
    0x1C, /* u */                                                               \
    0x1C, /* v */                                                               \
    0x1C, /* w */                                                               \
    0x76, /* x */                                                               \
    0x66, /* y */                                                               \
    0x5B  /* z */                                                               \
}

/** Encoded numbers. */ 
#define DISPLAY_NUMBERS                                                         \
{                                                                               \
    0x3F, /* 0 */                                                               \
    0x06, /* 1 */                                                               \
    0x5B, /* 2 */                                                               \
    0x4F, /* 3 */                                                               \
    0x66, /* 4 */                                                               \
    0x6D, /* 5 */                                                               \
    0x7D, /* 6 */                                                               \
    0x07, /* 7 */                                                               \
    0x7F, /* 8 */                                                               \
    0x6F, /* 9 */                                                               \
    0x40, /* - */                                                               \
}

/** Sign digit position in number array. */
#define DISPLAY_NUMBER_SIGN_IDX         0x0A

/** Degree Celsius symbol. */
#define DISPLAY_SYMBOL_DEGREE_CELSIUS   0x63

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void display_init(void);
static inline void display_enable(void);
static inline void display_disable(void);
void display_reset(void);
void display_turnOn(void);
void display_turnOff(void);
void display_print(void);
static inline bool display_stillTransmitting(void);

void display_setAllSegments(void);
void display_clearAllSegments(void);

void display_setIcon(E_ICON_t E_icon);
static inline void display_setAllIcons(void);

void display_setDot(E_DOT_t E_dot);
static inline void display_setAllDots(void);

void display_setSegments(uint8_t uc_digit, uint8_t uc_bitpattern);
void display_setDigit(uint8_t uc_digit, uint8_t uc_number);
void display_setNumber(int64_t ll_value, uint8_t uc_numPaddingZeros);
void display_setDouble(double_t d_value, uint8_t uc_numDecimalPlaces);
void display_setCharacter(uint8_t uc_digit, uint8_t uc_data);
void display_setString(uint8_t *puc_string);
void display_setTemperature(double_t d_temperature);

static inline void display_printAllSegments(void);
static inline void display_printIcon(E_ICON_t E_icon);
static inline void display_printAllIcons(void);
static inline void display_printDot(E_DOT_t E_dot);
static inline void display_printAllDots(void);

static inline void display_printDigit(uint8_t uc_digit, uint8_t uc_number);
static inline void display_printNumber(int32_t l_value, uint8_t uc_posPaddingZeros);
static inline void display_printDouble(double_t d_value, uint8_t uc_numDecimalPlaces);
static inline void display_printCharacter(uint8_t uc_digit, uint8_t uc_data);
static inline void display_printString(uint8_t *puc_string);
static inline void display_printTemperature(double_t d_temperature);

/*******************************************************************************
 * Inline functions
 ******************************************************************************/

/**
 * @brief
 *      Enable all peripherals required for display operation.
 *
 * @note
 *      It is required to reinitialize I2C interface because it probably has
 *      a hardware bug.
 */
static inline void display_enable(void)
{
}

/**
 * @brief
 *      Disable all peripherals required for display operation.
 */
static inline void display_disable(void)
{
}

/**
 * @brief
 *      Set all icons.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 */
static inline void display_setAllIcons(void)
{
    display_setIcon(E_ICON_REVERSE_FLOW);
    display_setIcon(E_ICON_WATER);
    display_setIcon(E_ICON_ERROR);
    display_setIcon(E_ICON_TRANSMISSION);
    display_setIcon(E_ICON_LOW_BAT);
    display_setIcon(E_ICON_M3);
    display_setIcon(E_ICON_PER_HOUR);
}

/**
 * @brief
 *      Set all dots.
 *
 * @note
 *      This method does change only LCD driver's internal buffer. Therefore,
 *      the physical LCD is not updated when this method is called.
 */
static inline void display_setAllDots(void)
{
    display_setDot(E_DOT_POSITION_0);
    display_setDot(E_DOT_POSITION_1);
    display_setDot(E_DOT_POSITION_2);
    display_setDot(E_DOT_POSITION_3);
    display_setDot(E_DOT_POSITION_4);
    display_setDot(E_DOT_POSITION_5);
    display_setDot(E_DOT_POSITION_6);
    display_setDot(E_DOT_POSITION_7);
}

/**
 * @brief
 *      Turn on all display segments.
 */
static inline void display_printAllSegments(void)
{
    display_setAllSegments();
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Turn off all display segments.
 */
static inline void display_eraseAllSegments(void)
{
    display_clearAllSegments();
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print an icon. 
 *
 * @param[in]   E_icon
 *      Icon selection. For more details, please refer to the enumeration
 *      definition (@see E_ICON_t).
 */
static inline void display_printIcon(E_ICON_t E_icon)
{
    display_clearAllSegments();
    display_setIcon(E_icon);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Turn on all icons.
 */
static inline void display_printAllIcons(void)
{
    display_clearAllSegments();
    display_setAllIcons();
    display_setAllDots();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print a dot. 
 *
 * @param[in]   E_dot
 *      Dot selection. For more details, please refer to the enumeration
 *      definition (@see E_DOT_t).
 */
static inline void display_printDot(E_DOT_t E_dot)
{
    display_clearAllSegments();
    display_setDot(E_dot);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Turn on all dots.
 */
static inline void display_printAllDots(void)
{
    display_clearAllSegments();
    display_setAllDots();
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print a digit on the display.
 *
 * @details
 *      This method prints a number according to the value of @p uc_number in
 *      the digit position @p uc_digit.
 *
 * @param[in]   uc_digit
 *      Number to be set. It can assume values in the range [0-10]. If it is 10
 *      a minus sign is set.
 *
 * @param[in]   uc_number
 *      Number to be printed.
 */
static inline void display_printDigit(uint8_t uc_digit, uint8_t uc_number)
{
    display_clearAllSegments();
    display_setDigit(uc_digit, uc_number);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print an integer number ranging from -99,999,999 to 999,999,999 to the
 *      display.
 *
 * @details
 *      This method prints an integer @p l_value ranging from -99,999,999 to
 *      999,999,999. Otherwise, there will be not enough digits on the display
 *      to represent it, and, consequently, the leftmost digits will not be
 *      printed on the display.
 *
 * @param[in]   l_value
 *      Signed integer ranging from -99,999,999 to 999,999,999.
 *
 * @param[in]   uc_posPaddingZeros
 *      Position of the leftmost zero used for padding.
 */
static inline void display_printNumber(int32_t l_value, uint8_t uc_posPaddingZeros)
{
    display_clearAllSegments();
    display_setNumber(l_value, uc_posPaddingZeros);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print a double number on the screen.
 *
 * @details
 *      This method prints the number @p d_value in double format with the
 *      precision determined by @p uc_numDecimalPlaces on the display.
 *
 * @param[in]   d_value
 *      Number in double format.
 *
 * @param[in]   uc_numDecimalPlaces
 *      Number of decimal places that should be used.
 */
static inline void display_printDouble(double_t d_value, uint8_t uc_numDecimalPlaces)
{
    display_clearAllSegments();
    display_setDouble(d_value, uc_numDecimalPlaces);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print a character.
 *
 * @param[in]   uc_digit
 *      Digit position.
 *
 * @param[in]   uc_data
 *      Character in ASCII format.
 */
static inline void display_printCharacter(uint8_t uc_digit, uint8_t uc_data)
{
    display_clearAllSegments();
    display_setCharacter(uc_digit, uc_data);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Print a string to the screen.
 *
 * @param[in]   *puc_string
 *      Pointer to the string.
 */
static inline void display_printString(uint8_t *puc_string)
{
    display_clearAllSegments();
    display_setString(puc_string);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Set temperature.
 *
 * @param[in]   d_temperature
 *      Temperature in double format.
 */
static inline void display_printTemperature(double_t d_temperature)
{
    display_clearAllSegments();
    display_setTemperature(d_temperature);
    display_print();
    while(display_stillTransmitting());
}

/**
 * @brief
 *      Check if there is a transmission in progress.
 *
 * @return
 *      Returns 'true' if a transmission is in progress. Otherwise, returns
 *      'false'.
 */
static inline bool display_stillTransmitting(void)
{
    return false;
}

#endif /* DISPLAY_H_ */
