# tests/CMakeLists.txt - Boost.Test configuration

# Find Boost.Test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    message(FATAL_ERROR 
        "Boost.Test not found. Install Boost with unit_test_framework component:\n"
        "  Ubuntu/Debian: sudo apt-get install libboost-test-dev\n"
        "  macOS: brew install boost")
endif()

# Required for dynamic linking with Boost.Test shared library
add_compile_definitions(BOOST_TEST_DYN_LINK)

# Boost.Test requires exceptions and RTTI - override ULL flags for tests
set(TEST_OVERRIDE_FLAGS "")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(TEST_OVERRIDE_FLAGS "-fexceptions" "-frtti")
endif()

# ============================================================
# Test for Task 1: CustomSharedPtr, CustomWeakPtr, lock()
# ============================================================
add_executable(test_task1 
    test_task1.cpp 
    ${CMAKE_SOURCE_DIR}/src/task1_weak_ptr/custom_weak_ptr.cpp
)
target_link_libraries(test_task1 PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_task1 PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(test_task1 PRIVATE ${TEST_OVERRIDE_FLAGS})
add_test(NAME Task1Tests COMMAND test_task1 --log_level=test_suite)

# ============================================================
# Test for Task 2: Vector erase methods
# ============================================================
add_executable(test_task2 
    test_task2.cpp 
    ${CMAKE_SOURCE_DIR}/src/task2_vector_erase/vector_erase.cpp
    ${CMAKE_SOURCE_DIR}/src/benchmark/benchmark_utils.cpp
)
target_link_libraries(test_task2 PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_task2 PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(test_task2 PRIVATE ${TEST_OVERRIDE_FLAGS})
add_test(NAME Task2Tests COMMAND test_task2 --log_level=test_suite)

# ============================================================
# Test for Task 3: Container benchmark functions
# ============================================================
add_executable(test_task3 
    test_task3.cpp 
    ${CMAKE_SOURCE_DIR}/src/task3_mapping/container_benchmark.cpp
    ${CMAKE_SOURCE_DIR}/src/benchmark/benchmark_utils.cpp
)
target_link_libraries(test_task3 PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_task3 PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(test_task3 PRIVATE ${TEST_OVERRIDE_FLAGS})
add_test(NAME Task3Tests COMMAND test_task3 --log_level=test_suite)
