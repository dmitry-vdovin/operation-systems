#pragma once
#include <cstddef>
#include <cmath>
#include <stdexcept>

// Population standard deviation (деление на N).
inline double stddev_population_welford(const double* data, size_t n) {
    if (!data || n == 0) throw std::invalid_argument("empty data");
    long double mean = 0.0L, M2 = 0.0L;
    for (size_t k = 0; k < n; ++k) {
        long double x = data[k];
        long double delta = x - mean;
        mean += delta / (k + 1);
        long double delta2 = x - mean;
        M2 += delta * delta2;
    }
    long double var = M2 / n; // population variance
    return std::sqrt((double)var);
}