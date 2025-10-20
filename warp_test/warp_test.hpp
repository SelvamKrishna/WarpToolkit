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

namespace warp::test {

namespace internal {

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

class WARP_TOOLKIT_API Suite final {
private:
  internal::Summary _test_summary;

  static void _logTestCase(bool cond, std::string_view desc) noexcept;

public:
  Suite() = delete;
  explicit Suite(std::string_view desc) noexcept;
  ~Suite() noexcept;

  void checkEq(bool cond, std::string_view desc) noexcept;
  void checkNeq(bool cond, std::string_view desc) noexcept;

  [[nodiscard]] constexpr internal::Summary close() const noexcept {
    return _test_summary;
  }
};

class WARP_TOOLKIT_API Registry final {
private:
  internal::Summary _test_summary;

public:
  explicit Registry() noexcept;
  ~Registry() noexcept;

  [[nodiscard]] Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept;

  [[nodiscard]] constexpr int close() const noexcept {
    return (_test_summary.getFailedCases() == 0) ? 0 : 1;
  }
};

#define TEST_SUITE(fn_name) \
  warp::test::internal::Summary fn_name()

} // namespace warp::test

#endif // WARP_TEST