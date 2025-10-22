#pragma once

#include "misc.hpp"

namespace warp::log {

using Tag = std::string;

[[nodiscard]] inline Tag makeDefaultTag(std::string_view text) noexcept { return std::string{text}; }

/// A ANSI foreground color coded string
[[nodiscard]] inline Tag makeColoredTag(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

} // namespace warp::log