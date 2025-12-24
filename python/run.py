#!/usr/bin/env python3
"""
Interactive console for running tasks and benchmarks
"""
import sys
import os
import time
import signal
import select
import subprocess
from typing import Optional

# Global flag for graceful shutdown
_shutdown_requested = False


def signal_handler(signum, frame):
    """Handle shutdown signals"""
    global _shutdown_requested
    _shutdown_requested = True
    print("\n\nShutdown signal received. Exiting...")
    sys.exit(0)


# Register signal handlers for graceful shutdown
signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)


def interruptible_input(prompt: str) -> str:
    """Read input with ability to be interrupted by signals"""
    global _shutdown_requested
    sys.stdout.write(prompt)
    sys.stdout.flush()

    while not _shutdown_requested:
        # Check if stdin has data (timeout 0.5 sec)
        ready, _, _ = select.select([sys.stdin], [], [], 0.5)
        if ready:
            line = sys.stdin.readline()
            if not line:  # EOF
                raise KeyboardInterrupt()
            return line.rstrip("\n")

    raise KeyboardInterrupt()


# Add path to modules
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    from db_manager import DatabaseManager
except ImportError:
    print("Error: failed to import db_manager")
    print("Make sure dependencies are installed: pip3 install -r requirements.txt")
    sys.exit(1)

# Global flags for C++ module availability
CPP_MODULE_AVAILABLE = False
cpp = None

# Try to import C++ bindings module
try:
    # Get script directory and add build/ to sys.path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(os.path.dirname(script_dir), 'build')
    
    if build_dir not in sys.path:
        sys.path.insert(0, build_dir)
    
    import cpp_interview_bindings as cpp
    CPP_MODULE_AVAILABLE = True
except ImportError as e:
    # Graceful degradation: continue in stub mode
    print("\n" + "=" * 70)
    print("WARNING: C++ module 'cpp_interview_bindings' not found")
    print("=" * 70)
    print("The application will continue in stub mode.")
    print("\nTo build the C++ module, run:")
    print("  mkdir -p build && cd build && cmake .. && make")
    print("\nError details:", str(e))
    print("=" * 70 + "\n")
    CPP_MODULE_AVAILABLE = False


class InterviewDemoConsole:
    def __init__(self):
        self.db = DatabaseManager()
        self.running = True

    def check_cpp_module(self) -> bool:
        """
        Check if C++ module is available.
        
        Returns:
            True if module is loaded, False otherwise.
            If False, prints warning and build instructions.
        """
        if not CPP_MODULE_AVAILABLE:
            print("\n" + "=" * 70)
            print("WARNING: C++ module is not available")
            print("=" * 70)
            print("To build the C++ module, run:")
            print("  mkdir -p build && cd build && cmake .. && make")
            print("=" * 70 + "\n")
        return CPP_MODULE_AVAILABLE

    def print_menu(self):
        """Print main menu"""
        print("\n" + "=" * 50)
        print("C++ Interview Demo Project - Main Menu")
        print("=" * 50)
        print("[1] Task 1: weak_ptr::lock() implementation")
        print("[2] Task 2: Remove every second element from vector")
        print("[3] Task 3: Mapping integer to string")
        print("[4] Run all tasks")
        print("[5] View results from DB")
        print("[6] Start Boost.Asio server")
        print("[0] Exit")
        print("=" * 50)

    def get_user_input(self, prompt: str, default: Optional[str] = None) -> str:
        """Get input from user"""
        global _shutdown_requested
        if _shutdown_requested:
            raise KeyboardInterrupt()
        try:
            if default:
                user_input = interruptible_input(f"{prompt} [{default}]: ").strip()
                return user_input if user_input else default
            return interruptible_input(f"{prompt}: ").strip()
        except EOFError:
            # Handle EOF (e.g., when stdin is closed)
            raise KeyboardInterrupt()

    def get_int_input(self, prompt: str, default: int) -> int:
        """Get integer input from user"""
        while True:
            try:
                value = self.get_user_input(prompt, str(default))
                return int(value)
            except ValueError:
                print("Error: enter an integer")

    def task1_menu(self):
        """Task 1 menu"""
        print("\n=== Task 1: weak_ptr::lock() ===")
        print("Demonstration of reference counter implementation")
        print("\n[1] Show implementation source code")
        print("[2] Run demonstration")
        print("[3] Run benchmark")
        print("[0] Back to main menu")
        
        choice = self.get_user_input("Select option", "0")
        
        if choice == "1":
            self.show_task1_code()
        elif choice == "2":
            self.run_task1_demo()
        elif choice == "3":
            self.run_task1_benchmark()
        elif choice == "0":
            return
        else:
            print("Invalid choice")

    def show_task1_code(self):
        """Show task 1 code"""
        code_path = "src/task1_weak_ptr/custom_weak_ptr.hpp"
        if os.path.exists(code_path):
            print(f"\n=== Source code: {code_path} ===\n")
            with open(code_path, 'r', encoding='utf-8') as f:
                print(f.read())
        else:
            print(f"File {code_path} not found")

    def run_task1_demo(self):
        """Run task 1 demonstration"""
        if not self.check_cpp_module():
            return
        
        print("\nRunning demonstration...")
        try:
            cpp.demonstrate_weak_ptr_lock()
        except Exception as e:
            print(f"Error running demonstration: {e}")

    def run_task1_benchmark(self):
        """Run task 1 benchmark"""
        if not self.check_cpp_module():
            return
        
        iterations = self.get_int_input("Number of iterations", 1000000)
        thread_count = self.get_int_input("Number of threads", 1)
        
        print(f"\nRunning benchmark: iterations={iterations}, threads={thread_count}")
        
        try:
            execution_time_ns = cpp.benchmark_weak_ptr_lock(iterations, thread_count)
            
            # Display results
            execution_time_ms = execution_time_ns / 1e6
            operations_per_second = iterations / (execution_time_ns / 1e9)

            print("\nResults:")
            print(f"  Execution time: {execution_time_ms:.2f} ms")
            print(f"  Operations per second: {operations_per_second:,.0f}")
            
            if self.db.is_connected():
                self.db.save_benchmark_result(
                    task_number=1,
                    task_name="weak_ptr::lock()",
                    method_name="CustomWeakPtr::lock()",
                    execution_time_ns=execution_time_ns,
                    parameters={"iterations": iterations},
                    thread_count=thread_count,
                    operations_per_second=operations_per_second
                )
                print("  ✓ Results saved to DB")
        except Exception as e:
            print(f"Error running benchmark: {e}")

    def task2_menu(self):
        """Task 2 menu"""
        print("\n=== Task 2: Remove every second element ===")
        print("[1] Erase in loop method (naive)")
        print("[2] remove_if + erase method")
        print("[3] Iterators method")
        print("[4] Copy to new vector method")
        print("[5] Run comparative benchmark of all methods")
        print("[0] Back to main menu")
        
        choice = self.get_user_input("Select option", "0")
        
        if choice == "5":
            self.run_task2_benchmark()
        elif choice in ["1", "2", "3", "4"]:
            if self.check_cpp_module():
                try:
                    cpp.demonstrate_vector_erase()
                except Exception as e:
                    print(f"Error running demonstration: {e}")
        elif choice == "0":
            return
        else:
            print("Invalid choice")

    def run_task2_benchmark(self):
        """Run task 2 benchmark"""
        if not self.check_cpp_module():
            return
        
        vector_size = self.get_int_input("Vector size", 100000)
        iterations = self.get_int_input("Number of iterations", 100)
        thread_count = self.get_int_input("Number of threads", 1)

        print("\nRunning comparative benchmark...")
        print(f"Vector size: {vector_size}, Iterations: {iterations}, Threads: {thread_count}")
        
        # Define methods with their benchmark functions
        methods = [
            ("Naive erase", "naive_erase", cpp.benchmark_naive_erase),
            ("remove_if + erase", "remove_if_erase", cpp.benchmark_remove_if_erase),
            ("Iterators", "iterators_erase", cpp.benchmark_iterators_erase),
            ("Copy to new vector", "copy_erase", cpp.benchmark_copy_erase),
            ("Partition", "partition_erase", cpp.benchmark_partition_erase)
        ]
        
        results = []
        
        try:
            for method_name, method_key, benchmark_func in methods:
                execution_time_ns = benchmark_func(vector_size, iterations, thread_count)
                execution_time_ms = execution_time_ns / 1e6
                operations_per_second = iterations / (execution_time_ns / 1e9)
                results.append((method_name, method_key, execution_time_ns, execution_time_ms, operations_per_second))
            
            # Display results table
            print("\n" + "=" * 80)
            print(f"{'Method':<25} {'Time (ms)':<15} {'Ops/sec':<20}")
            print("=" * 80)
            
            fastest_time_ns = min(r[2] for r in results)
            
            for method_name, method_key, execution_time_ns, execution_time_ms, ops_per_sec in results:
                is_fastest = execution_time_ns == fastest_time_ns
                marker = " ⭐ FASTEST" if is_fastest else ""
                print(f"{method_name:<25} {execution_time_ms:>12.2f}  {ops_per_sec:>15,.0f}{marker}")
            
            print("=" * 80)
            
            # Save to DB
            if self.db.is_connected():
                for method_name, method_key, execution_time_ns, _, ops_per_sec in results:
                    self.db.save_benchmark_result(
                        task_number=2,
                        task_name="Vector erase",
                        method_name=method_key,
                        execution_time_ns=execution_time_ns,
                        parameters={"vector_size": vector_size, "iterations": iterations},
                        thread_count=thread_count,
                        operations_per_second=ops_per_sec
                    )
                print("\n✓ Results saved to DB (5 methods)")
        except Exception as e:
            print(f"Error running benchmark: {e}")

    def task3_menu(self):
        """Task 3 menu"""
        print("\n=== Task 3: Mapping number to string ===")
        print("[1] Show container analysis")
        print("[2] Run comparative benchmark")
        print("[0] Back to main menu")
        
        choice = self.get_user_input("Select option", "0")
        
        if choice == "1":
            self.show_task3_analysis()
        elif choice == "2":
            self.run_task3_benchmark()
        elif choice == "0":
            return

    def show_task3_analysis(self):
        """Show container analysis"""
        print("\n=== Container analysis for int -> string mapping ===\n")
        print("1. std::map<int, std::string>")
        print("   - Lookup complexity: O(log n)")
        print("   - Ordered: yes")
        print("   - Use case: when ordering is needed")
        print()
        print("2. std::unordered_map<int, std::string>")
        print("   - Lookup complexity: O(1) average, O(n) worst")
        print("   - Ordered: no")
        print("   - Use case: for maximum lookup performance")
        print()
        print("3. std::vector<std::pair<int, std::string>>")
        print("   - Lookup complexity: O(n)")
        print("   - Ordered: depends on implementation")
        print("   - Use case: for small datasets (<100 elements)")
        print()
        print("Recommendation: std::unordered_map for most cases")

    def run_task3_benchmark(self):
        """Run task 3 benchmark"""
        if not self.check_cpp_module():
            return
        
        element_count = self.get_int_input("Number of elements", 100000)
        lookup_iterations = self.get_int_input("Number of lookup iterations", 1000000)

        print("\nRunning benchmark...")
        print(f"Elements: {element_count}, Lookup iterations: {lookup_iterations}")
        
        try:
            # Call benchmark functions that return BenchmarkResult objects
            map_result = cpp.benchmark_map(element_count, lookup_iterations)
            umap_result = cpp.benchmark_unordered_map(element_count, lookup_iterations)
            vec_result = cpp.benchmark_vector(element_count, lookup_iterations)
            
            results = [
                map_result,
                umap_result,
                vec_result
            ]
            
            # Display results table
            print("\n" + "=" * 100)
            print(f"{'Container':<25} {'Insert (ms)':<15} {'Lookup (ms)':<15} {'Erase (ms)':<15} {'Memory (MB)':<15}")
            print("=" * 100)
            
            fastest_lookup_time_ns = min(r.lookup_time_ns for r in results)
            
            for result in results:
                insert_ms = result.insert_time_ns / 1e6
                lookup_ms = result.lookup_time_ns / 1e6
                erase_ms = result.erase_time_ns / 1e6
                memory_mb = result.memory_usage_bytes / (1024 * 1024)
                
                is_fastest = result.lookup_time_ns == fastest_lookup_time_ns
                marker = " ⭐ FASTEST LOOKUP" if is_fastest else ""
                
                print(f"{result.container_name:<25} {insert_ms:>12.2f}  {lookup_ms:>12.2f}  {erase_ms:>12.2f}  {memory_mb:>12.2f}{marker}")
            
            print("=" * 100)
            
            # Display recommendation
            fastest_container = min(results, key=lambda r: r.lookup_time_ns)
            print(f"\nRecommendation: Use {fastest_container.container_name} for fastest lookup performance")
            
            # Save to DB
            if self.db.is_connected():
                for result in results:
                    parameters = {
                        "element_count": element_count,
                        "lookup_iterations": lookup_iterations,
                        "insert_time_ns": result.insert_time_ns,
                        "erase_time_ns": result.erase_time_ns,
                        "memory_usage_bytes": result.memory_usage_bytes
                    }
                    
                    self.db.save_benchmark_result(
                        task_number=3,
                        task_name="Mapping benchmark",
                        method_name=result.container_name,
                        execution_time_ns=result.lookup_time_ns,  # Primary metric
                        parameters=parameters,
                        thread_count=1,
                        operations_per_second=lookup_iterations / (result.lookup_time_ns / 1e9)
                    )
                print("\n✓ Results saved to DB (3 containers)")
        except Exception as e:
            print(f"Error running benchmark: {e}")

    def show_results(self):
        """Show results from DB"""
        if not self.db.is_connected():
            print("DB not connected")
            return

        print("\n=== View results ===")
        print("[1] All results")
        print("[2] Task 1 results")
        print("[3] Task 2 results")
        print("[4] Task 3 results")
        print("[0] Back to main menu")
        
        choice = self.get_user_input("Select option", "0")
        
        task_number = None
        if choice == "2":
            task_number = 1
        elif choice == "3":
            task_number = 2
        elif choice == "4":
            task_number = 3
        elif choice == "0":
            return

        results = self.db.get_results(task_number=task_number, limit=50)
        
        if not results:
            print("No saved results")
            return

        print(f"\n{'Date/Time':<20} {'Task':<30} {'Method':<30} {'Time (ns)':<15} {'Threads':<10}")
        print("-" * 105)
        for result in results:
            timestamp = result['timestamp'].strftime('%Y-%m-%d %H:%M:%S')
            print(f"{timestamp:<20} {result['task_name']:<30} {result['method_name']:<30} "
                  f"{result['execution_time_ns']:<15} {result['thread_count']:<10}")

    def run_asio_server(self):
        """Start Boost.Asio server"""
        # Determine server executable path relative to script location
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(script_dir)
        server_path = os.path.join(project_root, "build", "asio_server")
        
        # Check if executable exists
        if not os.path.exists(server_path):
            print(f"\n{'=' * 70}")
            print(f"ERROR: Server executable not found: {server_path}")
            print(f"{'=' * 70}")
            print("To build the server, run:")
            print("  mkdir -p build && cd build && cmake .. && make")
            print(f"{'=' * 70}\n")
            return
        
        # Display server information
        print("\n" + "=" * 70)
        print("Starting Boost.Asio HTTP Server")
        print("=" * 70)
        print("Server URL: http://localhost:8080")
        print("\nAvailable endpoints:")
        print("  GET /health              - Server status")
        print("  GET /benchmark/task1     - weak_ptr::lock() benchmark")
        print("  GET /benchmark/task2?size=N - Vector erase benchmark")
        print("  GET /benchmark/task3?size=N - Mapping benchmark")
        print("  GET /results             - Results from DB")
        print("\nPress Ctrl+C to stop the server")
        print("=" * 70 + "\n")
        
        # Start server process
        try:
            process = subprocess.Popen(
                [server_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                bufsize=1,
                universal_newlines=True
            )
            
            # Stream output with prefix
            try:
                for line in process.stdout:
                    print(f"[server] {line.rstrip()}")
                    sys.stdout.flush()
            except KeyboardInterrupt:
                print("\n\nStopping server...")
                process.terminate()
                
                # Wait for graceful shutdown with timeout
                try:
                    process.wait(timeout=5)
                    print("Server stopped.")
                except subprocess.TimeoutExpired:
                    print("Server did not stop gracefully, killing...")
                    process.kill()
                    process.wait()
                    print("Server stopped.")
            
        except Exception as e:
            print(f"Error starting server: {e}")
            return

    def run_all_tasks(self):
        """Run all tasks"""
        # Check module availability
        if not self.check_cpp_module():
            return
        
        print("\n" + "=" * 70)
        print("Running All Tasks - Complete Demonstration")
        print("=" * 70)
        
        # Default parameters
        task1_iterations = 1000000
        task1_threads = 1
        task2_vector_size = 100000
        task2_iterations = 100
        task2_threads = 1
        task3_element_count = 100000
        task3_lookup_iterations = 1000000
        
        results_summary = []
        
        # Task 1: Demo and benchmark
        print("\n📌 Task 1: weak_ptr::lock()")
        print("-" * 70)
        try:
            print("Running demonstration...")
            cpp.demonstrate_weak_ptr_lock()
            time.sleep(0.5)
            
            print(f"\nRunning benchmark: iterations={task1_iterations}, threads={task1_threads}")
            execution_time_ns = cpp.benchmark_weak_ptr_lock(task1_iterations, task1_threads)
            execution_time_ms = execution_time_ns / 1e6
            operations_per_second = task1_iterations / (execution_time_ns / 1e9)
            
            print(f"  ✓ Execution time: {execution_time_ms:.2f} ms")
            print(f"  ✓ Operations per second: {operations_per_second:,.0f}")
            
            if self.db.is_connected():
                self.db.save_benchmark_result(
                    task_number=1,
                    task_name="weak_ptr::lock()",
                    method_name="CustomWeakPtr::lock()",
                    execution_time_ns=execution_time_ns,
                    parameters={"iterations": task1_iterations},
                    thread_count=task1_threads,
                    operations_per_second=operations_per_second
                )
                print("  ✓ Results saved to DB")
            
            results_summary.append(("Task 1", execution_time_ms, "ms"))
        except Exception as e:
            print(f"  ⚠ Error: {e}")
            results_summary.append(("Task 1", "ERROR", ""))
        
        time.sleep(1)
        
        # Task 2: Benchmark
        print("\n📌 Task 2: Vector erase")
        print("-" * 70)
        print(f"Running benchmark: vector_size={task2_vector_size}, iterations={task2_iterations}, threads={task2_threads}")
        
        methods = [
            ("Naive erase", "naive_erase", cpp.benchmark_naive_erase),
            ("remove_if + erase", "remove_if_erase", cpp.benchmark_remove_if_erase),
            ("Iterators", "iterators_erase", cpp.benchmark_iterators_erase),
            ("Copy to new vector", "copy_erase", cpp.benchmark_copy_erase),
            ("Partition", "partition_erase", cpp.benchmark_partition_erase)
        ]
        
        task2_results = []
        try:
            for method_name, method_key, benchmark_func in methods:
                execution_time_ns = benchmark_func(task2_vector_size, task2_iterations, task2_threads)
                execution_time_ms = execution_time_ns / 1e6
                operations_per_second = task2_iterations / (execution_time_ns / 1e9)
                task2_results.append((method_name, method_key, execution_time_ns, execution_time_ms, operations_per_second))
            
            fastest_time_ns = min(r[2] for r in task2_results)
            fastest_method = next(r[0] for r in task2_results if r[2] == fastest_time_ns)
            
            print(f"  ✓ Fastest method: {fastest_method} ({min(r[3] for r in task2_results):.2f} ms)")
            
            if self.db.is_connected():
                for method_name, method_key, execution_time_ns, _, ops_per_sec in task2_results:
                    self.db.save_benchmark_result(
                        task_number=2,
                        task_name="Vector erase",
                        method_name=method_key,
                        execution_time_ns=execution_time_ns,
                        parameters={"vector_size": task2_vector_size, "iterations": task2_iterations},
                        thread_count=task2_threads,
                        operations_per_second=ops_per_sec
                    )
                print("  ✓ Results saved to DB (5 methods)")
            
            results_summary.append(("Task 2", min(r[3] for r in task2_results), "ms (fastest)"))
        except Exception as e:
            print(f"  ⚠ Error: {e}")
            results_summary.append(("Task 2", "ERROR", ""))
        
        time.sleep(1)
        
        # Task 3: Benchmark
        print("\n📌 Task 3: Mapping int→string")
        print("-" * 70)
        print(f"Running benchmark: elements={task3_element_count}, lookup_iterations={task3_lookup_iterations}")
        
        try:
            map_result = cpp.benchmark_map(task3_element_count, task3_lookup_iterations)
            umap_result = cpp.benchmark_unordered_map(task3_element_count, task3_lookup_iterations)
            vec_result = cpp.benchmark_vector(task3_element_count, task3_lookup_iterations)
            
            results = [map_result, umap_result, vec_result]
            fastest_lookup_time_ns = min(r.lookup_time_ns for r in results)
            fastest_container = min(results, key=lambda r: r.lookup_time_ns)
            
            fastest_lookup_ms = fastest_lookup_time_ns / 1e6
            print(f"  ✓ Fastest container: {fastest_container.container_name} ({fastest_lookup_ms:.2f} ms)")
            
            if self.db.is_connected():
                for result in results:
                    parameters = {
                        "element_count": task3_element_count,
                        "lookup_iterations": task3_lookup_iterations,
                        "insert_time_ns": result.insert_time_ns,
                        "erase_time_ns": result.erase_time_ns,
                        "memory_usage_bytes": result.memory_usage_bytes
                    }
                    
                    self.db.save_benchmark_result(
                        task_number=3,
                        task_name="Mapping benchmark",
                        method_name=result.container_name,
                        execution_time_ns=result.lookup_time_ns,
                        parameters=parameters,
                        thread_count=1,
                        operations_per_second=task3_lookup_iterations / (result.lookup_time_ns / 1e9)
                    )
                print("  ✓ Results saved to DB (3 containers)")
            
            results_summary.append(("Task 3", fastest_lookup_ms, "ms (fastest lookup)"))
        except Exception as e:
            print(f"  ⚠ Error: {e}")
            results_summary.append(("Task 3", "ERROR", ""))
        
        # Display summary
        print("\n" + "=" * 70)
        print("Summary of Results")
        print("=" * 70)
        for task_name, value, unit in results_summary:
            if value == "ERROR":
                print(f"  {task_name:<20} {value}")
            else:
                print(f"  {task_name:<20} {value:>10.2f} {unit}")
        print("=" * 70 + "\n")

    def run_task1_benchmark_auto(self, iterations: int, threads: int):
        """Auto run task 1 benchmark"""
        print(f"  Iterations: {iterations}, Threads: {threads}")
        
        if not CPP_MODULE_AVAILABLE:
            print("  ⚠ C++ module not available, skipping benchmark")
            return
        
        try:
            execution_time_ns = cpp.benchmark_weak_ptr_lock(iterations, threads)
            operations_per_second = iterations / (execution_time_ns / 1e9)
            
            if self.db.is_connected():
                self.db.save_benchmark_result(
                    task_number=1,
                    task_name="weak_ptr::lock()",
                    method_name="CustomWeakPtr::lock()",
                    execution_time_ns=execution_time_ns,
                    parameters={"iterations": iterations},
                    thread_count=threads,
                    operations_per_second=operations_per_second
                )
                print("  ✓ Results saved to DB")
        except Exception as e:
            print(f"  ⚠ Error running benchmark: {e}")

    def run_task2_benchmark_auto(self, vector_size: int, iterations: int, threads: int):
        """Auto run task 2 benchmark"""
        print(f"  Vector size: {vector_size}, Iterations: {iterations}, Threads: {threads}")
        
        if not CPP_MODULE_AVAILABLE:
            print("  ⚠ C++ module not available, skipping benchmark")
            return
        
        methods = [
            ("naive_erase", cpp.benchmark_naive_erase),
            ("remove_if_erase", cpp.benchmark_remove_if_erase),
            ("iterators_erase", cpp.benchmark_iterators_erase),
            ("copy_erase", cpp.benchmark_copy_erase),
            ("partition_erase", cpp.benchmark_partition_erase)
        ]
        
        try:
            for method_name, benchmark_func in methods:
                execution_time_ns = benchmark_func(vector_size, iterations, threads)
                operations_per_second = iterations / (execution_time_ns / 1e9)
                
                if self.db.is_connected():
                    self.db.save_benchmark_result(
                        task_number=2,
                        task_name="Vector erase",
                        method_name=method_name,
                        execution_time_ns=execution_time_ns,
                        parameters={"vector_size": vector_size, "iterations": iterations},
                        thread_count=threads,
                        operations_per_second=operations_per_second
                    )
            print("  ✓ Results saved to DB (5 methods)")
        except Exception as e:
            print(f"  ⚠ Error running benchmark: {e}")

    def run_task3_benchmark_auto(self, element_count: int, lookup_iterations: int):
        """Auto run task 3 benchmark"""
        print(f"  Elements: {element_count}, Lookup iterations: {lookup_iterations}")
        
        if not CPP_MODULE_AVAILABLE:
            print("  ⚠ C++ module not available, skipping benchmark")
            return
        
        try:
            # Call benchmark functions that return BenchmarkResult objects
            map_result = cpp.benchmark_map(element_count, lookup_iterations)
            umap_result = cpp.benchmark_unordered_map(element_count, lookup_iterations)
            vec_result = cpp.benchmark_vector(element_count, lookup_iterations)
            
            results = [map_result, umap_result, vec_result]
            
            for result in results:
                parameters = {
                    "element_count": element_count,
                    "lookup_iterations": lookup_iterations,
                    "insert_time_ns": result.insert_time_ns,
                    "erase_time_ns": result.erase_time_ns,
                    "memory_usage_bytes": result.memory_usage_bytes
                }
                
                if self.db.is_connected():
                    self.db.save_benchmark_result(
                        task_number=3,
                        task_name="Mapping benchmark",
                        method_name=result.container_name,
                        execution_time_ns=result.lookup_time_ns,  # Primary metric
                        parameters=parameters,
                        thread_count=1,
                        operations_per_second=lookup_iterations / (result.lookup_time_ns / 1e9)
                    )
            print("  ✓ Results saved to DB (3 containers)")
        except Exception as e:
            print(f"  ⚠ Error running benchmark: {e}")

    def run(self):
        """Main loop"""
        global _shutdown_requested
        while self.running and not _shutdown_requested:
            self.print_menu()
            choice = self.get_user_input("Select option", "0")
            
            if choice == "1":
                self.task1_menu()
            elif choice == "2":
                self.task2_menu()
            elif choice == "3":
                self.task3_menu()
            elif choice == "4":
                self.run_all_tasks()
            elif choice == "5":
                self.show_results()
            elif choice == "6":
                self.run_asio_server()
            elif choice == "0":
                print("Exiting...")
                self.running = False
            else:
                print("Invalid choice. Please enter a number from 0 to 6")

        if self.db:
            self.db.close()


def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='C++ Interview Demo - Interactive console')
    parser.add_argument('--autorun', action='store_true',
                        help='Automatically run all benchmarks and exit')
    parser.add_argument('--task', '-t', type=int, choices=[1, 2, 3],
                        help='Run benchmark for specified task only')
    parser.add_argument('--iterations', '-i', type=int, default=1000000,
                        help='Number of iterations for benchmarks (default: 1000000)')
    parser.add_argument('--threads', '-j', type=int, default=1,
                        help='Number of threads (default: 1)')
    parser.add_argument('--vector-size', '-s', type=int, default=100000,
                        help='Vector size for task 2 (default: 100000)')
    
    args = parser.parse_args()
    
    console = InterviewDemoConsole()
    
    try:
        if args.autorun or args.task:
            # Automatic mode
            print("=" * 50)
            print("C++ Interview Demo - Automatic mode")
            print("=" * 50)
            
            if args.task == 1 or args.autorun:
                print("\n📌 Task 1: weak_ptr::lock()")
                console.run_task1_benchmark_auto(args.iterations, args.threads)
            
            if args.task == 2 or args.autorun:
                print("\n📌 Task 2: Vector erase")
                console.run_task2_benchmark_auto(args.vector_size, 100, args.threads)
            
            if args.task == 3 or args.autorun:
                print("\n📌 Task 3: Mapping int→string")
                console.run_task3_benchmark_auto(args.vector_size, args.iterations)
            
            print("\n✅ Benchmarks completed!")
            print("To view results: python3 python/view_results.py")
        else:
            # Interactive mode
            console.run()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    finally:
        if console.db:
            console.db.close()


if __name__ == "__main__":
    main()
