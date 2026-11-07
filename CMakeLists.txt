cmake_minimum_required(VERSION 3.24)
project(AlgoVisualizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ALGOVIZ_BUILD_APP "Build the SFML desktop app" ON)
option(ALGOVIZ_BUILD_TESTS "Build the unit tests" ON)
option(ALGOVIZ_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

include(FetchContent)

# ---------------------------------------------------------------- warnings
add_library(algoviz_warnings INTERFACE)
if(MSVC)
  target_compile_options(algoviz_warnings INTERFACE /W4 $<$<BOOL:${ALGOVIZ_WARNINGS_AS_ERRORS}>:/WX>)
else()
  target_compile_options(algoviz_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    $<$<BOOL:${ALGOVIZ_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

# ---------------------------------------------------------------- core (no deps)
add_library(algoviz_core
  core/src/grid.cpp
  core/src/search.cpp
  core/src/maze.cpp)
target_include_directories(algoviz_core PUBLIC core/include)
target_link_libraries(algoviz_core PRIVATE algoviz_warnings)

# ---------------------------------------------------------------- web (Emscripten)
if(EMSCRIPTEN)
  add_executable(algoviz_web web/bindings.cpp)
  target_link_libraries(algoviz_web PRIVATE algoviz_core algoviz_warnings)
  set_target_properties(algoviz_web PROPERTIES OUTPUT_NAME "algoviz")
  target_link_options(algoviz_web PRIVATE
    -lembind -O3
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createAlgoViz
    -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1 -sFILESYSTEM=0)
  return()
endif()

# ---------------------------------------------------------------- desktop app (SFML 3)
if(ALGOVIZ_BUILD_APP)
  find_package(SFML 3 COMPONENTS Graphics Window System QUIET)
  if(NOT SFML_FOUND)
    message(STATUS "System SFML 3 not found, fetching SFML 3.0.2")
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SFML
      GIT_REPOSITORY https://github.com/SFML/SFML.git
      GIT_TAG 3.0.2
      GIT_SHALLOW ON
      EXCLUDE_FROM_ALL
      SYSTEM)
    FetchContent_MakeAvailable(SFML)
  endif()
  add_executable(algoviz app/main.cpp app/app.cpp app/pixel_font.cpp)
  target_compile_features(algoviz PRIVATE cxx_std_20)
  target_link_libraries(algoviz PRIVATE algoviz_core algoviz_warnings SFML::Graphics)
endif()

# ---------------------------------------------------------------- tests
if(ALGOVIZ_BUILD_TESTS)
  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW ON
    SYSTEM)
  FetchContent_MakeAvailable(Catch2)
  enable_testing()
  add_executable(algoviz_tests
    tests/test_grid.cpp
    tests/test_search.cpp
    tests/test_maze.cpp)
  target_link_libraries(algoviz_tests PRIVATE algoviz_core algoviz_warnings Catch2::Catch2WithMain)
  include(CTest)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(Catch)
  catch_discover_tests(algoviz_tests)
endif()
