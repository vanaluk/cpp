#define BOOST_TEST_MODULE Task3Tests
#include <boost/test/unit_test.hpp>
#include "task3_mapping/container_benchmark.hpp"

BOOST_AUTO_TEST_SUITE(Task3TestSuite)

BOOST_AUTO_TEST_CASE(test_benchmark_map_returns_valid_result) {
    BenchmarkResult result = benchmark_map(1000, 1000);
    
    BOOST_CHECK(!result.container_name.empty());
    BOOST_CHECK_EQUAL(result.container_name, "std::map");
    BOOST_CHECK_GT(result.insert_time_ns, 0);
    BOOST_CHECK_GT(result.lookup_time_ns, 0);
    BOOST_CHECK_GT(result.erase_time_ns, 0);
    BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_benchmark_unordered_map_returns_valid_result) {
    BenchmarkResult result = benchmark_unordered_map(1000, 1000);
    
    BOOST_CHECK(!result.container_name.empty());
    BOOST_CHECK_EQUAL(result.container_name, "std::unordered_map");
    BOOST_CHECK_GT(result.insert_time_ns, 0);
    BOOST_CHECK_GT(result.lookup_time_ns, 0);
    BOOST_CHECK_GT(result.erase_time_ns, 0);
    BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_benchmark_vector_returns_valid_result) {
    BenchmarkResult result = benchmark_vector(1000, 1000);
    
    BOOST_CHECK(!result.container_name.empty());
    BOOST_CHECK_EQUAL(result.container_name, "std::vector<pair>");
    BOOST_CHECK_GT(result.insert_time_ns, 0);
    BOOST_CHECK_GT(result.lookup_time_ns, 0);
    BOOST_CHECK_GT(result.erase_time_ns, 0);
    BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_container_name_is_set) {
    BenchmarkResult map_result = benchmark_map(100, 100);
    BenchmarkResult unordered_map_result = benchmark_unordered_map(100, 100);
    BenchmarkResult vector_result = benchmark_vector(100, 100);
    
    BOOST_CHECK(!map_result.container_name.empty());
    BOOST_CHECK(!unordered_map_result.container_name.empty());
    BOOST_CHECK(!vector_result.container_name.empty());
    
    BOOST_CHECK_EQUAL(map_result.container_name, "std::map");
    BOOST_CHECK_EQUAL(unordered_map_result.container_name, "std::unordered_map");
    BOOST_CHECK_EQUAL(vector_result.container_name, "std::vector<pair>");
}

BOOST_AUTO_TEST_CASE(test_memory_usage_is_positive) {
    BenchmarkResult map_result = benchmark_map(1000, 1000);
    BenchmarkResult unordered_map_result = benchmark_unordered_map(1000, 1000);
    BenchmarkResult vector_result = benchmark_vector(1000, 1000);
    
    BOOST_CHECK_GT(map_result.memory_usage_bytes, 0);
    BOOST_CHECK_GT(unordered_map_result.memory_usage_bytes, 0);
    BOOST_CHECK_GT(vector_result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_insert_time_scales_with_size) {
    BenchmarkResult small = benchmark_map(100, 100);
    BenchmarkResult large = benchmark_map(10000, 100);
    
    // Larger container should take longer to insert
    BOOST_CHECK_GT(large.insert_time_ns, small.insert_time_ns);
}

BOOST_AUTO_TEST_CASE(test_lookup_time_scales_with_iterations) {
    BenchmarkResult few_iterations = benchmark_map(1000, 100);
    BenchmarkResult many_iterations = benchmark_map(1000, 10000);
    
    // More iterations should have larger lookup time
    BOOST_CHECK_GT(many_iterations.lookup_time_ns, few_iterations.lookup_time_ns);
}

BOOST_AUTO_TEST_CASE(test_vector_lookup_slower_than_map) {
	const int elements= 50000;
	const int lookups= 50000;

	BenchmarkResult map_result = benchmark_map(elements, lookups);
    BenchmarkResult vec_result = benchmark_vector(elements, lookups);

	// Vector O(n) lookup should be significantly slower than map O(log n) lookup
	// Use 2x factor to avoid flaky results on CI runners
	BOOST_CHECK_GT(vec_result.lookup_time_ns, map_result.lookup_time_ns * 2);
}

BOOST_AUTO_TEST_SUITE_END()
