// Copyright (c) 2025 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      filter.c
*@brief     Various filter designs
*@email     ziga.miklosic@gmail.com
*@date      30.08.2025
*@version   V3.0.0
*
*@section   Description
*   
*   This module contains different kind of digital filter
*   implementation. All of the following filter types are
*   being simulated in python for validation purposes.
*
*@section     Dependencies
*
*     Some implementation of filters uses ring buffers.
*
*/
////////////////////////////////////////////////////////////////////////////////
/*!
* @addtogroup FILTER
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "filter.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/**
 *     Compatibility check with RING_BUFFER
 *
 *     Support version V3.x.x
 */
_Static_assert( 3 == RING_BUFFER_VER_MAJOR );

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static filter_status_t  filter_rc_calculate_alpha   (const float32_t fc, const float32_t fs, float32_t * const p_alpha);
static filter_status_t  filter_cr_calculate_alpha   (const float32_t fc, const float32_t fs, float32_t * const p_alpha);
static void             filter_buf_fill             (const p_ring_buffer_t buf_inst, const float32_t val);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*       Calculate RC alpha
*
* @param[in]    fc      - Cutoff frequency
* @param[in]    fs      - Sample frequency
* @param[out]   alpha   - CR alpha
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static filter_status_t filter_rc_calculate_alpha(const float32_t fc, const float32_t fs, float32_t * const p_alpha)
{
    // Check Nyquist/Shannon sampling theorem
    if  (   ( fc <= ( fs / 2.0f ))
        &&  ( fs > 0.0f )
        &&  ( fc > 0.0f )
        &&  ( p_alpha != NULL ))
    {
        *p_alpha = (float32_t) ( 1.0f / ( 1.0f + ( fs / ( UTILS_TWOPI * fc ))));
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Calculate CR alpha
*
* @param[in]    fc      - Cutoff frequency
* @param[in]    fs      - Sample frequency
* @param[out]   alpha   - CR alpha
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static filter_status_t filter_cr_calculate_alpha(const float32_t fc, const float32_t fs, float32_t * const p_alpha)
{
    // Check Nyquist/Shannon sampling theorem
    if (    ( fc <= ( fs / 2.0f ))
        &&  ( fs > 0.0f )
        &&  ( fc > 0.0f )
        &&  ( p_alpha != NULL ))
    {
        *p_alpha = (float32_t) (( 1.0f / ( UTILS_TWOPI * fc )) / (( 1.0f / fs ) + ( 1.0f / ( UTILS_TWOPI * fc ))));
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Fill sample buffer with value
*
* @param[in]    buf_inst    - Sample buffer instance
* @param[in]    val         - Value to fill buffer with
* @return       void
*/
////////////////////////////////////////////////////////////////////////////////
static void filter_buf_fill(const p_ring_buffer_t buf_inst, const float32_t val)
{
    // Fill with wanted value
    for ( uint32_t i = 0; i < ring_buffer_get_size( buf_inst ); i++ )
    {
        (void) ring_buffer_add( buf_inst, (float32_t*) &val );
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup FILTER_API
* @{ <!-- BEGIN GROUP -->
*
*   Following functions are part of API calls.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize RC filter
*
* @note Order of RC filter is represented as number of cascaded RC
*        analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in]    p_filter_inst   - Pointer to RC filter instance
* @param[in]    fc              - Filter cutoff frequency
* @param[in]    fs              - Sample frequency
* @param[in]    order           - Order of filter (number of cascaded filter)
* @param[in]    init_value      - Initial value
* @return       status          - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_init(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)
{
    if ( NULL == p_filter_inst ) return eFILTER_ERROR_INST;

    // Check if that instance is already allocated
    // By meaning that this buffer instance was initialised before...
    if ( NULL != *p_filter_inst ) return eFILTER_ERROR_INIT;

    // Allocate space
    *p_filter_inst          = calloc( 1U, sizeof(filter_rc_t));
    (*p_filter_inst)->p_y   = calloc( 1U, order * sizeof(float32_t));

    // Check allocation
    if (( NULL == *p_filter_inst ) || ( NULL == (*p_filter_inst)->p_y ))
    {
        free(*p_filter_inst);
        free((*p_filter_inst)->p_y);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_MEM;
    }

    // Calculate coefficient
    if ( eFILTER_OK == filter_rc_calculate_alpha( fc, fs, &(*p_filter_inst)->alpha ))
    {
        // Store order & fc
        (*p_filter_inst)->order = order;
        (*p_filter_inst)->fc = fc;
        (*p_filter_inst)->fs = fs;

        // Initial value
        for ( uint32_t i = 0; i < order; i++)
        {
            (*p_filter_inst)->p_y[i] = init_value;
        }

        // Init success
        (*p_filter_inst)->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        free((*p_filter_inst)->p_y);
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize statically RC filter
*
* @note Order of RC filter is represented as number of cascaded RC
*        analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    fc          - Filter cutoff frequency
* @param[in]    fs          - Sample frequency
* @param[in]    order       - Order of filter (number of cascaded filter)
* @param[in]    init_value  - Initial value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_init_static(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)
{
    if ( NULL == filter_inst ) return eFILTER_ERROR_INST;

    // Check if filter memory is allocated
    if ( NULL == filter_inst->p_y ) return eFILTER_ERROR_INIT;

    // Calculate coefficient
    if ( eFILTER_OK == filter_rc_calculate_alpha( fc, fs, &filter_inst->alpha ))
    {
        // Store order & fc
        filter_inst->order = order;
        filter_inst->fc = fc;
        filter_inst->fs = fs;

        // Initial value
        for ( uint32_t i = 0; i < order; i++)
        {
            filter_inst->p_y[i] = init_value;
        }

        // Init success
        filter_inst->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get initialization status of RC filter
*
* @param[in]    filter_inst - RC filter instance
* @return       true if buffer instance is initialized
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_rc_is_init(p_filter_rc_t filter_inst)
{
    if ( NULL != filter_inst )
    {
        return filter_inst->is_init;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle RC filter
*
* @note This function must be called in equidistant time period defined by 1/fs!
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    in          - Input value
* @return       Output (filtered) value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_rc_hndl(p_filter_rc_t filter_inst, const float32_t in)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init )) return 0;

    for ( uint32_t n = 0; n < filter_inst->order; n++)
    {
        if ( 0 == n )
        {
            filter_inst->p_y[0] = ( filter_inst->p_y[0] + ( filter_inst->alpha * ( in - filter_inst->p_y[0] )));
        }
        else
        {
            filter_inst->p_y[n] = ( filter_inst->p_y[n] + ( filter_inst->alpha * ( filter_inst->p_y[n-1] - filter_inst->p_y[n] )));
        }
    }

    return filter_inst->p_y[ filter_inst->order - 1U ];
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset RC filter buffers
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    rst_value   - Reset value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_reset(p_filter_rc_t filter_inst, const float32_t rst_value)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    // Initial value
    for ( uint32_t i = 0; i < filter_inst->order; i++)
    {
        filter_inst->p_y[i] = rst_value;
    }

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set cutoff frequency of RC filter on-the-fly
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    fc          - Cutoff frequency
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_set_fc(p_filter_rc_t filter_inst, const float32_t fc)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    if ( fc != filter_rc_get_fc( filter_inst ))
    {
        float32_t alpha;

        if ( eFILTER_OK == filter_rc_calculate_alpha( fc, filter_inst->fs, &alpha ))
        {
            filter_inst->alpha = alpha;
            filter_inst->fc = fc;
            return eFILTER_OK;
        }
    }

    return eFILTER_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get RC filter cutoff frequency
*
* @param[in]    filter_inst - RC filter instance
* @return       Filter cutoff frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_rc_get_fc(p_filter_rc_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return 0;
    }
    else
    {
        return filter_inst->fc;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get RC filter sampling frequency
*
* @param[in]    filter_inst - RC filter instance
* @return       Filter sampling frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_rc_get_fs(p_filter_rc_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return 0;
    }
    else
    {
        return filter_inst->fs;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize CR filter
*
* @note Order of CR filter is represented as number of cascaded CR
*       analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in]    p_filter_inst   - Pointer to CR filter instance
* @param[in]    fc              - Filter cutoff frequency
* @param[in]    fs              - Sample frequency
* @param[in]    order           - Order of filter (number of cascaded filter)
* @return       status          - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_init(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)
{
    if ( NULL == p_filter_inst ) return eFILTER_ERROR_INST;

    // Check if that instance is already allocated
    // By meaning that this buffer instance was initialised before...
    if ( NULL != *p_filter_inst ) return eFILTER_ERROR_INIT;

    // Allocate space
    *p_filter_inst          = calloc( 1U, sizeof(filter_cr_t));
    (*p_filter_inst)->p_y   = calloc( 1U, order * sizeof(float32_t));
    (*p_filter_inst)->p_x   = calloc( 1U, order * sizeof(float32_t));

    // Check if allocation succeed
    if (( NULL == *p_filter_inst ) || ( NULL == (*p_filter_inst)->p_y ) || ( NULL == (*p_filter_inst)->p_x ))
    {
        free(*p_filter_inst);
        free((*p_filter_inst)->p_x);
        free((*p_filter_inst)->p_y);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_MEM;
    }

    // Calculate coefficient
    if ( eFILTER_OK == filter_cr_calculate_alpha( fc, fs, &(*p_filter_inst)->alpha ))
    {
        // Store order & fc
        (*p_filter_inst)->order = order;
        (*p_filter_inst)->fc = fc;
        (*p_filter_inst)->fs = fs;

        // Initial value
        for ( uint32_t i = 0; i < order; i++)
        {
            (*p_filter_inst)->p_y[i] = 0.0f;
            (*p_filter_inst)->p_x[i] = 0.0f;
        }

        // Init success
        (*p_filter_inst)->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        free((*p_filter_inst)->p_x);
        free((*p_filter_inst)->p_y);
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize statically CR filter
*
* @note    Order of CR filter is represented as number of cascaded CR
*        analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in]    filter_inst - CR filter instance
* @param[in]    fc          - Filter cutoff frequency
* @param[in]    fs          - Sample frequency
* @param[in]    order       - Order of filter (number of cascaded filter)
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_init_static(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)
{
    if ( NULL == filter_inst ) return eFILTER_ERROR_INST;

    // Check if filter memory is allocated
    if (( NULL == filter_inst->p_y ) || ( NULL == filter_inst->p_x )) return eFILTER_ERROR_INIT;

    // Calculate coefficient
    if ( eFILTER_OK == filter_cr_calculate_alpha( fc, fs, &filter_inst->alpha ))
    {
        // Store order & fc
        filter_inst->order = order;
        filter_inst->fc = fc;
        filter_inst->fs = fs;

        // Initial value
        for ( uint32_t i = 0; i < order; i++)
        {
            filter_inst->p_y[i] = 0.0f;
            filter_inst->p_x[i] = 0.0f;
        }

        // Init success
        filter_inst->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get initialization status of CR filter
*
* @param[in]    filter_inst - CR filter instance
* @return       true if buffer instance is initialized
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_cr_is_init(p_filter_cr_t filter_inst)
{
    if ( NULL != filter_inst )
    {
        return filter_inst->is_init;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle CR filter
*
* @note This function must be called in equidistant time period defined by 1/fs!
*
* @param[in]    filter_inst - CR filter instance
* @param[in]    in          - Input value
* @return       Output (filtered) value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_cr_hndl(p_filter_cr_t filter_inst, const float32_t in)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init )) return 0;

    for ( uint32_t n = 0U; n < filter_inst->order; n++)
    {
        if ( 0U == n )
        {
            filter_inst->p_y[0] = (( filter_inst->alpha * filter_inst->p_y[0] ) + ( filter_inst->alpha * ( in - filter_inst->p_x[0] )));
            filter_inst->p_x[0] = in;
        }
        else
        {
            filter_inst->p_y[n] = (( filter_inst->alpha * filter_inst->p_y[n] ) + ( filter_inst->alpha * ( filter_inst->p_y[n-1] - filter_inst->p_x[n] )));
            filter_inst->p_x[n] = filter_inst->p_y[n-1];
        }
    }

    return filter_inst->p_y[ filter_inst->order - 1U ];
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset CR filter buffers
*
* @param[in]    filter_inst - CR filter instance
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_reset(p_filter_cr_t filter_inst)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    // Initial value
    for ( uint32_t i = 0U; i < filter_inst->order; i++ )
    {
        filter_inst->p_y[i] = 0.0f;
        filter_inst->p_x[i] = 0.0f;
    }

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set cutoff frequency of CR filter on-the-fly
*
* @param[in]    filter_inst - CR filter instance
* @param[in]    fc          - Cutoff frequency
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_set_fc(p_filter_cr_t filter_inst, const float32_t fc)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    if ( fc != filter_cr_get_fc( filter_inst ))
    {
        float32_t alpha;

        if ( eFILTER_OK == filter_cr_calculate_alpha( fc, filter_inst->fs, &alpha ))
        {
            filter_inst->alpha = alpha;
            filter_inst->fc = fc;
            return eFILTER_OK;
        }
    }

    return eFILTER_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get CR filter cutoff frequency
*
* @param[in]    filter_inst - CR filter instance
* @return       Filter cutoff frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_cr_get_fc(p_filter_cr_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return 0;
    }
    else
    {
        return filter_inst->fc;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get CR filter sampling frequency
*
* @param[in]    filter_inst - CR filter instance
* @return       Filter sampling frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_cr_get_fs(p_filter_cr_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return 0;
    }
    else
    {
        return filter_inst->fs;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize boolean/debounce filter
*
* @brief    Boolean filter is basically LPF (RC filter) + comparator
*           at the end of signal path.
*
*           Input to filter in bool and output of filer is bool. Signal
*           in between is being converted to float either 0.0f or 1.0f. That
*           signal then goes to LPF. Output of LPF goes to schmitt trigger
*           comparator with configurable trip levels at init phase
*
*           Input "comp_lvl" setup comparator trip level symmetrical to 0.5
*           value. E.g.: comp_lvl = 0.1 will result in levels:
*
*               OFF -> ON:  level = 0.9
*               ON  -> OFF: level = 0.1
*
* @param[in]    filter_inst - Pointer to bool filter instance
* @param[in]    fc          - Cuttoff frequency of LPF
* @param[in]    fs          - Sample time of filter
* @param[in]    comp_lvl    - Comparator trip level
* @return       status      - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_init(p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl)
{
    if ( NULL == p_filter_inst ) return eFILTER_ERROR_INST;

    // Check if that instance is already allocated
    // By meaning that this buffer instance was initialised before...
    // Check args
    if (( NULL != *p_filter_inst ) || ( comp_lvl < 0.0f ) || ( comp_lvl > 0.4f )) return eFILTER_ERROR_INIT;

    // Allocate space
    *p_filter_inst = calloc( 1U, sizeof(filter_bool_t));

    // Check allocation
    if ( NULL == *p_filter_inst )
    {
        return eFILTER_ERROR_MEM;
    }

    // Init LPF
    (*p_filter_inst)->lpf.p_y = &((*p_filter_inst)->lpf_mem);
    if ( eFILTER_OK == filter_rc_init_static( &(*p_filter_inst)->lpf, fc, fs, 1U, 0.0f ))
    {
        (*p_filter_inst)->comp_lvl  = comp_lvl;
        (*p_filter_inst)->y = false;

        // Init succeed
        (*p_filter_inst)->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize statically boolean/debounce filter
*
* @brief    Boolean filter is basically LPF (RC filter) + comparator
*           at the end of signal path.
*
*           Input to filter in bool and output of filer is bool. Signal
*           in between is being converted to float either 0.0f or 1.0f. That
*           signal then goes to LPF. Output of LPF goes to schmitt trigger
*           comparator with configurable trip levels at init phase
*
*           Input "comp_lvl" setup comparator trip level symmetrical to 0.5
*           value. E.g.: comp_lvl = 0.1 will result in levels:
*
*               OFF -> ON:  level = 0.9
*               ON  -> OFF: level = 0.1
*
* @param[in]    filter_inst - Bool filter instance
* @param[in]    fc          - Cuttoff frequency of LPF
* @param[in]    fs          - Sample time of filter
* @param[in]    comp_lvl    - Comparator trip level
* @return       status      - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_init_static(p_filter_bool_t filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl)
{
    if ( NULL == filter_inst ) return eFILTER_ERROR_INST;

    // Check args
    if (( comp_lvl < 0.0f ) || ( comp_lvl > 0.4f )) return eFILTER_ERROR_INIT;

    // Assign LPF memory
    filter_inst->lpf.p_y = &filter_inst->lpf_mem;

    // Init LPF
    if ( eFILTER_OK == filter_rc_init_static( &filter_inst->lpf, fc, fs, 1U, 0.0f ))
    {
        filter_inst->comp_lvl  = comp_lvl;
        filter_inst->y = false;

        // Init succeed
        filter_inst->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get initialization status of boolean filter
*
* @param[in]    filter_inst - Boolean filter instance
* @return       true if buffer instance is initialized
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_bool_is_init(p_filter_bool_t filter_inst)
{
    if ( NULL != filter_inst )
    {
        return filter_inst->is_init;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle boolean filter
*
* @note This function must be called in equidistant time period defined by 1/fs!
*
* @param[in]    filter_inst - Boolean filter instance
* @param[in]    in          - Input value
* @return       Output (filtered) value
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_bool_hndl(p_filter_bool_t filter_inst, const bool in)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init )) return 0;

    // Apply filter
    const float32_t filt_out = filter_rc_hndl( &filter_inst->lpf, (float32_t) in );

    // Apply comparator
    if (( false == filter_inst->y ) && ( filt_out >= ( 1.0f - filter_inst->comp_lvl )))
    {
        filter_inst->y  = true;
    }
    else if (( true == filter_inst->y ) &&  ( filt_out <= filter_inst->comp_lvl ))
    {
        filter_inst->y  = false;
    }
    else
    {
        // No actions...
    }

    // Return output
    return filter_inst->y;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset Boolean filter buffers
*
* @param[in]    filter_inst - Boolean filter instance
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_reset(p_filter_bool_t filter_inst)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    filter_inst->y = false;
    return filter_rc_reset( &filter_inst->lpf, 0.0f );
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set cutoff frequency of Boolean filter on-the-fly
*
* @param[in]    filter_inst - Boolean filter instance
* @param[in]    fc          - Cutoff frequency
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_set_fc(p_filter_bool_t filter_inst, const float32_t fc)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    return filter_rc_set_fc( &filter_inst->lpf, fc );
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get Boolean filter cutoff frequency
*
* @param[in]    filter_inst - Boolean filter instance
* @return       Filter cutoff frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_bool_get_fc(p_filter_bool_t filter_inst)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    return filter_rc_get_fc( &filter_inst->lpf );
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get Boolean filter sampling frequency
*
* @param[in]    filter_inst - RC filter instance
* @return       Filter sampling frequency in Hz
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_bool_get_fs(p_filter_bool_t filter_inst)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    return filter_rc_get_fs( &filter_inst->lpf );
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize FIR filter
*
* @note     Filter order cannot be changed later!
*
* @param[in]    p_filter_inst   - Pointer to FIR filter instance
* @param[in]    p_a             - FIR coefficients
* @param[in]    order           - Number of taps
* @return       status          - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_fir_init(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order, const float32_t init_value)
{
    if ( NULL == p_filter_inst ) return eFILTER_ERROR_INST;

    // Check if that instance is already allocated
    // By meaning that this buffer instance was initialised before...
    if ( NULL != *p_filter_inst ) return eFILTER_ERROR_INIT;

    // Check args
    if (( NULL == p_a ) || ( order == 0 ))
    {
        return eFILTER_ERROR_INIT;
    }

    // Allocate filter space
    *p_filter_inst = calloc( 1U, sizeof( filter_fir_t ));

    // Check allocation
    if ( NULL == *p_filter_inst )
    {
        return eFILTER_ERROR_MEM;
    }

    // Allocate filter coefficient memory
    (*p_filter_inst)->p_a = calloc( 1U, order * sizeof(float32_t));

    // Check allocation
    if ( NULL == (*p_filter_inst)->p_a )
    {
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_MEM;
    }

    // Setup sample buffer
    ring_buffer_attr_t buf_attr =
    {
        .name       = NULL,
        .override   = true,
        .item_size  = sizeof( float32_t )
    };

    // Allocate buffer memory
    buf_attr.p_mem = calloc( 1U, ( order * sizeof(float32_t)));

    // Create ring buffer for sample memory handling
    if ( eRING_BUFFER_OK == ring_buffer_init_static( &(*p_filter_inst)->buf_x, order, &buf_attr ))
    {
        // Get filter coefficient & order
        memcpy( (*p_filter_inst)->p_a, p_a, order * sizeof( float32_t ));
        (*p_filter_inst)->order = order;

        // Fill buffer with initial value
        filter_buf_fill( &(*p_filter_inst)->buf_x, init_value );

        // Init success
        (*p_filter_inst)->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        free((*p_filter_inst)->p_a);
        free(buf_attr.p_mem);
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Initialize FIR filter instance using statically allocated memory.
 *
 * @param[in,out] filter_inst  Pointer to FIR filter instance structure.
 * @param[in]     p_a          Pointer to coefficient array of length @p order.
 *                             The caller must ensure this array remains valid
 *                             and unchanged for the lifetime of the filter instance.
 * @param[in]     order        Filter order (number of taps).
 * @param[in]     p_mem        Pointer to caller-provided memory buffer for state.
 *                             Must be preallocated with exactly
 *                             @p order * sizeof(float32_t) bytes.
 * @param[in]     reset_value  Initial value to prefill the state buffer.
 *
 * @return eFILTER_OK on success, error code otherwise.
 *
 * @note
 * - No dynamic memory is allocated inside this function.
 * - The caller is responsible for keeping @p p_a and @p p_mem alive for the
 *   lifetime of the filter instance.
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_fir_init_static(p_filter_fir_t filter_inst, const float32_t * p_mem, const float32_t * p_a, const uint32_t order, const float32_t init_value)
{
    if ( NULL == filter_inst ) return eFILTER_ERROR_INST;

    // Check args
    if (( NULL == p_mem ) || ( NULL == p_a ) || ( order == 0 ))
    {
        return eFILTER_ERROR_INIT;
    }

    // Store filter properties
    filter_inst->p_a = (float32_t*) p_a;
    filter_inst->order = order;

    // Setup sample buffer
    const ring_buffer_attr_t buf_attr =
    {
        .name       = NULL,
        .p_mem      = (float32_t*) p_mem,
        .override   = true,
        .item_size  = sizeof( float32_t )
    };

    // Create ring buffer for sample memory handling
    if ( eRING_BUFFER_OK == ring_buffer_init_static( &filter_inst->buf_x, order, &buf_attr ))
    {
        // Fill buffer with initial value
        filter_buf_fill( &filter_inst->buf_x, init_value );

        // Init success
        filter_inst->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get initialization status of FIR filter
*
* @param[in]    filter_inst - FIR filter instance
* @return       true if buffer instance is initialized
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_fir_is_init(p_filter_fir_t filter_inst)
{
    if ( NULL != filter_inst )
    {
        return filter_inst->is_init;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Handle FIR filter
*
*   General FIR differential equation:
*
*       y[n] = SUM( a[i] * x[n-i] ),
*
*       where:
*           a - FIR coefficients
*           x - Input (un-filtered) signal
*           y - Output (filtered) signal
*
* @note Above described equation is basically convolution operation.
*
* @note This function must be called in equidistant time period defined by 1/fs,
*       when calculating FIR filter coefficients (a)!
*
* @param[in]    filter_inst - FIR filter instance
* @param[in]    in          - Input value
* @return       Output (filtered) value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_fir_hndl(p_filter_fir_t filter_inst, const float32_t in)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init )) return 0;

    // Add new sample to buffer
    ring_buffer_add( &filter_inst->buf_x, (float32_t*) &in );

    // Make convolution
    float32_t output = 0.0f;
    for ( uint32_t i = 0U; i < filter_inst->order; i++ )
    {
        // Get buffer value
        float32_t buf_val;
        ring_buffer_get_by_index( &filter_inst->buf_x, (float32_t*) &buf_val,  (int32_t)(( -i ) - 1U ));

        // Calculate convolution
        output += ( filter_inst->p_a[i] * buf_val );
    }

    return output;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset FIR filter buffers
*
* @param[in]    filter_inst - FIR filter instance
* @param[in]    rst_value   - Reset value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_fir_reset(p_filter_fir_t filter_inst, const float32_t rst_value)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    // First reset buffer
    (void) ring_buffer_reset( &filter_inst->buf_x );

    // Fill buffer with reset value
    filter_buf_fill( &filter_inst->buf_x, rst_value );

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set coefficient of FIR filter on-the-fly
*
* @note     It is recommended to reset filter afterwards!
*
* @note     Make sure to provide filter order size of coefficients!
*
* @param[in]    filter_inst - FIR filter instance
* @param[in]    p_a         - New FIR filter coefficients
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_fir_set_coeff(p_filter_fir_t filter_inst, const float32_t * const p_a)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;
    if ( NULL == p_a )                      return eFILTER_ERROR;

    // Get filter coefficients
    memcpy( filter_inst->p_a, p_a, ( filter_inst->order * sizeof( float32_t )));

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get FIR filter cutoff frequency
*
* @param[in]    filter_inst - FIR filter instance
* @return       FIR coefficients
*/
////////////////////////////////////////////////////////////////////////////////
const float32_t * filter_fir_get_coeff(p_filter_fir_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return NULL;
    }
    else
    {
        return filter_inst->p_a;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize IIR filter
*
*   General IIR filter difference equation:
*
*       y[n] = 1/a[0] * ( SUM( b[i] * x[n-i]) - ( SUM( a[i+1] * y[n-i-1] )))
*
*
*   General IIR impulse response in time discrete space:
*
*       H(z) = ( b0 + b1*z^-1 + b2*z^-2 + ... bn*z^(-n-1) ) / ( -a0 - a1*z^-1 - a2*z^-2 - ... - an*z^(-n-1)),
*
*           where:     a - filter poles,
*                   b - filter zeros
*
* @note Make sure that a[0] is non-zero value as it can later result in division by zero error!
*
* @note     Number of zeros and poles cannot be change later!
*
*
* @param[in]    p_filter_inst   - Pointer to IIR filter instance
* @param[in]    p_coeff         - IIR filter coefficients
* @return       status          - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_init(p_filter_iir_t * p_filter_inst, const filter_iir_coeff_t * const p_coeff)
{
    if ( NULL == p_filter_inst ) return eFILTER_ERROR_INST;

    // Check if that instance is already allocated
    // By meaning that this buffer instance was initialised before...
    if ( NULL != *p_filter_inst ) return eFILTER_ERROR_INIT;

    // Check args
    if (( NULL == p_coeff ) || ( 0 == p_coeff->num_of_pole ) || ( 0 == p_coeff->num_of_zero ) || ( NULL == p_coeff->p_pole ) || ( NULL == p_coeff->p_zero ))
    {
        return eFILTER_ERROR_INIT;
    }

    // Allocate filter space
    *p_filter_inst = calloc( 1U, sizeof(filter_iir_t));

    // Check allocation
    if ( NULL == *p_filter_inst )
    {
        return eFILTER_ERROR_MEM;
    }

    // Allocate space for filter coefficients
    (*p_filter_inst)->coeff.p_pole = calloc( 1U, p_coeff->num_of_pole * sizeof(float32_t));
    (*p_filter_inst)->coeff.p_zero = calloc( 1U, p_coeff->num_of_zero * sizeof(float32_t));

    // Check allocation
    if (( NULL == (*p_filter_inst)->coeff.p_pole  ) || ( NULL == (*p_filter_inst)->coeff.p_zero  ))
    {
        free((*p_filter_inst)->coeff.p_pole);
        free((*p_filter_inst)->coeff.p_zero);
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_MEM;
    }

    // Setup sample buffer
    ring_buffer_status_t buf_status = eRING_BUFFER_OK;
    ring_buffer_attr_t buf_attr =
    {
        .name       = NULL,
        .override   = true,
        .item_size  = sizeof( float32_t )
    };

    // Allocate and create buffer for inputs
    float32_t *in_mem = NULL, *out_mem = NULL;
    in_mem  = calloc(1U, p_coeff->num_of_zero * sizeof(float32_t));
    buf_attr.p_mem = in_mem;
    buf_status  = ring_buffer_init_static( &(*p_filter_inst)->buf_x, p_coeff->num_of_zero, &buf_attr );

    // Allocate and create buffer for outputs
    out_mem = calloc(1U, p_coeff->num_of_pole * sizeof(float32_t));
    buf_attr.p_mem = out_mem;
    buf_status |= ring_buffer_init_static( &(*p_filter_inst)->buf_y, p_coeff->num_of_pole, &buf_attr );

    // Check if ring buffer created
    // and filter coefficient memory allocation succeed
    if ( eRING_BUFFER_OK == buf_status )
    {
        // Get filter coefficient & order
        memcpy( (*p_filter_inst)->coeff.p_pole, p_coeff->p_pole, p_coeff->num_of_pole * sizeof( float32_t ));
        memcpy( (*p_filter_inst)->coeff.p_zero, p_coeff->p_zero, p_coeff->num_of_zero * sizeof( float32_t ));
        (*p_filter_inst)->coeff.num_of_pole = p_coeff->num_of_pole;
        (*p_filter_inst)->coeff.num_of_zero = p_coeff->num_of_zero;

        // Fill buffers with zero
        filter_buf_fill( &(*p_filter_inst)->buf_x, 0.0f );
        filter_buf_fill( &(*p_filter_inst)->buf_y, 0.0f );

        // Init success
        (*p_filter_inst)->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        free(in_mem);
        free(out_mem);
        free((*p_filter_inst)->coeff.p_pole);
        free((*p_filter_inst)->coeff.p_zero);
        free(*p_filter_inst);
        *p_filter_inst = NULL;
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Initialize IIR filter instance using statically allocated memory.
 *
 * @param[in,out] filter_inst  Pointer to IIR filter instance structure.
 * @param[in]     p_coeff      Pointer to coefficient descriptor (poles & zeros).
 *                             The caller must ensure this structure and its
 *                             coefficient arrays remain valid and unchanged
 *                             for the lifetime of the filter instance.
 * @param[in]     p_mem        Pointer to caller-provided memory buffer for state.
 *                             Must be preallocated with exactly
 *                             (num_of_zero + num_of_pole) * sizeof(float32_t) bytes.
 *                             The first @p num_of_zero elements are used for
 *                             the input history, the next @p num_of_pole elements
 *                             are used for the output history.
 * @param[in]     reset_value  Initial value to prefill both history buffers.
 *
 * @return eFILTER_OK on success, error code otherwise.
 *
 * @note
 * - No dynamic memory is allocated inside this function.
 * - The caller is responsible for keeping @p p_coeff and @p p_mem alive for the
 *   lifetime of the filter instance.
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_init_static(p_filter_iir_t filter_inst, const float32_t * p_mem, const filter_iir_coeff_t * const p_coeff)
{
    if ( NULL == filter_inst ) return eFILTER_ERROR_INST;

    // Check args
    if (( NULL == p_mem ) || ( NULL == p_coeff ) || ( 0 == p_coeff->num_of_pole ) || ( 0 == p_coeff->num_of_zero ) || ( NULL == p_coeff->p_pole ) || ( NULL == p_coeff->p_zero ))
    {
        return eFILTER_ERROR_INIT;
    }

    // Store filter properties
    filter_inst->coeff.p_pole       = p_coeff->p_pole;
    filter_inst->coeff.num_of_pole  = p_coeff->num_of_pole;
    filter_inst->coeff.p_zero       = p_coeff->p_zero;
    filter_inst->coeff.num_of_zero  = p_coeff->num_of_zero;

    // Setup sample buffer
    ring_buffer_status_t buf_status = eRING_BUFFER_OK;
    ring_buffer_attr_t buf_attr =
    {
        .name       = NULL,
        .p_mem      = (float32_t*) p_mem,
        .override   = true,
        .item_size  = sizeof( float32_t )
    };

    // Create buffer for inputs
    buf_status  = ring_buffer_init_static( &filter_inst->buf_x, p_coeff->num_of_zero, &buf_attr );

    // Assign memory offset and create buffer for outputs
    buf_attr.p_mem = (float32_t*) &p_mem[p_coeff->num_of_zero];
    buf_status |= ring_buffer_init_static( &filter_inst->buf_y, p_coeff->num_of_pole, &buf_attr );

    // Check if ring buffer created
    // and filter coefficient memory allocation succeed
    if ( eRING_BUFFER_OK == buf_status )
    {
        // Fill buffers with zero
        filter_buf_fill( &filter_inst->buf_x, 0.0f );
        filter_buf_fill( &filter_inst->buf_y, 0.0f );

        // Init success
        filter_inst->is_init = true;
        return eFILTER_OK;
    }
    else
    {
        return eFILTER_ERROR_INIT;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Get initialization status of IIR filter
*
* @param[in]    filter_inst - IIR filter instance
* @return       true if buffer instance is initialized
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_iir_is_init(p_filter_iir_t filter_inst)
{
    if ( NULL != filter_inst )
    {
        return filter_inst->is_init;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle IIR filter
*
* @note     This function must be called in equidistant time period defined by 1/fs,
*           when zeros and poles are calculated!
*
* @note     In case that a[0] is zero, NAN is returned!
*
* @param[in]    filter_inst - IIR filter instance
* @param[in]    in          - Input value
* @return       Output (filtered) value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_hndl(p_filter_iir_t filter_inst, const float32_t in)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init )) return 0;

    // Add new input to buffer
    ring_buffer_add( &filter_inst->buf_x, (float32_t*) &in );

    // Calculate filter value
    float32_t output = 0.0f;
    float32_t buf_val = 0.0f;
    for ( uint32_t i = 0; i < filter_inst->coeff.num_of_zero; i++ )
    {
        // Get sample
        ring_buffer_get_by_index( &filter_inst->buf_x, (float32_t*) &buf_val, (int32_t)(( -i ) - 1 ));

        // Sum zeros
        output += ( filter_inst->coeff.p_zero[i] * buf_val );
    }

    for ( uint32_t i = 1; i < filter_inst->coeff.num_of_pole; i++ )
    {
        // Get sample
        ring_buffer_get_by_index( &filter_inst->buf_y, (float32_t*) &buf_val, (int32_t)-i );

        // Subtract sum of poles
        output -= ( filter_inst->coeff.p_pole[i] * buf_val );
    }

    // Check division by
    if ( filter_inst->coeff.p_pole[0] == 0.0f )
    {
        output = NAN;
    }
    else
    {
        output = ( output / filter_inst->coeff.p_pole[0] );
    }

    // Add new output to buffer
    (void) ring_buffer_add( &filter_inst->buf_y, (float32_t*) &output );

    return output;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Reset IIR filter buffers
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    rst_value   - Reset value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_reset(p_filter_iir_t filter_inst, const float32_t rst_val)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;

    // Reset buffers
    ring_buffer_reset( &filter_inst->buf_x );
    ring_buffer_reset( &filter_inst->buf_y );

    // Fill buffers with zero
    filter_buf_fill( &filter_inst->buf_x, rst_val );
    filter_buf_fill( &filter_inst->buf_y, rst_val );

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Set coefficient of IIR filter on-the-fly
*
* @note     It is recommended to reset filter afterwards!
*
* @note     Make sure to provide filter order size of coefficients!
*
* @param[in]    filter_inst - FIR filter instance
* @param[in]    p_coeff     - New IIR filter coefficients
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_set_coeff(p_filter_iir_t filter_inst, const filter_iir_coeff_t * const p_coeff)
{
    if ( NULL == filter_inst )              return eFILTER_ERROR_INST;
    if ( false == filter_inst->is_init )    return eFILTER_ERROR_INIT;
    if ( NULL == p_coeff )                  return eFILTER_ERROR;

    memcpy( filter_inst->coeff.p_pole, p_coeff->p_pole, ( filter_inst->coeff.num_of_pole * sizeof(float32_t)));
    memcpy( filter_inst->coeff.p_zero, p_coeff->p_zero, ( filter_inst->coeff.num_of_zero * sizeof(float32_t)));

    return eFILTER_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Get IIR filter coefficients
*
* @note This functions copy coefficients into place pointing by p_zero
*       and p_pole parameter
*
* @param[in]    filter_inst - Pointer to FIR filter instance
* @return       Filter coefficients
*/
////////////////////////////////////////////////////////////////////////////////
const filter_iir_coeff_t * filter_iir_get_coeff(p_filter_iir_t filter_inst)
{
    if (( NULL == filter_inst ) || ( false == filter_inst->is_init ))
    {
        return NULL;
    }
    else
    {
        return &filter_inst->coeff;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Calculate IIR 2nd order low pass filter coefficients
*
* @note      Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
* @note      Additional check is made if sampling theorem is fulfilled.
*
* @param[in]    fc      - Cutoff frequency
* @param[in]    zeta    - Damping factor
* @param[in]    fs      - Sampling frequency
* @param[out]   p_pole  - Pointer to newly calculated IIR poles
* @param[out]   p_zero  - Pointer to newly calculated IIR zeros
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_coeff_calc_2nd_lpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
    filter_status_t status      = eFILTER_OK;
    float32_t       omega       = 0.0f;
    float32_t       cos_omega   = 0.0f;
    float32_t       alpha       = 0.0f;

    if  (   ( NULL != p_pole )
        &&  ( NULL != p_zero ))
    {
        // Check Nyquist/Shannon sampling theorem
        if (( fc < ( fs / 2.0f )) && ( fc > 0.0f ) && ( fs > 0.0f ))
        {
            omega = ( 2.0f * ( (float32_t)M_PI * ( fc / fs )));
            alpha = ( sinf( omega ) * zeta );
            cos_omega = cosf( omega );

            // Calculate zeros & poles
            p_zero[0] = (( 1.0f - cos_omega ) / 2.0f );
            p_zero[1] = ( 1.0f - cos_omega );
            p_zero[2] = (( 1.0f - cos_omega ) / 2.0f );
            p_pole[0] = ( 1.0f + alpha );
            p_pole[1] = ( -2.0f * cos_omega );
            p_pole[2] = ( 1.0f - alpha );
        }
        else
        {
            status = eFILTER_ERROR;
        }
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Calculate IIR 2nd order high pass filter coefficients
*
* @note     Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
* @note     Additional check is made if sampling theorem is fulfilled.
*
* @param[in]    fc      - Cutoff frequency
* @param[in]    zeta    - Damping factor
* @param[in]    fs      - Sampling frequency
* @param[out]   p_pole  - Pointer to newly calculated IIR poles
* @param[out]   p_zero  - Pointer to newly calculated IIR zeros
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_coeff_calc_2nd_hpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
    filter_status_t status      = eFILTER_OK;
    float32_t       omega       = 0.0f;
    float32_t       cos_omega   = 0.0f;
    float32_t       alpha       = 0.0f;

    if  (   ( NULL != p_pole )
        &&  ( NULL != p_zero ))
    {
        // Check Nyquist/Shannon sampling theorem
        if (( fc < ( fs / 2.0f )) && ( fc > 0.0f ) && ( fs > 0.0f ))
        {
            omega = ( 2.0f * ( (float32_t)M_PI * ( fc / fs )));
            alpha = ( sinf( omega ) * zeta );
            cos_omega = cosf( omega );

            // Calculate zeros & poles
            p_zero[0] = (( 1.0f + cos_omega ) / 2.0f );
            p_zero[1] = -( 1.0f + cos_omega );
            p_zero[2] = (( 1.0f + cos_omega ) / 2.0f );
            p_pole[0] = ( 1.0f + alpha );
            p_pole[1] = ( -2.0f * cos_omega );
            p_pole[2] = ( 1.0f - alpha );
        }
        else
        {
            status = eFILTER_ERROR;
        }
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Calculate IIR 2nd order notch (band stop) filter coefficients
*
* @note     Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
* @note     Additional check is made if sampling theorem is fulfilled.
*
* @note     Value of r must be within 0.0 and 1.0, typically it is around
*            0.80 - 0.99.
*
* @param[in]    fc      - Cutoff frequency
* @param[in]    r       - Bandwidth of filter
* @param[in]    fs      - Sampling frequency
* @param[out]   p_pole  - Pointer to newly calculated IIR poles
* @param[out]   p_zero  - Pointer to newly calculated IIR zeros
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_coeff_calc_2nd_bpf(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
    filter_status_t status      = eFILTER_OK;
    float32_t       omega       = 0.0f;
    float32_t       cos_omega   = 0.0f;

    if  (   ( NULL != p_pole )
        &&  ( NULL != p_zero )
        &&  (( r > 0.0f ) && ( r < 1.0f )))
    {
        // Check Nyquist/Shannon sampling theorem
        if (( fc < ( fs / 2.0f )) && ( fc > 0.0f ) && ( fs > 0.0f ))
        {
            omega = ( 2.0f * ( (float32_t)M_PI * ( fc / fs )));
            cos_omega = cosf( omega );

            // Calculate zeros & poles
            p_zero[0] = 1.0f;
            p_zero[1] = ( -2.0f  * cos_omega );
            p_zero[2] = 1.0f;
            p_pole[0] = 1.0f;
            p_pole[1] = ( -2.0f * ( r * cos_omega ));
            p_pole[2] = ( r * r );
        }
        else
        {
            status = eFILTER_ERROR;
        }
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Calculate gain at DC frequency of LPF IIR filter based on it's poles & zeros
*
* @note     Equations taken from book:
*
*        "The Scientist and Engineer's Guide to Digital Signal Processing",
*
*        G = 1/a0 * (( b0 + b1 + ... + bn ) / ( 1 + (( a1 + a2 + ... + an+1 ) / a0 ))),
*
*        where:  a - poles
*                b - zeros
*
* @param[in]    p_coeff - IIR filter coefficients
* @return       dc_gain - Gain of filter at zero (DC) frequency
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_calc_lpf_gain(const filter_iir_coeff_t * const p_coeff)
{
    float32_t dc_gain     = NAN;
    float32_t pole_sum     = 0.0f;
    float32_t zero_sum     = 0.0f;

    if ( NULL != p_coeff )
    {
        // Sum poles & zeros
        for( uint32_t i = 1; i < p_coeff->num_of_pole; i++ )
        {
            pole_sum += p_coeff->p_pole[i];
        }

        for( uint32_t i = 0; i < p_coeff->num_of_zero; i++ )
        {
            zero_sum += p_coeff->p_zero[i];
        }

        // Calculate gain at 0 frequency
        if ( p_coeff->p_pole[0] != 0.0f )
        {
            pole_sum = (( pole_sum / p_coeff->p_pole[0] ) + 1.0f );

            if ( pole_sum != 0.0f )
            {
                dc_gain = (( zero_sum / pole_sum ) / p_coeff->p_pole[0] );
            }
        }
    }

    return dc_gain;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Calculate gain @0.5 normalize frequency (w=fc/fs) of HPF IIR
*   filter based on it's poles & zeros
*
* @note Normalize frequency of 0.5 is the highest cutoff frequency for HPF
*         filter that don't break Nyquist/Shannon sampling theorem.
*
* @note Equations taken from book:
*
*        "The Scientist and Engineer's Guide to Digital Signal Processing"
*
*
*        G = 1/a0 * (( b0 - b1 + b2 - b3 +... + bn ) / ( 1 + (( a1 - a2 + a3 - a4 + ... + an+1 ) / a0 )))
*
*        where:  a - poles
*                b - zeros
*
* @param[in]     p_pole     - Pointer to IIR poles
* @param[in]     p_zero     - Pointer to IIR zeros
* @param[in]     pole_size  - Number of poles
* @param[in]     zero_size  - Number of zeros
* @return        dc_gain    - Gain of filter at 0.5 normalized frequency (Nyquist freq)
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_calc_hpf_gain(const filter_iir_coeff_t * const p_coeff)
{
    float32_t dc_gain   = NAN;
    float32_t pole_sum  = 0.0f;
    float32_t zero_sum  = 0.0f;

    if ( NULL != p_coeff)
    {
        // Sum poles
        for( uint32_t i = 1; i < p_coeff->num_of_pole; i++ )
        {
            if ( i & 0x01U )
            {
                pole_sum -= p_coeff->p_pole[i];
            }
            else
            {
                pole_sum += p_coeff->p_pole[i];
            }
        }

        // Sum zeros
        for( uint32_t i = 0; i < p_coeff->num_of_zero; i++ )
        {
            if ( i & 0x01U )
            {
                zero_sum -= p_coeff->p_zero[i];
            }
            else
            {
                zero_sum += p_coeff->p_zero[i];
            }
        }

        // Calculate gain at Nyquist frequency
        if ( p_coeff->p_pole[0] != 0.0f )
        {
            pole_sum = (( pole_sum / p_coeff->p_pole[0] ) + 1.0f );

            if ( pole_sum != 0.0f )
            {
                dc_gain = (( zero_sum / pole_sum ) / p_coeff->p_pole[0] );
            }
        }
    }

    return dc_gain;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Normalize zeros of IIR filter in order to get unity gain at DC frequency
*
* @note     Implementation taken from book:
*
*           "The Scientist and Engineer's Guide to Digital Signal Processing"
*
*       If requirement is to have a gain of 1 at DC frequency then simply
*       call this function across already calculated coefficients. This newly
*       calculated coefficients will result in unity gain filter.
*
* @note     This techniques simply calculated DC gain (G) and then divide all
*           zeros of IIR filter with it. Thus only zeros are affected by
*           this function math.
*
* @param[in]    p_coeff - IIR filter coefficients
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_coeff_to_unity_gain_lpf(filter_iir_coeff_t * const p_coeff)
{
    filter_status_t status = eFILTER_OK;

    if ( NULL != p_coeff )
    {
        // Calculate DC gain
        const float32_t dc_gain = filter_iir_calc_lpf_gain( p_coeff );

        // Check gain
        if ( dc_gain > 0.0f )
        {
            // Normalize zeros
            for ( uint32_t i = 0; i < p_coeff->num_of_zero; i++ )
            {
                p_coeff->p_zero[i] = ( p_coeff->p_zero[i] / dc_gain );
            }
        }
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Normalize zeros of IIR filter in order to get unity gain at
*   0.5 normalized frequency
*
* @note Implementation taken from book:
*
*        "The Scientist and Engineer's Guide to Digital Signal Processing"
*
*    If requirement is to have a gain of 1 at DC frequency then simply
*    call this function across already calculated coefficients. This newly
*    calculated coefficients will result in unity gain filter.
*
* @note     This techniques simply calculated DC gain (G) and then divide all
*           zeros of IIR filter with it. Thus only zeros are affected by
*           this function math.
*
* @param[in]    p_coeff - IIR filter coefficients
* @return       status  - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_coeff_to_unity_gain_hpf(filter_iir_coeff_t * const p_coeff)
{
    filter_status_t status = eFILTER_OK;

    if ( NULL != p_coeff )
    {
        // Calculate DC gain
        const float32_t dc_gain = filter_iir_calc_hpf_gain( p_coeff );

        // Check gain
        if ( dc_gain > 0.0f )
        {
            // Normalize zeros
            for ( uint32_t i = 0; i < p_coeff->num_of_zero; i++ )
            {
                p_coeff->p_zero[i] = ( p_coeff->p_zero[i] / dc_gain );
            }
        }
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
