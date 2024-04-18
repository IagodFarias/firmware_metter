/***************************************************************************//**
 * @brief   
 * @file 	utils.h
 * @author 	Robson Cardoso
 * @date 	14/02/2017
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef UTILS_H_
#define UTILS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

#include "types.h"
#include "general_settings.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef DEBUG_ASSERT
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(__FILE__, __LINE__))
#else
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(expr))
#endif /* DEBUG_ASSERT */

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** Lookup table coordinate structure. */
typedef struct S_LUT_COORDINATE
{
    /** Abscissa */
    double_t d_abscissa;
    /** Ordinate */
    double_t d_ordinate;
} s_lut_coordinate_t;

#if FIXED_POINT_CALC
/** Lookup table coordinate structure. */
typedef struct S_LUT_COORDINATE_IQ
{
    /** Abscissa */
    uint32_t iq_abscissa;
    /** Ordinate */
    uint32_t iq_ordinate;
} s_lut_coordinate_iq_t;
#endif

/*******************************************************************************
 * Public functions
 ******************************************************************************/

#ifdef DEBUG_ASSERT
/** Due to footprint considerations, we only pass file name and line number, 
 *  not the assert expression (nor function name (C99)) */
void utils_assert(const uint8_t *puc_file, uint16_t ui_line);
#endif /* DEBUG_ASSERT */
/**
 * @brief
 *      Perform a linear interpolation.
 *
 * @details
 *      This method approximates a function \f$f\f$ by means of linear
 *      interpolation to determine the value of \f$Y=f(X)\f$ at the point
 *      \f$X\f$ according to the equation
 *      \f$Y=Y_{0}+(X-X_{0})\frac{Y_{1}-Y_{0}}{X_{1}-X_{0}}\f$, where
 *      \f$X_{1}\f$ and \f$X_{0}\f$ are the neighbors of \f$X\f$, and
 *      \f$Y_{1}\f$ and \f$Y_{0}\f$ are the values of the function \f$f\f$ when
 *      applied to \f$X_{1}\f$ and \f$X_{0}\f$ respectively.
 *
 *      A lookup table should be provided with abscissa and ordinate values
 *      which define the location of a point in two-dimensional rectangular
 *      space.
 *
 * @param[in]   d_X
 *      Abscissa of the point at which we desire to know the value of \f$f\f$.
 *
 * @param[in]   *ps_lut
 *      A pointer to the lookup table.
 *
 * @param[in]   uc_size
 *      The size of the lookup table.
 *
 * @return
 *      The value of \f$Y=f(X)\f$.
 */
double_t utils_linearInterp(double_t d_X, s_lut_coordinate_t *ps_lut,
                            uint8_t uc_size);
/**
 * @brief
 *      Search a lookup table.
 *
 * @details
 *      This method searches for the position \f$i\f$ in the lookup table such
 *      that \f$X_{i-1}<=X<X_{i}\f$, where \f$X\f$ is the searched value, and
 *      \f$X_{i-1}\f$ and \f$X_{i}\f$ are the abscissas in the neighborhood of
 *      \f$X\f$.
 *
 * @param[in]   d_X
 *      Searched value.
 *
 * @param[in]   *ps_lut
 *      Pointer to the lookup table.
 *
 * @param[in]   uc_size
 *      Size of the lookup table.
 *
 * @return
 *      Returns the index of the lookup table where the the number @p d_X lies
 *      within.
 */
uint8_t utils_lutSearch(double_t d_X, s_lut_coordinate_t *ps_lut,
                        uint8_t uc_size);
#if FIXED_POINT_CALC
uint8_t utils_lutSearchIQ(uint32_t d_X, s_lut_coordinate_iq_t *ps_lut,
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
#endif
