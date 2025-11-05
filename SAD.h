#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H

#include <limits>
#include <vector>

struct MatchResult {
    double min_sad = std::numeric_limits<double>::max();
    long long index = -1;
};

double calculate_sad(const std::vector<float>& series, const std::vector<float>& query, long long start_index);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H