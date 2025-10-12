#pragma once

#ifndef WARP_LOG
#define WARP_LOG

#define WARP_LOG_VER 1

#ifndef WARP_TOOLKIT_API
	#ifdef _WIN32
		#define WARP_TOOLKIT_API __declspec(dllexport)
	#else
		#define WARP_TOOLKIT_API
	#endif
#endif

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <string_view>

namespace warp::log {

enum class level : uint8_t {
	INFO,
	DEBUG,
	WARN,
	ERROR,
};

class WARP_TOOLKIT_API i_tag {
public:
	i_tag() = default;
	virtual ~i_tag() = default;
	[[nodiscard]] virtual std::string get_str() const noexcept = 0;
};

using tag_vec = std::vector<i_tag*>;

namespace internal {

WARP_TOOLKIT_API void write_to_console(level lvl, std::string_view pre, std::string_view msg);

[[nodiscard]] inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
	return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

WARP_TOOLKIT_API std::string cache_tag_vec(const tag_vec& tags, std::string_view delim = "");

} /// namespace internal

class WARP_TOOLKIT_API sender final {
private:
	std::string _context;

public:
	sender() = delete;
	
	explicit sender(std::string context) noexcept : _context(std::move(context)) {}

	explicit sender(const i_tag& tag) noexcept : _context(tag.get_str()) {}

	explicit sender(const tag_vec& tags) noexcept : _context(internal::cache_tag_vec(tags)) {}

	void info(std::string_view msg) const noexcept { internal::write_to_console(level::INFO, _context, msg); }
	void dbg(std::string_view msg) const noexcept { internal::write_to_console(level::DEBUG, _context, msg); }
	void warn(std::string_view msg) const noexcept { internal::write_to_console(level::WARN, _context, msg); }
	void error(std::string_view msg) const noexcept { internal::write_to_console(level::ERROR, _context, msg); }
};

} /// namespace warp::log

#endif /// WARP_LOG