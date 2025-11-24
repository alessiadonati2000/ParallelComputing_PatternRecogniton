#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>      // Per scrivere su file
#include <numeric>      // Per std::accumulate (calcolo media)
#include <iomanip>      // Per formattare l'output (setw, setprecision)
#include <sstream>      // Per costruire stringhe formattate

// Assicurati che questi header esistano nel tuo progetto
#include "CSVReader.h"
#include "SequentialRecognition.h"
#include "ParallelRecognition.h"

using namespace std;

// Funzione helper per calcolare la media di un vettore di tempi
double calculateAverage(const vector<double>& times) {
    if (times.empty()) return 0.0;
    return accumulate(times.begin(), times.end(), 0.0) / times.size();
}

int main() {
    // ========================================================
    // 1. CONFIGURAZIONE E CARICAMENTO DATI
    // ========================================================
    const int NUM_TESTS = 10; // Numero di ripetizioni per calcolare la media
    string data_path = "../data";
    string query_path = data_path + "/query.csv";

    // Creazione del percorso per i risultati con Timestamp (come nel main.cu)
    auto time = std::time(nullptr);
    auto localTime = *std::localtime(&time);
    std::ostringstream date;
    date << std::put_time(&localTime, "%Y%m%d-%H%M%S");

    // Assicurati che la cartella Results esista o cambiala se necessario
    string outputDir = "../Results";
    if (!filesystem::exists(outputDir)) {
        filesystem::create_directory(outputDir);
    }
    string resultsPath = outputDir + "/results-" + date.str() + ".txt";

    cout << "========================================================" << endl;
    cout << "           CARICAMENTO DATASET" << endl;
    cout << "========================================================" << endl;

    // Caricamento Query
    vector<float> query;
    try {
        query = read_csv(query_path);
    } catch (const exception& e) {
        cerr << "Errore caricamento query: " << e.what() << endl;
        return -1;
    }

    // Caricamento Timeseries
    vector<vector<float>> timeseries;
    for (const auto& entry : filesystem::directory_iterator(data_path)) {
        string filename = entry.path().filename().string();
        if (filename.rfind("series_", 0) == 0) {
            try {
                timeseries.push_back(read_csv(entry.path().string()));
            } catch (const exception& e) {
                cerr << "Errore file " << filename << ": " << e.what() << endl;
            }
        }
    }

    if (timeseries.empty()) {
        cerr << "Nessuna serie temporale trovata." << endl;
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
    MatchResult finalResult; // Per verificare la correttezza (opzionale)

    // ========================================================
    // 2. BENCHMARK SEQUENZIALE
    // ========================================================
    cout << "\n========================================================" << endl;
    cout << "           ESPERIMENTO SEQUENZIALE (CPU)" << endl;
    cout << "========================================================" << endl;

    vector<double> seqTimes;
    for (int i = 0; i < NUM_TESTS; ++i) {
        printf("Avvio test sequenziale %d/%d...\n", i + 1, NUM_TESTS);

        auto start = chrono::high_resolution_clock::now();

        // Esecuzione logica sequenziale per tutte le serie
        // Nota: Qui replico il loop che fai nel codice originale per processare tutte le serie
        MatchResult best_match;
        for (size_t j = 0; j < timeseries.size(); ++j) {
             MatchResult res = sequential_recognition(timeseries[j], query);
             if (j == 0 || res.min_sad < best_match.min_sad) best_match = res;
        }
        finalResult = best_match; // Salviamo l'ultimo risultato per controllo

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> elapsed = end - start; // In millisecondi
        seqTimes.push_back(elapsed.count());
    }
    seqAvg = calculateAverage(seqTimes);
    printf("\nTempo MEDIO Sequenziale: %4.2f ms\n", seqAvg);


    // ========================================================
    // 3. BENCHMARK PARALLELO (V1: BOTTLENECK)
    // ========================================================
    cout << "\n========================================================" << endl;
    cout << "           ESPERIMENTO PARALLELO (V1: BOTTLENECK)" << endl;
    cout << "========================================================" << endl;

    vector<double> p1Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        // printf("Avvio test V1 %d/%d...\n", i + 1, NUM_TESTS); // Decommenta se vuoi output verboso
        auto start = chrono::high_resolution_clock::now();

        parallel_recognition_bottleneck(timeseries, query); // Ignoriamo result qui per velocità

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> elapsed = end - start;
        p1Times.push_back(elapsed.count());
    }
    p1Avg = calculateAverage(p1Times);
    printf("Tempo MEDIO V1 (Bottleneck): %4.2f ms | Speedup: %4.2fx\n", p1Avg, seqAvg / p1Avg);


    // ========================================================
    // 4. BENCHMARK PARALLELO (V2: STANDARD)
    // ========================================================
    cout << "\n========================================================" << endl;
    cout << "           ESPERIMENTO PARALLELO (V2: STANDARD)" << endl;
    cout << "========================================================" << endl;

    vector<double> p2Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        auto start = chrono::high_resolution_clock::now();

        parallel_recognition_standard(timeseries, query);

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> elapsed = end - start;
        p2Times.push_back(elapsed.count());
    }
    p2Avg = calculateAverage(p2Times);
    printf("Tempo MEDIO V2 (Standard):   %4.2f ms | Speedup: %4.2fx\n", p2Avg, seqAvg / p2Avg);


    // ========================================================
    // 5. BENCHMARK PARALLELO (V3: REDUCTION)
    // ========================================================
    cout << "\n========================================================" << endl;
    cout << "           ESPERIMENTO PARALLELO (V3: REDUCTION)" << endl;
    cout << "========================================================" << endl;

    vector<double> p3Times;
    for (int i = 0; i < NUM_TESTS; ++i) {
        auto start = chrono::high_resolution_clock::now();

        parallel_recognition_reduction(timeseries, query);

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> elapsed = end - start;
        p3Times.push_back(elapsed.count());
    }
    p3Avg = calculateAverage(p3Times);
    printf("Tempo MEDIO V3 (Reduction):  %4.2f ms | Speedup: %4.2fx\n", p3Avg, seqAvg / p3Avg);


    // ========================================================
    // 6. SINTESI DEI RISULTATI (CONSOLE & FILE)
    // ========================================================

    // Costruiamo la tabella dei risultati
    stringstream ss;
    ss << left; // Allinea a sinistra per tutto lo stream

    // Header Tabella
    ss << "\n========================================================" << endl;
    ss << "                 SINTESI DEI RISULTATI" << endl;
    ss << "========================================================" << endl;
    ss << "Parametri: " << timeseries.size() << " serie, "
       << timeseries[0].size() << " punti/serie, " << NUM_TESTS << " test mediati." << endl;
    ss << "Miglior Match trovato (SAD): " << finalResult.min_sad
       << " @ Indice " << finalResult.index << endl;
    ss << "--------------------------------------------------------" << endl;

    // Intestazioni Colonne
    ss << setw(25) << "Metodo" << " | "
       << setw(18) << "Tempo Medio (ms)" << " | "
       << setw(10) << "Speedup" << endl;

    ss << setw(25) << "-------------------------" << " | "
       << setw(18) << "------------------" << " | "
       << setw(10) << "----------" << endl;

    // Riga Sequenziale
    ss << setw(25) << "Sequenziale (CPU)" << " | "
       << setw(18) << fixed << setprecision(2) << seqAvg << " | "
       << setw(10) << fixed << setprecision(2) << 1.00 << "x" << endl;

    // Riga V1
    ss << setw(25) << "Parallelo V1 (Bottle)" << " | "
       << setw(18) << fixed << setprecision(2) << p1Avg << " | "
       << setw(10) << fixed << setprecision(2) << (seqAvg / p1Avg) << "x" << endl;

    // Riga V2
    ss << setw(25) << "Parallelo V2 (Standard)" << " | "
       << setw(18) << fixed << setprecision(2) << p2Avg << " | "
       << setw(10) << fixed << setprecision(2) << (seqAvg / p2Avg) << "x" << endl;

    // Riga V3
    ss << setw(25) << "Parallelo V3 (Reduction)" << " | "
       << setw(18) << fixed << setprecision(2) << p3Avg << " | "
       << setw(10) << fixed << setprecision(2) << (seqAvg / p3Avg) << "x" << endl;

    ss << "========================================================" << endl;

    // Stampa a video
    cout << ss.str();

    // Scrittura su file
    ofstream resultsFile(resultsPath);
    if (resultsFile.is_open()) {
        resultsFile << ss.str();
        resultsFile.close();
        cout << "\nRisultati salvati con successo in: " << resultsPath << endl;
    } else {
        cerr << "\nErrore: Impossibile scrivere il file dei risultati in " << resultsPath << endl;
    }

    return 0;
}