cmake_minimum_required(VERSION 3.20)
project(Prophet6_Plugin VERSION 1.0.0 LANGUAGES C CXX)

# Fetch JUCE framework
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Prophet6
    COMPANY_NAME "Dave Smith / Sequential Tribute"
    BUNDLE_ID "com.dsi.prophet6"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE DSI_
    PLUGIN_CODE P6pt
    FORMATS Standalone VST3
    PRODUCT_NAME "Prophet-6"
)

target_sources(Prophet6 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(Prophet6 PRIVATE
    Source
)

target_link_libraries(Prophet6 PRIVATE
    prophet6_core
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_graphics
)

target_compile_definitions(Prophet6 PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
