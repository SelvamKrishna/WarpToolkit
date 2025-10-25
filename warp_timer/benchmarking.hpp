#pragma once

#include "misc.hpp"

#include "warp_log/tag.hpp"
#include "warp_log/Logger.hpp"

#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>

namespace warp::timer::internal {

/// --- Benchmarking utils ---

[[nodiscard]] inline double measureCallableTimeMS(const std::function<void()>& callable) noexcept {
  const auto START {std::chrono::high_resolution_clock::now()};
  callable();
  const auto STOP {std::chrono::high_resolution_clock::now()};
  return std::chrono::duration<double, std::milli>(STOP - START).count();
}

[[nodiscard]] inline std::pair<double, double> getMeanAndMedian(const std::vector<double> sorted_ls) noexcept {
  const size_t SIZE {sorted_ls.size()};

  return std::make_pair(
    std::accumulate(sorted_ls.begin(), sorted_ls.end(), 0.0) / SIZE,
    (SIZE % 2 == 0) ? (sorted_ls[SIZE / 2 - 1] + sorted_ls[SIZE / 2]) / 2.0 : sorted_ls[SIZE / 2]
  );
}

inline void logElapsed(std::string_view desc, double elapsed, TimeUnit unit) noexcept {
  log::Logger {
    log::makeColoredTag(log::ANSIFore::Blue, "[TIMER]")
  } .msg("{} : {}\n", formatElapsed(elapsed, unit), desc);
}

inline void logBenchmark(std::string_view desc, std::vector<double>& results, TimeUnit time_unit) noexcept {
  static const log::Logger BENCHMARK_LOG {makeColoredTag(log::ANSIFore::Blue, "[TIMER][BENCHMARK]")};

  if (results.empty()) [[unlikely]] {
    BENCHMARK_LOG.warn("Trying to benchmark empty results");
    return;
  }

  std::sort(results.begin(), results.end());
  const auto [MEAN, MEDIAN] {getMeanAndMedian(results)};

  BENCHMARK_LOG.msg(
    "{}\n"
    "\t{}: {}\n"
    "\t{}: {}\n",
    desc,
    log::makeColoredTag(log::ANSIFore::Green, "[MEAN]   "), formatElapsed(MEAN, time_unit),
    log::makeColoredTag(log::ANSIFore::Green, "[MEDIAN] "), formatElapsed(MEDIAN, time_unit)
  );
}

} // namespace warp::timer::internal

namespace warp::timer {

/// --- Benchmarking tools ---

/// Measures the total time taken to execute the given callable function
template <TimeUnit InTimeUnit = TimeUnit::MilliSeconds>
inline double measure(std::string_view desc, const std::function<void()>& callable) noexcept {
  const double ELAPSED {
    internal::convertUnit<TimeUnit::MilliSeconds, InTimeUnit>(internal::measureCallableTimeMS(callable))
  };
  internal::logElapsed(desc, ELAPSED, InTimeUnit);
  return ELAPSED;
}

/// Benchmarks the execution time of the given callable function
template <TimeUnit InTimeUnit = TimeUnit::MilliSeconds>
inline void benchmark(std::string_view desc, const std::function<void()>& callable, uint32_t samples = 8) noexcept {
  std::vector<double> results;
  results.reserve(samples);

  while (samples--)
    results.push_back(internal::convertUnit<TimeUnit::MilliSeconds, InTimeUnit>(internal::measureCallableTimeMS(callable)));

  internal::logBenchmark(desc, results, InTimeUnit);
}

} // namespace warp::timer
