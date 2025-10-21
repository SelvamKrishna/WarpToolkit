#pragma once

#include "warp_log.hpp"

#include <format>
#include <vector>
#include <cstdint>
#include <functional>
#include <string_view>

namespace warp::test {

namespace internal {

/// Provides utilities to store and evaluate test cases
class Summary final {
private:
  uint64_t _total_case  {0};
  uint64_t _passed_case {0};

public:
  explicit Summary() noexcept = default;

  constexpr void addCase(bool case_res) noexcept {
    _total_case++;
    _passed_case += static_cast<int>(case_res);
  }

  [[nodiscard]] constexpr uint64_t getTotalCases()  const noexcept { return _total_case; }
  [[nodiscard]] constexpr uint64_t getPassedCases() const noexcept { return _passed_case; }
  [[nodiscard]] constexpr uint64_t getFailedCases() const noexcept { return _total_case - _passed_case; }

  friend Summary& operator+=(Summary& self, const Summary& other) noexcept {
    self._total_case  += other._total_case;
    self._passed_case += other._passed_case;
    return self;
  }
};

} // namespace internal

/// Evaluates and logs multiple test cases using warp_log
class Suite final {
private:
  internal::Summary _test_summary;
  warp::Logger _logger;

  void _logTestCase(bool cond, std::string_view desc) noexcept {
    static warp::LogTag case_tag = tag_factory::makeColored(ANSIFore::Blue, "[BLUE]");

    if (cond) {
      _logger.info("\t\t\t\033[34m[CASE]\033[0m\033[32m[PASS]\033[0m : {}", desc);
    } else {
      _logger.err("\t\t\t\033[34m[CASE]\033[0m\033[31m[FAIL]\033[0m : {}", desc);
    }
  }

public:
  Suite() = delete;

  explicit Suite(std::string_view desc) noexcept
    : _logger({"\t\t\033[34m[SUITE]\033[0m : " + std::string(desc) + " {"}) {}

  ~Suite() noexcept {
    _logger.info("\t\t}} \033[33m[{}/{}]\033[0m",
                 _test_summary.getPassedCases(),
                 _test_summary.getTotalCases());
  }

  void test(bool cond, std::string_view desc) noexcept {
    _test_summary.addCase(cond);
    _logTestCase(cond, desc);
  }

  [[nodiscard]] constexpr internal::Summary getSummary() const noexcept {
    return _test_summary;
  }
};

class Registry final {
private:
  internal::Summary _test_summary;
  warp::Logger _logger;

public:
  explicit Registry() noexcept
  : _logger("\033[34m[REGISTRY]\033[0m {") {}

  ~Registry() noexcept {
    _logger.info("}} \033[33m[{}/{}]\033[0m", _test_summary.getPassedCases(), _test_summary.getTotalCases());
  }

  /// Evaluates a collection of test suites
  [[nodiscard]] Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept {
    warp::Logger collection_logger({"\t\033[34m[COLLECTION]\033[0m : " + std::string(name) + " {"});
    internal::Summary collection_summary {};

    for (const auto& TEST : suites) collection_summary += TEST();

    collection_logger.info(
      "\t}} \033[33m[{}/{}]\033[0m",
      collection_summary.getPassedCases(),
      collection_summary.getTotalCases()
    );

    _test_summary += collection_summary;
    return *this;
  }

  [[nodiscard]] constexpr int conclude() const noexcept { return (_test_summary.getFailedCases() == 0) ? 0 : 1; }
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