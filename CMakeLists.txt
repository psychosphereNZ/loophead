cmake_minimum_required(VERSION 3.22)
project(LOOPHEAD VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: lokalen Pfad nutzen (-DJUCE_PATH=...) oder automatisch von GitHub laden
set(JUCE_PATH "" CACHE PATH "Optionaler Pfad zu einem lokalen JUCE-Checkout")
if(JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Loophead
    COMPANY_NAME             "G Audio"
    BUNDLE_ID                com.gaudio.loophead
    PLUGIN_MANUFACTURER_CODE Gaud
    PLUGIN_CODE              Lph1
    FORMATS                  VST3 AU Standalone
    PRODUCT_NAME             "LOOPHEAD"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    VST3_CATEGORIES          Fx Delay
    AU_MAIN_TYPE             kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD  FALSE)

target_sources(Loophead PRIVATE
       PluginProcessor.cpp
       PluginEditor.cpp)

target_compile_definitions(Loophead PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Loophead
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
