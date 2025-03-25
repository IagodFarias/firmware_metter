/***************************************************************************//**
 * @brief
 * @file    lcddriver.h
 * @author  Robson Cardoso
 * @date    16/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef LCDDRIVER_H_
#define LCDDRIVER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Misc. */
#define LCDDRIVER_I2C_SLAVE_ADDR                0x38                                    /** I2C slave device driver address */
#define LCDDRIVER_BUFFER_SIZE                   40                                      /** LCD driver's buffer size */
#define LCDDRIVER_NUM_COM                       2                                       /** LCD driver's number of COMs */
#define LCDDRIVER_NUM_SIGNALS                   40                                      /** LCD driver's number of signals */

/** Bit fields for reset register */
#define LCDDRIVER_REG_RESET_ADDR                0x00                                    /** Address of the reset command register */
#define LCDDRIVER_RESET_CMD_SHIFT               0x00                                    /** Shift value for Reset command */
#define LCDDRIVER_RESET_CMD_MASK                0xFF                                    /** Mask for Reset Command */
#define LCDDRIVER_RESET_CMD_DEFAULT             0x00                                    /** Default value for Reset Command */
#define LCDDRIVER_RESET_CMD_NO_RESET            (0x00 << LCDDRIVER_RESET_CMD_SHIFT)     /** No reset */
#define LCDDRIVER_RESET_CMD_SW_RESET            (0x2C << LCDDRIVER_RESET_CMD_SHIFT)     /** Software reset */

/* Bit fields for device control register */
#define LCDDRIVER_REG_DEVCTRL_ADDR              0x01                                    /** Address of the device control register */
#define LCDDRIVER_DEVCTRL_COE_SHIFT             0x00                                    /** Shift value for Clock Output Enable*/
#define LCDDRIVER_DEVCTRL_COE_MASK              0x01                                    /** Mask for Clock Output Enable */
#define LCDDRIVER_DEVCTRL_COE_DEFAULT           0x00                                    /** Default value for Clock Output Enable */
#define LCDDRIVER_DEVCTRL_COE_CLKOUT_DISABLE    (0x00 << LCDDRIVER_DEVCTRL_COE_SHIFT)   /** Close signal not available on pin CLK */
#define LCDDRIVER_DEVCTRL_COE_CLKOUT_ENABLE     (0x01 << LCDDRIVER_DEVCTRL_COE_SHIFT)   /** Clock signal available on pin CLK */
#define LCDDRIVER_DEVCTRL_OSC_SHIFT             0x01                                    /** Shit value for Internal Oscillator Control */
#define LCDDRIVER_DEVCTRL_OSC_MASK              0x01                                    /** Mask for Internal Oscillator Control */
#define LCDDRIVER_DEVCTRL_OSC_DEFAULT           0x00                                    /** Default value for Internal Oscillator Control */
#define LCDDRIVER_DEVCTRL_OSC_INTOSC_ENABLE     (0x00 << LCDDRIVER_DEVCTRL_OSC_SHIFT)   /** Internal Oscillator Control enabled */
#define LCDDRIVER_DEVCTRL_OSC_INTOSC_DISABLE    (0x01 << LCDDRIVER_DEVCTRL_OSC_SHIFT)   /** Internal Oscillator Control disabled */
#define LCDDRIVER_DEVCTRL_FF_SHIFT              0x02                                    /** Shift value for Frame Frequency Selection */
#define LCDDRIVER_DEVCTRL_FF_MASK               0x03                                    /** Mask for Frame Frequency Selection */
#define LCDDRIVER_DEVCTRL_FF_DEFAULT            0x01                                    /** Default value for Frame Frequency Selection */
#define LCDDRIVER_DEVCTRL_FF_32HZ               (0x00 << LCDDRIVER_DEVCTRL_FF_SHIFT)    /** Select f_{fr} = 32 Hz*/
#define LCDDRIVER_DEVCTRL_FF_64HZ               (0x01 << LCDDRIVER_DEVCTRL_FF_SHIFT)    /** Select f_{fr} = 64 Hz */
#define LCDDRIVER_DEVCTRL_FF_96HZ               (0x10 << LCDDRIVER_DEVCTRL_FF_SHIFT)    /** Select f_{fr} = 96 Hz */
#define LCDDRIVER_DEVCTRL_FF_128HZ              (0x11 << LCDDRIVER_DEVCTRL_FF_SHIFT)    /** Select f_{fr} = 128 Hz */

/* Bit fields for display control 1 register */
#define LCDDRIVER_REG_DCTRL1_ADDR               0x02                                    /** Address of the display control 1 register */
#define LCDDRIVER_DCTRL1_DE_SHIFT               0x00                                    /** Shift value for Display Enable */
#define LCDDRIVER_DCTRL1_DE_MASK                0x01                                    /** Mask for Display Enable */
#define LCDDRIVER_DCTRL1_DE_DEFAULT             0x00                                    /** Default value for Display Enable */
#define LCDDRIVER_DCTRL1_DE_DISABLE             (0x00 << LCDDRIVER_DCTRL1_DE_SHIFT)     /** Disable display */
#define LCDDRIVER_DCTRL1_DE_ENABLE              (0x01 << LCDDRIVER_DCTRL1_DE_SHIFT)     /** Enable display */
#define LCDDRIVER_DCTRL1_B_SHIFT                0x01                                    /** Shift value for Bias Mode Selection */
#define LCDDRIVER_DCTRL1_B_MASK                 0x01                                    /** Mask value for Bias Mode Selection */
#define LCDDRIVER_DCTRL1_B_DEFAULT              0x00                                    /** Default value for Bias Mode Selection */
#define LCDDRIVER_DCTRL1_B_ONE_THIRD            (0x00 << LCDDRIVER_DCTRL1_B_SHIFT)      /** Select 1/3 bias */
#define LCDDRIVER_DCTRL1_B_ONE_HALF             (0x01 << LCDDRIVER_DCTRL1_B_SHIFT)      /** Select 1/2 bias */
#define LCDDRIVER_DCTRL1_MUX_SHIFT              0x02                                    /** Shit value for Multiplex Drive Mode Selection */
#define LCDDRIVER_DCTRL1_MUX_MASK               0x03                                    /** Mask for Multiplex Drive Mode Selection */
#define LCDDRIVER_DCTRL1_MUX_DEFAULT            0x00                                    /** Default value for Multiplex Drive Mode Selection */
#define LCDDRIVER_DCTRL1_MUX_4                  (0x00 << LCDDRIVER_DCTRL1_MUX_SHIFT)    /** Select 1:4 multiplex mode (COM0 to COM3) */
#define LCDDRIVER_DCTRL1_MUX_3                  (0x01 << LCDDRIVER_DCTRL1_MUX_SHIFT)    /** Select 1:3 multiplex mode (COM0 to COM2) */
#define LCDDRIVER_DCTRL1_MUX_2                  (0x02 << LCDDRIVER_DCTRL1_MUX_SHIFT)    /** Select 1:2 multiplex mode (COM0 to COM1) */
#define LCDDRIVER_DCTRL1_MUX_1                  (0x03 << LCDDRIVER_DCTRL1_MUX_SHIFT)    /** Select static drive mode (COM0) */
#define LCDDRIVER_DCTRL1_BOOST_SHIFT            0x04                                    /** Shift value for Large Display Mode Support */
#define LCDDRIVER_DCTRL1_BOOST_MASK             0x01                                    /** Mask for Large Display Mode Support */
#define LCDDRIVER_DCTRL1_BOOST_DEFAULT          0x00                                    /** Default value for Large Display Mode Support */
#define LCDDRIVER_DCTRL1_BOOST_STANDARD         (0x00 << LCDDRIVER_DCTRL1_BOOST_SHIFT)  /** Select standard power drive scheme */
#define LCDDRIVER_DCTRL1_BOOST_ENHANCED         (0x01 << LCDDRIVER_DCTRL1_BOOST_SHIFT)  /** Select enhanced power drive mode for higher display loads */

/* Bit fields for display control 2 register */
#define LCDDRIVER_REG_DCTRL2_ADDR               0x03                                    /** Address of the display control 2 register */
#define LCDDRIVER_DCTRL2_INV_SHIFT              0x00                                    /** Shift value for Inversion Mode Selection */
#define LCDDRIVER_DCTRL2_INV_MASK               0x01                                    /** Mask for Inversion Mode Selection */
#define LCDDRIVER_DCTRL2_INV_DEFAULT            0x00                                    /** Default value for Inversion Mode Selection */
#define LCDDRIVER_DCTRL2_INV_LINE               (0x00 << LCDDRIVER_DCTRL2_INV_SHIFT)    /** Select line inversion mode */
#define LCDDRIVER_DCTRL2_INV_FRAME              (0x01 << LCDDRIVER_DCTRL2_INV_SHIFT)    /** Select frame inversion mode */
#define LCDDRIVER_DCTRL2_BL_SHIFT               0x01                                    /** Shift value for Blink Control */
#define LCDDRIVER_DCTRL2_BL_MASK                0x03                                    /** Mask for Blink Control */
#define LCDDRIVER_DCTRL2_BL_DEFAULT             0x00                                    /** Default for Blink Control */
#define LCDDRIVER_DCTRL2_BL_OFF                 (0x00 << LCDDRIVER_DCTRL2_BL_SHIFT)     /** Blinking off */
#define LCDDRIVER_DCTRL2_BL_500MS               (0x01 << LCDDRIVER_DCTRL2_BL_SHIFT)     /** Blinking on with frequency 0.5 Hz */
#define LCDDRIVER_DCTRL2_BL_1HZ                 (0x02 << LCDDRIVER_DCTRL2_BL_SHIFT)     /** Blinking on with frequency 1 Hz */
#define LCDDRIVER_DCTRL2_BL_2HZ                 (0x03 << LCDDRIVER_DCTRL2_BL_SHIFT)     /** Blinking on with frequency 2 Hz */

/** Bit fields for device data register */
#define LCDDRIVER_REG_DATA_ADDR                 0x04                                    /* Address of the display data register */

/** Enable LCD configuration. */
#define LCDDRIVER_CONF_ENABLE_LCD                                               \
{                                                                               \
    .e_clockOutEnable       = E_LCDDRIVER_COE_DISABLE,                          \
    .e_oscOutEnable         = E_LCDDRIVER_OSC_ENABLE,                           \
    .e_frameFrequencySel    = E_LCDDRIVER_FF_32HZ,                              \
    .e_displayEnable        = E_LCDDRIVER_DE_ENABLE,                            \
    .e_biasSel              = E_LCDDRIVER_BIAS_ONE_HALF,                        \
    .e_muxDriveSel          = E_LCDDRIVER_MUX_2,                                \
    .e_boostModeSel         = E_LCDDRIVER_BOOST_STANDARD,                       \
    .e_invModeSel           = E_LCDDRIVER_INV_LINE,                             \
    .e_blinkModeSel         = E_LCDDRIVER_BLINK_OFF                             \
}

/** Disable LCD configuration. */
#define LCDDRIVER_CONF_DISABLE_LCD                                              \
{                                                                               \
    .e_clockOutEnable       = E_LCDDRIVER_COE_DISABLE,                          \
    .e_oscOutEnable         = E_LCDDRIVER_OSC_ENABLE,                           \
    .e_frameFrequencySel    = E_LCDDRIVER_FF_32HZ,                              \
    .e_displayEnable        = E_LCDDRIVER_DE_DISABLE,                           \
    .e_biasSel              = E_LCDDRIVER_BIAS_ONE_HALF,                        \
    .e_muxDriveSel          = E_LCDDRIVER_MUX_2,                                \
    .e_boostModeSel         = E_LCDDRIVER_BOOST_STANDARD,                       \
    .e_invModeSel           = E_LCDDRIVER_INV_LINE,                             \
    .e_blinkModeSel         = E_LCDDRIVER_BLINK_OFF                             \
}

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/** Clock output setting */
typedef enum
{
    /** Enable clock output */
    E_LCDDRIVER_COE_ENABLE      = LCDDRIVER_DEVCTRL_COE_CLKOUT_ENABLE,
    /** Disable clock output */
    E_LCDDRIVER_COE_DISABLE     = LCDDRIVER_DEVCTRL_COE_CLKOUT_DISABLE
} E_LCDDRIVER_COE_t;

/** Internal oscillator output setting */
typedef enum E_LCDDRIVER_OSC
{
    /** Enable internal oscillator output */
    E_LCDDRIVER_OSC_ENABLE      = LCDDRIVER_DEVCTRL_OSC_INTOSC_ENABLE,
    /** Disable internal oscillator output */
    E_LCDDRIVER_OSC_DISABLE     = LCDDRIVER_DEVCTRL_OSC_INTOSC_DISABLE
} E_LCDDRIVER_OSC_t;

/** Frame frequency setting */
typedef enum E_LCDDRIVER_FF
{
    /** Select 32Hz*/
    E_LCDDRIVER_FF_32HZ         = LCDDRIVER_DEVCTRL_FF_32HZ,
    /** Select 64Hz*/
    E_LCDDRIVER_FF_64HZ         = LCDDRIVER_DEVCTRL_FF_64HZ,
    /** Select 96Hz*/
    E_LCDDRIVER_FF_96HZ         = LCDDRIVER_DEVCTRL_FF_96HZ,
    /** Select 128Hz*/
    E_LCDDRIVER_FF_128HZ        = LCDDRIVER_DEVCTRL_FF_128HZ
} E_LCDDRIVER_FF_t;

/** Display enable setting */
typedef enum E_LCDDRIVER_DE
{
    /** Enable display */
    E_LCDDRIVER_DE_ENABLE       = LCDDRIVER_DCTRL1_DE_ENABLE,
    /** Disable display */
    E_LCDDRIVER_DE_DISABLE      = LCDDRIVER_DCTRL1_DE_DISABLE
} E_LCDDRIVER_DE_t;

/** Bias mode setting */
typedef enum E_LCDDRIVER_BIAS
{
    /** Select 1/3 bias */
    E_LCDDRIVER_BIAS_ONE_THIRD  = LCDDRIVER_DCTRL1_B_ONE_THIRD,
    /** Select 1/2 bias */
    E_LCDDRIVER_BIAS_ONE_HALF   = LCDDRIVER_DCTRL1_B_ONE_HALF
} E_LCDDRIVER_BIAS_t;

/** Multiplex driver mode setting */
typedef enum E_LCDDRIVER_MUX
{
    /** Select static drive mode (COM0) */
    E_LCDDRIVER_MUX_1           = LCDDRIVER_DCTRL1_MUX_1,
    /** Select 1:2 multiplex mode (COM0 to COM1) */
    E_LCDDRIVER_MUX_2           = LCDDRIVER_DCTRL1_MUX_2,
    /** Select 1:3 multiplex mode (COM0 to COM2) */
    E_LCDDRIVER_MUX_3           = LCDDRIVER_DCTRL1_MUX_3,
    /** Select 1:4 multiplex mode (COM0 to COM3) */
    E_LCDDRIVER_MUX_4           = LCDDRIVER_DCTRL1_MUX_4
} E_LCDDRIVER_MUX_t;

/** Display mode setting */
typedef enum E_LCDDRIVER_BOOST
{
    /** Select standard power drive scheme */
    E_LCDDRIVER_BOOST_STANDARD  = LCDDRIVER_DCTRL1_BOOST_STANDARD,
    /** Select enhanced power drive mode for higher display loads */
    E_LCDDRIVER_BOOST_ENHANCED  = LCDDRIVER_DCTRL1_BOOST_ENHANCED
} E_LCDDRIVER_BOOST_t;

/** Inversion mode setting */
typedef enum E_LCDDRIVER_INV
{
    /** Select line inversion mode */
    E_LCDDRIVER_INV_LINE        = LCDDRIVER_DCTRL2_INV_LINE,
    /** Select frame inversion mode */
    E_LCDDRIVER_INV_FRAME       = LCDDRIVER_DCTRL2_INV_FRAME
} E_LCDDRIVER_INV_t;

/** Blink control setting */
typedef enum E_LCDDRIVER_BLINK
{
    /** Blinking off */
    E_LCDDRIVER_BLINK_OFF       = LCDDRIVER_DCTRL2_BL_OFF,
    /** Blinking on with frequency 0.5 Hz */
    E_LCDDRIVER_BLINK_500MS     = LCDDRIVER_DCTRL2_BL_500MS,
    /** Blinking on with frequency 1 Hz */
    E_LCDDRIVER_BLINK_1HZ       = LCDDRIVER_DCTRL2_BL_1HZ,
    /** Blinking on with frequency 2 Hz */
    E_LCDDRIVER_BLINK_2HZ       = LCDDRIVER_DCTRL2_BL_2HZ
} E_LCDDRIVER_BLINK_t;

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** LCD driver configuration structure */
typedef struct S_LCDDRIVER_CONFIG
{
    /** Clock output configuration */
    E_LCDDRIVER_COE_t e_clockOutEnable;
    /** Internal oscillation output configuration */
    E_LCDDRIVER_OSC_t e_oscOutEnable;
    /** Frame frequency selection */
    E_LCDDRIVER_FF_t e_frameFrequencySel;
    /** Display enable setting */
    E_LCDDRIVER_DE_t e_displayEnable;
    /** Bias selection */
    E_LCDDRIVER_BIAS_t e_biasSel;
    /** Multiplex driver mode selection */
    E_LCDDRIVER_MUX_t e_muxDriveSel;
    /** Display mode selection */
    E_LCDDRIVER_BOOST_t e_boostModeSel;
    /** Inversion mode selection */
    E_LCDDRIVER_INV_t e_invModeSel;
    /** Blink control selection */
    E_LCDDRIVER_BLINK_t e_blinkModeSel;
} s_lcddriver_config_t;

/** LCD driver class */
typedef struct S_LCDDRIVER
{
    /** Size of the package */
    uint8_t uc_length;
    /** Internal buffer to store the converted package */
    uint8_t auc_buffer[LCDDRIVER_BUFFER_SIZE];
} s_lcddriver_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void lcddriver_init(s_lcddriver_t *const ps_lcddriver);
void lcddriver_packSwResetCmd(s_lcddriver_t *const ps_lcddriver);
void lcddriver_packConfigCmd(s_lcddriver_t *const ps_lcddriver,
                             const s_lcddriver_config_t *ps_initConf);
void lcddriver_packDataCmd(s_lcddriver_t *const ps_lcddriver);
void lcddriver_print(s_lcddriver_t *const ps_lcddriver);
void lcddriver_setSegment(s_lcddriver_t *const ps_lcddriver, 
                          uint8_t const uc_com,
                          uint8_t const uc_signal);
void lcddriver_clearSegment(s_lcddriver_t *const ps_lcddriver,
                            uint8_t const uc_com,
                            uint8_t const uc_signal);
void lcddriver_setByte(s_lcddriver_t *const ps_lcddriver, 
                       uint8_t const uc_com,
                       uint8_t const uc_signal);
void lcddriver_clearByte(s_lcddriver_t *const ps_lcddriver,
                         uint8_t const uc_com,
                         uint8_t const uc_signal);
void lcddriver_setAllSegments(s_lcddriver_t *const ps_lcddriver);
void lcddriver_clearAllSegments(s_lcddriver_t *const ps_lcddriver);
void lcddriver_turnOnLcd(void);
void lcddriver_turnOffLcd(void);

#endif /* LCDDRIVER_H_ */
