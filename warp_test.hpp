#pragma once
#ifndef WARP_TEST
#define WARP_TEST

#ifndef WARP_TOOLKIT_API
  #ifdef _WIN32
    #define WARP_TOOLKIT_API __declspec(dllexport)
  #else
    #define WARP_TOOLKIT_API
  #endif
#endif

#include <cstdint>
#include <string_view>
#include <functional>
#include <vector>
#include <iostream>
#include <format>

namespace warp::test {

namespace internal {

/// Keeps track of all test cases
class Summary final {
private:
  uint64_t _total_case  {0};
  uint64_t _passed_case {0};

public:
  explicit Summary() noexcept = default;

  constexpr void addCase(bool case_res) noexcept {
    _total_case++;
    if (case_res == true) _passed_case++;
  }

  [[nodiscard]] constexpr uint64_t getTotalCases() const noexcept { return _total_case; }
  [[nodiscard]] constexpr uint64_t getPassedCases() const noexcept { return _passed_case; }
  [[nodiscard]] constexpr uint64_t getFailedCases() const noexcept { return _total_case - _passed_case; }

  friend Summary& operator+=(Summary& self, const Summary& other) noexcept {
    self._total_case += other._total_case;
    self._passed_case += other._passed_case;
    return self;
  }
};

} // namespace internal

/// A evaluates a collection of test cases
class WARP_TOOLKIT_API Suite final {
private:
  internal::Summary _test_summary;

  /// Logs test description and result to the console
  static void _logTestCase(bool cond, std::string_view desc) noexcept {
    std::cout << std::format(
      ""
      "\t\t\t\033[34m[CASE]\033[0m"
      "{}"
      " : {}\n",
      (cond ? "\033[32m[PASS]\033[0m" : "\033[31m[FAIL]\033[0m"), desc
    );
  }

public:
  Suite() = delete;

  explicit Suite(std::string_view desc) noexcept {
    std::cout << std::format("\t\t\033[34m[SUITE]\033[0m : {}", desc) << " {\n";
  }

  ~Suite() noexcept {
    std::cout << std::format(
      "\t\t{} \033[33m[{}/{}]\033[0m\n", '}',
      _test_summary.getPassedCases(), _test_summary.getTotalCases()
    );
  }

  void test(bool cond, std::string_view desc) noexcept {
    _test_summary.addCase(cond);
    Suite::_logTestCase(cond, desc);
  }

  [[nodiscard]] constexpr internal::Summary getSummary() const noexcept {
    return _test_summary;
  }
};

/// Master class to handle all test cases
class WARP_TOOLKIT_API Registry final {
private:
  internal::Summary _test_summary;

public:
  explicit Registry() noexcept {
    std::cout << std::format("\033[34m[REGISTRY]\033[0m") << " {\n";
  }

  ~Registry() noexcept {
    std::cout << std::format(
      "{} \033[33m[{}/{}]\033[0m\n", '}',
      _test_summary.getPassedCases(), _test_summary.getTotalCases()
    ) << std::endl;
  }

  /// Evaluates a collection of test suites
  [[nodiscard]] Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept {
    std::cout << std::format("\t\033[34m[COLLECTION]\033[0m : {}", name) << " {\n";
    internal::Summary collection_summary {};
    for (const auto& TEST : suites) collection_summary += TEST();

    std::cout << std::format(
      "\t{} \033[33m[{}/{}]\033[0m\n", '}',
      collection_summary.getPassedCases(), collection_summary.getTotalCases()
    );

    _test_summary += collection_summary;
    return *this;
  }

  [[nodiscard]] constexpr int conclude() const noexcept {
    return (_test_summary.getFailedCases() == 0) ? 0 : 1;
  }
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

#endif // WARP_TEST