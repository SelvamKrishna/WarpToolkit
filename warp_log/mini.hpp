#pragma once

#include <cstdint>

namespace warp::log {

enum class MiniLevel : uint8_t {
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

} // namespace warp::log
