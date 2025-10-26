#pragma once

/// --- Configuration ---
/// Modify if needed

#define ENABLE_ANSI_COLOR_CODE (true)
#define ENABLE_TIMESTAMP       (true)

/// --- Includes ---

#include <cstdint>
#include <iostream>

#if ENABLE_TIMESTAMP
#include <chrono>
#endif

/// --- Utilities ---

enum LogLevel : uint8_t {
  L_TRACE,
  L_DEBUG,
  L_INFO,
  L_WARN,
  L_ERROR,
  L_FATAL,
};

namespace warp::mini {

static constexpr const char* LEVEL_STR[] {
  "[TRACE]",
  "[DEBUG]",
  "[INFO]",
  "[WARN]",
  "[ERROR]",
  "[FATAL]",
};

#if ENABLE_ANSI_COLOR_CODE
static constexpr const char* COLOR_TABLE[] {
  "\033[90m",
  "\033[36m",
  "\033[32m",
  "\033[33m",
  "\033[31m",
  "\033[41m",
};
#endif

[[nodiscard]] static constexpr inline std::string_view openLog(LogLevel level) noexcept {
#if ENABLE_ANSI_COLOR_CODE
  return COLOR_TABLE[level];
#else
  return "";
#endif
}

[[nodiscard]] static constexpr inline const char* closeLog() noexcept {
  return "\033[0m : ";
}

/// Automatically resets timer at the end of program
struct ResetTerminal {
  ~ResetTerminal() noexcept {
    std::cout << "\033[0m" << std::endl;
  }
};

static ResetTerminal s_reset_term {};

[[nodiscard]] static inline std::string_view getTimestamp() noexcept {
#if ENABLE_TIMESTAMP
  std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm tm_struct{};

  static char buf[sizeof("\n[HH:MM:SS]")] {};

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

} // namespace warp::mini

/// --- MACROS ---

#define WLOG(LVL) \
  (LVL < L_WARN ? std::cout : std::cerr) \
    << warp::mini::openLog(LVL) \
    << warp::mini::getTimestamp() \
    << warp::mini::LEVEL_STR[LVL] \
    << warp::mini::closeLog() \

#define WLOGT WLOG(L_TRACE)
#define WLOGD WLOG(L_DEBUG)
#define WLOGI WLOG(L_INFO)
#define WLOGW WLOG(L_WARN)
#define WLOGE WLOG(L_ERROR)
#define WLOGF WLOG(L_FATAL)
