cmake_minimum_required(VERSION 3.15)
project(kron
    VERSION 0.1.0
    DESCRIPTION "Kron is a multifunctional CLI tool for file manipulation and inspection."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Wformat=2
    -fsanitize=address,undefined  # ← ASan + UBSan
    -g                            # ← símbolos de debug
    -fno-omit-frame-pointer       # ← stack traces legibles
)

add_link_options(
    -fsanitize=address,undefined  # ← necesario también en el linker
)

file(GLOB_RECURSE KRON_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(kron ${KRON_SOURCES})

target_include_directories(kron PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(kron PRIVATE
    KRON_VERSION="${PROJECT_VERSION}"
)

install(TARGETS kron
    RUNTIME DESTINATION bin
)
