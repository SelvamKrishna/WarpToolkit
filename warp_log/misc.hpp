#pragma once

#include <mutex>
#include <format>
#include <string>
#include <cstdint>
#include <iostream>

namespace warp::log {

/// Levels of logging
enum class Level : uint8_t { Message, Info, Debug, Warn, Error };

/// ANSI codes for colored console output
enum class ANSIFore : uint8_t {
  Black = 30, Red, Green, Yellow, Blue, Magenta, Cyan, White,
  Reset = 39,
  LightBlack = 90, LightRed, LightGreen, LightYellow, LightBlue, LightMagenta, LightCyan, LightWhite
};

/// --- Console logging utils ---

inline std::ostream& operator<<(std::ostream& os, ANSIFore fg) noexcept { return os << "\033[" << static_cast<int>(fg) << 'm'; }

[[nodiscard]] inline std::string setColor(ANSIFore fg) noexcept { return std::format("\033[{}m", static_cast<int>(fg)); }
[[nodiscard]] inline constexpr const char* resetColor() noexcept { return "\033[0m"; }

inline constexpr const char* BREAK_LINE = "---\n";

} // namespace warp::log

namespace warp::log::internal {

/// --- warp::log::Level utils ---

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

/// Pre-allocated string buffer for performance boost
struct ThreadLocalBuffer {
  std::string log_buf;
  std::string fmt_buf;

/// Change below constants if needed
  static constexpr size_t DEFAULT_LOG_BUFFER_SIZE = 256;
  static constexpr size_t DEFAULT_FMT_BUFFER_SIZE = 128;

  explicit ThreadLocalBuffer() {
    log_buf.reserve(DEFAULT_LOG_BUFFER_SIZE);
    fmt_buf.reserve(DEFAULT_FMT_BUFFER_SIZE);
  }
};

static std::mutex s_console_mutex {};
inline thread_local ThreadLocalBuffer tl_buf {};

/// Logs to console with added level prefix
inline void writeToConsole(Level lvl, std::string_view pre, std::string_view msg) {
  std::string& log_buf = tl_buf.log_buf; // uses pre allocated buffer for performance
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

} // namespace warp::log::internal
