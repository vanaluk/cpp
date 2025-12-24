#pragma once

#include <chrono>

/**
 * Benchmark utilities
 */

namespace benchmark_constants {
constexpr double kNanosecondsPerSecond= 1'000'000'000.0;
constexpr int kDefaultWarmupIterations= 1000;
} // namespace benchmark_constants

// High resolution timer
class HighResolutionTimer {
public:
	void start() {
		start_time_= std::chrono::high_resolution_clock::now();
	}

	long long elapsed_nanoseconds() const {
		auto end_time= std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_).count();
	}

	double elapsed_seconds() const {
		return static_cast<double>(elapsed_nanoseconds()) / benchmark_constants::kNanosecondsPerSecond;
	}

private:
	std::chrono::high_resolution_clock::time_point start_time_;
};

// Warm-up function
template<typename Func>
void warmup(Func&& func, int iterations= benchmark_constants::kDefaultWarmupIterations) {
	for(int i= 0; i < iterations; ++i) {
		func();
	}
}
