cmake_minimum_required(VERSION 3.22)

project(LumoraSynth VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
endif()

option(LUMORA_BUILD_TESTS "Build the offline render test" ON)

# JUCE: either a local checkout (-DJUCE_DIR=/path/to/JUCE) or fetched automatically.
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

set(LUMORA_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND LUMORA_FORMATS AU)
endif()

juce_add_plugin(LumoraSynth
    VERSION 1.0.0
    COMPANY_NAME "Lumora"
    COMPANY_WEBSITE "https://github.com/danielpluta2002-alt"
    PLUGIN_MANUFACTURER_CODE Lmra
    PLUGIN_CODE LmS1
    FORMATS ${LUMORA_FORMATS}
    PRODUCT_NAME "Lumora Synth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice)

set(LUMORA_SOURCES
    Source/Parameters.cpp
    Source/SynthParams.cpp
    Source/Presets.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/dsp/Voice.cpp
    Source/dsp/SynthEngine.cpp
    Source/dsp/Arpeggiator.cpp
    Source/dsp/Effects.cpp
    Source/gui/LookAndFeel.cpp
    Source/gui/Controls.cpp)

target_sources(LumoraSynth PRIVATE ${LUMORA_SOURCES})

target_include_directories(LumoraSynth PRIVATE Source)

target_compile_definitions(LumoraSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(LumoraSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(LUMORA_BUILD_TESTS)
    juce_add_console_app(LumoraRenderTest PRODUCT_NAME "LumoraRenderTest")
    target_sources(LumoraRenderTest PRIVATE tests/RenderTest.cpp ${LUMORA_SOURCES})
    target_include_directories(LumoraRenderTest PRIVATE Source)
    target_compile_definitions(LumoraRenderTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(LumoraRenderTest PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
