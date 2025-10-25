# WarpToolkit

**WarpToolkit** is a lightweight, high-performance **C++20 utility library** for **logging**, **timing**, **benchmarking**, and **unit testing**.
It provides modern C++ utilities for:

- High-resolution timers and hierarchical task timing

- Flexible, ANSI-colored logging

- Lightweight, structured unit testing

---

## Features Overview

|Module|Features|
|------|--------|
|[**warp_log**](warp-log)|ANSI-colored logging, custom tags, timestamps, multiple log levels|
|[**warp_test**](warp-test)|High-resolution timers, hierarchical timers, function benchmarking|
|[**warp_timer**](warp-timer)|Structured unit testing, collections, automatic summaries|

---

## Warp Log

*lightweight*, *modern*, and *flexible* logging library for C++20 and above.
It provides *ANSI-colored* console logging, *context tagging*, and optional *timestamped* logging for real-time diagnostics and clean runtime output.

|Feature|Description|
|-------|-----------|
|**ANSI Colors**|Colored output for log levels and tags|
|**Custom Tags**|Default or colored tags; can combine multiple tags|
|**Timestamps**|Optional timestamp logging per message|
|**Custom Log Levels**|`Message`, `Info`, `Debug`, `Warn`, `Error`|
|**Thread Safety**|Logs from multiple threads safely|

### Logging Examples

```cpp
using namespace warp::log;

// Default tag
Logger logger1 { makeDefaultTag("[HELLO]") };
logger1.info("Hello World!");

// Colored tags
Logger logger2 {{
    makeColoredTag(ANSIFore::Blue, "[ENGINE]"),
    makeColoredTag(ANSIFore::Cyan, "[SYSTEM]")
}};

logger2.warn("Unable to locate resource: {}", "assets/texture.png");
logger2.err("Failed to load plugin");

// Timestamps

TimedLogger timed_logger { makeDefaultTag("[TIMED]"), ANSIFore::Yellow };
timed_logger.info("Hello with timestamp!");

// Colored console logging
std::cout << ANSIFore::Blue << "Hello World\n" << ANSIFore::Reset;
```

## Warp Test

**Warp Test** is a **lightweight unit testing framework** with hierarchical, colorized reporting.

| Feature | Description |
| ------- | ----------- |
|**Colorized Output**|Distinguishes suites, cases, pass/fail status|
|**Hierarchical Organization**|Suites → Collections → Registry|
|**Automatic Summaries**|[passed/total] reporting for suites and collections|
|**Macro Helpers**|`TEST_SUITE`, `TEST_EQ`, `TEST_NEQ` simplify test writing|

---

### Example: Test Suite

```cpp
using namespace warp::test;

TEST_SUITE(MathTests) {
    Suite suite("Basic Math");

    suite.test(1 + 1 == 2, "Addition works");
    TEST_EQ(suite, 5 - 3, 2);
    TEST_NEQ(suite, 2 + 2, 5);

    return suite.getSummary();
}
```

### Example: Registry with Multiple Collections

```cpp
int main() {
    return Registry {}
        .addCollection("Arithmetic", { &MathTests, &AlgebraTests })
        .addCollection("Physics", { &KinematicsTests, &DynamicsTests })
        .conclude();
}
```

### Macros Overview

|Macro|Description|
|-----|-----------|
|`TEST_SUITE(FN)`|Defines a function returning a Summary for a suite.|
|`TEST_EQ(SUITE, ACTUAL, EXPECTED)`|Checks that ACTUAL == EXPECTED.|
|`TEST_NEQ(SUITE, ACTUAL, EXPECTED)`|Checks that ACTUAL != EXPECTED.|

---

---

## Warp Timer

**Warp Timer** measures execution duration with **high precision** and supports **benchmarking**.

|Feature|Description|
|-------|-----------|
|**High-Resolution Timing**|RAII-based timers and manual start/stop|
|**Function Benchmarking**|Measure mean and median across multiple runs|
|**Flexible Time Units**|`MicroSeconds`, `MilliSeconds`, `Seconds`|
|**ANSI-Colored Output**|Logs times in visually clear, color-coded format|

### Examples

- Scoped Timer

```cpp
using namespace warp::timer;

void sleepFor500ms() {
    Timer t("Sleep For 500ms");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
```

- Manual Timer

```cpp
Timer t("Sorting", TimeUnit::Milliseconds);
t.start();
sort(array.begin(), array.end());
t.stop();
```

- Function Timing

```cpp
Timer::measure<TimeUnit::Seconds>(
    "Multiply matrices",
    [] { multiplyMatrix(100); }
);
```

- Function Benchmarking

```cpp
Timer::benchmark<TimeUnit::Seconds>(
    "Matrix multiplication (20 runs)",
    [] { multiplyMatrix(100); },
    20 // sample size
);
```

- Hierarchy Timer

```cpp
HierarchyTimer ht("Load Assets");

// Subtask 1
ht.subTask("Load Texture", [] {
    loadTexture("texture.png");
});

// Subtask 2
ht.subTask("Load Music", [] {
    loadMusic("music.mp3");
});
```
