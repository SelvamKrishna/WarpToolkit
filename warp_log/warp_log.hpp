#pragma once
#ifndef WARP_LOG
#define WARP_LOG

#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

namespace warp {

/// Logging levels
enum class Level : uint8_t { Info, Debug, Warn, Error };

/// ANSI foreground colors for terminal output
enum class ANSIFore : uint8_t {
  Black = 30,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
  Reset = 39,
  LightBlack = 90,
  LightRed,
  LightGreen,
  LightYellow,
  LightBlue,
  LightMagenta,
  LightCyan,
  LightWhite
};

using LogTag = std::string;

namespace tag_factory {

  /// Create a simple tag (just text)
  [[nodiscard]] static inline LogTag makeDefault(std::string_view text) noexcept { return std::string{text}; }

  /// Create a colored tag using ANSI codes
  [[nodiscard]] static inline LogTag makeColored(ANSIFore fg, std::string_view text) noexcept {
    return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
  }

};

namespace internal {

static std::mutex g_console_mutex;

[[nodiscard]] inline std::ostream& streamFromLevel(Level lvl) noexcept {
  return (lvl == Level::Info || lvl == Level::Debug) ? std::cout : std::cerr;
}

inline void writeToConsole(Level lvl, std::string_view prefix, std::string_view msg) {
  std::scoped_lock lock{g_console_mutex};
  std::ostream& os = streamFromLevel(lvl);

  os.write(prefix.data(), prefix.size());
  os.write("\033[", 2); // ESC prefix

  switch (lvl) {
    case Level::Info:  os.write("32m[INFO]", 9); break;   // Green
    case Level::Debug: os.write("36m[DEBUG]", 10); break; // Cyan
    case Level::Warn:  os.write("33m[WARN]", 9); break;   // Yellow
    case Level::Error: os.write("31m[ERROR]", 10); break; // Red
  }

  os.write("\033[0m : ", 7);
  os.write(msg.data(), msg.size());
  os.put('\n');
  os.flush();
}

inline std::string cacheTagVec(const std::vector<LogTag>& tags, std::string_view delim = "") {
  if (tags.empty()) return {};

  std::string result = tags[0];
  for (size_t i = 1; i < tags.size(); ++i) result.append(delim).append(tags[i]);

  return result;
}

} // namespace internal

class Sender final {
private:
  std::string _context;
  bool _allow_log_timestamp;
  ANSIFore _time_stamp_color;

  [[nodiscard]] std::string _getTimestamp() const noexcept {
    char buf[sizeof("[HH:MM:SS]")]{};
    const std::time_t TIME = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_struct{};

    #ifdef _WIN32
    localtime_s(&tm_struct, &TIME);
    #else
    localtime_r(&TIME, &tm_struct);
    #endif

    std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
    return std::string{buf};
  }

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(
      lvl,
      (_allow_log_timestamp)
        ? tag_factory::makeColored(_time_stamp_color, _getTimestamp()) + _context
        : _context,
      std::format(msg, std::forward<Args>(args)...)
    );
  }

public:
  Sender() = delete;

  explicit Sender(LogTag tag, bool log_timestamp = false) noexcept
  : _context {std::move(tag)}, _allow_log_timestamp {log_timestamp}, _time_stamp_color {ANSIFore::White} {}

  explicit Sender(const std::vector<LogTag>& tags, bool log_timestamp = false) noexcept
  : _context {internal::cacheTagVec(tags)}, _allow_log_timestamp {log_timestamp}, _time_stamp_color {ANSIFore::White} {}

  #define LOG_FN_IMPL(fn_name, log_lvl) \
    template <typename... Args>         \
    void fn_name(std::format_string<Args...> msg, Args&&... args) const { _log(log_lvl, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(dbg, Level::Debug)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err, Level::Error)

  #undef LOG_HELPER

  constexpr void allowTimestampLogging(bool flag) noexcept { _allow_log_timestamp = flag; }
  constexpr void setTimestampColor(ANSIFore color) noexcept { _time_stamp_color = color; }
};

} // namespace warp

#endif // WARP_LOG