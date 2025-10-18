#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>

#include "warp_timer.hpp"

namespace warp {

// --- timer ---

void Timer::_logElapsed(std::string_view desc, double elapsed, TimeUnit time_unit) noexcept {
  std::cout << std::format(
    "\033[34m[TIMER]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n",
    elapsed , internal::timeUnitPrefix(time_unit) , desc
  );
}

void Timer::_logBenchmark(std::string_view desc, std::vector<double> results, TimeUnit time_unit) noexcept {
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
    }()
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
    "\t\033[32m[MEAN]\033[0m   : {:.3f} {}s\n"
    "\t\033[32m[MEDIAN]\033[0m : {:.3f} {}s\n"
    "\t\033[32m[MODE]\033[0m   : {:.3f} {}s\n",
    desc, MEAN, U, MEDIAN, U, MODE, U
);
}

[[nodiscard]] double Timer::_getTimeSinceStart() const noexcept {
  const auto now = std::chrono::high_resolution_clock::now();
  const double elapsed_ms = std::chrono::duration<double, std::milli>(now - _start).count();
  return internal::convertUnit(elapsed_ms, TimeUnit::MilliSeconds, _UNIT);
}

[[nodiscard]] double Timer::_stopAndGetElapsed() noexcept {
  const double elapsed = _getTimeSinceStart();
  _is_running = false;
  _start = std::chrono::high_resolution_clock::now(); // reset start
  return elapsed;
}

[[nodiscard]] double Timer::_measureCallableTimeMS(const std::function<void()>& callable) noexcept {
  const auto t1 = std::chrono::high_resolution_clock::now();
  callable();
  const auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(t2 - t1).count();
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

HierarchyTimer::~HierarchyTimer() noexcept { if (_is_running) stop(); }

void HierarchyTimer::stop() noexcept {
  const double ELAPSED = _stopAndGetElapsed();
  std::cout << "\n} " << std::format("\033[32m[{:.3f} {}s]\033[0m\n", ELAPSED, internal::timeUnitPrefix(_UNIT));
}

void HierarchyTimer::subTask(std::string_view desc, const std::function<void()>& callable) noexcept {
  _subTaskImpl(desc, _measureCallableTimeMS(callable), _UNIT);
}

} /// namespace warp