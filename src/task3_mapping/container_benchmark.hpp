#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

/**
 * Benchmarks for different containers for mapping unique integer to string
 *
 * Options:
 * 1. std::map<int, std::string> - O(log n) lookup, ordered
 * 2. std::unordered_map<int, std::string> - O(1) average lookup, O(n) worst case
 * 3. std::vector<std::pair<int, std::string>> - for small sets, O(n) lookup
 */

struct BenchmarkResult {
	std::string container_name;
	long long insert_time_ns;
	long long lookup_time_ns;
	long long erase_time_ns;
	size_t memory_usage_bytes;
};

// Benchmark for std::map
BenchmarkResult benchmark_map(int element_count, int lookup_iterations);

// Benchmark for std::unordered_map
BenchmarkResult benchmark_unordered_map(int element_count, int lookup_iterations);

// Benchmark for std::vector (linear search)
BenchmarkResult benchmark_vector(int element_count, int lookup_iterations);

// Comparative analysis of all containers
void compare_containers(int element_count, int lookup_iterations);
