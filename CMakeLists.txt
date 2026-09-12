cmake_minimum_required(VERSION 3.15)
project(PlataformaStreamingPeliculas CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(include)

add_executable(plataforma_peliculas
    src/main.cpp
    src/CSVReader.cpp
    src/TextUtils.cpp
    src/SuffixTrie.cpp
    src/TextIndex.cpp
    src/SearchEngine.cpp
    src/UserProfile.cpp
)
