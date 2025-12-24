"""
Manager for working with PostgreSQL database
"""
import os
import psycopg2
from psycopg2.extras import RealDictCursor
import json
from typing import Optional, Dict, List


def get_build_type() -> str:
    """Get build type from .build_info file or environment"""
    # First check environment variable
    build_type = os.getenv("BUILD_TYPE", "")
    if build_type:
        return build_type

    # Then check .build_info file (created during Docker build)
    build_info_path = "/app/.build_info"
    if os.path.exists(build_info_path):
        with open(build_info_path, "r") as f:
            for line in f:
                if line.startswith("BUILD_TYPE="):
                    return line.strip().split("=", 1)[1]

    # Default to Release
    return "Release"


class DatabaseManager:
    def __init__(self):
        self.conn = None
        self._connect()

    def _connect(self):
        """Connect to database"""
        try:
            self.conn = psycopg2.connect(
                host=os.getenv("DB_HOST", "localhost"),
                port=os.getenv("DB_PORT", "5432"),
                database=os.getenv("DB_NAME", "cpp_interview_db"),
                user=os.getenv("DB_USER", "cpp_interview"),
                password=os.getenv("DB_PASSWORD", "cpp_interview_pass"),
            )
            self.conn.autocommit = True
        except psycopg2.OperationalError as e:
            print(f"Database connection error: {e}")
            print("Make sure PostgreSQL is running and accessible")
            self.conn = None

    def is_connected(self) -> bool:
        """Check connection"""
        return self.conn is not None

    def save_benchmark_result(
        self,
        task_number: int,
        task_name: str,
        method_name: str,
        execution_time_ns: int,
        parameters: Optional[Dict] = None,
        thread_count: int = 1,
        operations_per_second: Optional[float] = None,
        notes: Optional[str] = None,
        build_type: Optional[str] = None,
    ) -> bool:
        """Save benchmark result"""
        if not self.is_connected():
            return False

        # Auto-detect build type if not provided
        if build_type is None:
            build_type = get_build_type()

        try:
            with self.conn.cursor() as cur:
                cur.execute(
                    """
                    INSERT INTO benchmark_results 
                    (task_number, task_name, method_name, parameters, 
                     execution_time_ns, operations_per_second, thread_count, build_type, notes)
                    VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
                """,
                    (
                        task_number,
                        task_name,
                        method_name,
                        json.dumps(parameters) if parameters else None,
                        execution_time_ns,
                        operations_per_second,
                        thread_count,
                        build_type,
                        notes,
                    ),
                )
            return True
        except Exception as e:
            print(f"Error saving result: {e}")
            return False

    def get_results(
        self,
        task_number: Optional[int] = None,
        limit: int = 100
    ) -> List[Dict]:
        """Get results from database"""
        if not self.is_connected():
            return []

        try:
            with self.conn.cursor(cursor_factory=RealDictCursor) as cur:
                if task_number:
                    cur.execute("""
                        SELECT * FROM benchmark_results
                        WHERE task_number = %s
                        ORDER BY timestamp DESC
                        LIMIT %s
                    """, (task_number, limit))
                else:
                    cur.execute("""
                        SELECT * FROM benchmark_results
                        ORDER BY timestamp DESC
                        LIMIT %s
                    """, (limit,))
                return cur.fetchall()
        except Exception as e:
            print(f"Error getting results: {e}")
            return []

    def get_task_statistics(
        self, task_number: int, build_type: Optional[str] = None
    ) -> Dict:
        """Get task statistics, optionally filtered by build type"""
        if not self.is_connected():
            return {}

        try:
            with self.conn.cursor(cursor_factory=RealDictCursor) as cur:
                if build_type:
                    cur.execute(
                        """
                        SELECT 
                            method_name,
                            build_type,
                            COUNT(*) as count,
                            AVG(execution_time_ns) as avg_time_ns,
                            MIN(execution_time_ns) as min_time_ns,
                            MAX(execution_time_ns) as max_time_ns,
                            AVG(operations_per_second) as avg_ops_per_sec
                        FROM benchmark_results
                        WHERE task_number = %s AND build_type = %s
                        GROUP BY method_name, build_type
                        ORDER BY avg_time_ns
                    """,
                        (task_number, build_type),
                    )
                else:
                    cur.execute(
                        """
                        SELECT 
                            method_name,
                            build_type,
                            COUNT(*) as count,
                            AVG(execution_time_ns) as avg_time_ns,
                            MIN(execution_time_ns) as min_time_ns,
                            MAX(execution_time_ns) as max_time_ns,
                            AVG(operations_per_second) as avg_ops_per_sec
                        FROM benchmark_results
                        WHERE task_number = %s
                        GROUP BY method_name, build_type
                        ORDER BY build_type, avg_time_ns
                    """,
                        (task_number,),
                    )
                return cur.fetchall()
        except Exception as e:
            print(f"Error getting statistics: {e}")
            return {}

    def compare_build_types(
        self, task_number: int, method_name: Optional[str] = None
    ) -> List[Dict]:
        """Compare Release vs Debug performance for a task"""
        if not self.is_connected():
            return []

        try:
            with self.conn.cursor(cursor_factory=RealDictCursor) as cur:
                if method_name:
                    cur.execute(
                        """
                        SELECT 
                            method_name,
                            build_type,
                            COUNT(*) as runs,
                            AVG(execution_time_ns) as avg_time_ns,
                            MIN(execution_time_ns) as best_time_ns,
                            AVG(operations_per_second) as avg_ops_per_sec
                        FROM benchmark_results
                        WHERE task_number = %s AND method_name = %s
                        GROUP BY method_name, build_type
                        ORDER BY method_name, build_type
                    """,
                        (task_number, method_name),
                    )
                else:
                    cur.execute(
                        """
                        SELECT 
                            method_name,
                            build_type,
                            COUNT(*) as runs,
                            AVG(execution_time_ns) as avg_time_ns,
                            MIN(execution_time_ns) as best_time_ns,
                            AVG(operations_per_second) as avg_ops_per_sec
                        FROM benchmark_results
                        WHERE task_number = %s
                        GROUP BY method_name, build_type
                        ORDER BY method_name, build_type
                    """,
                        (task_number,),
                    )
                return cur.fetchall()
        except Exception as e:
            print(f"Error comparing build types: {e}")
            return []

    def close(self):
        """Close connection"""
        if self.conn:
            self.conn.close()
            self.conn = None
