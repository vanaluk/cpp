# Quick Compare: Release vs Debug Performance

This guide provides commands for comparing performance between Release (ULL-optimized) and Debug builds.

## Overview

| Build Type | Purpose | Key Flags |
|------------|---------|-----------|
| **Release** | Production, benchmarks | `-O3 -march=native -flto -ffast-math` + ULL flags |
| **Debug** | Development, debugging | `-g -O0`, full symbols, assertions |

---

## 1. Build Images

### Build Release Image (with Ultra Low Latency optimizations)

```bash
# Build Release image (default, ULL-optimized)
docker-compose --profile release build app
docker-compose --profile server build benchmark_server
```

### Build Debug Image (with debug symbols)

```bash
# Build Debug image (no optimizations, debug symbols)
docker-compose --profile debug build app-debug
docker-compose --profile debug-server build benchmark_server_debug
```

### Build Both Images at Once

```bash
# Build all images for comparison
docker-compose --profile release build app && \
docker-compose --profile debug build app-debug && \
docker-compose --profile server build benchmark_server && \
docker-compose --profile debug-server build benchmark_server_debug
```

---

## 2. Run Benchmarks

### Option A: Using Interactive Console

```bash
# Run Release benchmarks (choose tasks 1-3 in menu)
docker-compose --profile release run --rm --service-ports app

# Run Debug benchmarks (choose tasks 1-3 in menu)
docker-compose --profile debug run --rm --service-ports app-debug
```

> **Note:** Menu option `[6] Start REST API server` works from interactive console.
> Use `--service-ports` flag to enable port mapping:
> - Release (`app`) → server on port 8080
> - Debug (`app-debug`) → server on port 8081

### Option B: Using REST API Server

**Server URLs by build type:**

| Build | URL | Port |
|-------|-----|------|
| **Release** | `http://localhost:8080` | 8080 |
| **Debug** | `http://localhost:8081` | 8081 |

#### Start Release Server (port 8080)

```bash
# Start Release benchmark server in background
docker-compose --profile server up -d benchmark_server

# Run benchmarks via API
curl "http://localhost:8080/benchmark/task1?iterations=1000"
curl "http://localhost:8080/benchmark/task2?size=10000&iterations=100"
curl "http://localhost:8080/benchmark/task3?size=10000&lookups=100000"

# Stop server when done
docker-compose --profile server down benchmark_server
```

#### Start Debug Server (port 8081)

```bash
# Start Debug benchmark server in background
docker-compose --profile debug-server up -d benchmark_server_debug

# Run benchmarks via API (note: port 8081)
curl "http://localhost:8081/benchmark/task1?iterations=1000"
curl "http://localhost:8081/benchmark/task2?size=10000&iterations=100"
curl "http://localhost:8081/benchmark/task3?size=10000&lookups=100000"

# Stop server when done
docker-compose --profile debug-server down benchmark_server_debug
```

#### Run Both Servers Simultaneously

```bash
# Start both servers (Release on 8080, Debug on 8081)
docker-compose --profile server --profile debug-server up -d

# Run same benchmark on both builds
curl "http://localhost:8080/benchmark/task3?size=10000&lookups=100000"
curl "http://localhost:8081/benchmark/task3?size=10000&lookups=100000"

# Stop both servers
docker-compose --profile server --profile debug-server down
```

---

## 3. Compare Results

### View All Results with Build Type

```bash
# Show all results with build type column
docker-compose --profile viewer run --rm results_viewer

# Show only Release results
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --build Release

# Show only Debug results
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --build Debug
```

### Compare Release vs Debug Performance

```bash
# Show side-by-side comparison with speedup factor
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare
```

**Output example:**

```
================================================================================
                    RELEASE vs DEBUG COMPARISON
================================================================================
Task | Method              | Debug (ns)    | Release (ns)  | Speedup
--------------------------------------------------------------------------------
   1 | weak_ptr::lock()    |     1,250,000 |       125,000 | 10.0x
   2 | remove_if           |    85,000,000 |     8,500,000 | 10.0x
   3 | std::unordered_map  |   450,000,000 |    45,000,000 | 10.0x
--------------------------------------------------------------------------------
```

**Columns explained:**
- **Debug (ns)**: Execution time in nanoseconds for Debug build
- **Release (ns)**: Execution time in nanoseconds for Release build  
- **Speedup**: How many times faster Release is compared to Debug (`Debug / Release`)

### Filter by Task

```bash
# Compare specific task only
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare --task 1
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare --task 2
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare --task 3
```

### Export Results

```bash
# Export to CSV for external analysis
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --export csv > results.csv

# Export to JSON
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --export json > results.json
```

---

## Full Comparison Workflow

Complete workflow to compare Release vs Debug performance:

```bash
# Step 1: Start database
docker-compose up -d postgres

# Step 2: Build both images
docker-compose --profile release build app
docker-compose --profile debug build app-debug

# Step 3: Run Release benchmarks (runs all 3 tasks automatically)
docker-compose --profile release run --rm app python3 python/run.py --autorun

# Step 4: Run Debug benchmarks (runs all 3 tasks automatically)
docker-compose --profile debug run --rm app-debug python3 python/run.py --autorun

# Step 5: Compare results
docker-compose --profile viewer run --rm results_viewer python3 python/view_results.py --compare

# Step 6: Cleanup
docker-compose down
```

---

## Understanding Release Ops/s

In the comparison output, you may see **Ops/s** (Operations per Second) metrics:

| Metric | Formula | Meaning |
|--------|---------|---------|
| **Release Ops/s** | `1,000,000,000 / Release_time_ns` | Operations per second in Release build |
| **Debug Ops/s** | `1,000,000,000 / Debug_time_ns` | Operations per second in Debug build |
| **Speedup** | `Debug_time / Release_time` | How many times faster Release is |

**Example:**
- Debug time: 1,000,000 ns → Debug Ops/s: 1,000
- Release time: 100,000 ns → Release Ops/s: 10,000
- Speedup: 10.0x (Release is 10 times faster)

---

## Tips

1. **Run benchmarks multiple times** for more accurate averages
2. **Use same parameters** when comparing builds
3. **Warm up the CPU** with a few test runs before final measurements
4. **Close other applications** to reduce noise in measurements
5. **Check database** has results from both builds before comparing

---

## IDE Setup (clangd)

For local development with IDE support, generate `compile_commands.json`:

```bash
# Build with compile commands export
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# Create symlink for IDE
ln -sf build/compile_commands.json compile_commands.json

# Restart clangd in IDE: Ctrl+Shift+P → "clangd: Restart language server"
```

