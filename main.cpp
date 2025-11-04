#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

#include "CSVReader.h"
#include "SequentialRecognition.h"
#include "ParallelRecognition.h"

int main() {
    // Acquisizione dati
    std::string data_path = "../data";
    std::string query_path = data_path + "/query.csv";

    std::vector<float> query = read_csv(query_path);

    std::vector<std::vector<float>> timeseries;
    for (const auto& entry : std::filesystem::directory_iterator(data_path)) {
        std::string filename = entry.path().filename().string();
        if (filename.rfind("series_", 0) == 0) {
            try {
                std::vector<float> current_timeserie = read_csv(entry.path().string());
                timeseries.push_back(current_timeserie);
            } catch (const std::exception& e) {
                std::cerr << "Errore durante il caricamento di " << filename << ": " << e.what() << std::endl;
            }
        }
    }

    // Sequenziale
    std::cout << "\n--- Avvio Ricerca Sequenziale ---" << std::endl;
    std::vector<MatchResult> results_seq;
    results_seq.reserve(timeseries.size()); // Ottimizzazione: pre-alloca memoria

    auto start_time_seq = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < timeseries.size(); ++i) {
            MatchResult result = sequential_recognition(timeseries[i], query);
            results_seq.push_back(result);
        }

    auto end_time_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_seq = end_time_seq - start_time_seq;

    // Riepilogo Risultati e Tempi
    std::cout << "\n--- Riepilogo Esecuzione Sequenziale ---" << std::endl;
    std::cout << "Tempo totale: " << duration_seq.count() << " secondi" << std::endl;
    std::cout << "Serie analizzate: " << timeseries.size() << std::endl;

    double best_sad_so_far = std::numeric_limits<double>::max();
    int best_series_index = -1;

    for (size_t i = 0; i < results_seq.size(); ++i) {
        if (results_seq[i].min_sad < best_sad_so_far) {
            best_sad_so_far = results_seq[i].min_sad;
            best_series_index = i;
        }
    }

    // Stampa il miglior risultato trovato
    if (best_series_index != -1) {
        std::cout << "\n--- Miglior Match Trovato ---" << std::endl;
        std::cout << "  Indice nel file: " << results_seq[best_series_index].index << std::endl;
        std::cout << "  Valore SAD (minimo): " << results_seq[best_series_index].min_sad << std::endl;
    } else {
        std::cout << "\nNessun match è stato trovato in nessuna serie." << std::endl;
    }
    std::cout << "--------------------------------------" << std::endl;

    // Parallelo

    // Risultati parallelo

    // Confronto sequenziale vs parallelo

    return 0;
}
