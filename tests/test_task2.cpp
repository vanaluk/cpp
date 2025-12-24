#define BOOST_TEST_MODULE Task2Tests
#include <boost/test/unit_test.hpp>
#include <vector>
#include "task2_vector_erase/vector_erase.hpp"

BOOST_AUTO_TEST_SUITE(NaiveMethod)

BOOST_AUTO_TEST_CASE(test_basic) {
	std::vector<int> vec= {1, 2, 3, 4, 5, 6};
	erase_every_second_naive(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_empty) {
	std::vector<int> vec= {};
	erase_every_second_naive(vec);
	BOOST_CHECK(vec.empty());
}

BOOST_AUTO_TEST_CASE(test_single) {
	std::vector<int> vec= {1};
	erase_every_second_naive(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_two) {
	std::vector<int> vec= {1, 2};
	erase_every_second_naive(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_odd) {
	std::vector<int> vec= {1, 2, 3, 4, 5};
	erase_every_second_naive(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(RemoveIfMethod)

BOOST_AUTO_TEST_CASE(test_basic) {
	std::vector<int> vec= {1, 2, 3, 4, 5, 6};
	erase_every_second_remove_if(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_empty) {
	std::vector<int> vec= {};
	erase_every_second_remove_if(vec);
	BOOST_CHECK(vec.empty());
}

BOOST_AUTO_TEST_CASE(test_single) {
	std::vector<int> vec= {1};
	erase_every_second_remove_if(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_two) {
	std::vector<int> vec= {1, 2};
	erase_every_second_remove_if(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_odd) {
	std::vector<int> vec= {1, 2, 3, 4, 5};
	erase_every_second_remove_if(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(IteratorsMethod)

BOOST_AUTO_TEST_CASE(test_basic) {
	std::vector<int> vec= {1, 2, 3, 4, 5, 6};
	erase_every_second_iterators(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_empty) {
	std::vector<int> vec= {};
	erase_every_second_iterators(vec);
	BOOST_CHECK(vec.empty());
}

BOOST_AUTO_TEST_CASE(test_single) {
	std::vector<int> vec= {1};
	erase_every_second_iterators(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_two) {
	std::vector<int> vec= {1, 2};
	erase_every_second_iterators(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_odd) {
	std::vector<int> vec= {1, 2, 3, 4, 5};
	erase_every_second_iterators(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CopyMethod)

BOOST_AUTO_TEST_CASE(test_basic) {
	std::vector<int> vec= {1, 2, 3, 4, 5, 6};
	erase_every_second_copy(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_empty) {
	std::vector<int> vec= {};
	erase_every_second_copy(vec);
	BOOST_CHECK(vec.empty());
}

BOOST_AUTO_TEST_CASE(test_single) {
	std::vector<int> vec= {1};
	erase_every_second_copy(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_two) {
	std::vector<int> vec= {1, 2};
	erase_every_second_copy(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_odd) {
	std::vector<int> vec= {1, 2, 3, 4, 5};
	erase_every_second_copy(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PartitionMethod)

BOOST_AUTO_TEST_CASE(test_basic) {
	std::vector<int> vec= {1, 2, 3, 4, 5, 6};
	erase_every_second_partition(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_empty) {
	std::vector<int> vec= {};
	erase_every_second_partition(vec);
	BOOST_CHECK(vec.empty());
}

BOOST_AUTO_TEST_CASE(test_single) {
	std::vector<int> vec= {1};
	erase_every_second_partition(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_two) {
	std::vector<int> vec= {1, 2};
	erase_every_second_partition(vec);
	std::vector<int> expected= {1};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(test_odd) {
	std::vector<int> vec= {1, 2, 3, 4, 5};
	erase_every_second_partition(vec);
	std::vector<int> expected= {1, 3, 5};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		vec.begin(), vec.end(),
		expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// Performance Regression Tests
// ============================================================================
// These tests detect performance regressions by comparing against baseline thresholds.
// Thresholds are set conservatively (10x-20x of expected) to avoid flaky failures on CI.
//
// Expected performance (1000 elements, 100 iterations):
//   - naive:      ~1ms     (O(n²) - erase shifts elements)
//   - iterators:  ~1ms     (O(n²) - same as naive)
//   - remove_if:  ~0.07ms  (O(n) - single pass)
//   - copy:       ~0.06ms  (O(n) - single pass with allocation)
//   - partition:  ~0.06ms  (O(n) - in-place reordering)

BOOST_AUTO_TEST_SUITE(PerformanceRegressionSuite)

namespace {
// Test parameters
constexpr size_t VEC_SIZE= 1000;
constexpr int ITERATIONS= 100;

// Thresholds (10-20x safety margin for CI)
// O(n²) methods - slower, higher threshold
constexpr long long NAIVE_THRESHOLD_NS= 50'000'000; // 50ms
constexpr long long ITERATORS_THRESHOLD_NS= 50'000'000; // 50ms

// O(n) methods - faster, lower threshold
constexpr long long REMOVE_IF_THRESHOLD_NS= 10'000'000; // 10ms
constexpr long long COPY_THRESHOLD_NS= 10'000'000; // 10ms
constexpr long long PARTITION_THRESHOLD_NS= 10'000'000; // 10ms

// Conversion constants
constexpr double kNsToMs= 1'000'000.0; // Nanoseconds to milliseconds
constexpr double kPercentBase= 100.0;
constexpr int kSpeedupFactor= 3; // O(n) should be at least 3x faster than O(n²)

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

BOOST_AUTO_TEST_CASE(test_naive_performance) {
	long long time_ns= benchmark_vector_erase(
		erase_every_second_naive<int>, "naive", VEC_SIZE, ITERATIONS, 1);
	check_performance("erase_naive", time_ns, NAIVE_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_remove_if_performance) {
	long long time_ns= benchmark_vector_erase(
		erase_every_second_remove_if<int>, "remove_if", VEC_SIZE, ITERATIONS, 1);
	check_performance("erase_remove_if", time_ns, REMOVE_IF_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_iterators_performance) {
	long long time_ns= benchmark_vector_erase(
		erase_every_second_iterators<int>, "iterators", VEC_SIZE, ITERATIONS, 1);
	check_performance("erase_iterators", time_ns, ITERATORS_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_copy_performance) {
	long long time_ns= benchmark_vector_erase(
		erase_every_second_copy<int>, "copy", VEC_SIZE, ITERATIONS, 1);
	check_performance("erase_copy", time_ns, COPY_THRESHOLD_NS);
}

BOOST_AUTO_TEST_CASE(test_partition_performance) {
	long long time_ns= benchmark_vector_erase(
		erase_every_second_partition<int>, "partition", VEC_SIZE, ITERATIONS, 1);
	check_performance("erase_partition", time_ns, PARTITION_THRESHOLD_NS);
}

// Verify that O(n) methods are significantly faster than O(n²)
BOOST_AUTO_TEST_CASE(test_on_methods_faster_than_on2) {
	long long naive_time= benchmark_vector_erase(
		erase_every_second_naive<int>, "naive", VEC_SIZE, ITERATIONS, 1);
	long long copy_time= benchmark_vector_erase(
		erase_every_second_copy<int>, "copy", VEC_SIZE, ITERATIONS, 1);

	// O(n) method should be at least 3x faster than O(n²) on 1000 elements
	// (theoretically n/2 = 500x, but CI variance requires conservative threshold)
	BOOST_CHECK_MESSAGE(naive_time > copy_time * kSpeedupFactor,
		"O(n) copy method should be at least " << kSpeedupFactor << "x faster than O(n²) naive method: "
			<< "naive=" << static_cast<double>(naive_time) / kNsToMs << "ms, "
			<< "copy=" << static_cast<double>(copy_time) / kNsToMs << "ms");
}

BOOST_AUTO_TEST_SUITE_END()
