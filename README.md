# **Filter**
This repository contains digital filters implementation written in C for general embedded application. All types of filters are written to be use as individual instances with zero level of coupling in between them.

Filter memory space is dynamically allocated and success of allocation is taken into consideration before using that instance. Deallocation on exsisting filter instance is not supported as it's not good practice to free memory in C world.

All of the filters has been evaluated in python and tested on STM32 platform. For more details
go look at this [repository](https://github.com/ZiGaMi/filters).

I found usefull "***The Scientist and Engineer's Guide to Digital Signal Processing***" book by Steven W. Smitch. Following implementation of filters are inspired by that book. 

## **Dependencies**

### **1. Ring Buffer**
Filter module needs [Ring Buffer](https://github.com/Misc-library-for-DSP/ring_buffer) C module in order to store samples and processing. Ring Buffer module is part of *General Embedded C Libraries Ecosystem*.

It is mandatory to be under following path in order to be compatible "General Embedded C Libraries Ecosystem":
```
root/middleware/ring_buffer/src/ring_buffer.h
```

## **General Embedded C Libraries Ecosystem**
In order to be part of *General Embedded C Libraries Ecosystem* this module must be placed in following path: 
```
root/middleware/filter/"module_space"
```

## **List of supported filters**
 - RC filter (IIR 1st order LPF)
 - CR filter (IIR 1st order HPF)
 - FIR
 - IIR
 - Boolean (RC + comparator): This is made up filter in order to debounce digital signals

## **RC (Low-Pass) Filter API**

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **filter_rc_init**            | Initialization of RC filter | filter_status_t filter_rc_init(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value) |
| **filter_rc_update**          | Update (handle) RC filter  | float32_t filter_rc_update(p_filter_rc_t filter_inst, const float32_t x) |
| **filter_rc_change_cutoff**   | Change RC filter cutoff frequency  | filter_status_t filter_rc_change_cutoff(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs) |
| **filter_rc_get_cutoff**      | Get RC filter cutoff frequency  | float32_t filter_rc_get_cutoff(p_filter_rc_t filter_inst) |
| **filter_rc_is_init**         | Get RC filter initialization state  | bool filter_rc_is_init(p_filter_rc_t filter_inst) |

## **CR (High-Pass) Filter API**

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **filter_cr_init** | Initialization of CR filter | filter_status_t filter_cr_init(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order) |
| **filter_cr_update** | Update (handle) CR filter  | float32_t filter_cr_update(p_filter_cr_t filter_inst, const float32_t x) |
| **filter_cr_change_cutoff** | Change CR filter cutoff frequency  | filter_status_t filter_cr_change_cutoff(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs) |
| **filter_cr_get_cutoff** | Get CR filter cutoff frequency  | float32_t filter_cr_get_cutoff(p_filter_cr_t filter_inst) |
| **filter_cr_is_init** | Get CR filter initialization state  | bool filter_cr_is_init(p_filter_cr_t filter_inst) |

## **FIR (Finite Impulse Response) Filter API**

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **filter_fir_init** | Initialization of FIR filter | filter_status_t filter_fir_init(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order) |
| **filter_fir_update** | Update (handle) FIR filter | float32_t filter_fir_update(p_filter_fir_t filter_inst, const float32_t x) |
| **filter_fir_get_coeff** | Get FIR filter coefficient | filter_status_t filter_fir_get_coeff(p_filter_fir_t filter_inst, float32_t * const p_a) |
| **filter_fir_is_init** | Get FIR filter initialization state | bool filter_fir_is_init(p_filter_fir_t filter_inst) |

## **IIR (Infinite Impulse Response) Filter API**

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **filter_iir_init** | Initialization of IIR filter | filter_status_t filter_iir_init(p_filter_iir_t * p_filter_inst, const float32_t * p_pole, const float32_t * p_zero, const uint32_t pole_size, const uint32_t zero_size) |
| **filter_iir_update** | Update (handle) IIR filter | float32_t filter_iir_update(p_filter_iir_t filter_inst, const float32_t x) |
| **filter_iir_change_coeff** | Change IIR filter coefficients | filter_status_t filter_iir_change_coeff(p_filter_iir_t filter_inst, const float32_t * const p_pole, const float32_t * const p_zero)|
| **filter_iir_calc_coeff_2nd_lpf** | Calculate 2nd order low-pass IIR filter coefficients | filter_status_t filter_iir_calc_coeff_2nd_lpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)|
| **filter_iir_calc_coeff_2nd_hpf** | Calculate 2nd order high-pass IIR filter coefficients | filter_status_t filter_iir_calc_coeff_2nd_hpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)|
| **filter_iir_calc_coeff_2nd_notch** | Calculate 2nd order band-pass (notch) IIR filter coefficients | filter_status_t filter_iir_calc_coeff_2nd_notch(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)|
| **filter_iir_calc_lpf_gain** | Calculate low-pass filter gain at DC frequency | float32_t	filter_iir_calc_lpf_gain(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)|
| **filter_iir_calc_hpf_gain** | Calculate high-pass filter gain at 0.5 normalized frequency (fc/fs) | float32_t filter_iir_calc_hpf_gain(const float32_t * const p_pole, const float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)|
| **filter_iir_lpf_norm_zeros_to_unity_gain** | Normalize low-pass filter coefficient to achieve unity gain at DC frequency | filter_status_t	filter_iir_lpf_norm_zeros_to_unity_gain	(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)|
| **filter_iir_hpf_norm_zeros_to_unity_gain** | Normalize high-pass filter coefficient to achieve unity gain at 0.5 normalized frequency | filter_status_t filter_iir_hpf_norm_zeros_to_unity_gain(const float32_t * const p_pole, float32_t * const p_zero, const uint32_t pole_size, const uint32_t zero_size)|
| **filter_iir_is_init** | Get IIR filter initialization state | bool filter_iir_is_init(p_filter_iir_t filter_inst)|
| **filter_iir_get_coeff** | Get IIR filter coefficients | filter_status_t filter_iir_get_coeff(p_filter_iir_t filter_inst, float32_t * const p_pole, float32_t * const p_zero)|

## **Boolean Filter API**

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **filter_bool_init** | Initialization of boolean filter | filter_status_t	filter_bool_init(p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl) |
| **filter_bool_is_init** | Get boolean filter initialization state | filter_status_t filter_bool_is_init(p_filter_bool_t filter_inst, bool * const p_is_init) |
| **filter_bool_update** | Update (handle) boolean filter | filter_status_t filter_bool_update(p_filter_bool_t filter_inst, const bool in, bool * const p_out) |
| **filter_bool_get_fc** | Get boolean filter cutoff frequency | filter_status_t filter_bool_get_fc(p_filter_bool_t filter_inst, float32_t * const p_fc) |
| **filter_bool_change_cutoff** | Change boolean filter cutoff frequency | filter_status_t filter_bool_change_cutoff(p_filter_bool_t filter_inst, const float32_t fc, const float32_t fs) |

 ## **RC/CR filters**
 RC/CR filter C implementation support also cascading filter but user shall notice that cascading two RC or CR filters does not have same characteristics as IIR 2nd order filter. To define 2nd order IIR filter beside cutoff frequency (fc) also damping factors ($\zeta$) must be defined.

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

 ## **IIR filters**
 For IIR 2nd order filters there are functions to calcualte zeros and poles for high-pass, low-pass and notch filter type. These functions becomes very usefully if filter coefficients needs to be change during runtime of a application. 

```C
// 1. Declare filter instance
p_filter_iir_t gp_filter_iir = NULL;

/* 2. Prepare IIR coefficients
* NOTE: Use iir_filter.py sript or external tool to get IIR coefficients.
*/
const float32_t gf_iir_poles_coef[3] =
{
	1.0f,
	-1.04377111f,
	0.27236453f
};

const float32_t gf_iir_zeros_coef[3] =
{
	0.05714836f,
	0.11429671f,
	0.05714836f
};

/* 
*   3. Init IIR filter with following parameters
*/ 
if ( eFILTER_OK != filter_iir_init( &gp_filter_iir, &gf_iir_poles_coef, &gf_iir_zeros_coef, 3, 3 ))
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

## **FIR filters**
FIR filter coefficients can be calculated on T-Filter webpage ([link](http://t-filter.engineerjs.com/)).

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