#ifndef PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H
#define PARALLELCOMPUTING_PATTERNRECOGNITION_CSVREADER_H

#include <string>
#include <vector>

/**
 * @brief Legge il file .csv e ne estrae i valori salvandoli in vectorLegge il file .csv e ne estrae i valori salvandoli in vector
*/
std::vector<float> read_csv(const std::string& filepath);

#endif