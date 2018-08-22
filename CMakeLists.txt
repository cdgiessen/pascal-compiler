cmake_minimum_required (VERSION 3.7)
project (pascal_compiler VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

add_subdirectory(fmt)

add_executable(lexer src/lexer.cpp) 