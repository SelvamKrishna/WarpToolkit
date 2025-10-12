#include "warp_log.hpp"

#include <cstddef>
#include <mutex>

static std::mutex g_console_mutex;

void warp::log::internal::write_to_console(
	std::ostream& os,
	std::string_view pre,
	std::string_view msg
) {
	std::scoped_lock lock{g_console_mutex};

	std::string buffer;
	buffer.reserve(pre.size() + msg.size() + 4);

	buffer.append(pre);
	buffer.append(" : ");
	buffer.append(msg);
	buffer.append("\n");

	os << buffer;
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