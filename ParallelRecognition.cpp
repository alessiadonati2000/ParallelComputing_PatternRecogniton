#include <iostream>

#include "SequentialRecognition.h"
#include "ParallelRecognition.h"

MatchResult parallel_recognition_bottleneck(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query) {

    MatchResult best_global_match; // Unica variabile condivisa

    // parallel for divide il loop 'i' tra i thread
    // schedule(static, 1) assicura un chunk piccolo per massimizzare la contesa (per scopi didattici)
    #pragma omp parallel for schedule(static, 1)
    for (size_t i = 0; i < dataset.size(); ++i) {

        // 1. Ogni thread calcola il suo miglior match *locale* per la serie 'i'
        MatchResult series_result = sequential_recognition(dataset[i], query);

        // 2. Errore "Bottleneck": si accede alla risorsa condivisa ad *ogni* iterazione.
        // Tutti i thread si mettono in coda qui, uno alla volta, uccidendo il parallelismo.
        #pragma omp critical
        {
            if (series_result.min_sad < best_global_match.min_sad) {
                best_global_match.min_sad = series_result.min_sad;
                best_global_match.index = series_result.index;
            }
        }
    }
    return best_global_match;
}

MatchResult parallel_recognition_standard(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query) {

    MatchResult best_global_match; // Risultato finale condiviso

    #pragma omp parallel
    {
        // 1. Ogni thread crea il *proprio* miglior risultato locale
        MatchResult best_thread_match;

        // 2. Il loop 'for' è diviso tra i thread.
        // schedule(dynamic) è una buona scelta se le serie hanno lunghezze molto diverse.
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < dataset.size(); ++i) {

            MatchResult series_result = sequential_recognition(dataset[i], query);

            // 3. Ogni thread aggiorna solo la *propria* variabile locale.
            // Nessuna contesa, questo è velocissimo.
            if (series_result.min_sad < best_thread_match.min_sad) {
                best_thread_match.min_sad = series_result.min_sad;
                best_thread_match.index = series_result.index;
            }
        }

        // 4. Sezione Critica (Fine):
        // Solo *una volta* per thread, si confronta il miglior risultato
        // locale (best_thread_match) con quello globale (best_global_match).
        #pragma omp critical
        {
            if (best_thread_match.min_sad < best_global_match.min_sad) {
                best_global_match = best_thread_match;
            }
        }
    }

    return best_global_match;
}


/**
 * @brief Funzione helper per la riduzione custom (V3)
 * Confronta due risultati e restituisce il "minore" (quello con SAD minore).
 */
MatchResult min_sad_reducer(MatchResult a, MatchResult b) {
    if (a.min_sad < b.min_sad) {
        return a;
    } else {
        return b;
    }
}

#pragma omp declare reduction(min_sad_result : MatchResult : \
    omp_out = min_sad_reducer(omp_out, omp_in)) \
    initializer(omp_priv = MatchResult())

MatchResult parallel_recognition_reduction(const std::vector<std::vector<float>>& dataset, const std::vector<float>& query) {
    MatchResult best_global_match; // Questa variabile accumulerà il risultato

#pragma omp parallel for schedule(dynamic) reduction(min_sad_result:best_global_match)
    for (size_t i = 0; i < dataset.size(); ++i) {

        MatchResult series_result = sequential_recognition(dataset[i], query);

        // Se questo risultato è migliore del "best_global_match" *locale* del thread,
        // lo aggiorna. OpenMP gestisce l'unione di tutti i risultati
        // "best_global_match" locali alla fine.
        if (series_result.min_sad < best_global_match.min_sad) {
            best_global_match.min_sad = series_result.min_sad;
            best_global_match.index = series_result.index;
        }
    }

    // Non serve nessuna sezione critica! OpenMP fa tutto.
    return best_global_match;
}