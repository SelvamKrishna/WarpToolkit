#pragma once

/// --- Configuration ---

#define ENABLE_ANSI_COLOR_CODE (true)
#define ENABLE_TIMESTAMP       (false)

/// --- Includes ---

#include <cstdint>
#include <iostream>

#if ENABLE_TIMESTAMP
#include <chrono>
#endif

/// --- Utilities ---

namespace warp::log::mini {

enum MiniLogLevel : uint8_t {
  L_TRACE,
  L_DEBUG,
  L_INFO,
  L_WARN,
  L_ERROR,
  L_FATAL,
};

static constexpr const char* LEVEL_STR[] {
#if ENABLE_ANSI_COLOR_CODE
  "[TRACE] : ",
  "[DEBUG] : ",
  "[INFO]  : ",
  "[WARN]  : ",
  "[ERROR] : ",
  "[FATAL] : ",
#else
  "[TRACE] : ",
  "[DEBUG] : ",
  "[INFO]  : ",
  "[WARN]  : ",
  "[ERROR] : ",
  "[FATAL] : ",
#endif
};

[[nodiscard]] static inline std::string_view getTimestamp() noexcept {
#if ENABLE_TIMESTAMP
  std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm tm_struct{};

  static char buf[sizeof("\n[HH:MM:SS]") - 1] {};

#ifdef _WIN32
  localtime_s(&tm_struct, &t);
#else
  localtime_r(&t, &tm_struct);
#endif

  std::strftime(buf, sizeof(buf), "\n[%H:%M:%S]", &tm_struct);
  return buf;
#else
  return "\n";
#endif
}

} // namespace warp::log::mini

/// --- MACROS ---

#define WLOG(LVL) \
  (LVL < warp::log::mini::L_WARN ? std::cout : std::cerr) \
    << warp::log::mini::getTimestamp() \
    << warp::log::mini::LEVEL_STR[LVL] \

inline void test() {
  auto LVL = warp::log::mini::L_DEBUG;
  WLOG(warp::log::mini::L_DEBUG) << "Hello World";
}
