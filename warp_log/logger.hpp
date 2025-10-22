#pragma once

#include "warp_log/misc.hpp"
#include "warp_log/tag.hpp"

#include <format>
#include <iterator>
#include <utility>
#include <vector>
#include <string>

namespace warp::log {

class Logger {
protected:
  std::string _context;

  template <typename... Args>
  void _log(Level lvl, std::format_string<Args...> msg, Args&&... args) const {
    if constexpr (sizeof...(Args) == 0) internal::writeToConsole(lvl, _context, msg.get());
    else {
      std::string& format_buffer = internal::tl_buf.format_buffer;
      format_buffer.clear();
      std::format_to(std::back_inserter(format_buffer), msg, std::forward<Args>(args)...);
      internal::writeToConsole(lvl, _context, format_buffer);
    }
  }

public:
  explicit Logger(Tag tag) noexcept : _context {std::move(tag)} {}

  explicit Logger(const std::vector<Tag>& tags) noexcept
  : _context {internal::cacheTagVec(tags)} {}

  /// Macro for creating log functions
  #define LOG_FN_IMPL(FN, LVL)  \
    template <typename... Args> \
    void FN(std::format_string<Args...> msg, Args&&... args) const { _log(LVL, msg, std::forward<Args>(args)...); }

  LOG_FN_IMPL(msg, Level::Message)
  LOG_FN_IMPL(info, Level::Info)
  LOG_FN_IMPL(warn, Level::Warn)
  LOG_FN_IMPL(err , Level::Error)
  #ifdef NDEBUG
    template <typename... Args>
    void dbg(std::format_string<Args...> msg, Args&&... args) const {}
  #else
    LOG_FN_IMPL(dbg , Level::Debug)
  #endif

  #undef LOG_FN_IMPL

};

} // namespace warp
