#pragma once

#include <libpq-fe.h>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <fstream>

/**
 * PostgreSQL client for saving and retrieving benchmark results
 * Uses libpq (PostgreSQL C library)
 */
class PostgresClient {
public:
	PostgresClient() :
		conn_(nullptr) {}

	~PostgresClient() {
		disconnect();
	}

	// Non-copyable
	PostgresClient(const PostgresClient&)= delete;
	PostgresClient& operator=(const PostgresClient&)= delete;

	// Connect to PostgreSQL using environment variables
	bool connect() {
		if(conn_ != nullptr) {
			// Check if existing connection is still healthy
			if(PQstatus(conn_) == CONNECTION_OK) {
				return true; // Already connected and healthy
			}
			// Connection exists but is unhealthy (e.g., network timeout, DB restart)
			// Clean up before reconnecting
			disconnect();
		}

		std::string host= get_env("DB_HOST", "postgres");
		std::string port= get_env("DB_PORT", "5432");
		std::string dbname= get_env("DB_NAME", "cpp_interview_db");
		std::string user= get_env("DB_USER", "cpp_interview");
		std::string password= get_env("DB_PASSWORD", "cpp_interview_pass");

		std::string conninfo= "host=" + host +
			" port=" + port +
			" dbname=" + dbname +
			" user=" + user +
			" password=" + password +
			" connect_timeout=5";

		conn_= PQconnectdb(conninfo.c_str());

		if(PQstatus(conn_) != CONNECTION_OK) {
			last_error_= PQerrorMessage(conn_);
			PQfinish(conn_);
			conn_= nullptr;
			return false;
		}

		return true;
	}

	void disconnect() {
		if(conn_ != nullptr) {
			PQfinish(conn_);
			conn_= nullptr;
		}
	}

	bool is_connected() const {
		return conn_ != nullptr && PQstatus(conn_) == CONNECTION_OK;
	}

	std::string get_last_error() const {
		return last_error_;
	}

	// Save benchmark result to database
	bool save_result(int task_number, const std::string& task_name,
		const std::string& method_name, long long execution_time_ns,
		double operations_per_second, int thread_count= 1,
		const std::string& build_type= "Release",
		const std::string& notes= "") {
		if(!ensure_connected()) {
			return false;
		}

		// Prepare SQL statement
		std::stringstream sql;
		sql << "INSERT INTO benchmark_results "
			<< "(task_number, task_name, method_name, execution_time_ns, "
			<< "operations_per_second, thread_count, build_type, notes) "
			<< "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)";

		// Convert parameters to strings
		std::string task_num_str= std::to_string(task_number);
		std::string exec_time_str= std::to_string(execution_time_ns);
		std::ostringstream ops_stream;
		ops_stream << std::fixed << std::setprecision(2) << operations_per_second;
		std::string ops_str= ops_stream.str();
		std::string thread_str= std::to_string(thread_count);

		const char* params[]= {
			task_num_str.c_str(),
			task_name.c_str(),
			method_name.c_str(),
			exec_time_str.c_str(),
			ops_str.c_str(),
			thread_str.c_str(),
			build_type.c_str(),
			notes.empty() ? nullptr : notes.c_str()};

		PGresult* res= PQexecParams(conn_, sql.str().c_str(),
			8, nullptr, params, nullptr, nullptr, 0);

		if(PQresultStatus(res) != PGRES_COMMAND_OK) {
			last_error_= PQerrorMessage(conn_);
			PQclear(res);
			return false;
		}

		PQclear(res);
		return true;
	}

	// Get results from database as JSON string
	std::string get_results_json(int limit= 100, int task_filter= 0) {
		if(!ensure_connected()) {
			return build_error_json("Database connection failed: " + last_error_);
		}

		std::stringstream sql;
		sql << "SELECT id, timestamp, task_number, task_name, method_name, "
			<< "execution_time_ns, operations_per_second, thread_count, build_type, notes "
			<< "FROM benchmark_results ";

		if(task_filter > 0) {
			sql << "WHERE task_number = " << task_filter << " ";
		}

		sql << "ORDER BY timestamp DESC LIMIT " << limit;

		PGresult* res= PQexec(conn_, sql.str().c_str());

		if(PQresultStatus(res) != PGRES_TUPLES_OK) {
			last_error_= PQerrorMessage(conn_);
			PQclear(res);
			return build_error_json("Query failed: " + last_error_);
		}

		int rows= PQntuples(res);

		std::stringstream json;
		json << "{\n  \"results\": [\n";

		for(int i= 0; i < rows; ++i) {
			json << "    {\n";
			json << "      \"id\": " << PQgetvalue(res, i, 0) << ",\n";
			json << "      \"timestamp\": \"" << PQgetvalue(res, i, 1) << "\",\n";
			json << "      \"task_number\": " << PQgetvalue(res, i, 2) << ",\n";
			json << "      \"task_name\": \"" << escape_json(PQgetvalue(res, i, 3)) << "\",\n";
			json << "      \"method_name\": \"" << escape_json(PQgetvalue(res, i, 4)) << "\",\n";
			json << "      \"execution_time_ns\": " << PQgetvalue(res, i, 5) << ",\n";
			// Handle nullable operations_per_second field
			json << "      \"operations_per_second\": ";
			if(PQgetisnull(res, i, 6) != 0) {
				json << "null";
			} else {
				json << PQgetvalue(res, i, 6);
			}
			json << ",\n";
			json << "      \"thread_count\": " << PQgetvalue(res, i, 7) << ",\n";
			json << "      \"build_type\": \"" << escape_json(PQgetvalue(res, i, 8)) << "\"";

			// Handle nullable notes field
			if(PQgetisnull(res, i, 9) == 0) {
				json << ",\n      \"notes\": \"" << escape_json(PQgetvalue(res, i, 9)) << "\"";
			}
			json << "\n    }";

			if(i < rows - 1)
				json << ",";
			json << "\n";
		}

		json << "  ],\n";
		json << "  \"total\": " << rows << "\n";
		json << "}";

		PQclear(res);
		return json.str();
	}

private:
	PGconn* conn_;
	std::string last_error_;

	bool ensure_connected() {
		if(is_connected()) {
			return true;
		}
		return connect();
	}

	static std::string get_env(const std::string& name, const std::string& default_value) {
		const char* value= std::getenv(name.c_str());
		return value ? value : default_value;
	}

	static std::string escape_json(const char* str) {
		if(str == nullptr)
			return "";
		std::string result;
		while(*str) {
			switch(*str) {
			case '"':
				result+= "\\\"";
				break;
			case '\\':
				result+= "\\\\";
				break;
			case '\n':
				result+= "\\n";
				break;
			case '\r':
				result+= "\\r";
				break;
			case '\t':
				result+= "\\t";
				break;
			default:
				result+= *str;
				break;
			}
			++str;
		}
		return result;
	}

	static std::string build_error_json(const std::string& error) {
		return "{\"results\": [], \"total\": 0, \"error\": \"" + escape_json(error.c_str()) + "\"}";
	}
};

// Global PostgreSQL client instance for the server
inline PostgresClient& get_db_client() {
	static PostgresClient client;
	return client;
}

// Helper to get build type from environment or file
inline std::string get_build_type() {
	const char* env_type= std::getenv("BUILD_TYPE");
	if(env_type)
		return env_type;

	// Try to read from .build_info file
	std::ifstream file("/app/.build_info");
	if(file.is_open()) {
		std::string line;
		while(std::getline(file, line)) {
			if(line.find("BUILD_TYPE=") == 0) {
				return line.substr(11);
			}
		}
	}
	return "Release";
}
