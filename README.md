# WarpToolkit

**WarpToolkit** is a lightweight, high-performance C++ utility library for timing, logging, and benchmarking.
It provides modern C++ utilities like high-resolution timers, hierarchical task timing, and flexible logging with ANSI color support.

---

## Features

### [**warp_log**](warp-log) : Logging Utilities

- Flexible, ANSI-colored logging via simple API.
- Easy integration with existing code.

### [**warp_timer**](warp-timer) : Timing Utilities

- High-resolution timers (`timer`) with auto-start and automatic logging.
- Hierarchical timers (`hierarchy_timer`) for nested subtasks.
- Function benchmarking with configurable iterations.

---

## Installation

Clone the repository:

```shell
git clone https://github.com/SelvamKrishna/WarpToolkit.git
cd WarpToolkit
```

Get further build instructions:

```shell
python build.py help
```

---

## Warp Log v1.0

A minimal logging library for C++ 20 and above with ANSI color support.

| Feature | Description |
| ------- | ----------- |
| ANSI Colors | Supports colored output for better log readability. |
| Easy Integration | Simple API for integrating into existing projects. |
| Timestamp Logging | Option to include timestamps in log messages. |

### `warp::log` Showcase

```cpp
using namespace warp; // For brevity

Sender messenger1 {tag_factory::makeDefault("[HELLO]")}; // Default tag
messenger1.allowTimestampLogging(true);
messenger1.setTimestampColor(ANSIFore::Yellow);

messenger1.info("Hello World"); // [HH:MM:SS][HELLO][INFO] : Hello World

// Multiple tags
Sender messenger2 {
    {
        tag_factory::makeColored(ANSIFore::Blue, "[ENGINE]"),
        tag_factory::makeColored(ANSIFore::Cyan, "[SYSTEM]"),
    },
    true // Enable timestamp logging at construction
};

// Sample log messages
messenger2.info("System Version: {}", getSystemVersion());
messenger2.dbg("System Capacity: {}", getSystemCapacity());

std::string resource_path = "assets/texture.png"; // Example resource path
messenger2.warn("Unable to locate resource: {}", resource_path);

messenger2.err("Failed to load plugin. Terminating...");
```

### Making Custom `warp::log::Tag`

```cpp
// Creates a custom log tag combining a name and address in the format [name@address].
warp::LogTag makeNameAddrTag(std::string_view name, std::string_view addr) {
  return std::format("[{}@{}]", name, addr); // A log tag is just a std::string
}
```

---

## Warp Timer v2.0

A high-resolution timing library for C++ 20 and above with hierarchical task timing.

| Feature | Description |
| ------- | ----------- |
| High-Resolution Timers | Provides accurate timing for code execution. |
| Hierarchical Timing | Supports nested timers for detailed performance analysis. |
| Function Benchmarking | Allows benchmarking of functions with configurable iterations. |

---

### `warp::Timer` Showcase

```cpp
using namespace warp; // For brevity

{
    Timer t0("Subtask 1"); // Create timer
    // simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
} // End of scope, timer stops and logs time taken

// Create timer with custom time unit
Timer t1 {"Sort an array", TimeUnit::Milliseconds};

t1.start(); // Manually start the timer
demo::sortRNGArray(100);
t1.stop(); // Manually stop the timer

// Measure function execution time
Timer::measure<TimeUnit::Seconds>(
    "Multiply 2 matrices", // Task name
    [] { demo::multiplyRNGMatrix(100); } // Function to measure
);

// Benchmark function over multiple iterations
Timer::benchmark<TimeUnit::Seconds>(
    "Multiply 2 matrices",
    [] { demo::multiplyRNGMatrix(100); },
    20 // Sample size
);
```

### `warp::HierarchyTimer` Showcase

```cpp
using namespace warp; // For brevity

void demo::loadStuff() {
    HierarchyTimer ht {"Main Task"}; // Create main hierarchical timer

    // Subtask 1
    ht.subTask("Load Texture", [&] {
        render::loadTexture("texture.png");
    });

    // Subtask 2 with custom time unit
    ht.subTask<TimeUnit::Microseconds>("Load Music", [&] {
        audio::loadMusic("music.mp3");
    });

} // Main Task ends, logs hierarchical timing information
```

---
