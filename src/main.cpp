#include <iostream>
#include "core/benchmark_kit.hpp"
#include "examples/weak_ptr/custom_weak_ptr.hpp"
#include "examples/vector_erase/vector_erase.hpp"
#include "examples/container_lookup/container_benchmark.hpp"

namespace {
constexpr int kDemoElementCount= 10000;
constexpr int kDemoLookupIterations= 100000;
} // namespace

int main() {
	std::cout << "╔══════════════════════════════════════════════════════════╗\n";
	std::cout << "║           C++ Benchmark Kit v" << benchmark_kit::VERSION << "                      ║\n";
	std::cout << "╠══════════════════════════════════════════════════════════╣\n";
	std::cout << "║  A flexible framework for benchmarking C++ code          ║\n";
	std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

	// Demonstrate example 1: weak_ptr::lock()
	std::cout << "=== Example 1: weak_ptr::lock() Implementation ===\n";
	demonstrate_weak_ptr_lock();

	// Demonstrate example 2: Vector erase methods
	std::cout << "\n=== Example 2: Vector Erase Methods ===\n";
	demonstrate_vector_erase();

	// Demonstrate example 3: Container comparison
	std::cout << "\n=== Example 3: Container Lookup Performance ===\n";
	compare_containers(kDemoElementCount, kDemoLookupIterations);

	// Quick demo of the new BenchmarkRunner API
	std::cout << "\n=== BenchmarkRunner API Demo ===\n";

	// Simple benchmark
	auto result= benchmark_kit::BenchmarkRunner::run("vector_push_back", []() {
		std::vector<int> v;
		for(int i= 0; i < 1000; ++i) {
			v.push_back(i);
		}
	});
	result.print();

	return 0;
}
