cmake_minimum_required(VERSION 3.22) # Versión mínima para soporte robusto de C++23
project(KronList 
    VERSION 0.0.1 
    LANGUAGES CXX
)

# 1. GENERACIÓN DE compile_commands.json
# Esto es vital para que clangd en NvChad detecte headers y flags correctamente.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. CONFIGURACIÓN DEL ESTÁNDAR
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Evita extensiones de compilador (como GNU++) para asegurar portabilidad

# 3. RECOLECCIÓN DE FUENTES
# Se recomienda listar archivos explícitamente para mayor control y evitar problemas con el caché de CMake.
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

# 4. DEFINICIÓN DEL EJECUTABLE
add_executable(kron ${SOURCES})

# 5. GESTIÓN DE INCLUDES (DIRECCIONES DE CABECERAS)
# 'PUBLIC' permite que si este target fuera una librería, otros targets hereden estos paths.
target_include_directories(kron PUBLIC 
    "src"
    "general-utilities"
)

# 6. FLAGS DE COMPILACIÓN: OPTIMIZACIÓN Y SEGURIDAD
if(MSVC)
    target_compile_options(kron PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(kron PRIVATE 
        -Wall -Wextra -Wpedantic    # Advertencias estrictas
        -Wshadow -Wconversion      # Seguridad en tipos y nombres
        -O3                        # Optimización máxima
        -march=native              # Optimización específica para tu CPU
        -fstack-protector-strong   # Protección contra buffer overflow (Security mantra)
        -D_FORTIFY_SOURCE=2        # Detección de desbordamientos en tiempo de ejecución
    )
endif()

# 7. LINK-TIME OPTIMIZATION (LTO / IPO)
# Crucial para tu mantra de optimización: permite optimizar entre diferentes archivos .cpp
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
if(lto_supported)
    set_target_properties(kron PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO habilitado para máxima optimización de binario.")
endif()

# 8. TESTS (Opcional pero recomendado)
enable_testing()
# Aquí podrías añadir: add_subdirectory(test)
