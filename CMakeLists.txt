cmake_minimum_required(VERSION 3.20)
project(FlashSearch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. Creamos una librería de interfaz para las configuraciones compartidas
add_library(flash_config INTERFACE)

# 2. Seteamos los includes una sola vez para todos
target_include_directories(flash_config INTERFACE 
    extern 
    src/common
)

# 3. Flags de compilación inteligentes (Modern CMake style)
if(MSVC)
    target_compile_options(flash_config INTERFACE /W4 /Z7) # /Z7 para debug info en el .obj
else()
    # Para tu GCC 15.2.0
    target_compile_options(flash_config INTERFACE 
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -march=native>
        -Wall -Wextra # Un poco de disciplina no te viene mal
    )
endif()

# Fuentes comunes
set(COMMON_SOURCES src/common/flash_search.cpp src/common/load.cpp)

# 4. Definimos los ejecutables y vinculamos la configuración
# Esto es mucho más limpio que repetir los targets
set(TARGETS FlashAnalistics FlashBenchmarks FlashUnitTest)

add_executable(FlashAnalistics src/analistics.cpp ${COMMON_SOURCES})
add_executable(FlashBenchmarks src/benchmarks.cpp ${COMMON_SOURCES})
add_executable(FlashUnitTest src/unit-test.cpp ${COMMON_SOURCES})

foreach(target IN LISTS TARGETS)
    target_link_libraries(${target} PRIVATE flash_config)
endforeach()