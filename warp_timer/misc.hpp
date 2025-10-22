#pragma once

#include "warp_log/misc.hpp"

#include <string>
#include <format>
#include <cstdint>

namespace warp {

enum class TimeUnit : uint8_t { MicroSeconds, MilliSeconds, Seconds, };

namespace internal {

inline constexpr int unitID(TimeUnit u) noexcept { return static_cast<int>(u); }

static inline constexpr double TABLE[3][3] {
  {1.0,       0.001,    0.000001},
  {1000.0,    1.0,      0.001   },
  {1'000'000, 1000.0,   1.0     },
};

template <TimeUnit Source = TimeUnit::MilliSeconds, TimeUnit Target>
inline constexpr double convertUnit(double value) noexcept {

  if constexpr (Source == Target) return value;
  return value * TABLE[unitID(Source)][unitID(Target)];
}

inline double convertUnit(double value, TimeUnit source, TimeUnit target) noexcept {
  if (source == target) return value;
  return value * TABLE[unitID(source)][unitID(target)];
}

inline constexpr char timeUnitPrefix(TimeUnit u) noexcept {
  const char PREFIX_CHAR[3] { 'u', 'm', '\x00' };
  return PREFIX_CHAR[unitID(u)];
}

inline std::string formatElapsed(double value, TimeUnit u) noexcept {
  return std::format(
    "{}[{:.3f} {}s]{}", log::setColor(log::ANSIFore::Yellow), value, timeUnitPrefix(u), log::resetColor()
  );
}

} // namespace internal

} // namespace warp