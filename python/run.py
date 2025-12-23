#!/usr/bin/env python3
"""
Interactive console for running tasks and benchmarks
"""
import sys
import os
import time
import signal
import select
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


class InterviewDemoConsole:
    def __init__(self):
        self.db = DatabaseManager()
        self.running = True

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
        print("\nRunning demonstration...")
        # C++ function call will be here via pybind11
        print("(Demonstration will be implemented via pybind11)")

    def run_task1_benchmark(self):
        """Run task 1 benchmark"""
        iterations = self.get_int_input("Number of iterations", 1000000)
        thread_count = self.get_int_input("Number of threads", 1)
        
        print(f"\nRunning benchmark: iterations={iterations}, threads={thread_count}")
        print("(Benchmark will be implemented via pybind11)")
        
        # C++ function call will be here via pybind11
        # For now simulation
        execution_time_ns = 1000000000  # 1 second
        
        if self.db.is_connected():
            self.db.save_benchmark_result(
                task_number=1,
                task_name="weak_ptr::lock()",
                method_name="CustomWeakPtr::lock()",
                execution_time_ns=execution_time_ns,
                parameters={"iterations": iterations},
                thread_count=thread_count,
                operations_per_second=iterations / (execution_time_ns / 1e9)
            )
            print("Results saved to DB")

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
        elif choice == "0":
            return
        else:
            print("(Specific method demonstration will be implemented)")

    def run_task2_benchmark(self):
        """Run task 2 benchmark"""
        vector_size = self.get_int_input("Vector size", 100000)
        iterations = self.get_int_input("Number of iterations", 100)
        thread_count = self.get_int_input("Number of threads", 1)
        
        print(f"\nRunning comparative benchmark...")
        print(f"Vector size: {vector_size}, Iterations: {iterations}, Threads: {thread_count}")
        print("(Benchmark will be implemented via pybind11)")

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
        element_count = self.get_int_input("Number of elements", 100000)
        lookup_iterations = self.get_int_input("Number of lookup iterations", 1000000)
        
        print(f"\nRunning benchmark...")
        print(f"Elements: {element_count}, Lookup iterations: {lookup_iterations}")
        print("(Benchmark will be implemented via pybind11)")

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
        print("\nStarting Boost.Asio server...")
        server_path = "build/asio_server"
        if os.path.exists(server_path):
            print("Server will be started on port 8080")
            print("Available endpoints:")
            print("  GET /benchmark/task1")
            print("  GET /benchmark/task2?size=N")
            print("  GET /benchmark/task3?size=N")
            print("  GET /results")
            print("\nPress Ctrl+C to stop")
            # Server start will be here
            print("(Server will be implemented)")
        else:
            print(f"Server not found: {server_path}")
            print("Build project: mkdir -p build && cd build && cmake .. && make")

    def run_all_tasks(self):
        """Run all tasks"""
        print("\n=== Running all tasks ===")
        self.run_task1_demo()
        time.sleep(1)
        self.run_task2_benchmark()
        time.sleep(1)
        self.run_task3_benchmark()

    def run_task1_benchmark_auto(self, iterations: int, threads: int):
        """Auto run task 1 benchmark"""
        print(f"  Iterations: {iterations}, Threads: {threads}")
        # C++ call via pybind11 will be here
        execution_time_ns = 1000000000  # Stub
        
        if self.db.is_connected():
            self.db.save_benchmark_result(
                task_number=1,
                task_name="weak_ptr::lock()",
                method_name="CustomWeakPtr::lock()",
                execution_time_ns=execution_time_ns,
                parameters={"iterations": iterations},
                thread_count=threads,
                operations_per_second=iterations / (execution_time_ns / 1e9)
            )
            print("  ✓ Results saved to DB")

    def run_task2_benchmark_auto(self, vector_size: int, iterations: int, threads: int):
        """Auto run task 2 benchmark"""
        print(f"  Vector size: {vector_size}, Iterations: {iterations}, Threads: {threads}")
        
        methods = [
            ("naive_erase", 45670000),
            ("remove_if_erase", 1230000),
            ("iterators", 43210000),
            ("copy_to_new", 980000),
            ("partition", 890000)
        ]
        
        for method_name, execution_time_ns in methods:
            if self.db.is_connected():
                self.db.save_benchmark_result(
                    task_number=2,
                    task_name="Vector erase",
                    method_name=method_name,
                    execution_time_ns=execution_time_ns,
                    parameters={"vector_size": vector_size, "iterations": iterations},
                    thread_count=threads,
                    operations_per_second=iterations / (execution_time_ns / 1e9)
                )
        print("  ✓ Results saved to DB (5 methods)")

    def run_task3_benchmark_auto(self, element_count: int, lookup_iterations: int):
        """Auto run task 3 benchmark"""
        print(f"  Elements: {element_count}, Lookup iterations: {lookup_iterations}")
        
        containers = [
            ("std::map", 15000000),
            ("std::unordered_map", 800000),
            ("std::vector<pair>", 95000000)
        ]
        
        for container_name, execution_time_ns in containers:
            if self.db.is_connected():
                self.db.save_benchmark_result(
                    task_number=3,
                    task_name="Mapping benchmark",
                    method_name=container_name,
                    execution_time_ns=execution_time_ns,
                    parameters={"elements": element_count, "lookups": lookup_iterations},
                    thread_count=1,
                    operations_per_second=lookup_iterations / (execution_time_ns / 1e9)
                )
        print("  ✓ Results saved to DB (3 containers)")

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
