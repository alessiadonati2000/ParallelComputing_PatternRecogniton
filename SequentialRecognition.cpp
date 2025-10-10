#include "SequentialRecognition.h"

MatchResult find_best_match_sequential(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        return {};
    }

    MatchResult result;
    long long search_range = series.values.size() - query.values.size();

    for (long long i = 0; i <= search_range; ++i) {
        // Chiama la funzione dedicata per calcolare il SAD
        double current_sad = calculate_sad(series, query, i);

        if (current_sad < result.min_sad) {
            result.min_sad = current_sad;
            result.index = i;
        }
    }
    return result;
}