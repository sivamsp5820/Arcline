cmake_minimum_required(VERSION 3.22)

project(ARCLINE 
    VERSION 0.1.0 
    DESCRIPTION "Professional, creative-first desktop Digital Audio Workstation"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Destination output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ------------------------------------------------------------------------------
# Compiler Diagnostics, Static Analysis & Sanitizers
# ------------------------------------------------------------------------------
option(ARCLINE_ENABLE_ASAN "Enable AddressSanitizer (ASan) memory safety checks" OFF)
option(ARCLINE_STRICT_WARNINGS "Enable compiler warning diagnostics" ON)

if(ARCLINE_ENABLE_ASAN)
    message(STATUS "ARCLINE: AddressSanitizer (ASan) enabled")
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(ARCLINE_STRICT_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /wd4100 /wd4244 /wd4458 /wd4996 /wd4127)
    else()
        add_compile_options(-Wall -Wextra -Wno-unused-parameter)
    endif()
endif()

# ------------------------------------------------------------------------------
# JUCE Framework Dependency Configuration
# ------------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    message(STATUS "Configuring local JUCE 8 submodule from libs/JUCE")
    add_subdirectory(libs/JUCE)
else()
    message(STATUS "Fetching pinned JUCE 8.0.6 via CMake FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ------------------------------------------------------------------------------
# Source Files Definition
# ------------------------------------------------------------------------------
set(ARCLINE_CORE_SOURCES
    src/domain/Track.cpp
    src/domain/Region.cpp
    src/domain/Project.cpp
    src/domain/Preferences.cpp
    src/domain/RecentProjects.cpp
    src/domain/UndoHistory.cpp
    src/core/Logger.cpp
    src/core/CommandRegistry.cpp
    src/core/Localization.cpp
    src/app/NavigationController.cpp
    src/app/AppContext.cpp
    src/infrastructure/FileSystem.cpp
    src/infrastructure/ProjectRepository.cpp
    src/infrastructure/PreferencesRepository.cpp
    src/audio/engine/AudioEngine.cpp
    src/audio/devices/AudioDeviceService.cpp
    src/audio/transport/TransportController.cpp
)

set(ARCLINE_UI_SOURCES
    src/ui/theme/SonoraTheme.cpp
    src/ui/theme/SonoraLookAndFeel.cpp
    src/ui/components/HeaderBar.cpp
    src/ui/components/TransportBar.cpp
    src/ui/components/InspectorPanel.cpp
    src/ui/components/LowerEditorPanel.cpp
    src/ui/components/LibraryPanel.cpp
    src/ui/components/MainLayoutComponent.cpp
    src/ui/workspaces/StartStudioView.cpp
    src/ui/workspaces/ArrangementView.cpp
    src/ui/dialogs/NewProjectDialog.cpp
    src/ui/dialogs/PreferencesDialog.cpp
    src/ui/dialogs/CommandPaletteOverlay.cpp
    src/app/MainWindow.cpp
    src/app/SonoraApplication.cpp
)

# ------------------------------------------------------------------------------
# Main Application Target (Native Desktop Executable)
# ------------------------------------------------------------------------------
juce_add_gui_app(Arcline
    PRODUCT_NAME "ARCLINE"
    VERSION "0.1.0"
    BUNDLE_ID "com.arcline.daw"
    COMPANY_NAME "ARCLINE Audio"
)

target_sources(Arcline PRIVATE
    src/main.cpp
    ${ARCLINE_CORE_SOURCES}
    ${ARCLINE_UI_SOURCES}
)

target_include_directories(Arcline PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Arcline PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="ARCLINE"
    JUCE_APPLICATION_VERSION_STRING="0.1.0"
)

target_link_libraries(Arcline PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
)

# Legacy alias target for backward compatibility
add_custom_target(Sonora DEPENDS Arcline)

# ------------------------------------------------------------------------------
# Automated Tests Target
# ------------------------------------------------------------------------------
option(ARCLINE_BUILD_TESTS "Build automated unit and integration tests" ON)
if(DEFINED SONORA_BUILD_TESTS)
    set(ARCLINE_BUILD_TESTS ${SONORA_BUILD_TESTS})
endif()

if(ARCLINE_BUILD_TESTS)
    enable_testing()

    add_executable(ArclineTests
        tests/TestRunner.cpp
        tests/unit/ProjectTests.cpp
        tests/unit/UndoHistoryTests.cpp
        tests/unit/PreferencesTests.cpp
        tests/unit/RecentProjectsTests.cpp
        tests/unit/CommandRegistryTests.cpp
        tests/unit/NavigationTests.cpp
        tests/unit/LocalizationTests.cpp
        tests/unit/LayoutVerificationTests.cpp
        tests/unit/ProjectFormatDurabilityTests.cpp
        tests/integration/ProjectLifecycleTests.cpp
        tests/unit/AudioEngineLifecycleTests.cpp
        tests/unit/AudioClockTests.cpp
        tests/unit/TestToneSignalTests.cpp
        tests/unit/MetronomeTests.cpp
        tests/unit/DeviceSettingsTests.cpp
        ${ARCLINE_CORE_SOURCES}
    )

    target_include_directories(ArclineTests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    target_compile_definitions(ArclineTests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
    )

    if(MSVC)
        target_compile_options(ArclineTests PRIVATE /UNDEBUG)
    else()
        target_compile_options(ArclineTests PRIVATE -UNDEBUG)
    endif()

    target_link_libraries(ArclineTests PRIVATE
        juce::juce_core
        juce::juce_events
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
    )

    add_custom_target(SonoraTests DEPENDS ArclineTests)

    add_test(NAME ArclineAutomatedTestSuite COMMAND ArclineTests)
endif()
