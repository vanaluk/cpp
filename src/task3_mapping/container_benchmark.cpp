#include "container_benchmark.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <unordered_map>
#include <vector>

namespace {

// Estimated node overhead for std::map (red-black tree node)
// Typical layout: 3 pointers (parent, left, right) + 1 color byte + padding
constexpr size_t MAP_NODE_OVERHEAD= 3 * sizeof(void*) + sizeof(int);

// Estimated node overhead for std::unordered_map (hash bucket entry)
// Typical layout: 1 pointer (next) + cached hash value
constexpr size_t UMAP_NODE_OVERHEAD= sizeof(void*) + sizeof(size_t);

} // namespace

BenchmarkResult benchmark_map(int element_count, int lookup_iterations) {
	std::map<int, std::string> container;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, element_count * 2);

	// Insert
	auto start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count; ++i) {
		container[i]= "value_" + std::to_string(i);
	}
	auto end= std::chrono::high_resolution_clock::now();
	long long insert_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Lookup
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < lookup_iterations; ++i) {
		int key= dis(gen) % element_count;
		auto it= container.find(key);
		(void)it; // suppress warning
	}
	end= std::chrono::high_resolution_clock::now();
	long long lookup_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Erase
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count / 10; ++i) {
		container.erase(i * 10);
	}
	end= std::chrono::high_resolution_clock::now();
	long long erase_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Approximate memory estimation
	// Each map node contains: key, value, and tree node overhead (pointers + color)
	size_t memory= sizeof(container) +
		static_cast<size_t>(element_count) * (sizeof(int) + sizeof(std::string) + MAP_NODE_OVERHEAD);

	return {"std::map", insert_time, lookup_time, erase_time, memory};
}

BenchmarkResult benchmark_unordered_map(int element_count, int lookup_iterations) {
	std::unordered_map<int, std::string> container;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, element_count * 2);

	// Insert
	auto start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count; ++i) {
		container[i]= "value_" + std::to_string(i);
	}
	auto end= std::chrono::high_resolution_clock::now();
	long long insert_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Lookup
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < lookup_iterations; ++i) {
		int key= dis(gen) % element_count;
		auto it= container.find(key);
		(void)it;
	}
	end= std::chrono::high_resolution_clock::now();
	long long lookup_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Erase
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count / 10; ++i) {
		container.erase(i * 10);
	}
	end= std::chrono::high_resolution_clock::now();
	long long erase_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Approximate memory estimation
	// Each unordered_map node contains: key, value, and hash bucket overhead
	size_t memory= sizeof(container) +
		static_cast<size_t>(element_count) * (sizeof(int) + sizeof(std::string) + UMAP_NODE_OVERHEAD);

	return {"std::unordered_map", insert_time, lookup_time, erase_time, memory};
}

BenchmarkResult benchmark_vector(int element_count, int lookup_iterations) {
	std::vector<std::pair<int, std::string>> container;
	container.reserve(element_count);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, element_count * 2);

	// Insert
	auto start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count; ++i) {
		container.push_back({i, "value_" + std::to_string(i)});
	}
	auto end= std::chrono::high_resolution_clock::now();
	long long insert_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Lookup (linear)
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < lookup_iterations; ++i) {
		int key= dis(gen) % element_count;
		auto it= std::find_if(container.begin(), container.end(),
			[key](const std::pair<int, std::string>& p) { return p.first == key; });
		(void)it;
	}
	end= std::chrono::high_resolution_clock::now();
	long long lookup_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Erase
	start= std::chrono::high_resolution_clock::now();
	for(int i= 0; i < element_count / 10; ++i) {
		auto it= std::find_if(container.begin(), container.end(),
			[i](const std::pair<int, std::string>& p) { return p.first == i * 10; });
		if(it != container.end()) {
			container.erase(it);
		}
	}
	end= std::chrono::high_resolution_clock::now();
	long long erase_time= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Memory estimation for vector - straightforward, no node overhead
	size_t memory= sizeof(container) +
		static_cast<size_t>(element_count) * sizeof(std::pair<int, std::string>);

	return {"std::vector<pair>", insert_time, lookup_time, erase_time, memory};
}

void compare_containers(int element_count, int lookup_iterations) {
	std::cout << "=== Comparing containers for int -> string mapping ===\n\n";
	std::cout << "Parameters: elements = " << element_count
			  << ", lookup iterations = " << lookup_iterations << "\n\n";

	auto map_result= benchmark_map(element_count, lookup_iterations);
	auto unordered_map_result= benchmark_unordered_map(element_count, lookup_iterations);
	auto vector_result= benchmark_vector(element_count, lookup_iterations);

	std::cout << "Container          | Insert (ns)  | Lookup (ns)   | Erase (ns) | Memory (bytes)\n";
	std::cout << "-------------------|---------------|---------------|---------------|---------------\n";
	printf("%-18s | %13lld | %13lld | %13lld | %13zu\n",
		map_result.container_name.c_str(), map_result.insert_time_ns,
		map_result.lookup_time_ns, map_result.erase_time_ns, map_result.memory_usage_bytes);
	printf("%-18s | %13lld | %13lld | %13lld | %13zu\n",
		unordered_map_result.container_name.c_str(), unordered_map_result.insert_time_ns,
		unordered_map_result.lookup_time_ns, unordered_map_result.erase_time_ns,
		unordered_map_result.memory_usage_bytes);
	printf("%-18s | %13lld | %13lld | %13lld | %13zu\n",
		vector_result.container_name.c_str(), vector_result.insert_time_ns,
		vector_result.lookup_time_ns, vector_result.erase_time_ns, vector_result.memory_usage_bytes);

	std::cout << "\nRecommendation:\n";
	std::cout << "- For large datasets (>1000 elements): std::unordered_map\n";
	std::cout << "- If ordering is needed: std::map\n";
	std::cout << "- For small datasets (<100 elements): std::vector may be faster\n";
	std::cout << "\n";
}
