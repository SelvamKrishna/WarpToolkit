#include <chrono>
#include <iostream>

#include "warp_timer.hpp"

namespace warp {

timer::~timer() noexcept {
	const auto END = std::chrono::high_resolution_clock::now();
	const auto ELAPSED {
		std::chrono::duration_cast<std::chrono::microseconds>(END - _START).count()
	};

	std::cout 
		<< "\033[34m[TIMER]\033[0m" 
		<< std::format("\033[32m[{} micro]\033[0m ", ELAPSED)
		<< _DESC << "\n"
	;
}

} /// namespace warp