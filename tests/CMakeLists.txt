include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- CONFIGURACIÓN DE HERRAMIENTAS ---

option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

# Creamos una lista de flags para no repetir código
set(SANITIZER_FLAGS "")
if(ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    set(SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    message(STATUS "Sanitizers enabled")
endif()

set(COVERAGE_FLAGS "")
if(ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    set(COVERAGE_FLAGS --coverage -fprofile-arcs -ftest-coverage)
    message(STATUS "Coverage instrumentation enabled")
endif()

# --- TARGET DE TESTS ---

add_executable(kls_tests
    TOKENIZATION_TEST.cpp
    PARSING_TEST.cpp 
    VALIDATION_TEST.cpp
    EXECUTOR_TEST.cpp
    SPEED_TEST_LIST.cpp
)

target_compile_options(kls_tests PRIVATE ${SANITIZER_FLAGS} ${COVERAGE_FLAGS} -g)
target_link_options(kls_tests PRIVATE ${SANITIZER_FLAGS} ${COVERAGE_FLAGS})

target_include_directories(kls_tests PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/generated
)

target_link_libraries(kls_tests 
    PRIVATE
    kls_core
    gtest_main 
    gtest
)

include(GoogleTest)
gtest_discover_tests(kls_tests)
