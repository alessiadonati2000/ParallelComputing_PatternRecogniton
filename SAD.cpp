#include "SAD.h"
#include <cmath>

double calculate_sad(const std::vector<float>& series, const std::vector<float>& query, long long start_index) {
    double current_sad = 0.0;
    for (size_t j = 0; j < query.size(); ++j) {
        current_sad += std::abs(series[start_index + j] - query[j]);
    }
    return current_sad;
}