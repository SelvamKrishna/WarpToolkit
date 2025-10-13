#include <chrono>
#include <iostream>
#include <format>

#include "warp_timer.hpp"

namespace warp {

timer::~timer() noexcept { if (_is_running) stop(); }

void timer::start() noexcept {
	_is_running = true;
	_start = std::chrono::high_resolution_clock::now();
}

void timer::stop() noexcept {
	if (!_is_running) [[unlikely]] {
		std::cout
			<< "\033[34m[TIMER]\033[0m"
			<< "\033[33m[WARNING]\033[0m "
			<< "Trying to stop timer but timer is not running.\n"
		;
		return;
	}

	const double ELAPSED = get_elapsed();
	_is_running = false;

	const char* UNIT_SU = nullptr;
	switch (_TIME_UNIT) {
		case time_unit::MICRO_SECONDS: UNIT_SU = "us"; break;
		case time_unit::MILLI_SECONDS: UNIT_SU = "ms"; break;
		case time_unit::SECONDS:       UNIT_SU = "s"; break;
		default:                       UNIT_SU = "unknown"; break;
	}

	std::cout 
		<< "\033[34m[TIMER]\033[0m "
		<< std::format("\033[32m[{:.3f} {}]\033[0m : {}\n", ELAPSED, UNIT_SU, _DESC)
	;
}

void timer::reset() noexcept { start(); }

[[nodiscard]] double timer::get_elapsed() const noexcept {
	if (!_is_running) return 0.0;

	const auto END = std::chrono::high_resolution_clock::now();
	const auto DURATION = END - _start;

	switch (_TIME_UNIT) {
		case time_unit::MICRO_SECONDS:
			return std::chrono::duration_cast<std::chrono::microseconds>(DURATION).count();
		case time_unit::MILLI_SECONDS:
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(DURATION).count();
		case time_unit::SECONDS:
			return std::chrono::duration_cast<std::chrono::duration<double>>(DURATION).count();
		default: return 0.0;
	}
}

} // namespace warp