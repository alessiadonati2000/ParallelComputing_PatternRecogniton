#include "SequentialRecognition.h"
#include <stdexcept>
#include "SAD.h"

MatchResult sequential_recognition(const std::vector<float>& serie, const std::vector<float>& query) {
    if (query.size() > serie.size() || query.empty()) {
        throw std::runtime_error("Query più grande della serie oppure vuota.");
    }

    MatchResult result;
    long long search_range = serie.size() - query.size();

    for (long long i = 0; i <= search_range; ++i) {
        double current_sad = calculate_sad(serie, query, i);

        if (current_sad < result.min_sad) {
            result.min_sad = current_sad;
            result.index = i;
        }
    }
    return result;
}