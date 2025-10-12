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

#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace warp::log {

/// @enum `level`
/// @brief Logging levels supported by the logger
enum class level : uint8_t {
	INFO,   /// < Informational messages
	DEBUG,  /// < Debug messages
	WARN,   /// < Warning messages
	ERROR   /// < Error messages
};

/// @enum `ansi_fg_color`
/// @brief ANSI code for terminal foreground colors
enum class ansi_fg_color : uint8_t {
	BLACK = 30,       /// < Standard black
	RED,              /// < Standard red
	GREEN,            /// < Standard green
	YELLOW,           /// < Standard yellow
	BLUE,             /// < Standard blue
	MAGENTA,          /// < Standard magenta
	CYAN,             /// < Standard cyan
	WHITE,            /// < Standard white
	RESET = 39,       /// < Reset to default color

	LIGHT_BLACK_EX = 90,  /// < Bright black (gray)
	LIGHT_RED_EX,         /// < Bright red
	LIGHT_GREEN_EX,       /// < Bright green
	LIGHT_YELLOW_EX,      /// < Bright yellow
	LIGHT_BLUE_EX,        /// < Bright blue
	LIGHT_MAGENTA_EX,     /// < Bright magenta
	LIGHT_CYAN_EX,        /// < Bright cyan
	LIGHT_WHITE_EX        /// < Bright white
};

#pragma region /// Tag

/// @brief A string representing a log tag
using tag = std::string;

#pragma region /// Tag factories

/// @brief Create a simple text tag
/// @param text The tag content
/// @return A `tag` containing the given text
[[nodiscard]] WARP_TOOLKIT_API constexpr tag make_default_tag(std::string_view text) noexcept {
	return std::string{text};
}

/// @brief Create a colored ANSI tag
/// @param fg_color The foreground color to use
/// @param text The tag content
/// @return A `tag` containing ANSI-colored text
[[nodiscard]] WARP_TOOLKIT_API constexpr tag make_colored_tag(ansi_fg_color fg_color, std::string_view text) noexcept {
	return std::format("\033[{}m{}\033[0m", static_cast<int>(fg_color), text);
}

#pragma endregion /// Tag factories
#pragma endregion /// Tag

namespace internal {

/// @brief Write a log message to the console with a prefix
/// @param lvl Log level
/// @param pre Context or tag string
/// @param msg The actual message
WARP_TOOLKIT_API void write_to_console(level lvl, std::string_view pre, std::string_view msg);

/// @brief Concatenate multiple tags into a single string
/// @param tags Vector of tags
/// @param delim Optional delimiter between tags
/// @return Concatenated string
WARP_TOOLKIT_API std::string cache_tag_vec(const std::vector<tag>& tags, std::string_view delim = "");

/// @brief Select stream based on log level
/// @param lvl Log level
/// @return `std::cout` for INFO/DEBUG, `std::cerr` for WARN/ERROR
[[nodiscard]] inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
	return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

} /// namespace internal

/// @class sender
/// @brief Logger instance that can send messages with optional tags and runtime timestamp
class WARP_TOOLKIT_API sender final {
private:
	std::string _context;     		 /// < The context or prefix for all messages
	bool _log_timestamp;      		 /// < Whether to prepend a timestamp
	ansi_fg_color _time_stamp_color; /// < Foreground color of timestamp

#pragma region /// Helpers

	/// @brief Generate a runtime timestamp
	/// @return String representation of current date and time
	[[nodiscard]] std::string _get_timestamp() const noexcept;

	/// @brief Internal logging helper
	/// @param lvl Log level
	/// @param msg Format string message
	/// @param args Arguments for formatting
	template <typename... Args>
	void _log(level lvl, std::format_string<Args...> msg, Args&&... args) const {
		std::string formatted {
			_log_timestamp
			? make_colored_tag(_time_stamp_color, _get_timestamp()) + _context
			: _context
		};

		internal::write_to_console(lvl, formatted, std::format(msg, std::forward<Args>(args)...));
	}

#pragma endregion /// Helpers
public:
#pragma region /// Constructors

	sender() = delete;

	/// @brief Construct sender from a single tag
	/// @param t Tag to use as context
	/// @param log_timestamp Should the timestamp be logged with the message
	explicit sender(tag t, bool log_timestamp = false) noexcept
	: _context(std::move(t))
	, _log_timestamp(log_timestamp)
	, _time_stamp_color(ansi_fg_color::WHITE) {}

	/// @brief Construct sender from multiple tags
	/// @param tags Vector of tags to combine as context
	/// @param log_timestamp Should the timestamp be logged with the message
	explicit sender(const std::vector<tag>& tags, bool log_timestamp = false) noexcept
	: _context(internal::cache_tag_vec(tags))
	, _log_timestamp(log_timestamp)
	, _time_stamp_color(ansi_fg_color::WHITE) {}

#pragma endregion /// Constructors
#pragma region /// Logging functions

	/// @brief Log an informational message
	template <typename... Args>
	void info(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::INFO, msg, std::forward<Args>(args)...);
	}

	/// @brief Log a debug message
	template <typename... Args>
	void dbg(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::DEBUG, msg, std::forward<Args>(args)...);
	}

	/// @brief Log a warning message
	template <typename... Args>
	void warn(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::WARN, msg, std::forward<Args>(args)...);
	}

	/// @brief Log an error message
	template <typename... Args>
	void error(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::ERROR, msg, std::forward<Args>(args)...);
	}

	/// @brief Enable or disable runtime timestamps for all log messages
	/// @param flag `true` to enable, `false` to disable
	constexpr void log_timestamp(bool flag) noexcept { _log_timestamp = flag; }

	/// @brief Change the timestamp color
	/// @param color The new color of the timestamp
	constexpr void set_timestamp_color(ansi_fg_color color) noexcept { _time_stamp_color = color; }

#pragma endregion /// Logging functions
};

} /// namespace warp::log

#endif /// WARP_LOG