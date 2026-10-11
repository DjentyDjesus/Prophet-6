cmake_minimum_required(VERSION 3.20)
project(Prophet6 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /Oi /Ot /fp:fast /W4)
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

add_subdirectory(core)
add_subdirectory(tests)
add_subdirectory(targets/desktop)
