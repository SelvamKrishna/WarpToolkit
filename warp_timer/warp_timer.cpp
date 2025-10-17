#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include <numeric>

#include "warp_timer.hpp"

namespace warp {

void timer::_log(std::string_view desc, double elapsed, time_unit time_unit) noexcept {
	std::cout
		<< std::format(
			"\033[34m[TIMER]\033[0m\033[32m[{:.3f} {}s]\033[0m : {}\n"
			, elapsed
			, internal::time_unit_to_string(time_unit)
			, desc
		)
	;
}

void timer::_log_benchmark(std::string_view desc, std::vector<double> results, time_unit time_unit) noexcept {
	if (results.empty()) [[unlikely]] {
		std::cout
			<< "\033[34m[TIMER][BENCHMARK]\033[0m\033[33m[WARNING]\033[0m : "
			<< "Trying to benchmark empty results\n"
		;
	}

	std::sort(results.begin(), results.end());
	const size_t SIZE {results.size()};
	const double MEAN {std::accumulate(results.begin(), results.end(), 0.0) / SIZE};

	// If odd number of results the middle value
	// Else the average of the 2 middle values
	const double MEDIAN {
		(SIZE % 2 == 0) ? (results[SIZE / 2 - 1] + results[SIZE / 2]) / 2.0 : results[SIZE / 2]
	};

	const double MODE {
		[&results, &SIZE] {
			double max_value {results[0]};
			uint8_t max_count {1};
			uint8_t current_count {1};

			// Array already sorted so all equivalent values are arranged together
			for (size_t i = 1; i < SIZE; i++) {
				current_count = (results[i] == results[i - 1]) ? current_count + 1 : 1;

				if (current_count > max_count) {
					max_value = results[i];
					max_count = current_count;
				}
			}

			return max_value;
		}()
	};

	const char U = internal::time_unit_to_string(time_unit);
	std::cout
		<< std::format("\033[34m[TIMER][BENCHMARK]\033[0m : {}\n" , desc)
		<< std::format("\t\033[32m[MEAN]\033[0m   : {:.3f} {}s\n", MEAN, U)
		<< std::format("\t\033[32m[MEDIAN]\033[0m : {:.3f} {}s\n", MEDIAN, U)
		<< std::format("\t\033[32m[MODE]\033[0m   : {:.3f} {}s\n", MODE, U)
	;
}

[[nodiscard]] double timer::_get_time_since_start() const noexcept {
	using namespace std::chrono;

	if (!_is_running) return 0.0;

	const auto END = high_resolution_clock::now();

	switch (_TIME_UNIT) {
		case time_unit::MICRO_SECONDS: return duration_cast<microseconds>(END - _start).count();
		case time_unit::MILLI_SECONDS: return duration_cast<duration<double, std::milli>>(END - _start).count();
		case time_unit::SECONDS: return duration_cast<duration<double>>(END - _start).count();
		default: return 0.0;
	}
}

[[nodiscard]] double timer::_measure_function_time_ms(const std::function<void()>& callable) noexcept {
	using namespace std::chrono;

	const auto START = high_resolution_clock::now();
	callable();
	const auto END = high_resolution_clock::now();

	return duration<double, std::milli>(END - START).count();
}

timer::~timer() noexcept { if (_is_running) stop(); }

void timer::start() noexcept {
	_is_running = true;
	_start = std::chrono::high_resolution_clock::now();
}

void timer::stop() noexcept {
	if (!_is_running) [[unlikely]] {
		std::cout
			<< "\033[34m[TIMER]\033[0m\033[33m[WARNING]\033[0m : "
			<< "Trying to stop timer but timer is not running.\n"
		;
		return;
	}

	const double ELAPSED = _get_time_since_start();
	_is_running = false;
	_log(_DESC, ELAPSED, _TIME_UNIT);
}

void timer::reset() noexcept { start(); }

} /// namespace warp