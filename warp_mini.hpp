#pragma once

/// --- Config ---

#define ENABLE_COLOR_CODE true
#define ENABLE_TIMESTAMP  false

#define TEST_PASS_TEXT "[PASS]"
#define TEST_FAIL_TEXT "[FAIL]"

#define SCOPE_ENTER_TEXT "--{"
#define SCOPE_LEAVE_TEXT "}--"

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
  "[INFO] ",
  "[WARN] ",
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

/// [PASS], [FAIL]

#if ENABLE_COLOR_CODE
static constexpr const char* PASS {"\033[32m" TEST_PASS_TEXT "\033[0m"};
static constexpr const char* FAIL {"\033[31m" TEST_FAIL_TEXT "\033[0m"};
#else
static constexpr const char* PASS {TEST_PASS_TEXT};
static constexpr const char* FAIL {TEST_FAIL_TEXT};
#endif

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

/// NDEBUG support
#ifdef NDEBUG
static constexpr LogLevel MIN_LOG_LEVEL = L_INFO;
#else
static constexpr LogLevel MIN_LOG_LEVEL = L_TRACE;
#endif

} // namespace warp::mini

/// --- MACROS ---

/// `os <<`

#define WLOG_RAW  std::cout << "\n\033[0m"

#define WLOG(LVL)                                  \
  if constexpr (LVL >= warp::mini::MIN_LOG_LEVEL)  \
    (LVL < L_WARN ? std::cout : std::cerr)         \
      << "\n"                                      \
      << warp::mini::openColor(LVL)                \
      << warp::mini::getTimestamp()                \
      << warp::mini::LEVEL_STR[LVL]                \
      << warp::mini::closeColor()                  \

#define WLOGT  WLOG(L_TRACE)
#define WLOGD  WLOG(L_DEBUG)
#define WLOGI  WLOG(L_INFO)
#define WLOGW  WLOG(L_WARN)
#define WLOGE  WLOG(L_ERROR)
#define WLOGF  WLOG(L_FATAL)

namespace warp::mini {

/// Automatically logs trace messages for functions
struct ScopeTracer {
  std::string FN_NAME;

#if ENABLE_COLOR_CODE
  static constexpr const char* ENTER_TEXT {"\033[92m" SCOPE_ENTER_TEXT "\033[0m"};
  static constexpr const char* LEAVE_TEXT {"\033[91m" SCOPE_LEAVE_TEXT "\033[0m"};
#else
  static constexpr const char* ENTER_TEXT {SCOPE_ENTER_TEXT};
  static constexpr const char* LEAVE_TEXT {SCOPE_LEAVE_TEXT};
#endif

  explicit ScopeTracer(std::string_view fn_name) : FN_NAME {fn_name} {
    WLOGT << ENTER_TEXT << " : " << FN_NAME;
  }

  ~ScopeTracer() {
    WLOGT << LEAVE_TEXT << " : " << FN_NAME;
  }

};

} // namespace warp::mini

/// ScopeTracer {fn};
#define WTRACE           warp::mini::ScopeTracer __scope_tracer__ {std::format("{}()", __FUNCTION__)}
#define WTRACE_C(CLASS)  warp::mini::ScopeTracer __scope_tracer__ {std::format("{}::{}()", #CLASS, __FUNCTION__)}

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

#define WTEST(CONDITION) do {                               \
  std::cout                                                 \
    << warp::mini::colorText(34, "\n[TEST]")                \
    << ((CONDITION) ? warp::mini::PASS : warp::mini::FAIL)  \
    << " : " #CONDITION "\n";                               \
} while (0)

#define WTEST_EQ(ACTUAL, EXPECTED)  WTEST((ACTUAL) == (EXPECTED))
#define WTEST_NE(ACTUAL, EXPECTED)  WTEST((ACTUAL) != (EXPECTED))

/// `if (!flag) WLOGF << flag; abort();`

#define WASSERT(CONDITION) do {                          \
  if (!(CONDITION)) {                                    \
    WLOGF                                                \
      << warp::mini::colorText(41, "[ASSERT][FAILURE]")  \
      << " : " #CONDITION "\n";                          \
    std::abort();                                        \
  }                                                      \
} while (0)

#define WASSERT_EQ(ACTUAL, EXPECTED)  WASSERT((ACTUAL) == (EXPECTED))
#define WASSERT_NE(ACTUAL, EXPECTED)  WASSERT((ACTUAL) != (EXPECTED))
