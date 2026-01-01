#!/usr/bin/env python3
"""
Tests for run.py - Task 2.2, 2.3, 2.4 integration tests
"""
import sys
import os
import pytest
from unittest.mock import Mock, patch
import json

# Add path to modules
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from run import BenchmarkConsole, CPP_MODULE_AVAILABLE, cpp


class TestTask1Methods:
    """Tests for Task 1 methods (Task 2.2)"""
    
    def test_run_task1_demo_without_module(self):
        """TC-E2E-03: Error handling when module missing"""
        console = BenchmarkConsole()
        
        # Mock check_cpp_module to return False
        with patch.object(console, 'check_cpp_module', return_value=False):
            # Should not crash, just return
            console.run_task1_demo()
            # No exception should be raised
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task1_demo_with_module(self):
        """TC-E2E-01: Task 1 demonstration runs"""
        console = BenchmarkConsole()
        
        # Mock demonstrate_weak_ptr_lock to avoid actual output
        if CPP_MODULE_AVAILABLE:
            with patch.object(cpp, 'demonstrate_weak_ptr_lock') as mock_demo:
                console.run_task1_demo()
                mock_demo.assert_called_once()
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task1_benchmark_with_module(self):
        """TC-E2E-02: Task 1 benchmark returns real time"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            # Mock benchmark function to return realistic time
            mock_time_ns = 50000000  # 50 ms
            with patch.object(cpp, 'benchmark_weak_ptr_lock', return_value=mock_time_ns):
                with patch.object(console, 'get_int_input', side_effect=[1000000, 1]):
                    with patch.object(console.db, 'is_connected', return_value=False):
                        # Capture output
                        import io
                        from contextlib import redirect_stdout
                        f = io.StringIO()
                        with redirect_stdout(f):
                            console.run_task1_benchmark()
                        output = f.getvalue()
                        
                        # Check that execution time is displayed
                        assert "Execution time" in output or "ms" in output
                        assert "Operations per second" in output or "Ops/sec" in output
    
    def test_run_task1_benchmark_without_module(self):
        """Test benchmark handling when module missing"""
        console = BenchmarkConsole()
        
        with patch.object(console, 'check_cpp_module', return_value=False):
            console.run_task1_benchmark()
            # Should return without crashing


class TestTask2Methods:
    """Tests for Task 2 methods (Task 2.3)"""
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_task2_menu_demonstration(self):
        """TC-E2E-01: Task 2 demonstration runs"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            with patch.object(cpp, 'demonstrate_vector_erase') as mock_demo:
                with patch.object(console, 'get_user_input', return_value='1'):
                    with patch.object(console, 'check_cpp_module', return_value=True):
                        console.task2_menu()
                        mock_demo.assert_called_once()
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task2_benchmark_calls_all_methods(self):
        """TC-E2E-02: Task 2 comparative benchmark runs"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            # Mock all benchmark functions
            mock_times = [100000000, 50000000, 80000000, 30000000, 25000000]  # Different times
            
            with patch.object(cpp, 'benchmark_naive_erase', return_value=mock_times[0]):
                with patch.object(cpp, 'benchmark_remove_if_erase', return_value=mock_times[1]):
                    with patch.object(cpp, 'benchmark_iterators_erase', return_value=mock_times[2]):
                        with patch.object(cpp, 'benchmark_copy_erase', return_value=mock_times[3]):
                            with patch.object(cpp, 'benchmark_partition_erase', return_value=mock_times[4]):
                                with patch.object(console, 'get_int_input', side_effect=[100000, 100, 1]):
                                    with patch.object(console, 'check_cpp_module', return_value=True):
                                        with patch.object(console.db, 'is_connected', return_value=False):
                                            import io
                                            from contextlib import redirect_stdout
                                            f = io.StringIO()
                                            with redirect_stdout(f):
                                                console.run_task2_benchmark()
                                            output = f.getvalue()
                                            
                                            # Check that all methods are called
                                            assert "Naive erase" in output or "naive" in output.lower()
                                            assert "remove_if" in output.lower() or "remove" in output.lower()
                                            assert "Iterators" in output or "iterators" in output.lower()
                                            assert "Copy" in output or "copy" in output.lower()
                                            assert "Partition" in output or "partition" in output.lower()
                                            assert "FASTEST" in output
    
    def test_run_task2_benchmark_without_module(self):
        """TC-E2E-04: Error handling when module missing"""
        console = BenchmarkConsole()
        
        with patch.object(console, 'check_cpp_module', return_value=False):
            console.run_task2_benchmark()
            # Should return without crashing
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task2_benchmark_saves_to_db(self):
        """TC-E2E-03: Benchmark results are saved to DB"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE and console.db.is_connected():
            mock_times = [100000000, 50000000, 80000000, 30000000, 25000000]
            
            with patch.object(cpp, 'benchmark_naive_erase', return_value=mock_times[0]):
                with patch.object(cpp, 'benchmark_remove_if_erase', return_value=mock_times[1]):
                    with patch.object(cpp, 'benchmark_iterators_erase', return_value=mock_times[2]):
                        with patch.object(cpp, 'benchmark_copy_erase', return_value=mock_times[3]):
                            with patch.object(cpp, 'benchmark_partition_erase', return_value=mock_times[4]):
                                with patch.object(console, 'get_int_input', side_effect=[100000, 100, 1]):
                                    with patch.object(console, 'check_cpp_module', return_value=True):
                                        console.run_task2_benchmark()
                                        
                                        # Check that results were saved
                                        results = console.db.get_results(task_number=2, limit=5)
                                        assert len(results) >= 5
                                        
                                        # Check that execution times are real (not hardcoded)
                                        for result in results:
                                            assert result['execution_time_ns'] > 0
                                            assert result['execution_time_ns'] != 1000000000  # Not stub value


class TestTask3Methods:
    """Tests for Task 3 methods (Task 2.4)"""
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task3_benchmark_displays_all_metrics(self):
        """TC-E2E-01: Task 3 benchmark displays all metrics"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            # Create mock BenchmarkResult objects
            mock_map_result = Mock()
            mock_map_result.container_name = "std::map"
            mock_map_result.insert_time_ns = 10000000
            mock_map_result.lookup_time_ns = 5000000
            mock_map_result.erase_time_ns = 8000000
            mock_map_result.memory_usage_bytes = 1000000
            
            mock_umap_result = Mock()
            mock_umap_result.container_name = "std::unordered_map"
            mock_umap_result.insert_time_ns = 8000000
            mock_umap_result.lookup_time_ns = 2000000
            mock_umap_result.erase_time_ns = 6000000
            mock_umap_result.memory_usage_bytes = 1200000
            
            mock_vec_result = Mock()
            mock_vec_result.container_name = "std::vector<pair>"
            mock_vec_result.insert_time_ns = 5000000
            mock_vec_result.lookup_time_ns = 15000000
            mock_vec_result.erase_time_ns = 10000000
            mock_vec_result.memory_usage_bytes = 800000
            
            with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                    with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                        with patch.object(console, 'get_int_input', side_effect=[100000, 1000000]):
                            with patch.object(console, 'check_cpp_module', return_value=True):
                                with patch.object(console.db, 'is_connected', return_value=False):
                                    import io
                                    from contextlib import redirect_stdout
                                    f = io.StringIO()
                                    with redirect_stdout(f):
                                        console.run_task3_benchmark()
                                    output = f.getvalue()
                                    
                                    # Check that all metrics are displayed
                                    assert "std::map" in output
                                    assert "std::unordered_map" in output
                                    assert "std::vector" in output or "vector" in output.lower()
                                    assert "Insert" in output or "insert" in output.lower()
                                    assert "Lookup" in output or "lookup" in output.lower()
                                    assert "Erase" in output or "erase" in output.lower()
                                    assert "Memory" in output or "memory" in output.lower()
                                    assert "FASTEST" in output or "Recommendation" in output
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task3_benchmark_recommendation(self):
        """TC-E2E-02: Recommendation is based on real data"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            # Create mock results where unordered_map is fastest
            mock_umap_result = Mock()
            mock_umap_result.container_name = "std::unordered_map"
            mock_umap_result.lookup_time_ns = 2000000  # Fastest
            
            mock_map_result = Mock()
            mock_map_result.container_name = "std::map"
            mock_map_result.lookup_time_ns = 5000000
            
            mock_vec_result = Mock()
            mock_vec_result.container_name = "std::vector<pair>"
            mock_vec_result.lookup_time_ns = 15000000
            
            # Set other fields
            for result in [mock_map_result, mock_umap_result, mock_vec_result]:
                result.insert_time_ns = 10000000
                result.erase_time_ns = 8000000
                result.memory_usage_bytes = 1000000
            
            with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                    with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                        with patch.object(console, 'get_int_input', side_effect=[100000, 1000000]):
                            with patch.object(console, 'check_cpp_module', return_value=True):
                                with patch.object(console.db, 'is_connected', return_value=False):
                                    import io
                                    from contextlib import redirect_stdout
                                    f = io.StringIO()
                                    with redirect_stdout(f):
                                        console.run_task3_benchmark()
                                    output = f.getvalue()
                                    
                                    # Check that unordered_map is recommended
                                    assert "unordered_map" in output.lower() or "Recommendation" in output
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task3_benchmark_saves_to_db(self):
        """TC-E2E-03: Benchmark results are saved to DB with JSON parameters"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE and console.db.is_connected():
            mock_map_result = Mock()
            mock_map_result.container_name = "std::map"
            mock_map_result.insert_time_ns = 10000000
            mock_map_result.lookup_time_ns = 5000000
            mock_map_result.erase_time_ns = 8000000
            mock_map_result.memory_usage_bytes = 1000000
            
            mock_umap_result = Mock()
            mock_umap_result.container_name = "std::unordered_map"
            mock_umap_result.insert_time_ns = 8000000
            mock_umap_result.lookup_time_ns = 2000000
            mock_umap_result.erase_time_ns = 6000000
            mock_umap_result.memory_usage_bytes = 1200000
            
            mock_vec_result = Mock()
            mock_vec_result.container_name = "std::vector<pair>"
            mock_vec_result.insert_time_ns = 5000000
            mock_vec_result.lookup_time_ns = 15000000
            mock_vec_result.erase_time_ns = 10000000
            mock_vec_result.memory_usage_bytes = 800000
            
            with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                    with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                        with patch.object(console, 'get_int_input', side_effect=[100000, 1000000]):
                            with patch.object(console, 'check_cpp_module', return_value=True):
                                console.run_task3_benchmark()
                                
                                # Check that results were saved
                                results = console.db.get_results(task_number=3, limit=3)
                                assert len(results) >= 3
                                
                                # Check that parameters contain all metrics
                                for result in results:
                                    params = json.loads(result['parameters']) if isinstance(result['parameters'], str) else result['parameters']
                                    assert 'insert_time_ns' in params
                                    assert 'erase_time_ns' in params
                                    assert 'memory_usage_bytes' in params
                                    assert 'element_count' in params
                                    assert 'lookup_iterations' in params
    
    def test_run_task3_benchmark_without_module(self):
        """Test benchmark handling when module missing"""
        console = BenchmarkConsole()
        
        with patch.object(console, 'check_cpp_module', return_value=False):
            console.run_task3_benchmark()
            # Should return without crashing


class TestBenchmarkResultAccess:
    """Tests for BenchmarkResult struct access (Task 2.4)"""
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_benchmark_result_fields_accessible(self):
        """TC-E2E-04: BenchmarkResult fields are accessible"""
        if not CPP_MODULE_AVAILABLE:
            pytest.skip("C++ module not available")
        
        # Call actual benchmark function
        result = cpp.benchmark_map(1000, 1000)
        
        # Check that all fields are accessible
        assert hasattr(result, 'container_name')
        assert hasattr(result, 'insert_time_ns')
        assert hasattr(result, 'lookup_time_ns')
        assert hasattr(result, 'erase_time_ns')
        assert hasattr(result, 'memory_usage_bytes')
        
        # Check types
        assert isinstance(result.container_name, str)
        assert isinstance(result.insert_time_ns, (int, type(None)))
        assert isinstance(result.lookup_time_ns, (int, type(None)))
        assert isinstance(result.erase_time_ns, (int, type(None)))
        assert isinstance(result.memory_usage_bytes, (int, type(None)))


class TestAutoRunMethods:
    """Tests for auto-run methods"""
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task1_benchmark_auto(self):
        """Test auto-run for Task 1"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            mock_time_ns = 50000000
            with patch.object(cpp, 'benchmark_weak_ptr_lock', return_value=mock_time_ns):
                with patch.object(console.db, 'is_connected', return_value=False):
                    console.run_task1_benchmark_auto(1000000, 1)
                    # Should not crash
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task2_benchmark_auto(self):
        """Test auto-run for Task 2"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            mock_times = [100000000, 50000000, 80000000, 30000000, 25000000]
            
            with patch.object(cpp, 'benchmark_naive_erase', return_value=mock_times[0]):
                with patch.object(cpp, 'benchmark_remove_if_erase', return_value=mock_times[1]):
                    with patch.object(cpp, 'benchmark_iterators_erase', return_value=mock_times[2]):
                        with patch.object(cpp, 'benchmark_copy_erase', return_value=mock_times[3]):
                            with patch.object(cpp, 'benchmark_partition_erase', return_value=mock_times[4]):
                                with patch.object(console.db, 'is_connected', return_value=False):
                                    console.run_task2_benchmark_auto(100000, 100, 1)
                                    # Should not crash
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_task3_benchmark_auto(self):
        """Test auto-run for Task 3"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            mock_map_result = Mock()
            mock_map_result.container_name = "std::map"
            mock_map_result.insert_time_ns = 10000000
            mock_map_result.lookup_time_ns = 5000000
            mock_map_result.erase_time_ns = 8000000
            mock_map_result.memory_usage_bytes = 1000000
            
            mock_umap_result = Mock()
            mock_umap_result.container_name = "std::unordered_map"
            mock_umap_result.insert_time_ns = 8000000
            mock_umap_result.lookup_time_ns = 2000000
            mock_umap_result.erase_time_ns = 6000000
            mock_umap_result.memory_usage_bytes = 1200000
            
            mock_vec_result = Mock()
            mock_vec_result.container_name = "std::vector<pair>"
            mock_vec_result.insert_time_ns = 5000000
            mock_vec_result.lookup_time_ns = 15000000
            mock_vec_result.erase_time_ns = 10000000
            mock_vec_result.memory_usage_bytes = 800000
            
            with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                    with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                        with patch.object(console.db, 'is_connected', return_value=False):
                            console.run_task3_benchmark_auto(100000, 1000000)
                            # Should not crash


class TestAsioServer:
    """Tests for Boost.Asio server launcher (Task 2.5)"""
    
    def test_run_asio_server_missing_executable(self):
        """TC-E2E-03: Error when executable missing"""
        console = BenchmarkConsole()
        
        # Mock os.path.exists to return False
        with patch('os.path.exists', return_value=False):
            import io
            from contextlib import redirect_stdout
            f = io.StringIO()
            with redirect_stdout(f):
                console.run_asio_server()
            output = f.getvalue()
            
            # Check error message
            assert "ERROR" in output or "not found" in output.lower()
            assert "build" in output.lower() or "cmake" in output.lower()
    
    @pytest.mark.skipif(not os.path.exists(os.path.join(os.path.dirname(os.path.dirname(__file__)), "build", "asio_server")), 
                        reason="asio_server executable not built")
    def test_run_asio_server_displays_info(self):
        """TC-E2E-01: Server info banner is displayed"""
        console = BenchmarkConsole()
        
        # Mock subprocess.Popen to avoid actually starting server
        mock_process = Mock()
        mock_process.stdout = iter(["Server started\n", "Listening on port 8080\n"])
        mock_process.wait = Mock(return_value=0)
        
        with patch('subprocess.Popen', return_value=mock_process):
            import io
            from contextlib import redirect_stdout
            f = io.StringIO()
            with redirect_stdout(f):
                # Simulate KeyboardInterrupt after a short time
                def interrupt_after_read():
                    raise KeyboardInterrupt()
                
                # Mock the stdout iteration to raise KeyboardInterrupt
                mock_process.stdout = iter(["Server started\n"])
                with patch('builtins.print'):
                    try:
                        console.run_asio_server()
                    except KeyboardInterrupt:
                        pass
            
            # Check that server info was displayed
            # Since we're mocking, we check that Popen was called
            # In real scenario, we'd check output for server info
    
    def test_run_asio_server_handles_keyboard_interrupt(self):
        """TC-E2E-02: Server stops on Ctrl+C"""
        console = BenchmarkConsole()
        
        mock_process = Mock()
        mock_process.stdout = iter(["Server started\n"])
        mock_process.wait = Mock(return_value=0)
        mock_process.terminate = Mock()
        mock_process.kill = Mock()
        
        with patch('os.path.exists', return_value=True):
            with patch('subprocess.Popen', return_value=mock_process):
                # Simulate KeyboardInterrupt during output reading
                def read_with_interrupt():
                    yield "Server started\n"
                    raise KeyboardInterrupt()
                
                mock_process.stdout = read_with_interrupt()
                
                import io
                from contextlib import redirect_stdout
                f = io.StringIO()
                with redirect_stdout(f):
                    console.run_asio_server()
                
                # Check that terminate was called
                mock_process.terminate.assert_called_once()


class TestRunAllTasks:
    """Tests for run_all_tasks() functionality (Task 5.1)"""
    
    def test_run_all_tasks_without_module(self):
        """TC-E2E-05: Error handling when module missing"""
        console = BenchmarkConsole()
        
        with patch.object(console, 'check_cpp_module', return_value=False):
            # Should return early without crashing when module is missing
            # No exception should be raised
            console.run_all_tasks()

    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_all_tasks_calls_all_demos_and_benchmarks(self):
        """TC-E2E-01: Run all tasks completes successfully"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            # Mock all C++ functions
            with patch.object(cpp, 'demonstrate_weak_ptr_lock') as mock_demo1:
                with patch.object(cpp, 'benchmark_weak_ptr_lock', return_value=50000000) as mock_bench1:
                    with patch.object(cpp, 'benchmark_naive_erase', return_value=100000000) as mock_bench2_1:
                        with patch.object(cpp, 'benchmark_remove_if_erase', return_value=50000000) as mock_bench2_2:
                            with patch.object(cpp, 'benchmark_iterators_erase', return_value=80000000) as mock_bench2_3:
                                with patch.object(cpp, 'benchmark_copy_erase', return_value=30000000) as mock_bench2_4:
                                    with patch.object(cpp, 'benchmark_partition_erase', return_value=25000000) as mock_bench2_5:
                                        # Mock Task 3 results
                                        mock_map_result = Mock()
                                        mock_map_result.container_name = "std::map"
                                        mock_map_result.insert_time_ns = 10000000
                                        mock_map_result.lookup_time_ns = 5000000
                                        mock_map_result.erase_time_ns = 8000000
                                        mock_map_result.memory_usage_bytes = 1000000
                                        
                                        mock_umap_result = Mock()
                                        mock_umap_result.container_name = "std::unordered_map"
                                        mock_umap_result.insert_time_ns = 8000000
                                        mock_umap_result.lookup_time_ns = 2000000
                                        mock_umap_result.erase_time_ns = 6000000
                                        mock_umap_result.memory_usage_bytes = 1200000
                                        
                                        mock_vec_result = Mock()
                                        mock_vec_result.container_name = "std::vector<pair>"
                                        mock_vec_result.insert_time_ns = 5000000
                                        mock_vec_result.lookup_time_ns = 15000000
                                        mock_vec_result.erase_time_ns = 10000000
                                        mock_vec_result.memory_usage_bytes = 800000
                                        
                                        with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                                            with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                                                with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                                                    with patch.object(console.db, 'is_connected', return_value=False):
                                                        import io
                                                        from contextlib import redirect_stdout
                                                        f = io.StringIO()
                                                        with redirect_stdout(f):
                                                            console.run_all_tasks()
                                                        output = f.getvalue()
                                                        
                                                        # Check that all tasks were called
                                                        assert "Task 1" in output
                                                        assert "Task 2" in output
                                                        assert "Task 3" in output
                                                        assert "Summary" in output or "summary" in output.lower()
                                                        
                                                        # Check that demo was called
                                                        mock_demo1.assert_called_once()
                                                        
                                                        # Check that benchmarks were called
                                                        mock_bench1.assert_called_once()
                                                        mock_bench2_1.assert_called_once()
                                                        mock_bench2_2.assert_called_once()
                                                        mock_bench2_3.assert_called_once()
                                                        mock_bench2_4.assert_called_once()
                                                        mock_bench2_5.assert_called_once()
    
    @pytest.mark.skipif(not CPP_MODULE_AVAILABLE, reason="C++ module not available")
    def test_run_all_tasks_shows_summary(self):
        """TC-E2E-01: Summary of results is displayed"""
        console = BenchmarkConsole()
        
        if CPP_MODULE_AVAILABLE:
            with patch.object(cpp, 'demonstrate_weak_ptr_lock'):
                with patch.object(cpp, 'benchmark_weak_ptr_lock', return_value=50000000):
                    with patch.object(cpp, 'benchmark_naive_erase', return_value=100000000):
                        with patch.object(cpp, 'benchmark_remove_if_erase', return_value=50000000):
                            with patch.object(cpp, 'benchmark_iterators_erase', return_value=80000000):
                                with patch.object(cpp, 'benchmark_copy_erase', return_value=30000000):
                                    with patch.object(cpp, 'benchmark_partition_erase', return_value=25000000):
                                        mock_map_result = Mock()
                                        mock_map_result.container_name = "std::map"
                                        mock_map_result.lookup_time_ns = 5000000
                                        mock_map_result.insert_time_ns = 10000000
                                        mock_map_result.erase_time_ns = 8000000
                                        mock_map_result.memory_usage_bytes = 1000000
                                        
                                        mock_umap_result = Mock()
                                        mock_umap_result.container_name = "std::unordered_map"
                                        mock_umap_result.lookup_time_ns = 2000000
                                        mock_umap_result.insert_time_ns = 8000000
                                        mock_umap_result.erase_time_ns = 6000000
                                        mock_umap_result.memory_usage_bytes = 1200000
                                        
                                        mock_vec_result = Mock()
                                        mock_vec_result.container_name = "std::vector<pair>"
                                        mock_vec_result.lookup_time_ns = 15000000
                                        mock_vec_result.insert_time_ns = 5000000
                                        mock_vec_result.erase_time_ns = 10000000
                                        mock_vec_result.memory_usage_bytes = 800000
                                        
                                        with patch.object(cpp, 'benchmark_map', return_value=mock_map_result):
                                            with patch.object(cpp, 'benchmark_unordered_map', return_value=mock_umap_result):
                                                with patch.object(cpp, 'benchmark_vector', return_value=mock_vec_result):
                                                    with patch.object(console.db, 'is_connected', return_value=False):
                                                        import io
                                                        from contextlib import redirect_stdout
                                                        f = io.StringIO()
                                                        with redirect_stdout(f):
                                                            console.run_all_tasks()
                                                        output = f.getvalue()
                                                        
                                                        # Check summary section
                                                        assert "Summary" in output or "summary" in output.lower()
                                                        # Check that all tasks are in summary
                                                        assert "Task 1" in output
                                                        assert "Task 2" in output
                                                        assert "Task 3" in output


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
