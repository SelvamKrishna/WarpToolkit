#pragma once

#include "logger.hpp"

namespace warp::log {

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
    return makeColoredTag(_time_stamp_color, buf);
  }

  template <typename... Args>
  void _logTimed(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    internal::writeToConsole(lvl, _getTimestampTag() + _context, std::format(msg, std::forward<Args>(args)...));
  }

public:
  TimedLogger() = delete;

  explicit TimedLogger(Tag tag) noexcept
  : Logger            {std::move(tag)}
  , _time_stamp_color {ANSIFore::White} {}

  explicit TimedLogger(const std::vector<Tag>& tags) noexcept
  : Logger            {internal::cacheTagVec(tags)}
  , _time_stamp_color {ANSIFore::White} {}

  /// Macro for creating log functions
  #define LOG_FN_IMPL(FN, LVL)  \
    template <typename... Args> \
    void FN(std::format_string<Args...> msg, Args&&... args) const { _logTimed(LVL, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(msg, Level::Message)
  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)
  #ifdef NDEBG
    template <typename... Args>
    void dbg(std::format_string<Args...> msg, Args&&... args) const {}
  #else
    LOG_FN_IMPL(dbg , Level::Debug)
  #endif

  #undef LOG_FN_IMPL

  constexpr void setTimestampColor(ANSIFore color) noexcept { _time_stamp_color = color; }
};

} // namespace warp::log
