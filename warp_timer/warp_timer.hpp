#pragma once
#ifndef WARP_TIMER
#define WARP_TIMER

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <format>

#ifndef WARP_TOOLKIT_API
  #ifdef _WIN32
    #define WARP_TOOLKIT_API __declspec(dllexport)
  #else
    #define WARP_TOOLKIT_API
  #endif
#endif

namespace warp {

enum class time_unit : uint8_t { MICRO_SECONDS, MILLI_SECONDS, SECONDS };

namespace internal {

inline constexpr int unit_id(time_unit u) noexcept { return static_cast<int>(u); }

static constexpr double table[3][3] {
  {1.0,       0.001,    0.000001},
  {1000.0,    1.0,      0.001   },
  {1'000'000, 1000.0,   1.0     }
};

// Compile-time conversion
template <time_unit Source = time_unit::MILLI_SECONDS, time_unit Target>
inline constexpr double convert_units(double value) noexcept {
  if constexpr (Source == Target) return value;
  return value * table[unit_id(Source)][unit_id(Target)];
}

// Runtime conversion
inline double convert_units(double value, time_unit source, time_unit target) noexcept {
  if (source == target) return value;
  return value * table[unit_id(source)][unit_id(target)];
}

inline constexpr char time_unit_prefix(time_unit u) noexcept {
  switch(u) {
    case time_unit::MICRO_SECONDS: return 'u';
    case time_unit::MILLI_SECONDS: return 'm';
    case time_unit::SECONDS:       return '\0';
  }
  return '?';
}

inline std::string format_elapsed(double value, time_unit u) noexcept {
  if (time_unit_prefix(u) == '\0')
    return std::format("\033[32m[{:.3f} s]\033[0m", value);
  return std::format("\033[32m[{:.3f} {}s]\033[0m", value, time_unit_prefix(u));
}

} // namespace internal

class WARP_TOOLKIT_API timer {
private:
  static void _log(std::string_view desc, double elapsed, time_unit unit) noexcept;
  static void _log_benchmark(std::string_view desc, std::vector<double> results, time_unit unit) noexcept;

protected:
  const std::string _DESC;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  const time_unit _TIME_UNIT;
  bool _is_running {true};

  [[nodiscard]] double _get_time_since_start() const noexcept;
  [[nodiscard]] double _stop_and_get_elapsed() noexcept;
  [[nodiscard]] static double _measure_callable_time_ms(const std::function<void()>& callable) noexcept;

public:
  explicit timer() noexcept
  : _DESC {""}, _start {std::chrono::high_resolution_clock::now()}, _TIME_UNIT {time_unit::MILLI_SECONDS} {}

  explicit timer(std::string description, time_unit unit = time_unit::MILLI_SECONDS) noexcept
  : _DESC {std::move(description)}, _start {std::chrono::high_resolution_clock::now()}, _TIME_UNIT {unit} {}

  ~timer() noexcept;

  void start() noexcept;
  void stop() noexcept;
  void reset() noexcept { start(); }

  template <time_unit Target = time_unit::MILLI_SECONDS>
  static double measure_function(std::string_view desc, const std::function<void()>& callable) noexcept {
    const double elapsed = internal::convert_units<time_unit::MILLI_SECONDS, Target>(_measure_callable_time_ms(callable));
    _log(desc, elapsed, Target);
    return elapsed;
  }

  template <time_unit Target = time_unit::MILLI_SECONDS>
  static void default_benchmark(std::string_view desc, const std::function<void()>& callable, uint32_t iterations = 8) noexcept {
    std::vector<double> results;
    results.reserve(iterations);

    while (iterations--) results.push_back(
      internal::convert_units<time_unit::MILLI_SECONDS, Target>(_measure_callable_time_ms(callable))
    );

    _log_benchmark(desc, std::move(results), Target);
  }
};

class WARP_TOOLKIT_API hierarchy_timer final : public timer {
private:
  double _sub_task_measure {0.0};

  void _log_timer_start() const noexcept;
  void _sub_task_impl(std::string_view desc, double elapsed_ms, time_unit display_unit) noexcept;

public:
  explicit hierarchy_timer() noexcept : timer {} { _log_timer_start(); }
  explicit hierarchy_timer(std::string description, time_unit unit = time_unit::MILLI_SECONDS) noexcept
  : timer {std::move(description), unit} { _log_timer_start(); }

  ~hierarchy_timer() noexcept;

  void start() noexcept = delete;
  void reset() noexcept = delete;
  void stop() noexcept;

  template <time_unit Target = time_unit::MILLI_SECONDS>
  void sub_task(std::string_view desc, const std::function<void()>& callable) noexcept {
    _sub_task_impl(desc, _measure_callable_time_ms(callable), Target);
  }

  void sub_task(std::string_view desc, const std::function<void()>& callable) noexcept;
};

} // namespace warp

#endif // WARP_TIMER