#include "vector_erase.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <numeric>
#include <iostream>
#include <string>

// Demonstration function
void demonstrate_vector_erase() {
	std::cout << "=== Demonstration of removing every second element ===\n\n";

	std::vector<int> vec= {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	std::cout << "Original vector: ";
	for(int x: vec)
		std::cout << x << " ";
	std::cout << "\n";

	// Method 4 (copy) - safest for demonstration
	std::vector<int> vec_copy= vec;
	erase_every_second_copy(vec_copy);

	std::cout << "After removing every second: ";
	for(int x: vec_copy)
		std::cout << x << " ";
	std::cout << "\n\n";
}

// Benchmark function
long long benchmark_vector_erase(
	void (*method)(std::vector<int>&),
	[[maybe_unused]] const std::string& method_name,
	size_t vector_size,
	int iterations,
	int thread_count) {
	auto start= std::chrono::high_resolution_clock::now();

	if(thread_count == 1) {
		// Single-threaded mode
		for(int i= 0; i < iterations; ++i) {
			std::vector<int> vec(vector_size);
			std::iota(vec.begin(), vec.end(), 0);
			method(vec);
		}
	} else {
		// Multi-threaded mode
		std::vector<std::thread> threads;
		int iterations_per_thread= iterations / thread_count;

		for(int t= 0; t < thread_count; ++t) {
			threads.emplace_back([method, iterations_per_thread, vector_size]() {
				for(int i= 0; i < iterations_per_thread; ++i) {
					std::vector<int> vec(vector_size);
					std::iota(vec.begin(), vec.end(), 0);
					method(vec);
				}
			});
		}

		for(auto& thread: threads) {
			thread.join();
		}
	}

	auto end= std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}
