#pragma once

/// --- Config ---

#define ENABLE_COLOR_CODE true
#define ENABLE_TIMESTAMP  true

/// --- Includes ---

#include <cstdint>
#include <iostream>

#if ENABLE_TIMESTAMP
#include <chrono>
#endif

/// --- Utilities ---

/// Supported log levels
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

#if ENABLE_COLOR_CODE
static constexpr const char* COLOR_TABLE[] {
  "\033[90m",
  "\033[36m",
  "\033[32m",
  "\033[33m",
  "\033[31m",
  "\033[41m",
};
#endif

[[nodiscard]] static constexpr inline std::string_view openColor(LogLevel level) noexcept {
#if ENABLE_COLOR_CODE
  return COLOR_TABLE[level];
#else
  return "";
#endif
}

[[nodiscard]] static constexpr inline const char* closeColor() noexcept {
#if ENABLE_COLOR_CODE
  return "\033[0m : ";
#else
  return " : ";
#endif
}

[[nodiscard]] static inline std::string colorText(int color_code, const char* text) noexcept {
#if ENABLE_COLOR_CODE
  return std::format("\033[{}m{}\033[0m", color_code, text);
#else
  return text;
#endif
}

[[nodiscard]] static inline std::string_view getTimestamp() noexcept {
#if ENABLE_TIMESTAMP
  std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm tm_struct{};

  static char buf[sizeof("[HH:MM:SS]")] {};

#ifdef _WIN32
  localtime_s(&tm_struct, &t);
#else
  localtime_r(&t, &tm_struct);
#endif

  std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
  return buf;
#else
  return "";
#endif
}

/// Automatically resets terminal at the end of program
struct ResetTerminal {
  ~ResetTerminal() noexcept {
    std::cout << "\033[0m" << std::endl;
  }
};

static ResetTerminal s_reset_term {};

} // namespace warp::mini

/// --- MACROS ---

/// `os <<`

#define WLOG(LVL)                         \
  (LVL < L_WARN ? std::cout : std::cerr)  \
    << "\n"                               \
    << warp::mini::openColor(LVL)         \
    << warp::mini::getTimestamp()         \
    << warp::mini::LEVEL_STR[LVL]         \
    << warp::mini::closeColor()           \

#define WLOGT  WLOG(L_TRACE)
#define WLOGD  WLOG(L_DEBUG)
#define WLOGI  WLOG(L_INFO)
#define WLOGW  WLOG(L_WARN)
#define WLOGE  WLOG(L_ERROR)
#define WLOGF  WLOG(L_FATAL)

/// `if (flag) os <<`

#define WLOG_IF(LVL, CONDITION) \
  if (CONDITION) WLOG(LVL)

#define WLOGT_IF(CONDITION)  WLOG_IF(L_TRACE, CONDITION)
#define WLOGD_IF(CONDITION)  WLOG_IF(L_DEBUG, CONDITION)
#define WLOGI_IF(CONDITION)  WLOG_IF(L_INFO, CONDITION)
#define WLOGW_IF(CONDITION)  WLOG_IF(L_WARN, CONDITION)
#define WLOGE_IF(CONDITION)  WLOG_IF(L_ERROR, CONDITION)
#define WLOGF_IF(CONDITION)  WLOG_IF(L_FATAL, CONDITION)

/// `os << "a ==|!= b" << (flag) ? [P] : [F]`

#define WTEST(CONDITION) do {                            \
  std::cout << warp::mini::colorText(34, "\n[TEST]");    \
  (CONDITION)                                            \
    ? std::cout << warp::mini::colorText(32, "[PASS]")   \
    : std::cout << warp::mini::colorText(31, "[FAIL]");  \
  std::cout << " : " #CONDITION "\n";                    \
} while (0)

#define WTEST_EQ(ACTUAL, EXPECTED)  WTEST((ACTUAL) == (EXPECTED))
#define WTEST_NE(ACTUAL, EXPECTED)  WTEST((ACTUAL) != (EXPECTED))

/// `if (!flag) WLOGF << flag; abort();`

#define WASSERT(CONDITION) do {                       \
  if (!(CONDITION)) {                                 \
    WLOGF                                             \
      << warp::mini::colorText(41, "[ASSERT][FAIL]")  \
      << " : " #CONDITION "\n";                       \
    std::abort();                                     \
  }                                                   \
} while (0)

#define WASSERT_EQ(ACTUAL, EXPECTED)  WASSERT((ACTUAL) == (EXPECTED))
#define WASSERT_NE(ACTUAL, EXPECTED)  WASSERT((ACTUAL) != (EXPECTED))
