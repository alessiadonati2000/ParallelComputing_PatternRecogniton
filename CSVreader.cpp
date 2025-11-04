#include "CSVReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

// Legge il file .csv e ne estrae i valori salvandoli in vector
std::vector<float> read_csv(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Impossibile aprire il file: " + filepath);
    }

    std::vector<float> timeserie;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;

        // Leggi la prima cella (timestamp), ma ignorala
        std::getline(ss, cell, ';');

        // Leggi la seconda cella (valore)
        if (std::getline(ss, cell, ';')) {
            try {
                timeserie.push_back(std::stof(cell));
            } catch (const std::invalid_argument& e) {
                // Ignora righe mal formattate se necessario
            }
        }
    }

    file.close();
    return timeserie;
}
