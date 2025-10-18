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

enum class TimeUnit : uint8_t { MicroSeconds, MilliSeconds, Seconds, };

namespace internal {

inline constexpr int unitID(TimeUnit u) noexcept { return static_cast<int>(u); }

/// Conversion Table: [us, ms, s]
static inline constexpr double TABLE[3][3] {
  /*us*/ {1.0,       0.001,    0.000001},
  /*ms*/ {1000.0,    1.0,      0.001   },
  /* s*/ {1'000'000, 1000.0,   1.0     },
};

/// Compile-time conversion
template <TimeUnit Source = TimeUnit::MilliSeconds, TimeUnit Target>
inline constexpr double convertUnit(double value) noexcept {
  if constexpr (Source == Target) return value;
  return value * TABLE[unitID(Source)][unitID(Target)];
}

/// Runtime conversion
inline double convertUnit(double value, TimeUnit source, TimeUnit target) noexcept {
  if (source == target) return value;
  return value * TABLE[unitID(source)][unitID(target)];
}

/// Character to represent time unit
inline constexpr char timeUnitPrefix(TimeUnit u) noexcept {
  const char PREFIX_CHAR[3] { 'u', 'm', '\x00' };
  return PREFIX_CHAR[unitID(u)];
}

/// Formatted ANSI colored tag-like elapsed string
inline std::string formatElapsed(double value, TimeUnit u) noexcept {
  return std::format("\033[32m[{:.3f} {}s]\033[0m", value, timeUnitPrefix(u));
}

} // namespace internal

/// High-precision timing and benchmarking utility.
class WARP_TOOLKIT_API Timer {
private:
  static void _logElapsed(std::string_view desc, double elapsed, TimeUnit unit) noexcept;
  static void _logBenchmark(std::string_view desc, std::vector<double> results, TimeUnit unit) noexcept;

protected:
  const std::string _DESC;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  const TimeUnit _UNIT;
  bool _is_running {true};

  [[nodiscard]] double _getTimeSinceStart() const noexcept;
  [[nodiscard]] double _stopAndGetElapsed() noexcept;

  /// Returns elapsed time in ms
  [[nodiscard]] static double _measureCallableTimeMS(const std::function<void()>& callable) noexcept;

public:
  explicit Timer() noexcept
  : _DESC {""}, _start {std::chrono::high_resolution_clock::now()}, _UNIT {TimeUnit::MilliSeconds} {}

  explicit Timer(std::string description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : _DESC {std::move(description)}, _start {std::chrono::high_resolution_clock::now()}, _UNIT {unit} {}

  ~Timer() noexcept;

  void start() noexcept;
  void stop() noexcept;
  void reset() noexcept { start(); }

  /// Measures & logs a single callable execution
  template <TimeUnit Target = TimeUnit::MilliSeconds>
  static double measure(std::string_view desc, const std::function<void()>& callable) noexcept {
    const double ELAPSED = internal::convertUnit<TimeUnit::MilliSeconds, Target>(_measureCallableTimeMS(callable));
    _logElapsed(desc, ELAPSED, Target);
    return ELAPSED;
  }

  /// Benchmarks callable multiple times and report summary.
  template <TimeUnit Target = TimeUnit::MilliSeconds>
  static void benchmark(std::string_view desc, const std::function<void()>& callable, uint32_t samples = 8) noexcept {
    std::vector<double> results;
    results.reserve(samples);

    while (samples--) results.push_back(
      internal::convertUnit<TimeUnit::MilliSeconds, Target>(_measureCallableTimeMS(callable))
    );

    _logBenchmark(desc, std::move(results), Target);
  }
};

/// Hierarchical timer used for nested or scoped measurements.
class WARP_TOOLKIT_API HierarchyTimer final : public Timer {
private:
  double _sub_task_measure {0.0};

  void _logTimerStart() const noexcept;
  void _subTaskImpl(std::string_view desc, double elapsed_ms, TimeUnit display_unit) noexcept;

public:
  explicit HierarchyTimer() noexcept : Timer {} { _logTimerStart(); }

  explicit HierarchyTimer(std::string description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : Timer {std::move(description), unit} { _logTimerStart(); }

  ~HierarchyTimer() noexcept;

  void start() noexcept = delete;
  void reset() noexcept = delete;
  void stop()  noexcept;

  /// Calls and measures the provided callable
  template <TimeUnit Target = TimeUnit::MilliSeconds>
  void subTask(std::string_view desc, const std::function<void()>& callable) noexcept {
    _subTaskImpl(desc, _measureCallableTimeMS(callable), Target);
  }

  /// Calls and measures the provided callable (uses HierarchyTimer::_UNIT) as time unit
  void subTask(std::string_view desc, const std::function<void()>& callable) noexcept;

  template <TimeUnit>
  static double measure(std::string_view, const std::function<void()>&) = delete;

  template <TimeUnit>
  static void benchmark(std::string_view, const std::function<void()>&, uint32_t) = delete;
};

} // namespace warp

#endif // WARP_TIMER