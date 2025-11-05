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

    std::cout << "Caricamento query da: " << query_path << std::endl;
    std::vector<float> query = read_csv(query_path);

    std::cout << "Caricamento dataset da: " << data_path << std::endl;
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
    std::cout << "Caricate " << timeseries.size() << " serie temporali." << std::endl;

    // --- Sequenziale ---
    std::cout << "\n--- Avvio Ricerca Sequenziale ---" << std::endl;
    std::vector<MatchResult> results_seq;
    results_seq.reserve(timeseries.size()); // Ottimizzazione: pre-alloca memoria

    auto start_time_seq = std::chrono::high_resolution_clock::now();

    // NOTA: Questo loop calcola il best-match per *ogni* serie
    // e lo salva in un vettore.
    for (size_t i = 0; i < timeseries.size(); ++i) {
        MatchResult result = sequential_recognition(timeseries[i], query);
        results_seq.push_back(result);
    }

    auto end_time_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_seq = end_time_seq - start_time_seq;

    // Trova il migliore *tra tutti* i risultati
    MatchResult best_sequential_match;
    for (size_t i = 0; i < results_seq.size(); ++i) {
        if (results_seq[i].min_sad < best_sequential_match.min_sad) {
            best_sequential_match = results_seq[i];
            // Potremmo voler salvare anche l'indice della *serie*
            // best_sequential_match.series_index = i; // (Richiede modifica a MatchResult)
        }
    }

    std::cout << "Tempo totale Sequenziale: " << duration_seq.count() << " secondi" << std::endl;

    // Stampa il miglior risultato trovato
    if (best_sequential_match.index != -1) {
        std::cout << "\n--- Miglior Match Trovato (Sequenziale) ---" << std::endl;
        std::cout << "  Indice nel file: " << best_sequential_match.index << std::endl;
        std::cout << "  Valore SAD (minimo): " << best_sequential_match.min_sad << std::endl;
    } else {
        std::cout << "\nNessun match è stato trovato in nessuna serie." << std::endl;
    }
    std::cout << "--------------------------------------" << std::endl;


    // === INIZIO CODICE AGGIUNTO ===

    // --- Parallelo V1: Bottleneck ---
    std::cout << "\n--- Avvio Ricerca Parallela (V1: Bottleneck) ---" << std::endl;
    auto start_time_p1 = std::chrono::high_resolution_clock::now();
    MatchResult result_p1 = parallel_recognition_bottleneck(timeseries, query);
    auto end_time_p1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_p1 = end_time_p1 - start_time_p1;
    std::cout << "Tempo (Bottleneck): " << duration_p1.count() << " secondi" << std::endl;


    // --- Parallelo V2: Standard ---
    std::cout << "\n--- Avvio Ricerca Parallela (V2: Standard) ---" << std::endl;
    auto start_time_p2 = std::chrono::high_resolution_clock::now();
    MatchResult result_p2 = parallel_recognition_standard(timeseries, query);
    auto end_time_p2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_p2 = end_time_p2 - start_time_p2;
    std::cout << "Tempo (Standard): " << duration_p2.count() << " secondi" << std::endl;


    // --- Parallelo V3: Reduction ---
    /*std::cout << "\n--- Avvio Ricerca Parallela (V3: Reduction) ---" << std::endl;
    auto start_time_p3 = std::chrono::high_resolution_clock::now();
    MatchResult result_p3 = parallel_recognition_reduction(timeseries, query);
    auto end_time_p3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_p3 = end_time_p3 - start_time_p3;
    std::cout << "Tempo (Reduction): " << duration_p3.count() << " secondi" << std::endl;*/


    // --- Riepilogo e Confronto ---
    std::cout << "\n\n--- Riepilogo e Confronto Risultati ---" << std::endl;
    std::cout << "Miglior Match (Sequenziale): \tSAD=" << best_sequential_match.min_sad
              << " | Indice=" << best_sequential_match.index << std::endl;
    std::cout << "Miglior Match (Par. Bottleneck): \tSAD=" << result_p1.min_sad
              << " | Indice=" << result_p1.index << std::endl;
    std::cout << "Miglior Match (Par. Standard): \tSAD=" << result_p2.min_sad
              << " | Indice=" << result_p2.index << std::endl;
    /*std::cout << "Miglior Match (Par. Reduction): \tSAD=" << result_p3.min_sad
              << " | Indice=" << result_p3.index << std::endl;*/

    // --- Confronto Tempi e Speedup ---
    std::cout << "\n--- Riepilogo Tempi (Speedup vs Sequenziale) ---" << std::endl;
    std::cout << "Sequenziale: \t\t" << duration_seq.count() << " secondi \t(Baseline 1.00x)" << std::endl;

    std::cout << "Par. (Bottleneck): \t" << duration_p1.count() << " secondi \t(Speedup: "
              << (duration_seq.count() / duration_p1.count()) << "x)" << std::endl;

    std::cout << "Par. (Standard): \t" << duration_p2.count() << " secondi \t(Speedup: "
              << (duration_seq.count() / duration_p2.count()) << "x)" << std::endl;

    /*std::cout << "Par. (Reduction): \t" << duration_p3.count() << " secondi \t(Speedup: "
              << (duration_seq.count() / duration_p3.count()) << "x)" << std::endl;*/

    std::cout << "------------------------------------------------" << std::endl;

    // === FINE CODICE AGGIUNTO ===

    return 0;
}