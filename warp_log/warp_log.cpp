#include <mutex>

#include "warp_log.hpp"

namespace warp::log::internal {

static std::mutex g_console_mutex;

void write_to_console(level lvl, std::string_view pre, std::string_view msg) {
	std::scoped_lock lock {g_console_mutex};
	std::ostream& os {stream_from_level(lvl)};

	os.write(pre.data(), pre.size());
	os.write("\033[", 2);

	switch (lvl) {
		case level::INFO:  os.write("32m[INFO]", 9); break;   // Green
		case level::DEBUG: os.write("36m[DEBUG]", 10); break; // Cyan
		case level::WARN:  os.write("33m[WARN]", 9); break;   // Yellow
		case level::ERROR: os.write("31m[ERROR]", 10); break; // Red
	}

	os.write("\033[0m : ", 7);

	os.write(msg.data(), msg.size());
	os.put('\n');
	os.flush();
}

std::string cache_tag_vec(const std::vector<tag>& tags, std::string_view delim) {
	if (tags.empty()) return {};

    std::string result;
    result.append(tags[0]);

    for (size_t i = 1; i < tags.size(); ++i) {
        result.append(delim);
        result.append(tags[i]);
    }

	return result;
}

} /// namespace warp::log::internal

namespace warp::log {

[[nodiscard]] std::string sender::_get_timestamp() const noexcept {
    char buf[sizeof("[HH:MM:SS]")] {0};
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_struct{};

#ifdef _WIN32
    localtime_s(&tm_struct, &t);
#else
    localtime_r(&t, &tm_struct);
#endif

    std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm_struct);
    return std::string(buf);
}

} /// namespace warp::log