#include "warp_log.hpp"

void warp::log::internal::write_to_console(
	std::ostream& os,
	const std::string& pre,
	const std::string& suf
) {
	os << pre << " : " << suf << "\n";
}

void warp::log::sender::log(level lvl, const std::string& msg) const noexcept {
	internal::write_to_console(
		internal::stream_from_level(lvl),
		this->_context,
		msg
	);
}