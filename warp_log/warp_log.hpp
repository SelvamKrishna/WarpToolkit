#pragma once

/// @file warp_log.hpp
/// @brief High-performance, colorized logging system with tag support.
///
/// This header defines the `warp::log` namespace, including:
///  - Log levels (`INFO`, `DEBUG`, `WARN`, `ERROR`)
///  - `i_tag` interface for context tags
///  - `sender` class for logging messages
///  - `internal` functions for console output
///
/// Supports pre-formatted colored tags, multi-tag concatenation,
/// and directing messages to `std::cout` / `std::cerr` based on log level.
///
/// @author SelvamKrishna
/// @link https://www.github.com/SelvamKrishna
/// @version 1
/// @date 2025-10-12

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
#include <format>
#include <utility>

namespace warp::log {

/// @enum `level`
/// @brief Logging levels supported by the logger.
enum class level : uint8_t {
	INFO,   /// < Informational messages
	DEBUG,  /// < Debug messages
	WARN,   /// < Warning messages
	ERROR   /// < Error messages
};

/// @class `i_tag`
/// @brief Interface for a log tag.
/// A tag represents a label or context for a log message.
/// All tags must implement `get_str()` to provide their string representation.
class WARP_TOOLKIT_API i_tag {
public:
#pragma region /// C-tors & D-tors

	i_tag() = default;
	virtual ~i_tag() = default;

#pragma endregion /// C-tors & D-tors
#pragma region /// Interface

	///@brief Retrieve the string representation of the tag.
	///@return The string for this tag.
	[[nodiscard]] virtual std::string get_str() const noexcept = 0;

#pragma endregion /// Interface
};

/// @brief A vector of non-owning pointers to i_tag objects.
using tag_vec = std::vector<i_tag*>;

namespace internal {

/// @brief Write a log message to the console with a given level and prefix.
/// @param lvl The log level of the message.
/// @param pre The prefix or context string (Typically one or many tags).
/// @param msg The actual log message.
WARP_TOOLKIT_API void write_to_console(level lvl, std::string_view pre, std::string_view msg);

/// @brief Get the appropriate output stream for a log level.
/// `INFO` and `DEBUG` use `std::cout`, `WARN` and `ERROR` use `std::cerr`.
/// @param lvl The log level.
/// @return Reference to the output stream.
[[nodiscard]] inline constexpr std::ostream& stream_from_level(level lvl) noexcept {
	return (lvl == level::INFO || lvl == level::DEBUG) ? std::cout : std::cerr;
}

/// @brief Concatenate a vector of tags into a single string.
/// Optionally separates tags using a delimiter.
/// @param tags Vector of tag pointers (non-owning).
/// @param delim Optional delimiter between tags.
/// @return Concatenated string of all tag representations.
WARP_TOOLKIT_API std::string cache_tag_vec(const tag_vec& tags, std::string_view delim = "");

} /// namespace internal

/// @class `sender`
/// @brief A logger instance that can send messages with a given context or tags.
/// Can log messages at different levels (INFO, DEBUG, WARN, ERROR) using optional tags or string contexts.
class WARP_TOOLKIT_API sender final {
private:
	std::string _context; /// < The context or prefix for all messages sent by this sender.

	/// @brief Helper function to log message to console from `sender`
	/// @param lvl The log level.
	/// @param msg The message to log.
	/// @param args The arguments for formatting.
	template <typename... Args>
	void _log(level lvl, std::string_view msg, Args&&... args) const {
		try {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			internal::write_to_console(lvl, _context, formatted);
		} catch(const std::format_error& e) {
			internal::write_to_console(
				level::ERROR, 
				"",
				"Format error: " + std::string(e.what())
			);
		}
	}

public:
#pragma region /// C-tors & D-tors

	sender() = delete;

	/// @brief Construct a sender with a custom string context.
	/// @param context The prefix/context for log messages.
	explicit sender(std::string context) noexcept : _context(std::move(context)) {}

	/// @brief Construct a sender from a single tag.
	/// @param tag The tag to use as context.
	explicit sender(const i_tag& tag) noexcept : _context(tag.get_str()) {}

	/// @brief Construct a sender from multiple tags.
	/// @param tags Vector of tags to combine as context.
	explicit sender(const tag_vec& tags) noexcept : _context(internal::cache_tag_vec(tags)) {}

#pragma endregion /// C-tors & D-tors
#pragma region /// Utility

	/// @brief Logs an informational message.
	/// @param msg The message to log.
	/// @param args The arguments for formatting.
	template <typename... Args>
	void info(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::INFO, msg, std::forward<Args>(args)...);
	}

	/// @brief Logs a debug message.
	/// @param msg The message to log.
	/// @param args The arguments for formatting.
	template <typename... Args>
	void dbg(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::DEBUG, msg, std::forward<Args>(args)...);
	}
	
	/// @brief Logs a warning message.
	/// @param msg The message to log.
	/// @param args The arguments for formatting.
	template <typename... Args>
	void warn(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::WARN, msg, std::forward<Args>(args)...);
	}
	
	/// @brief Logs an error message.
	/// @param msg The message to log.
	/// @param args The arguments for formatting.
	template <typename... Args>
	void error(std::format_string<Args...> msg, Args&&... args) const {
		_log(level::ERROR, msg, std::forward<Args>(args)...);
	}
	
#pragma endregion /// Utility
};

} /// namespace warp::log

#endif /// WARP_LOG