# Warp Toolkit

Warp Toolkit is a collection of high-performance, general-purpose utility libraries for C++.

## Overview
- **[WarpLog](#warp-log)** : A lightweight, high-performance logging library

---

## Warp Log

Warp Log is a lightweight, high-performance logging library designed for C++ projects.  
It supports colorized output, tag-based context, and thread-safe console logging — making it ideal for real-time applications like game engines, tools, or embedded systems.

---

### Features
- **Multiple log levels:** `INFO`, `DEBUG`, `WARN`, `ERROR`  
  Each level is color-coded for improved console readability.  
- **Thread-safe:** Console output is synchronized using mutexes to prevent interleaved logs in multi-threaded environments.  
- **High-performance:** Uses minimal allocations and cached tag strings for low overhead.  
- **Flexible usage:** Supports custom string contexts, single tags, or multiple tags for contextual logging.  
- **Cross-platform:** Works on Windows, Linux, and any ANSI-compatible terminal.

---

### Usage Overview

| Component           | Description                                                  |
|---------------------|--------------------------------------------------------------|
| `warp::log::level`  | Enum defining log levels (`INFO`, `DEBUG`, `WARN`, `ERROR`)  |
| `warp::log::i_tag`  | Interface for defining custom tags                           |
| `warp::log::sender` | Logger instance that outputs formatted messages with context |

---

### Creating a custom tag

```cpp
class text_tag : public warp::log::i_tag {
private:
	std::string _text;

public:
	explicit text_tag(std::string text) : _text{std::move(text)} {}

	[[nodiscard]] std::string get_str() const noexcept override {
		return _text;
	}
};
```

---

### Creating a `sender`

#### 1. with a custom string context
```cpp
warp::log::sender logger {"SYSTEM"};
```

#### 2. with a tag
```cpp
warp::log::sender logger {text_tag{"[ENGINE]"}};
```

#### 2. with multiple tags
```cpp
static text_tag s_lib_tag{"[ENGINE]"};
static text_tag s_sys_tag{"[SCENE_SYS]"};

warp::log::sender logger {warp::log::tag_vec {
	&s_lib_tag,
	&s_sys_tag,
}};
```

---

### Logging to console with a `sender`

```cpp
logger.info("System initialized."); // Logging information
logger.dbg("Time took for initialization: {}.", timer.time_took()); // Debugging
logger.warn("Audio plugin not found."); // Logging warnings
logger.info("Unable to load audio {}.", audio_file_name); // Logging Errors
```

---

### Notes
- `sender` is a lightweight class which is easily *copyable* & *movable*
- All logging utility functions of `sender` class uses Variadic templates with `std::format` for efficient formatting.

---

### Templates
Pre-defined templates for common use cases

- `warp::log::templates::ansi_foreground_color` : Foreground ANSI color codes for terminal output.
- `warp::log::templates::colored_tag` : A log tag with a fixed foreground color.

| Component | Description |
|-----|-----|
| `warp::log::templates::ansi_foreground_color` | Foreground ANSI color codes for terminal output |
| `warp::log::templates::colored_tag` | A log tag with a fixed foreground color |