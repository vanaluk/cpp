#pragma once

#include <chrono>
#include <string>

/**
 * Benchmark utilities
 */

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
		return elapsed_nanoseconds() / 1e9;
	}

private:
	std::chrono::high_resolution_clock::time_point start_time_;
};

// Warm-up function
template<typename Func>
void warmup(Func&& func, int iterations= 1000) {
	for(int i= 0; i < iterations; ++i) {
		func();
	}
}
