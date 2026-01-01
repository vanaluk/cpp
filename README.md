# C++ Benchmark Kit

A flexible, high-performance framework for benchmarking arbitrary C++ code with Ultra Low Latency (ULL) optimizations.

## Features

- **Template-based BenchmarkRunner** — benchmark any callable (lambdas, functions, functors)
- **Statistical Analysis** — mean, stddev, min, max, percentiles (P50, P95, P99)
- **Multi-threaded Benchmarks** — easily test parallel performance
- **Setup/Teardown Support** — exclude setup time from measurements
- **Comparison Mode** — compare multiple implementations side-by-side
- **Ultra Low Latency Flags** — Release build with aggressive optimizations
- **REST API Server** — run benchmarks via HTTP (Boost.Asio)
- **PostgreSQL Storage** — persist and analyze benchmark results
- **Python Bindings** — call C++ benchmarks from Python (pybind11)

## Quick Start

### Adding Your Own Benchmark

The simplest way to benchmark your code:

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    // Simple benchmark with default config (1000 iterations)
    auto result = BenchmarkRunner::run("my_function", []() {
        // Your code here
        my_function();
    });
    
    result.print();
    return 0;
}
```

### Custom Configuration

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    BenchmarkConfig config;
    config.iterations = 10000;        // Number of benchmark iterations
    config.warmup_iterations = 1000;  // Warmup before timing
    config.threads = 4;               // Multi-threaded benchmark
    config.verbose = true;            // Print progress

    auto result = BenchmarkRunner::run("parallel_sort", config, [&data]() {
        std::sort(std::execution::par, data.begin(), data.end());
    });

    result.print();

    // Access individual statistics
    std::cout << "Mean: " << result.stats.mean_ns << " ns\n";
    std::cout << "P99:  " << result.stats.p99_ns << " ns\n";
    std::cout << "Ops/sec: " << result.operations_per_second << "\n";
}
```

### Comparing Multiple Implementations

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    std::vector<int> data(100000);
    
    // Compare different sorting algorithms
    std::vector<std::pair<std::string, std::function<void()>>> benchmarks = {
        {"std::sort", [&]() {
            auto copy = data;
            std::sort(copy.begin(), copy.end());
        }},
        {"std::stable_sort", [&]() {
            auto copy = data;
            std::stable_sort(copy.begin(), copy.end());
        }},
        {"std::partial_sort", [&]() {
            auto copy = data;
            std::partial_sort(copy.begin(), copy.begin() + 100, copy.end());
        }},
    };

    BenchmarkConfig config;
    config.iterations = 100;

    auto results = BenchmarkRunner::compare(benchmarks, config);
    BenchmarkRunner::print_comparison(results);  // Prints formatted table
}
```

### Benchmark with Setup (Exclude Setup Time)

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    BenchmarkConfig config;
    config.iterations = 1000;

    auto result = BenchmarkRunner::run_with_setup(
        "sort_random_data",
        config,
        // Setup function - called before each iteration, time NOT measured
        []() {
            std::vector<int> data(10000);
            std::iota(data.begin(), data.end(), 0);
            std::random_shuffle(data.begin(), data.end());
            return data;
        },
        // Benchmark function - receives setup result, time IS measured
        [](std::vector<int>& data) {
            std::sort(data.begin(), data.end());
        }
    );

    result.print();
}
```

### Quick One-Liners

```cpp
#include "core/benchmark_kit.hpp"

// Quick benchmark with specified iterations
auto r = benchmark_kit::quick_bench("test", 10000, []() { /* code */ });

// Quick comparison
benchmark_kit::quick_compare({
    {"method_a", []() { method_a(); }},
    {"method_b", []() { method_b(); }},
}, 10000);
```

### Using Timer Directly

```cpp
#include "core/timer.hpp"

benchmark_kit::Timer timer;
timer.start();
// ... your code ...
std::cout << "Elapsed: " << timer.elapsed_milliseconds() << " ms\n";

// Or use scoped timer for automatic printing
{
    benchmark_kit::ScopedTimer t("my_operation");
    // ... code ...
}  // Prints "my_operation: X.XX ms" on scope exit
```

## Project Structure

```
cpp-benchmark-kit/
├── CMakeLists.txt              # CMake configuration
├── Dockerfile                  # Docker image
├── docker-compose.yml          # Container orchestration
├── src/
│   ├── core/                   # Benchmark framework core
│   │   ├── benchmark_kit.hpp   # Main include header
│   │   ├── benchmark_runner.hpp # BenchmarkRunner template class
│   │   ├── timer.hpp           # High-resolution timer
│   │   └── statistics.hpp      # Statistical analysis utilities
│   ├── server/                 # REST API server (Boost.Asio)
│   │   └── server.cpp
│   ├── storage/                # Database integration
│   ├── bindings/               # Python bindings (pybind11)
│   ├── examples/               # Example benchmarks
│   │   ├── weak_ptr/           # Reference counting implementation
│   │   ├── vector_erase/       # Vector erase algorithms
│   │   └── container_lookup/   # Container comparison
│   ├── benchmark/              # Legacy benchmark utilities
│   └── main.cpp                # Demo application
├── python/
│   ├── run.py                  # Interactive console
│   ├── view_results.py         # Results viewer
│   └── db_manager.py           # PostgreSQL manager
├── tests/                      # Unit tests (Boost.Test)
└── sql/
    └── init.sql                # Database schema
```

## Build Modes

| Mode | Use Case | Key Features |
|------|----------|--------------|
| **Debug** | Development, debugging | `-g -O0`, full symbols, assertions |
| **Release** | Production, benchmarks | Ultra Low Latency optimizations |

### Ultra Low Latency (ULL) Flags (Release only)

| Flag | Purpose |
|------|---------|
| `-O3 -march=native -mtune=native` | Maximum CPU-specific optimizations |
| `-flto` | Link-Time Optimization |
| `-ffast-math` | Aggressive floating-point optimizations |
| `-funroll-loops` | Loop unrolling |
| `-fomit-frame-pointer` | Free up register (rbp) |
| `-fno-exceptions -fno-rtti` | Disable C++ overhead |
| `-fno-stack-protector` | Disable stack canaries |
| `-fno-plt` | Direct function calls |
| `-fprefetch-loop-arrays` | Cache prefetch hints |
| `-falign-functions=32 -falign-loops=32` | Cache-line alignment |

## Build Commands

```bash
# Release build (recommended for benchmarks)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug build (for development)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)

# Run the demo
./build/CppBenchmarkKit

# Run tests
cd build && ctest --output-on-failure
```

## Docker

```bash
# Build and run (Release)
docker-compose --profile release build app
docker-compose --profile release run --rm --service-ports app

# Start REST API server
docker-compose --profile server up

# View results
docker-compose --profile viewer run --rm results_viewer
```

### REST API Endpoints

| Endpoint | Description |
|----------|-------------|
| `GET /health` | Server status |
| `GET /benchmark/task1` | weak_ptr::lock() benchmark |
| `GET /benchmark/task2?size=N` | Vector erase benchmark |
| `GET /benchmark/task3?size=N` | Container lookup benchmark |
| `GET /results` | Get results from DB |

## Requirements

- **OS:** Linux (tested on Ubuntu 22.04)
- **Compiler:** GCC 9+ or Clang 10+ with C++17 support
- **Dependencies:**
  - Boost (system)
  - PostgreSQL client (libpq) - optional
  - Python 3.8+ - optional, for bindings
  - pybind11 - fetched automatically if not found

> **Note:** Python bindings are built for the Python version found by CMake at build time.
> Use the same Python version to run scripts (e.g., `python3.12` if built with Python 3.12).

## API Reference

### BenchmarkConfig

```cpp
struct BenchmarkConfig {
    int iterations = 1000;           // Number of timed iterations
    int warmup_iterations = 100;     // Warmup iterations (not timed)
    int threads = 1;                 // Number of threads
    bool collect_samples = true;     // Collect individual samples
    bool verbose = false;            // Print progress
};
```

### BenchmarkResult

```cpp
struct BenchmarkResult {
    std::string name;                // Benchmark name
    BenchmarkConfig config;          // Configuration used
    BenchmarkStats stats;            // Statistical results
    long long total_time_ns;         // Total execution time
    double operations_per_second;    // Throughput
    bool success;                    // Success flag
    
    void print() const;              // Print formatted results
    double total_time_ms() const;    // Total time in milliseconds
};
```

### BenchmarkStats

```cpp
struct BenchmarkStats {
    double mean_ns;      // Mean time per iteration
    double stddev_ns;    // Standard deviation
    double min_ns;       // Minimum time
    double max_ns;       // Maximum time
    double p50_ns;       // Median (50th percentile)
    double p95_ns;       // 95th percentile
    double p99_ns;       // 99th percentile
    
    double mean_us() const;  // Mean in microseconds
    double mean_ms() const;  // Mean in milliseconds
};
```

## Examples

The `examples/` directory contains complete example benchmarks:

1. **weak_ptr/** — Custom reference counting implementation with `lock()` operation
2. **vector_erase/** — Comparison of 5 different vector erase algorithms
3. **container_lookup/** — std::map vs std::unordered_map vs std::vector lookup
4. **sorting/** — Sorting algorithms benchmark (std::sort vs QuickSort)

Run examples:
```bash
# Main demo
./build/CppBenchmarkKit

# Sorting benchmark (standalone example)
./build/sorting_benchmark
```

### Sorting Benchmark Output Example

```
==========================================================================================
Benchmark                           Mean (ns)        Ops/sec     vs Fastest        Status
==========================================================================================
std::sort                           401064.39       2.49e+03          1.00x        FASTEST
QuickSort (Lomuto)                  504661.06       1.98e+03          1.26x               
QuickSort + Insertion               426962.61       2.34e+03          1.06x               
std::stable_sort                    515459.98       1.94e+03          1.29x               
==========================================================================================
```

## Environment Variables

```bash
# PostgreSQL connection
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=cpp_benchmark_db
export DB_USER=benchmark
export DB_PASSWORD=benchmark_pass
```

## License

MIT License

## Author

**Ivan Lukianenko**

- 📧 Email: [vanaluk@gmail.com](mailto:vanaluk@gmail.com)
- 💼 LinkedIn: [linkedin.com/in/ivan-lukianenko-31502894](https://www.linkedin.com/in/ivan-lukianenko-31502894/)
