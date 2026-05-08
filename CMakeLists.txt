cmake_minimum_required(VERSION 3.15)
project(kron
    VERSION 1.0.0
    DESCRIPTION "Kron is a multifunctional CLI tool for file manipulation and inspection."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Configuración de Compilación
add_compile_options(
    -Wall -Wextra -Wpedantic -Wconversion 
    -Wshadow -fsanitize=address,undefined 
    -g -fno-omit-frame-pointer
)
add_link_options(-fsanitize=address,undefined)

# 2. Definir la Librería del Núcleo (Sin el main)
set(LIB_SOURCES
    src/core-system/tokenization.cpp
    src/core-system/parsing.cpp
    src/core-system/executor.cpp
    src/core-system/validator.cpp
    src/error-register-information/error_hanlder.cpp
    src/command-register-information/command-information.cpp
    src/command-register-information/command-get-option.cpp
    src/command-register-information/command-handler-include/command-hanlder-cpp/list.cpp
    src/option-register-information/option-information.cpp
    src/option-register-information/option-get-option.cpp
    src/option-register-information/special-option/help-hanldler.cpp
    src/option-register-information/special-option/version-handler.cpp
)

add_library(kron_core STATIC ${LIB_SOURCES})

target_include_directories(kron_core PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/generated
)

# 3. Configurar archivos generados
# 3. Configurar archivos generados
configure_file(
    "${PROJECT_SOURCE_DIR}/include/special-option/version-option.hpp.in"
    "${PROJECT_BINARY_DIR}/generated/special-option/version-option.hpp" # Añadido special-option/
)

# 4. El Ejecutable Principal
add_executable(kron src/kls-main.cpp)
target_link_libraries(kron PRIVATE kron_core)

# --- 5. INFRAESTRUCTURA DE TESTS (MODIFICACIÓN AQUÍ) ---
# Habilitamos CTest a nivel global
enable_testing()

# Añadimos el subdirectorio de tests. 
# Esto buscará el archivo 'tests/CMakeLists.txt'
add_subdirectory(tests)

# 6. Instalación
install(TARGETS kron RUNTIME DESTINATION bin)
