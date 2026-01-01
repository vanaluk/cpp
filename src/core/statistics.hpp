#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

/**
 * C++ Benchmark Kit - Statistical Analysis
 *
 * Utility functions for analyzing benchmark results.
 */

namespace benchmark_kit {
namespace stats {

/**
 * Calculate mean of a vector of samples
 */
template<typename T>
double mean(const std::vector<T>& samples) {
	if (samples.empty()) return 0.0;
	double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
	return sum / static_cast<double>(samples.size());
}

/**
 * Calculate standard deviation
 */
template<typename T>
double stddev(const std::vector<T>& samples) {
	if (samples.size() < 2) return 0.0;

	double m = mean(samples);
	double sq_sum = 0.0;
	for (const auto& s : samples) {
		double diff = static_cast<double>(s) - m;
		sq_sum += diff * diff;
	}
	return std::sqrt(sq_sum / static_cast<double>(samples.size() - 1));
}

/**
 * Calculate variance
 */
template<typename T>
double variance(const std::vector<T>& samples) {
	double sd = stddev(samples);
	return sd * sd;
}

/**
 * Calculate percentile (0.0 to 1.0)
 * Note: samples must be sorted
 */
template<typename T>
T percentile_sorted(const std::vector<T>& sorted_samples, double p) {
	if (sorted_samples.empty()) return T{};
	if (p <= 0.0) return sorted_samples.front();
	if (p >= 1.0) return sorted_samples.back();

	double idx = p * static_cast<double>(sorted_samples.size() - 1);
	size_t lower = static_cast<size_t>(std::floor(idx));
	size_t upper = static_cast<size_t>(std::ceil(idx));

	if (lower == upper) {
		return sorted_samples[lower];
	}

	// Linear interpolation
	double frac = idx - static_cast<double>(lower);
	return static_cast<T>(
		static_cast<double>(sorted_samples[lower]) * (1.0 - frac) +
		static_cast<double>(sorted_samples[upper]) * frac
	);
}

/**
 * Calculate percentile (sorts a copy)
 */
template<typename T>
T percentile(std::vector<T> samples, double p) {
	if (samples.empty()) return T{};
	std::sort(samples.begin(), samples.end());
	return percentile_sorted(samples, p);
}

/**
 * Calculate median (P50)
 */
template<typename T>
T median(std::vector<T> samples) {
	return percentile(std::move(samples), 0.5);
}

/**
 * Calculate min
 */
template<typename T>
T min(const std::vector<T>& samples) {
	if (samples.empty()) return T{};
	return *std::min_element(samples.begin(), samples.end());
}

/**
 * Calculate max
 */
template<typename T>
T max(const std::vector<T>& samples) {
	if (samples.empty()) return T{};
	return *std::max_element(samples.begin(), samples.end());
}

/**
 * Remove outliers using IQR method
 * Returns a new vector without outliers
 */
template<typename T>
std::vector<T> remove_outliers(std::vector<T> samples, double iqr_multiplier = 1.5) {
	if (samples.size() < 4) return samples;

	std::sort(samples.begin(), samples.end());

	T q1 = percentile_sorted(samples, 0.25);
	T q3 = percentile_sorted(samples, 0.75);
	double iqr = static_cast<double>(q3) - static_cast<double>(q1);

	double lower_bound = static_cast<double>(q1) - iqr_multiplier * iqr;
	double upper_bound = static_cast<double>(q3) + iqr_multiplier * iqr;

	std::vector<T> result;
	result.reserve(samples.size());

	for (const auto& s : samples) {
		double val = static_cast<double>(s);
		if (val >= lower_bound && val <= upper_bound) {
			result.push_back(s);
		}
	}

	return result;
}

/**
 * Summary statistics structure
 */
struct Summary {
	double mean = 0.0;
	double stddev = 0.0;
	double variance = 0.0;
	double min = 0.0;
	double max = 0.0;
	double median = 0.0;
	double p5 = 0.0;
	double p25 = 0.0;
	double p75 = 0.0;
	double p95 = 0.0;
	double p99 = 0.0;
	size_t count = 0;

	std::string to_string() const {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2);
		oss << "Count: " << count << "\n";
		oss << "Mean: " << mean << " (±" << stddev << ")\n";
		oss << "Min: " << min << ", Max: " << max << "\n";
		oss << "Percentiles: P5=" << p5 << ", P25=" << p25
			<< ", P50=" << median << ", P75=" << p75
			<< ", P95=" << p95 << ", P99=" << p99;
		return oss.str();
	}
};

/**
 * Calculate full summary statistics
 */
template<typename T>
Summary summarize(std::vector<T> samples) {
	Summary s;
	if (samples.empty()) return s;

	s.count = samples.size();
	s.mean = mean(samples);
	s.stddev = stddev(samples);
	s.variance = s.stddev * s.stddev;

	std::sort(samples.begin(), samples.end());
	s.min = static_cast<double>(samples.front());
	s.max = static_cast<double>(samples.back());

	s.p5 = static_cast<double>(percentile_sorted(samples, 0.05));
	s.p25 = static_cast<double>(percentile_sorted(samples, 0.25));
	s.median = static_cast<double>(percentile_sorted(samples, 0.50));
	s.p75 = static_cast<double>(percentile_sorted(samples, 0.75));
	s.p95 = static_cast<double>(percentile_sorted(samples, 0.95));
	s.p99 = static_cast<double>(percentile_sorted(samples, 0.99));

	return s;
}

} // namespace stats
} // namespace benchmark_kit

