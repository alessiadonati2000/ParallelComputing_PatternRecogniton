#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <omp.h>
#include <map>
#include <fstream>
#include <iomanip>
#include <functional> // Per std::function
#include <sstream>

#include "CSVReader.h"
#include "SequentialRecognition.h"
#include "ParallelRecognition.h"

// Struttura per contenere i risultati di un test
struct PerformanceResult {
    int threads;
    double time_ms;
    double speedup;
};

// Alias per una funzione di riconoscimento per rendere il codice più pulito
using RecognitionFunction = std::function<MatchResult(const TimeSeries&, const TimeSeries&)>;

// Funzione helper per eseguire un test di scalabilità su una data funzione
std::vector<PerformanceResult> run_scalability_test(
    const std::string& strategy_name,
    const std::vector<int>& thread_counts,
    const std::vector<std::string>& series_files,
    const TimeSeries& query,
    const RecognitionFunction& func,
    double sequential_time_ms
) {
    std::cout << "\n--- Test Strategia: " << strategy_name << " ---" << std::endl;
    std::vector<PerformanceResult> results;

    for (int num_threads : thread_counts) {
        omp_set_num_threads(num_threads);
        std::cout << "  Eseguo con " << num_threads << " thread..." << std::flush;

        auto start_time = std::chrono::high_resolution_clock::now();

        MatchResult overall_best;
        // Questo ciclo esterno sui file è sequenziale.
        // La funzione passata (func) applicherà il parallelismo all'interno di ogni file.
        for (const auto& file_path : series_files) {
            TimeSeries series = read_csv(file_path);
            MatchResult result = func(series, query);
            if (result.min_sad < overall_best.min_sad) {
                overall_best = result;
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        double parallel_time_ms = duration.count();
        double speedup = (parallel_time_ms > 0) ? sequential_time_ms / parallel_time_ms : 0.0;

        results.push_back({num_threads, parallel_time_ms, speedup});
        std::cout << " -> Tempo: " << std::fixed << std::setprecision(2) << parallel_time_ms << " ms | Speedup: " << speedup << "x" << std::endl;
    }
    return results;
}


int main() {
    // --- 1. SETUP INIZIALE ---
    std::string data_path = "../data";
    std::cout << "Cerco i dati nella cartella: " << std::filesystem::absolute(data_path) << std::endl;
    std::string query_path = data_path + "/query.csv";

    TimeSeries query = read_csv(query_path);
    std::cout << "Query caricata con successo (" << query.values.size() << " punti)." << std::endl;

    std::vector<std::string> series_files;
    for (const auto& entry : std::filesystem::directory_iterator(data_path)) {
        if (entry.path().extension() == ".csv" && entry.path().filename().string().rfind("series_", 0) == 0) {
            series_files.push_back(entry.path().string());
        }
    }
    std::cout << "Trovati " << series_files.size() << " file di serie temporali da analizzare." << std::endl;
    if (series_files.empty()) {
        std::cerr << "ERRORE: Nessun file 'series_*.csv' trovato. Esecuzione terminata." << std::endl;
        return 1;
    }

    // --- 2. ESECUZIONE SEQUENZIALE (Baseline) ---
    std::cout << "\n--- Eseguo la versione sequenziale (baseline)... ---" << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    MatchResult overall_best_seq;
    for (const auto& file_path : series_files) {
        TimeSeries series = read_csv(file_path);
        MatchResult result = sequential_recognition(series, query);
        if (result.min_sad < overall_best_seq.min_sad) {
            overall_best_seq = result;
        }
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    double sequential_time_ms = std::chrono::duration<double, std::milli>(end_seq - start_seq).count();
    std::cout << "Tempo di esecuzione [Sequenziale]: " << sequential_time_ms << " ms" << std::endl;

    // --- 3. STUDIO DI SCALABILITA' PER OGNI STRATEGIA ---
    std::vector<int> thread_counts = {1, 2, 4, 8, 12};
    std::map<std::string, std::vector<PerformanceResult>> all_results;

    // Eseguiamo i test per le strategie che parallelizzano il calcolo INTERNO al file
    all_results["Bottleneck"] = run_scalability_test("Bottleneck", thread_counts, series_files, query, parallel_recognition_bottleneck, sequential_time_ms);
    all_results["Local Results"] = run_scalability_test("Local Results", thread_counts, series_files, query, parallel_recognition, sequential_time_ms);
    all_results["Reduction"] = run_scalability_test("Reduction", thread_counts, series_files, query, parallel_recognition_reduction, sequential_time_ms);

    // Test per la strategia che parallelizza l'elaborazione DEI FILE
    std::cout << "\n--- Test Strategia: Parallelism Over Files ---" << std::endl;
    std::vector<PerformanceResult> over_files_results;
    for (int num_threads : thread_counts) {
        omp_set_num_threads(num_threads);
        std::cout << "  Eseguo con " << num_threads << " thread..." << std::flush;

        auto start_par = std::chrono::high_resolution_clock::now();
        MatchResult overall_best_par;
        #pragma omp parallel
        {
            MatchResult thread_best_result;
            #pragma omp for
            for (int i = 0; i < series_files.size(); ++i) {
                TimeSeries series = read_csv(series_files[i]);
                MatchResult result = sequential_recognition(series, query); // Chiamata sequenziale interna
                if (result.min_sad < thread_best_result.min_sad) {
                    thread_best_result = result;
                }
            }
            #pragma omp critical
            {
                if (thread_best_result.min_sad < overall_best_par.min_sad) {
                    overall_best_par = thread_best_result;
                }
            }
        }
        auto end_par = std::chrono::high_resolution_clock::now();
        double parallel_time_ms = std::chrono::duration<double, std::milli>(end_par - start_par).count();
        double speedup = (parallel_time_ms > 0) ? sequential_time_ms / parallel_time_ms : 0.0;
        over_files_results.push_back({num_threads, parallel_time_ms, speedup});
        std::cout << " -> Tempo: " << std::fixed << std::setprecision(2) << parallel_time_ms << " ms | Speedup: " << speedup << "x" << std::endl;
    }
    all_results["Parallel Over Files"] = over_files_results;

    // --- 4. STAMPA TABELLA RIASSUNTIVA A CONSOLE ---
    std::cout << "\n\n=================================== RIEPILOGO FINALE ===================================" << std::endl;
    std::cout << "Tempo Sequenziale di Riferimento: " << sequential_time_ms << " ms" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    // Stampa intestazione dinamica
    std::cout << std::left << std::setw(10) << "Threads";
    for(const auto& pair : all_results) {
        std::cout << std::setw(22) << (pair.first + " (Speedup)");
    }
    std::cout << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;

    // Stampa righe dati
    for (size_t i = 0; i < thread_counts.size(); ++i) {
        std::cout << std::left << std::setw(10) << thread_counts[i];
        for (const auto& pair : all_results) {
            const auto& result = pair.second[i];
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << result.time_ms << "ms (" << result.speedup << "x)";
            std::cout << std::setw(22) << ss.str();
        }
        std::cout << std::endl;
    }
    std::cout << "======================================================================================" << std::endl;

    // --- 5. SALVATAGGIO RISULTATI SU FILE CSV ---
    std::string output_filename = "full_benchmark_results.csv";
    std::ofstream output_file(output_filename);
    if (!output_file.is_open()) {
        std::cerr << "ERRORE: Impossibile creare il file di output: " << output_filename << std::endl;
    } else {
        output_file << "Strategy,Threads,Time_ms,Speedup,SequentialBaseTime_ms\n";
        for (const auto& pair : all_results) {
            const std::string& strategy_name = pair.first;
            for (const auto& result : pair.second) {
                output_file << strategy_name << ","
                            << result.threads << ","
                            << result.time_ms << ","
                            << result.speedup << ","
                            << sequential_time_ms << "\n";
            }
        }
        output_file.close();
        std::cout << "\nI risultati completi sono stati salvati nel file: "
                  << std::filesystem::absolute(output_filename) << std::endl;
    }

    return 0;
}

/*int main() {
    std::string data_path = "../data";
    std::cout << "Cerco i dati nella cartella: " << std::filesystem::absolute(data_path) << std::endl;
    std::string query_path = data_path + "/query.csv";

    // 1. Carica la query
    TimeSeries query = read_csv(query_path);
    std::cout << "Query caricata con successo (" << query.values.size() << " punti)." << std::endl;

    // 2. Trova tutti i file delle serie
    std::vector<std::string> series_files;
    for (const auto& entry : std::filesystem::directory_iterator(data_path)) {
        if (entry.path().extension() == ".csv" && entry.path().filename().string().rfind("series_", 0) == 0) {
            series_files.push_back(entry.path().string());
        }
    }
    std::cout << "Trovati " << series_files.size() << " file di serie temporali da analizzare." << std::endl;


    // ------------------------------------ ESECUZIONE SEQUENZIALE -----------------------------------------------------
    std::cout << "\n--- Avvio Ricerca Sequenziale ---" << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
        MatchResult overall_best_seq;
        std::string best_serie_seq;
        for (const auto& file_path : series_files) {
            TimeSeries series = read_csv(file_path);
            MatchResult result = sequential_recognition(series, query);
            if (result.min_sad < overall_best_seq.min_sad) {
                overall_best_seq = result;
                best_serie_seq = std::filesystem::path(file_path).filename().string();
            }
        }
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_seq = end_seq - start_seq;


    // ---------------------------------------- ESECUZIONE PARALLELA ---------------------------------------------------
    std::cout << "\n--- Avvio Ricerca Parallela (Collo di bottiglia) ---" << std::endl;
    std::cout << "Numero massimo di thread disponibili: " << omp_get_max_threads() << std::endl;
    auto start_par_bottleneck = std::chrono::high_resolution_clock::now();
        MatchResult overall_best_par_bottleneck;
        std::string best_file_par_bottleneck;
        for (const auto& file_path : series_files) {
            TimeSeries series = read_csv(file_path);
            MatchResult result = parallel_recognition_bottleneck(series, query);
            if (result.min_sad < overall_best_par_bottleneck.min_sad) {
                overall_best_par_bottleneck = result;
                best_file_par_bottleneck = std::filesystem::path(file_path).filename().string();
            }
        }
    auto end_par_bottleneck = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_par_bottleneck = end_par_bottleneck - start_par_bottleneck;


    std::cout << "\n--- Avvio Ricerca Parallela (OpenMP) ---" << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
        MatchResult overall_best_par;
        std::string best_file_par;
        for (const auto& file_path : series_files) {
            TimeSeries series = read_csv(file_path);
            MatchResult result = parallel_recognition(series, query);
            if (result.min_sad < overall_best_par.min_sad) {
                overall_best_par = result;
                best_file_par = std::filesystem::path(file_path).filename().string();
            }
        }
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_par = end_par - start_par;


    // --- ESECUZIONE PARALLELA su ciascun file---
    std::cout << "\n--- Avvio Ricerca Parallela per serie (OpenMP) ---" << std::endl;
    auto start_par_serie = std::chrono::high_resolution_clock::now();
        MatchResult overall_best_par_serie;
        std::string best_file_par_serie;
        // Struttura per tenere traccia del risultato per ogni file
        struct FileResult {
            MatchResult match;
            std::string filename;
        };
        // Vettore per raccogliere i risultati da ogni thread
        std::vector<FileResult> results_per_thread(series_files.size());
#pragma omp parallel for
        for (int i = 0; i < series_files.size(); ++i) {
            const auto& file_path = series_files[i];
            TimeSeries series = read_csv(file_path);
            MatchResult result = sequential_recognition(series, query);
            results_per_thread[i] = {result, std::filesystem::path(file_path).filename().string()};
        }
        for(const auto& res : results_per_thread) {
            if (res.match.min_sad < overall_best_par_serie.min_sad) {
                overall_best_par_serie = res.match;
                best_file_par_serie = res.filename;
            }
        }
    auto end_par_serie = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_par_serie = end_par_serie - start_par_serie;

    std::cout << "\n--- Avvio Ricerca Parallela (Reduction) ---" << std::endl;
    auto start_par_reduction = std::chrono::high_resolution_clock::now();
    MatchResult overall_best_par_reduction;
    std::string best_file_par_reduction;
    for (const auto& file_path : series_files) {
        TimeSeries series = read_csv(file_path);
        MatchResult result = parallel_recognition_reduction(series, query);
        if (result.min_sad < overall_best_par_reduction.min_sad) {
            overall_best_par_reduction = result;
            best_file_par_reduction = std::filesystem::path(file_path).filename().string();
        }
    }
    auto end_par_reduction = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_par_reduction = end_par_reduction - start_par_reduction;


    // --- STAMPA RISULTATI ---
    std::cout << "\n================= RISULTATI =================" << std::endl;
    std::cout << "[Sequenziale] Match migliore trovato in '" << best_serie_seq << "' all'indice " << overall_best_seq.index << " (SAD: " << overall_best_seq.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "[Parallelo bottleneck]   Match migliore trovato in '" << best_file_par_bottleneck << "' all'indice " << overall_best_par_bottleneck.index << " (SAD: " << overall_best_par_bottleneck.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_par_bottleneck.count() << " ms" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "[Parallelo]   Match migliore trovato in '" << best_file_par << "' all'indice " << overall_best_par.index << " (SAD: " << overall_best_par.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_par.count() << " ms" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "[Parallelo per thread]   Match migliore trovato in '" << best_file_par_serie << "' all'indice " << overall_best_par_serie.index << " (SAD: " << overall_best_par_serie.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_par_serie.count() << " ms" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "[Parallelo reduction]   Match migliore trovato in '" << best_file_par_reduction << "' all'indice " << overall_best_par_reduction.index << " (SAD: " << overall_best_par_reduction.min_sad << ")" << std::endl;
    std::cout << "              Tempo di esecuzione: " << duration_par_reduction.count() << " ms" << std::endl;
    std::cout << "===========================================" << std::endl;

    double speedup_bottleneck = (duration_seq.count() > 0) ? duration_seq.count() / duration_par_bottleneck.count() : 0.0;
    std::cout << "\nSpeedup - bottleneck: " << speedup_bottleneck << "x" << std::endl;
    double speedup = (duration_seq.count() > 0) ? duration_seq.count() / duration_par.count() : 0.0;
    std::cout << "\nSpeedup: " << speedup << "x" << std::endl;
    double speedup_serie = (duration_seq.count() > 0) ? duration_seq.count() / duration_par_serie.count() : 0.0;
    std::cout << "\nSpeedup - per serie: " << speedup_serie << "x" << std::endl;
    double speedup_reduction = (duration_seq.count() > 0) ? duration_seq.count() / duration_par_reduction.count() : 0.0;
    std::cout << "\nSpeedup - reduction: " << speedup_reduction << "x" << std::endl;

    return 0;
}*/