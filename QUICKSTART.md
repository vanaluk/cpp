# Quick Start Guide

## 1. Via Docker (recommended)

### 1.1 Build

```bash
cd cpp

# Run script (automatically detects Docker)
./install.sh
```

### 1.2 Start Interactive Console

```bash
# Start PostgreSQL + interactive console (recommended)
docker-compose run --rm app

# Alternative: Start in background, then connect
docker-compose up -d
docker-compose exec app python3 python/run.py
```

### 1.3 Run Specific Commands

```bash
# Run all benchmarks automatically
docker-compose run --rm app python3 python/run.py --autorun

# View results
docker-compose run --rm app python3 python/view_results.py

# View statistics
docker-compose run --rm app python3 python/view_results.py --stats

# Export results
docker-compose run --rm app python3 python/view_results.py --export csv

# Run C++ binary directly
docker-compose run --rm app ./build/CppInterviewDemo
```

### 1.4 Run Unit Tests

```bash
# Run all unit tests inside Docker
docker-compose run --rm app bash -c "cd build && ctest --output-on-failure"

# Run specific test suite
docker-compose run --rm app bash -c "cd build && ctest -R Task1 --verbose"
docker-compose run --rm app bash -c "cd build && ctest -R Task2 --verbose"
docker-compose run --rm app bash -c "cd build && ctest -R Task3 --verbose"

# Run tests with detailed output
docker-compose run --rm app bash -c "cd build && ctest -V"
```

### 1.5 Stop

```bash
# Stop all containers
docker-compose down

# Stop and delete DB data
docker-compose down -v
```

---

## 2. Without Docker (local build)

### 2.1 Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake g++ \
    python3 python3-pip python3-dev \
    libboost-all-dev libpq-dev \
    postgresql postgresql-client
```

### 2.2 Build Project

```bash
cd cpp

# Option 1: Via script (force local build, no Docker)
./install.sh --local

# Option 2: Manually
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
cd ..
```

> **Note:** Use `--local` flag to guarantee local build even if Docker is installed.
> Use `./install.sh --help` to see all available options.

### 2.3 Configure PostgreSQL

**Option A: Quick setup (use your system user, no password needed)**

```bash
# Create PostgreSQL user matching your system username (one-time setup)
sudo -u postgres createuser -s $USER
createdb cpp_interview_db
psql -d cpp_interview_db -f sql/init.sql

export DB_HOST=
export DB_PORT=5432
export DB_NAME=cpp_interview_db
export DB_USER=$USER
export DB_PASSWORD=
```

> **Note:** `DB_HOST=` (empty) uses Unix socket with peer authentication.
> If you set `DB_HOST=localhost`, PostgreSQL requires password authentication.

**Option B: Dedicated user with password**

```bash
# Create DB and user
sudo -u postgres createdb cpp_interview_db
sudo -u postgres psql -c "CREATE USER cpp_interview WITH PASSWORD 'cpp_interview_pass';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE cpp_interview_db TO cpp_interview;"

# Initialize schema
psql -h localhost -U cpp_interview -d cpp_interview_db -f sql/init.sql

# Set environment variables
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=cpp_interview_db
export DB_USER=cpp_interview
export DB_PASSWORD=cpp_interview_pass
```

### 2.4 Run Tasks

```bash
# Interactive console
python3 python/run.py

# In console select:
# [1] - Task 1 (weak_ptr::lock)
# [2] - Task 2 (vector erase)
# [3] - Task 3 (mapping int→string)
# [4] - Run all
# [5] - View results
# [6] - Start HTTP server
```

### 2.5 Run Unit Tests

```bash
cd build

# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R Task1 --verbose    # Task 1: CustomSharedPtr, CustomWeakPtr
ctest -R Task2 --verbose    # Task 2: Vector erase methods
ctest -R Task3 --verbose    # Task 3: Container benchmarks

# Run all tests with detailed output
ctest -V

# Run tests and show test names
ctest -N    # List all tests without running
```

**Test coverage:**
- **Task 1:** 8 tests (CustomSharedPtr, CustomWeakPtr, lock(), multi-threading)
- **Task 2:** 25 tests (5 erase methods × 5 scenarios each)
- **Task 3:** 8 tests (BenchmarkResult validation, positive values, scaling)

### 2.6 View Results

```bash
# All results
python3 python/view_results.py

# Task 1 only
python3 python/view_results.py --task 1

# Task 2 only
python3 python/view_results.py --task 2

# Task 3 only
python3 python/view_results.py --task 3

# Statistics by methods
python3 python/view_results.py --stats

# Statistics for task 2 only
python3 python/view_results.py --stats --task 2

# Export to CSV
python3 python/view_results.py --export csv

# Export to JSON
python3 python/view_results.py --export json --output results.json

# Monitoring mode (update every 5 sec)
python3 python/view_results.py --watch

# Limit number of records
python3 python/view_results.py --limit 10
```

---

## 3. Via HTTP Server (REST API)

### 3.1 Start Server

```bash
# Via Docker (recommended)
docker-compose --profile server up

# Or locally
./build/asio_server 8080
```

### 3.2 Available Endpoints

| Method | URL | Description |
|--------|-----|-------------|
| GET | `/benchmark/task1` | Task 1 benchmark (weak_ptr::lock) |
| GET | `/benchmark/task2` | Task 2 benchmark (vector erase) |
| GET | `/benchmark/task3` | Task 3 benchmark (mapping) |
| GET | `/results` | Get all results from DB |
| GET | `/health` | Server health check |

### 3.3 Request Examples

```bash
# Task 1 benchmark
curl http://localhost:8080/benchmark/task1

# Task 2 benchmark with parameters
curl "http://localhost:8080/benchmark/task2?size=100000&iterations=100"

# Task 3 benchmark with parameters
curl "http://localhost:8080/benchmark/task3?size=50000"

# Get results
curl http://localhost:8080/results

# Server health check
curl http://localhost:8080/health

# Pretty JSON output (via jq)
curl -s http://localhost:8080/results | jq

# Save results to file
curl -s http://localhost:8080/results > results.json
```

### 3.4 Response Examples

**Task 1 Benchmark:**
```json
{
  "task": 1,
  "task_name": "weak_ptr::lock()",
  "method": "CustomWeakPtr::lock()",
  "execution_time_ns": 1234567,
  "operations_per_second": 810045.2,
  "status": "success"
}
```

**Task 2 Benchmark:**
```json
{
  "task": 2,
  "task_name": "Vector erase",
  "methods": [
    {"name": "naive_erase", "time_ns": 45670000},
    {"name": "remove_if_erase", "time_ns": 1230000},
    {"name": "iterators", "time_ns": 43210000},
    {"name": "copy_to_new", "time_ns": 980000},
    {"name": "partition", "time_ns": 890000}
  ],
  "vector_size": 100000,
  "iterations": 100,
  "status": "success"
}
```

**Results from DB:**
```json
{
  "results": [
    {
      "id": 1,
      "timestamp": "2024-12-23T14:30:15",
      "task_number": 2,
      "task_name": "Vector erase",
      "method_name": "remove_if_erase",
      "execution_time_ns": 1230000,
      "operations_per_second": 813008.1,
      "thread_count": 1
    }
  ],
  "total": 1
}
```

### 3.5 Automation via Scripts

```bash
#!/bin/bash
# run_all_benchmarks.sh

SERVER="http://localhost:8080"

echo "Running all benchmarks..."

# Task 1
echo "Task 1: weak_ptr::lock()"
curl -s "$SERVER/benchmark/task1" | jq

# Task 2 with different sizes
for size in 10000 50000 100000; do
    echo "Task 2: vector size=$size"
    curl -s "$SERVER/benchmark/task2?size=$size" | jq
done

# Task 3
echo "Task 3: mapping benchmark"
curl -s "$SERVER/benchmark/task3?size=100000" | jq

# Get all results
echo "All results:"
curl -s "$SERVER/results" | jq
```

---

## 4. Run Unit Tests

Unit tests are built with **Boost.Test** framework and integrated with **CTest**.

### 4.1 Via Docker

```bash
# Run all tests
docker-compose run --rm app bash -c "cd build && ctest --output-on-failure"

# Run specific test
docker-compose run --rm app bash -c "cd build && ctest -R Task1 --verbose"
```

### 4.2 Local

```bash
cd build

# Run all tests
ctest --output-on-failure

# Run with verbose output
ctest -V

# Run specific test suite
ctest -R Task2 --verbose
```

### 4.3 Test Summary

| Test Suite | Tests | Description |
|------------|-------|-------------|
| Task1Tests | 8 | CustomSharedPtr, CustomWeakPtr, lock(), threading |
| Task2Tests | 25 | 5 erase methods × 5 scenarios |
| Task3Tests | 8 | BenchmarkResult validation, container benchmarks |
| **Total** | **41** | |

---

## 5. Command Summary Table

### Run Tasks

| Method | Command |
|--------|---------|
| Docker (interactive) | `docker-compose run --rm app` |
| Docker (auto) | `docker-compose run --rm app python3 python/run.py --autorun` |
| Local (console) | `python3 python/run.py` |
| HTTP API | `curl http://localhost:8080/benchmark/task1` |

### View Results

| Method | Command |
|--------|---------|
| Docker | `docker-compose run --rm app python3 python/view_results.py` |
| Local | `python3 python/view_results.py` |
| HTTP API | `curl http://localhost:8080/results` |
| Statistics | `python3 python/view_results.py --stats` |
| Export CSV | `python3 python/view_results.py --export csv` |
| Monitoring | `python3 python/view_results.py --watch` |

### Run Unit Tests

| Method | Command |
|--------|---------|
| Docker (all) | `docker-compose run --rm app bash -c "cd build && ctest --output-on-failure"` |
| Docker (Task1) | `docker-compose run --rm app bash -c "cd build && ctest -R Task1 --verbose"` |
| Local (all) | `cd build && ctest --output-on-failure` |
| Local (verbose) | `cd build && ctest -V` |
| List tests | `cd build && ctest -N` |

### Docker Management

| Action | Command |
|--------|---------|
| Build | `docker-compose build` |
| Rebuild (no cache) | `docker-compose build --no-cache` |
| Start interactive console | `docker-compose run --rm app` |
| Start server | `docker-compose --profile server up` |
| Stop | `docker-compose down` |
| Clear data | `docker-compose down -v` |
| Logs | `docker-compose logs -f` |
| List project images | `docker images \| grep cpp` |
| Remove project images | `docker-compose down --rmi all` |
| Full cleanup (images + data) | `docker-compose down -v --rmi all` |
