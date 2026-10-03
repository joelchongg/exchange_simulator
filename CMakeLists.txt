cmake_minimum_required(VERSION 3.25)

project(exchange_simulator
    VERSION 0.1.0
    LANGUAGES CXX)

option(EXSIM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(EXSIM_NATIVE "Tune for the build machine (-march=native)" OFF)
option(EXSIM_BUILD_TESTS "Build the test suite" ON)
set(EXSIM_SANITIZER "" CACHE STRING "Sanitizer build: '', 'address' (ASan+UBSan) or 'thread' (TSan)")
set_property(CACHE EXSIM_SANITIZER PROPERTY STRINGS "" address thread)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(warnings)
include(sanitizers)

# Sanitizers apply globally (before third-party deps) so everything, GoogleTest
# included, is instrumented. Mixing instrumented and uninstrumented code gives
# TSan false positives.
exsim_enable_sanitizer("${EXSIM_SANITIZER}")

# Every first-party target links this: standard, warnings, tuning.
add_library(exsim_options INTERFACE)
add_library(exsim::options ALIAS exsim_options)
target_compile_features(exsim_options INTERFACE cxx_std_23)
exsim_set_warnings(exsim_options)
if(EXSIM_NATIVE)
    target_compile_options(exsim_options INTERFACE -march=native)
endif()

# ---- Libraries: one per component, include root is src/ ----
add_library(exsim_utils INTERFACE)
add_library(exsim::utils ALIAS exsim_utils)
target_include_directories(exsim_utils INTERFACE src)
target_compile_features(exsim_utils INTERFACE cxx_std_23)

# ---- Executables ----

add_executable(exchange
    apps/exchange/main.cpp)
target_link_libraries(exchange PRIVATE exsim::utils exsim::options)

if(EXSIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
