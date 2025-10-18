#include <mutex>
#include <chrono>
#include <ctime>

#include "warp_log.hpp"

namespace warp::log::internal {

static std::mutex g_console_mutex;

void writeToConsole(Level lvl, std::string_view pre, std::string_view msg) {
  std::scoped_lock lock{g_console_mutex};
  std::ostream& os = streamFromLevel(lvl);

  os.write(pre.data(), pre.size());
  os.write("\033[", 2); // ESC prefix

  switch (lvl) {
    case Level::Info:  os.write("32m[INFO]", 9); break;   // Green
    case Level::Debug: os.write("36m[DEBUG]", 10); break; // Cyan
    case Level::Warn:  os.write("33m[WARN]", 9); break;   // Yellow
    case Level::Error: os.write("31m[ERROR]", 10); break; // Red
  }

  os.write("\033[0m : ", 7);
  os.write(msg.data(), msg.size());
  os.put('\n');
  os.flush();
}

std::string cacheTagVec(const std::vector<Tag>& tags, std::string_view delim) {
  if (tags.empty()) return {};

  std::string result = tags[0];
  for (size_t i = 1; i < tags.size(); ++i) result.append(delim).append(tags[i]);

  return result;
}

} // namespace warp::log::internal

namespace warp::log {

[[nodiscard]] std::string Sender::_getTimestamp() const noexcept {
  char buf[sizeof("HH:MM:SS")] {};
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm_struct{};

  #ifdef _WIN32
    localtime_s(&tm_struct, &t);
  #else
    localtime_r(&t, &tm_struct);
  #endif

  std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
  return buf;
}

} // namespace warp::log