include(FetchContent)

# Download GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Force the use of the same runtime configuration
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Define the test executable
add_executable(kls_tests
    TOKENIZATION_TEST.cpp
    PARSING_TEST.cpp 
    VALIDATION_TEST.cpp
    EXECUTOR_TEST.cpp
    SPEED_TEST_LIST.cpp
)

# Add include directories for tests
target_include_directories(kls_tests PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/generated
)

# Link against the project logic and GTest
target_link_libraries(kls_tests 
    PRIVATE
    kls_core
    gtest_main 
    gtest
)

# Enable automatic test discovery for CTest
include(GoogleTest)
gtest_discover_tests(kls_tests)
