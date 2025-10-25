#pragma once

#include "misc.hpp"
#include "benchmarking.hpp"

#include <chrono>
#include <string_view>

namespace warp::timer {

/// Tool to record and log the elapsed time
class Timer {
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
    return ELAPSED;
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
    internal::logElapsed(_DESC, ELAPSED, _UNIT);
  }

  void reset() noexcept { start(); }

};

} // namespace warp::timer
