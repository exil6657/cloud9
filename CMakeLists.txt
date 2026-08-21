cmake_minimum_required(VERSION 3.20)

project(Cloud9 VERSION 0.1.0 LANGUAGES CXX)

option(CLOUD9_BUILD_DEMO "Build the host-independent Cloud9 demo" ON)
option(CLOUD9_BUILD_TESTS "Build Cloud9 unit tests" ON)
option(CLOUD9_ENABLE_ZLIB "Enable gzip-compressed NBT support when zlib is available" ON)
option(CLOUD9_BUILD_WINDOWS_DLL "Build the optional, non-injecting Windows DLL shell" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cloud9_core STATIC
    src/client/CapabilityRegistry.cpp
    src/client/Client.cpp
    src/client/CommandManager.cpp
    src/client/ConfigManager.cpp
    src/client/FriendManager.cpp
    src/gui/SchematicPanel.cpp
    src/gui/ThemeManager.cpp
    src/modules/hud/ArrayList.cpp
    src/modules/hud/Clock.cpp
    src/modules/hud/Coordinates.cpp
    src/modules/hud/CPSCounter.cpp
    src/modules/hud/FPSCounter.cpp
    src/modules/hud/Keystrokes.cpp
    src/modules/hud/PingDisplay.cpp
    src/modules/hud/SessionInfo.cpp
    src/modules/hud/Speedometer.cpp
    src/modules/hud/Watermark.cpp
    src/events/EventManager.cpp
    src/hooks/HookManager.cpp
    src/hooks/SignatureScanner.cpp
    src/modules/Module.cpp
    src/modules/ModuleManager.cpp
    src/modules/FeatureCatalog.cpp
    src/modules/visual/Fullbright.cpp
    src/render/ScreenProjection.cpp
    src/render/WorldRenderer.cpp
    src/schematic/BlockMapping.cpp
    src/schematic/NBTParser.cpp
    src/schematic/Schematic.cpp
    src/schematic/SchematicLoader.cpp
    src/schematic/SchematicManager.cpp
    src/schematic/SchematicRenderer.cpp
    src/schematic/SchematicUI.cpp
    src/schematic/SchematicVerifier.cpp
    src/utils/Json.cpp
    src/utils/Logger.cpp
)

target_include_directories(cloud9_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(cloud9_core PUBLIC cxx_std_20)

target_compile_definitions(cloud9_core PUBLIC
    CLOUD9_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CLOUD9_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CLOUD9_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if (MSVC)
    target_compile_options(cloud9_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(cloud9_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if (CLOUD9_ENABLE_ZLIB)
    find_package(ZLIB QUIET)
    if (ZLIB_FOUND)
        target_link_libraries(cloud9_core PRIVATE ZLIB::ZLIB)
        target_compile_definitions(cloud9_core PRIVATE CLOUD9_HAS_ZLIB=1)
    else()
        message(STATUS "zlib not found: gzip NBT loading will report a clear runtime error")
    endif()
endif()

if (CLOUD9_BUILD_DEMO)
    add_executable(cloud9_demo src/main.cpp)
    target_link_libraries(cloud9_demo PRIVATE cloud9_core)
endif()

if (CLOUD9_BUILD_TESTS)
    enable_testing()
    add_executable(cloud9_tests tests/smoke.cpp)
    target_link_libraries(cloud9_tests PRIVATE cloud9_core)
    add_test(NAME cloud9_smoke COMMAND cloud9_tests)
endif()

# This is deliberately only a host shell. It does not install hooks, scan a
# remote process, or alter Minecraft memory. A future Windows integration must
# supply an explicit, reviewed platform adapter behind these interfaces.
if (WIN32 AND CLOUD9_BUILD_WINDOWS_DLL)
    add_library(Cloud9Shell SHARED src/platform/windows/DllMain.cpp)
    target_link_libraries(Cloud9Shell PRIVATE cloud9_core)
    target_compile_definitions(Cloud9Shell PRIVATE CLOUD9_WINDOWS_SHELL=1)
endif()
