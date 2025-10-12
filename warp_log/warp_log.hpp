#pragma once

/// @file warp_log.hpp
/// @brief High-performance, colorized logging system with tag support.
/// Supports runtime timestamps, pre-formatted colored tags, multi-tag concatenation,
/// and directing messages to std::cout / std::cerr based on log level.
/// ---
/// @author SelvamKrishna
/// @link https://www.github.com/SelvamKrishna
/// @date 2025-10-12

#ifndef WARP_LOG
#define WARP_LOG

#ifndef WARP_TOOLKIT_API
	#ifdef _WIN32
		#define WARP_TOOLKIT_API __declspec(dllexport)
	#else
		#define WARP_TOOLKIT_API
	#endif
#endif

#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace warp::log {

/// @enum `level`
/// @brief Logging levels supported by the logger.
enum class level : uint8_t {
    INFO,
    DEBUG,
    WARN,
    ERROR
};

/// @enum `ansi_fg_color`
/// @brief ANSI code for all terminal supported foreground colors
enum class ansi_fg_color : uint8_t {
    BLACK = 30,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE,
	RESET = 39,

    LIGHT_BLACK_EX = 90,
	LIGHT_RED_EX,
	LIGHT_GREEN_EX,
	LIGHT_YELLOW_EX,
    LIGHT_BLUE_EX,
	LIGHT_MAGENTA_EX,
	LIGHT_CYAN_EX,
	LIGHT_WHITE_EX
};

#pragma region /// Tag

using tag = std::string;

[[nodiscard]] WARP_TOOLKIT_API inline tag make_default_tag(std::string_view text) noexcept {
    return std::string{text};
}

[[nodiscard]] WARP_TOOLKIT_API inline tag make_colored_tag(ansi_fg_color fg_color, std::string_view text) noexcept {
    return std::format("\033[{}m{}\033[0m", static_cast<int>(fg_color), text);
}

#pragma endregion /// Tag

namespace internal {

void write_to_console(level lvl, std::string_view pre, std::string_view msg);

std::string cache_tag_vec(const std::vector<tag>& tags, std::string_view delim = "");

[[nodiscard]] inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
    return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

} /// namespace internal

/// @class sender
/// @brief Logger instance with optional tags and runtime timestamp support.
class WARP_TOOLKIT_API sender final {
private:
    std::string _context;
    bool _log_timestamp{false};

    [[nodiscard]] std::string _get_timestamp() const noexcept;

    template <typename... Args>
    void _log(level lvl, std::format_string<Args...> msg, Args&&... args) const {
        internal::write_to_console(
			lvl, 
			std::move((_log_timestamp ? _get_timestamp() : "") + _context),
			std::move(std::format(msg, std::forward<Args>(args)...))
		);
    }

public:
    sender() = delete;

    explicit sender(tag t) noexcept : _context(std::move(t)) {}
    explicit sender(const std::vector<tag>& tags) noexcept : _context(internal::cache_tag_vec(tags)) {}

    template <typename... Args>
    void info(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::INFO, msg, std::forward<Args>(args)...); 
	}

    template <typename... Args>
    void dbg(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::DEBUG, msg, std::forward<Args>(args)...); 
	}

    template <typename... Args>
    void warn(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::WARN, msg, std::forward<Args>(args)...); 
	}

    template <typename... Args>
    void error(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::ERROR, msg, std::forward<Args>(args)...); 
	}

    constexpr void log_timestamp(bool flag) noexcept { _log_timestamp = flag; }
};

} /// namespace warp::log

#endif /// WARP_LOG