# Filter
Digital filters written in C for general embedded application. All types of filters are written to be use as individual instances with no connections between them at all.

Filter memory space is dynamically allocated and success of allocation is taken into consideration before using that instance. Deallocation on exsisting filter instance is not supported as it's not good practice to free memory in C world.

All of the filters has been evaluated in python and tested on STM32 platform. For more details
go look at this [repository](https://github.com/ZiGaMi/filters).

I found usefull "***The Scientist and Engineer's Guide to Digital Signal Processing***" by Steven W. Smitch book where digital filters implementations and desing techniques are in depth described. 

#### Dependencies
Filter module needs ring buffer in order to store data. Ring buffer sources can be found under this [link](https://github.com/Misc-library-for-DSP/ring_buffer). 

Definition of flaot32_t must be provided by user. In current implementation it is defined in "*project_config.h*". Just add following statement to your code where it suits the best.

```C
// Define float
typedef float float32_t;
```

#### List of all filters
 - RC filter (IIR 1st order LPF)
 - CR filter (IIR 1st order HPF)
 - FIR
 - IIR

 #### API

 - filter_status_t **filter_rc_init**(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)
- float32_t	**filter_rc_update**(p_filter_rc_t filter_inst, const float32_t x)
- filter_status_t **filter_rc_change_cutoff**(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs)

 - filter_status_t **filter_cr_init**(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)
 - float32_t **filter_cr_update**(p_filter_cr_t filter_inst, const float32_t x)
 - filter_status_t **filter_cr_change_cutoff**(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs)

 - filter_status_t **filter_fir_init**(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order)
 - float32_t **filter_fir_update**(p_filter_fir_t filter_inst, const float32_t x)

 - filter_status_t **filter_iir_init**(p_filter_iir_t * p_filter_inst, const float32_t * p_pole, const float32_t * p_zero, const uint32_t pole_size, const uint32_t zero_size);
 - float32_t **filter_iir_update**(p_filter_iir_t filter_inst, const float32_t x)
 - filter_status_t **filter_iir_change_coeff**(p_filter_iir_t filter_inst, const float32_t * const p_pole, const float32_t * const p_zero)
 - filter_status_t **filter_iir_calc_coeff_2nd_lpf**(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
 - filter_status_t **filter_iir_calc_coeff_2nd_hpf**(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
 - filter_status_t filter_iir_calc_coeff_2nd_notch	(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)
 - float32_t **filter_iir_calc_dc_gain**(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)
 - filter_status_t **filter_iir_norm_to_unity_gain**(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)

 NOTE: Detailed usage of filters be found in doxygen!

 #### RC/CR filters
 RC/CR filter C implementation support also cascading filter but user shall notice that cascading two RC or CR filters does not have same characteristics as IIR 2nd order filter. To define 2nd order IIR filter beside cutoff frequency (fc) also damping factors ($\zeta$) must be defined.

  ##### Example of usage

```C
// 1. Declare filter instance
p_filter_rc_t my_filter_inst = NULL;

/* 
*   2. Init RC filter with following parameters:
*   - fc = 10Hz
*   - fs = 100Hz
*   - order = 1
*   - inititial value = 0
*/ 
if ( eFILTER_OK != filter_rc_init( &my_filter_instance, 10.0f, 100.0f, 1, 0 ))
{
    // Filter init failed
    // Further actions here...
}

// 3. Apply filter in period of SAMPLE_TIME
loop @SAMPLE_TIME
{
    // Update filter
    filtered_signal = filter_rc_update( my_filter_inst, raw_signal );
}

```

 #### IIR filters
 For IIR 2nd order filters there are functions to calcualte zeros and poles for high-pass, low-pass and notch filter type. These functions becomes very usefully if filter coefficients needs to be change during runtime of a application. 


  ##### Example of usage
```C
// 1. Declare filter instance
p_filter_iir_t gp_filter_iir = NULL;

/* 2. Prepare IIR coefficients
* NOTE: Use iir_filter.py sript or external tool to get IIR coefficients.
*/
const float32_t gf_iir_a_coef[3] =
{
	1.0f,
	-1.04377111f,
	0.27236453f
};

const float32_t gf_iir_b_coef[3] =
{
	0.05714836f,
	0.11429671f,
	0.05714836f
};

/* 
*   3. Init IIR filter with following parameters
*/ 
if ( eFILTER_OK != filter_iir_init( &gp_filter_iir, &gf_iir_a_coef, &gf_iir_b_coef, 3, 3 ))
{
    // Filter init failed
    // Further actions here...
}

// 3. Apply filter in period of SAMPLE_TIME
loop @SAMPLE_TIME
{
    // Update filter
    filtered_signal = filter_iir_update( gp_filter_iir, raw_signal );
}
```

#### FIR filters
FIR filter coefficients can be calculated on this webpage ([t-filter](http://t-filter.engineerjs.com/)).

##### Example of usage
```C
// 1. Declare filter instance
p_filter_fir_t gp_filter_fir = NULL;

/* 
 * 2. Prepare FIR coefficients
*/
const float32_t gf_fir_coef[9] =
{   0.02341152899192398f,
    0.06471122356467367f,
    0.12060371719780817f,
    0.16958710211144923f,
    0.1891554348168665f,
    0.16958710211144923f,
    0.12060371719780817f,
    0.06471122356467367f,
    0.02341152899192398f
};

/* 
*   3. Init FIR filter with following parameters
*/ 
if ( eFILTER_OK != filter_fir_init( &gp_filter_fir, &gf_fir_coeff, 9 ))
{
    // Filter init failed
    // Further actions here...
}

// 3. Apply filter in period of SAMPLE_TIME
loop @SAMPLE_TIME
{
    // Update filter
    filtered_signal = filter_fir_update( gp_filter_fir, raw_signal );
}

```