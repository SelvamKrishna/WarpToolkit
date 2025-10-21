#include <iostream>
#include "warp_timer.hpp"

namespace warp {

[[nodiscard]] double Timer::_getTimeSinceStart() const noexcept {
  const auto NOW = std::chrono::high_resolution_clock::now();
  const double ELAPSED = std::chrono::duration<double, std::milli>(NOW - _start).count();
  return internal::convertUnit(ELAPSED, TimeUnit::MilliSeconds, _UNIT);
}

[[nodiscard]] double Timer::_stopAndGetElapsed() noexcept {
  const double ELAPSED = _getTimeSinceStart();
  _is_running = false;
  _start = std::chrono::high_resolution_clock::now(); // reset start
  return ELAPSED;
}

[[nodiscard]] double Timer::_measureCallableTimeMS(const std::function<void()>& callable) noexcept {
  const auto START = std::chrono::high_resolution_clock::now();
  callable();
  const auto STOP = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(STOP - START).count();
}

Timer::~Timer() noexcept { if (_is_running) stop(); }

void Timer::start() noexcept {
  _is_running = true;
  _start = std::chrono::high_resolution_clock::now();
}

void Timer::stop() noexcept {
  const double ELAPSED = _stopAndGetElapsed();
  _logElapsed(_DESC, ELAPSED, _UNIT);
}

// --- hierarchy_timer ---

void HierarchyTimer::_logTimerStart() const noexcept {
  std::cout << "\033[34m[TIMER]\033[0m : " << _DESC << " {\n\n";
}

void HierarchyTimer::_subTaskImpl(
  std::string_view desc,
  double elapsed_ms,
  TimeUnit display_unit
) noexcept {
  _sub_task_measure += internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);

  std::cout
    << "  \033[34m[TIMER][SUB]\033[0m"
    << internal::formatElapsed(
      internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, display_unit),
      display_unit
    )
    << " : " << desc << '\n';
}

} /// namespace warp