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

std::string cache_tag_vec(const tag_vec& tags, std::string_view delim) {
	if (tags.empty()) return {};

	// Find first non null tag
	size_t start = 0;
	while (start < tags.size() && tags[start] == nullptr) ++start;

	// If all tags are null
	if (start == tags.size()) return {};

	std::string result {tags[start]->get_str()};

	for (size_t i = start + 1; i < tags.size(); ++i) {
		const auto* TAG = tags[i];
		if (TAG == nullptr) [[unlikely]] continue;

		if (!delim.empty()) result.append(delim);
		result.append(TAG->get_str());
	}

	return result;
}

} /// namespace warp::log::internal