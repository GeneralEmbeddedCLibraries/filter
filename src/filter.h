// Copyright (c) 2022 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      filter.h
*@brief     Various filter designs
*@author    Ziga Miklosic
*@date      02.01.2021
*@version   V1.0.2
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
#include "project_config.h"
#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Module version
 */
#define FILTER_VER_MAJOR		( 1 )
#define FILTER_VER_MINOR		( 0 )
#define FILTER_VER_DEVELOP		( 2 )

/**
 * 	Filter status
 */
typedef enum
{
	eFILTER_OK 			= 0x00,		/**<Normal operation */
	eFILTER_ERROR		= 0x01,		/**<General error */
} filter_status_t;

/**
 * 	RC filter instance type
 */
typedef struct filter_rc_s * p_filter_rc_t;

/**
 * 	CR filter instance type
 */
typedef struct filter_cr_s * p_filter_cr_t;

/**
 * 	FIR filter instance type
 */
typedef struct filter_fir_s * p_filter_fir_t;

/**
 * 	IIR filter instance type
 */
typedef struct filter_iir_s * p_filter_iir_t;

/**
 * 	Boolean filter instance type
 */
typedef struct filter_bool_s * p_filter_bool_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_init			(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value);
float32_t 		filter_rc_update		(p_filter_rc_t filter_inst, const float32_t x);
filter_status_t filter_rc_change_cutoff	(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs);
float32_t		filter_rc_get_cutoff	(p_filter_rc_t filter_inst);
bool			filter_rc_is_init		(p_filter_rc_t filter_inst);

filter_status_t filter_cr_init			(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order);
float32_t 		filter_cr_update		(p_filter_cr_t filter_inst, const float32_t x);
filter_status_t filter_cr_change_cutoff	(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs);
float32_t		filter_cr_get_cutoff	(p_filter_cr_t filter_inst);
bool			filter_cr_is_init		(p_filter_cr_t filter_inst);

filter_status_t filter_fir_init			(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order);
float32_t		filter_fir_update		(p_filter_fir_t filter_inst, const float32_t x);
filter_status_t	filter_fir_get_coeff	(p_filter_fir_t filter_inst, float32_t * const p_a);
bool			filter_fir_is_init		(p_filter_fir_t filter_inst);

filter_status_t filter_iir_init							(p_filter_iir_t * p_filter_inst, const float32_t * p_pole, const float32_t * p_zero, const uint32_t pole_size, const uint32_t zero_size);
float32_t		filter_iir_update						(p_filter_iir_t filter_inst, const float32_t x);
filter_status_t filter_iir_change_coeff					(p_filter_iir_t filter_inst, const float32_t * const p_pole, const float32_t * const p_zero);
filter_status_t filter_iir_calc_coeff_2nd_lpf			(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
filter_status_t filter_iir_calc_coeff_2nd_hpf			(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
filter_status_t filter_iir_calc_coeff_2nd_notch			(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero);
float32_t		filter_iir_calc_lpf_gain				(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size);
float32_t		filter_iir_calc_hpf_gain				(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size);
filter_status_t	filter_iir_lpf_norm_zeros_to_unity_gain	(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size);
filter_status_t	filter_iir_hpf_norm_zeros_to_unity_gain	(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size);
bool			filter_iir_is_init						(p_filter_iir_t filter_inst);
filter_status_t filter_iir_get_coeff					(p_filter_iir_t filter_inst, float32_t * const p_pole, float32_t * const p_zero);

filter_status_t	filter_bool_init			(p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl);
filter_status_t	filter_bool_is_init			(p_filter_bool_t filter_inst, bool * const p_is_init);
filter_status_t	filter_bool_update			(p_filter_bool_t filter_inst, const bool in, bool * const p_out);
filter_status_t filter_bool_get_fc			(p_filter_bool_t filter_inst, float32_t * const p_fc);
filter_status_t filter_bool_change_cutoff	(p_filter_bool_t filter_inst, const float32_t fc, const float32_t fs);

#endif // __FILTER_H


////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
