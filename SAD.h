#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H

#include "CSVReader.h"
#include <limits>

// Struttura comune per contenere il risultato
struct MatchResult {
    double min_sad = std::numeric_limits<double>::max();
    long long index = -1;
};

// Calcola il SAD tra una porzione di 'series' e l'intera 'query'
double calculate_sad(const TimeSeries& series, const TimeSeries& query, long long start_index);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H