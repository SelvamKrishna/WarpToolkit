#pragma once

#include "warp_log/misc.hpp"
#include "warp_log/tag.hpp"

#include <string>
#include <format>
#include <vector>
#include <utility>
#include <iterator>

namespace warp::log {

class Logger {
protected:
  std::string _ctx;

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    std::string& fmt_buffer {internal::tl_buf.fmt_buf};
    fmt_buffer.clear();
    std::format_to(std::back_inserter(fmt_buffer), msg, std::forward<Args>(args)...);
    internal::writeToConsole(lvl, _ctx, fmt_buffer);
  }

public:
  constexpr explicit Logger()                   noexcept = default;
  constexpr explicit Logger(Tag tag)            noexcept : _ctx {std::move(tag)} {}
  explicit Logger(const std::vector<Tag>& tags) noexcept : _ctx {std::move(internal::cacheTagVec(tags))} {}

#define LOG_FN_IMPL(FN, LVL) \
  template <typename... Args> \
  void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); } \
  void FN(std::string_view msg) const { internal::writeToConsole(LVL, _ctx, msg); }

  LOG_FN_IMPL(msg , Level::Message)
  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)

#ifdef NDEBUG // Logger::dbg() does nothing in release build
  template <typename... Args>
  constexpr void dbg(std::format_string<Args...>, Args&&...) const noexcept {}
  constexpr void dbg(std::string_view) const noexcept {}
#else
  LOG_FN_IMPL(dbg, Level::Debug)
#endif

#undef LOG_FN_IMPL
};

} // namespace warp::log
