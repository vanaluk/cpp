#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <map>
#include "examples/weak_ptr/custom_weak_ptr.hpp"
#include "examples/vector_erase/vector_erase.hpp"
#include "examples/container_lookup/container_benchmark.hpp"
#include "db/postgres_client.hpp"

using boost::asio::ip::tcp;

// HTTP status codes
namespace http_status {
constexpr int kOk= 200;
constexpr int kBadRequest= 400;
constexpr int kNotFound= 404;
} // namespace http_status

// Server configuration constants
namespace server_config {
constexpr short kDefaultPort= 8080;
constexpr size_t kReadBufferSize= 4096;
constexpr size_t kMaxThreads= 64;
constexpr double kNanosecondsPerSecond= 1'000'000'000.0;
} // namespace server_config

// Benchmark parameter limits
namespace benchmark_limits {
constexpr size_t kDefaultTask1Iterations= 1000000;
constexpr size_t kMaxTask1Iterations= 100000000;

constexpr size_t kDefaultVectorSize= 100000;
constexpr size_t kMaxVectorSize= 10000000;
constexpr size_t kDefaultVectorIterations= 100;
constexpr size_t kMaxVectorIterations= 10000;

constexpr size_t kDefaultMappingSize= 100000;
constexpr size_t kMaxMappingSize= 10000000;
constexpr size_t kDefaultLookups= 1000000;
constexpr size_t kMaxLookups= 100000000;

constexpr int kDefaultResultsLimit= 100;
} // namespace benchmark_limits

/**
 * C++ Benchmark Kit - REST API Server (Boost.Asio)
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
		explicit Session(const boost::asio::any_io_executor& executor) :
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
		static std::map<std::string, std::string> parse_query(const std::string& url) {
			std::map<std::string, std::string> params;
			size_t pos= url.find('?');
			if(pos == std::string::npos) {
				return params;
			}

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

		static int get_param_int(const std::map<std::string, std::string>& params,
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
		static size_t get_param_size(const std::map<std::string, std::string>& params,
			const std::string& key, size_t default_value, size_t max_value) { // NOLINT(bugprone-easily-swappable-parameters)
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

		static std::string json_response(int code, const std::string& body) {
			std::stringstream ss;
			std::string status_text;
			if(code == http_status::kOk) {
				status_text= "OK";
			} else if(code == http_status::kBadRequest) {
				status_text= "Bad Request";
			} else if(code == http_status::kNotFound) {
				status_text= "Not Found";
			} else {
				status_text= "Error";
			}
			ss << "HTTP/1.1 " << code << " " << status_text << "\r\n";
			ss << "Content-Type: application/json\r\n";
			ss << "Access-Control-Allow-Origin: *\r\n";
			ss << "Content-Length: " << body.length() << "\r\n";
			ss << "\r\n";
			ss << body;
			return ss.str();
		}

		static std::string json_error(int code, const std::string& message) {
			std::stringstream json;
			json << "{\"error\": \"" << message << "\", \"status\": \"error\"}";
			return json_response(code, json.str());
		}

		static double safe_ops_per_sec(double operations, long long duration_ns) {
			if(duration_ns <= 0) {
				return 0.0;
			}
			return operations / (static_cast<double>(duration_ns) / server_config::kNanosecondsPerSecond);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static, readability-function-cognitive-complexity)
		std::string process_request(const std::string& request) {
			// Extract URL from request
			std::string url;
			std::istringstream iss(request);
			std::string method;
			iss >> method >> url;

			auto params= parse_query(url);

			// GET /health
			if(url.find("/health") == 0) {
				return json_response(http_status::kOk, R"({"status": "ok", "server": "C++ Benchmark Kit REST API"})");
			}

			// GET /benchmark/task1
			if(url.find("/benchmark/task1") == 0) {
				size_t iterations= get_param_size(params, "iterations",
					benchmark_limits::kDefaultTask1Iterations, benchmark_limits::kMaxTask1Iterations);
				size_t threads= get_param_size(params, "threads", 1, server_config::kMaxThreads);

				if(iterations == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'iterations' parameter: must be positive integer <= 100000000");
				}
				if(threads == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'threads' parameter: must be positive integer <= 64");
				}

				// Call real C++ benchmark function
				long long duration= benchmark_weak_ptr_lock(static_cast<int>(iterations), static_cast<int>(threads));
				double ops_per_sec= safe_ops_per_sec(static_cast<double>(iterations), duration);

				// Save to database
				bool saved= get_db_client().save_result(
					1, "weak_ptr::lock()", "CustomWeakPtr::lock()",
					duration, ops_per_sec, static_cast<int>(threads),
					get_build_type());

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 1,\n";
				json << "  \"task_name\": \"weak_ptr::lock()\",\n";
				json << "  \"method\": \"CustomWeakPtr::lock()\",\n";
				json << "  \"iterations\": " << iterations << ",\n";
				json << "  \"threads\": " << threads << ",\n";
				json << "  \"execution_time_ns\": " << duration << ",\n";
				json << "  \"operations_per_second\": " << std::fixed << ops_per_sec << ",\n";
				json << "  \"saved_to_db\": " << (saved ? "true" : "false") << ",\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(http_status::kOk, json.str());
			}

			// GET /benchmark/task2
			if(url.find("/benchmark/task2") == 0) {
				size_t size= get_param_size(params, "size",
					benchmark_limits::kDefaultVectorSize, benchmark_limits::kMaxVectorSize);
				size_t iterations= get_param_size(params, "iterations",
					benchmark_limits::kDefaultVectorIterations, benchmark_limits::kMaxVectorIterations);
				size_t threads= get_param_size(params, "threads", 1, server_config::kMaxThreads);

				if(size == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'size' parameter: must be positive integer <= 10000000");
				}
				if(iterations == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'iterations' parameter: must be positive integer <= 10000");
				}
				if(threads == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'threads' parameter: must be positive integer <= 64");
				}

				// Call real benchmarks for each method
				std::vector<std::pair<std::string, long long>> methods= {
					{"naive_erase", benchmark_vector_erase(erase_every_second_naive<int>, "naive", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"remove_if_erase", benchmark_vector_erase(erase_every_second_remove_if<int>, "remove_if", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"iterators_erase", benchmark_vector_erase(erase_every_second_iterators<int>, "iterators", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"copy_erase", benchmark_vector_erase(erase_every_second_copy<int>, "copy", size, static_cast<int>(iterations), static_cast<int>(threads))},
					{"partition_erase", benchmark_vector_erase(erase_every_second_partition<int>, "partition", size, static_cast<int>(iterations), static_cast<int>(threads))}};

				// Save each method result to database
				std::string build_type= get_build_type();
				int saved_count= 0;
				for(const auto& m: methods) {
					double ops= safe_ops_per_sec(static_cast<double>(iterations), m.second);
					if(get_db_client().save_result(2, "Vector erase", m.first, m.second, ops,
						   static_cast<int>(threads), build_type)) {
						++saved_count;
					}
				}

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
					if(i < methods.size() - 1) {
						json << ",";
					}
					json << "\n";
				}

				json << "  ],\n";
				json << "  \"saved_to_db\": " << saved_count << ",\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(http_status::kOk, json.str());
			}

			// GET /benchmark/task3
			if(url.find("/benchmark/task3") == 0) {
				size_t size= get_param_size(params, "size",
					benchmark_limits::kDefaultMappingSize, benchmark_limits::kMaxMappingSize);
				size_t lookups= get_param_size(params, "lookups",
					benchmark_limits::kDefaultLookups, benchmark_limits::kMaxLookups);

				if(size == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'size' parameter: must be positive integer <= 10000000");
				}
				if(lookups == 0) {
					return json_error(http_status::kBadRequest, "Invalid 'lookups' parameter: must be positive integer <= 100000000");
				}

				// Call real benchmarks for each container
				BenchmarkResult map_result= benchmark_map(static_cast<int>(size), static_cast<int>(lookups));
				BenchmarkResult umap_result= benchmark_unordered_map(static_cast<int>(size), static_cast<int>(lookups));
				BenchmarkResult vec_result= benchmark_vector(static_cast<int>(size), static_cast<int>(lookups));

				// Save each container result to database
				std::string build_type= get_build_type();
				int saved_count= 0;
				auto save_container_result= [&](const BenchmarkResult& r) {
					double ops= safe_ops_per_sec(static_cast<double>(lookups), r.lookup_time_ns);
					if(get_db_client().save_result(3, "Mapping benchmark", r.container_name,
						   r.lookup_time_ns, ops, 1, build_type)) {
						++saved_count;
					}
				};
				save_container_result(map_result);
				save_container_result(umap_result);
				save_container_result(vec_result);

				// Helper to get complexity string
				auto get_complexity= [](const std::string& name) -> std::string {
					if(name == "std::map") {
						return "O(log n)";
					}
					if(name == "std::unordered_map") {
						return "O(1) average";
					}
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
					if(i < results.size() - 1) {
						json << ",";
					}
					json << "\n";
				}

				json << "  ],\n";
				json << "  \"recommendation\": \"" << recommendation << "\",\n";
				json << "  \"saved_to_db\": " << saved_count << ",\n";
				json << "  \"status\": \"success\"\n";
				json << "}";

				return json_response(http_status::kOk, json.str());
			}

			// GET /results
			if(url.find("/results") == 0) {
				int limit= get_param_int(params, "limit", benchmark_limits::kDefaultResultsLimit);
				int task= get_param_int(params, "task", 0);

				std::string results_json= get_db_client().get_results_json(limit, task);
				return json_response(http_status::kOk, results_json);
			}

			// 404 Not Found
			return json_response(http_status::kNotFound, R"({"error": "Not found", "available_endpoints": ["/health", "/benchmark/task1", "/benchmark/task2", "/benchmark/task3", "/results"]})");
		}

		tcp::socket socket_;
		std::array<char, server_config::kReadBufferSize> data_;
	};

	tcp::acceptor acceptor_;
};

void print_usage(const char* program_name) {
	std::cout << "Usage: " << program_name << " [port]\n\n";
	std::cout << "C++ Benchmark Kit - REST API Server\n\n";
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
		short port= server_config::kDefaultPort;

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
		std::cout << "║   C++ Benchmark Kit - REST API Server                    ║\n";
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
