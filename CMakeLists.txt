cmake_minimum_required(VERSION 3.15)
project(kls
    VERSION 1.0.0
    DESCRIPTION "kls is a security-focused directory listing tool."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Configuración de Compilación
add_compile_options(
    -Wall -Wextra -Wpedantic -Wconversion 
    -Wshadow -fsanitize=address,undefined 
    -g -fno-omit-frame-pointer
)
add_link_options(-fsanitize=address,undefined)

# 2. Fuentes del Core
set(CORE_SOURCES
    src/core-system/tokenization.cpp
    src/core-system/parsing.cpp
    src/core-system/validator.cpp
    src/core-system/executor.cpp
    src/error-register-information/error_hanlder.cpp
    src/option-register-information/option-get-option.cpp
    src/option-register-information/option-information.cpp
    src/option-register-information/special-option/help-hanldler.cpp
    src/option-register-information/special-option/version-handler.cpp
    src/command-register-information/command-handler-include/command-hanlder-cpp/list.cpp
)

# 3. Biblioteca Core
add_library(kls_core STATIC ${CORE_SOURCES})
target_include_directories(kls_core PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/generated
)

# 4. Configurar archivos generados
configure_file(
    "${PROJECT_SOURCE_DIR}/include/special-option/version-option.hpp.in"
    "${PROJECT_BINARY_DIR}/generated/special-option/version-option.hpp"
)

# 5. El Ejecutable Principal
add_executable(kls src/kls-main.cpp)
target_link_libraries(kls PRIVATE kls_core)

# 6. Infraestructura de Tests
enable_testing()
add_subdirectory(tests)

# 7. Instalación
install(TARGETS kls RUNTIME DESTINATION bin)
