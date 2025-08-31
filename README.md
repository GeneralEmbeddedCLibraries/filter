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

### **2. Utils Module**
[Utils](https://github.com/GeneralEmbeddedCLibraries/utils) module must take following path:
```
"root/common/utils/src/utils.h"
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

| API Function               | Description                          | Prototype |
|--------------------------- |--------------------------------------|-----------|
| **filter_rc_init**         | Initialization of RC filter          | `filter_status_t filter_rc_init(p_filter_rc_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)` |
| **filter_rc_init_static**  | Static initialization of RC filter   | `filter_status_t filter_rc_init_static(p_filter_rc_t filter_inst, const float32_t fc, const float32_t fs, const uint8_t order, const float32_t init_value)` |
| **filter_rc_is_init**      | Get RC initialization state          | `bool filter_rc_is_init(p_filter_rc_t filter_inst)` |
| **filter_rc_hndl**         | Process one sample through RC        | `float32_t filter_rc_hndl(p_filter_rc_t filter_inst, const float32_t in)` |
| **filter_rc_reset**        | Reset RC filter state                | `filter_status_t filter_rc_reset(p_filter_rc_t filter_inst, const float32_t rst_value)` |
| **filter_rc_set_fc**       | Set RC cutoff frequency              | `filter_status_t filter_rc_set_fc(p_filter_rc_t filter_inst, const float32_t fc)` |
| **filter_rc_get_fc**       | Get RC cutoff frequency              | `float32_t filter_rc_get_fc(p_filter_rc_t filter_inst)` |
| **filter_rc_get_fs**       | Get RC sample rate                   | `float32_t filter_rc_get_fs(p_filter_rc_t filter_inst)` |

## **CR (High-Pass) Filter API**

| API Function               | Description                          | Prototype |
|--------------------------- |--------------------------------------|-----------|
| **filter_cr_init**         | Initialization of CR filter          | `filter_status_t filter_cr_init(p_filter_cr_t * p_filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)` |
| **filter_cr_init_static**  | Static initialization of CR filter   | `filter_status_t filter_cr_init_static(p_filter_cr_t filter_inst, const float32_t fc, const float32_t fs, const uint8_t order)` |
| **filter_cr_is_init**      | Get CR initialization state          | `bool filter_cr_is_init(p_filter_cr_t filter_inst)` |
| **filter_cr_hndl**         | Process one sample through CR        | `float32_t filter_cr_hndl(p_filter_cr_t filter_inst, const float32_t in)` |
| **filter_cr_reset**        | Reset CR filter state                | `filter_status_t filter_cr_reset(p_filter_cr_t filter_inst)` |
| **filter_cr_set_fc**       | Set CR cutoff frequency              | `filter_status_t filter_cr_set_fc(p_filter_cr_t filter_inst, const float32_t fc)` |
| **filter_cr_get_fc**       | Get CR cutoff frequency              | `float32_t filter_cr_get_fc(p_filter_cr_t filter_inst)` |
| **filter_cr_get_fs**       | Get CR sample rate                   | `float32_t filter_cr_get_fs(p_filter_cr_t filter_inst)` |

## **Boolean (Debounce) LPF Filter API**

| API Function                  | Description                               | Prototype |
|------------------------------ |-------------------------------------------|-----------|
| **filter_bool_init**          | Initialization of Boolean LPF + Schmitt   | `filter_status_t filter_bool_init(p_filter_bool_t * p_filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl)` |
| **filter_bool_init_static**   | Static initialization of Boolean filter   | `filter_status_t filter_bool_init_static(p_filter_bool_t filter_inst, const float32_t fc, const float32_t fs, const float32_t comp_lvl)` |
| **filter_bool_is_init**       | Get Boolean filter initialization state   | `bool filter_bool_is_init(p_filter_bool_t filter_inst)` |
| **filter_bool_hndl**          | Process one sample; returns debounced bit | `bool filter_bool_hndl(p_filter_bool_t filter_inst, const bool in)` |
| **filter_bool_reset**         | Reset Boolean filter state                | `filter_status_t filter_bool_reset(p_filter_bool_t filter_inst)` |
| **filter_bool_set_fc**        | Set Boolean LPF cutoff frequency          | `filter_status_t filter_bool_set_fc(p_filter_bool_t filter_inst, const float32_t fc)` |
| **filter_bool_get_fc**        | Get Boolean LPF cutoff frequency          | `float32_t filter_bool_get_fc(p_filter_bool_t filter_inst)` |
| **filter_bool_get_fs**        | Get Boolean LPF sample rate               | `float32_t filter_bool_get_fs(p_filter_bool_t filter_inst)` |

## **FIR (Finite Impulse Response) Filter API**

| API Function               | Description                          | Prototype |
|--------------------------- |--------------------------------------|-----------|
| **filter_fir_init**        | Initialization of FIR filter         | `filter_status_t filter_fir_init(p_filter_fir_t * p_filter_inst, const float32_t * p_a, const uint32_t order, const float32_t init_value)` |
| **filter_fir_init_static** | Static initialization of FIR filter  | `filter_status_t filter_fir_init_static(p_filter_fir_t filter_inst, const float32_t * p_mem, const float32_t * p_a, const uint32_t order, const float32_t init_value)` |
| **filter_fir_is_init**     | Get FIR initialization state         | `bool filter_fir_is_init(p_filter_fir_t filter_inst)` |
| **filter_fir_hndl**        | Process one sample through FIR       | `float32_t filter_fir_hndl(p_filter_fir_t filter_inst, const float32_t in)` |
| **filter_fir_reset**       | Reset FIR filter state               | `filter_status_t filter_fir_reset(p_filter_fir_t filter_inst, const float32_t rst_val)` |
| **filter_fir_set_coeff**   | Set FIR coefficients                 | `filter_status_t filter_fir_set_coeff(p_filter_fir_t filter_inst, const float32_t * const p_a)` |
| **filter_fir_get_coeff**   | Get FIR coefficients                 | `const float32_t * filter_fir_get_coeff(p_filter_fir_t filter_inst)` |

## **IIR (Infinite Impulse Response) Filter API**

| API Function                 | Description                          | Prototype |
|----------------------------- |--------------------------------------|-----------|
| **filter_iir_init**          | Initialization of IIR filter         | `filter_status_t filter_iir_init(p_filter_iir_t * p_filter_inst, const filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_init_static**   | Static initialization of IIR filter  | `filter_status_t filter_iir_init_static(p_filter_iir_t filter_inst, const float32_t * p_mem, const filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_is_init**       | Get IIR initialization state         | `bool filter_iir_is_init(p_filter_iir_t filter_inst)` |
| **filter_iir_hndl**          | Process one sample through IIR       | `float32_t filter_iir_hndl(p_filter_iir_t filter_inst, const float32_t in)` |
| **filter_iir_reset**         | Reset IIR filter state               | `filter_status_t filter_iir_reset(p_filter_iir_t filter_inst, const float32_t rst_val)` |
| **filter_iir_set_coeff**     | Set IIR zeros & poles                | `filter_status_t filter_iir_set_coeff(p_filter_iir_t filter_inst, const filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_get_coeff**     | Get IIR zeros & poles                | `const filter_iir_coeff_t * filter_iir_get_coeff(p_filter_iir_t filter_inst)` |

## **IIR Helper Functions**

| Helper Function                          | Description                        | Prototype |
|------------------------------------------|------------------------------------|-----------|
| **filter_iir_coeff_calc_2nd_lpf**        | 2nd-order low-pass (cookbook)      | `filter_status_t filter_iir_coeff_calc_2nd_lpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)` |
| **filter_iir_coeff_calc_2nd_hpf**        | 2nd-order high-pass (cookbook)     | `filter_status_t filter_iir_coeff_calc_2nd_hpf(const float32_t fc, const float32_t zeta, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)` |
| **filter_iir_coeff_calc_2nd_bpf**        | 2nd-order notch (band-stop)        | `filter_status_t filter_iir_coeff_calc_2nd_bpf(const float32_t fc, const float32_t r, const float32_t fs, float32_t * const p_pole, float32_t * const p_zero)` |
| **filter_iir_calc_lpf_gain**             | DC gain for LPF section            | `float32_t filter_iir_calc_lpf_gain(const filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_calc_hpf_gain**             | High-frequency gain for HPF        | `float32_t filter_iir_calc_hpf_gain(const filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_coeff_to_unity_gain_lpf**   | Normalize LPF to unity DC gain     | `filter_status_t filter_iir_coeff_to_unity_gain_lpf(filter_iir_coeff_t * const p_coeff)` |
| **filter_iir_coeff_to_unity_gain_hpf**   | Normalize HPF to unity HF gain     | `filter_status_t filter_iir_coeff_to_unity_gain_hpf(filter_iir_coeff_t * const p_coeff)` |

 ## **RC/CR filters**
 RC/CR filter C implementation support also cascading filter but user shall notice that cascading two RC or CR filters does not have same characteristics as IIR 2nd order filter. To define 2nd order IIR filter beside cutoff frequency (fc) also damping factors ($\zeta$) must be defined.

```C
// ===================================================
// DYNAMIC FILTER MEMORY ALLOCATION
// ===================================================

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
    (void) filter_rc_hndl( my_filter_inst, raw_signal, &filtered_signal );
}


// ===================================================
// STATIC FILTER MEMORY ALLOCATION
// ===================================================

// 1. Declare filter instance and memory statically
filter_rc_t my_filter_inst = {0};
float32_t filter_mem[ORDER];

// 2. Assign filter instance allocated memory
my_filter_inst.p_y = &filter_mem;

/* 
*   3. Init RC filter with following parameters:
*   - fc = 10Hz
*   - fs = 100Hz
*   - order = ORDER
*   - inititial value = 0
*/ 
if ( eFILTER_OK != filter_rc_init_static( &my_filter_instance, 10.0f, 100.0f, ORDER, 0 ))
{
    // Filter init failed
    // Further actions here...
}

// 4. Apply filter in period of SAMPLE_TIME
loop @SAMPLE_TIME
{
    // Update filter
    (void) filter_rc_hndl( &my_filter_inst, raw_signal, &filtered_signal );
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
    (void) filter_iir_hndl( gp_filter_iir, raw_signal, &filtered_signal );
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
{   
    0.02341152899192398f,
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
    (void) filter_fir_hndl( gp_filter_fir, raw_signal, &filtered_signal );
}

```