#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

/**
 * C++ Benchmark Kit - Core Benchmark Runner
 *
 * A flexible framework for benchmarking arbitrary C++ functions with:
 * - High-resolution timing
 * - Warmup iterations
 * - Statistical analysis (mean, stddev, min, max, percentiles)
 * - Multi-threaded benchmarks
 * - Easy integration with custom functions
 *
 * Usage:
 *   // Simple benchmark
 *   auto result = BenchmarkRunner::run("my_function", []() {
 *       my_function();
 *   });
 *
 *   // With configuration
 *   BenchmarkConfig config;
 *   config.iterations = 10000;
 *   config.warmup_iterations = 1000;
 *   config.threads = 4;
 *
 *   auto result = BenchmarkRunner::run("parallel_task", config, [&]() {
 *       parallel_task();
 *   });
 */

namespace benchmark_kit {

// Constants - define guard to prevent redefinition
#ifndef BENCHMARK_KIT_CONSTANTS_DEFINED
#define BENCHMARK_KIT_CONSTANTS_DEFINED
namespace constants {
constexpr double kNanosecondsPerSecond = 1'000'000'000.0;
constexpr double kNanosecondsPerMillisecond = 1'000'000.0;
constexpr double kNanosecondsPerMicrosecond = 1'000.0;
constexpr int kDefaultIterations = 1000;
constexpr int kDefaultWarmupIterations = 100;
constexpr int kDefaultThreads = 1;
} // namespace constants
#else
namespace constants {
constexpr int kDefaultIterations = 1000;
constexpr int kDefaultWarmupIterations = 100;
constexpr int kDefaultThreads = 1;
} // namespace constants
#endif

/**
 * Configuration for benchmark execution
 */
struct BenchmarkConfig {
	int iterations = constants::kDefaultIterations;
	int warmup_iterations = constants::kDefaultWarmupIterations;
	int threads = constants::kDefaultThreads;
	bool collect_samples = true;  // Collect individual samples for statistics
	bool verbose = false;         // Print progress during benchmark
};

/**
 * Statistical results from a benchmark run
 */
struct BenchmarkStats {
	double mean_ns = 0.0;
	double stddev_ns = 0.0;
	double min_ns = 0.0;
	double max_ns = 0.0;
	double p50_ns = 0.0;  // Median
	double p95_ns = 0.0;
	double p99_ns = 0.0;
	std::vector<long long> samples;  // Individual timing samples

	// Convenience methods for different time units
	double mean_us() const { return mean_ns / constants::kNanosecondsPerMicrosecond; }
	double mean_ms() const { return mean_ns / constants::kNanosecondsPerMillisecond; }
	double mean_s() const { return mean_ns / constants::kNanosecondsPerSecond; }
};

/**
 * Complete benchmark result
 */
struct BenchmarkResult {
	std::string name;
	BenchmarkConfig config;
	BenchmarkStats stats;
	long long total_time_ns = 0;
	double operations_per_second = 0.0;
	bool success = true;
	std::string error_message;

	// Convenience accessors
	double total_time_ms() const {
		return static_cast<double>(total_time_ns) / constants::kNanosecondsPerMillisecond;
	}

	double total_time_s() const {
		return static_cast<double>(total_time_ns) / constants::kNanosecondsPerSecond;
	}

	// Print formatted result
	void print() const {
		std::cout << "\n=== Benchmark: " << name << " ===\n";
		std::cout << "Configuration:\n";
		std::cout << "  Iterations: " << config.iterations << "\n";
		std::cout << "  Warmup: " << config.warmup_iterations << "\n";
		std::cout << "  Threads: " << config.threads << "\n";
		std::cout << "\nResults:\n";
		std::cout << std::fixed << std::setprecision(2);
		std::cout << "  Total time: " << total_time_ms() << " ms\n";
		std::cout << "  Mean: " << stats.mean_ns << " ns (" << stats.mean_us() << " us)\n";
		std::cout << "  Std Dev: " << stats.stddev_ns << " ns\n";
		std::cout << "  Min: " << stats.min_ns << " ns\n";
		std::cout << "  Max: " << stats.max_ns << " ns\n";
		std::cout << "  P50 (Median): " << stats.p50_ns << " ns\n";
		std::cout << "  P95: " << stats.p95_ns << " ns\n";
		std::cout << "  P99: " << stats.p99_ns << " ns\n";
		std::cout << "  Ops/sec: " << std::scientific << operations_per_second << "\n";
		std::cout << std::fixed;
	}
};

/**
 * High-resolution timer for precise measurements
 */
class HighResolutionTimer {
public:
	void start() {
		start_time_ = std::chrono::high_resolution_clock::now();
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
 * Main benchmark runner class
 */
class BenchmarkRunner {
public:
	/**
	 * Run a benchmark with default configuration
	 *
	 * @param name Name of the benchmark for reporting
	 * @param func Function to benchmark (callable with no arguments)
	 * @return BenchmarkResult with timing statistics
	 *
	 * Example:
	 *   auto result = BenchmarkRunner::run("vector_push", []() {
	 *       std::vector<int> v;
	 *       for (int i = 0; i < 1000; ++i) v.push_back(i);
	 *   });
	 */
	template<typename Func>
	static BenchmarkResult run(const std::string& name, Func&& func) {
		return run(name, BenchmarkConfig{}, std::forward<Func>(func));
	}

	/**
	 * Run a benchmark with custom configuration
	 *
	 * @param name Name of the benchmark for reporting
	 * @param config Benchmark configuration
	 * @param func Function to benchmark
	 * @return BenchmarkResult with timing statistics
	 *
	 * Example:
	 *   BenchmarkConfig config;
	 *   config.iterations = 10000;
	 *   config.warmup_iterations = 500;
	 *
	 *   auto result = BenchmarkRunner::run("sort_benchmark", config, [&data]() {
	 *       std::sort(data.begin(), data.end());
	 *   });
	 */
	template<typename Func>
	static BenchmarkResult run(const std::string& name,
		const BenchmarkConfig& config,
		Func&& func) {

		BenchmarkResult result;
		result.name = name;
		result.config = config;

		// Warmup phase
		if (config.verbose) {
			std::cout << "Warming up (" << config.warmup_iterations << " iterations)...\n";
		}
		warmup(std::forward<Func>(func), config.warmup_iterations);

		// Benchmark phase
		if (config.verbose) {
			std::cout << "Running benchmark (" << config.iterations << " iterations)...\n";
		}

		std::vector<long long> samples;
		if (config.collect_samples) {
			samples.reserve(config.iterations);
		}

		HighResolutionTimer total_timer;
		total_timer.start();

		if (config.threads <= 1) {
			// Single-threaded benchmark
			for (int i = 0; i < config.iterations; ++i) {
				if (config.collect_samples) {
					HighResolutionTimer sample_timer;
					sample_timer.start();
					func();
					samples.push_back(sample_timer.elapsed_nanoseconds());
				} else {
					func();
				}
			}
		} else {
			// Multi-threaded benchmark
			samples = run_multithreaded(config, std::forward<Func>(func));
		}

		result.total_time_ns = total_timer.elapsed_nanoseconds();

		// Calculate statistics
		if (config.collect_samples && !samples.empty()) {
			result.stats = calculate_stats(samples);
		} else {
			// Estimate mean from total time
			result.stats.mean_ns = static_cast<double>(result.total_time_ns) /
				config.iterations;
		}

		// Calculate operations per second
		if (result.total_time_ns > 0) {
			result.operations_per_second =
				static_cast<double>(config.iterations) /
				(static_cast<double>(result.total_time_ns) / constants::kNanosecondsPerSecond);
		}

		return result;
	}

	/**
	 * Run a benchmark with setup/teardown per iteration
	 *
	 * @param name Name of the benchmark
	 * @param config Configuration
	 * @param setup Function called before each iteration (returns data for func)
	 * @param func Function to benchmark (receives setup result)
	 *
	 * Example:
	 *   auto result = BenchmarkRunner::run_with_setup(
	 *       "sort_benchmark",
	 *       config,
	 *       []() {
	 *           // Setup: create random data
	 *           std::vector<int> data(10000);
	 *           std::iota(data.begin(), data.end(), 0);
	 *           std::random_shuffle(data.begin(), data.end());
	 *           return data;
	 *       },
	 *       [](std::vector<int>& data) {
	 *           // Benchmark: sort the data
	 *           std::sort(data.begin(), data.end());
	 *       }
	 *   );
	 */
	template<typename SetupFunc, typename Func>
	static BenchmarkResult run_with_setup(
		const std::string& name,
		const BenchmarkConfig& config,
		SetupFunc&& setup,
		Func&& func) {

		BenchmarkResult result;
		result.name = name;
		result.config = config;

		// Warmup phase
		for (int i = 0; i < config.warmup_iterations; ++i) {
			auto data = setup();
			func(data);
		}

		// Benchmark phase
		std::vector<long long> samples;
		if (config.collect_samples) {
			samples.reserve(config.iterations);
		}

		HighResolutionTimer total_timer;
		total_timer.start();

		for (int i = 0; i < config.iterations; ++i) {
			auto data = setup();

			HighResolutionTimer sample_timer;
			sample_timer.start();
			func(data);
			long long sample_ns = sample_timer.elapsed_nanoseconds();

			if (config.collect_samples) {
				samples.push_back(sample_ns);
			}
		}

		result.total_time_ns = total_timer.elapsed_nanoseconds();

		// Note: total_time includes setup time, but samples contain only func time
		if (config.collect_samples && !samples.empty()) {
			result.stats = calculate_stats(samples);
			// Recalculate total based on samples (excluding setup)
			result.total_time_ns = std::accumulate(samples.begin(), samples.end(), 0LL);
		}

		if (result.total_time_ns > 0) {
			result.operations_per_second =
				static_cast<double>(config.iterations) /
				(static_cast<double>(result.total_time_ns) / constants::kNanosecondsPerSecond);
		}

		return result;
	}

	/**
	 * Compare multiple implementations
	 *
	 * @param benchmarks Vector of (name, function) pairs
	 * @param config Shared configuration for all benchmarks
	 * @return Vector of results
	 *
	 * Example:
	 *   std::vector<std::pair<std::string, std::function<void()>>> benchmarks = {
	 *       {"std::sort", [&]() { std::sort(data.begin(), data.end()); }},
	 *       {"std::stable_sort", [&]() { std::stable_sort(data.begin(), data.end()); }},
	 *   };
	 *
	 *   auto results = BenchmarkRunner::compare(benchmarks, config);
	 */
	static std::vector<BenchmarkResult> compare(
		const std::vector<std::pair<std::string, std::function<void()>>>& benchmarks,
		const BenchmarkConfig& config = BenchmarkConfig{}) {

		std::vector<BenchmarkResult> results;
		results.reserve(benchmarks.size());

		for (const auto& [name, func] : benchmarks) {
			results.push_back(run(name, config, func));
		}

		return results;
	}

	/**
	 * Print comparison table for multiple results
	 */
	static void print_comparison(const std::vector<BenchmarkResult>& results) {
		if (results.empty()) return;

		// Find the fastest
		auto fastest = std::min_element(results.begin(), results.end(),
			[](const BenchmarkResult& a, const BenchmarkResult& b) {
				return a.stats.mean_ns < b.stats.mean_ns;
			});

		std::cout << "\n";
		std::cout << std::string(90, '=') << "\n";
		std::cout << std::left << std::setw(30) << "Benchmark"
			<< std::right << std::setw(15) << "Mean (ns)"
			<< std::setw(15) << "Ops/sec"
			<< std::setw(15) << "vs Fastest"
			<< std::setw(15) << "Status\n";
		std::cout << std::string(90, '=') << "\n";

		for (const auto& r : results) {
			double ratio = r.stats.mean_ns / fastest->stats.mean_ns;
			std::string status = (&r == &(*fastest)) ? " FASTEST" : "";

			std::cout << std::left << std::setw(30) << r.name
				<< std::right << std::fixed << std::setprecision(2)
				<< std::setw(15) << r.stats.mean_ns
				<< std::scientific << std::setw(15) << r.operations_per_second
				<< std::fixed << std::setw(14) << ratio << "x"
				<< std::setw(15) << status << "\n";
		}

		std::cout << std::string(90, '=') << "\n";
	}

private:
	template<typename Func>
	static void warmup(Func&& func, int iterations) {
		for (int i = 0; i < iterations; ++i) {
			func();
		}
	}

	template<typename Func>
	static std::vector<long long> run_multithreaded(
		const BenchmarkConfig& config,
		Func&& func) {

		std::vector<long long> all_samples;
		std::vector<std::thread> threads;
		std::vector<std::vector<long long>> thread_samples(config.threads);

		int iterations_per_thread = config.iterations / config.threads;
		int extra_iterations = config.iterations % config.threads;

		for (int t = 0; t < config.threads; ++t) {
			int thread_iterations = iterations_per_thread +
				(t < extra_iterations ? 1 : 0);

			threads.emplace_back([&func, &thread_samples, t, thread_iterations,
				collect = config.collect_samples]() {
				if (collect) {
					thread_samples[t].reserve(thread_iterations);
				}

				for (int i = 0; i < thread_iterations; ++i) {
					if (collect) {
						HighResolutionTimer timer;
						timer.start();
						func();
						thread_samples[t].push_back(timer.elapsed_nanoseconds());
					} else {
						func();
					}
				}
			});
		}

		for (auto& t : threads) {
			t.join();
		}

		// Merge samples
		if (config.collect_samples) {
			for (const auto& ts : thread_samples) {
				all_samples.insert(all_samples.end(), ts.begin(), ts.end());
			}
		}

		return all_samples;
	}

	static BenchmarkStats calculate_stats(std::vector<long long>& samples) {
		BenchmarkStats stats;
		stats.samples = samples;

		if (samples.empty()) return stats;

		// Sort for percentiles
		std::sort(samples.begin(), samples.end());

		// Min/Max
		stats.min_ns = static_cast<double>(samples.front());
		stats.max_ns = static_cast<double>(samples.back());

		// Mean
		double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
		stats.mean_ns = sum / static_cast<double>(samples.size());

		// Standard deviation
		double sq_sum = 0.0;
		for (auto s : samples) {
			double diff = static_cast<double>(s) - stats.mean_ns;
			sq_sum += diff * diff;
		}
		stats.stddev_ns = std::sqrt(sq_sum / static_cast<double>(samples.size()));

		// Percentiles
		auto percentile = [&samples](double p) -> double {
			size_t idx = static_cast<size_t>(p * static_cast<double>(samples.size() - 1));
			return static_cast<double>(samples[idx]);
		};

		stats.p50_ns = percentile(0.50);
		stats.p95_ns = percentile(0.95);
		stats.p99_ns = percentile(0.99);

		return stats;
	}
};

/**
 * Macro for quick inline benchmarking (convenience)
 */
#define BENCHMARK_KIT_RUN(name, code) \
	benchmark_kit::BenchmarkRunner::run(name, [&]() { code; })

#define BENCHMARK_KIT_RUN_N(name, iterations, code) \
	benchmark_kit::BenchmarkRunner::run(name, \
		benchmark_kit::BenchmarkConfig{iterations}, [&]() { code; })

} // namespace benchmark_kit
