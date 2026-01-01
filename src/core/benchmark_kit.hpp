#pragma once

/**
 * C++ Benchmark Kit - Main Header
 *
 * Include this single header to access all benchmark kit functionality.
 *
 * Usage:
 *   #include "core/benchmark_kit.hpp"
 *
 *   using namespace benchmark_kit;
 *
 *   int main() {
 *       auto result = BenchmarkRunner::run("my_test", []() {
 *           // Your code here
 *       });
 *       result.print();
 *   }
 */

#include "benchmark_runner.hpp"
#include "timer.hpp"
#include "statistics.hpp"

namespace benchmark_kit {

// Version info
constexpr const char* VERSION = "1.0.0";
constexpr const char* PROJECT_NAME = "cpp-benchmark-kit";

/**
 * Quick benchmark function for simple use cases
 *
 * @param name Benchmark name
 * @param iterations Number of iterations
 * @param func Function to benchmark
 * @return BenchmarkResult
 *
 * Example:
 *   auto r = benchmark_kit::quick_bench("test", 1000, my_func);
 */
template<typename Func>
inline BenchmarkResult quick_bench(const std::string& name, int iterations, Func&& func) {
	BenchmarkConfig config;
	config.iterations = iterations;
	config.warmup_iterations = iterations / 10;
	return BenchmarkRunner::run(name, config, std::forward<Func>(func));
}

/**
 * Quick comparison of multiple implementations
 *
 * @param benchmarks Vector of (name, function) pairs
 * @param iterations Number of iterations for each
 *
 * Example:
 *   benchmark_kit::quick_compare({
 *       {"method_a", method_a},
 *       {"method_b", method_b},
 *   }, 10000);
 */
inline void quick_compare(
	const std::vector<std::pair<std::string, std::function<void()>>>& benchmarks,
	int iterations = 1000) {

	BenchmarkConfig config;
	config.iterations = iterations;
	config.warmup_iterations = iterations / 10;

	auto results = BenchmarkRunner::compare(benchmarks, config);
	BenchmarkRunner::print_comparison(results);
}

} // namespace benchmark_kit
