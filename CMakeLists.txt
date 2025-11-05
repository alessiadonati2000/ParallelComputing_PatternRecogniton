cmake_minimum_required(VERSION 3.20)
project(ParallelComputing_PatternRecognition CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
# Svuota la variabile "incriminata".
# Questo impedisce a CMake di aggiungere automaticamente -fopenmp
# quando linki OpenMP::OpenMP_CXX.
set(OpenMP_CXX_FLAGS "")

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

# Aggiunge l'opzione specifica per MSVC per abilitare il supporto
# completo a OpenMP, incluse le riduzioni min/max.
if(MSVC)
    # FORZA LA RIMOZIONE dei flag errati (per GCC) che CLion
    # potrebbe aggiungere in automatico, causando il conflitto.
    string(REPLACE "-openmp" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "-fopenmp" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # Aggiunge il flag MSVC corretto
    target_compile_options(ParallelComputing_PatternRecognition PRIVATE "/openmp:llvm")
else()
    # Assicura che per altri compilatori (GCC/Clang) il flag corretto sia usato
    target_compile_options(ParallelComputing_PatternRecognition PRIVATE "-fopenmp")
endif()

target_link_libraries(ParallelComputing_PatternRecognition PRIVATE OpenMP::OpenMP_CXX)