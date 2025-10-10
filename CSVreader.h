#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H

#include <string>
#include <vector>

// Struttura per contenere i valori di una serie temporale
struct TimeSeries {
    std::vector<double> values;
};

// Funzione per leggere una serie temporale da un file CSV
TimeSeries read_csv(const std::string& filepath);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H