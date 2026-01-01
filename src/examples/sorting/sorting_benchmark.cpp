/**
 * Example: Sorting Algorithm Benchmark
 *
 * This example demonstrates how to use cpp-benchmark-kit to benchmark
 * custom sorting implementations.
 */

#include "core/benchmark_kit.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

using namespace benchmark_kit;

// ============================================================================
// Custom QuickSort Implementation
// ============================================================================

namespace sorting {

/**
 * Partition function for QuickSort
 * Uses Lomuto partition scheme
 */
template<typename T>
int partition(std::vector<T>& arr, int low, int high) {
    T pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

/**
 * QuickSort recursive implementation
 */
template<typename T>
void quicksort(std::vector<T>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

/**
 * QuickSort wrapper for user convenience
 */
template<typename T>
void quicksort(std::vector<T>& arr) {
    if (!arr.empty()) {
        quicksort(arr, 0, static_cast<int>(arr.size()) - 1);
    }
}

/**
 * Optimized QuickSort with insertion sort for small arrays
 */
template<typename T>
void insertion_sort(std::vector<T>& arr, int low, int high) {
    for (int i = low + 1; i <= high; ++i) {
        T key = arr[i];
        int j = i - 1;
        while (j >= low && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

template<typename T>
void quicksort_optimized(std::vector<T>& arr, int low, int high) {
    constexpr int kInsertionThreshold = 16;

    if (high - low < kInsertionThreshold) {
        insertion_sort(arr, low, high);
        return;
    }

    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_optimized(arr, low, pi - 1);
        quicksort_optimized(arr, pi + 1, high);
    }
}

template<typename T>
void quicksort_optimized(std::vector<T>& arr) {
    if (!arr.empty()) {
        quicksort_optimized(arr, 0, static_cast<int>(arr.size()) - 1);
    }
}

} // namespace sorting

// ============================================================================
// Helper: Generate random data
// ============================================================================

std::vector<int> generate_random_data(size_t size) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(size * 10));

    for (auto& val : data) {
        val = dist(gen);
    }
    return data;
}

// ============================================================================
// Main: Run sorting benchmarks
// ============================================================================

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Sorting Algorithm Benchmark Example                ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Comparing: std::sort vs QuickSort vs QuickSort+Insertion║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    constexpr size_t kDataSize = 10000;
    constexpr int kIterations = 100;
    constexpr int kWarmup = 10;

    std::cout << "Configuration:\n";
    std::cout << "  Array size: " << kDataSize << " elements\n";
    std::cout << "  Iterations: " << kIterations << "\n";
    std::cout << "  Warmup: " << kWarmup << " iterations\n\n";

    // Prepare base data
    auto base_data = generate_random_data(kDataSize);

    // ========================================================================
    // Method 1: Using run_with_setup (recommended for sorting)
    // Setup generates fresh random data for each iteration
    // ========================================================================

    std::cout << "=== Method 1: run_with_setup (fresh data each iteration) ===\n\n";

    BenchmarkConfig config;
    config.iterations = kIterations;
    config.warmup_iterations = kWarmup;

    // Benchmark std::sort
    auto result_std_sort = BenchmarkRunner::run_with_setup(
        "std::sort",
        config,
        [kDataSize]() {
            return generate_random_data(kDataSize);
        },
        [](std::vector<int>& data) {
            std::sort(data.begin(), data.end());
        }
    );

    // Benchmark custom QuickSort
    auto result_quicksort = BenchmarkRunner::run_with_setup(
        "QuickSort (Lomuto)",
        config,
        [kDataSize]() {
            return generate_random_data(kDataSize);
        },
        [](std::vector<int>& data) {
            sorting::quicksort(data);
        }
    );

    // Benchmark optimized QuickSort
    auto result_quicksort_opt = BenchmarkRunner::run_with_setup(
        "QuickSort + Insertion",
        config,
        [kDataSize]() {
            return generate_random_data(kDataSize);
        },
        [](std::vector<int>& data) {
            sorting::quicksort_optimized(data);
        }
    );

    // Benchmark std::stable_sort
    auto result_stable_sort = BenchmarkRunner::run_with_setup(
        "std::stable_sort",
        config,
        [kDataSize]() {
            return generate_random_data(kDataSize);
        },
        [](std::vector<int>& data) {
            std::stable_sort(data.begin(), data.end());
        }
    );

    // Print individual results
    result_std_sort.print();
    result_quicksort.print();
    result_quicksort_opt.print();
    result_stable_sort.print();

    // ========================================================================
    // Method 2: Using compare() for side-by-side comparison
    // ========================================================================

    std::cout << "\n=== Method 2: compare() with formatted table ===\n";

    // Need to use copies since each benchmark mutates data
    std::vector<std::pair<std::string, std::function<void()>>> benchmarks = {
        {"std::sort", [&base_data]() {
            auto copy = base_data;
            std::sort(copy.begin(), copy.end());
        }},
        {"QuickSort (Lomuto)", [&base_data]() {
            auto copy = base_data;
            sorting::quicksort(copy);
        }},
        {"QuickSort + Insertion", [&base_data]() {
            auto copy = base_data;
            sorting::quicksort_optimized(copy);
        }},
        {"std::stable_sort", [&base_data]() {
            auto copy = base_data;
            std::stable_sort(copy.begin(), copy.end());
        }},
    };

    auto comparison_results = BenchmarkRunner::compare(benchmarks, config);
    BenchmarkRunner::print_comparison(comparison_results);

    // ========================================================================
    // Method 3: Quick one-liner using quick_bench
    // ========================================================================

    std::cout << "\n=== Method 3: quick_bench one-liner ===\n";

    auto quick_result = quick_bench("std::sort (quick)", 50, [&base_data]() {
        auto copy = base_data;
        std::sort(copy.begin(), copy.end());
    });

    std::cout << "Mean time: " << quick_result.stats.mean_us() << " µs\n";
    std::cout << "Ops/sec: " << quick_result.operations_per_second << "\n";

    // ========================================================================
    // Verify correctness
    // ========================================================================

    std::cout << "\n=== Correctness Verification ===\n";

    auto test_data = generate_random_data(1000);
    auto test_copy1 = test_data;
    auto test_copy2 = test_data;

    std::sort(test_copy1.begin(), test_copy1.end());
    sorting::quicksort(test_copy2);

    bool correct = (test_copy1 == test_copy2);
    std::cout << "QuickSort produces same result as std::sort: "
              << (correct ? "YES ✓" : "NO ✗") << "\n";

    return 0;
}

