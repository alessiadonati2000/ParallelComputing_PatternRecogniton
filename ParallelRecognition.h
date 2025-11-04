#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H

#include <vector>
#include <string>
#include <limits>
#include "SAD.h" // Per usare calculate_sad

/**
 * @brief Versione 1: "Bottleneck"
 * Mostra un approccio ingenuo con una sezione critica *all'interno* del loop.
 * È lento e serve solo a scopo didattico per mostrare cosa *non* fare.
 */
MatchResult parallel_recognition_bottleneck(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query);

/**
 * @brief Versione 2: "Standard" (Good Practice)
 * Approccio efficiente che usa una variabile "local-best" per ogni thread.
 * Riduce la contesa sulla sezione critica a una sola volta per thread.
 */
MatchResult parallel_recognition_standard(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query);

/**
 * @brief Versione 3: "Reduction" (Advanced)
 * Utilizza una riduzione custom di OpenMP (richiede C++11 e OpenMP 4.5+).
 * È la versione più "pulita" ed elegante dal punto di vista del codice.
 * [cite_start]NOTA: Il tuo CMakeLists.txt con /openmp:llvm (MSVC) [cite: 2] dovrebbe supportarlo.
 */
MatchResult parallel_recognition_reduction(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query);


#endif //PARALLELCOMPUTING_PATTERNRECOGNITION_PARALLELRECOGNITION_H