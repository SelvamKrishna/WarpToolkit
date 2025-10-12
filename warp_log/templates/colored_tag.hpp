#pragma once

#include <string>
#include "../warp_log.hpp"

namespace warp::log::templates {

/// @enum `ansi_foreground_color`
/// @brief Foreground ANSI color codes for terminal output.
/// Used by colored_tag to generate colorized log tags.
enum class ansi_foreground_color : uint8_t {
	BLACK = 30,           /// < Standard black
	RED,                  /// < Standard red
	GREEN,                /// < Standard green
	YELLOW,               /// < Standard yellow
	BLUE,                 /// < Standard blue
	MAGENTA,              /// < Standard magenta
	CYAN,                 /// < Standard cyan
	WHITE,                /// < Standard white
	RESET = 39,           /// < Reset to default foreground color

	LIGHT_BLACK_EX = 90,  /// < Bright black (gray)
	LIGHT_RED_EX,         /// < Bright red
	LIGHT_GREEN_EX,       /// < Bright green
	LIGHT_YELLOW_EX,      /// < Bright yellow
	LIGHT_BLUE_EX,        /// < Bright blue
	LIGHT_MAGENTA_EX,     /// < Bright magenta
	LIGHT_CYAN_EX,        /// < Bright cyan
	LIGHT_WHITE_EX        /// < Bright white
};

namespace internal {

/// @brief Convert an `ansi_foreground_color` enum value to its corresponding ANSI code string.
/// @param fg The foreground color enum value.
/// @return A string literal representing the ANSI code for the color.
[[nodiscard]] inline constexpr const char* ansi_code(ansi_foreground_color fg) noexcept {
	switch (fg) {
		case ansi_foreground_color::BLACK: return "30";
		case ansi_foreground_color::RED: return "31";
		case ansi_foreground_color::GREEN: return "32";
		case ansi_foreground_color::YELLOW: return "33";
		case ansi_foreground_color::BLUE: return "34";
		case ansi_foreground_color::MAGENTA: return "35";
		case ansi_foreground_color::CYAN: return "36";
		case ansi_foreground_color::WHITE: return "37";
		case ansi_foreground_color::RESET: return "39";
		case ansi_foreground_color::LIGHT_BLACK_EX: return "90";
		case ansi_foreground_color::LIGHT_RED_EX: return "91";
		case ansi_foreground_color::LIGHT_GREEN_EX: return "92";
		case ansi_foreground_color::LIGHT_YELLOW_EX: return "93";
		case ansi_foreground_color::LIGHT_BLUE_EX: return "94";
		case ansi_foreground_color::LIGHT_MAGENTA_EX: return "95";
		case ansi_foreground_color::LIGHT_CYAN_EX: return "96";
		case ansi_foreground_color::LIGHT_WHITE_EX: return "97";
		default: return "39"; /// < Default to reset color if unknown
	}
}

} /// namespace internal

/// @class `colored_tag`
/// @brief A log tag with a fixed foreground color.
/// This template class constructs a colorized tag string at construction time.
/// The ANSI escape codes are prepended and appended to the provided text.
/// @tparam FG The ANSI foreground color to apply to the tag.
template <ansi_foreground_color FG>
class colored_tag : public i_tag {
private:
	std::string _text; /// < Preformatted colored string stored for efficient reuse

public:
	/// @brief Construct a colored tag from a string.
	/// The resulting tag is immutable; get_str() returns the preformatted ANSI string.
	/// @param text The text content of the tag.
	explicit colored_tag(const std::string& text) {
		_text.reserve(text.size() + sizeof("\033[FGm\033[0m"));
		_text.append("\033[");
		_text.append(internal::ansi_code(FG));
		_text.append("m");
		_text.append(text);
		_text.append("\033[0m");
	}

	/// @brief Get the preformatted string representation of the tag.
	/// @return The colorized tag string, including ANSI escape codes.
	[[nodiscard]] std::string get_str() const noexcept override { return _text; }
};

} /// namespace warp::log::templates