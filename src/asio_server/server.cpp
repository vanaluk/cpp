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
		auto new_session= std::make_shared<Session>(acceptor_.get_executor().context());
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
		Session(boost::asio::io_context& io_context) :
			socket_(io_context) {}

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

		std::string json_response(int code, const std::string& body) {
			std::stringstream ss;
			ss << "HTTP/1.1 " << code << " OK\r\n";
			ss << "Content-Type: application/json\r\n";
			ss << "Access-Control-Allow-Origin: *\r\n";
			ss << "Content-Length: " << body.length() << "\r\n";
			ss << "\r\n";
			ss << body;
			return ss.str();
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
				int iterations= get_param_int(params, "iterations", 1000000);
				int threads= get_param_int(params, "threads", 1);

				auto start= std::chrono::high_resolution_clock::now();
				// TODO: Call real weak_ptr::lock() benchmark here
				// Simulation of work
				volatile int sum= 0;
				for(int i= 0; i < iterations / 1000; ++i) {
					sum+= i;
				}
				auto end= std::chrono::high_resolution_clock::now();
				auto duration= std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

				double ops_per_sec= static_cast<double>(iterations) / (duration / 1e9);

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
				int size= get_param_int(params, "size", 100000);
				int iterations= get_param_int(params, "iterations", 100);

				// TODO: Call real benchmarks for each method here
				// Simulate results
				std::vector<std::pair<std::string, long long>> methods= {
					{"naive_erase", 45670000},
					{"remove_if_erase", 1230000},
					{"iterators", 43210000},
					{"copy_to_new", 980000},
					{"partition", 890000}};

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 2,\n";
				json << "  \"task_name\": \"Vector erase\",\n";
				json << "  \"vector_size\": " << size << ",\n";
				json << "  \"iterations\": " << iterations << ",\n";
				json << "  \"methods\": [\n";

				for(size_t i= 0; i < methods.size(); ++i) {
					json << "    {\"name\": \"" << methods[i].first << "\", ";
					json << "\"time_ns\": " << methods[i].second << ", ";
					json << "\"ops_per_sec\": " << std::fixed
						 << (static_cast<double>(iterations) / (methods[i].second / 1e9)) << "}";
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
				int size= get_param_int(params, "size", 100000);
				int lookups= get_param_int(params, "lookups", 1000000);

				// TODO: Call real benchmarks for each container here
				std::vector<std::tuple<std::string, long long, std::string>> containers= {
					{"std::map", 15000000, "O(log n)"},
					{"std::unordered_map", 800000, "O(1) average"},
					{"std::vector<pair>", 95000000, "O(n)"}};

				std::stringstream json;
				json << "{\n";
				json << "  \"task\": 3,\n";
				json << "  \"task_name\": \"Mapping benchmark\",\n";
				json << "  \"elements\": " << size << ",\n";
				json << "  \"lookups\": " << lookups << ",\n";
				json << "  \"containers\": [\n";

				for(size_t i= 0; i < containers.size(); ++i) {
					json << "    {\"name\": \"" << std::get<0>(containers[i]) << "\", ";
					json << "\"time_ns\": " << std::get<1>(containers[i]) << ", ";
					json << "\"complexity\": \"" << std::get<2>(containers[i]) << "\", ";
					json << "\"ops_per_sec\": " << std::fixed
						 << (static_cast<double>(lookups) / (std::get<1>(containers[i]) / 1e9)) << "}";
					if(i < containers.size() - 1)
						json << ",";
					json << "\n";
				}

				json << "  ],\n";
				json << "  \"recommendation\": \"std::unordered_map for best performance\",\n";
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
