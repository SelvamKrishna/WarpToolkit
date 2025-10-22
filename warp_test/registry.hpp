#pragma once

#include "summary.hpp"

#include "warp_log/logger.hpp"

#include <string_view>
#include <vector>
#include <functional>

namespace warp::test {

class Registry final {
private:
  internal::Summary _test_summary;
  log::Logger _logger;

public:
  explicit Registry() noexcept
  : _logger{"\033[34m[REGISTRY]\033[0m {"} {}

  ~Registry() noexcept {
    _logger.info("}} \033[33m[{}/{}]\033[0m", _test_summary.getPassedCases(), _test_summary.getTotalCases());
  }

  /// Evaluates a collection of test suites
  [[nodiscard]] Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept {
    log::Logger collection_logger({"\t\033[34m[COLLECTION]\033[0m : " + std::string(name) + " {"});
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

}