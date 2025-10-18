#pragma once
#ifndef WARP_LOG
#define WARP_LOG

#ifndef WARP_TOOLKIT_API
  #ifdef _WIN32
    #define WARP_TOOLKIT_API __declspec(dllexport)
  #else
    #define WARP_TOOLKIT_API
  #endif
#endif

#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace warp::log {

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

/// A string adding context to the message
using Tag = std::string;

namespace tag {

/// Create a simple tag (just text)
[[nodiscard]] inline Tag makeDefault(std::string_view text) noexcept { return std::string{text}; }

/// Create a colored tag using ANSI codes
[[nodiscard]] inline Tag makeColored(ANSIFore fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

} // namespace tag

namespace internal {

WARP_TOOLKIT_API void writeToConsole(Level lvl, std::string_view prefix, std::string_view msg);

/// Concatenate multiple tags into a single string (with optional delimiter)
WARP_TOOLKIT_API std::string cacheTagVec(const std::vector<Tag>& tags, std::string_view delim = "");

/// Returns std::cout for Info/Debug and std::cerr for Warn/Error
[[nodiscard]] inline constexpr std::ostream& streamFromLevel(Level lvl) noexcept {
  return (lvl == Level::Info || lvl == Level::Debug) ? std::cout : std::cerr;
}

} // namespace internal

/// Logging sender class
class WARP_TOOLKIT_API Sender final {
private:
  std::string _context;
  bool _log_timestamp;
  ANSIFore _time_stamp_color;

  [[nodiscard]] std::string _getTimestamp() const noexcept;

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(
      lvl,
      _log_timestamp ? tag::makeColored(_time_stamp_color, _getTimestamp()) + _context : _context,
      std::format(msg, std::forward<Args>(args)...)
    );
  }

public:
  Sender() = delete;

  explicit Sender(Tag tag, bool log_timestamp = false) noexcept
  : _context {std::move(tag)}, _log_timestamp {log_timestamp}, _time_stamp_color {ANSIFore::White} {}

  explicit Sender(const std::vector<Tag>& tags, bool log_timestamp = false) noexcept
  : _context {internal::cacheTagVec(tags)}, _log_timestamp {log_timestamp}, _time_stamp_color {ANSIFore::White} {}

  /// Macro to define logging functions for each level
  #define LOG_FN_IMPL(fn_name, log_lvl) \
    template <typename... Args>         \
    void fn_name(std::format_string<Args...> msg, Args&&... args) const { _log(log_lvl, msg, std::forward<Args>(args)...); }
  
  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(dbg, Level::Debug)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(error, Level::Error)

  #undef LOG_HELPER

  constexpr void logTimestamp(bool flag) noexcept { _log_timestamp = flag; }
  constexpr void setTimestampColor(ANSIFore color) noexcept { _time_stamp_color = color; }
};

} // namespace warp::log

#endif // WARP_LOG