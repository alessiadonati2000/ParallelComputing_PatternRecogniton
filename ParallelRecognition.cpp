#include "ParallelRecognition.h"
#include <omp.h>
#include <vector>

// Questa versione con omp critical genera un collo di bottiglia
/*MatchResult parallel_recognition(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        return {};
    }

    MatchResult global_result;
    long long search_range = series.values.size() - query.values.size();

#pragma omp parallel for
    for (long long i = 0; i <= search_range; ++i) {
        // Chiama la funzione dedicata per calcolare il SAD
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
}*/

// Questa versione elimina il collo di bottiglia
MatchResult parallel_recognition(const TimeSeries& series, const TimeSeries& query) {
    if (query.values.size() > series.values.size() || query.values.empty()) {
        return {};
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