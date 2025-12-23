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

See [QUICKSTART.md](QUICKSTART.md) for detailed installation and running instructions.

---

## 🐳 Using Docker

### Mode 1: Interactive Console

```bash
# Start PostgreSQL + interactive console
docker-compose run --rm app
```

This will start:
- PostgreSQL (with automatic DB initialization)
- Interactive console for running tasks

### Mode 2: REST API Server for Benchmarks

```bash
# Start PostgreSQL + Boost.Asio HTTP server
docker-compose --profile server up
```

Server will be available at `http://localhost:8080`:
- `GET /benchmark/task1` — task 1 benchmark
- `GET /benchmark/task2?size=100000` — task 2 benchmark
- `GET /benchmark/task3?size=100000` — task 3 benchmark
- `GET /results` — get results from DB

### Mode 3: View Results

```bash
# Start results viewing utility
docker-compose --profile viewer run --rm results_viewer
```

### Mode 4: Run Tests Inside Docker (not implemented)

```bash
# Run tests
docker-compose run --rm app ./build/tests/run_tests

# Or via bash
docker-compose run --rm app bash -c "cd build && ctest --output-on-failure"
```

### Separate Commands

```bash
# Build only (without running)
docker-compose build

# Run specific task
docker-compose run --rm app python3 python/run.py

# View results
docker-compose run --rm app python3 python/view_results.py --stats

# Export results to CSV
docker-compose run --rm app python3 python/view_results.py --export csv

# Run in monitoring mode
docker-compose run --rm app python3 python/view_results.py --watch
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

## 🔧 Development in VS Code

1. Open project in VS Code
2. Install extensions: C/C++, CMake Tools
3. `Ctrl+Shift+P` → "CMake: Configure"
4. `F5` for debugging

---

## 📈 Profiling with perf

```bash
# Record profile
perf record ./build/CppInterviewDemo

# Analyze
perf report

# Or via Docker
docker-compose run --rm app perf record ./build/CppInterviewDemo
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
