 Changelog
All notable changes to this project/module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project/module adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
## V2.0.0 - 26.10.2023

### Added
 - Option to reset filter

### Changed
 - Complete API changed
 - Unifying return statuses of API functions

---
## V1.1.0 - 03.11.2022

### Fixed
 - Added checks for invalid filter initialization
 - Removed "float32_t" dependency
 - Replace static asserts with C11 standard one (_Static_assert)

### Todo
 - Fixed-point values support

---
## V1.0.3 - 25.05.2022

### Added
 - Added boolean filter 
 - Added check for invalid RC/CR filter settings

### Todo
 - Fixed-point values support
 - Asserts for development bug catcher

---
## V1.0.2 - 31.07.2021

### Added
 - Updated due to ring buffer version increase

### Todo
 - Fixed-point values support
 - Asserts for development bug catcher

---
## V1.0.1 - 25.07.2021

### Added
 - Added copyright notice
 - Added module version
 - Added compatibility checks for needed modules

### Todo
 - Fixed-point values support
 - Asserts for development bug catcher
---

## V1.0.0 - 19.02.2022

### Added
 - Initial implementation of filters
 - Supported filters: RC, CR, FIR and IIR
 -  IIR 2nd order filter coefficient calculations (suported LPF, HPF and NOTCH)
 -  Calculation IIR filter gain
 -  Normalization of IIR filter coefficients to achieve unity gain
---