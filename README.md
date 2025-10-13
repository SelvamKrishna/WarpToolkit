# Warp Toolkit

Warp Toolkit is a collection of high-performance, general-purpose utility libraries for C++.

## Overview
- **[WarpLog](#warp-log):** A lightweight, high-performance logging library
- **[WarpTimer](#warp-timer):** A lightweight, high-performance timing and benchmarking library

---

## Warp Log

Warp Log is a lightweight, high-performance logging library designed for C++ projects.
It supports colorized output, tag-based context, optional runtime timestamps, and thread-safe console logging — making it ideal for real-time applications like game engines, tools, or embedded systems.

---

### Features
- **Multiple log levels:** `INFO`, `DEBUG`, `WARN`, `ERROR`  
  Each level is color-coded for improved console readability.  
- **Thread-safe:** Console output is synchronized using mutexes to prevent interleaved logs in multi-threaded environments.  
- **High-performance:** Uses minimal allocations and cached tag strings for low overhead.  
- **Flexible usage:** Supports custom string contexts, single tags, or multiple tags for contextual logging.  
- **Runtime timestamps:** Optionally log the current time and date with each message.
- **Cross-platform:** Works on Windows, Linux, and any ANSI-compatible terminal.

---

### Usage

| Component           | Description                                                  |
|---------------------|--------------------------------------------------------------|
| `warp::log::level`  | Enum defining log levels (`INFO`, `DEBUG`, `WARN`, `ERROR`)  |
| `warp::log::tag`    | Simple string representing a tag                             |
| `warp::log::sender` | Logger instance that outputs formatted messages with context |


#### `sender` with a custom string context
```cpp
warp::log::sender logger {"SYSTEM"};
```

#### `sender` With a single tag
```cpp
warp::log::sender logger {warp::log::make_default_tag("[ENGINE]")};
```

#### `sender` With multiple tags
```cpp
std::vector<warp::log::tag> tags {
    warp::log::make_default_tag("[ENGINE]"),
    warp::log::make_default_tag("[SCENE_SYS]")
};

warp::log::sender logger {tags};
```

#### Logging to console with a `sender`

```cpp
logger.info("System initialized."); // Logging information
logger.dbg("Time took for initialization: {}.", timer.time_took()); // Debugging
logger.warn("Audio plugin not found."); // Logging warnings
logger.info("Unable to load audio {}.", audio_file_name); // Logging Errors

// Enable runtime timestamps
logger.log_timestamp(true);
logger.info("Logged with timestamp.");
```

---

### Notes
- `tag` is a simple `std::string` for lightweight, flexible usage.
- `sender` is lightweight, copyable, and movable.
- All logging functions of `sender` use variadic templates with `std::format` for type-safe, efficient formatting.
- Runtime timestamps are calculated on-demand for each log call when enabled.
---

## Warp Timer
*In-Progress*