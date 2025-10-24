#pragma once

#include "logger.hpp"

#include <string>
#include <format>
#include <chrono>

namespace warp::log {

class TimedLogger final : public Logger {
private:
  ANSIFore                                      _timestamp_color  {ANSIFore::White};
  mutable std::string                           _cached_timestamp {""};
  mutable std::chrono::system_clock::time_point _last_update      {
    std::chrono::system_clock::time_point::min()
  };

  static constexpr std::chrono::seconds TIMESTAMP_CACHE_DURATION {1};

  [[nodiscard]] std::string _getTimestampTag() const noexcept {
    auto now = std::chrono::system_clock::now();

    if (now - _last_update > TIMESTAMP_CACHE_DURATION || _cached_timestamp.empty()) {
      char buf[sizeof("[HH:MM:SS]")] {};
      std::time_t t = std::chrono::system_clock::to_time_t(now);
      std::tm tm_struct{};
#ifdef _WIN32
      localtime_s(&tm_struct, &t);
#else
      localtime_r(&t, &tm_struct);
#endif
      std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
      _cached_timestamp = makeColoredTag(_timestamp_color, buf);
      _last_update = now;
    }

    return _cached_timestamp;
  }

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    std::string& format_buffer {internal::tl_buf.fmt_buf};
    format_buffer.clear();
    std::format_to(std::back_inserter(format_buffer), msg, std::forward<Args>(args)...);
    internal::writeToConsole(lvl, _getTimestampTag() + _ctx, format_buffer);
  }

public:
  constexpr explicit TimedLogger() noexcept = default;

  constexpr explicit TimedLogger(Tag tag, ANSIFore timestamp_color = ANSIFore::White) noexcept
  : Logger           {std::move(tag)}
  , _timestamp_color {timestamp_color} {}

  explicit TimedLogger(
    const std::vector<Tag>& tags,
    ANSIFore timestamp_color = ANSIFore::White
  ) noexcept
  : Logger           {std::move(internal::cacheTagVec(tags))}
  , _timestamp_color {timestamp_color} {}

#define LOG_FN_IMPL(FN, LVL)  \
  template <typename... Args> \
  void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); } \
  void FN(std::string_view msg) const { internal::writeToConsole(LVL, _getTimestampTag() + _ctx, msg); }

  LOG_FN_IMPL(msg, Level::Message)
  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)

#ifdef NDEBUG // Logger::dbg() does nothing in release build
  template <typename... Args>
  constexpr void dbg(std::format_string<Args...>, Args&&...) const noexcept {}
  constexpr void dbg(std::string_view) const noexcept {}
#else
  LOG_FN_IMPL(dbg , Level::Debug)
#endif

  constexpr void setTimestampColor(ANSIFore color) noexcept { _timestamp_color = color; }
  constexpr void refreshTimestamp() const noexcept { _last_update = std::chrono::system_clock::time_point::min(); }

#undef LOG_FN_IMPL
};

} // namespace warp::log
