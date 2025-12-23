# Quick Start Guide

## 1. Without Docker (local build)

### 1.1 Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake g++ \
    python3 python3-pip python3-dev \
    libboost-all-dev libpq-dev \
    postgresql postgresql-client

# Fedora/RHEL
sudo dnf install -y \
    gcc-c++ cmake \
    python3 python3-pip python3-devel \
    boost-devel libpq-devel postgresql
```

### 1.2 Build Project

```bash
cd cpp

# Option 1: Via script
./install.sh

# Option 2: Manually
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
cd ..
```

### 1.3 Configure PostgreSQL

```bash
# Create DB
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

### 1.4 Run Tests

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

### 1.5 View Results

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

## 2. Via Docker

### 2.1 Build

```bash
cd cpp

# Build image
docker-compose build

# Or via script (automatically detects Docker)
./install.sh
```

### 2.2 Start Interactive Console

```bash
# Start PostgreSQL + console
docker-compose up

# In background mode
docker-compose up -d

# Connect to console (if running in background)
docker-compose exec app python3 python/run.py
```

### 2.3 Run Specific Commands

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

### 2.4 Stop

```bash
# Stop all containers
docker-compose down

# Stop and delete DB data
docker-compose down -v
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

## 4. Command Summary Table

### Run Tests

| Method | Command |
|--------|---------|
| Local (console) | `python3 python/run.py` |
| Docker (console) | `docker-compose up` |
| Docker (auto) | `docker-compose run --rm app python3 python/run.py --autorun` |
| HTTP API | `curl http://localhost:8080/benchmark/task1` |

### View Results

| Method | Command |
|--------|---------|
| Local | `python3 python/view_results.py` |
| Docker | `docker-compose run --rm app python3 python/view_results.py` |
| HTTP API | `curl http://localhost:8080/results` |
| Statistics | `python3 python/view_results.py --stats` |
| Export CSV | `python3 python/view_results.py --export csv` |
| Monitoring | `python3 python/view_results.py --watch` |

### Docker Management

| Action | Command |
|--------|---------|
| Build | `docker-compose build` |
| Start console | `docker-compose up` |
| Start server | `docker-compose --profile server up` |
| Stop | `docker-compose down` |
| Clear data | `docker-compose down -v` |
| Logs | `docker-compose logs -f` |
