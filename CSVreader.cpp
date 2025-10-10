#include "CSVReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

TimeSeries read_csv(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Impossibile aprire il file: " + filepath);
    }

    TimeSeries ts;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;

        // Leggi la prima cella (timestamp), ma ignorala
        std::getline(ss, cell, ';');

        // Leggi la seconda cella (valore)
        if (std::getline(ss, cell, ';')) {
            try {
                ts.values.push_back(std::stod(cell));
            } catch (const std::invalid_argument& e) {
                // Ignora righe mal formattate se necessario
            }
        }
    }

    file.close();
    return ts;
}
