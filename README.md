# C++ Interview Demo Project

A demonstration project for preparing for a C++ Developer interview. The project demonstrates solutions to three test tasks with the ability to measure and compare performance of different implementations.

## Requirements

- **Tested on:** Ubuntu 22.04
- **Recommended:** Ubuntu with Docker installed

## Technology Stack

| Technology | Purpose |
|------------|---------|
| **C++17/20** | Main implementation language |
| **pybind11** | C++ and Python integration (calling C++ functions from Python) |
| **Boost.Asio** | Asynchronous HTTP server for REST API benchmarks |
| **PostgreSQL** | Storage of benchmark results |
| **Docker** | Full environment: build, run, test |
| **CMake** | Cross-platform build |

## ⚡ Build Modes

The project supports two build configurations optimized for different use cases:

| Mode | Use Case | Key Features |
|------|----------|--------------|
| **Debug** | Development, debugging | `-g -O0`, full symbols, assertions enabled |
| **Release** | Production, benchmarks | Ultra Low Latency optimizations (see below) |

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

> **Note:** pybind11 module and Boost.Test require exceptions/RTTI and compile with `-fexceptions -frtti` even in Release mode.


## Project Structure

```
cpp/
├── CMakeLists.txt          # CMake configuration
├── Dockerfile              # Docker image
├── docker-compose.yml      # Container orchestration
├── install.sh              # Installation script
├── .vscode/                # VS Code configuration
├── src/
│   ├── main.cpp
│   ├── task1_weak_ptr/     # Task 1: weak_ptr::lock()
│   ├── task2_vector_erase/ # Task 2: Vector erase
│   ├── task3_mapping/      # Task 3: Mapping int→string
│   ├── benchmark/          # Benchmark utilities
│   ├── asio_server/        # Boost.Asio HTTP server
│   └── bindings/           # pybind11 bindings
├── python/
│   ├── run.py              # Interactive console
│   ├── view_results.py     # Results viewing utility
│   └── db_manager.py       # PostgreSQL manager
├── sql/
│   └── init.sql            # DB schema
└── tests/
```

---

## 🚀 Quick Start

See [QUICK_START.md](QUICK_START.md) for detailed installation and running instructions.

### Build Commands Summary

```bash
# Docker Release (ULL optimizations)
docker-compose --profile release build app
docker-compose --profile release run --rm --service-ports app

# Docker Debug (with debug symbols)
docker-compose --profile debug build app-debug
docker-compose --profile debug run --rm --service-ports app-debug

# Local Release build (Ultra Low Latency)
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

# Local Debug build (for development)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build-debug -j$(nproc)
```

---

## 📊 Quick Compare: Release vs Debug

See [QUICK_COMPARE.md](QUICK_COMPARE.md) for detailed performance comparison between Release and Debug builds.

### Compare Commands Summary

```bash
# Build both images
docker-compose --profile release build app
docker-compose --profile debug build app-debug

# Run benchmarks on both builds (runs all 3 tasks automatically)
docker-compose --profile release run --rm app python3 python/run.py --autorun
docker-compose --profile debug run --rm app-debug python3 python/run.py --autorun

# Compare results (shows speedup factor)
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare
```

---

## 🐳 Using Docker

### Mode 1: Interactive Console

```bash
# Start PostgreSQL + interactive console
docker-compose --profile release run --rm --service-ports app
```

> **Note:** Use `--service-ports` to enable port mapping for menu option `[6] Start Boost.Asio server`.

This will start:
- PostgreSQL (with automatic DB initialization)
- Interactive console for running tasks

### Mode 2: REST API Server for Benchmarks

```bash
# Start PostgreSQL + Boost.Asio HTTP server (Release)
docker-compose --profile server up

# Start Debug server (port 8081)
docker-compose --profile debug-server up
```

**Server endpoints:**
| Build | URL | Port |
|-------|-----|------|
| **Release** | `http://localhost:8080` | 8080 |
| **Debug** | `http://localhost:8081` | 8081 |

Available endpoints:
- `GET /benchmark/task1` — task 1 benchmark
- `GET /benchmark/task2?size=100000` — task 2 benchmark
- `GET /benchmark/task3?size=100000` — task 3 benchmark
- `GET /results` — get results from DB

### Mode 3: View Results

```bash
# Start results viewing utility
docker-compose --profile viewer run --rm results_viewer
```

### Mode 4: Run Unit Tests

Unit tests are built with **Boost.Test** framework and integrated with **CTest**.

```bash
# Run all unit tests
docker-compose --profile release run --rm app bash -c "cd build && ctest --output-on-failure"

# Run specific test suite
docker-compose --profile release run --rm app bash -c "cd build && ctest -R Task1 --verbose"
docker-compose --profile release run --rm app bash -c "cd build && ctest -R Task2 --verbose"
docker-compose --profile release run --rm app bash -c "cd build && ctest -R Task3 --verbose"

# Run with detailed output
docker-compose --profile release run --rm app bash -c "cd build && ctest -V"

# List all available tests
docker-compose --profile release run --rm app bash -c "cd build && ctest -N"
```

**Test coverage:**
| Test Suite | Tests | Description |
|------------|-------|-------------|
| Task1Tests | 8 | CustomSharedPtr, CustomWeakPtr, lock(), multi-threading |
| Task2Tests | 25 | 5 erase methods × 5 scenarios each |
| Task3Tests | 8 | BenchmarkResult validation, container benchmarks |
| **Total** | **41** | |

### Separate Commands

```bash
# Build only (without running)
docker-compose build

# Run specific task
docker-compose --profile release run --rm app python3 python/run.py

# View results
docker-compose --profile release run --rm app python3 python/view_results.py --stats

# Export results to CSV
docker-compose --profile release run --rm app python3 python/view_results.py --export csv

# Run in monitoring mode
docker-compose --profile release run --rm app python3 python/view_results.py --watch
```

---

## 📊 Results Viewing Utility

All benchmark results are saved in PostgreSQL. Use `view_results.py` to view them:

```bash
# All results
python3 python/view_results.py

# Task 2 only
python3 python/view_results.py --task 2

# Statistics by methods
python3 python/view_results.py --stats

# Export to CSV
python3 python/view_results.py --export csv --output my_results.csv

# Export to JSON
python3 python/view_results.py --export json

# Monitoring mode (update every 5 sec)
python3 python/view_results.py --watch

# Help
python3 python/view_results.py --help
```

Example output:
```
========================================================================================================================
Date/Time            Task                      Method                         Time            Ops/sec      Threads
========================================================================================================================
2024-12-23 14:30:15  Vector erase              remove_if + erase              1.23 ms         812.45K      1
2024-12-23 14:30:14  Vector erase              Naive (erase in loop)          45.67 ms        21.89K       1
2024-12-23 14:30:12  weak_ptr::lock()          CustomWeakPtr::lock()          234.56 ns       4.26M        4
------------------------------------------------------------------------------------------------------------------------
Total records: 3
```

---

## 📝 Tasks

### Task 1: weak_ptr::lock()

Reference counter implementation:
- `CustomSharedPtr<T>` — std::shared_ptr analog
- `CustomWeakPtr<T>` — std::weak_ptr analog
- `lock()` method uses `compare_exchange_weak` for thread safety

### Task 2: Remove Every Second Element

5 methods with different complexity:

| Method | Complexity | Features |
|--------|------------|----------|
| Naive (erase in loop) | O(n²) | Simple but slow |
| remove_if + erase | O(n) | Erase-remove idiom |
| Iterators | O(n²) | More "idiomatic" |
| Copy to new vector | O(n) | Additional memory |
| std::partition | O(n) | Most efficient |

### Task 3: Mapping int → string

Container comparison:
- `std::map` — O(log n), ordered
- `std::unordered_map` — O(1) average, best choice for performance
- `std::vector<pair>` — O(n), only for small sets (<100 elements)

---

## 🧪 Running Unit Tests (Local Build)

After building the project locally:

```bash
cd build

# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R Task1 --verbose    # Task 1: CustomSharedPtr, CustomWeakPtr
ctest -R Task2 --verbose    # Task 2: Vector erase methods  
ctest -R Task3 --verbose    # Task 3: Container benchmarks

# Run with full output
ctest -V

# List all tests without running
ctest -N
```

---

## 🔧 Development in VS Code

### IDE Setup (clangd support)

The project uses `compile_commands.json` for IDE features (code completion, go-to-definition, linting). After building:

```bash
# Option 1: Create symlink in project root (recommended)
ln -sf build/compile_commands.json compile_commands.json

# Option 2: Configure .clangd to point to build directory (already done)
# See .clangd file: CompilationDatabase: build
```

### VS Code

1. Open project in VS Code
2. Install extensions: clangd
3. Build the project first: `cmake -B build && cmake --build build`
4. Restart clangd: `Ctrl+Shift+P` → "clangd: Restart language server"
5. `F5` for debugging

> **Note:** If clangd shows "file not found" errors, ensure `compile_commands.json` exists in `build/` directory and restart the language server.

---

## 📈 Profiling with perf

```bash
# Record profile (Release build)
perf record ./build/CppInterviewDemo

# Record profile (Debug build)
perf record ./build-debug/CppInterviewDemo

# Analyze
perf report

# Or via Docker
docker-compose --profile release run --rm app perf record ./build/CppInterviewDemo
```

---

## Environment Variables

**Option A: Peer authentication (no password, Unix socket)**
```bash
export DB_HOST=        # Empty = Unix socket (peer auth)
export DB_PORT=5432
export DB_NAME=cpp_interview_db
export DB_USER=$USER
export DB_PASSWORD=
```

**Option B: Password authentication (TCP/IP)**
```bash
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=cpp_interview_db
export DB_USER=cpp_interview
export DB_PASSWORD=cpp_interview_pass
```

> **Note:** Docker uses Option B internally. For local development, Option A is easier.

---

## License

Project created for demonstration purposes.
