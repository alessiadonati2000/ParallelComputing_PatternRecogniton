#include "ParallelRecognition.h"
#include <omp.h>

MatchResult find_best_match_parallel_omp(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        return {};
    }

    MatchResult global_result;
    long long search_range = series.values.size() - query.values.size();

#pragma omp parallel for
    for (long long i = 0; i <= search_range; ++i) {
        // Chiama la funzione dedicata per calcolare il SAD
        double current_sad = calculate_sad(series, query, i);

#pragma omp critical
        {
            if (current_sad < global_result.min_sad) {
                global_result.min_sad = current_sad;
                global_result.index = i;
            }
        }
    }
    return global_result;
}