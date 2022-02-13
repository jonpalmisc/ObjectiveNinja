cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ObjectiveNinja)

# Default the build type to "release with debug info".
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Get the current Git branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the current Git commit hash
execute_process(
  COMMAND git describe --always --dirty
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add subdirectory for the Binary Ninja C++ API; skip linking against the UI
# library since it isn't needed for this plugin
set(HEADLESS ON CACHE BOOL "Skip building UI functionality")
add_subdirectory(Vendor/BinaryNinjaAPI)

# Core library -----------------------------------------------------------------

set(CORE_SOURCE
  Include/ObjectiveNinjaCore/Analyzers/CFStringAnalyzer.h
  Include/ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h
  Include/ObjectiveNinjaCore/Analyzers/SelectorAnalyzer.h
  Include/ObjectiveNinjaCore/Support/BinaryViewFile.h
  Include/ObjectiveNinjaCore/ABI.h
  Include/ObjectiveNinjaCore/AbstractFile.h
  Include/ObjectiveNinjaCore/AnalysisInfo.h
  Include/ObjectiveNinjaCore/AnalysisProvider.h
  Include/ObjectiveNinjaCore/Analyzer.h
  Include/ObjectiveNinjaCore/TypeParser.h
  Core/Analyzers/CFStringAnalyzer.cpp
  Core/Analyzers/ClassAnalyzer.cpp
  Core/Analyzers/SelectorAnalyzer.cpp
  Core/Support/BinaryViewFile.cpp
  Core/ABI.cpp
  Core/AbstractFile.cpp
  Core/AnalysisInfo.cpp
  Core/AnalysisProvider.cpp
  Core/Analyzer.cpp
  Core/TypeParser.cpp)

add_library(ObjectiveNinjaCore STATIC ${CORE_SOURCE})
target_compile_features(ObjectiveNinjaCore PRIVATE cxx_std_17)
target_include_directories(ObjectiveNinjaCore PUBLIC Include)

# Option to specify whether the core library should be built with support
# modules for integrating with Binary Ninja. Enabled by default, but able to be
# turned off so the core library can be built without the Binary Ninja
# dependency; an alternative AbstractFile implementation will be needed.
option(ONC_BN_SUPPORT "Build core library support for Binary Ninja" ON)
if(ONC_BN_SUPPORT)
  target_compile_definitions(ObjectiveNinjaCore PUBLIC ONC_BN_SUPPORT=1)
  target_link_libraries(ObjectiveNinjaCore PRIVATE binaryninjaapi)
endif()

# Binary Ninja plugin ----------------------------------------------------------

configure_file(Plugin/BuildInfo.h.in ${CMAKE_SOURCE_DIR}/Plugin/BuildInfo.h @ONLY)

set(PLUGIN_SOURCE
  Plugin/Commands.h
  Plugin/Commands.cpp
  Plugin/CustomTypes.h
  Plugin/CustomTypes.cpp
  Plugin/DataRenderers.h
  Plugin/DataRenderers.cpp
  Plugin/GlobalState.h
  Plugin/GlobalState.cpp
  Plugin/InfoHandler.h
  Plugin/InfoHandler.cpp
  Plugin/Plugin.cpp
  Plugin/Workflow/Workflow.h
  Plugin/Workflow/Workflow.cpp)

# Used to enable some debugging actions, etc.
option(DEV_MODE "Enable developer features" OFF)

add_library(ObjectiveNinja SHARED ${PLUGIN_SOURCE})
target_link_libraries(ObjectiveNinja ObjectiveNinjaCore binaryninjaapi)
target_compile_features(ObjectiveNinja PRIVATE cxx_std_17 c_std_99)
target_compile_definitions(ObjectiveNinja PRIVATE
  DEV_MODE=${DEV_MODE})

# Library targets linking against the Binary Ninja API need to be compiled with
# position-independent code on Linux.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_compile_options(ObjectiveNinjaCore PRIVATE "-fPIC")
  target_compile_options(ObjectiveNinja PRIVATE "-fPIC")
endif()

# Option to automatically install the plugin after successful builds.
option(AUTO_INSTALL_PLUGIN "Install the plugin after building (macOS only)" OFF)
if(AUTO_INSTALL_PLUGIN)
  add_custom_command(TARGET ObjectiveNinja POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ObjectiveNinja>
    "~/Library/Application Support/Binary Ninja/plugins/")
endif()

# Internal tools ---------------------------------------------------------------

set(DUMP_INFO_SOURCE
  Tools/DumpInfo.cpp)

add_executable(DumpInfo ${DUMP_INFO_SOURCE})
target_link_libraries(DumpInfo PRIVATE ObjectiveNinjaCore binaryninjaapi)
target_compile_features(DumpInfo PRIVATE cxx_std_17 c_std_99)

set(TOOLS_SOURCE ${DUMP_INFO_SOURCE})

# Extra targets ----------------------------------------------------------------

add_custom_target(format
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND clang-format -i --style=WebKit
  ${CORE_SOURCE} ${PLUGIN_SOURCE} ${TOOLS_SOURCE})

unset(CMAKE_BUILD_TYPE CACHE)
