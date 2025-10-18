#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>

#include "warp_timer.hpp"

namespace warp {

// --- timer ---

void timer::_log(std::string_view desc, double elapsed, time_unit time_unit) noexcept {
  std::cout
    << std::format(
      "\033[34m[TIMER]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n"
      , elapsed
      , internal::time_unit_prefix(time_unit)
      , desc
    )
  ;
}

void timer::_log_benchmark(std::string_view desc, std::vector<double> results, time_unit time_unit) noexcept {
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

  const char U = internal::time_unit_prefix(time_unit);
  std::cout << std::format(
    ""
    "\033[34m[TIMER][BENCHMARK]\033[0m : {}\n"
    "\t\033[32m[MEAN]\033[0m   : {:.3f} {}s\n"
    "\t\033[32m[MEDIAN]\033[0m : {:.3f} {}s\n"
    "\t\033[32m[MODE]\033[0m   : {:.3f} {}s\n",
    desc, MEAN, U, MEDIAN, U, MODE, U
);
}

[[nodiscard]] double timer::_get_time_since_start() const noexcept {
  const auto now = std::chrono::high_resolution_clock::now();
  const double elapsed_ms = std::chrono::duration<double, std::milli>(now - _start).count();
  return internal::convert_units(elapsed_ms, time_unit::MILLI_SECONDS, _TIME_UNIT);
}

[[nodiscard]] double timer::_stop_and_get_elapsed() noexcept {
  const double elapsed = _get_time_since_start();
  _is_running = false;
  _start = std::chrono::high_resolution_clock::now(); // reset start
  return elapsed;
}

[[nodiscard]] double timer::_measure_callable_time_ms(const std::function<void()>& callable) noexcept {
  const auto t1 = std::chrono::high_resolution_clock::now();
  callable();
  const auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(t2 - t1).count();
}

timer::~timer() noexcept { if (_is_running) stop(); }

void timer::start() noexcept {
  _is_running = true;
  _start = std::chrono::high_resolution_clock::now();
}

void timer::stop() noexcept {
  const double ELAPSED = _stop_and_get_elapsed();
  _log(_DESC, ELAPSED, _TIME_UNIT);
}

// --- hierarchy_timer ---

void hierarchy_timer::_log_timer_start() const noexcept {
  std::cout << "\033[34m[TIMER]\033[0m : " << _DESC << " {\n\n";
}

void hierarchy_timer::_sub_task_impl(
  std::string_view desc,
  double elapsed_ms,
  time_unit display_unit
) noexcept {
  _sub_task_measure += internal::convert_units(elapsed_ms, time_unit::MILLI_SECONDS, _TIME_UNIT);

  std::cout
    << "\033[34m[TASK]\033[0m "
    << internal::format_elapsed(
      internal::convert_units(elapsed_ms, time_unit::MILLI_SECONDS, display_unit),
      display_unit
    )
    << " : " << desc << '\n';
}

hierarchy_timer::~hierarchy_timer() noexcept {
  if (_is_running) stop();
}

void hierarchy_timer::stop() noexcept {
  const double ELAPSED = _stop_and_get_elapsed();
  std::cout << "\n} " << std::format("\033[32m[{:.3f} {}s]\033[0m\n", ELAPSED, internal::time_unit_prefix(_TIME_UNIT));
}

void hierarchy_timer::sub_task(std::string_view desc, const std::function<void()>& callable) noexcept {
  _sub_task_impl(desc, _measure_callable_time_ms(callable), _TIME_UNIT);
}

} /// namespace warp