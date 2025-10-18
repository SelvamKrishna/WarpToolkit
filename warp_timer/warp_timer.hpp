#pragma once

/// @file warp_timer.hpp
/// @brief High-precision timing and benchmarking utility with ANSI-colored output.
/// Provides automatic measurement, benchmarking helpers, and customizable time units.
/// ---
/// @author SelvamKrishna
/// @link https://www.github.com/SelvamKrishna
/// @date 2025-10-17

#include <iostream>
#ifndef WARP_TIMER
#define WARP_TIMER

#ifndef WARP_TOOLKIT_API
  #ifdef _WIN32
    #define WARP_TOOLKIT_API __declspec(dllexport)
  #else
    #define WARP_TOOLKIT_API
  #endif
#endif

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <string_view>

namespace warp {

/// @enum time_unit
/// @brief Represents supported time units for timing and benchmarking.
enum class time_unit : uint8_t {
  MICRO_SECONDS,  ///< Microseconds (µs)
  MILLI_SECONDS,  ///< Milliseconds (ms)
  SECONDS         ///< Seconds (s)
};

namespace internal {

/// @brief Converts a time unit to its shorthand character representation.
/// @param unit The time unit.
/// @return 'u' for microseconds, 'm' for milliseconds, or '\x00' for seconds.
inline constexpr char time_unit_to_string(time_unit unit) noexcept {
  switch (unit) {
    case time_unit::MICRO_SECONDS: return 'u';
    case time_unit::MILLI_SECONDS: return 'm';
    case time_unit::SECONDS:       return '\x00';
    default:                       return '?';
  }
}

/// @brief Converts elapsed milliseconds into another time unit.
/// @param elapsed_ms Elapsed time in milliseconds.
/// @return Elapsed time converted into the target unit.
template <time_unit ToTimeUnit>
inline constexpr double time_unit_cast(double elapsed_ms) noexcept {
  switch (ToTimeUnit) {
    case time_unit::MICRO_SECONDS: return elapsed_ms * 1000.0; // ms → µs
    case time_unit::SECONDS:       return elapsed_ms / 1000.0; // ms → s
    default:                       return elapsed_ms;          // stays in ms
  }
}

} /// namespace internal

/// @class timer
/// @brief A high-precision timing and benchmarking utility.
/// The `timer` class measures elapsed time between `start()` and `stop()`,
/// or benchmarks callable functions across multiple iterations.
class WARP_TOOLKIT_API timer {
private:
#pragma region /// Helpers

  /// @brief Print a single elapsed-time log entry.
  /// @param desc   Description or tag of the timed section.
  /// @param elapsed Time value in the specified unit.
  /// @param unit   Time unit used for display.
  static void _log(std::string_view desc, double elapsed, time_unit unit) noexcept;

  /// @brief Print benchmark statistics (mean, median, mode).
  /// @param desc   Description of the benchmark.
  /// @param results Vector of raw results (in milliseconds).
  /// @param unit   Display unit.
  static void _log_benchmark(std::string_view desc, std::vector<double> results, time_unit unit) noexcept;

#pragma endregion /// Helpers
protected:
#pragma region /// Inheritance support

  const std::string _DESC;     ///< Description used for log output.
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;  ///< Start timestamp.
  const time_unit _TIME_UNIT;  ///< Current time unit.
  bool _is_running {true};     ///< Whether the timer is currently active.

  /// @brief Calculate elapsed time since the last `start()` call.
  /// @return Elapsed time in the configured time unit.
  [[nodiscard]] double _get_time_since_start() const noexcept;

  /// @brief Measure execution time of a callable (in milliseconds).
  /// @param callable The function or lambda to execute.
  /// @return Elapsed time in milliseconds.
  [[nodiscard]] static double _measure_function_time_ms(const std::function<void()>& callable) noexcept;

  [[nodiscard]] double _stop_and_get_elapsed() noexcept;

#pragma endregion /// Inheritance support
public:
#pragma region /// Constructors & Destructors

  /// @brief Default-constructs and starts the timer immediately.
  explicit timer() noexcept
  : _DESC {""}
  , _start {std::chrono::high_resolution_clock::now()}
  , _TIME_UNIT {time_unit::MILLI_SECONDS} {}

  /// @brief Construct a named timer with an optional time unit.
  /// @param description Timer label (for logging).
  /// @param unit        Time unit for logging (default: milliseconds).
  explicit timer(std::string description, time_unit unit = time_unit::MILLI_SECONDS) noexcept
  : _DESC {std::move(description)}
  , _start {std::chrono::high_resolution_clock::now()}
  , _TIME_UNIT {unit} {}

  /// @brief Destructor automatically stops and logs if still running.
  ~timer() noexcept;

#pragma endregion /// Constructors & Destructors
#pragma region /// Basic Control Functions

  /// @brief Starts (or restarts) the timer.
  void start() noexcept;

  /// @brief Stops the timer and logs the elapsed time.
  void stop() noexcept;

  /// @brief Resets and restarts the timer
  void reset() noexcept;

#pragma endregion /// Basic Control Functions
#pragma region /// Benchmarking Tools

  /// @brief Measure execution time of a callable in a specified unit.
  /// @tparam InTimeUnit Output time unit.
  /// @param desc     Label or description for the measurement.
  /// @param callable Function to execute.
  /// @return Elapsed time in the specified unit.
  template <time_unit InTimeUnit>
  static double measure_function(std::string_view desc, const std::function<void()>& callable) noexcept;

  /// @brief Benchmark a callable function multiple times and summarize results.
  /// @tparam InTimeUnit Output time unit.
  /// @param desc        Label or description for the benchmark.
  /// @param callable    Function or lambda to benchmark.
  /// @param total_iterations Number of iterations to perform (default = 8).
  template <time_unit InTimeUnit>
  static void default_benchmark(
    std::string_view desc,
    const std::function<void()>& callable,
    uint32_t total_iterations = 8
  ) noexcept;

#pragma endregion /// Benchmarking Tools
};

#pragma region /// warp::timer

template <time_unit InTimeUnit>
double timer::measure_function(std::string_view desc, const std::function<void()>& callable) noexcept {
  const double ELAPSED {internal::time_unit_cast<InTimeUnit>(_measure_function_time_ms(callable))};
  _log(desc, ELAPSED, InTimeUnit);
  return ELAPSED;
}

template <time_unit InTimeUnit>
void timer::default_benchmark(
  std::string_view desc,
  const std::function<void()>& callable,
  uint32_t total_iterations
) noexcept {
  std::vector<double> results;
  results.reserve(total_iterations);

  while (total_iterations--) results.push_back(
    internal::time_unit_cast<InTimeUnit>(_measure_function_time_ms(callable))
  );

  _log_benchmark(desc, results, InTimeUnit);
}

#pragma endregion /// warp::timer

class WARP_TOOLKIT_API hierarchy_timer final : public timer {
private:
  double _sub_task_measure {0};

#pragma region /// Helpers

  void _log_timer_start() const noexcept;

#pragma endregion /// Helpers
public:
#pragma region /// Constructors & Destructors

  /// @brief Default-constructs and starts the timer immediately.
  explicit hierarchy_timer() noexcept;

  /// @brief Construct a named timer with an optional time unit.
  /// @param description Timer label (for logging).
  /// @param unit        Time unit for logging (default: milliseconds).
  explicit hierarchy_timer(
    std::string description,
    time_unit unit = time_unit::MILLI_SECONDS
  ) noexcept;

  /// @brief Destructor automatically stops and logs if still running.
  ~hierarchy_timer() noexcept;

#pragma endregion /// Constructors & Destructors
#pragma region /// Basic Control Functions

  /// @warning `warp::hierarchy_timer` does not support manual starting
  void start() noexcept = delete;

  /// @brief Stops the timer and logs the elapsed time.
  void stop() noexcept;

  /// @warning `warp::hierarchy_timer` does not support manual resetting
  void reset() noexcept = delete;

#pragma endregion /// Basic Control Functions
#pragma region /// Sub-task

  template <time_unit InTimeUnit>
  void sub_task(std::string_view desc, const std::function<void()>& callable) noexcept;

  void sub_task(std::string_view desc, const std::function<void()>& callable) noexcept;

#pragma endregion /// Sub-task
};

#pragma region /// warp::hierarchy_timer

inline hierarchy_timer::hierarchy_timer() noexcept : timer {} { _log_timer_start(); }

inline hierarchy_timer::hierarchy_timer(
  std::string description,
  time_unit unit
) noexcept : timer {description, unit} { _log_timer_start(); }

template <time_unit InTimeUnit>
void hierarchy_timer::sub_task(std::string_view desc, const std::function<void()>& callable) noexcept {
  const double ELAPSED_MS = _measure_function_time_ms(callable);
  const double MEASURE = internal::time_unit_cast<InTimeUnit>(ELAPSED_MS);

  switch (_TIME_UNIT) {
    case time_unit::MICRO_SECONDS: _sub_task_measure += ELAPSED_MS * 1000; break;
    case time_unit::MILLI_SECONDS: _sub_task_measure += ELAPSED_MS; break;
    case time_unit::SECONDS: _sub_task_measure += ELAPSED_MS / 1000; break;
    default: break;
  }

  std::cout
    << std::format(
      "  \033[34m[TASK]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n"
      , MEASURE
      , internal::time_unit_to_string(InTimeUnit)
      , desc
    )
  ;
}

inline void hierarchy_timer::sub_task(std::string_view desc, const std::function<void()>& callable) noexcept {
  const double ELAPSED_MS = _measure_function_time_ms(callable);

  switch (_TIME_UNIT) {
    case time_unit::MICRO_SECONDS: _sub_task_measure += ELAPSED_MS * 1000; break;
    case time_unit::MILLI_SECONDS: _sub_task_measure += ELAPSED_MS; break;
    case time_unit::SECONDS: _sub_task_measure += ELAPSED_MS / 1000; break;
    default: break;
  }

  std::cout
    << std::format(
      "  \033[34m[TASK]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n"
      , ELAPSED_MS
      , internal::time_unit_to_string(_TIME_UNIT)
      , desc
    )
  ;
}

#pragma endregion /// warp::hierarchy_timer

} /// namespace warp

#endif /// WARP_TIMER