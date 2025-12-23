#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "../task1_weak_ptr/custom_weak_ptr.hpp"
#include "../task2_vector_erase/vector_erase.hpp"
#include "../task3_mapping/container_benchmark.hpp"

namespace py= pybind11;

PYBIND11_MODULE(cpp_interview_bindings, m) {
	m.doc()= "C++ Interview Demo Project - Python bindings";

	// Task 1: weak_ptr::lock()
	m.def("demonstrate_weak_ptr_lock", &demonstrate_weak_ptr_lock,
		"Demonstration of CustomWeakPtr::lock() operation");

	m.def("benchmark_weak_ptr_lock", &benchmark_weak_ptr_lock,
		"Benchmark for weak_ptr::lock()",
		py::arg("iterations"), py::arg("thread_count")= 1);

	// Task 2: Removing every second element
	m.def("demonstrate_vector_erase", &demonstrate_vector_erase,
		"Demonstration of removing every second element");

	// Export erase functions
	m.def("erase_every_second_naive", &erase_every_second_naive<int>,
		"Naive erase method");
	m.def("erase_every_second_remove_if", &erase_every_second_remove_if<int>,
		"Method using remove_if + erase");
	m.def("erase_every_second_iterators", &erase_every_second_iterators<int>,
		"Method using iterators");
	m.def("erase_every_second_copy", &erase_every_second_copy<int>,
		"Method copying to new vector");
	m.def("erase_every_second_partition", &erase_every_second_partition<int>,
		"Method using partition");

	// Task 3: Mapping number to string
	m.def("compare_containers", &compare_containers,
		"Comparing containers for int -> string mapping",
		py::arg("element_count"), py::arg("lookup_iterations"));

	m.def("benchmark_map", &benchmark_map,
		"Benchmark for std::map",
		py::arg("element_count"), py::arg("lookup_iterations"));

	m.def("benchmark_unordered_map", &benchmark_unordered_map,
		"Benchmark for std::unordered_map",
		py::arg("element_count"), py::arg("lookup_iterations"));

	m.def("benchmark_vector", &benchmark_vector,
		"Benchmark for std::vector",
		py::arg("element_count"), py::arg("lookup_iterations"));
}
