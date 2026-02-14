cmake_minimum_required(VERSION 3.20) # Versión mínima decente para soporte moderno
project(FlashSearch VERSION 1.0 LANGUAGES CXX)

# El toque mágico para C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Para mantenerte puro y no usar extensiones de GNU
include_directories(extern/)
include_directories(src/common)
# Definir el ejecutable
set(COMMON_SOURCES 
    src/common/flash_search.cpp
)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_BUILD_TYPE Debug)
add_executable(${PROJECT_NAME} src/main.cpp ${COMMON_SOURCES})

# Si necesitas incluir carpetas (como las de ICU que mencionaste antes)
# target_include_directories(${PROJECT_NAME} PRIVATE include)