#pragma once

#include "summary.hpp"

#include "warp_log/logger.hpp"

#include <format>
#include <string_view>

namespace warp::test {

/// Evaluates and logs multiple test cases using warp_log
class Suite final {
private:
  internal::Summary _test_summary;
  log::Logger _logger;

  void _logTestCase(bool cond, std::string_view desc) noexcept {
    static const log::Tag CASE_TAG = log::makeColoredTag(log::ANSIFore::Blue, "\t\t\t[CASE]");
    static const log::Tag PASS_TAG = log::makeColoredTag(log::ANSIFore::Green, "[PASS]");
    static const log::Tag FAIL_TAG = log::makeColoredTag(log::ANSIFore::Red, "[FAIL]");
    std::cout << CASE_TAG << (cond ? PASS_TAG : FAIL_TAG) << " : " << desc << '\n';
  }

public:
  Suite() = delete;

  explicit Suite(std::string_view desc) noexcept : _logger {
    log::makeColoredTag(log::ANSIFore::Blue, "\t\t[SUITE]")
  } { _logger.msg(desc); }

  ~Suite() noexcept {
    _logger.msg(
      "{}[{}/{}]{}",
      log::setColor(log::ANSIFore::Yellow),
      _test_summary.getPassedCases(),
      _test_summary.getTotalCases(),
      log::resetColor()
    );
  }

  void test(bool cond, std::string_view desc) noexcept {
    _test_summary.addCase(cond);
    _logTestCase(cond, desc);
  }

  [[nodiscard]] constexpr internal::Summary getSummary() const noexcept { return _test_summary; }
};

#define TEST_SUITE(FN) \
  warp::test::internal::Summary FN()

#define TEST_EQ(SUITE, ACTUAL, EXPECTED) do { \
  SUITE.test((ACTUAL) == (EXPECTED), #ACTUAL " == " #EXPECTED); \
} while(0)

#define TEST_NEQ(SUITE, ACTUAL, EXPECTED) do { \
  SUITE.test((ACTUAL) != (EXPECTED), #ACTUAL " != " #EXPECTED); \
} while(0)

} // namespace warp::test