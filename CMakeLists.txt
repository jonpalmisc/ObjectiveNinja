cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ObjectiveNinja)

# Add subdirectory for the Binary Ninja C++ API
add_subdirectory(vendor/api)

set(OBJ_NINJA_SOURCE
  source/AnalysisRecords.hpp
  source/CustomTypes.cpp
  source/CustomTypes.hpp
  source/GlobalState.cpp
  source/GlobalState.hpp
  source/OneShot.cpp
  source/OneShot.hpp
  source/Plugin.cpp
  source/StructureAnalyzer.cpp
  source/StructureAnalyzer.hpp
  source/Workflow.cpp
  source/Workflow.hpp)

# Used to enable some debugging actions, etc.
option(DEV_MODE "Enable developer features" OFF)

add_library(ObjectiveNinja SHARED ${OBJ_NINJA_SOURCE})
target_link_libraries(ObjectiveNinja binaryninjaapi)
target_compile_features(ObjectiveNinja PRIVATE cxx_std_17 c_std_99)
target_compile_definitions(ObjectiveNinja PRIVATE DEV_MODE=${DEV_MODE})

option(AUTO_INSTALL_PLUGIN "Install the plugin after building (macOS only)" OFF)
if(AUTO_INSTALL_PLUGIN)
  add_custom_command(TARGET ObjectiveNinja POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ObjectiveNinja>
    "~/Library/Application Support/Binary Ninja/plugins/")
endif()
