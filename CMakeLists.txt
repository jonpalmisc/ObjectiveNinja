cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ObjectiveNinja)

# Add subdirectory for the Binary Ninja C++ API
add_subdirectory(vendor/api)

set(CORE_SOURCE
  Include/ObjectiveNinjaCore/Analyzers/CFStringAnalyzer.h
  Include/ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h
  Include/ObjectiveNinjaCore/AbstractFile.h
  Include/ObjectiveNinjaCore/AnalysisInfo.h
  Include/ObjectiveNinjaCore/Analyzer.h
  Core/Analyzers/CFStringAnalyzer.cpp
  Core/Analyzers/ClassAnalyzer.cpp
  Core/Analyzer.cpp)

add_library(ObjectiveNinjaCore STATIC ${CORE_SOURCE})
target_compile_features(ObjectiveNinjaCore PRIVATE cxx_std_17)
target_include_directories(ObjectiveNinjaCore PUBLIC Include)

set(PLUGIN_SOURCE
  Plugin/Support/BinaryViewFile.h
  Plugin/Support/BinaryViewFile.cpp
  Plugin/AnalysisRecords.hpp
  Plugin/CustomTypes.cpp
  Plugin/CustomTypes.hpp
  Plugin/DataRenderers.hpp
  Plugin/DataRenderers.cpp
  Plugin/GlobalState.cpp
  Plugin/GlobalState.hpp
  Plugin/InfoHandler.h
  Plugin/InfoHandler.cpp
  Plugin/OneShot.cpp
  Plugin/OneShot.hpp
  Plugin/Plugin.cpp
  Plugin/StructureAnalyzer.cpp
  Plugin/StructureAnalyzer.hpp
  Plugin/Workflow.cpp
  Plugin/Workflow.hpp)

# Used to enable some debugging actions, etc.
option(DEV_MODE "Enable developer features" OFF)

add_library(ObjectiveNinja SHARED ${PLUGIN_SOURCE})
target_link_libraries(ObjectiveNinja ObjectiveNinjaCore binaryninjaapi)
target_compile_features(ObjectiveNinja PRIVATE cxx_std_17 c_std_99)
target_compile_definitions(ObjectiveNinja PRIVATE DEV_MODE=${DEV_MODE})

option(AUTO_INSTALL_PLUGIN "Install the plugin after building (macOS only)" OFF)
if(AUTO_INSTALL_PLUGIN)
  add_custom_command(TARGET ObjectiveNinja POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ObjectiveNinja>
    "~/Library/Application Support/Binary Ninja/plugins/")
endif()

add_custom_target(format
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND clang-format -i --style=WebKit ${CORE_SOURCE} ${PLUGIN_SOURCE})
