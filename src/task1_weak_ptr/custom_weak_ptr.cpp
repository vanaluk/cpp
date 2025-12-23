#include "custom_weak_ptr.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// Demonstration function
void demonstrate_weak_ptr_lock() {
	std::cout << "=== CustomWeakPtr::lock() Demonstration ===\n\n";

	// Create shared_ptr
	CustomSharedPtr<int> shared(new int(42));
	std::cout << "1. Created CustomSharedPtr, use_count = " << shared.use_count() << "\n";

	// Create weak_ptr
	CustomWeakPtr<int> weak(shared);
	std::cout << "2. Created CustomWeakPtr from shared_ptr, use_count = " << shared.use_count() << "\n";

	// Call lock() when object is alive
	{
		auto locked= weak.lock();
		if(locked) {
			std::cout << "3. lock() successful! use_count = " << shared.use_count()
					  << ", value = " << *locked << "\n";
		}
	}

	std::cout << "4. After exiting locked scope, use_count = "
			  << shared.use_count() << "\n";

	// Destroy shared_ptr
	shared= CustomSharedPtr<int>();
	std::cout << "5. shared_ptr destroyed\n";

	// Try to call lock() when object is dead
	auto locked_after= weak.lock();
	if(!locked_after) {
		std::cout << "6. lock() returned nullptr (object already deleted)\n";
	}

	std::cout << "\n=== Demonstration completed ===\n\n";
}

// Benchmark function
long long benchmark_weak_ptr_lock(int iterations, int thread_count) {
	auto start= std::chrono::high_resolution_clock::now();

	if(thread_count == 1) {
		// Single-threaded mode
		for(int i= 0; i < iterations; ++i) {
			CustomSharedPtr<int> shared(new int(i));
			CustomWeakPtr<int> weak(shared);
			auto locked= weak.lock();
			(void)locked; // suppress unused variable warning
		}
	} else {
		// Multi-threaded mode
		std::vector<std::thread> threads;
		int iterations_per_thread= iterations / thread_count;

		for(int t= 0; t < thread_count; ++t) {
			threads.emplace_back([iterations_per_thread]() {
				for(int i= 0; i < iterations_per_thread; ++i) {
					CustomSharedPtr<int> shared(new int(i));
					CustomWeakPtr<int> weak(shared);
					auto locked= weak.lock();
					(void)locked;
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
