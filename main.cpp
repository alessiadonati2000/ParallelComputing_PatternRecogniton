#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <algorithm>

#include "CSVReader.h"
#include "SequentialRecognition.h" // Nuovo include
#include "ParallelRecognition.h"  // Nuovo include

namespace fs = std::filesystem;

int main() {
    std::string data_path = "../data";
    std::cout << "Cerco i dati nella cartella: " << fs::absolute(data_path) << std::endl;
    std::string query_path = data_path + "/query.csv";

    // 1. Carica la query
    TimeSeries query = read_csv(query_path);
    std::cout << "Query caricata con successo (" << query.values.size() << " punti)." << std::endl;

    // 2. Trova tutti i file delle serie
    std::vector<std::string> series_files;
    for (const auto& entry : fs::directory_iterator(data_path)) {
        if (entry.path().extension() == ".csv" && entry.path().filename().string().rfind("series_", 0) == 0) {
            series_files.push_back(entry.path().string());
        }
    }
    std::cout << "Trovati " << series_files.size() << " file di serie temporali da analizzare." << std::endl;

    // --- ESECUZIONE SEQUENZIALE ---
    std::cout << "\n--- Avvio Ricerca Sequenziale ---" << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    MatchResult overall_best_seq;
    std::string best_file_seq;
    for (const auto& file_path : series_files) {
        TimeSeries series = read_csv(file_path);
        MatchResult result = find_best_match_sequential(series, query); // Chiama la funzione sequenziale
        if (result.min_sad < overall_best_seq.min_sad) {
            overall_best_seq = result;
            best_file_seq = fs::path(file_path).filename().string();
        }
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_seq = end_seq - start_seq;

    // --- ESECUZIONE PARALLELA ---
    std::cout << "\n--- Avvio Ricerca Parallela (OpenMP) ---" << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    MatchResult overall_best_par;
    std::string best_file_par;
    for (const auto& file_path : series_files) {
        TimeSeries series = read_csv(file_path);
        MatchResult result = find_best_match_parallel_omp(series, query); // Chiama la funzione parallela
        if (result.min_sad < overall_best_par.min_sad) {
            overall_best_par = result;
            best_file_par = fs::path(file_path).filename().string();
        }
    }
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_par = end_par - start_par;

    // --- STAMPA RISULTATI ---
    std::cout << "\n================= RISULTATI =================" << std::endl;
    std::cout << "[Sequenziale] Match migliore trovato in '" << best_file_seq << "' all'indice " << overall_best_seq.index << " (SAD: " << overall_best_seq.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "[Parallelo]   Match migliore trovato in '" << best_file_par << "' all'indice " << overall_best_par.index << " (SAD: " << overall_best_par.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_par.count() << " ms" << std::endl;
    std::cout << "===========================================" << std::endl;
    double speedup = (duration_seq.count() > 0) ? duration_seq.count() / duration_par.count() : 0.0;
    std::cout << "\nSpeedup: " << speedup << "x" << std::endl;

    return 0;
}