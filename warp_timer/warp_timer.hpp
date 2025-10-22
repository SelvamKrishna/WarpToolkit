#pragma once

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <format>
#include <iostream>
#include <algorithm>
#include <numeric>

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

static inline constexpr double TABLE[3][3] {
  /*us*/ {1.0,       0.001,    0.000001},
  /*ms*/ {1000.0,    1.0,      0.001   },
  /* s*/ {1'000'000, 1000.0,   1.0     },
};

template <TimeUnit Source = TimeUnit::MilliSeconds, TimeUnit Target>
inline constexpr double convertUnit(double value) noexcept {

  if constexpr (Source == Target) return value;
  return value * TABLE[unitID(Source)][unitID(Target)];
}

inline double convertUnit(double value, TimeUnit source, TimeUnit target) noexcept {
  if (source == target) return value;
  return value * TABLE[unitID(source)][unitID(target)];
}

inline constexpr char timeUnitPrefix(TimeUnit u) noexcept {
  const char PREFIX_CHAR[3] { 'u', 'm', '\x00' };
  return PREFIX_CHAR[unitID(u)];
}

inline std::string formatElapsed(double value, TimeUnit u) noexcept {
  return std::format("\033[32m[{:.3f} {}s]\033[0m", value, timeUnitPrefix(u));
}

} // namespace internal

class WARP_TOOLKIT_API Timer {
private:
  static void _logElapsed(std::string_view desc, double elapsed, TimeUnit unit) noexcept {
    std::cout << std::format(
      "\033[34m[TIMER]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n",
      elapsed , internal::timeUnitPrefix(unit) , desc
    );
  }

  static void _logBenchmark(std::string_view desc, std::vector<double> results, TimeUnit time_unit) noexcept {
    if (results.empty()) [[unlikely]] {
      std::cout
        << "\033[34m[TIMER][BENCHMARK]\033[0m\033[33m[WARNING]\033[0m : "
        << "Trying to benchmark empty results\n"
      ;
    }

    const size_t SIZE {results.size()};
    std::sort(results.begin(), results.end());

    const double MEAN {std::accumulate(results.begin(), results.end(), 0.0) / SIZE};

    const double MEDIAN {
      [&results, &SIZE]{
        size_t mid = SIZE / 2;
        return (SIZE % 2 == 0) ? (results[mid - 1] + results[mid]) / 2.0 : results[mid];
      } ()
    };

    const double MODE {
      [&results, &SIZE] {
        double mode = results[0];
        size_t max_count = 1, count = 1;

        for (size_t i = 1; i < SIZE; ++i) {
          count = (results[i] == results[i - 1]) ? count + 1 : 1;
          if (count > max_count) { max_count = count; mode = results[i]; }
        }

        return mode;
      } ()
    };

    const char U = internal::timeUnitPrefix(time_unit);
    std::cout << std::format(
      ""
      "\033[34m[TIMER][BENCHMARK]\033[0m : {}\n"
      "  \033[32m[MEAN]\033[0m   : {:.3f} {}s\n"
      "  \033[32m[MEDIAN]\033[0m : {:.3f} {}s\n"
      "  \033[32m[MODE]\033[0m   : {:.3f} {}s\n",
      desc, MEAN, U, MEDIAN, U, MODE, U
    );
  }

protected:
  const std::string _DESC;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  const TimeUnit _UNIT;
  bool _is_running {true};

  [[nodiscard]] double _getTimeSinceStart() const noexcept {
    const auto NOW = std::chrono::high_resolution_clock::now();
    const double ELAPSED = std::chrono::duration<double, std::milli>(NOW - _start).count();
    return internal::convertUnit(ELAPSED, TimeUnit::MilliSeconds, _UNIT);
  }

  [[nodiscard]] double _stopAndGetElapsed() noexcept {
    const double ELAPSED = _getTimeSinceStart();
    _is_running = false;
    _start = std::chrono::high_resolution_clock::now(); // reset start
    return ELAPSED;
  }

  [[nodiscard]] static double _measureCallableTimeMS(const std::function<void()>& callable) noexcept {
    const auto START = std::chrono::high_resolution_clock::now();
    callable();
    const auto STOP = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(STOP - START).count();
  }

public:
  explicit Timer() noexcept
  : _DESC {""}, _start {std::chrono::high_resolution_clock::now()}, _UNIT {TimeUnit::MilliSeconds} {}

  explicit Timer(std::string description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : _DESC {std::move(description)}, _start {std::chrono::high_resolution_clock::now()}, _UNIT {unit} {}

  ~Timer() noexcept { if (_is_running) stop(); }

  void start() noexcept {
    _is_running = true;
    _start = std::chrono::high_resolution_clock::now();
  }

  void stop() noexcept  {
    const double ELAPSED = _stopAndGetElapsed();
    _logElapsed(_DESC, ELAPSED, _UNIT);
  }

  void reset() noexcept { start(); }

  template <TimeUnit Target = TimeUnit::MilliSeconds>
  static double measure(std::string_view desc, const std::function<void()>& callable) noexcept {
    const double ELAPSED = internal::convertUnit<TimeUnit::MilliSeconds, Target>(_measureCallableTimeMS(callable));
    _logElapsed(desc, ELAPSED, Target);
    return ELAPSED;
  }

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

class WARP_TOOLKIT_API HierarchyTimer final : public Timer {
private:
  double _sub_task_measure {0.0};

  void _logTimerStart() const noexcept {
    std::cout << "\033[34m[TIMER]\033[0m : " << _DESC << " {\n\n";
  }

  void _subTaskImpl(std::string_view desc, double elapsed_ms, TimeUnit display_unit) noexcept {
    {
      _sub_task_measure += internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);

      std::cout
        << "  \033[34m[TIMER][SUB]\033[0m"
        << internal::formatElapsed(
          internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, display_unit),
          display_unit
        )
        << " : " << desc << '\n';
    }
  }

public:
  explicit HierarchyTimer() noexcept : Timer {} { _logTimerStart(); }

  explicit HierarchyTimer(std::string description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : Timer {std::move(description), unit} { _logTimerStart(); }

  ~HierarchyTimer() noexcept { if (_is_running) stop(); }

  void start() noexcept = delete;
  void reset() noexcept = delete;

  void stop()  noexcept {
    const double ELAPSED = _stopAndGetElapsed();
    std::cout << "\n} " << std::format("\033[32m[{:.3f} {}s]\033[0m\n", ELAPSED, internal::timeUnitPrefix(_UNIT));
  }


  template <TimeUnit Target = TimeUnit::MilliSeconds>
  void subTask(std::string_view desc, const std::function<void()>& callable) noexcept {
    _subTaskImpl(desc, _measureCallableTimeMS(callable), Target);
  }

  void subTask(std::string_view desc, const std::function<void()>& callable) noexcept {
    _subTaskImpl(desc, _measureCallableTimeMS(callable), _UNIT);
  }

  template <TimeUnit>
  static double measure(std::string_view, const std::function<void()>&) = delete;

  template <TimeUnit>
  static void benchmark(std::string_view, const std::function<void()>&, uint32_t) = delete;
};

} // namespace warp
