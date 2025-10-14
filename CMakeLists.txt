cmake_minimum_required(VERSION 3.20)
project(ParallelComputing_PatternRecognition CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Aggiunge l'opzione specifica per MSVC per abilitare il supporto
# completo a OpenMP, incluse le riduzioni min/max.
if(MSVC)
    target_compile_options(ParallelComputing_PatternRecognition PRIVATE "/openmp:llvm")
endif()

target_link_libraries(ParallelComputing_PatternRecognition PRIVATE OpenMP::OpenMP_CXX)