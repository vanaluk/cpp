#pragma once

#include <vector>
#include <algorithm>
#include <string>

// Function declarations for demonstration and benchmarks
void demonstrate_vector_erase();
long long benchmark_vector_erase(
	void (*method)(std::vector<int>&),
	const std::string& method_name,
	size_t vector_size,
	int iterations,
	int thread_count);

/**
 * Different methods for removing every second element from std::vector
 */

// Method 1: Naive approach - erase in loop (O(n²) due to element shifting)
template<typename T>
void erase_every_second_naive(std::vector<T>& vec) {
	for(size_t i= 1; i < vec.size(); i+= 1) {
		vec.erase(vec.begin() + i);
	}
}

// Method 2: Using remove_if + erase (erase-remove idiom)
template<typename T>
void erase_every_second_remove_if(std::vector<T>& vec) {
	bool keep= true;
	auto new_end= std::remove_if(vec.begin(), vec.end(),
		[&keep](const T&) {
			bool result= !keep;
			keep= !keep;
			return result;
		});
	vec.erase(new_end, vec.end());
}

// Method 3: Using iterators with index tracking
template<typename T>
void erase_every_second_iterators(std::vector<T>& vec) {
	auto it= vec.begin();
	size_t index= 0;

	while(it != vec.end()) {
		if(index % 2 == 1) {
			it= vec.erase(it);
		} else {
			++it;
		}
		++index;
	}
}

// Method 4: Copy to new vector (O(n), but extra memory)
template<typename T>
void erase_every_second_copy(std::vector<T>& vec) {
	std::vector<T> result;
	result.reserve((vec.size() + 1) / 2);

	for(size_t i= 0; i < vec.size(); i+= 2) {
		result.push_back(vec[i]);
	}

	vec= std::move(result);
}

// Method 5: Using std::stable_partition with index tracking
// Note: std::partition does not guarantee predicate call order, so we use
// std::stable_partition with index-based tracking via std::distance
template<typename T>
void erase_every_second_partition(std::vector<T>& vec) {
	auto partition_point= std::stable_partition(vec.begin(), vec.end(),
		[&vec](const T& elem) {
			// Calculate original index using pointer arithmetic
			size_t index= static_cast<size_t>(&elem - vec.data());
			return index % 2 == 0; // Keep elements at even indices
		});
	vec.erase(partition_point, vec.end());
}
