#pragma once

#include "warp_log/misc.hpp"

#include <cstdint>
#include <format>

namespace warp::test::internal {

/// Tool to monitor the results of the test cases
class Summary final {
private:
  uint32_t _total_case  {0};
  uint32_t _passed_case {0};

public:
  explicit Summary() noexcept = default;

  /// Re-evaluate summary with new test case
  constexpr void addCase(bool case_res) noexcept {
    _total_case++;
    _passed_case += static_cast<int>(case_res);
  }

  [[nodiscard]] constexpr uint32_t getTotalCases()  const noexcept { return _total_case; }
  [[nodiscard]] constexpr uint32_t getPassedCases() const noexcept { return _passed_case; }
  [[nodiscard]] constexpr uint32_t getFailedCases() const noexcept { return _total_case - _passed_case; }

  friend Summary& operator+=(Summary& self, const Summary& other) noexcept {
    self._total_case  += other._total_case;
    self._passed_case += other._passed_case;
    return self;
  }

  /// Formated string of the summary : "[passed/total]"
  std::string getSummaryString() const noexcept {
    return std::format("{}[{}/{}]{}", log::setColor(log::ANSIFore::Yellow), _passed_case, _total_case, log::resetColor());
  }
};

} // namespace warp::test::internal
