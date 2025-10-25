#pragma once

#include "misc.hpp"
#include "timer.hpp"
#include "benchmarking.hpp"

#include "warp_log/misc.hpp"

#include <string_view>
#include <functional>

namespace warp::timer {

/// Measures and logs the total time taken in a hierarchical manner
class HierarchyTimer final : public Timer {
private:
  double   _sub_task_measure {0.0};
  uint32_t _sub_task_depth   {1};

  void _logTimerStart() const noexcept {
    log::Logger {log::makeColoredTag(log::ANSIFore::Blue, "[TIMER][HIERARCHY]")}.msg(_DESC);
  }

  void _subTaskImpl(std::string_view desc, double elapsed_ms, TimeUnit display_unit) noexcept {
    _sub_task_measure += internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);
    log::Logger {{
      log::makeDepthTag(_sub_task_depth++),
      log::makeColoredTag(log::ANSIFore::Blue, "[TIMER][TASK]")
    }}.msg(
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
    _subTaskImpl(desc, internal::measureCallableTimeMS(callable), Target);
  }

  void subTask(std::string_view desc, const std::function<void()>& callable) noexcept {
    _subTaskImpl(desc, internal::measureCallableTimeMS(callable), _UNIT);
  }

};

} // namespace warp::timer
