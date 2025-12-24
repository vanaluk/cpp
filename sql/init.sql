-- Create table for benchmark results
CREATE TABLE IF NOT EXISTS benchmark_results (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    task_number INTEGER NOT NULL,
    task_name VARCHAR(255) NOT NULL,
    method_name VARCHAR(255) NOT NULL,
    parameters JSONB,
    execution_time_ns BIGINT NOT NULL,
    operations_per_second DOUBLE PRECISION,
    thread_count INTEGER DEFAULT 1,
    build_type VARCHAR(20) DEFAULT 'Release',  -- Release or Debug
    notes TEXT
);

-- Migration: Add build_type column if it doesn't exist (for existing databases)
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.columns
        WHERE table_name = 'benchmark_results' AND column_name = 'build_type'
    ) THEN
        ALTER TABLE benchmark_results ADD COLUMN build_type VARCHAR(20) DEFAULT 'Release';
    END IF;
END $$;

-- Indexes for fast search
CREATE INDEX IF NOT EXISTS idx_benchmark_results_task ON benchmark_results(task_number);
CREATE INDEX IF NOT EXISTS idx_benchmark_results_timestamp ON benchmark_results(timestamp);
CREATE INDEX IF NOT EXISTS idx_benchmark_results_method ON benchmark_results(method_name);
CREATE INDEX IF NOT EXISTS idx_benchmark_results_build_type ON benchmark_results(build_type);

-- Table for run metadata
CREATE TABLE IF NOT EXISTS benchmark_runs (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    task_number INTEGER NOT NULL,
    total_methods INTEGER NOT NULL,
    environment_info JSONB
);
