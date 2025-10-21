#pragma once

#include <ctime>
#include <mutex>
#include <chrono>
#include <format>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace warp {

/// Logging levels
enum class Level : uint8_t { Message, Info, Debug, Warn, Error };

/// ANSI foreground colors for terminal output
enum class ANSIFore : uint8_t {
  Black = 30, Red, Green, Yellow, Blue, Magenta, Cyan, White,
  Reset = 39,
  LightBlack = 90, LightRed, LightGreen, LightYellow, LightBlue, LightMagenta, LightCyan, LightWhite
};

[[nodiscard]] inline std::string setColor(ANSIFore fg) noexcept {
  return std::format("\033[{}m", static_cast<int>(fg));
}

[[nodiscard]] inline constexpr const char* resetColor() noexcept { return "\033[0m"; }

using LogTag = std::string;

namespace tag_factory {

[[nodiscard]] inline LogTag makeDefault(std::string_view text) noexcept { return std::string{text}; }

[[nodiscard]] inline LogTag makeColored(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

} // namespace tag_factory

namespace internal {

static std::mutex s_console_mutex;

[[nodiscard]] inline std::string levelToString(Level lvl) noexcept {
  switch (lvl) {
    case Level::Info:  return "[INFO]";
    case Level::Debug: return "[DEBUG]";
    case Level::Warn:  return "[WARN]";
    case Level::Error: return "[ERROR]";
    default:           return "";
  }
}

[[nodiscard]] inline ANSIFore levelToColor(Level lvl) noexcept {
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

[[nodiscard]] inline std::string formatLogPrefix(Level lvl, std::string_view prefix) {
  return std::format(
    "{}\033[{}m{}\033[0m : ", prefix, static_cast<int>(levelToColor(lvl)), levelToString(lvl).c_str()
  );
}

inline void writeToConsole(Level lvl, std::string_view prefix, std::string_view msg) {
  std::scoped_lock lock{s_console_mutex};
  std::ostream& os = streamFromLevel(lvl);

  static constexpr size_t BUFFER_SIZE = 256;
  static thread_local std::string s_log_buffer {};
  s_log_buffer.clear();

  if (s_log_buffer.size() < BUFFER_SIZE) [[unlikely]] s_log_buffer.reserve(256);

  s_log_buffer.append(formatLogPrefix(lvl, prefix));
  s_log_buffer.append(msg);
  s_log_buffer.push_back('\n');

  os.write(s_log_buffer.data(), static_cast<std::streamsize>(s_log_buffer.size()));
  os.flush();
}

[[nodiscard]] inline std::string cacheTagVec(const std::vector<LogTag>& tags, std::string_view delim = "") {
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

class Logger {
private:
  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(lvl, _context, std::format(msg, std::forward<Args>(args)...));
  }

protected:
  std::string _context;

public:
  explicit Logger(LogTag tag) noexcept
  : _context {std::move(tag)} {}

  explicit Logger(const std::vector<LogTag>& tags) noexcept
  : _context {internal::cacheTagVec(tags)} {}

  /// Macro for creating log functions
  #define LOG_FN_IMPL(FN, LVL)  \
    template <typename... Args> \
    void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(dbg , Level::Debug)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)

  #undef LOG_FN_IMPL

};

/// Provides contextual logging support with tags
class TimedLogger final : public Logger {
private:
  ANSIFore    _time_stamp_color;

  [[nodiscard]] std::string _getTimestampTag() const noexcept {
    char buf[sizeof("[HH:MM:SS]")] {};
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_struct{};
    #ifdef _WIN32
      localtime_s(&tm_struct, &t);
    #else
      localtime_r(&t, &tm_struct);
    #endif
    std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
    return tag_factory::makeColored(_time_stamp_color, buf);
  }

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(lvl, _getTimestampTag() + _context, std::format(msg, std::forward<Args>(args)...));
  }

public:
  TimedLogger() = delete;

  explicit TimedLogger(LogTag tag) noexcept
  : Logger            {std::move(tag)}
  , _time_stamp_color {ANSIFore::White} {}

  explicit TimedLogger(const std::vector<LogTag>& tags) noexcept
  : Logger            {internal::cacheTagVec(tags)}
  , _time_stamp_color {ANSIFore::White} {}

  /// General message without level
  template <typename... Args>
  void msg(std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(Level::Message, _getTimestampTag() + _context, std::format(msg, std::forward<Args>(args)...));
  }

  constexpr void setTimestampColor(ANSIFore color) noexcept { _time_stamp_color = color; }
};

} // namespace warp