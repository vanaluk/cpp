#define BOOST_TEST_MODULE Task3Tests
#include <boost/test/unit_test.hpp>
#include "task3_mapping/container_benchmark.hpp"

BOOST_AUTO_TEST_SUITE(Task3TestSuite)

BOOST_AUTO_TEST_CASE(test_benchmark_map_returns_valid_result) {
	BenchmarkResult result= benchmark_map(1000, 1000);

	BOOST_CHECK(!result.container_name.empty());
	BOOST_CHECK_EQUAL(result.container_name, "std::map");
	BOOST_CHECK_GT(result.insert_time_ns, 0);
	BOOST_CHECK_GT(result.lookup_time_ns, 0);
	BOOST_CHECK_GT(result.erase_time_ns, 0);
	BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_benchmark_unordered_map_returns_valid_result) {
	BenchmarkResult result= benchmark_unordered_map(1000, 1000);

	BOOST_CHECK(!result.container_name.empty());
	BOOST_CHECK_EQUAL(result.container_name, "std::unordered_map");
	BOOST_CHECK_GT(result.insert_time_ns, 0);
	BOOST_CHECK_GT(result.lookup_time_ns, 0);
	BOOST_CHECK_GT(result.erase_time_ns, 0);
	BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_benchmark_vector_returns_valid_result) {
	BenchmarkResult result= benchmark_vector(1000, 1000);

	BOOST_CHECK(!result.container_name.empty());
	BOOST_CHECK_EQUAL(result.container_name, "std::vector<pair>");
	BOOST_CHECK_GT(result.insert_time_ns, 0);
	BOOST_CHECK_GT(result.lookup_time_ns, 0);
	BOOST_CHECK_GT(result.erase_time_ns, 0);
	BOOST_CHECK_GT(result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_container_name_is_set) {
	BenchmarkResult map_result= benchmark_map(100, 100);
	BenchmarkResult unordered_map_result= benchmark_unordered_map(100, 100);
	BenchmarkResult vector_result= benchmark_vector(100, 100);

	BOOST_CHECK(!map_result.container_name.empty());
	BOOST_CHECK(!unordered_map_result.container_name.empty());
	BOOST_CHECK(!vector_result.container_name.empty());

	BOOST_CHECK_EQUAL(map_result.container_name, "std::map");
	BOOST_CHECK_EQUAL(unordered_map_result.container_name, "std::unordered_map");
	BOOST_CHECK_EQUAL(vector_result.container_name, "std::vector<pair>");
}

BOOST_AUTO_TEST_CASE(test_memory_usage_is_positive) {
	BenchmarkResult map_result= benchmark_map(1000, 1000);
	BenchmarkResult unordered_map_result= benchmark_unordered_map(1000, 1000);
	BenchmarkResult vector_result= benchmark_vector(1000, 1000);

	BOOST_CHECK_GT(map_result.memory_usage_bytes, 0);
	BOOST_CHECK_GT(unordered_map_result.memory_usage_bytes, 0);
	BOOST_CHECK_GT(vector_result.memory_usage_bytes, 0);
}

BOOST_AUTO_TEST_CASE(test_insert_time_scales_with_size) {
	BenchmarkResult small= benchmark_map(100, 100);
	BenchmarkResult large= benchmark_map(10000, 100);

	// Larger container should take longer to insert
	BOOST_CHECK_GT(large.insert_time_ns, small.insert_time_ns);
}

BOOST_AUTO_TEST_CASE(test_lookup_time_scales_with_iterations) {
	BenchmarkResult few_iterations= benchmark_map(1000, 100);
	BenchmarkResult many_iterations= benchmark_map(1000, 10000);

	// More iterations should have larger lookup time
	BOOST_CHECK_GT(many_iterations.lookup_time_ns, few_iterations.lookup_time_ns);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// Performance Regression Tests
// ============================================================================
// These tests detect performance regressions by comparing against baseline thresholds.
// Thresholds are set conservatively (10x-20x of expected) to avoid flaky failures on CI.
// If a test fails, it shows how much the actual time exceeded the threshold.

BOOST_AUTO_TEST_SUITE(PerformanceRegressionSuite)

namespace {
// Baseline thresholds (nanoseconds per operation)
// Measured on typical CI runner with 10x safety margin

// std::map thresholds (10k elements, 100k lookups)
constexpr long long MAP_INSERT_THRESHOLD_NS= 100'000'000; // 100ms total
constexpr long long MAP_LOOKUP_THRESHOLD_NS= 500'000'000; // 500ms total
constexpr long long MAP_ERASE_THRESHOLD_NS= 50'000'000; // 50ms total

// std::unordered_map thresholds (10k elements, 100k lookups)
constexpr long long UMAP_INSERT_THRESHOLD_NS= 50'000'000; // 50ms total
constexpr long long UMAP_LOOKUP_THRESHOLD_NS= 100'000'000; // 100ms total
constexpr long long UMAP_ERASE_THRESHOLD_NS= 20'000'000; // 20ms total

// std::vector thresholds (1k elements, 1k lookups - smaller due to O(n) complexity)
constexpr long long VEC_INSERT_THRESHOLD_NS= 10'000'000; // 10ms total
constexpr long long VEC_LOOKUP_THRESHOLD_NS= 500'000'000; // 500ms total
constexpr long long VEC_ERASE_THRESHOLD_NS= 200'000'000; // 200ms total

// Test parameters
constexpr int MAP_ELEMENTS= 10000;
constexpr int MAP_LOOKUPS= 100000;
constexpr int VEC_ELEMENTS= 1000;
constexpr int VEC_LOOKUPS= 1000;

// Conversion constants
constexpr double kNsToMs= 1'000'000.0; // Nanoseconds to milliseconds
constexpr double kPercentBase= 100.0;

void check_performance(const char* operation, long long actual_ns, long long threshold_ns) {
	bool passed= actual_ns <= threshold_ns;
	if(!passed) {
		double exceeded_by= (static_cast<double>(actual_ns) / static_cast<double>(threshold_ns) - 1.0) * kPercentBase;
		BOOST_CHECK_MESSAGE(passed, operation << " exceeded threshold: " << static_cast<double>(actual_ns) / kNsToMs << "ms actual vs " << static_cast<double>(threshold_ns) / kNsToMs << "ms threshold " << "(+" << exceeded_by << "% over limit)");
	} else {
		double margin= (1.0 - static_cast<double>(actual_ns) / static_cast<double>(threshold_ns)) * kPercentBase;
		BOOST_CHECK_MESSAGE(passed, operation << ": " << static_cast<double>(actual_ns) / kNsToMs << "ms (" << margin << "% under threshold)");
	}
}
} // namespace

BOOST_AUTO_TEST_CASE(test_map_performance_regression) {
	BenchmarkResult result= benchmark_map(MAP_ELEMENTS, MAP_LOOKUPS);

	check_performance("std::map insert", result.insert_time_ns, MAP_INSERT_THRESHOLD_NS);
	check_performance("std::map lookup", result.lookup_time_ns, MAP_LOOKUP_THRESHOLD_NS);
	check_performance("std::map erase", result.erase_time_ns, MAP_ERASE_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_unordered_map_performance_regression) {
	BenchmarkResult result= benchmark_unordered_map(MAP_ELEMENTS, MAP_LOOKUPS);

	check_performance("std::unordered_map insert", result.insert_time_ns, UMAP_INSERT_THRESHOLD_NS);
	check_performance("std::unordered_map lookup", result.lookup_time_ns, UMAP_LOOKUP_THRESHOLD_NS);
	check_performance("std::unordered_map erase", result.erase_time_ns, UMAP_ERASE_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_vector_performance_regression) {
	// Use smaller dataset for vector due to O(n) complexity
	BenchmarkResult result= benchmark_vector(VEC_ELEMENTS, VEC_LOOKUPS);

	check_performance("std::vector insert", result.insert_time_ns, VEC_INSERT_THRESHOLD_NS);
	check_performance("std::vector lookup", result.lookup_time_ns, VEC_LOOKUP_THRESHOLD_NS);
	check_performance("std::vector erase", result.erase_time_ns, VEC_ERASE_THRESHOLD_NS);
}

BOOST_AUTO_TEST_SUITE_END()
