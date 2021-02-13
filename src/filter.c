////////////////////////////////////////////////////////////////////////////////
/**
*@file      filter.c
*@brief     Various filter designs
*@author    Ziga Miklosic
*@date      02.01.2021
*@version   V0.0.1
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

#include "middleware/ring_buffer/src/ring_buffer.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	RC Filter data
 */
typedef struct filter_rc_s
{
	float32_t *	p_y;		/**<Output of filter + previous values */
	float32_t	alpha;		/**<Filter smoothing factor */
	uint8_t		order;		/**<Filter order - number of cascaded filter */
} filter_rc_t;

/**
 * 	CR Filter data
 */
typedef struct filter_cr_s
{
	float32_t * p_y;		/**<Output of filter + previous values */
	float32_t * p_x;		/**<Input of filter + previous values */
	float32_t  	alpha;		/**<Filter smoothing factor */
	uint8_t 	order;		/**<Filter order - number of cascaded filter */
} filter_cr_t;

/**
 * 	FIR Filter data
 */
typedef struct filter_fir_s
{
	p_ring_buffer_t   p_x;		/**<Previous values of input filter */
	float32_t 		* p_a;		/**<Filter coefficients */
	uint32_t		  order;	/**<Number of FIR filter taps - order of filter */
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
} filter_iir_t;


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static float32_t filter_rc_calculate_alpha(const float32_t fc, const float32_t fs);
static float32_t filter_cr_calculate_alpha(const float32_t fc, const float32_t fs);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*   Initialize RC filter
*
*@Note: Order of RC filter is represented as number of cascaded RC
*		analog equivalent circuits!
*
* @param[in] 	p_filter_inst	- Pointer to RC filter instance
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
	uint8_t i;

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
			//(*p_filter_inst)->alpha = (float32_t) ( dt / ( dt + ( 1.0f / ( M_TWOPI * fc ))));
			(*p_filter_inst)->alpha = filter_rc_calculate_alpha( fc, fs );

			// Store order
			(*p_filter_inst)->order = order;

			// Initial value
			for ( i = 0; i < order; i++)
			{
				(*p_filter_inst)->p_y[i] = init_value;
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
*   Update RC filter
*
* @param[in] 	filter_inst	- Pointer to RC filter instance
* @param[in] 	x			- Input value
* @return 		y			- Output value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_rc_update(p_filter_rc_t filter_inst, const float32_t x)
{
	float32_t y = 0.0f;
	uint8_t n;

	if ( NULL != filter_inst )
	{
		for ( n = 0; n < filter_inst->order; n++)
		{
			if ( 0 == n )
			{
				filter_inst->p_y[0] = ( filter_inst->p_y[0] + ( filter_inst->alpha * ( x - filter_inst->p_y[0] )));
			}
			else
			{
				filter_inst->p_y[n] = ( filter_inst->p_y[n] + ( filter_inst->alpha * ( filter_inst->p_y[n-1] - filter_inst->p_y[n] )));
			}
		}

		y = filter_inst->p_y[ filter_inst->order - 1U ];
	}

	return y;
}


static float32_t filter_rc_calculate_alpha(const float32_t fc, const float32_t fs)
{
	float32_t alpha = 0.0f;

	// Check Nyquist/Shannon sampling theorem
	if ( fc < ( fs / 2.0f ))
	{
		alpha = (float32_t) ( 1.0f / ( 1.0f + ( fs / ( M_TWOPI * fc ))));
	}

	return alpha;
}

filter_status_t filter_rc_change_cutoff	(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs)
{
	filter_status_t status 	= eFILTER_OK;
	float32_t 		alpha	= 0.0f;

	if ( NULL != filter_inst )
	{
		// Calculate new alpha
		alpha = filter_rc_calculate_alpha( fc, fs );

		if ( alpha > 0.0f )
		{
			filter_inst->alpha = alpha;
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
*@Note: Order of CR filter is represented as number of cascaded CR
*		analog equivalent circuits!
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
	uint8_t i;

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
			//(*p_filter_inst)->alpha = (float32_t) (( 1.0f / ( M_TWOPI * fc )) / ( dt + ( 1.0f / ( M_TWOPI * fc ))));
			(*p_filter_inst)->alpha = filter_cr_calculate_alpha( fc, fs );

			// Store order
			(*p_filter_inst)->order = order;

			// Initial value
			for ( i = 0; i < order; i++)
			{
				(*p_filter_inst)->p_y[i] = 0.0f;
				(*p_filter_inst)->p_x[i] = 0.0f;
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
*   Update CR filter
*
* @param[in] 	filter_inst	- Pointer to CR filter instance
* @param[in] 	x			- Input value
* @return 		y			- Output value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_cr_update(p_filter_cr_t filter_inst, const float32_t x)
{
	float32_t y = 0.0f;
	uint8_t n;

	if ( NULL != filter_inst )
	{
		for ( n = 0; n < filter_inst->order; n++)
		{
			if ( 0 == n )
			{
				filter_inst->p_y[0] = (( filter_inst->alpha * filter_inst->p_y[0] ) + ( filter_inst->alpha * ( x - filter_inst->p_x[0] )));
				filter_inst->p_x[0] = x;
			}
			else
			{
				filter_inst->p_y[n] = (( filter_inst->alpha * filter_inst->p_y[n] ) + ( filter_inst->alpha * ( filter_inst->p_y[n-1] - filter_inst->p_x[n] )));
				filter_inst->p_x[n] = filter_inst->p_y[n-1];
			}
		}

		y = filter_inst->p_y[ filter_inst->order - 1U ];
	}

	return y;
}


static float32_t filter_cr_calculate_alpha(const float32_t fc, const float32_t fs)
{
	float32_t alpha = 0.0f;

	// Check Nyquist/Shannon sampling theorem
	if (( fc < ( fs / 2.0f ))
		&& ( fs > 0.0f )
		&& ( fc > 0.0f ))
	{
		alpha = (float32_t) (( 1.0f / ( M_TWOPI * fc )) / (( 1.0f / fs ) + ( 1.0f / ( M_TWOPI * fc ))));
	}

	return alpha;
}

filter_status_t filter_cr_change_cutoff(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs)
{
	filter_status_t status 	= eFILTER_OK;
	float32_t		alpha 	= 0.0f;

	if ( NULL != filter_inst )
	{
		// Calculate new alpha
		alpha = filter_cr_calculate_alpha( fc, fs );

		if ( alpha > 0.0f )
		{
			filter_inst->alpha = alpha;
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
			buf_status = ring_buffer_init( &(*p_filter_inst)->p_x, order );

			// Ring buffer created
			// and filter coefficient memory allocation succeed
			if 	(	( eRING_BUFFER_OK == buf_status )
				&& 	( NULL != (*p_filter_inst)->p_a ))
			{
				// Get filter coefficient & order
				memcpy( (*p_filter_inst)->p_a, p_a, order * sizeof( float32_t ));
				(*p_filter_inst)->order = order;
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
* @param[in] 	filter_inst	- Pointer to FIR filter instance
* @param[in] 	x			- Input value
* @return 		y			- Output value
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_fir_update(p_filter_fir_t filter_inst, const float32_t x)
{
	float32_t y = 0.0f;
	uint32_t i;

	if ( NULL != filter_inst )
	{
		// Add new sample to buffer
		ring_buffer_add_f( filter_inst->p_x, x );

		// Make convolution
		for ( i = 0; i < filter_inst -> order; i++ )
		{
			y += ( filter_inst->p_a[i] * ring_buffer_get_f( filter_inst->p_x, (( -i ) - 1 )));
		}
	}

	return y;
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
* @param[in] 	a_size			- Number of poles
* @param[in] 	b_size			- Number of zeros
* @return 		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t filter_iir_init(p_filter_iir_t * p_filter_inst, const float32_t * p_pole, const float32_t * p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	filter_status_t 		status 		= eFILTER_OK;
	ring_buffer_status_t 	buf_status 	= eRING_BUFFER_OK;

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
			buf_status = ring_buffer_init( &(*p_filter_inst)->p_x, zero_size );
			buf_status |= ring_buffer_init( &(*p_filter_inst)->p_y, pole_size );

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
	float32_t y = 0.0f;
	uint32_t i;

	if ( NULL != filter_inst )
	{
		// Add new input to buffer
		ring_buffer_add_f( filter_inst->p_x, x );

		// Calculate filter value
		for ( i = 0; i < filter_inst->zero_size; i++ )
		{
			y += ( filter_inst->p_zero[i] * ring_buffer_get_f( filter_inst->p_x, (( -i ) - 1 )));
		}

		for ( i = 1; i < filter_inst->pole_size; i++ )
		{
			y -= ( filter_inst->p_pole[i] * ring_buffer_get_f( filter_inst->p_y, -i ));
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
		ring_buffer_add_f( filter_inst->p_y, y );
	}

	return y;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Change IIR filter coefficient on the fly
*
*@note: Coefficient size must stay the same, only value is permited to be change!
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
		memcpy( filter_inst->p_pole, p_pole, filter_inst->pole_size * sizeof(float32_t));
		memcpy( filter_inst->p_zero, p_zero, filter_inst->zero_size * sizeof(float32_t));
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
*   Calculate DC gain of IIR filter based on it's poles & zeros - NEED TO BE TESTED!!!
*
*@note: Equations taken from book:
*
*		"The Scientist and Engineer's Guide to Digital Signal Processing"
*
*
*	G = a0 + a1 + ... + an / ( 1 - ( b1 + b2 + ... + bn+1 ))
*
* @param[in] 	p_pole		- Pointer to IIR poles
* @param[in] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		dc_gain		- Gain of filter at zero (DC) frequency
*/
////////////////////////////////////////////////////////////////////////////////
float32_t filter_iir_calc_dc_gain(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
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

		// Calculate dc gain
		pole_sum = ( 1.0f - pole_sum );

		if ( pole_sum != 0.0f )
		{
			dc_gain = ( zero_sum / pole_sum );
		}
	}

	return dc_gain;
}

////////////////////////////////////////////////////////////////////////////////
/**
*   Normalize zeros of IIR filter in order to get unity gain at DC frequency. - NEED TO BE TESTED!!!
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
* @param[out] 	p_zero		- Pointer to IIR zeros
* @param[in] 	pole_size	- Number of poles
* @param[in] 	zero_size	- Number of zeros
* @return 		dc_gain		- Gain of filter at zero (DC) frequency
*/
////////////////////////////////////////////////////////////////////////////////
filter_status_t	filter_iir_norm_to_unity_gain(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
{
	filter_status_t status 		= eFILTER_OK;
	float32_t		dc_gain		= 0.0f;
	uint32_t		i;

	if 	(	( NULL != p_pole )
		&&	( NULL != p_zero ))
	{
		// Calculate DC gain
		dc_gain = filter_iir_calc_dc_gain( p_pole, p_zero, pole_size, zero_size );

		// Check if gain is really above 1
		if ( dc_gain > 1.0f )
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
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
