# C++ Benchmark Kit - Quick Start Guide

## Adding Your Own Benchmark

### Method 1: Simple One-Liner

```cpp
#include "core/benchmark_kit.hpp"

int main() {
    auto result = benchmark_kit::BenchmarkRunner::run("my_function", []() {
        // Your code to benchmark
        my_function();
    });
    
    result.print();  // Prints detailed statistics
    return 0;
}
```

### Method 2: Custom Configuration

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    BenchmarkConfig config;
    config.iterations = 10000;        // How many times to run
    config.warmup_iterations = 1000;  // Warmup runs (not measured)
    config.threads = 4;               // Run in parallel
    config.verbose = true;            // Show progress

    auto result = BenchmarkRunner::run("parallel_work", config, []() {
        do_work();
    });

    // Access individual stats
    std::cout << "Mean: " << result.stats.mean_ns << " ns\n";
    std::cout << "P99:  " << result.stats.p99_ns << " ns\n";
    std::cout << "Ops/sec: " << result.operations_per_second << "\n";
    
    return 0;
}
```

### Method 3: Compare Multiple Implementations

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    std::vector<int> data(100000);
    std::iota(data.begin(), data.end(), 0);

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
    BenchmarkRunner::print_comparison(results);  // Formatted table
    
    return 0;
}
```

### Method 4: Benchmark with Setup (Exclude Setup Time)

When you need to prepare data before each iteration but don't want setup time included:

```cpp
#include "core/benchmark_kit.hpp"

using namespace benchmark_kit;

int main() {
    BenchmarkConfig config;
    config.iterations = 1000;

    auto result = BenchmarkRunner::run_with_setup(
        "sort_random_data",
        config,
        // Setup: called before each iteration (NOT timed)
        []() {
            std::vector<int> data(10000);
            std::iota(data.begin(), data.end(), 0);
            std::random_shuffle(data.begin(), data.end());
            return data;
        },
        // Benchmark: receives setup result (IS timed)
        [](std::vector<int>& data) {
            std::sort(data.begin(), data.end());
        }
    );

    result.print();
    return 0;
}
```

## Build Commands

```bash
# Release build (recommended for accurate benchmarks)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug build (for development)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)

# Run
./build/CppBenchmarkKit

# Run Python console (use the Python version matching build)
# Check which version: ls build/*.so (e.g., cpython-312 means python3.12)
python3.12 python/run.py
```

> **Note:** Python bindings are built for the Python version found by CMake.
> If you see "ModuleNotFoundError", check the `.so` filename and use the matching Python version.

## Docker

```bash
# Build and run interactive console
docker-compose --profile release run --rm --service-ports app

# Run all benchmarks automatically
docker-compose --profile release run --rm app python3 python/run.py --autorun

# Start REST API server
docker-compose --profile server up
```

## REST API

Start the server:
```bash
./build/benchmark_server 8080
```

Make requests:
```bash
# Health check
curl http://localhost:8080/health

# Run benchmark
curl "http://localhost:8080/benchmark/task1?iterations=1000000"
curl "http://localhost:8080/benchmark/task2?size=100000"
curl "http://localhost:8080/benchmark/task3?size=50000"

# Get results
curl http://localhost:8080/results
```

## Project Structure

```
cpp-benchmark-kit/
├── src/
│   ├── core/                   # Framework core (include these)
│   │   ├── benchmark_kit.hpp   # Main header (includes all)
│   │   ├── benchmark_runner.hpp # BenchmarkRunner class
│   │   ├── timer.hpp           # Timer utilities
│   │   └── statistics.hpp      # Statistical analysis
│   ├── examples/               # Example benchmarks
│   │   ├── weak_ptr/           # Reference counting
│   │   ├── vector_erase/       # Vector algorithms
│   │   └── container_lookup/   # Container comparison
│   ├── server/                 # REST API server
│   └── main.cpp                # Demo application
├── python/
│   ├── run.py                  # Interactive console
│   └── view_results.py         # Results viewer
└── tests/                      # Unit tests
```

## API Reference

### BenchmarkConfig

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `iterations` | int | 1000 | Number of timed runs |
| `warmup_iterations` | int | 100 | Warmup runs (not timed) |
| `threads` | int | 1 | Number of parallel threads |
| `collect_samples` | bool | true | Collect individual samples |
| `verbose` | bool | false | Print progress |

### BenchmarkStats

| Field | Type | Description |
|-------|------|-------------|
| `mean_ns` | double | Mean time per iteration (nanoseconds) |
| `stddev_ns` | double | Standard deviation |
| `min_ns` | double | Minimum time |
| `max_ns` | double | Maximum time |
| `p50_ns` | double | Median (50th percentile) |
| `p95_ns` | double | 95th percentile |
| `p99_ns` | double | 99th percentile |

### BenchmarkResult

| Method | Returns | Description |
|--------|---------|-------------|
| `print()` | void | Print formatted results |
| `total_time_ms()` | double | Total time in milliseconds |
| `total_time_s()` | double | Total time in seconds |
