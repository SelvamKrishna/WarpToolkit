#include <mutex>

#include "warp_log.hpp"

static std::mutex g_console_mutex;

void warp::log::internal::write_to_console(
	level lvl, 
	std::string_view pre, 
	std::string_view msg
) {
	auto& os = stream_from_level(lvl);

	os.write(pre.data(), pre.size());
	os.write("\033[", 2);

	switch (lvl) {
		case level::INFO:  os.write("32m[INFO]", 9); break;
		case level::DEBUG: os.write("36m[DEBUG]", 10); break;
		case level::WARN:  os.write("33m[WARN]", 9); break;
		case level::ERROR: os.write("31m[ERROR]", 10); break;
	}

	os.write("\033[0m : ", 7);

	os.write(msg.data(), msg.size());
	os.put('\n');
	os.flush();
}

std::string warp::log::internal::cache_tag_vec(
	const warp::log::tag_vec& tag_vec,
	std::string_view delim
) {
	if (tag_vec.empty()) return {};

	size_t total_size = (tag_vec.size() - 1) * delim.size();
	for (const auto* TAG: tag_vec) {
		if (TAG) total_size += TAG->get_str().size();
	}

	std::string result;
	result.reserve(total_size);
	result.append(tag_vec[0]->get_str());

	for (size_t i = 1; i < tag_vec.size(); ++i)	{
		if (tag_vec[i] == nullptr) [[unlikely]] continue;

		result.append(delim);
		result.append(tag_vec[i]->get_str());
	}

	return result;
}