#pragma once

#include "misc.hpp"

namespace warp::log {

using Tag = std::string;

[[nodiscard]] inline Tag makeDefaultTag(std::string_view text) noexcept { return std::string{text}; }

[[nodiscard]] inline Tag makeColoredTag(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

[[nodiscard]] inline constexpr size_t sizeOfColorWrap() noexcept { return sizeof("\033[FGm\033[0m"); }

} // namespace warp::log
