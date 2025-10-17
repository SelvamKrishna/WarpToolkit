# Warp Toolkit

Warp Toolkit is a collection of high-performance, general-purpose utility libraries for C++.

## Overview
- **[WarpLog](#warp-log):** A lightweight, high-performance logging library
- **[WarpTimer](#warp-timer):** A lightweight, high-performance timing and benchmarking library

---

## Warp Log

Warp Log is a **logging** library designed for C++ projects.
It supports colorized output, tag-based context, optional runtime timestamps, and thread-safe console logging.

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

## Warp Timer

Warp Timer is a **timing and benchmarking utility** designed for real-time applications, profiling tools, and performance-sensitive C++ systems.

---

### Features

- **High precision:** Built on `std::chrono::high_resolution_clock` for sub-microsecond accuracy.  
- **Colorized output:** Uses ANSI escape sequences to visually distinguish timing logs.  
- **Benchmarking tools:** Run multiple iterations and view mean, median, and mode performance data.  
- **Flexible units:** Measure time in microseconds, milliseconds, or seconds.  
- **RAII-friendly:** Automatically logs elapsed time on destruction.

---

### Usage

| Component                    | Description                                                               |
|------------------------------|---------------------------------------------------------------------------|
| `warp::timer`                | Main class for timing and benchmarking.                                   |
| `warp::time_unit`            | Enum specifying time units (`MICRO_SECONDS`, `MILLI_SECONDS`, `SECONDS`). |
| `timer::measure_function()`  | Measures the execution time of a callable function.                       |
| `timer::default_benchmark()` | Runs repeated measurements and reports benchmark statistics.              |

#### Timing functions and code blocks
```cpp
void matrix_multiply(size_t n) {
	warp::timer t0 { // Constructed timer is pushed into the function stack
		std::format("Overall Matrix Multiplication of {}x{} matrix", n, n),
		warp::time_unit::MICRO_SECONDS // Timer will use micro seconds as unit of time
	};

	std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
	std::vector<std::vector<double>> B(n, std::vector<double>(n, 0.0));
	std::vector<std::vector<double>> C(n, std::vector<double>(n, 0.0));
	
	{ // Code blocks created to carry out designated task
		warp::timer t1 {"Matrix RNG Population", warp::time_unit::MICRO_SECONDS};
		std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<double> dist(0.0, 1.0);

		for (size_t i = 0; i < n; ++i)
			for (size_t j = 0; j < n; ++j) {
				A[i][j] = dist(rng);
				B[i][j] = dist(rng);
			}

		// Timer t1 is stopped and destroyed
	}

	{ // Code blocks created to carry out designated task
		warp::timer t2 {"Matrix Mutiplication", warp::time_unit::MICRO_SECONDS};

		for (size_t i = 0; i < n; ++i)
			for (size_t j = 0; j < n; ++j)
				for (size_t k = 0; k < n; ++k)
					C[i][j] += A[i][k] * B[k][j];

		// Timer t2 is stopped and destroyed
	}

	// Timer t0 is stopped and destroyed
}
```

#### Measuring and Benchmarking with builtin tools
```cpp
// Size of the matrix to multiply
size_t n = 100;

// Measure and log the execution time of a single matrix multiplication
warp::timer::measure_function<warp::time_unit::MICRO_SECONDS>(
    std::format("Single multiplication of a {}x{} matrix", n, n),
    [&n] { matrix_multiply(n); }
);

uint32_t total_iterations = 100;

// Benchmark the function over multiple runs
// - Executes the lambda `total_iterations` times
// - Collects all execution times
// - Computes and logs Mean, Median, and Mode
warp::timer::default_benchmark<warp::time_unit::MICRO_SECONDS>(
    std::format("Benchmark: {}x{} matrix multiplication", n, n),
    [&n] { matrix_multiply(n); },
    total_iterations
);
```