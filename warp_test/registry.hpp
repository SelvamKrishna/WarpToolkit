#pragma once

#include "misc.hpp"

#include "warp_log/misc.hpp"
#include "warp_log/tag.hpp"
#include "warp_log/logger.hpp"

#include <string_view>
#include <vector>
#include <functional>

namespace warp::test {

class Registry final {
private:
  std::vector<std::string> _collection_score_vec;
  internal::Summary _test_summary;

public:
  explicit Registry() noexcept = default;

  ~Registry() noexcept {
    log::internal::writeToConsole(
      log::Level::Message,
      std::format("{}{}", log::BREAK_LINE, log::makeColoredTag(log::ANSIFore::Blue, "[REGISTRY]")),
      _test_summary.getSummaryString()
    );

    const log::Logger COLLECTION_LOG {log::makeColoredTag(log::ANSIFore::Blue, "\t[COLLECTION]")};
    for (const auto& COLL_SCORE : _collection_score_vec) COLLECTION_LOG.msg(COLL_SCORE);
  }

  Registry& addCollection(
    std::string_view name,
    std::vector<std::function<internal::Summary()>> suites
  ) noexcept {
    static const log::Logger COLLECTION_LOG {log::makeColoredTag(log::ANSIFore::Blue, "[COLLECTION]")};
    COLLECTION_LOG.msg(name);

    internal::Summary collection_summary {};
    for (const auto& TEST : suites) collection_summary += TEST();
    std::string collection_summary_str {std::move(collection_summary.getSummaryString())};

    COLLECTION_LOG.msg(collection_summary_str);
    _test_summary += collection_summary;

    _collection_score_vec.push_back(std::format("{} : {}", collection_summary_str, name));
    return *this;
  }

  [[nodiscard]] constexpr int conclude() const noexcept { return (_test_summary.getFailedCases() == 0) ? 0 : 1; }
};

} // namespace warp::test
