cmake_minimum_required(VERSION 3.22)

project(SoundBox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")

# Pull JUCE automatically — no manual install needed.
include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.4
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SoundBox
  PRODUCT_NAME "SoundBox"
  COMPANY_NAME "Dilara"
  BUNDLE_ID com.dilara.soundbox
  PLUGIN_MANUFACTURER_CODE Dlra
  PLUGIN_CODE Sbx1
  FORMATS AU Standalone
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE      # drops the .component into ~/Library/Audio/Plug-Ins/Components
)

target_sources(SoundBox PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
)

# embed the web UI (pedal HTML/CSS/JS + JUCE's JS bridge) into the binary
juce_add_binary_data(SoundBoxData
  SOURCES
    WebUI/index.html
    WebUI/js/juce.js
    WebUI/js/check_native_interop.js
)

target_compile_definitions(SoundBox PUBLIC
  JUCE_WEB_BROWSER=1
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SoundBox
  PRIVATE
    SoundBoxData
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
