#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H

#include "CSVReader.h"
#include "SAD.h" // Per usare MatchResult e calculate_sad

MatchResult parallel_recognition_bottleneck(const TimeSeries& series, const TimeSeries& query);

MatchResult parallel_recognition(const TimeSeries& series, const TimeSeries& query);

MatchResult parallel_recognition_reduction(const TimeSeries& series, const TimeSeries& query);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H