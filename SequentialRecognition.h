#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H

#include "CSVReader.h"
#include "SAD.h" // Per usare MatchResult e calculate_sad

MatchResult sequential_recognition(const TimeSeries& series, const TimeSeries& query);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H