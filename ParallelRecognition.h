#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H

#include "CSVReader.h"
#include "SAD.h" // Per usare MatchResult e calculate_sad

MatchResult find_best_match_parallel_omp(const TimeSeries& series, const TimeSeries& query);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H