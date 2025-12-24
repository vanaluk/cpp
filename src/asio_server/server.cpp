#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <map>
#include <regex>
#include "task1_weak_ptr/custom_weak_ptr.hpp"
#include "task2_vector_erase/vector_erase.hpp"
#include "task3_mapping/container_benchmark.hpp"

using boost::asio::ip::tcp;

/**
 * Boost.Asio HTTP server for benchmarks
 *
 * Endpoints:
 *   GET /benchmark/task1              - Benchmark weak_ptr::lock()
 *   GET /benchmark/task2?size=N       - Benchmark vector erase
 *   GET /benchmark/task3?size=N       - Benchmark mapping
 *   GET /results                      - Get all results
 *   GET /health                       - Health check
 */

class HttpServer {
public:
	HttpServer(boost::asio::io_context& io_context, short port) :
		acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
		start_accept();
	}

private:
	void start_accept() {
		auto new_session= std::make_shared<Session>(acceptor_.get_executor());
		acceptor_.async_accept(new_session->socket(),
			[this, new_session](boost::system::error_code ec) {
				if(!ec) {
					new_session->start();
				}
				start_accept();
			});
	}

	class Session : public std::enable_shared_from_this<Session> {
	public:
		explicit Session(boost::asio::any_io_executor executor) :
			socket_(executor) {}

		tcp::socket& socket() {
			return socket_;
		}

		void start() {
			socket_.async_read_some(boost::asio::buffer(data_),
				std::bind(&Session::handle_read, shared_from_this(),
					std::placeholders::_1, std::placeholders::_2));
		}

	private:
		void handle_read(boost::system::error_code ec, std::size_t bytes_transferred) {
			if(!ec) {
				std::string request(data_.data(), bytes_transferred);
				std::string response= process_request(request);

				boost::asio::async_write(socket_,
					boost::asio::buffer(response),
					std::bind(&Session::handle_write, shared_from_this(),
						std::placeholders::_1));
			}
		}

		void handle_write(boost::system::error_code ec) {
			if(!ec) {
				socket_.shutdown(tcp::socket::shutdown_send);
			}
		}

		// Parse query parameters
		std::map<std::string, std::string> parse_query(const std::string& url) {
			std::map<std::string, std::string> params;
			size_t pos= url.find('?');
			if(pos == std::string::npos)
				return params;

			std::string query= url.substr(pos + 1);
			std::istringstream iss(query);
			std::string pair;

			while(std::getline(iss, pair, '&')) {
				size_t eq= pair.find('=');
				if(eq != std::string::npos) {
					params[pair.substr(0, eq)]= pair.substr(eq + 1);
				}
			}
			return params;
		}

		int get_param_int(const std::map<std::string, std::string>& params,
			const std::string& key, int default_value) {
			auto it= params.find(key);
			if(it != params.end()) {
				try {
					return std::stoi(it->second);
				} catch(...) {}
			}
			return default_value;
		}

		// Get positive integer parameter (returns 0 for invalid/negative values)
		size_t get_param_size(const std::map<std::string, std::string>& params,
			const std::string& key, size_t default_value, size_t max_value= 100000000) {
			auto it= params.find(key);
			if(it != params.end()) {
				try {
					long long val= std::stoll(it->second);
					if(val <= 0) {
						return 0; // Invalid: non-positive
					}
					if(static_cast<unsigned long long>(val) > max_value) {
						return 0; // Invalid: exceeds max
					}
					return static_cast<size_t>(val);
				} catch(...) {
					return 0; // Invalid: parse error
				}
			}
			return default_value;
		}

		std::string json_response(int code, const std::string& body) {
			std::stringstream ss;
			std::string status_text= (code == 200) ? "OK" : (code == 400) ? "Bad Request"
																		  : "Error";
			ss << "HTTP/1.1 " << code << " " << status_text << "\r\n";
			ss << "Content-Type: application/json\r\n";
			ss << "Access-Control-Allow-Origin: *\r\n";
			ss << "Content-Length: " << body.length() << "\r\n";
			ss << "\r\n";
			ss << body;
			return ss.str();
		}

		std::string json_error(int code, const std::string& message) {
			std::stringstream json;
			json << "{\"error\": \"" << message << "\", \"status\": \"error\"}";
			return json_response(code, json.str());
		}

		double safe_ops_per_sec(double operations, long long duration_ns) {
			if(duration_ns <= 0) {
				return 0.0;
			}
			return operations / (static_cast<double>(duration_ns) / 1e9);
		}

		std::string process_request(const std::string& request) {
			// Extract URL from request
			std::string url;
			std::istringstream iss(request);
			std::string method;
			iss >> method >> url;

			auto params= parse_query(url);

			// GET /health
			if(url.find("/health") == 0) {
				return json_response(200, R"({"status": "ok", "server": "Boost.Asio C++ Interview Demo"})");
			}

			// GET /benchmark/task1
			if(url.find("/benchmark/task1") == 0) {
				size_t iterations= get_param_size(params, "iterations", 1000000, 100000000);
				size_t threads= get_param_size(params, "threads", 1, 64);

				if(iterations == 0) {
					return json_error(400, "Invalid 'iterations' parameter: must be positive integer <= 100000000");
				}
				if(threads == 0) {
					return json_error(400, "Invalid 'threads' parameter: must be positive integer <= 64");
				}

				// Call real C++ benchmark function
				long long duration= benchmark_weak_ptr_lock(static_cast<int>(iterations), static_cast<int>(threads));
				double ops_per_sec= safe_ops_per_sec(static_cast<double>(iterations), duration);

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 1,\n";
				json << "  \"task_name\": \"weak_ptr::lock()\",\n";
				json << "  \"method\": \"CustomWeakPtr::lock()\",\n";
				json << "  \"iterations\": " << iterations << ",\n";
				json << "  \"threads\": " << threads << ",\n";
				json << "  \"execution_time_ns\": " << duration << ",\n";
				json << "  \"operations_per_second\": " << std::fixed << ops_per_sec << ",\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(200, json.str());
			}

			// GET /benchmark/task2
			if(url.find("/benchmark/task2") == 0) {
				size_t size= get_param_size(params, "size", 100000, 10000000);
				size_t iterations= get_param_size(params, "iterations", 100, 10000);
				size_t threads= get_param_size(params, "threads", 1, 64);

				if(size == 0) {
					return json_error(400, "Invalid 'size' parameter: must be positive integer <= 10000000");
				}
				if(iterations == 0) {
					return json_error(400, "Invalid 'iterations' parameter: must be positive integer <= 10000");
				}
				if(threads == 0) {
					return json_error(400, "Invalid 'threads' parameter: must be positive integer <= 64");
				}

				// Call real benchmarks for each method
				std::vector<std::pair<std::string, long long>> methods= {
					{"naive_erase", benchmark_vector_erase(erase_every_second_naive<int>, "naive", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"remove_if_erase", benchmark_vector_erase(erase_every_second_remove_if<int>, "remove_if", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"iterators_erase", benchmark_vector_erase(erase_every_second_iterators<int>, "iterators", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"copy_erase", benchmark_vector_erase(erase_every_second_copy<int>, "copy", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"partition_erase", benchmark_vector_erase(erase_every_second_partition<int>, "partition", size, static_cast<int>(iterations), static_cast<int>(threads))}};

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 2,\n";
				json << "  \"task_name\": \"Vector erase\",\n";
				json << "  \"vector_size\": " << size << ",\n";
				json << "  \"iterations\": " << iterations << ",\n";
				json << "  \"threads\": " << threads << ",\n";
				json << "  \"methods\": [\n";

				for(size_t i= 0; i < methods.size(); ++i) {
					json << "    {\"name\": \"" << methods[i].first << "\", ";
					json << "\"time_ns\": " << methods[i].second << ", ";
					json << "\"ops_per_sec\": " << std::fixed
						 << safe_ops_per_sec(static_cast<double>(iterations), methods[i].second) << "}";
					if(i < methods.size() - 1)
						json << ",";
					json << "\n";
				}

				json << "  ],\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(200, json.str());
			}

			// GET /benchmark/task3
			if(url.find("/benchmark/task3") == 0) {
				size_t size= get_param_size(params, "size", 100000, 10000000);
				size_t lookups= get_param_size(params, "lookups", 1000000, 100000000);

				if(size == 0) {
					return json_error(400, "Invalid 'size' parameter: must be positive integer <= 10000000");
				}
				if(lookups == 0) {
					return json_error(400, "Invalid 'lookups' parameter: must be positive integer <= 100000000");
				}

				// Call real benchmarks for each container
				BenchmarkResult map_result= benchmark_map(static_cast<int>(size), static_cast<int>(lookups));
				BenchmarkResult umap_result= benchmark_unordered_map(static_cast<int>(size), static_cast<int>(lookups));
				BenchmarkResult vec_result= benchmark_vector(static_cast<int>(size), static_cast<int>(lookups));

				// Helper to get complexity string
				auto get_complexity= [](const std::string& name) -> std::string {
					if(name == "std::map")
						return "O(log n)";
					if(name == "std::unordered_map")
						return "O(1) average";
					return "O(n)";
				};

				// Determine recommendation based on actual results (find fastest lookup among all containers)
				std::string recommendation;
				if(umap_result.lookup_time_ns <= map_result.lookup_time_ns &&
					umap_result.lookup_time_ns <= vec_result.lookup_time_ns) {
					recommendation= "std::unordered_map for best lookup performance";
				} else if(map_result.lookup_time_ns <= umap_result.lookup_time_ns &&
					map_result.lookup_time_ns <= vec_result.lookup_time_ns) {
					recommendation= "std::map for this dataset size";
				} else {
					recommendation= "std::vector<pair> for this dataset size";
				}

				std::vector<BenchmarkResult> results= {map_result, umap_result, vec_result};

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 3,\n";
				json << "  \"task_name\": \"Mapping benchmark\",\n";
				json << "  \"elements\": " << size << ",\n";
				json << "  \"lookups\": " << lookups << ",\n";
				json << "  \"containers\": [\n";

				for(size_t i= 0; i < results.size(); ++i) {
					const auto& result= results[i];
					json << "    {\"name\": \"" << result.container_name << "\", ";
					json << "\"insert_ns\": " << result.insert_time_ns << ", ";
					json << "\"lookup_ns\": " << result.lookup_time_ns << ", ";
					json << "\"erase_ns\": " << result.erase_time_ns << ", ";
					json << "\"memory_bytes\": " << result.memory_usage_bytes << ", ";
					json << "\"complexity\": \"" << get_complexity(result.container_name) << "\", ";
					json << "\"ops_per_sec\": " << std::fixed
						 << safe_ops_per_sec(static_cast<double>(lookups), result.lookup_time_ns) << "}";
					if(i < results.size() - 1)
						json << ",";
					json << "\n";
				}

				json << "  ],\n";
				json << "  \"recommendation\": \"" << recommendation << "\",\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(200, json.str());
			}

			// GET /results
			if(url.find("/results") == 0) {
				// TODO: Query PostgreSQL for results here
				std::stringstream json;
				json << "{\n";
				json << "  \"results\": [],\n";
				json << "  \"total\": 0,\n";
				json << "  \"note\": \"Connect to PostgreSQL to see real results\"\n";
				json << "}";

				return json_response(200, json.str());
			}

			// 404 Not Found
			return json_response(404, R"({"error": "Not found", "available_endpoints": ["/health", "/benchmark/task1", "/benchmark/task2", "/benchmark/task3", "/results"]})");
		}

		tcp::socket socket_;
		std::array<char, 4096> data_;
	};

	tcp::acceptor acceptor_;
};

void print_usage(const char* program_name) {
	std::cout << "Usage: " << program_name << " [port]\n\n";
	std::cout << "Boost.Asio HTTP server for C++ Interview Demo benchmarks\n\n";
	std::cout << "Endpoints:\n";
	std::cout << "  GET /health                      - Health check\n";
	std::cout << "  GET /benchmark/task1             - Benchmark weak_ptr::lock()\n";
	std::cout << "      ?iterations=N                - Number of iterations (default: 1000000)\n";
	std::cout << "      ?threads=N                   - Number of threads (default: 1)\n";
	std::cout << "  GET /benchmark/task2             - Benchmark vector erase\n";
	std::cout << "      ?size=N                      - Vector size (default: 100000)\n";
	std::cout << "      ?iterations=N                - Number of iterations (default: 100)\n";
	std::cout << "  GET /benchmark/task3             - Benchmark int→string mapping\n";
	std::cout << "      ?size=N                      - Number of elements (default: 100000)\n";
	std::cout << "      ?lookups=N                   - Number of lookups (default: 1000000)\n";
	std::cout << "  GET /results                     - Get results from DB\n";
	std::cout << "\nExamples:\n";
	std::cout << "  curl http://localhost:8080/health\n";
	std::cout << "  curl http://localhost:8080/benchmark/task1\n";
	std::cout << "  curl \"http://localhost:8080/benchmark/task2?size=50000\"\n";
}

int main(int argc, char* argv[]) {
	try {
		short port= 8080;

		if(argc > 1) {
			std::string arg= argv[1];
			if(arg == "-h" || arg == "--help") {
				print_usage(argv[0]);
				return 0;
			}
			port= static_cast<short>(std::atoi(argv[1]));
		}

		boost::asio::io_context io_context;
		HttpServer server(io_context, port);

		std::cout << "╔══════════════════════════════════════════════════════════╗\n";
		std::cout << "║   C++ Interview Demo - Boost.Asio HTTP Server            ║\n";
		std::cout << "╠══════════════════════════════════════════════════════════╣\n";
		std::cout << "║   Port: " << port << "                                              ║\n";
		std::cout << "╠══════════════════════════════════════════════════════════╣\n";
		std::cout << "║   Endpoints:                                              ║\n";
		std::cout << "║     GET /health              - Server status              ║\n";
		std::cout << "║     GET /benchmark/task1     - weak_ptr::lock()          ║\n";
		std::cout << "║     GET /benchmark/task2     - Vector erase              ║\n";
		std::cout << "║     GET /benchmark/task3     - Mapping benchmark         ║\n";
		std::cout << "║     GET /results             - Results from DB           ║\n";
		std::cout << "╠══════════════════════════════════════════════════════════╣\n";
		std::cout << "║   Examples:                                              ║\n";
		std::cout << "║     curl http://localhost:" << port << "/health                   ║\n";
		std::cout << "║     curl http://localhost:" << port << "/benchmark/task1          ║\n";
		std::cout << "╚══════════════════════════════════════════════════════════╝\n";
		std::cout << "\nServer started. Press Ctrl+C to stop.\n\n";

		io_context.run();
	} catch(std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
