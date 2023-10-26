// Copyright (c) 2022 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      filter.c
*@brief     Various filter designs
*@author    Ziga Miklosic
*@date      02.01.2021
*@version   V1.0.3
*
*@section   Description
*   
*   This module contains different kind of digital filter
*   implementation. All of the following filter types are
*   being simulated in python for validation purposes.
*
*@section 	Dependencies
*
* 	Some implementation of filters uses ring buffers.
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

#include "middleware/ring_buffer/src/ring_buffer.h"

/**
 * 	Compatibility check with RING_BUFFER
 *
 * 	Support version V2.x.x
 */
_Static_assert( 2 == RING_BUFFER_VER_MAJOR );


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 *  Two times PI
 *
 * @note    M_PI should be defined in <math.h> lib!
 */
#define FILTER_TWOPI            ((float32_t) ( 2.0 * M_PI ))

/**
 * 	RC Filter data
 */
typedef struct filter_rc_s
{
	float32_t *	p_y;		/**<Output of filter + previous values */
	float32_t	alpha;		/**<Filter smoothing factor */
	float32_t	fc;			/**<Filter cutoff frequency */
	float32_t	fs;			/**<Filter sampling frequency */
	uint8_t		order;		/**<Filter order - number of cascaded filter */
	bool		is_init;	/**<Filter instance initialization success flag */
} filter_rc_t;

/**
 * 	CR Filter data
 */
typedef struct filter_cr_s
{
	float32_t * p_y;		/**<Output of filter + previous values */
	float32_t * p_x;		/**<Input of filter + previous values */
	float32_t  	alpha;		/**<Filter smoothing factor */
	float32_t	fc;			/**<Filter cutoff frequency */
    float32_t   fs;         /**<Filter sampling frequency */
	uint8_t 	order;		/**<Filter order - number of cascaded filter */
	bool		is_init;	/**<Filter instance initialization success flag */
} filter_cr_t;

/**
 * 	FIR Filter data
 */
typedef struct filter_fir_s
{
	p_ring_buffer_t   p_x;		/**<Previous values of input filter */
	float32_t 		* p_a;		/**<Filter coefficients */
	uint32_t		  order;	/**<Number of FIR filter taps - order of filter */
	bool			  is_init;	/**<Filter instance initialization success flag */
} filter_fir_t;

/**
 * 	IIR Filter data
 */
typedef struct filter_iir_s
{
	p_ring_buffer_t   p_y;			/**<Previous values of filter outputs */
	p_ring_buffer_t   p_x;			/**<Previous values of filter inputs*/
	float32_t 		* p_pole;		/**<Filter poles */
	float32_t		* p_zero;		/**<Filter zeros */
	uint32_t		  pole_size;	/**<Number of filter poles */
	uint32_t	 	  zero_size;	/**<Number of filter zeros */
	bool			  is_init;		/**<Filter instance initialization success flag */
} filter_iir_t;

/**
 * 	Boolean Filter data
 */
typedef struct filter_bool_s
{
	p_filter_rc_t	lpf;		/**<Low pass filter */
	float32_t		comp_lvl;	/**<Comparator trip level - symmetrical on 0.5 */
	bool			y;			/**<Output value of comparator/filter */
	bool			is_init;	/**<Filter instance initialization success flag */
} filter_bool_t;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static filter_status_t filter_rc_calculate_alpha	(const float32_t fc, const float32_t fs, float32_t * const p_alpha);
static filter_status_t filter_cr_calculate_alpha	(const float32_t fc, const float32_t fs, float32_t * const p_alpha);

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
    filter_status_t status = eFILTER_OK;

    // Check Nyquist/Shannon sampling theorem
    if  (   ( fc < ( fs / 2.0f ))
        &&  ( p_alpha != NULL ))
    {
        *p_alpha = (float32_t) ( 1.0f / ( 1.0f + ( fs / ( FILTER_TWOPI * fc ))));
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
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
    filter_status_t status  = eFILTER_OK;

    // Check Nyquist/Shannon sampling theorem
    if (( fc < ( fs / 2.0f ))
        && ( fs > 0.0f )
        && ( fc > 0.0f )
        && ( p_alpha != NULL ))
    {
        *p_alpha = (float32_t) (( 1.0f / ( FILTER_TWOPI * fc )) / (( 1.0f / fs ) + ( 1.0f / ( FILTER_TWOPI * fc ))));
    }

    return status;
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
*		analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in]    p_filter_inst   - Pointer to RC filter instance
* @param[in] 	fc				- Filter cutoff frequency
* @param[in] 	fs				- Sample frequency
* @param[in] 	order			- Order of filter (number of cascaded filter)
* @param[in] 	init_value		- Initial value
* @return 		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_init(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)
{
	filter_status_t status = eFILTER_OK;

	if (( NULL != p_filter_inst ) && ( order > 0UL ))
	{
		// Allocate space
		*p_filter_inst 			= malloc( sizeof( filter_rc_t ));
		(*p_filter_inst)->p_y 	= malloc( order  * sizeof( float32_t ));

		// Check if allocation succeed
		if 	(	( NULL != *p_filter_inst )
			&& 	( NULL != (*p_filter_inst)->p_y ))
		{
			// Calculate coefficient
			status = filter_rc_calculate_alpha( fc, fs, &(*p_filter_inst)->alpha );

			if ( eFILTER_OK == status )
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
			}
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
*       Get initialization status of RC filter
*
* @param[in]    filter_inst - RC filter instance
* @param[out]   p_is_init   - RC filter init state
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_is_init(p_filter_rc_t filter_inst, bool * const p_is_init)
{
    filter_status_t status = eFILTER_OK;

    if  (   ( NULL != filter_inst )
        &&  ( NULL != p_is_init ))
    {
        *p_is_init = filter_inst->is_init;
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle RC filter
*
* @note This function must be called in equidistant time period defined by 1/fs!
*
* @param[in]    filter_inst - RC filter instance
* @param[in] 	in			- Input value
* @param[out] 	p_out		- Output (filtered) value
* @return 		status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_hndl(p_filter_rc_t filter_inst, const float32_t in, float32_t * const p_out)
{
    filter_status_t status = eFILTER_OK;

	// Check for instance and success init
	if  (   ( NULL != filter_inst )
	    &&  ( NULL != p_out ))
	{
	    // Is instance init?
		if ( true == filter_inst->is_init )
		{
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

			*p_out = filter_inst->p_y[ filter_inst->order - 1U ];
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
*       Reset RC filter buffers
*
* @param[in]    filter_inst - RC filter instance
* @param[in]    rst_value   - Reset value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_reset(p_filter_rc_t filter_inst, const float32_t rst_value)
{
    filter_status_t status  = eFILTER_OK;

    if ( NULL != filter_inst )
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            // Initial value
            for ( uint32_t i = 0; i < filter_inst->order; i++)
            {
                filter_inst->p_y[i] = rst_value;
            }
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
*   	Set cutoff frequency of RC filter on-the-fly
*
* @param[in]    filter_inst - RC filter instance
* @param[in] 	fc			- Cutoff frequency
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_fc_set(p_filter_rc_t filter_inst, const float32_t fc)
{
	filter_status_t status 	= eFILTER_OK;
	float32_t 		alpha	= 0.0f;

	if ( NULL != filter_inst )
	{
        // Is instance init?
	    if ( true == filter_inst->is_init )
	    {
            // Calculate new alpha
            status = filter_rc_calculate_alpha( fc, filter_inst->fs, &alpha );

            // Store data for newly set cutoff
            if ( eFILTER_OK == status )
            {
                filter_inst->alpha = alpha;
                filter_inst->fc = fc;
            }
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
*   	Get filter cutoff frequency
*
* @param[in]    filter_inst - RC filter instance
* @param[out]   p_fc        - Filter cutoff frequency in Hz
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_fc_get(p_filter_rc_t filter_inst, float32_t * const p_fc)
{
    filter_status_t status = eFILTER_OK;

	if  (   ( NULL != filter_inst )
	    &&  ( NULL != p_fc ))
	{
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            *p_fc = filter_inst->fc;
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
*       Get filter sampling frequency
*
* @param[in]    filter_inst - RC filter instance
* @param[out]   p_fs        - Filter sampling frequency in Hz
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_rc_fs_get(p_filter_rc_t filter_inst, float32_t * const p_fs)
{
    filter_status_t status = eFILTER_OK;

    if  (   ( NULL != filter_inst )
        &&  ( NULL != p_fs ))
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            *p_fs = filter_inst->fs;
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
*   Initialize CR filter
*
* @note	Order of CR filter is represented as number of cascaded CR
*		analog equivalent circuits!
*
* @note Fs and order cannot be change later!
*
* @param[in] 	p_filter_inst	- Pointer to CR filter instance
* @param[in] 	fc				- Filter cutoff frequency
* @param[in] 	fs				- Sample frequency
* @param[in] 	order			- Order of filter (number of cascaded filter)
* @return 		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_init(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)
{
	filter_status_t status = eFILTER_OK;

	if (( NULL != p_filter_inst ) && ( order > 0UL ))
	{
		// Allocate space
		*p_filter_inst 			= malloc( sizeof( filter_cr_t ));
		(*p_filter_inst)->p_y 	= malloc( order  * sizeof( float32_t ));
		(*p_filter_inst)->p_x 	= malloc( order  * sizeof( float32_t ));

		// Check if allocation succeed
		if 	(	( NULL != *p_filter_inst )
			&& 	( NULL != (*p_filter_inst)->p_y )
			&&	( NULL != (*p_filter_inst)->p_x ))
		{
			// Calculate coefficient
			status = filter_cr_calculate_alpha( fc, fs, &(*p_filter_inst)->alpha );

			if ( eFILTER_OK == status )
			{
				// Store order & fc
				(*p_filter_inst)->order = order;
				(*p_filter_inst)->fc = fc;

				// Initial value
				for ( uint32_t i = 0; i < order; i++)
				{
					(*p_filter_inst)->p_y[i] = 0.0f;
					(*p_filter_inst)->p_x[i] = 0.0f;
				}

				// Init success
				(*p_filter_inst)->is_init = true;
			}
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
*       Get initialization status of CR filter
*
* @param[in]    filter_inst - CR filter instance
* @param[out]   p_is_init   - CR filter init state
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_is_init(p_filter_cr_t filter_inst, bool * const p_is_init)
{
    filter_status_t status = eFILTER_OK;

    if  (   ( NULL != filter_inst )
        &&  ( NULL != p_is_init ))
    {
        *p_is_init = filter_inst->is_init;
    }
    else
    {
        status = eFILTER_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       Handle CR filter
*
* @note This function must be called in equidistant time period defined by 1/fs!
*
* @param[in]    filter_inst - CR filter instance
* @param[in]    in          - Input value
* @param[out]   p_out       - Output (filtered) value
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_hndl(p_filter_cr_t filter_inst, const float32_t in, float32_t * const p_out)
{
    filter_status_t status = eFILTER_OK;

	// Check for instance and success init
	if  (   ( NULL != filter_inst )
	    &&  ( NULL != p_out ))
	{
        // Is instance init?
		if ( true == filter_inst->is_init )
		{
			for ( uint32_t n = 0; n < filter_inst->order; n++)
			{
				if ( 0 == n )
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

			*p_out = filter_inst->p_y[ filter_inst->order - 1U ];
		}
	}

	return status;
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
    filter_status_t status  = eFILTER_OK;

    if ( NULL != filter_inst )
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            // Initial value
            for ( uint32_t i = 0; i < filter_inst->order; i++ )
            {
                filter_inst->p_y[i] = 0.0f;
                filter_inst->p_x[i] = 0.0f;
            }
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
*       Set cutoff frequency of CR filter on-the-fly
*
* @param[in]    filter_inst - CR filter instance
* @param[in]    fc          - Cutoff frequency
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_fc_set(p_filter_cr_t filter_inst, const float32_t fc)
{
    filter_status_t status  = eFILTER_OK;
    float32_t       alpha   = 0.0f;

    if ( NULL != filter_inst )
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            // Calculate new alpha
            status = filter_cr_calculate_alpha( fc, filter_inst->fs, &alpha );

            // Store data for newly set cutoff
            if ( eFILTER_OK == status )
            {
                filter_inst->alpha = alpha;
                filter_inst->fc = fc;
            }
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
*       Get filter cutoff frequency
*
* @param[in]    filter_inst - CR filter instance
* @param[out]   p_fc        - Filter cutoff frequency in Hz
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_fc_get(p_filter_cr_t filter_inst, float32_t * const p_fc)
{
    filter_status_t status = eFILTER_OK;

    if  (   ( NULL != filter_inst )
        &&  ( NULL != p_fc ))
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            *p_fc = filter_inst->fc;
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
*       Get filter sampling frequency
*
* @param[in]    filter_inst - CR filter instance
* @param[out]   p_fs        - Filter sampling frequency in Hz
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_cr_fs_get(p_filter_cr_t filter_inst, float32_t * const p_fs)
{
    filter_status_t status = eFILTER_OK;

    if  (   ( NULL != filter_inst )
        &&  ( NULL != p_fs ))
    {
        // Is instance init?
        if ( true == filter_inst->is_init )
        {
            *p_fs = filter_inst->fs;
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
*   Initialize FIR filter
*
* @param[in] 	p_filter_inst	- Pointer to FIR filter instance
* @param[in] 	p_a				- Pointer to FIR coefficients
* @param[in] 	order			- Number of taps
* @return 		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_fir_init(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order)
{
	filter_status_t 		status 		= eFILTER_OK;
	ring_buffer_status_t	buf_status	= eRING_BUFFER_OK;
	ring_buffer_attr_t		buf_attr	= 	{
												.name		= NULL,
												.p_mem 		= NULL,	// dynamic allocation
												.override	= true,
												.item_size	= sizeof( float32_t )
											};

	if 	(	( NULL != p_filter_inst )
		&& 	( order > 0UL )
		&&	( NULL != p_a ))
	{
		// Allocate filter space
		*p_filter_inst = malloc( sizeof( filter_fir_t ));

		// Allocation succeed
		if ( NULL != *p_filter_inst )
		{
			// Allocate filter coefficient memory
			(*p_filter_inst)->p_a = malloc( order * sizeof(float32_t));

			// Create ring buffer
			buf_status = ring_buffer_init( &(*p_filter_inst)->p_x, order, &buf_attr );

			// Ring buffer created
			// and filter coefficient memory allocation succeed
			if 	(	( eRING_BUFFER_OK == buf_status )
				&& 	( NULL != (*p_filter_inst)->p_a ))
			{
				// Get filter coefficient & order
				memcpy( (*p_filter_inst)->p_a, p_a, order * sizeof( float32_t ));
				(*p_filter_inst)->order = order;

				// Init success
				(*p_filter_inst)->is_init = true;
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
	}
	else
	{
		status = eFILTER_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Update FIR filter
*
*   General FIR differential equation:
*
*   	y[n] = SUM( a[i] * x[n-i] ),
*
*   	where:
*   		a - FIR coefficients
*   		x - Input (un-filtered) signal
*   		y - Output (filtered) signal
*
* @note Above described equation is basically convolution operation.
*
* @param[in] 	filter_inst	- Pointer to FIR filter instance
* @param[in] 	x			- Input value
* @return 		y			- Output value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_fir_update(p_filter_fir_t filter_inst, const float32_t x)
{
	float32_t 	y 		= 0.0f;
	uint32_t 	i 		= 0UL;
	float32_t 	buf_val = 0.0f;

	// Check for instance and success init
	if ( NULL != filter_inst )
	{
		if ( true == filter_inst->is_init )
		{
			// Add new sample to buffer
			ring_buffer_add( filter_inst->p_x, (float32_t*) &x );

			// Make convolution
			for ( i = 0; i < filter_inst -> order; i++ )
			{
				// Get buffer value
				ring_buffer_get_by_index( filter_inst->p_x, (float32_t*) &buf_val,  (( -i ) - 1 ));

				// Calculate convolution
				y += ( filter_inst->p_a[i] * buf_val );
			}
		}
	}

	return y;
}

////////////////////////////////////////////////////////////////////////////////
/**
*  	Get FIR filter coefficients
*
* @note This functions copy coefficients into place pointing by p_a
* 		parameter
*
* @param[in] 	filter_inst	- Pointer to FIR filter instance
* @param[out] 	p_a			- Pointer to read coefficient
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_fir_get_coeff(p_filter_fir_t filter_inst, float32_t * const p_a)
{
	filter_status_t status = eFILTER_OK;

	if 	(	( NULL != filter_inst )
		&&	( NULL != p_a ))
	{
		if ( true == filter_inst->is_init )
		{
			// Copy coefficients
			memcpy( p_a, filter_inst->p_a, ( filter_inst->order * sizeof(float32_t)));
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
*  		Get initialization status of filter
*
* @param[in] 	filter_inst	- Pointer to FIR filter instance
* @return 		is_init		- Initialization success flag
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_fir_is_init(p_filter_fir_t filter_inst)
{
	bool is_init = false;

	if ( NULL != filter_inst )
	{
		is_init = filter_inst->is_init;
	}

	return is_init;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize IIR filter
*
*   General IIR filter difference equation:
*
*   	y[n] = 1/a[0] * ( SUM( b[i] * x[n-i]) - ( SUM( a[i+1] * y[n-i-1] )))
*
*
*   General IIR impulse response in time discrete space:
*
*   	H(z) = ( b0 + b1*z^-1 + b2*z^-2 + ... bn*z^(-n-1) ) / ( -a0 - a1*z^-1 - a2*z^-2 - ... - an*z^(-n-1)),
*
*   		where: 	a - filter poles,
*   				b - filter zeros
*
* @note Make sure that a[0] is non-zero value as it can later result in division by zero error!
*
*
* @param[in] 	p_filter_inst	- Pointer to IIR filter instance
* @param[in] 	p_pole			- Pointer to IIR pole
* @param[in] 	p_zero			- Pointer to IIR zeros
* @param[in] 	pole_size		- Number of poles
* @param[in] 	zero_size		- Number of zeros
* @return 		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_init(p_filter_iir_t * p_filter_inst, const float32_t * p_pole, const float32_t * p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	filter_status_t 		status 		= eFILTER_OK;
	ring_buffer_status_t 	buf_status 	= eRING_BUFFER_OK;
	ring_buffer_attr_t		buf_attr	= 	{
												.name		= NULL,
												.p_mem 		= NULL,	// dynamic allocation
												.override	= true,
												.item_size	= sizeof( float32_t )
											};

	if 	(	( NULL != p_filter_inst )
		&& 	(( pole_size > 0UL ) && ( pole_size > 0UL ))
		&&	(( NULL != p_pole ) && ( NULL != p_zero )))
	{
		// Allocate filter space
		*p_filter_inst = malloc( sizeof( filter_iir_t ));

		// Allocation succeed
		if ( NULL != *p_filter_inst )
		{
			// Create ring buffers
			buf_status = ring_buffer_init( &(*p_filter_inst)->p_x, zero_size, &buf_attr );
			buf_status |= ring_buffer_init( &(*p_filter_inst)->p_y, pole_size, &buf_attr );

			// Allocate space for filter coefficients
			(*p_filter_inst)->p_pole = malloc( pole_size * sizeof( float32_t ));
			(*p_filter_inst)->p_zero = malloc( zero_size * sizeof( float32_t ));

			// Check if ring buffer created
			// and filter coefficient memory allocation succeed
			if 	(	( eRING_BUFFER_OK == buf_status )
				&&	( NULL != (*p_filter_inst)->p_pole  )
				&&	( NULL != (*p_filter_inst)->p_zero  ))
			{
				// Get filter coefficient & order
				memcpy( (*p_filter_inst)->p_pole, p_pole, pole_size * sizeof( float32_t ));
				memcpy( (*p_filter_inst)->p_zero, p_zero, zero_size * sizeof( float32_t ));
				(*p_filter_inst)->pole_size = pole_size;
				(*p_filter_inst)->zero_size = zero_size;

				// Init success
				(*p_filter_inst)->is_init = true;
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
	}
	else
	{
		status = eFILTER_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Update IIR filter
*
*@note: In case that a[0] is zero, NAN is returned!
*
* @param[in] 	filter_inst	- Pointer to IIR filter instance
* @param[in] 	x			- Input value
* @return 		y			- Output value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_update(p_filter_iir_t filter_inst, const float32_t x)
{
	float32_t 	y 		= 0.0f;
	uint32_t 	i 		= 0UL;
	float32_t	buf_val = 0.0f;

	// Check for instance and success init
	if ( NULL != filter_inst )
	{
		if ( true == filter_inst->is_init )
		{
			// Add new input to buffer
			ring_buffer_add( filter_inst->p_x, (float32_t*) &x );

			// Calculate filter value
			for ( i = 0; i < filter_inst->zero_size; i++ )
			{
				// Get sample
				ring_buffer_get_by_index( filter_inst->p_x, (float32_t*) &buf_val, (( -i ) - 1 ));

				y += ( filter_inst->p_zero[i] * buf_val );
			}

			for ( i = 1; i < filter_inst->pole_size; i++ )
			{
				// Get sample
				ring_buffer_get_by_index( filter_inst->p_y, (float32_t*) &buf_val, -i );

				y -= ( filter_inst->p_pole[i] * buf_val );
			}

			// Check division by
			if ( filter_inst->p_pole[0] == 0.0f )
			{
				y = NAN;
			}
			else
			{
				y = ( y / filter_inst->p_pole[0] );
			}

			// Add new output to buffer
			ring_buffer_add( filter_inst->p_y, (float32_t*) &y );
		}
	}

	return y;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Change IIR filter coefficient on the fly
*
*@note: Coefficient size must stay the same, only value is permitted to be
*		change during runtime!
*
* @param[in] 	filter_inst	- Pointer to IIR filter instance
* @param[in] 	p_pole		- Pointer to new IIR poles
* @param[in] 	p_zero		- Pointer to new IIR zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_change_coeff(p_filter_iir_t filter_inst, const float32_t * const p_pole, const float32_t * const p_zero)
{
	filter_status_t status = eFILTER_OK;

	if 	(	( NULL != filter_inst )
		&&	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		if ( true == filter_inst->is_init )
		{
			memcpy( filter_inst->p_pole, p_pole, filter_inst->pole_size * sizeof(float32_t));
			memcpy( filter_inst->p_zero, p_zero, filter_inst->zero_size * sizeof(float32_t));
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
*   Calculate IIR 2nd order low pass filter coefficients
*
*@note: Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
*@note: Additional check is made if sampling theorem is fulfilled.
*
* @param[in] 	fc			- Cutoff frequency
* @param[in] 	zeta		- Damping factor
* @param[in] 	fs			- Sampling frequency
* @param[out] 	p_pole		- Pointer to newly calculated IIR poles
* @param[out] 	p_zero		- Pointer to newly calculated IIR zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_calc_coeff_2nd_lpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		omega		= 0.0f;
	float32_t		cos_omega	= 0.0f;
	float32_t		alpha		= 0.0f;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Check Nyquist/Shannon sampling theorem
		if ( fc < ( fs / 2.0f ))
		{
			omega = ( 2.0f * ( M_PI * ( fc / fs )));
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
*@note: Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
*@note: Additional check is made if sampling theorem is fulfilled.
*
* @param[in] 	fc			- Cutoff frequency
* @param[in] 	zeta		- Damping factor
* @param[in] 	fs			- Sampling frequency
* @param[out] 	p_pole		- Pointer to newly calculated IIR poles
* @param[out] 	p_zero		- Pointer to newly calculated IIR zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_calc_coeff_2nd_hpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		omega		= 0.0f;
	float32_t		cos_omega	= 0.0f;
	float32_t		alpha		= 0.0f;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Check Nyquist/Shannon sampling theorem
		if ( fc < ( fs / 2.0f ))
		{
			omega = ( 2.0f * ( M_PI * ( fc / fs )));
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
*@note: Equations taken from: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
*
*@note: Additional check is made if sampling theorem is fulfilled.
*
*@note: Value of r must be within 0.0 and 1.0, typically it is around
*		0.80 - 0.99.
*
* @param[in] 	fc			- Cutoff frequency
* @param[in] 	r			- Bandwidth of filter
* @param[in] 	fs			- Sampling frequency
* @param[out] 	p_pole		- Pointer to newly calculated IIR poles
* @param[out] 	p_zero		- Pointer to newly calculated IIR zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_calc_coeff_2nd_notch(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		omega		= 0.0f;
	float32_t		cos_omega	= 0.0f;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero )
		&&	(( r > 0.0f ) && ( r < 1.0f )))
	{
		// Check Nyquist/Shannon sampling theorem
		if ( fc < ( fs / 2.0f ))
		{
			omega = ( 2.0f * ( M_PI * ( fc / fs )));
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
*@note: Equations taken from book:
*
*		"The Scientist and Engineer's Guide to Digital Signal Processing",
*
*
*		G = 1/a0 * (( b0 + b1 + ... + bn ) / ( 1 + (( a1 + a2 + ... + an+1 ) / a0 ))),
*
*		where: 	a - poles
*				b - zeros
*
* @param[in] 	p_pole		- Pointer to IIR poles
* @param[in] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		dc_gain		- Gain of filter at zero (DC) frequency
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_calc_lpf_gain(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	float32_t dc_gain 	= NAN;
	float32_t pole_sum 	= 0.0f;
	float32_t zero_sum 	= 0.0f;
	uint32_t i;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Sum poles & zeros
		for( i = 1; i < pole_size; i++ )
		{
			pole_sum += p_pole[i];
		}

		for( i = 0; i < zero_size; i++ )
		{
			zero_sum += p_zero[i];
		}

		// Calculate gain at 0 frequency
		if ( p_pole[0] != 0.0f )
		{
			pole_sum = (( pole_sum / p_pole[0] ) + 1.0f );

			if ( pole_sum != 0.0f )
			{
				dc_gain = (( zero_sum / pole_sum ) / p_pole[0] );
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
* 		filter that don't break Nyquist/Shannon sampling theorem.
*
* @note Equations taken from book:
*
*		"The Scientist and Engineer's Guide to Digital Signal Processing"
*
*
*		G = 1/a0 * (( b0 - b1 + b2 - b3 +... + bn ) / ( 1 + (( a1 - a2 + a3 - a4 + ... + an+1 ) / a0 )))
*
*		where: 	a - poles
*				b - zeros
*
* @param[in] 	p_pole		- Pointer to IIR poles
* @param[in] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		dc_gain		- Gain of filter at 0.5 normalized frequency
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_calc_hpf_gain(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	float32_t dc_gain 	= NAN;
	float32_t pole_sum 	= 0.0f;
	float32_t zero_sum 	= 0.0f;
	uint32_t i;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Sum poles & zeros
		for( i = 1; i < pole_size; i++ )
		{
			if ( i & 0x01U )
			{
				pole_sum -= p_pole[i];
			}
			else
			{
				pole_sum += p_pole[i];
			}
		}

		for( i = 0; i < zero_size; i++ )
		{
			if ( i & 0x01U )
			{
				zero_sum -= p_zero[i];
			}
			else
			{
				zero_sum += p_zero[i];
			}
		}

		// Calculate gain at Nyquist frequency
		if ( p_pole[0] != 0.0f )
		{
			pole_sum = (( pole_sum / p_pole[0] ) + 1.0f );

			if ( pole_sum != 0.0f )
			{
				dc_gain = (( zero_sum / pole_sum ) / p_pole[0] );
			}
		}
	}

	return dc_gain;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Normalize zeros of IIR filter in order to get unity gain at DC frequency
*
*@note: Implementation taken from book:
*
*		"The Scientist and Engineer's Guide to Digital Signal Processing"
*
*	If requirement is to have a gain of 1 at DC frequency then simply
*	call this function across already calculated coefficients. This newly
*	calculated coefficients will result in unity gain filter.
*
*@note: This techniques simply calculated DC gain (G) and then divide all
*		zeros of IIR filter with it. Thus only zeros are affected by
*		this function math.
*
* @param[in] 	p_pole		- Pointer to IIR poles
* @param[in] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_iir_lpf_norm_zeros_to_unity_gain(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		dc_gain		= 0.0f;
	uint32_t		i;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Calculate DC gain
		dc_gain = filter_iir_calc_lpf_gain( p_pole, p_zero, pole_size, zero_size );

		// Check gain
		if ( dc_gain > 0.0f )
		{
			// Normalize zeros
			for ( i = 0; i < zero_size; i++ )
			{
				p_zero[i] = ( p_zero[i] / dc_gain );
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
*@note: Implementation taken from book:
*
*		"The Scientist and Engineer's Guide to Digital Signal Processing"
*
*	If requirement is to have a gain of 1 at DC frequency then simply
*	call this function across already calculated coefficients. This newly
*	calculated coefficients will result in unity gain filter.
*
*@note: This techniques simply calculated DC gain (G) and then divide all
*		zeros of IIR filter with it. Thus only zeros are affected by
*		this function math.
*
* @param[in] 	p_pole		- Pointer to IIR poles
* @param[in] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_iir_hpf_norm_zeros_to_unity_gain	(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		dc_gain		= 0.0f;
	uint32_t		i;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Calculate DC gain
		dc_gain = filter_iir_calc_hpf_gain( p_pole, p_zero, pole_size, zero_size );

		// Check gain
		if ( dc_gain > 0.0f )
		{
			// Normalize zeros
			for ( i = 0; i < zero_size; i++ )
			{
				p_zero[i] = ( p_zero[i] / dc_gain );
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
*  		Get initialization status of filter
*
* @param[in] 	filter_inst	- Pointer to IIR filter instance
* @return 		is_init		- Initialization success flag
*/
////////////////////////////////////////////////////////////////////////////////
bool filter_iir_is_init(p_filter_iir_t filter_inst)
{
	bool is_init = false;

	if ( NULL != filter_inst )
	{
		is_init = filter_inst->is_init;
	}

	return is_init;
}

////////////////////////////////////////////////////////////////////////////////
/**
*  	Get IIR filter coefficients
*
* @note This functions copy coefficients into place pointing by p_zero
* 		and p_pole parameter
*
* @param[in] 	filter_inst	- Pointer to FIR filter instance
* @param[out] 	p_zero		- Pointer to read zero coefficient
* @param[out] 	p_pole		- Pointer to read pole coefficient
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_get_coeff(p_filter_iir_t filter_inst, float32_t * const p_pole, float32_t * const p_zero)
{
	filter_status_t status = eFILTER_OK;

	if 	(	( NULL != filter_inst )
		&&	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		if ( true == filter_inst->is_init )
		{
			memcpy( p_pole, filter_inst->p_pole, ( filter_inst->pole_size * sizeof(float32_t)));
			memcpy( p_zero, filter_inst->p_zero, ( filter_inst->zero_size * sizeof(float32_t)));
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
*  	Initialize boolean filter
*
* @brief	Boolean filter is basically LPF (RC filter) + comparator
* 			at the end of signal path.
*
* 			Input to filter in bool and output of filer is bool. Signal
* 			in between is being converted to float either 0.0f or 1.0f. That
* 			signal then goes to LPF. Output of LPF goes to schmitt trigger
* 			comparator with configurable trip levels at init phase
*
* 			Input "comp_lvl" setup comparator trip level symmetrical to 0.5
* 			value. E.g.: comp_lvl = 0.1 will result in levels:
*
* 				OFF -> ON:	level = 0.9
* 				ON 	-> OFF:	level = 0.1
*
* @param[in] 	filter_inst	- Pointer to bool filter instance
* @param[in] 	fc			- Cuttoff frequency of LPF
* @param[in] 	fs			- Sample time of filter
* @param[in] 	comp_lvl	- Comparator trip level
* @return 		status 		- Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_bool_init(p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl)
{
	filter_status_t status = eFILTER_OK;

	if ( NULL != p_filter_inst )
	{
		// Allocate space
		*p_filter_inst = malloc( sizeof( filter_bool_t ));

		// Check if allocation succeed & valid configs
		if 	(	( NULL != p_filter_inst )
			&&	(( comp_lvl > 0.0f ) && ( comp_lvl < 0.4f )))
		{
			// Init LPF
			status = filter_rc_init( &(*p_filter_inst)->lpf, fc, fs, 1, 0.0f );

			if ( eFILTER_OK == status )
			{
				(*p_filter_inst)->comp_lvl 	= comp_lvl;
				(*p_filter_inst)->y = false;

				// Init succeed
				(*p_filter_inst)->is_init = true;
			}
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
*  	Update boolean filter
*
* @param[in] 	filter_inst	- Filter instance
* @param[in] 	p_is_init	- Pointer to is init flag
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_bool_is_init(p_filter_bool_t filter_inst, bool * const p_is_init)
{
	filter_status_t status = eFILTER_OK;

	if 	(	( NULL != filter_inst )
		&& 	( NULL != p_is_init ))
	{
		*p_is_init = filter_inst->is_init;
	}
	else
	{
		status = eFILTER_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*  	Update boolean filter
*
* @param[in] 	filter_inst	- Filter instance
* @param[in] 	in			- Input value
* @param[out] 	p_out		- Output value
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_bool_update(p_filter_bool_t filter_inst, const bool in, bool * const p_out)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		filt_in 	= 0.0f;
	float32_t		filt_out	= 0.0f;

	if ( NULL != filter_inst )
	{
		// Convert input to floating
		filt_in = (float32_t) in;

		// Apply filter
		(void) filter_rc_hndl( filter_inst->lpf, filt_in, &filt_out );

		// Apply comparator
		if 	(	( false == filter_inst->y )
			&& 	( filt_out >= ( 1.0f - filter_inst->comp_lvl )))
		{
			filter_inst->y  = true;
		}

		else if (	( true == filter_inst->y )
				&& 	( filt_out <= filter_inst->comp_lvl ))
		{
			filter_inst->y  = false;
		}

		else
		{
			// No actions...
		}

		// Return output
		if ( NULL != p_out )
		{
			*p_out = filter_inst->y ;
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
*  	Get boolean cutoff frequency
*
* @param[in] 	filter_inst	- Filter instance
* @param[out] 	p_fc		- Pointer to cuttoff freq
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_get_fc(p_filter_bool_t filter_inst, float32_t * const p_fc)
{
	filter_status_t status = eFILTER_OK;

	if 	(	( NULL != filter_inst )
		&& 	( NULL != p_fc ))
	{
		*p_fc = filter_inst->lpf->fc;
	}
	else
	{
		status = eFILTER_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*  	Change boolean filter cuttoff freq
*
* @param[in] 	filter_inst	- Filter instance
* @param[in] 	fc			- Cuttoff frequency of LPF
* @param[in] 	fs			- Sample time of filter
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_change_cutoff(p_filter_bool_t filter_inst, const float32_t fc, const float32_t fs)
{
	filter_status_t status 	= eFILTER_OK;

	if ( NULL != filter_inst )
	{
	    // TODO:
	    (void) fs;
		status = filter_rc_fc_set( filter_inst->lpf, fc );
	}
	else
	{
		status = eFILTER_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*  	Reset boolean filter
*
* @param[in] 	filter_inst	- Filter instance
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_bool_reset(p_filter_bool_t filter_inst)
{
	filter_status_t status 	= eFILTER_OK;

	if ( NULL != filter_inst )
	{
		status = filter_rc_reset( filter_inst->lpf, 0.0f );
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
