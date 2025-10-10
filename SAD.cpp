#include "SAD.h"
#include <cmath>

double calculate_sad(const TimeSeries& series, const TimeSeries& query, long long start_index) {
    double current_sad = 0.0;
    for (size_t j = 0; j < query.values.size(); ++j) {
        current_sad += std::abs(series.values[start_index + j] - query.values[j]);
    }
    return current_sad;
}