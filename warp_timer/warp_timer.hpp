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

enum class time_unit : uint8_t { MICRO_SECONDS, MILLI_SECONDS, SECONDS };

class WARP_TOOLKIT_API timer final {
private:
	const std::string _DESC {};
	std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	const time_unit _TIME_UNIT {time_unit::MILLI_SECONDS};
	bool _is_running {true};

public:
#pragma region /// C-tors & D-tors

	explicit timer() noexcept
	: _start {std::chrono::high_resolution_clock::now()} {}

	explicit timer(std::string description, time_unit time_unit = time_unit::MILLI_SECONDS) noexcept
	: _DESC {std::move(description)}
	, _TIME_UNIT {time_unit}
	, _start {std::chrono::high_resolution_clock::now()} {}

	~timer() noexcept;

#pragma endregion /// C-tors & D-tors
#pragma region /// Utility Functions

	void start() noexcept;
	void stop() noexcept;
	void reset() noexcept;

	[[nodiscard]] double get_elapsed() const noexcept;

#pragma endregion /// Utility Functions
};

} /// namespace warp

#endif /// WARP_TIMER