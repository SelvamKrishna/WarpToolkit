#pragma once

#include <string>

#include "../warp_log.hpp"

namespace warp::log::templates {

enum class ansi_foreground_color : uint8_t {
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

namespace internal {

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
		default: return "39";
	}
}

} /// namespace internal

template <ansi_foreground_color FG>
class colored_tag : public i_tag {
private:
	std::string _text;

public:
	colored_tag(const std::string& text) {
		_text.reserve(text.size() + sizeof("\033[FGm\033[0m"));

		_text.append("\033[");
		_text.append(internal::ansi_code(FG));
		_text.append("m");
		_text.append(text);
		_text.append("\033[0m");
	}
	
	[[nodiscard]] std::string get_str() const noexcept override { return _text; }
};

} /// namespace warp::log::templates