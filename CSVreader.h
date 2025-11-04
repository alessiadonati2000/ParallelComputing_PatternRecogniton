#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H

#include <string>
#include <vector>

// Funzione per leggere una serie temporale da un file CSV e restituirla come vector
std::vector<float> read_csv(const std::string& filepath);

#endif