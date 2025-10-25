#pragma once

#include "misc.hpp"

#include <string>
#include <format>
#include <vector>
#include <string_view>

namespace warp::log {

using Tag = std::string;

/// --- Tag factory ---

[[nodiscard]] inline Tag makeDefaultTag(std::string_view text) noexcept { return std::string{text}; }

[[nodiscard]] inline Tag makeColoredTag(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

} // namespace warp::log

namespace warp::log::internal {

/// --- Tag utils ---

[[nodiscard]] inline constexpr size_t sizeOfColorWrap() noexcept { return sizeof("\033[FGm\033[0m") - 1; }

/// Joins multiple tags into a single string
[[nodiscard]] inline std::string cacheTagVec(const std::vector<std::string>& tags, std::string_view delim = "") {
  if (tags.empty()) return {};

  size_t total_size {delim.size() * (tags.size() - 1)};
  for (const auto& TAG : tags) total_size += TAG.size();

  std::string result;

  result.reserve(total_size);
  result.append(tags[0]);
  for (size_t i = 1; i < tags.size(); ++i) result.append(delim).append(tags[i]);

  return result;
}

} // namespace warp::log::internal
