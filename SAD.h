#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H

#include "CSVReader.h"
#include <limits>

// Dimensione tipica di una cache line in byte
#define CACHE_LINE_SIZE 64

// Struttura comune per contenere il risultato
struct MatchResult {
    double min_sad = std::numeric_limits<double>::max();
    long long index = -1;
    // Aggiungiamo un'imbottitura per prevenire il False Sharing.
    // La struct ora occuperà 64 byte in totale.
    // (8 per double + 8 per long long = 16. Servono altri 48 byte)
    char padding[CACHE_LINE_SIZE - sizeof(double) - sizeof(long long)];
};

// Calcola il SAD tra una porzione di 'series' e l'intera 'query'
double calculate_sad(const TimeSeries& series, const TimeSeries& query, long long start_index);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_SAD_H