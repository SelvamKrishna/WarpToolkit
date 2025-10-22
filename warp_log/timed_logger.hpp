#pragma once

#include "logger.hpp"

namespace warp::log {

/// Provides contextual logging support with tags and timestamps
class TimedLogger final : public Logger {
private:
  ANSIFore _timestamp_color;

  /// Performance improvement
  mutable std::string _cached_time_stamp {"\033[FGm[HH:MM:SS]\033[0m"};
  mutable std::chrono::system_clock::time_point _last_timestamp_update;
  static constexpr std::chrono::seconds TIMESTAMP_CACHE_DURATION {1};

  [[nodiscard]] std::string _getTimestampTag() const noexcept {
    auto now = std::chrono::system_clock::now();

    if (now - _last_timestamp_update > TIMESTAMP_CACHE_DURATION) {
      char buf[sizeof("[HH:MM:SS]")] {};
      std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::tm tm_struct{};
      #ifdef _WIN32
        localtime_s(&tm_struct, &t);
      #else
        localtime_r(&t, &tm_struct);
      #endif
      std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
      _cached_time_stamp = makeColoredTag(_timestamp_color, buf);
    }

    return _cached_time_stamp;
  }

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    const std::string PREFIX = _getTimestampTag() + _context; // Can be improved

    if constexpr (sizeof...(Args) == 0) internal::writeToConsole(lvl, PREFIX, msg.get());
    else {
      std::string& format_buffer {internal::tl_buf.format_buffer};
      format_buffer.clear();
      std::format_to(std::back_inserter(format_buffer), msg, std::forward<Args>(args)...);
      internal::writeToConsole(lvl, PREFIX, format_buffer);
    }
  }

public:
  TimedLogger() = delete;

  explicit TimedLogger(Tag tag, ANSIFore timestamp_color = ANSIFore::White) noexcept
  : Logger                 {std::move(tag)}
  , _timestamp_color       {timestamp_color}
  , _last_timestamp_update {std::chrono::system_clock::time_point::min()} {}

  explicit TimedLogger(const std::vector<Tag>& tags, ANSIFore timestamp_color = ANSIFore::White) noexcept
  : Logger                 {internal::cacheTagVec(tags)}
  , _timestamp_color       {timestamp_color}
  , _last_timestamp_update {std::chrono::system_clock::time_point::min()} {}

  /// Macro for creating log functions
  #define LOG_FN_IMPL(FN, LVL)  \
    template <typename... Args> \
    void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); }

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

  constexpr void setTimestampColor(ANSIFore color) noexcept { _timestamp_color = color; }

  void refreshTimestamp() const {
    _last_timestamp_update = std::chrono::system_clock::time_point::min();
  }
};

} // namespace warp::log
