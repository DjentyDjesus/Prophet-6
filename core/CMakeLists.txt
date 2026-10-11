cmake_minimum_required(VERSION 3.20)

add_library(prophet6_core STATIC
    src/Voice.cpp
    src/VoiceManager.cpp
    src/SynthEngine.cpp
    src/FactoryPresets.cpp
)

target_include_directories(prophet6_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(prophet6_core PUBLIC cxx_std_20)
