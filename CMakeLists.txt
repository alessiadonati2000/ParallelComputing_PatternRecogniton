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

target_link_libraries(ParallelComputing_PatternRecognition PRIVATE OpenMP::OpenMP_CXX)