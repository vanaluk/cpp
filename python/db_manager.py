"""
Manager for working with PostgreSQL database
"""
import os
import psycopg2
from psycopg2.extras import RealDictCursor
import json
from typing import Optional, Dict, List


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
        notes: Optional[str] = None
    ) -> bool:
        """Save benchmark result"""
        if not self.is_connected():
            return False

        try:
            with self.conn.cursor() as cur:
                cur.execute("""
                    INSERT INTO benchmark_results 
                    (task_number, task_name, method_name, parameters, 
                     execution_time_ns, operations_per_second, thread_count, notes)
                    VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
                """, (
                    task_number, task_name, method_name,
                    json.dumps(parameters) if parameters else None,
                    execution_time_ns, operations_per_second, thread_count, notes
                ))
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

    def get_task_statistics(self, task_number: int) -> Dict:
        """Get task statistics"""
        if not self.is_connected():
            return {}

        try:
            with self.conn.cursor(cursor_factory=RealDictCursor) as cur:
                cur.execute("""
                    SELECT 
                        method_name,
                        COUNT(*) as count,
                        AVG(execution_time_ns) as avg_time_ns,
                        MIN(execution_time_ns) as min_time_ns,
                        MAX(execution_time_ns) as max_time_ns,
                        AVG(operations_per_second) as avg_ops_per_sec
                    FROM benchmark_results
                    WHERE task_number = %s
                    GROUP BY method_name
                    ORDER BY avg_time_ns
                """, (task_number,))
                return cur.fetchall()
        except Exception as e:
            print(f"Error getting statistics: {e}")
            return {}

    def close(self):
        """Close connection"""
        if self.conn:
            self.conn.close()
            self.conn = None
