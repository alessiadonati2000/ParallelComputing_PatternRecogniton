#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H

#include "SAD.h"

/**
 * @brief Funzione per l'esecuzione sequenziale della ricerca
 */
MatchResult sequential_recognition(const std::vector<float>& serie, const std::vector<float>& query);

#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_SEQUENTIALRECOGNITION_H