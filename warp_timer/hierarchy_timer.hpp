#pragma once

#include "timer.hpp"

#include "warp_log/misc.hpp"
#include "warp_log/logger.hpp"

#include <functional>

namespace warp::timer {

/// Measures and logs the total time taken in a hierarchical manner
class HierarchyTimer final : public Timer {
private:
  double _sub_task_measure {0.0};

  void _logTimerStart() const noexcept {
    log::Logger {log::makeColoredTag(log::ANSIFore::Blue, "[TIMER][HIERARCHY]")}.msg(_DESC);
  }

  void _subTaskImpl(std::string_view desc, double elapsed_ms, TimeUnit display_unit) noexcept {
    _sub_task_measure += internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);
    log::Logger {
      log::makeColoredTag(log::ANSIFore::Blue, "\t[TIMER][SUB_TASK]")
    }.msg(
      "{} : {}",
      internal::formatElapsed(
        internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, display_unit),
        display_unit
      ),
      desc
    );
  }

public:
  explicit HierarchyTimer() noexcept : Timer {} { _logTimerStart(); }

  explicit HierarchyTimer(std::string_view description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : Timer {std::move(description), unit} { _logTimerStart(); }

  ~HierarchyTimer() noexcept { if (_is_running) stop(); }

  void start() noexcept = delete;
  void reset() noexcept = delete;

  void stop() noexcept {
    const double ELAPSED {_stopAndGetElapsed()};
    log::Logger {
      log::makeColoredTag(log::ANSIFore::Blue, "[TIMER][HIERARCHY]")
    }.msg(internal::formatElapsed(ELAPSED, _UNIT));
  }

/// --- Sub task measuring utils ---

  template <TimeUnit Target>
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

} // namespace warp::timer
