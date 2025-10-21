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

### [**warp_test**](warp-test) : Testing Utilities

- Lightweight and flexible unit testing framework.
- Supports defining and running test cases with automatic result reporting.
- Provides hierarchical test organization for complex test suites.

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

## Warp Log

**Minimal** and efficient logging library for C++20 and above, featuring clean **ANSI-colored** console output and easy project integration.

| Feature | Description |
| ------- | ----------- |
| ANSI Colors | Supports colored output for better log readability. |
| Easy Integration | Simple API for integrating into existing projects. |
| Timestamp Logging | Optionally includes timestamps in messages for better runtime diagnostics. |
| Custom Log Levels | Supports user-defined log levels and formatted message styles. |

### Warp Log Showcase

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

### Making Custom `warp::LogTag`

```cpp
// Creates a custom log tag combining a name and address in the format [name@address].
warp::LogTag makeNameAddrTag(std::string_view name, std::string_view addr) {
    return std::format("[{}@{}]", name, addr); // A log tag is just a std::string
}
```

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

## Warp Test

**Lightweight** unit testing and validation framework designed for rapid development and clean console reporting.
It emphasizes **simplicity**, **readability**, and structured output for quick test analysis.

| Feature | Description |
| ------- | ----------- |
| ANSI Colored Output | Clearly distinguishes test statuses using colored terminal output. |
| Suite-Based Organization | Groups related test cases into Suites for modular and structured testing. |
| Collections and Registry | Allows grouping multiple suites into Collections and managing them through a central Registry. |
| Automatic Summaries | Each suite and collection automatically reports passed/failed case counts for clear progress tracking. |

### Creating a test function

```cpp
using namespace warp; // For brevity
TEST_SUITE(someTest1) { // `warp::test::internal::Summary someTest1()`
    test::Suite t {"Example Test 1"}; // Creating a Suite

    t.test(1 - 1 == 0, "Simple Subtraction"); // Logs test case result with description
    TEST_EQ(t, 1 + 1, 2); // `t.test((1 + 1) == (2), "1 + 1 == 2")`
    t.test(1 * 1 == 1, "Simple Multiplication");
    t.test(2 > 1, "Greater than");
    TEST_NEQ(t, 2 * 2, 5); // `t.test((2 * 2) != (5), "2 * 2 != 5")`

    return t.getSummary(); // Used by test::Registry
} // Suite is destroyed which automatically logs the summary
```

### `warp::test::Registry` Showcase

```cpp
int main() {
    return warp::test::Registry { // Create Registry
        /*
            Test Hierarchy from top tp bottom:
            Registry -> Collection -> Suite -> Test Cases.
        */
    }.addCollection( // Add Collection
        "Test Collection A", // Collection name
        { // List of pointers to test function
            &someTest1,
            &someTest2,
        }
    ).addCollection(
        "Test Collection B",
        {
            &someTest3,
            &someTest4,
        }
    ).conclude(); // 0 if all test's passed else 1
}
```

---
