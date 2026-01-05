#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>      // Per scrivere su file
#include <numeric>      // Per std::accumulate (calcolo media)
#include <iomanip>      // Per formattare l'output (setw, setprecision)
#include <sstream>      // Per costruire stringhe formattate
#include <omp.h>

#include "CSVReader.h"
#include "SequentialRecognition.h"
#include "ParallelRecognition.h"

/**
 * @brief Funzione helper per calcolare la media di un vettore di tempi
*/
double calculateAverage(const std::vector<double>& times) {
    if (times.empty()) return 0.0;
    return accumulate(times.begin(), times.end(), 0.0) / times.size();
}

/**
 * @brief Struttura per memorizzare i risultati di una specifica configurazione di thread
 */
struct ThreadConfigResult {
    int numThreads;
    double v1Avg, v2Avg, v3Avg;
};

int main() {
    // ========================================================
    // 1. CONFIGURAZIONE E CARICAMENTO DATI
    // ========================================================
    const int NUM_TESTS = 10;
    std::vector<int> THREAD_COUNTS = {2, 4, 8, 12};
    std::string data_path = "../data";
    std::string query_path = data_path + "/query.csv";

    // Creazione del percorso per i risultati con Timestamp
    auto time = std::time(nullptr);
    auto localTime = *std::localtime(&time);
    std::ostringstream date;
    date << std::put_time(&localTime, "%Y%m%d-%H%M%S");

    std::string outputDir = "../Results";
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directory(outputDir);
    }
    std::string resultsPath = outputDir + "/results-" + date.str() + ".txt";

    std::cout << "========================================================" << std::endl;
    std::cout << "           CARICAMENTO DATASET" << std::endl;
    std::cout << "========================================================" << std::endl;

    // Caricamento Query
    std::vector<float> query;
    try {
        query = read_csv(query_path);
    } catch (const std::exception& e) {
        std::cerr << "Errore caricamento query: " << e.what() << std::endl;
        return -1;
    }

    // Caricamento Timeseries
    std::vector<std::vector<float>> timeseries;
    for (const auto& entry : std::filesystem::directory_iterator(data_path)) {
        std::string filename = entry.path().filename().string();
        if (filename.rfind("series_", 0) == 0) {
            try {
                timeseries.push_back(read_csv(entry.path().string()));
            } catch (const std::exception& e) {
                std::cerr << "Errore file " << filename << ": " << e.what() << std::endl;
            }
        }
    }

    if (timeseries.empty()) {
        std::cerr << "Nessuna serie temporale trovata." << std::endl;
        return -1;
    }

    // Stampa Info Dataset
    printf("Serie temporali caricate: %zu\n", timeseries.size());
    printf("Lunghezza serie (punti):  %zu\n", timeseries[0].size());
    printf("Lunghezza query (punti):  %zu\n", query.size());
    printf("Ripetizioni test:         %d\n", NUM_TESTS);

    // Variabili per memorizzare i risultati finali
    double seqAvg = 0.0;
    MatchResult finalResult;

    // ========================================================
    // 2. BENCHMARK SEQUENZIALE
    // ========================================================
    std::cout << "\n========================================================" << std::endl;
    std::cout << "           ESPERIMENTO SEQUENZIALE (CPU)" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::vector<double> seqTimes;
    for (int i = 0; i < NUM_TESTS; ++i) {
        printf("Avvio test sequenziale %d/%d...\n", i + 1, NUM_TESTS);

        auto start = std::chrono::high_resolution_clock::now();

        MatchResult best_match;
        for (size_t j = 0; j < timeseries.size(); ++j) {
             MatchResult res = sequential_recognition(timeseries[j], query);
             if (j == 0 || res.min_sad < best_match.min_sad) best_match = res;
        }
        finalResult = best_match;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        seqTimes.push_back(elapsed.count());
    }
    seqAvg = calculateAverage(seqTimes);
    printf("\nTempo MEDIO Sequenziale: %4.2f ms\n", seqAvg);


    // ========================================================
    // 3. BENCHMARK PARALLELO (V1: BOTTLENECK)
    // ========================================================

    std::vector<ThreadConfigResult> allResults;

    for (int n : THREAD_COUNTS) {
        std::cout << "\n>>> AVVIO TEST CON " << n << " THREAD <<<" << std::endl;
        omp_set_num_threads(n);

        ThreadConfigResult currentRes;
        currentRes.numThreads = n;

        std::cout << "\n========================================================" << std::endl;
        std::cout << "           ESPERIMENTO PARALLELO (V1: BOTTLENECK)" << std::endl;
        std::cout << "========================================================" << std::endl;
        std::vector<double> v1Times;
        for (int i = 0; i < NUM_TESTS; ++i) {
            // printf("Avvio test V1 %d/%d...\n", i + 1, NUM_TESTS); // Decommenta se vuoi output verboso
            auto start = std::chrono::high_resolution_clock::now();

            parallel_recognition_bottleneck(timeseries, query);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            v1Times.push_back(elapsed.count());
        }
        currentRes.v1Avg = calculateAverage(v1Times);

        std::cout << "\n========================================================" << std::endl;
        std::cout << "           ESPERIMENTO PARALLELO (V2: STANDARD)" << std::endl;
        std::cout << "========================================================" << std::endl;
        std::vector<double> v2Times;
        for (int i = 0; i < NUM_TESTS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            parallel_recognition_standard(timeseries, query);
            auto end = std::chrono::high_resolution_clock::now();
            v2Times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        currentRes.v2Avg = calculateAverage(v2Times);

        std::cout << "\n========================================================" << std::endl;
        std::cout << "           ESPERIMENTO PARALLELO (V3: REDUCTION)" << std::endl;
        std::cout << "========================================================" << std::endl;
        std::vector<double> v3Times;
        for (int i = 0; i < NUM_TESTS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            parallel_recognition_reduction(timeseries, query);
            auto end = std::chrono::high_resolution_clock::now();
            v3Times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        currentRes.v3Avg = calculateAverage(v3Times);

        allResults.push_back(currentRes);
        printf("Completati test per %d thread.\n", n);

    }

    // ========================================================
    // 6. SINTESI DEI RISULTATI
    // ========================================================
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "\n================================================================================" << std::endl;
    ss << "                      REPORT SCALABILITÀ E PERFORMANCE" << std::endl;
    ss << "================================================================================" << std::endl;
    ss << "Tempo Sequenziale di riferimento: " << seqAvg << " ms\n" << std::endl;

    ss << std::left << std::setw(12) << "Threads"
       << " | " << std::setw(18) << "V1 Bottleneck"
       << " | " << std::setw(18) << "V2 Standard"
       << " | " << std::setw(18) << "V3 Reduction" << std::endl;
    ss << std::string(80, '-') << std::endl;

    for (const auto& res : allResults) {
        ss << std::left << std::setw(12) << ("  " + std::to_string(res.numThreads)) << " | ";

        // Colonna V1 (Tempo e Speedup)
        ss << std::setw(7) << res.v1Avg << " (" << std::setw(4) << (seqAvg/res.v1Avg) << "x) | ";

        // Colonna V2 (Tempo e Speedup)
        ss << std::setw(7) << res.v2Avg << " (" << std::setw(4) << (seqAvg/res.v2Avg) << "x) | ";

        // Colonna V3 (Tempo e Speedup)
        ss << std::setw(7) << res.v3Avg << " (" << std::setw(4) << (seqAvg/res.v3Avg) << "x)" << std::endl;
    }
    ss << "================================================================================" << std::endl;

    std::cout << ss.str();

    // Salvataggio su file
    auto t = std::time(nullptr);
    auto lt = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&lt, "%Y%m%d-%H%M%S");
    std::ofstream file(outputDir + "/benchmark_threads_" + oss.str() + ".txt");
    if (file.is_open()) { file << ss.str(); file.close(); }

    return 0;
}