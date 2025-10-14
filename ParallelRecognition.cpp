#include "ParallelRecognition.h"
#include <omp.h>
#include <vector>
#include <stdexcept>

// Questa versione con omp critical genera un collo di bottiglia
MatchResult parallel_recognition_bottleneck(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        throw std::runtime_error("Query più grande della serie oppure vuota.");
    }

    MatchResult global_result;
    long long search_range = series.values.size() - query.values.size();

#pragma omp parallel for
    for (long long i = 0; i <= search_range; ++i) {
        double current_sad = calculate_sad(series, query, i);

#pragma omp critical
        {
            if (current_sad < global_result.min_sad) {
                global_result.min_sad = current_sad;
                global_result.index = i;
            }
        }
    }
    return global_result;
}

// Questa versione elimina il collo di bottiglia
MatchResult parallel_recognition(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        throw std::runtime_error("Query più grande della serie oppure vuota.");
    }

    MatchResult global_result;
    long long search_range = series.values.size() - query.values.size();

    // Ogni thread avrà il suo risultato locale
    MatchResult local_result;

#pragma omp parallel private(local_result)
    {
        // Inizializza il risultato locale per questo thread
        local_result.min_sad = std::numeric_limits<double>::max();
        local_result.index = -1;

#pragma omp for nowait // nowait è opzionale qui ma può migliorare le performance
        for (long long i = 0; i <= search_range; ++i) {
            double current_sad = calculate_sad(series, query, i);
            if (current_sad < local_result.min_sad) {
                local_result.min_sad = current_sad;
                local_result.index = i;
            }
        }

        // Ora confrontiamo il risultato locale di questo thread con quello globale
        // Questa sezione critica è eseguita solo UNA VOLTA per thread, non ad ogni iterazione!
#pragma omp critical
        {
            if (local_result.min_sad < global_result.min_sad) {
                global_result = local_result;
            }
        }
    }

    return global_result;
}

MatchResult parallel_recognition_reduction(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        throw std::runtime_error("Query più grande della serie oppure vuota.");
    }

    long long search_range = series.values.size() - query.values.size();

    // 1. Creiamo un contenitore per i risultati parziali, uno per ogni thread
    int max_threads = omp_get_max_threads();
    std::vector<MatchResult> results_per_thread(max_threads);

    // 2. Inizia la regione parallela. Ogni thread lavora in modo indipendente.
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();

        // Inizializziamo il risultato locale per QUESTO thread
        results_per_thread[thread_id].min_sad = std::numeric_limits<double>::max();
        results_per_thread[thread_id].index = -1;

        // 3. Il lavoro del ciclo 'for' viene distribuito tra i thread
#pragma omp for
        for (long long i = 0; i <= search_range; ++i) {
            double current_sad = calculate_sad(series, query, i);

            // Ogni thread aggiorna SOLO la sua cella nell'array.
            // Non c'è conflitto (race condition) con altri thread.
            if (current_sad < results_per_thread[thread_id].min_sad) {
                results_per_thread[thread_id].min_sad = current_sad;
                results_per_thread[thread_id].index = i;
            }
        }
    } // --- Fine della regione parallela ---

    // 4. Riduzione Finale: il thread master trova il miglior risultato tra quelli parziali.
    // Questo ciclo è sequenziale ma estremamente veloce, perché itera solo sul numero di thread.
    MatchResult final_result;
    for (int i = 0; i < max_threads; ++i) {
        if (results_per_thread[i].min_sad < final_result.min_sad) {
            final_result = results_per_thread[i];
        }
    }

    return final_result;

}