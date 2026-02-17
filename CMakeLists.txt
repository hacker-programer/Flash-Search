cmake_minimum_required(VERSION 3.15...3.30)
project(FlashSearch VERSION 1.0 LANGUAGES CXX)

# C++23: Modernidad absoluta
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Forzar que si no se define un tipo de build, sea Release (porque somos pros)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# --- CONFIGURACIÓN DE DEPENDENCIAS (Google Benchmark) ---
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  google_benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG         v1.8.3
)
FetchContent_MakeAvailable(google_benchmark)

# --- LIBRERÍAS LOCALES (GTest/GMock) ---
add_library(gtest STATIC IMPORTED)
set_target_properties(gtest PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libgtest.a")
add_library(gmock STATIC IMPORTED)
set_target_properties(gmock PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libgmock.a")
add_library(gmock_main STATIC IMPORTED)
set_target_properties(gmock_main PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libgmock_main.a")

# --- CONFIGURACIÓN COMPARTIDA ---
add_library(flash_config INTERFACE)
target_include_directories(flash_config INTERFACE src/common)

# Definiciones globales
target_compile_definitions(flash_config INTERFACE 
    BENCHMARK_STATIC_DEFINE
    BENCHMARK_INCLUDE_VECTOR=true
    BENCHMARK_INCLUDE_MAP=true
    BENCHMARK_INCLUDE_SET=true
    BENCHMARK_INCLUDE_UNMAP=true
    BENCHMARK_INCLUDE_UNSET=true
    BENCHMARK_INCLUDE_FAST_SEARCH=true
    BENCHMARK_INCLUDE_SEARCH=true
)

# Flags de compilación inteligentes
if(MSVC)
    target_compile_options(flash_config INTERFACE /W4 /Z7 /std:c++latest)
    target_compile_options(flash_config INTERFACE $<$<CONFIG:Release>:/O2 /Oi /Ot /GL>)
else()
    target_compile_options(flash_config INTERFACE -Wall -Wextra)
    
    # Debug: Lento pero detallado para el debugger
    target_compile_options(flash_config INTERFACE 
        $<$<CONFIG:Debug>:-g3 -O0 -fno-omit-frame-pointer>
    )
    
    # Release: ¡A toda máquina!
    target_compile_options(flash_config INTERFACE 
        $<$<CONFIG:Release>:-O3 -march=native -flto -DNDEBUG>
    )
    
    # Para que VTune pueda ver los símbolos en Release
    target_compile_options(flash_config INTERFACE 
        $<$<CONFIG:RelWithDebInfo>:-O3 -march=native -g1>
    )
endif()

# LTO (Link Time Optimization) también necesita flags en el linker
if(NOT MSVC)
    target_link_options(flash_config INTERFACE $<$<CONFIG:Release>:-flto>)
endif()

set(COMMON_SOURCES src/common/flash_search.cpp src/common/load.cpp)

# --- EJECUTABLES ---

# 1. Analistics
add_executable(FlashAnalistics src/analistics.cpp ${COMMON_SOURCES})
target_link_libraries(FlashAnalistics PRIVATE flash_config)

# 2. Benchmarks (Acá está la magia)
add_executable(FlashBenchmarks src/benchmarks.cpp ${COMMON_SOURCES})
target_link_libraries(FlashBenchmarks PRIVATE 
    flash_config 
    benchmark::benchmark 
    shlwapi 
    version 
    pthread
)

# 3. Unit Tests
add_executable(FlashUnitTest src/unit-test.cpp ${COMMON_SOURCES})
target_link_libraries(FlashUnitTest PRIVATE flash_config gmock_main gmock gtest pthread)