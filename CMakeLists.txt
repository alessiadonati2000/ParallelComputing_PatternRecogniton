cmake_minimum_required(VERSION 3.20)
project(ParallelComputing_PatternRecognition CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Trova OpenMP (lo usiamo solo per 'MSVC')
find_package(OpenMP REQUIRED)

add_executable(
        ParallelComputing_PatternRecognition
        main.cpp
        CSVreader.cpp
        CSVreader.h
        SAD.cpp
        SAD.h
        SequentialRecognition.cpp
        SequentialRecognition.h
        ParallelRecognition.cpp
        ParallelRecognition.h
)

# 2. Logica esplicita per compilatore
if(MSVC)
    # --- Per Visual Studio ---
    # Aggiungi l'opzione di compilazione
    target_compile_options(ParallelComputing_PatternRecognition PRIVATE "/openmp:llvm")
    # Lascia che il target gestisca il linking
    target_link_libraries(ParallelComputing_PatternRecognition PRIVATE OpenMP::OpenMP_CXX)

else()
    # --- Per GCC (MinGW) o Clang ---
    # La flag "-fopenmp" serve SIA per compilare CHE per linkare.
    # Dobbiamo aggiungerla in entrambi i posti.

    # 2a. Aggiungila alle opzioni di COMPILAZIONE
    target_compile_options(ParallelComputing_PatternRecognition PRIVATE "-fopenmp")

    # 2b. Aggiungila ESPLICITAMENTE alle opzioni di LINKING
    # Questa è la riga chiave che mancava e che risolve l'errore
    target_link_options(ParallelComputing_PatternRecognition PRIVATE "-fopenmp")

    # Per sicurezza, lasciamo anche il target. Non fa male.
    target_link_libraries(ParallelComputing_PatternRecognition PRIVATE OpenMP::OpenMP_CXX)

endif()