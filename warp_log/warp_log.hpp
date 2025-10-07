#pragma once

#ifndef WARP_LOG
#define WARP_LOG

#define WARP_LOG_VER 1

#ifdef _WIN32
	#define WARP_LOG_API __declspec(dllexport)
#else
	#define WARP_LOG_API
#endif

#include <cstdint>
#include <iostream>

namespace warp::log {

#pragma region /// Helpers

enum class level : uint8_t {
	INFO,
	DEBUG,
	WARN,
	ERROR,
};

namespace internal {

WARP_LOG_API void write_to_console(std::ostream& os, const std::string& pre, const std::string& suf);

WARP_LOG_API inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
	return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

} /// internal

#pragma endregion /// Helpers

class WARP_LOG_API sender final {
private:
	std::string _context;

public:
#pragma region /// C-tors & D-tors

	explicit sender() noexcept = delete;
	explicit sender(std::string context) noexcept : _context(std::move(context)) {}

	~sender() noexcept = default;

#pragma endregion /// C-tors & D-tors
#pragma region /// Copy & Move (Default)

	explicit constexpr sender(const sender &) noexcept = default;
	sender &operator=(const sender &) noexcept = default;

	explicit constexpr sender(sender &&) noexcept = default;
	sender &operator=(sender &&) noexcept = default;

#pragma endregion /// Copy & Move (Default)
#pragma region /// Core features

	void log(level log_level, const std::string& msg) const noexcept;

#pragma endregion /// Core features
};

} /// namespace warp::log

#endif /// WARP_LOG