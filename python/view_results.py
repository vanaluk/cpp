#!/usr/bin/env python3
"""
Utility for viewing benchmark results from PostgreSQL

Usage:
  python3 view_results.py                    # All results
  python3 view_results.py --task 1           # Task 1 only
  python3 view_results.py --task 2           # Task 2 only
  python3 view_results.py --task 3           # Task 3 only
  python3 view_results.py --build Release    # Filter by build type
  python3 view_results.py --stats            # Statistics by methods
  python3 view_results.py --compare          # Compare Release vs Debug
  python3 view_results.py --export csv       # Export to CSV
  python3 view_results.py --export json      # Export to JSON
  python3 view_results.py --limit 100        # Limit number of records
  python3 view_results.py --watch            # Monitoring mode (update every 5 sec)
"""

import sys
import os
import argparse
import json
import csv
import time
from datetime import datetime
from typing import Optional, List, Dict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from db_manager import DatabaseManager


class ResultsViewer:
    def __init__(self):
        self.db = DatabaseManager()
        if not self.db.is_connected():
            print("❌ Error: failed to connect to DB")
            print("Make sure PostgreSQL is running")
            sys.exit(1)

    def format_time_ns(self, ns: int) -> str:
        """Format time to readable format"""
        if ns < 1000:
            return f"{ns} ns"
        elif ns < 1_000_000:
            return f"{ns / 1000:.2f} μs"
        elif ns < 1_000_000_000:
            return f"{ns / 1_000_000:.2f} ms"
        else:
            return f"{ns / 1_000_000_000:.2f} s"

    def format_ops(self, ops: Optional[float]) -> str:
        """Format operations per second"""
        if ops is None:
            return "N/A"
        if ops < 1000:
            return f"{ops:.2f}"
        elif ops < 1_000_000:
            return f"{ops / 1000:.2f}K"
        else:
            return f"{ops / 1_000_000:.2f}M"

    def print_header(self):
        """Print table header"""
        print()
        print("=" * 135)
        print(
            f"{'Date/Time':<20} {'Build':<8} {'Task':<22} {'Method':<28} {'Time':<15} {'Ops/sec':<12} {'Threads':<8}"
        )
        print("=" * 135)

    def print_results(self, results: List[Dict]):
        """Print results as table"""
        if not results:
            print("📭 No saved results")
            return

        self.print_header()
        
        for r in results:
            timestamp = r['timestamp'].strftime('%Y-%m-%d %H:%M:%S') if r['timestamp'] else 'N/A'
            build_type = r.get("build_type", "N/A")[:7]
            task_name = r.get("task_name", f"Task {r.get('task_number', '?')}")[:21]
            method_name = r.get("method_name", "Unknown")[:27]
            time_str = self.format_time_ns(r.get('execution_time_ns', 0))
            ops_str = self.format_ops(r.get('operations_per_second'))
            threads = r.get('thread_count', 1)

            print(
                f"{timestamp:<20} {build_type:<8} {task_name:<22} {method_name:<28} {time_str:<15} {ops_str:<12} {threads:<8}"
            )

        print("-" * 135)
        print(f"Total records: {len(results)}")

    def print_statistics(
        self, task_number: Optional[int] = None, build_type: Optional[str] = None
    ):
        """Print statistics by methods"""
        build_filter = f" ({build_type})" if build_type else ""
        print(f"\n📊 STATISTICS BY METHODS{build_filter}")
        print("=" * 110)

        tasks = [task_number] if task_number else [1, 2, 3]

        for task in tasks:
            stats = self.db.get_task_statistics(task, build_type)
            if not stats:
                continue

            print(f"\n📌 Task {task}")
            print("-" * 95)
            print(
                f"{'Method':<30} {'Build':<10} {'Runs':<8} {'Average':<15} {'Min':<15} {'Max':<15}"
            )
            print("-" * 95)

            for s in stats:
                method = s.get("method_name", "Unknown")[:29]
                build = s.get("build_type", "N/A")[:9]
                count = s.get("count", 0)
                avg = self.format_time_ns(int(s.get("avg_time_ns", 0)))
                min_t = self.format_time_ns(int(s.get("min_time_ns", 0)))
                max_t = self.format_time_ns(int(s.get("max_time_ns", 0)))

                print(
                    f"{method:<30} {build:<10} {count:<8} {avg:<15} {min_t:<15} {max_t:<15}"
                )

    def print_comparison(self, task_number: Optional[int] = None):
        """Print Release vs Debug comparison"""
        print("\n⚡ RELEASE vs DEBUG COMPARISON")
        print("=" * 120)

        tasks = [task_number] if task_number else [1, 2, 3]

        for task in tasks:
            comparison = self.db.compare_build_types(task)
            if not comparison:
                continue

            # Group by method
            methods = {}
            for row in comparison:
                method = row["method_name"]
                build = row["build_type"]
                if method not in methods:
                    methods[method] = {}
                methods[method][build] = row

            print(f"\n📌 Task {task}")
            print("-" * 110)
            print(
                f"{'Method':<35} {'Release Time':<18} {'Debug Time':<18} {'Speedup':<12} {'Release Ops/s':<15}"
            )
            print("-" * 110)

            for method, builds in methods.items():
                release = builds.get("Release", {})
                debug = builds.get("Debug", {})

                release_time = int(release.get("avg_time_ns", 0)) if release else 0
                debug_time = int(debug.get("avg_time_ns", 0)) if debug else 0
                release_ops = release.get("avg_ops_per_sec") if release else None

                release_str = (
                    self.format_time_ns(release_time) if release_time else "N/A"
                )
                debug_str = self.format_time_ns(debug_time) if debug_time else "N/A"
                ops_str = self.format_ops(release_ops)

                # Calculate speedup
                if release_time and debug_time:
                    speedup = debug_time / release_time
                    speedup_str = f"{speedup:.2f}x"
                    if speedup > 1:
                        speedup_str = f"🚀 {speedup_str}"
                else:
                    speedup_str = "N/A"

                print(
                    f"{method[:34]:<35} {release_str:<18} {debug_str:<18} {speedup_str:<12} {ops_str:<15}"
                )

    def export_csv(self, results: List[Dict], filename: str = "benchmark_results.csv"):
        """Export to CSV"""
        if not results:
            print("❌ No data to export")
            return
        
        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.DictWriter(
                f,
                fieldnames=[
                    "timestamp",
                    "build_type",
                    "task_number",
                    "task_name",
                    "method_name",
                    "execution_time_ns",
                    "operations_per_second",
                    "thread_count",
                    "parameters",
                ],
            )
            writer.writeheader()
            
            for r in results:
                row = {
                    "timestamp": r["timestamp"].isoformat() if r["timestamp"] else "",
                    "build_type": r.get("build_type", "Release"),
                    "task_number": r.get("task_number", ""),
                    "task_name": r.get("task_name", ""),
                    "method_name": r.get("method_name", ""),
                    "execution_time_ns": r.get("execution_time_ns", ""),
                    "operations_per_second": r.get("operations_per_second", ""),
                    "thread_count": r.get("thread_count", 1),
                    "parameters": json.dumps(r.get("parameters", {})),
                }
                writer.writerow(row)
        
        print(f"✅ Results exported to {filename}")

    def export_json(self, results: List[Dict], filename: str = "benchmark_results.json"):
        """Export to JSON"""
        if not results:
            print("❌ No data to export")
            return
        
        # Convert datetime to strings
        export_data = []
        for r in results:
            row = dict(r)
            if row.get('timestamp'):
                row['timestamp'] = row['timestamp'].isoformat()
            export_data.append(row)
        
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(export_data, f, indent=2, ensure_ascii=False)
        
        print(f"✅ Results exported to {filename}")

    def watch_mode(self, task_number: Optional[int] = None, interval: int = 5):
        """Monitoring mode - update every N seconds"""
        print(f"👁️ Monitoring mode (update every {interval} sec). Press Ctrl+C to exit.")
        
        try:
            while True:
                # Clear screen
                os.system('clear' if os.name == 'posix' else 'cls')
                
                print(f"⏰ Updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
                
                results = self.db.get_results(task_number=task_number, limit=20)
                self.print_results(results)
                
                time.sleep(interval)
        except KeyboardInterrupt:
            print("\n\n👋 Monitoring stopped")

    def close(self):
        """Close connection"""
        if self.db:
            self.db.close()


def main():
    parser = argparse.ArgumentParser(
        description="View benchmark results from PostgreSQL",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 view_results.py                    # All results
  python3 view_results.py --task 2           # Task 2 only
  python3 view_results.py --build Release    # Release builds only
  python3 view_results.py --stats            # Statistics
  python3 view_results.py --compare          # Compare Release vs Debug
  python3 view_results.py --export csv       # Export to CSV
  python3 view_results.py --watch            # Monitoring
        """,
    )
    
    parser.add_argument('--task', '-t', type=int, choices=[1, 2, 3],
                        help='Filter by task number (1, 2 or 3)')
    parser.add_argument(
        "--build",
        "-b",
        choices=["Release", "Debug"],
        help="Filter by build type (Release or Debug)",
    )
    parser.add_argument('--limit', '-l', type=int, default=50,
                        help='Maximum number of records (default: 50)')
    parser.add_argument('--stats', '-s', action='store_true',
                        help='Show statistics by methods')
    parser.add_argument(
        "--compare",
        "-c",
        action="store_true",
        help="Compare Release vs Debug performance",
    )
    parser.add_argument('--export', '-e', choices=['csv', 'json'],
                        help='Export results to file')
    parser.add_argument('--output', '-o', type=str,
                        help='Output filename for export')
    parser.add_argument('--watch', '-w', action='store_true',
                        help='Monitoring mode (update every 5 sec)')
    parser.add_argument('--interval', '-i', type=int, default=5,
                        help='Update interval for --watch (default: 5 sec)')
    
    args = parser.parse_args()
    
    viewer = ResultsViewer()
    
    try:
        if args.watch:
            viewer.watch_mode(task_number=args.task, interval=args.interval)
        elif args.compare:
            viewer.print_comparison(task_number=args.task)
        elif args.stats:
            viewer.print_statistics(task_number=args.task, build_type=args.build)
        elif args.export:
            results = viewer.db.get_results(task_number=args.task, limit=10000)
            filename = args.output or f"benchmark_results.{args.export}"
            if args.export == 'csv':
                viewer.export_csv(results, filename)
            else:
                viewer.export_json(results, filename)
        else:
            results = viewer.db.get_results(task_number=args.task, limit=args.limit)
            viewer.print_results(results)
    finally:
        viewer.close()


if __name__ == "__main__":
    main()
