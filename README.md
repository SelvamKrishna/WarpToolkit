# WarpToolkit

**WarpToolkit** is a lightweight, high-performance C++ utility library for timing, logging, and benchmarking.
It provides modern C++ utilities like high-resolution timers, hierarchical task timing, and flexible logging with ANSI color support.

---

## Features

- ### [**warp_log**](warp-log)

- ### [**warp_test**](warp-test)

- ### [**warp_timer**](warp-timer)

---

## Warp Log

*lightweight*, *modern*, and *flexible* logging library for C++20 and above.
It provides *ANSI-colored* console logging, *context tagging*, and optional *timestamped* logging for real-time diagnostics and clean runtime output.

|Feature|Description|
|-------|-----------|
|Multiple Log Levels|Supports Message, Info, Warn, Error, and Debug (auto-disabled in release builds).|
|Colored Output|Per-level and per-tag ANSI color customization for clear visibility.|
|Tagging System|Use one or more tags (`warp::log::Tag`) to organize and contextualize messages.|
|Timestamp Logging|`TimedLogger` automatically prepends timestamps to all log messages.|

### Example: Basic Logging

```cpp
#include "warp_log/tag.hpp"
#include "warp_log/logger.hpp"

using namespace warp::log;

int main() {
    Logger app_log { makeColoredTag(ANSIFore::Green, "[APP]") };

    app_log.info("Application started");
    app_log.warn("Low memory detected: {} MB left", 128);
    app_log.err("Critical error: {}", "Failed to load resource");
}
```

### Example: Timestamped Logging

```cpp
#include "warp_log/timed_logger.hpp"

using namespace warp::log;

int main() {
    TimedLogger timed_log {
        makeColoredTag(ANSIFore::Yellow, "[CORE]"),
        ANSIFore::BrightBlue // Timestamp color
    };

    timed_log.info("Initialized successfully");
    timed_log.dbg("Loading assets...");
    timed_log.err("Failed to initialize graphics backend");
}
```

### Example: Creating Tags

Tags are simple `std::string`identifiers used to categorize logs.

```cpp
Tag default_tag = makeDefaultTag("[GAME]");
Tag colored_tag = makeColoredTag(ANSIFore::Cyan, "[RENDER]");
```

You can also combine multiple tags for complex log contexts:

```cpp
std::vector<Tag> tags {
    makeColoredTag(ANSIFore::Red, "[ENGINE]"),
    makeColoredTag(ANSIFore::Blue, "[AUDIO]"),
};

Logger audio_log { tags };
audio_log.info("Audio device initialized");
```

### Internal Overview

|Component|Description|
|---------|-----------|
|`warp::log::Logger`|Base class for logging with context and colorized tags.|
|`warp::log::TimedLogger`|Extends `Logger` to add timestamped messages.|
|`warp::log::Tag`|Type alias for std::string, representing a contextual identifier.|

---

## Warp Test

*Lightweight*, header-only *unit testing framework* for C++20, designed for *readable*, *colorized*, and *structured* console output.
It integrates seamlessly with [Warp Log](warp-log) to give hierarchical test results with ANSI colors.

|Feature|Description|
|-------|-----------|
|Colorized Output|Clearly distinguishes suites, test cases, and pass/fail status.|
|Automatic Summaries|Each suite and collection automatically reports passed/total case counts for clear progress tracking.|
|Hierarchical Organization|Supports Test Cases -> Suites → Collections → Registry for structured tests.|

### Example: Creating a Test Suite

```cpp
#include "warp_test/suite.hpp"
using namespace warp::test;

TEST_SUITE(MathTests) {
    Suite suite("Basic Math");

    suite.test(1 + 1 == 2, "Addition works");
    suite.test(2 * 2 == 4, "Multiplication works");

    TEST_EQ(suite, 5 - 3, 2);  // Macro helper
    TEST_NEQ(suite, 2 + 2, 5);

    return suite.getSummary();
}
```

### Example: Organizing Multiple Suites

```cpp
#include "warp_test/registry.hpp"
using namespace warp::test;

int main() {
    Registry registry;

    registry.addCollection("Arithmetic", { &MathTests, &AlgebraTests });
    registry.addCollection("Physics", { &KinematicsTests, &DynamicsTests });

    return registry.conclude(); // 0 if all tests passed, 1 otherwise
}
```

### Macros Overview

|Macro|Description|
|-----|-----------|
|`TEST_SUITE(FN)`|Defines a function returning a Summary for a suite.|
|`TEST_EQ(SUITE, ACTUAL, EXPECTED)`|Checks that ACTUAL == EXPECTED.|
|`TEST_NEQ(SUITE, ACTUAL, EXPECTED)`|Checks that ACTUAL != EXPECTED.|

### Internal Overview

|Component|Description|
|---------|-----------|
|`warp::test::Suite`|Represents a single suite of tests; logs results and summary.|
|`warp::test::internal::Summary`|Tracks passed, failed, and total test counts|
|`warp::test::Registry`|Manages multiple collections of suites and prints the overall summary.|

---

## Warp Timer

**Lightweight** and precise timing utility designed to measure **execution durations**, **profile performance**, and **benchmark** code sections in real time.

| Feature | Description |
| ------- | ----------- |
| High-Resolution Timers | Provides accurate timing for code execution. |
| Scoped Timing | Automatically measures the duration of a code block when used with RAII-style wrappers. |
| Hierarchical Timing | Supports nested timers for detailed performance analysis. |
| Function Benchmarking | Allows benchmarking of functions with configurable iterations. |

---

### `warp::Timer` Showcase

```cpp
void someFunction() {
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
}
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
