#include "warp_test/warp_test.hpp"
#include <format>
#include <iostream>
#include <string_view>

namespace warp::test {

void Suite::_logTestCase(bool cond, std::string_view desc) noexcept {
  std::cout << std::format(
    ""
    "    \033[34m[CASE]\033[0m"
    "{}"
    " : {}\n",
    (cond ? "\033[32m[PASS]\033[0m" : "\033[31m[FAIL]\033[0m"), desc
  );
}

Suite::Suite(std::string_view desc) noexcept {
  std::cout << std::format("  \033[34m[SUITE]\033[0m : {} {}\n\n", desc, '{');
}

Suite::~Suite() noexcept {
  std::cout << std::format("\n  {} \033[33m[{}/{}]\033[0m\n\n", '}', _test_summary.getPassedCases(), _test_summary.getTotalCases());
}

void Suite::checkEq(bool cond, std::string_view desc) noexcept {
  _test_summary.addCase(cond);
  Suite::_logTestCase(cond, desc);
}

void Suite::checkNeq(bool cond, std::string_view desc) noexcept {
  _test_summary.addCase(!cond);
  Suite::_logTestCase(!cond, desc);
}

} // namespace warp::test