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

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <string_view>

namespace warp {

enum class time_unit : uint8_t { MICRO_SECONDS, MILLI_SECONDS, SECONDS };

namespace internal {
inline constexpr char time_unit_to_string(time_unit unit) noexcept {
	switch (unit) {
		case time_unit::MICRO_SECONDS: return 'u';
		case time_unit::MILLI_SECONDS: return 'm';
		case time_unit::SECONDS:       return '\x00';
		default:                       return '?';
	}
}

template <time_unit ToTimeUnit>
inline constexpr double time_unit_cast(double elapsed_ms) noexcept {
	switch (ToTimeUnit) {
		case time_unit::MICRO_SECONDS: return elapsed_ms * 1000.0; // ms → µs
		case time_unit::SECONDS:       return elapsed_ms / 1000.0; // ms → s
		default:                       return elapsed_ms;
	}
}

} /// namespace internal

class WARP_TOOLKIT_API timer final {
private:
	const std::string _DESC {};
	std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	const time_unit _TIME_UNIT {time_unit::MILLI_SECONDS};
	bool _is_running {true};

#pragma region /// Helper

	static void _log(std::string_view desc, double elapsed, time_unit time_unit) noexcept;

	static void _log_benchmark(std::string_view desc, std::vector<double> results, time_unit time_unit) noexcept;

	[[nodiscard]] double _get_time_since_start() const noexcept;

	[[nodiscard]] static double _measure_function_time_ms(const std::function<void()>& callable) noexcept;

#pragma endregion /// Helper
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

#pragma endregion /// Utility Functions
#pragma region /// Benchmarking Tools

	template <time_unit InTimeUnit>
	static double measure_function(std::string_view desc, const std::function<void()>& callable) noexcept {
		const double ELAPSED {internal::time_unit_cast<InTimeUnit>(_measure_function_time_ms(callable))};
		_log(desc, ELAPSED,InTimeUnit);
		return ELAPSED;
	}
	
	template <time_unit InTimeUnit>
	static void default_benchmark(
		std::string_view desc, 
		const std::function<void()>& callable, 
		uint32_t total_iterations = 8
	) noexcept {
		using namespace std::chrono;
		std::vector<double> results;
		results.reserve(total_iterations);

		while (total_iterations--) results.push_back(_measure_function_time_ms(callable));
		_log_benchmark(desc, results, InTimeUnit);
	}

#pragma endregion /// Benchmarking Tools
};

} /// namespace warp

#endif /// WARP_TIMER