#include "timer.hpp"

#include "warp_log/misc.hpp"

#include <string>
#include <iostream>
#include <functional>

namespace warp {

class HierarchyTimer final : public Timer {
private:
  double _sub_task_measure {0.0};

  void _logTimerStart() const noexcept {
    std::cout
      << log::setColor(log::ANSIFore::Blue)
      << "[TIMER]"
      << log::resetColor() << " : " << _DESC << " {\n\n";
  }

  void _subTaskImpl(std::string_view desc, double elapsed_ms, TimeUnit display_unit) noexcept {
    _sub_task_measure += internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);

    std::cout
      << log::setColor(log::ANSIFore::Blue)
      << "\t[TIMER][SUB]"
      << log::resetColor()
      << internal::formatElapsed(
        internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, display_unit),
        display_unit
      ) << " : " << desc << '\n';
  }

public:
  explicit HierarchyTimer() noexcept : Timer {} { _logTimerStart(); }

  explicit HierarchyTimer(std::string description, TimeUnit unit = TimeUnit::MilliSeconds) noexcept
  : Timer {std::move(description), unit} { _logTimerStart(); }

  ~HierarchyTimer() noexcept { if (_is_running) stop(); }

  void start() noexcept = delete;
  void reset() noexcept = delete;

  void stop() noexcept {
    const double ELAPSED = _stopAndGetElapsed();
    std::cout
      << "\n} "
      << std::format(
        "{}[{:.3f} {}s]{}\n",
        log::setColor(log::ANSIFore::Yellow),
        ELAPSED,
        internal::timeUnitPrefix(_UNIT),
        log::resetColor()
      );
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