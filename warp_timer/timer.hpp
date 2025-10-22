#pragma once

#include "misc.hpp"

#include "warp_log/tag.hpp"

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

namespace warp {

class Timer {
private:
  static void _logElapsed(std::string_view desc, double elapsed, TimeUnit unit) noexcept {
    std::cout
      << log::setColor(log::ANSIFore::Blue)
      << "[TIMER]"
      << log::resetColor()
      << internal::formatElapsed(elapsed, unit)
      << " : " << desc << "\n";
  }

  static inline void _logBenchmark(std::string_view desc, std::vector<double> results, TimeUnit time_unit) noexcept {
    if (results.empty()) [[unlikely]] {
      log::internal::writeToConsole(
        log::Level::Warn,
        warp::log::makeColoredTag(log::ANSIFore::Blue, "[TIMER][BENCHMARK]"),
        "Trying to benchmark empty results"
      );
      return;
    }

    const size_t SIZE = results.size();
    std::sort(results.begin(), results.end());

    const double MEAN = std::accumulate(results.begin(), results.end(), 0.0) / SIZE;
    const double MEDIAN = (SIZE % 2 == 0)
      ? (results[SIZE / 2 - 1] + results[SIZE / 2]) / 2.0
      : results[SIZE / 2];

    const double MODE {
      [&] {
        size_t max_count = 1, count = 1;
        double mode;
        for (size_t i = 1; i < SIZE; ++i) {
          count = (results[i] == results[i - 1]) ? count + 1 : 1;
          if (count > max_count) { max_count = count; mode = results[i]; }
        }

        return mode;
      } ()
    };

    const char U = internal::timeUnitPrefix(time_unit);

    const std::string prefix = makeColoredTag(log::ANSIFore::Blue, "[TIMER][BENCHMARK]");
    const std::string msg = std::format(
      "{}\n  {}[MEAN]   {}: {:.3f} {}s\n  {}[MEDIAN] {}: {:.3f} {}s\n  {}[MODE]   {}: {:.3f} {}s",
      desc,
      setColor(log::ANSIFore::Green), log::resetColor(), MEAN, U,
      setColor(log::ANSIFore::Green), log::resetColor(), MEDIAN, U,
      setColor(log::ANSIFore::Green), log::resetColor(), MODE, U
    );

    log::internal::writeToConsole(log::Level::Info, prefix, msg);
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
    _start = std::chrono::high_resolution_clock::now();
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

    while (samples--)
      results.push_back(internal::convertUnit<TimeUnit::MilliSeconds, Target>(_measureCallableTimeMS(callable)));

    _logBenchmark(desc, std::move(results), Target);
  }
};

} // namespace warp
