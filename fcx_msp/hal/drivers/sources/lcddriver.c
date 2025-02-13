/***************************************************************************//**
 * @brief
 * @file    lcddriver.c
 * @author  Robson Cardoso
 * @date    16/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lcddriver.h"
#include "utils_fcx.h"
#include "driverlib.h"
#include "hal_lcd.h"
#include <string.h>

#ifdef DEBUG_ASSERT
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(__FILE__, __LINE__))
#else
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(expr))
#endif /* DEBUG_ASSERT */

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Memory buffer structure definition */
typedef struct S_LCDDRIVER_MEMBUF
{
    /** Byte index */
    uint8_t uc_byteIdx;
    /** Bit index */
    uint8_t uc_bitIdx;
} s_lcddriver_membuf_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/** External variables required for LCD driver unit test. */
#ifdef UNIT_TEST
/** Stores the last byte accessed by the method lcddriver_setSegment(). */
uint8_t guc_lastByteIdx;
/** Store the last bit accessed by the method lcddriver_setSegment(). */
uint8_t guc_lastBitMask;
#endif

/**
 * @brief
 *      Maps (COM, SIGNAL) to its position (BYTE, BIT) in
 *      driver' internal buffer as shown in the table below.
 *
 * | COM | SIGNAL | BYTE | BIT |  | COM | SIGNAL | BYTE | BIT |
 * |-----|--------|------|-----|--|-----|--------|------|-----|
 * |   0 | SEG0   |    0 |   1 |  |   1 | SEG0   |    0 |   2 |
 * |   0 | SEG1   |    1 |   1 |  |   1 | SEG1   |    1 |   2 |
 * |   0 | SEG2   |    2 |   1 |  |   1 | SEG2   |    2 |   2 |
 * |   0 | SEG3   |    3 |   1 |  |   1 | SEG3   |    3 |   2 |
 * |   0 | SEG4   |    4 |   1 |  |   1 | SEG4   |    4 |   2 |
 * |   0 | SEG5   |    5 |   1 |  |   1 | SEG5   |    5 |   2 |
 * |   0 | SEG6   |    6 |   1 |  |   1 | SEG6   |    6 |   2 |
 * |   0 | SEG7   |    7 |   1 |  |   1 | SEG7   |    7 |   2 |
 * |   0 | SEG8   |    8 |   1 |  |   1 | SEG8   |    8 |   2 |
 * |   0 | SEG9   |    9 |   1 |  |   1 | SEG9   |    9 |   2 |
 * |   0 | SEG10  |   10 |   1 |  |   1 | SEG10  |   10 |   2 |
 * |   0 | SEG11  |   11 |   1 |  |   1 | SEG11  |   11 |   2 |
 * |   0 | SEG12  |   12 |   1 |  |   1 | SEG12  |   12 |   2 |
 * |   0 | SEG13  |   13 |   1 |  |   1 | SEG13  |   13 |   2 |
 * |   0 | SEG14  |   14 |   1 |  |   1 | SEG14  |   14 |   2 |
 * |   0 | SEG15  |   15 |   1 |  |   1 | SEG15  |   15 |   2 |
 * |   0 | SEG16  |   16 |   1 |  |   1 | SEG16  |   16 |   2 |
 * |   0 | SEG17  |   17 |   1 |  |   1 | SEG17  |   17 |   2 |
 * |   0 | SEG18  |   17 |   1 |  |   1 | SEG18  |   17 |   2 |
 * |   0 | SEG19  |   18 |   1 |  |   1 | SEG19  |   18 |   2 |
 * |   0 | SEG20  |   19 |   1 |  |   1 | SEG20  |   19 |   2 |
 * |   0 | SEG21  |   20 |   1 |  |   1 | SEG21  |   20 |   2 |
 * |   0 | SEG22  |   21 |   1 |  |   1 | SEG22  |   21 |   2 |
 * |   0 | SEG23  |   22 |   1 |  |   1 | SEG23  |   22 |   2 |
 * |   0 | SEG24  |   23 |   1 |  |   1 | SEG24  |   23 |   2 |
 * |   0 | SEG25  |   24 |   1 |  |   1 | SEG25  |   24 |   2 |
 * |   0 | SEG26  |   25 |   1 |  |   1 | SEG26  |   25 |   2 |
 * |   0 | SEG27  |   26 |   1 |  |   1 | SEG27  |   26 |   2 |
 * |   0 | SEG28  |   27 |   1 |  |   1 | SEG28  |   27 |   2 |
 * |   0 | SEG29  |   28 |   1 |  |   1 | SEG29  |   28 |   2 |
 * |   0 | SEG30  |   29 |   1 |  |   1 | SEG30  |   29 |   2 |
 * |   0 | SEG31  |   30 |   1 |  |   1 | SEG31  |   30 |   2 |
 * |   0 | SEG32  |   31 |   1 |  |   1 | SEG32  |   31 |   2 |
 * |   0 | SEG33  |   32 |   1 |  |   1 | SEG33  |   32 |   2 |
 * |   0 | SEG34  |   33 |   1 |  |   1 | SEG34  |   33 |   2 |
 * |   0 | SEG35  |   34 |   1 |  |   1 | SEG35  |   34 |   2 |
 * |   0 | SEG36  |   35 |   1 |  |   1 | SEG36  |   35 |   2 |
 * |   0 | SEG37  |   36 |   1 |  |   1 | SEG37  |   36 |   2 |
 * |   0 | SEG38  |   37 |   1 |  |   1 | SEG38  |   37 |   2 |
 * |   0 | SEG39  |   38 |   1 |  |   1 | SEG39  |   38 |   2 |
 */
static const s_lcddriver_membuf_t gauc_mapSegToMemoryBuffer[LCDDRIVER_NUM_COM][LCDDRIVER_NUM_SIGNALS] =
{
    { /* COM0 */
        { /* SEG0 */
            .uc_byteIdx = 0,
            .uc_bitIdx  = 0x01
        },
        { /* SEG1 */
            .uc_byteIdx = 1,
            .uc_bitIdx  = 0x01
        },
        { /* SEG2 */
            .uc_byteIdx = 2,
            .uc_bitIdx  = 0x01
        },
        { /* SEG3 */
            .uc_byteIdx = 3,
            .uc_bitIdx  = 0x01
        },
        { /* SEG4 */
            .uc_byteIdx = 4,
            .uc_bitIdx  = 0x01
        },
        { /* SEG5 */
            .uc_byteIdx = 5,
            .uc_bitIdx  = 0x01
        },
        { /* SEG6 */
            .uc_byteIdx = 6,
            .uc_bitIdx  = 0x01
        },
        { /* SEG7 */
            .uc_byteIdx = 7,
            .uc_bitIdx  = 0x01
        },
        { /* SEG8 */
            .uc_byteIdx = 8,
            .uc_bitIdx  = 0x01
        },
        { /* SEG9 */
            .uc_byteIdx = 9,
            .uc_bitIdx  = 0x01
        },
        { /* SEG10 */
            .uc_byteIdx = 10,
            .uc_bitIdx  = 0x01
        },
        { /* SEG11 */
            .uc_byteIdx = 11,
            .uc_bitIdx  = 0x01
        },
        { /* SEG12 */
            .uc_byteIdx = 12,
            .uc_bitIdx  = 0x01
        },
        { /* SEG13 */
            .uc_byteIdx = 13,
            .uc_bitIdx  = 0x01
        },
        { /* SEG14 */
            .uc_byteIdx = 14,
            .uc_bitIdx  = 0x01
        },
        { /* SEG15 */
            .uc_byteIdx = 15,
            .uc_bitIdx  = 0x01
        },
        { /* SEG16 */
            .uc_byteIdx = 16,
            .uc_bitIdx  = 0x01
        },
        { /* SEG17 */
            .uc_byteIdx = 17,
            .uc_bitIdx  = 0x01
        },
        { /* PIN21 */ // NOT USED PIN MAP //
            .uc_byteIdx = 0x00,
            .uc_bitIdx  = 0x00
        },
        { /* SEG18 */
            .uc_byteIdx = 18,
            .uc_bitIdx  = 0x01
        },
        { /* SEG19 */
            .uc_byteIdx = 19,
            .uc_bitIdx  = 0x01
        },
        { /* SEG20 */
            .uc_byteIdx = 20,
            .uc_bitIdx  = 0x01
        },
        { /* SEG21 */
            .uc_byteIdx = 21,
            .uc_bitIdx  = 0x01
        },
        { /* SEG22 */
            .uc_byteIdx = 22,
            .uc_bitIdx  = 0x01
        },
        { /* SEG23 */
            .uc_byteIdx = 23,
            .uc_bitIdx  = 0x01
        },
        { /* SEG24 */
            .uc_byteIdx = 24,
            .uc_bitIdx  = 0x01
        },
        { /* SEG26 */
            .uc_byteIdx = 25,
            .uc_bitIdx  = 0x01
        },
        { /* SEG26 */
            .uc_byteIdx = 26,
            .uc_bitIdx  = 0x01
        },
        { /* SEG27 */
            .uc_byteIdx = 27,
            .uc_bitIdx  = 0x01
        },
        { /* SEG28 */
            .uc_byteIdx = 28,
            .uc_bitIdx  = 0x01
        },
        { /* SEG29 */
            .uc_byteIdx = 29,
            .uc_bitIdx  = 0x01
        },
        { /* SEG30 */
            .uc_byteIdx = 30,
            .uc_bitIdx  = 0x01
        },
        { /* SEG31 */
            .uc_byteIdx = 31,
            .uc_bitIdx  = 0x01
        },
        { /* SEG32 */
            .uc_byteIdx = 32,
            .uc_bitIdx  = 0x01
        },
        { /* SEG33 */
            .uc_byteIdx = 33,
            .uc_bitIdx  = 0x01
        },
        { /* SEG34 */
            .uc_byteIdx = 34,
            .uc_bitIdx  = 0x01
        },
        { /* SEG35 */
            .uc_byteIdx = 35,
            .uc_bitIdx  = 0x01
        },
        { /* SEG36 */
            .uc_byteIdx = 36,
            .uc_bitIdx  = 0x01
        },
        { /* SEG37 */
            .uc_byteIdx = 37,
            .uc_bitIdx  = 0x01
        },
        { /* SEG38 */
            .uc_byteIdx = 38,
            .uc_bitIdx  = 0x01
        },
    },

    { /* COM1 */
        { /* SEG0 */
            .uc_byteIdx = 0,
            .uc_bitIdx  = 0x02
        },
        { /* SEG1 */
            .uc_byteIdx = 1,
            .uc_bitIdx  = 0x02
        },
        { /* SEG2 */
            .uc_byteIdx = 2,
            .uc_bitIdx  = 0x02
        },
        { /* SEG3 */
            .uc_byteIdx = 3,
            .uc_bitIdx  = 0x02
        },
        { /* SEG4 */
            .uc_byteIdx = 4,
            .uc_bitIdx  = 0x02
        },
        { /* SEG5 */
            .uc_byteIdx = 5,
            .uc_bitIdx  = 0x02
        },
        { /* SEG6 */
            .uc_byteIdx = 6,
            .uc_bitIdx  = 0x02
        },
        { /* SEG7 */
            .uc_byteIdx = 7,
            .uc_bitIdx  = 0x02
        },
        { /* SEG8 */
            .uc_byteIdx = 8,
            .uc_bitIdx  = 0x02
        },
        { /* SEG9 */
            .uc_byteIdx = 9,
            .uc_bitIdx  = 0x02
        },
        { /* SEG10 */
            .uc_byteIdx = 10,
            .uc_bitIdx  = 0x02
        },
        { /* SEG11 */
            .uc_byteIdx = 11,
            .uc_bitIdx  = 0x02
        },
        { /* SEG12 */
            .uc_byteIdx = 12,
            .uc_bitIdx  = 0x02
        },
        { /* SEG13 */
            .uc_byteIdx = 13,
            .uc_bitIdx  = 0x02
        },
        { /* SEG14 */
            .uc_byteIdx = 14,
            .uc_bitIdx  = 0x02
        },
        { /* SEG15 */
            .uc_byteIdx = 15,
            .uc_bitIdx  = 0x02
        },
        { /* SEG16 */
            .uc_byteIdx = 16,
            .uc_bitIdx  = 0x02
        },
        { /* SEG17 */
            .uc_byteIdx = 17,
            .uc_bitIdx  = 0x02
        },
        { /* PIN21 */ // NOT USED PIN21 MAP //
            .uc_byteIdx = 0x00,
            .uc_bitIdx  = 0x00
        },
        { /* SEG18 */
            .uc_byteIdx = 18,
            .uc_bitIdx  = 0x02
        },
        { /* SEG19 */
            .uc_byteIdx = 19,
            .uc_bitIdx  = 0x02
        },
        { /* SEG20 */
            .uc_byteIdx = 20,
            .uc_bitIdx  = 0x02
        },
        { /* SEG21 */
            .uc_byteIdx = 21,
            .uc_bitIdx  = 0x02
        },
        { /* SEG22 */
            .uc_byteIdx = 22,
            .uc_bitIdx  = 0x02
        },
        { /* SEG23 */
            .uc_byteIdx = 23,
            .uc_bitIdx  = 0x02
        },
        { /* SEG24 */
            .uc_byteIdx = 24,
            .uc_bitIdx  = 0x02
        },
        { /* SEG25 */
            .uc_byteIdx = 25,
            .uc_bitIdx  = 0x02
        },
        { /* SEG26 */
            .uc_byteIdx = 26,
            .uc_bitIdx  = 0x02
        },
        { /* SEG27 */
            .uc_byteIdx = 27,
            .uc_bitIdx  = 0x02
        },
        { /* SEG28 */
            .uc_byteIdx = 28,
            .uc_bitIdx  = 0x02
        },
        { /* SEG29 */
            .uc_byteIdx = 29,
            .uc_bitIdx  = 0x02
        },
        { /* SEG30 */
            .uc_byteIdx = 30,
            .uc_bitIdx  = 0x02
        },
        { /* SEG31 */
            .uc_byteIdx = 31,
            .uc_bitIdx  = 0x02
        },
        { /* SEG32 */
            .uc_byteIdx = 32,
            .uc_bitIdx  = 0x02
        },
        { /* SEG33 */
            .uc_byteIdx = 33,
            .uc_bitIdx  = 0x02
        },
        { /* SEG34 */
            .uc_byteIdx = 34,
            .uc_bitIdx  = 0x02
        },
        { /* SEG35 */
            .uc_byteIdx = 35,
            .uc_bitIdx  = 0x02
        },
        { /* SEG36 */
            .uc_byteIdx = 36,
            .uc_bitIdx  = 0x02
        },
        { /* SEG37 */
            .uc_byteIdx = 37,
            .uc_bitIdx  = 0x02
        },
        { /* SEG38 */
            .uc_byteIdx = 38,
            .uc_bitIdx  = 0x02
        }
    }
};

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize LCD driver.
 */
void lcddriver_init(s_lcddriver_t *const ps_lcddriver)
{
    hal_lcd_Init();
    hal_lcd_turnonLCD();
    lcddriver_clearAllSegments(ps_lcddriver);
    ps_lcddriver->uc_length = 0;
}

/**
 * @brief
 *      Prepare a software reset command.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_packSwResetCmd(s_lcddriver_t *const ps_lcddriver)
{
}

/**
 * @brief
 *      Prepare a configuration command.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 *
 * @param[in]   *ps_lcdDriverConfig
 *      Configuration structure. For more information, please refer to
 *      @ref s_lcddriver_config_t structure definition.
 */
void lcddriver_packConfigCmd(s_lcddriver_t *const ps_lcddriver,
                             const s_lcddriver_config_t *ps_lcdDriverConfig)
{
}

/**
 * @brief
 *      Prepare a send data command.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_packDataCmd(s_lcddriver_t *const ps_lcddriver)
{
}

/**
 * @brief
 *      Update display LCD.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_print(s_lcddriver_t *const ps_lcddriver)
{
    hal_lcd_print(ps_lcddriver->auc_buffer);
}

/**
 * @brief
 *      Set a segment.
 *
 * @details
 *      This method sets the internal buffer segment mapped to the tuple
 *      (@p uc_com, @p uc_signal).
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 *
 * @param[in]   uc_com
 *      COM line of the segment.
 *
 * @param[in]   uc_signal
 *      Bit index of which segment to update.
 */
void lcddriver_setSegment(s_lcddriver_t *const ps_lcddriver,
                          uint8_t const uc_com,
                          uint8_t const uc_signal)
{
    s_lcddriver_membuf_t s_seg = gauc_mapSegToMemoryBuffer[uc_com][uc_signal];

    // Sanity checks
    ASSERT(uc_com < LCDDRIVER_NUM_COM);
    ASSERT(uc_signal < LCDDRIVER_NUM_SIGNALS);

    ps_lcddriver->auc_buffer[s_seg.uc_byteIdx] |= s_seg.uc_bitIdx;

#ifdef UNIT_TEST
    guc_lastByteIdx = s_seg.uc_byteIdx;
    guc_lastBitMask = s_seg.uc_bitIdx;
#endif
}

/**
 * @brief
 *      Clear a segment.
 *
 * @details
 *      This method clears the internal buffer segment mapped to the tuple
 *      (@p uc_com, @p uc_signal).
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 *
 * @param[in]   uc_com
 *      COM line of the segment.
 *
 * @param[in]   uc_signal
 *      Bit index of which segment to update.
 */
void lcddriver_clearSegment(s_lcddriver_t *const ps_lcddriver,
                            uint8_t const uc_com,
                            uint8_t const uc_signal)
{
    s_lcddriver_membuf_t s_seg = gauc_mapSegToMemoryBuffer[uc_com][uc_signal];

    // Sanity checks
    ASSERT(uc_com < LCDDRIVER_NUM_COM);
    ASSERT(uc_signal < LCDDRIVER_NUM_SIGNALS);

    ps_lcddriver->auc_buffer[s_seg.uc_byteIdx] &= ~s_seg.uc_bitIdx;

#ifdef UNIT_TEST
    guc_lastByteIdx = s_seg.uc_byteIdx;
    guc_lastBitMask = s_seg.uc_bitIdx;
#endif
}

/**
 * @brief
 *      Set a byte of LCD driver's internal buffer.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_setByte(s_lcddriver_t *const ps_lcddriver,
                       uint8_t const uc_com,
                       uint8_t const uc_signal)
{
    // Sanity checks
    ASSERT(uc_com < LCDDRIVER_NUM_COM);
    ASSERT(uc_signal < LCDDRIVER_NUM_SIGNALS);

    s_lcddriver_membuf_t s_seg = gauc_mapSegToMemoryBuffer[uc_com][uc_signal];
    ps_lcddriver->auc_buffer[s_seg.uc_byteIdx] = 0xFF;
}

/**
 * @brief
 *      Clear a byte of LCD driver's internal buffer.
 *
 * @param[in]   *ps_lcddriver uc_com uc_signal
 *      LCD driver class instance.
 */
void lcddriver_clearByte(s_lcddriver_t *const ps_lcddriver,
                         uint8_t const uc_com,
                         uint8_t const uc_signal)
{
    // Sanity checks
    ASSERT(uc_com < LCDDRIVER_NUM_COM);
    ASSERT(uc_signal < LCDDRIVER_NUM_SIGNALS);

    s_lcddriver_membuf_t s_seg = gauc_mapSegToMemoryBuffer[uc_com][uc_signal];
    ps_lcddriver->auc_buffer[s_seg.uc_byteIdx] = 0x00;
}

/**
 * @brief
 *      Set all segments.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_setAllSegments(s_lcddriver_t *const ps_lcddriver)
{
    uint8_t uc_idx;

    for(uc_idx = 0; uc_idx < LCDDRIVER_BUFFER_SIZE; uc_idx++)
    {
        ps_lcddriver->auc_buffer[uc_idx] = 0x03;
    }
}

/**
 * @brief
 *      Clear all segments.
 *
 * @param[in]   *ps_lcddriver
 *      LCD driver class instance.
 */
void lcddriver_clearAllSegments(s_lcddriver_t *const ps_lcddriver)
{
    memset((void*)ps_lcddriver, 0, sizeof(s_lcddriver_t));
}

void lcddriver_turnOnLcd(void)
{
    hal_lcd_turnonLCD();
}

void lcddriver_turnOffLcd(void)
{
    hal_lcd_turnoffLCD();
}
