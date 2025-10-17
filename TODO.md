# TODO — Warp Toolkit

## Overview

|Library  |Version|Status     |Description               |
|---------|-------|-----------|--------------------------|
|WarpLog  |v1     |Done       |Logging Tool              |
|WarpTimer|NA     |In-Progress|Timing & Benchmarking Tool|

## General
- [ ] **FIX:** `constepxr` warning in `warp::log::tag` factory functions
- [ ] Reformat and refactor libraries to adapt modern standards 

---

## WarpTimer
- [x] RAII timer *starts with construction & ends with destruction*
- [x] Colored terminal logging of timer
- [x] Manual start and stop of timer
- [x] Manual pause and resume of timer
- [x] Utility tools to measure a funtion automatically
- [x] Utility tools for benchmarking functions

---

## Build System
- [x] Cross-platform `makefile`
- [x] Static library build support
- [x] `warp::log` support
- [ ] `warp::timer` support

---

> *This TODO file only tracks near-term goals and ideas.  
> Will be modified and updated periodically*