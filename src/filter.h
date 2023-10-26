// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      filter.h
*@brief     Various filter designs
*@author    Ziga Miklosic
*@date      26.10.2023
*@version   V2.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup FILTER_API
* @{ <!-- BEGIN GROUP -->
*
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef __FILTER_H
#define __FILTER_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 *     Module version
 */
#define FILTER_VER_MAJOR        ( 2 )
#define FILTER_VER_MINOR        ( 0 )
#define FILTER_VER_DEVELOP      ( 0 )

/**
 *     Filter status
 */
typedef enum
{
    eFILTER_OK      = 0x00U,        /**<Normal operation */
    eFILTER_ERROR   = 0x01U,        /**<General error */
} filter_status_t;

/**
 *     RC filter instance type
 */
typedef struct filter_rc_s * p_filter_rc_t;

/**
 *     CR filter instance type
 */
typedef struct filter_cr_s * p_filter_cr_t;

/**
 *     FIR filter instance type
 */
typedef struct filter_fir_s * p_filter_fir_t;

/**
 *     IIR filter instance type
 */
typedef struct filter_iir_s * p_filter_iir_t;

/**
 *     Boolean filter instance type
 */
typedef struct filter_bool_s * p_filter_bool_t;

/**
 *  32-bit floating data type definition
 */
typedef float float32_t;

/**
 *  IIR coefficients
 */
typedef struct
{
    float32_t * p_pole;         /**<Pole values */
    float32_t * p_zero;         /**<Zero values */
    uint32_t    num_of_pole;    /**<Number of poles */
    uint32_t    num_of_zero;    /**<Number of zeros */
} filter_iir_coeff_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

// RC filter API
filter_status_t filter_rc_init          (p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value);
filter_status_t filter_rc_is_init       (p_filter_rc_t filter_inst, bool * const p_is_init);
filter_status_t filter_rc_hndl          (p_filter_rc_t filter_inst, const float32_t in, float32_t * const p_out);
filter_status_t filter_rc_reset         (p_filter_rc_t filter_inst, const float32_t rst_value);
filter_status_t filter_rc_fc_set        (p_filter_rc_t filter_inst, const float32_t fc);
filter_status_t filter_rc_fc_get        (p_filter_rc_t filter_inst, float32_t * const p_fc);
filter_status_t filter_rc_fs_get        (p_filter_rc_t filter_inst, float32_t * const p_fs);

// CR filter API
filter_status_t filter_cr_init          (p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order);
filter_status_t filter_cr_is_init       (p_filter_cr_t filter_inst, bool * const p_is_init);
filter_status_t filter_cr_hndl          (p_filter_cr_t filter_inst, const float32_t in, float32_t * const p_out);
filter_status_t filter_cr_reset         (p_filter_cr_t filter_inst);
filter_status_t filter_cr_fc_set        (p_filter_cr_t filter_inst, const float32_t fc);
filter_status_t filter_cr_fc_get        (p_filter_cr_t filter_inst, float32_t * const p_fc);
filter_status_t filter_cr_fs_get        (p_filter_cr_t filter_inst, float32_t * const p_fs);

// Boolean (debouncing) LPF filter API
filter_status_t filter_bool_init        (p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl);
filter_status_t filter_bool_is_init     (p_filter_bool_t filter_inst, bool * const p_is_init);
filter_status_t filter_bool_hndl        (p_filter_bool_t filter_inst, const bool in, bool * const p_out);
filter_status_t filter_bool_reset       (p_filter_bool_t filter_inst);
filter_status_t filter_bool_fc_set      (p_filter_bool_t filter_inst, const float32_t fc);
filter_status_t filter_bool_fc_get      (p_filter_bool_t filter_inst, float32_t * const p_fc);
filter_status_t filter_bool_fs_get      (p_filter_bool_t filter_inst, float32_t * const p_fs);

// FIR filter API
filter_status_t filter_fir_init         (p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order, const float32_t init_value);
filter_status_t filter_fir_is_init      (p_filter_fir_t filter_inst, bool * const p_is_init);
filter_status_t filter_fir_hndl         (p_filter_fir_t filter_inst, const float32_t in, float32_t * const p_out);
filter_status_t filter_fir_reset        (p_filter_fir_t filter_inst, const float32_t rst_val);
filter_status_t filter_fir_coeff_set    (p_filter_fir_t filter_inst, const float32_t * const p_a);
filter_status_t filter_fir_coeff_get    (p_filter_fir_t filter_inst, float32_t ** const pp_a);

// IIR filter API
filter_status_t filter_iir_init         (p_filter_iir_t * p_filter_inst, const filter_iir_coeff_t * const p_coeff);
filter_status_t filter_iir_is_init      (p_filter_iir_t filter_inst, bool * const p_is_init);
filter_status_t filter_iir_hndl         (p_filter_iir_t filter_inst, const float32_t in, float32_t * const p_out);
filter_status_t filter_iir_reset        (p_filter_iir_t filter_inst);
filter_status_t filter_iir_coeff_set    (p_filter_iir_t filter_inst, const filter_iir_coeff_t * const p_coeff);
filter_status_t filter_iir_coeff_get    (p_filter_iir_t filter_inst, filter_iir_coeff_t ** const pp_coeff);

// IIR helper functions
filter_status_t filter_iir_coeff_calc_2nd_lpf       (const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
filter_status_t filter_iir_coeff_calc_2nd_hpf       (const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
filter_status_t filter_iir_coeff_calc_2nd_bpf       (const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
float32_t       filter_iir_calc_lpf_gain            (const filter_iir_coeff_t * const p_coeff);
float32_t       filter_iir_calc_hpf_gain            (const filter_iir_coeff_t * const p_coeff);
filter_status_t filter_iir_coeff_to_unity_gain_lpf  (filter_iir_coeff_t * const p_coeff);
filter_status_t filter_iir_coeff_to_unity_gain_hpf  (filter_iir_coeff_t * const p_coeff);

#endif // __FILTER_H

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
