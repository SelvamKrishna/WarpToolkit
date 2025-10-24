#pragma once

#include "misc.hpp"

#include "warp_log/tag.hpp"
#include "warp_log/logger.hpp"

#include <cstdint>
#include <chrono>
#include <vector>
#include <functional>
#include <string_view>
#include <format>
#include <algorithm>
#include <numeric>

namespace warp {

class Timer {
private:
  static void _logElapsed(std::string_view desc, double elapsed, TimeUnit unit) noexcept {
    log::Logger {
      log::makeColoredTag(log::ANSIFore::Blue, "[TIMER]")
    } .msg("{} : {}\n", internal::formatElapsed(elapsed, unit), desc);
  }

  [[nodiscard]] static std::pair<double, double> _getMeanAndMedian(const std::vector<double> sorted_ls) noexcept {
    const size_t SIZE {sorted_ls.size()};

    return std::make_pair(
      std::accumulate(sorted_ls.begin(), sorted_ls.end(), 0.0) / SIZE,
      (SIZE % 2 == 0) ? (sorted_ls[SIZE / 2 - 1] + sorted_ls[SIZE / 2]) / 2.0 : sorted_ls[SIZE / 2]
    );
  }

  static inline void _logBenchmark(std::string_view desc, std::vector<double>& results, TimeUnit time_unit) noexcept {
    static const log::Logger BENCHMARK_LOG {makeColoredTag(log::ANSIFore::Blue, "[TIMER][BENCHMARK]")};

    if (results.empty()) [[unlikely]] {
      BENCHMARK_LOG.warn("Trying to benchmark empty results");
      return;
    }

    std::sort(results.begin(), results.end());
    const auto [MEAN, MEDIAN] {_getMeanAndMedian(results)};

    BENCHMARK_LOG.msg(
      "{}\n"
      "\t{}[MEAN]   {}: {}\n"
      "\t{}[MEDIAN] {}: {}\n",
      desc,
      setColor(log::ANSIFore::Green), log::resetColor(), internal::formatElapsed(MEAN, time_unit),
      setColor(log::ANSIFore::Green), log::resetColor(), internal::formatElapsed(MEDIAN, time_unit)
    );
  }

protected:
  const std::string_view                                      _DESC;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  const TimeUnit                                              _UNIT       {TimeUnit::MilliSeconds};
  bool                                                        _is_running {true};

  [[nodiscard]] double _getTimeSinceStart() const noexcept {
    const auto NOW {std::chrono::high_resolution_clock::now()};
    const double ELAPSED {std::chrono::duration<double, std::milli>(NOW - _start).count()};
    return internal::convertUnit(ELAPSED, TimeUnit::MilliSeconds, _UNIT);
  }

  [[nodiscard]] double _stopAndGetElapsed() noexcept {
    const double ELAPSED {_getTimeSinceStart()};
    _is_running = false;
    _start = std::chrono::high_resolution_clock::now();
    return ELAPSED;
  }

  [[nodiscard]] static double _measureCallableTimeMS(const std::function<void()>& callable) noexcept {
    const auto START {std::chrono::high_resolution_clock::now()};
    callable();
    const auto STOP {std::chrono::high_resolution_clock::now()};
    return std::chrono::duration<double, std::milli>(STOP - START).count();
  }

public:
  explicit Timer() noexcept = default;

  explicit Timer(std::string_view description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : _DESC {description}, _start {std::chrono::high_resolution_clock::now()}, _UNIT {unit} {}

  ~Timer() noexcept { if (_is_running) stop(); }

  void start() noexcept {
    _is_running = true;
    _start = std::chrono::high_resolution_clock::now();
  }

  void stop() noexcept  {
    const double ELAPSED {_stopAndGetElapsed()};
    _logElapsed(_DESC, ELAPSED, _UNIT);
  }

  void reset() noexcept { start(); }

  template <TimeUnit InTimeUnit = TimeUnit::MilliSeconds>
  static double measure(std::string_view desc, const std::function<void()>& callable) noexcept {
    const double ELAPSED {
      internal::convertUnit<TimeUnit::MilliSeconds, InTimeUnit>(_measureCallableTimeMS(callable))
    };
    _logElapsed(desc, ELAPSED, InTimeUnit);
    return ELAPSED;
  }

  template <TimeUnit InTimeUnit = TimeUnit::MilliSeconds>
  static void benchmark(std::string_view desc, const std::function<void()>& callable, uint32_t samples = 8) noexcept {
    std::vector<double> results;
    results.reserve(samples);

    while (samples--)
      results.push_back(internal::convertUnit<TimeUnit::MilliSeconds, InTimeUnit>(_measureCallableTimeMS(callable)));

    _logBenchmark(desc, results, InTimeUnit);
  }
};

} // namespace warp
