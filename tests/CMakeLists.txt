include(FetchContent)

# Descargar GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Forzar el uso de la misma configuración de runtime
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Definir el ejecutable de pruebas
add_executable(kron_tests
    TOKENIZATION_TEST.cpp
    PARSING_TEST.cpp 
    VALIDATION_TEST.cpp
    EXECUTOR_TEST.cpp
    SPEED_TEST_LIST.cpp
)

# Linkear contra la lógica de tu proyecto y GTest
target_link_libraries(kron_tests 
    PRIVATE
    kron_core
    gtest_main 
    gtest
)
# Habilitar el descubrimiento automático de tests para CTest
include(GoogleTest)
gtest_discover_tests(kron_tests)
