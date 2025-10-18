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

enum class level : uint8_t { INFO, DEBUG, WARN, ERROR };

enum class ansi_fg_color : uint8_t {
  BLACK = 30,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  RESET = 39,
  LIGHT_BLACK_EX = 90,
  LIGHT_RED_EX,
  LIGHT_GREEN_EX,
  LIGHT_YELLOW_EX,
  LIGHT_BLUE_EX,
  LIGHT_MAGENTA_EX,
  LIGHT_CYAN_EX,
  LIGHT_WHITE_EX
};

using tag = std::string;

[[nodiscard]] inline tag make_default_tag(std::string_view text) noexcept { return std::string{text}; }

[[nodiscard]] inline tag make_colored_tag(ansi_fg_color fg, std::string_view text) noexcept {
  return std::format("\033[{}m{}\033[0m", static_cast<int>(fg), text);
}

namespace internal {

WARP_TOOLKIT_API void write_to_console(level lvl, std::string_view prefix, std::string_view msg);
WARP_TOOLKIT_API std::string cache_tag_vec(const std::vector<tag>& tags, std::string_view delim = "");

[[nodiscard]] inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
  return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

} // namespace internal

class WARP_TOOLKIT_API sender final {
private:
  std::string _context;
  bool _log_timestamp;
  ansi_fg_color _time_stamp_color;

  [[nodiscard]] std::string _get_timestamp() const noexcept;

  template <typename... Args>
  void _log(level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::write_to_console(
      lvl,
      _log_timestamp
        ? make_colored_tag(_time_stamp_color, _get_timestamp()) + _context
        : _context,
      std::format(msg, std::forward<Args>(args)...)
    );
  }

public:
  sender() = delete;

  explicit sender(tag t, bool log_timestamp = false) noexcept
  : _context(std::move(t)), _log_timestamp(log_timestamp), _time_stamp_color(ansi_fg_color::WHITE) {}

  explicit sender(const std::vector<tag>& tags, bool log_timestamp = false) noexcept
  : _context(internal::cache_tag_vec(tags)), _log_timestamp(log_timestamp), _time_stamp_color(ansi_fg_color::WHITE) {}

  #define LOG_FN_IMPL(fn_name, log_lvl) \
    template <typename... Args>         \
    void fn_name(std::format_string<Args...> msg, Args&&... args) const { _log(log_lvl, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(info, level::INFO)
  LOG_FN_IMPL(dbg, level::DEBUG)
  LOG_FN_IMPL(warn, level::WARN)
  LOG_FN_IMPL(error, level::ERROR)

  #undef LOG_HELPER

  constexpr void log_timestamp(bool flag) noexcept { _log_timestamp = flag; }
  constexpr void set_timestamp_color(ansi_fg_color color) noexcept { _time_stamp_color = color; }
};

} // namespace warp::log

#endif // WARP_LOG