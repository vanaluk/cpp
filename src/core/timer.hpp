#pragma once

#include <chrono>
#include <cstdio>

/**
 * C++ Benchmark Kit - High Resolution Timer
 *
 * Standalone timer component for manual timing control.
 * For most use cases, prefer BenchmarkRunner which handles timing automatically.
 */

namespace benchmark_kit {

// Use constants from benchmark_runner.hpp if included together
#ifndef BENCHMARK_KIT_CONSTANTS_DEFINED
#define BENCHMARK_KIT_CONSTANTS_DEFINED
namespace constants {
constexpr double kNanosecondsPerSecond = 1'000'000'000.0;
constexpr double kNanosecondsPerMillisecond = 1'000'000.0;
constexpr double kNanosecondsPerMicrosecond = 1'000.0;
} // namespace constants
#endif

/**
 * High-resolution timer for precise measurements
 *
 * Usage:
 *   benchmark_kit::Timer timer;
 *   timer.start();
 *   // ... code to measure ...
 *   long long ns = timer.elapsed_nanoseconds();
 */
class Timer {
public:
	void start() {
		start_time_ = std::chrono::high_resolution_clock::now();
	}

	void reset() {
		start();
	}

	long long elapsed_nanoseconds() const {
		auto end_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
			end_time - start_time_).count();
	}

	double elapsed_microseconds() const {
		return static_cast<double>(elapsed_nanoseconds()) /
			constants::kNanosecondsPerMicrosecond;
	}

	double elapsed_milliseconds() const {
		return static_cast<double>(elapsed_nanoseconds()) /
			constants::kNanosecondsPerMillisecond;
	}

	double elapsed_seconds() const {
		return static_cast<double>(elapsed_nanoseconds()) /
			constants::kNanosecondsPerSecond;
	}

private:
	std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * RAII-style scoped timer that prints elapsed time on destruction
 *
 * Usage:
 *   {
 *       benchmark_kit::ScopedTimer timer("my_operation");
 *       // ... code to measure ...
 *   } // Prints: "my_operation: 123.45 ms"
 */
class ScopedTimer {
public:
	explicit ScopedTimer(const char* name) : name_(name) {
		timer_.start();
	}

	~ScopedTimer() {
		double ms = timer_.elapsed_milliseconds();
		std::printf("%s: %.2f ms\n", name_, ms);
	}

	// Non-copyable
	ScopedTimer(const ScopedTimer&) = delete;
	ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
	const char* name_;
	Timer timer_;
};

/**
 * Warmup helper function
 *
 * @param func Function to warm up
 * @param iterations Number of warmup iterations
 */
template<typename Func>
void warmup(Func&& func, int iterations = 100) {
	for (int i = 0; i < iterations; ++i) {
		func();
	}
}

} // namespace benchmark_kit
