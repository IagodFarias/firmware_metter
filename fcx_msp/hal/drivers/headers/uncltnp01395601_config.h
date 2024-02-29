/***************************************************************************//**
 * @brief   Define the mapping
 * @file    uncltnp01395601_config.h.c
 * @author  Robson Cardoso
 * @date    17/02/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef UNCLTNP01395601_CONFIG_H_
#define UNCLTNP01395601_CONFIG_H_

/*******************************************************************************
 * Defines
 ******************************************************************************/

/** LCD parameters. */
#define DISPLAY_NUM_ICONS                       7       /** Number of icons */
#define DISPLAY_NUM_DIGITS                      9       /** Number of digits */
#define DISPLAY_NUM_DOTS                        8       /** Number of dots */
#define DISPLAY_NUM_SEGMENTS                    7       /** Number of segments of each digit */

/** LCD display configuration map. */
#define DISPLAY_UNCLTNP01395601_CONFIG                                          \
{                                                                               \
    .as_digits =                                                                \
    {                                                                           \
        { /* Digit 0 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 10,                                 \
            .uc_com[1] = 0, .uc_signal[1] = 11,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 11,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 12,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 13,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 13,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 12                                  \
        },                                                                      \
        { /* Digit 1 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 14,                                 \
            .uc_com[1] = 0, .uc_signal[1] = 15,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 15,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 16,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 17,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 17,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 16                                  \
        },                                                                      \
        { /* Digit 2 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 8,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 39,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 39,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 38,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 37,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 37,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 38                                  \
        },                                                                      \
        { /* Digit 3 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 7,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 36,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 36,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 35,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 34,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 34,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 35                                  \
        },                                                                      \
        { /* Digit 4 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 6,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 33,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 33,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 32,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 31,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 31,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 32                                  \
        },                                                                      \
        { /* Digit 5 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 4,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 30,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 30,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 29,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 28,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 28,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 29                                  \
        },                                                                      \
        { /* Digit 6 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 3,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 27,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 27,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 26,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 25,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 25,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 26                                  \
        },                                                                      \
        { /* Digit 7 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 2,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 24,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 24,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 23,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 22,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 22,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 23                                  \
        },                                                                      \
        { /* Digit 8 */                                                         \
            .uc_com[0] = 0, .uc_signal[0] = 1,                                  \
            .uc_com[1] = 0, .uc_signal[1] = 21,                                 \
            .uc_com[2] = 1, .uc_signal[2] = 21,                                 \
            .uc_com[3] = 1, .uc_signal[3] = 20,                                 \
            .uc_com[4] = 1, .uc_signal[4] = 19,                                 \
            .uc_com[5] = 0, .uc_signal[5] = 19,                                 \
            .uc_com[6] = 0, .uc_signal[6] = 20                                  \
        }                                                                       \
    },                                                                          \
    .as_icons =                                                                 \
    {                                                                           \
        { /* Reverse flow */                                                    \
            .uc_com     = 0,                                                    \
            .uc_signal  = 0                                                     \
        },                                                                      \
        { /* Leakage */                                                         \
            .uc_com     = 1,                                                    \
            .uc_signal  = 0                                                     \
        },                                                                      \
        { /* Air */                                                             \
            .uc_com     = 1,                                                    \
            .uc_signal  = 5                                                     \
        },                                                                      \
        { /* Radio transmission */                                              \
            .uc_com     = 0,                                                    \
            .uc_signal  = 5                                                     \
        },                                                                      \
        { /* Low battery */                                                     \
            .uc_com     = 0,                                                    \
            .uc_signal  = 9                                                     \
        },                                                                      \
        { /* M³ */                                                              \
            .uc_com     = 1,                                                    \
            .uc_signal  = 9                                                     \
        },                                                                      \
        { /* /h */                                                              \
            .uc_com     = 1,                                                    \
            .uc_signal  = 10                                                    \
        }                                                                       \
    },                                                                          \
    .as_dots =                                                                  \
    {                                                                           \
        { /* Position 0 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 14                                                    \
        },                                                                      \
        { /* Position 1 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 8                                                     \
        },                                                                      \
        { /* Position 2 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 7                                                     \
        },                                                                      \
        { /* Position 3 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 6                                                     \
        },                                                                      \
        { /* Position 4 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 4                                                     \
        },                                                                      \
        { /* Position 5 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 3                                                     \
        },                                                                      \
        { /* Position 6 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 2                                                     \
        },                                                                      \
        { /* Position 7 */                                                      \
            .uc_com     = 1,                                                    \
            .uc_signal  = 1                                                     \
        }                                                                       \
    }                                                                           \
}

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/** Enumeration to map the different icons available. */
typedef enum E_ICON
{
    /** Reverse flow indication */
    E_ICON_REVERSE_FLOW = 0,
    /** Water indication */
    E_ICON_WATER,
    /** Error indication */
    E_ICON_ERROR,
    /** Radio transmission indication */
    E_ICON_TRANSMISSION,
    /** Low battery indication */
    E_ICON_LOW_BAT,
    /** Cubic meter indication */
    E_ICON_M3,
    /** Flow rate indication */
    E_ICON_PER_HOUR
} E_ICON_t;

/** Enumeration to map the dots in different positions on the LCD display. */
typedef enum E_DOT
{
    /** Dot in position 0 (rightmost). */
    E_DOT_POSITION_0 = 0,
    /** Dot in position 1. */
    E_DOT_POSITION_1,
    /** Dot in position 2. */
    E_DOT_POSITION_2,
    /** Dot in position 3. */
    E_DOT_POSITION_3,
    /** Dot in position 4. */
    E_DOT_POSITION_4,
    /** Dot in position 5. */
    E_DOT_POSITION_5,
    /** Dot in position 6. */
    E_DOT_POSITION_6,
    /** Dot in position 7. */
    E_DOT_POSITION_7
} E_DOT_t;

#endif /* UNCLTNP01395601_CONFIG_H_ */
