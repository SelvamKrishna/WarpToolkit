#pragma once

#include <mutex>
#include <format>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

namespace warp::log {

enum class Level : uint8_t { Message, Info, Debug, Warn, Error };

enum class ANSIFore : uint8_t {
  Black = 30, Red, Green, Yellow, Blue, Magenta, Cyan, White,
  Reset = 39,
  LightBlack = 90, LightRed, LightGreen, LightYellow, LightBlue, LightMagenta, LightCyan, LightWhite
};

inline std::ostream& operator<<(std::ostream& os, ANSIFore fg) noexcept {
  return os << "\033[" << static_cast<int>(fg) << 'm';
}

[[nodiscard]] inline std::string setColor(ANSIFore fg) noexcept {
  return std::format("\033[{}m", static_cast<int>(fg));
}

[[nodiscard]] inline constexpr const char* resetColor() noexcept { return "\033[0m"; }

inline constexpr const char* BREAK_LINE = "---\n";

namespace internal {

static std::mutex s_console_mutex;

[[nodiscard]] inline constexpr std::string_view levelToString(Level lvl) noexcept {
  switch (lvl) {
    case Level::Info:  return "[INFO]";
    case Level::Debug: return "[DEBUG]";
    case Level::Warn:  return "[WARN]";
    case Level::Error: return "[ERROR]";
    default:           return "";
  }
}

[[nodiscard]] inline constexpr ANSIFore levelToColor(Level lvl) noexcept {
  switch (lvl) {
    case Level::Info:  return ANSIFore::Green;
    case Level::Debug: return ANSIFore::Cyan;
    case Level::Warn:  return ANSIFore::Yellow;
    case Level::Error: return ANSIFore::Red;
    default:           return ANSIFore::White;
  }
}

[[nodiscard]] inline std::ostream& streamFromLevel(Level lvl) noexcept {
  return (lvl == Level::Info || lvl == Level::Debug) ? std::cout : std::cerr;
}

struct ThreadLocalBuffer {
  std::string log_buf;
  std::string fmt_buf;

  static constexpr size_t DEFAULT_LOG_BUFFER_SIZE = 256;
  static constexpr size_t DEFAULT_FMT_BUFFER_SIZE = 128;

  explicit ThreadLocalBuffer() {
    log_buf.reserve(DEFAULT_LOG_BUFFER_SIZE);
    fmt_buf.reserve(DEFAULT_FMT_BUFFER_SIZE);
  }
};

inline thread_local ThreadLocalBuffer tl_buf {};

inline void writeToConsole(Level lvl, std::string_view pre, std::string_view msg) {
  std::string& log_buf = tl_buf.log_buf;
  log_buf.clear();
  log_buf.append(pre);

  const bool HAS_LEVEL = lvl != Level::Message;

  if (HAS_LEVEL) {
    if (!pre.empty()) log_buf.append(" : ");

    log_buf
      .append(setColor(levelToColor(lvl)))
      .append(levelToString(lvl))
      .append(resetColor());
  }

  if (HAS_LEVEL || !log_buf.empty()) log_buf.append(" : ");

  log_buf.append(msg).push_back('\n');

  std::ostream& os {streamFromLevel(lvl)};
  std::scoped_lock lock {s_console_mutex};
  os.write(log_buf.data(), static_cast<std::streamsize>(log_buf.size()));
  os.flush();
}

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

} // namespace internal

} // namespace warp::log
