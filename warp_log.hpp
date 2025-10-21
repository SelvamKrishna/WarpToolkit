#pragma once
#ifndef WARP_LOG
#define WARP_LOG

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
enum class Level : uint8_t { Info, Debug, Warn, Error };

/// ANSI foreground colors for terminal output
enum class ANSIFore : uint8_t {
  Black = 30, Red, Green, Yellow, Blue, Magenta, Cyan, White,
  Reset = 39,
  LightBlack = 90, LightRed, LightGreen, LightYellow, LightBlue, LightMagenta, LightCyan, LightWhite
};

using LogTag = std::string;

namespace tag_factory {

[[nodiscard]] static inline LogTag makeDefault(std::string_view text) noexcept { return std::string{text}; }

[[nodiscard]] static inline LogTag makeColored(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

} // namespace tag_factory

namespace internal {

static std::mutex s_console_mutex;

inline std::string levelToString(Level lvl) noexcept {
  switch (lvl) {
    case Level::Info:  return "INFO";
    case Level::Debug: return "DEBUG";
    case Level::Warn:  return "WARN";
    case Level::Error: return "ERROR";
  }

  return "UNKNOWN";
}

inline ANSIFore levelToColor(Level lvl) noexcept {
  switch (lvl) {
    case Level::Info:  return ANSIFore::Green;
    case Level::Debug: return ANSIFore::Cyan;
    case Level::Warn:  return ANSIFore::Yellow;
    case Level::Error: return ANSIFore::Red;
  }

  return ANSIFore::White;
}

[[nodiscard]] inline std::ostream& streamFromLevel(Level lvl) noexcept {
  return (lvl == Level::Info || lvl == Level::Debug) ? std::cout : std::cerr;
}

inline void writeToConsole(Level lvl, std::string_view prefix, std::string_view msg) {
  std::scoped_lock lock{s_console_mutex};
  std::ostream& os = streamFromLevel(lvl);

  os << prefix
    << "\033[" << static_cast<int>(levelToColor(lvl)) << "m[" << levelToString(lvl) << "]\033[0m : "
    << msg << '\n';
  os.flush();
}

[[nodiscard]] inline std::string cacheTagVec(const std::vector<LogTag>& tags, std::string_view delim = "") {
  if (tags.empty()) return {};

  std::string result {tags[0]};
  for (size_t i = 1; i < tags.size(); ++i) result.append(delim).append(tags[i]);
  return result;
}

} // namespace internal

/// Provides contextual logging support with tags
class Sender final {
private:
  std::string _context;
  bool        _allow_log_timestamp;
  ANSIFore    _time_stamp_color;

  [[nodiscard]] std::string _getTimestampTag() const noexcept {
    char buf[sizeof("[HH:MM:SS]")] {};
    std::time_t t {std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    std::tm tm_struct {};

    #ifdef _WIN32
      localtime_s(&tm_struct, &t);
    #else
      localtime_r(&t, &tm_struct);
    #endif

    std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
    return std::string{buf};
  }

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    std::string full_prefix {
      _allow_log_timestamp
        ? tag_factory::makeColored(_time_stamp_color, _getTimestampTag()) + _context
        : _context
    };

    internal::writeToConsole(lvl, full_prefix, std::format(msg, std::forward<Args>(args)...));
  }

public:
  Sender() = delete;

  explicit Sender(LogTag tag, bool log_timestamp = false) noexcept
  : _context             {std::move(tag)}
  , _allow_log_timestamp {log_timestamp}
  , _time_stamp_color    {ANSIFore::White} {}

  explicit Sender(const std::vector<LogTag>& tags, bool log_timestamp = false) noexcept
  : _context             {internal::cacheTagVec(tags)}
  , _allow_log_timestamp {log_timestamp}
  , _time_stamp_color    {ANSIFore::White} {}

  /// Macro for creating log functions
  #define LOG_FN_IMPL(FN, LVL)  \
    template <typename... Args> \
    void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(dbg , Level::Debug)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)

  #undef LOG_FN_IMPL

  constexpr void allowTimestampLogging(bool flag) noexcept { _allow_log_timestamp = flag; }
  constexpr void setTimestampColor(ANSIFore color) noexcept { _time_stamp_color = color; }
};

} // namespace warp

#endif // WARP_LOG