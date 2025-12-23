#include <iostream>
#include "task1_weak_ptr/custom_weak_ptr.hpp"
#include "task2_vector_erase/vector_erase.hpp"
#include "task3_mapping/container_benchmark.hpp"

int main() {
	std::cout << "C++ Interview Demo Project\n";
	std::cout << "==========================\n\n";

	// Demonstrate task 1
	demonstrate_weak_ptr_lock();

	// Demonstrate task 2
	demonstrate_vector_erase();

	// Demonstrate task 3
	compare_containers(10000, 100000);

	return 0;
}
