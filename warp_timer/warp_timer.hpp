#pragma once

#ifndef WARP_TIMER
#define WARP_TIMER

#ifndef WARP_TOOLKIT_API
    #ifdef _WIN32
        #define WARP_TOOLKIT_API __declspec(dllexport)
    #else
        #define WARP_TOOLKIT_API
    #endif
#endif

#include <chrono>
#include <string>

namespace warp {

class WARP_TOOLKIT_API timer final {
private:
	const std::string _DESC;
	const std::chrono::time_point<std::chrono::high_resolution_clock> _START;

public:
#pragma region /// C-tors & D-tors

	explicit timer() noexcept
	: _DESC {""}
	, _START {std::chrono::high_resolution_clock::now()} {}

	explicit timer(std::string description) noexcept
	: _DESC {std::move(description)}
	, _START {std::chrono::high_resolution_clock::now()} {}

	~timer() noexcept;

#pragma endregion /// C-tors & D-tors
};

} /// namespace warp

#endif /// WARP_TIMER