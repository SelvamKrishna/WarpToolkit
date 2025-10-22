#pragma once

#include "misc.hpp"

#include "warp_log/misc.hpp"
#include "warp_log/tag.hpp"

#include <iostream>
#include <string_view>
#include <vector>
#include <functional>

namespace warp::test {

class Registry final {
private:
  internal::Summary _test_summary;

public:
  explicit Registry() noexcept {
    std::cout << log::makeColoredTag(log::ANSIFore::Blue, "[REGISTRY]\n");
  }

  ~Registry() noexcept {
    std::cout
      << log::makeColoredTag(log::ANSIFore::Blue, "[REGISTRY]") << " : "
      << _test_summary.getSummaryString() << std::endl;
  }

  /// Evaluates a collection of test suites
  [[nodiscard]] Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept {
    static const log::Tag COLLECTION_TAG {log::makeColoredTag(log::ANSIFore::Blue, "\t[COLLECTION]")};
    std::cout << COLLECTION_TAG << " : " << name << '\n';

    internal::Summary collection_summary {};
    for (const auto& TEST : suites) collection_summary += TEST();

    std::cout << COLLECTION_TAG << " : " << collection_summary.getSummaryString() << "\n";
    _test_summary += collection_summary;
    return *this;
  }

  [[nodiscard]] constexpr int conclude() const noexcept { return (_test_summary.getFailedCases() == 0) ? 0 : 1; }
};

}
