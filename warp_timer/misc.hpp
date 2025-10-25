#pragma once

#include "warp_log/misc.hpp"

#include <string>
#include <format>
#include <cstdint>

namespace warp::timer {

/// Enumeration of all supported time units
enum class TimeUnit : uint8_t { MicroSeconds, MilliSeconds, Seconds, };

} // namespace warp::timer

namespace warp::timer::internal {

/// --- warp::TimeUnit utils ---

inline constexpr int unitID(TimeUnit u) noexcept { return static_cast<int>(u); }

static inline constexpr double TABLE[3][3] {
  {1.0,       0.001,    0.000001},
  {1000.0,    1.0,      0.001   },
  {1'000'000, 1000.0,   1.0     },
};

template <TimeUnit FromTimeUnit = TimeUnit::MilliSeconds, TimeUnit ToTimeUnit>
inline constexpr double convertUnit(double value) noexcept {

  if constexpr (FromTimeUnit == ToTimeUnit) return value;
  return value * TABLE[unitID(FromTimeUnit)][unitID(ToTimeUnit)];
}

inline double convertUnit(double val, TimeUnit from_u, TimeUnit to_u) noexcept {
  if (from_u == to_u) return val;
  return val * TABLE[unitID(from_u)][unitID(to_u)];
}

inline constexpr char timeUnitPrefix(TimeUnit u) noexcept {
  const char PREFIX_CHAR[3] { 'u', 'm', '\x00' };
  return PREFIX_CHAR[unitID(u)];
}

/// Returns the formatted string of the given elapsed time
[[nodiscard]] inline std::string formatElapsed(double val, TimeUnit u) noexcept {
  return std::format(
    "{}[{:.3f} {}s]{}", log::setColor(log::ANSIFore::Yellow), val, timeUnitPrefix(u), log::resetColor()
  );
}

} // warp::timer::internal
