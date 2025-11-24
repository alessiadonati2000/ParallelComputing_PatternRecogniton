#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>      // Per scrivere su file
#include <numeric>      // Per std::accumulate (calcolo media)
#include <iomanip>      // Per formattare l'output (setw, setprecision)
#include <sstream>      // Per costruire stringhe formattate

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

int main() {
    // ========================================================
    // 1. CONFIGURAZIONE E CARICAMENTO DATI
    // ========================================================
    const int NUM_TESTS = 10;
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
    double p1Avg = 0.0, p2Avg = 0.0, p3Avg = 0.0;
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

        // Esecuzione logica sequenziale per tutte le serie
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
    std::cout << "\n========================================================" << std::endl;
    std::cout << "           ESPERIMENTO PARALLELO (V1: BOTTLENECK)" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::vector<double> p1Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        // printf("Avvio test V1 %d/%d...\n", i + 1, NUM_TESTS); // Decommenta se vuoi output verboso
        auto start = std::chrono::high_resolution_clock::now();

        parallel_recognition_bottleneck(timeseries, query);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        p1Times.push_back(elapsed.count());
    }
    p1Avg = calculateAverage(p1Times);
    printf("Tempo MEDIO V1 (Bottleneck): %4.2f ms | Speedup: %4.2fx\n", p1Avg, seqAvg / p1Avg);


    // ========================================================
    // 4. BENCHMARK PARALLELO (V2: STANDARD)
    // ========================================================
    std::cout << "\n========================================================" << std::endl;
    std::cout << "           ESPERIMENTO PARALLELO (V2: STANDARD)" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::vector<double> p2Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        parallel_recognition_standard(timeseries, query);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        p2Times.push_back(elapsed.count());
    }
    p2Avg = calculateAverage(p2Times);
    printf("Tempo MEDIO V2 (Standard):   %4.2f ms | Speedup: %4.2fx\n", p2Avg, seqAvg / p2Avg);


    // ========================================================
    // 5. BENCHMARK PARALLELO (V3: REDUCTION)
    // ========================================================
    std::cout << "\n========================================================" << std::endl;
    std::cout << "           ESPERIMENTO PARALLELO (V3: REDUCTION)" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::vector<double> p3Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        parallel_recognition_reduction(timeseries, query);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        p3Times.push_back(elapsed.count());
    }
    p3Avg = calculateAverage(p3Times);
    printf("Tempo MEDIO V3 (Reduction):  %4.2f ms | Speedup: %4.2fx\n", p3Avg, seqAvg / p3Avg);


    // ========================================================
    // 6. SINTESI DEI RISULTATI (CONSOLE & FILE)
    // ========================================================

    // Costruiamo la tabella dei risultati
    std::stringstream ss;
    ss << std::left; // Allinea a sinistra per tutto lo stream

    // Header Tabella
    ss << "\n========================================================" << std::endl;
    ss << "                 SINTESI DEI RISULTATI" << std::endl;
    ss << "========================================================" << std::endl;
    ss << "Parametri: " << timeseries.size() << " serie, "
       << timeseries[0].size() << " punti/serie, " << NUM_TESTS << " test mediati." << std::endl;
    ss << "Miglior Match trovato (SAD): " << finalResult.min_sad
       << " @ Indice " << finalResult.index << std::endl;
    ss << "--------------------------------------------------------" << std::endl;

    // Intestazioni Colonne
    ss << std::setw(25) << "Metodo" << " | "
       << std::setw(18) << "Tempo Medio (ms)" << " | "
       << std::setw(10) << "Speedup" << std::endl;

    ss << std::setw(25) << "-------------------------" << " | "
       << std::setw(18) << "------------------" << " | "
       << std::setw(10) << "----------" << std::endl;

    // Riga Sequenziale
    ss << std::setw(25) << "Sequenziale (CPU)" << " | "
       << std::setw(18) << std::fixed << std::setprecision(2) << seqAvg << " | "
       << std::setw(10) << std::fixed << std::setprecision(2) << 1.00 << "x" << std::endl;

    // Riga V1
    ss << std::setw(25) << "Parallelo V1 (Bottle)" << " | "
       << std::setw(18) << std::fixed << std::setprecision(2) << p1Avg << " | "
       << std::setw(10) << std::fixed << std::setprecision(2) << (seqAvg / p1Avg) << "x" << std::endl;

    // Riga V2
    ss << std::setw(25) << "Parallelo V2 (Standard)" << " | "
       << std::setw(18) << std::fixed << std::setprecision(2) << p2Avg << " | "
       << std::setw(10) << std::fixed << std::setprecision(2) << (seqAvg / p2Avg) << "x" << std::endl;

    // Riga V3
    ss << std::setw(25) << "Parallelo V3 (Reduction)" << " | "
       << std::setw(18) << std::fixed << std::setprecision(2) << p3Avg << " | "
       << std::setw(10) << std::fixed << std::setprecision(2) << (seqAvg / p3Avg) << "x" << std::endl;

    ss << "========================================================" << std::endl;

    // Stampa a video
    std::cout << ss.str();

    // Scrittura su file
    std::ofstream resultsFile(resultsPath);
    if (resultsFile.is_open()) {
        resultsFile << ss.str();
        resultsFile.close();
        std::cout << "\nRisultati salvati con successo in: " << resultsPath << std::endl;
    } else {
        std::cerr << "\nErrore: Impossibile scrivere il file dei risultati in " << resultsPath << std::endl;
    }

    return 0;
}