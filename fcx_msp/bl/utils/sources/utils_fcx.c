/***************************************************************************//**
 * @brief
 * @file 	utils.c
 * @author 	Robson Cardoso
 * @date 	14/02/2017
 * @details 
 * @section license
 * 			FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "utils_fcx.h"
#include "general_settings.h"
#include "math.h"
#include "msp430fr6047.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint8_t utils_lutSearch(double_t d_X, const s_lut_coordinate_t *ps_lut,
                        uint8_t uc_size);
#if FIXED_POINT_CALC
uint8_t utils_lutSearchIQ(uint32_t d_X, const s_lut_coordinate_iq_t *ps_lut,
                        uint8_t uc_size);
#endif
double_t utils_calcStd(int16_t *pi_data, uint16_t ui_size);
double_t utils_calcSum(int16_t *pi_data, uint16_t ui_size);
double_t utils_calcSumPositives(int16_t *pi_data, uint16_t ui_size);
#if FIXED_POINT_CALC
int32_t utils_doubleToFixed(double_t input, uint8_t uc_fixedPoint);
double_t utils_fixedToDouble(int32_t input, uint8_t uc_fixedPoint);
int32_t utils_fixedAbs(int32_t input);
#endif
void utils_assert(bool b_expr);
/*******************************************************************************
 * Public functions
 ******************************************************************************/
double_t utils_linearInterp(double_t d_X, const s_lut_coordinate_t *ps_lut,
                            uint8_t uc_size)
{
    double_t d_Y = 0;
    double_t d_denom;
    double_t d_nom;
    double_t d_ratio;
    uint8_t uc_idx;

    if(d_X <= ps_lut[0].d_abscissa)
    {
        d_Y = ps_lut[0].d_ordinate;
    }
    else if(d_X >= ps_lut[uc_size-1].d_abscissa)
    {
        d_Y = ps_lut[uc_size-1].d_ordinate;
    }
    else
    {
        uc_idx = utils_lutSearch(d_X, ps_lut, uc_size);

        // Calculate (X1 - X0)
        d_denom = ps_lut[uc_idx].d_abscissa - ps_lut[uc_idx-1].d_abscissa;

        // Calculate (X - X0)
        d_nom = d_X - ps_lut[uc_idx-1].d_abscissa;

        // Calculate R = (X - X0)/(X1 - X0)
        d_ratio = d_nom / d_denom;

        // Calculate Y = (Y1 - Y0)*R + Y0
        d_Y = ps_lut[uc_idx].d_ordinate - ps_lut[uc_idx-1].d_ordinate;
        d_Y *= d_ratio;
        d_Y += ps_lut[uc_idx-1].d_ordinate;
    }

    return d_Y;
}

uint8_t utils_lutSearch(double_t d_X, const s_lut_coordinate_t *ps_lut,
                        uint8_t uc_size)
{
    uint8_t uc_idx;

    for(uc_idx = 0; (ps_lut[uc_idx].d_abscissa < d_X) && (uc_idx < uc_size);
        uc_idx++);
    return uc_idx;
}

#if FIXED_POINT_CALC
uint8_t utils_lutSearchIQ(uint32_t d_X, const s_lut_coordinate_iq_t *ps_lut,
                        uint8_t uc_size)
{
    uint8_t uc_idx;

    for(uc_idx = 0; (ps_lut[uc_idx].iq_abscissa < d_X) && (uc_idx < uc_size);
        uc_idx++);
    return uc_idx;
}
#endif

 double_t utils_calcStd(int16_t *pi_data, uint16_t ui_size)
{
    int i;
    double_t avg = 0;
    double_t d_stdDeviation = 0;

    avg = utils_calcSum(pi_data, ui_size);
    avg /= ui_size;

    for (i = 0; i < ui_size; ++i)
    {
        d_stdDeviation += pow((double) pi_data[i] - avg, 2);
    }

    return sqrt(d_stdDeviation / ui_size);
}

double_t utils_calcSum(int16_t *pi_data, uint16_t ui_size)
{
    int i;
    double_t d_sum = 0;

    for (i = 0; i < ui_size; ++i)
    {
        d_sum += pi_data[i];
    }

    return d_sum;
}

double_t utils_calcSumPositives(int16_t *pi_data, uint16_t ui_size)
{
    int i;
    double_t d_sum = 0;

    for (i = 0; i < ui_size; ++i)
    {
        if (pi_data[i] > 0)
        {
            d_sum += pi_data[i];
        }
    }

    return d_sum;
}

#if FIXED_POINT_CALC
int32_t utils_doubleToFixed(double_t input, uint8_t uc_fixedPoint)
{
    return (int32_t) (uint64_t) (input * (1LL << uc_fixedPoint));
}

double_t utils_fixedToDouble(int32_t input, uint8_t uc_fixedPoint)
{
    return ((double_t) input / (1LL << uc_fixedPoint));
}

int32_t utils_fixedAbs(int32_t input)
{
    if(input < 0)
    {
        input *= -1;
    }

    return input;
}

void utils_assert(bool b_expr)
{
#if !DEBUG
    while (!b_expr)
    {
        __disable_interrupt();
        // Enter into LPM4 to save energy and wait for WTD Reset
        __bis_SR_register(LPM4_bits);
    }
#endif
}

#endif
