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

# ============================================================
# Test for Example 1: CustomSharedPtr, CustomWeakPtr, lock()
# ============================================================
add_executable(test_weak_ptr 
    test_task1.cpp 
    ${CMAKE_SOURCE_DIR}/src/examples/weak_ptr/custom_weak_ptr.cpp
)
target_link_libraries(test_weak_ptr PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_weak_ptr PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
# Boost.Test requires exceptions and RTTI - override ULL flags in Release
target_compile_options(test_weak_ptr PRIVATE
    $<$<CONFIG:Release>:-fexceptions -frtti>
)
add_test(NAME WeakPtrTests COMMAND test_weak_ptr --log_level=test_suite)

# ============================================================
# Test for Example 2: Vector erase methods
# ============================================================
add_executable(test_vector_erase 
    test_task2.cpp 
    ${CMAKE_SOURCE_DIR}/src/examples/vector_erase/vector_erase.cpp
    ${CMAKE_SOURCE_DIR}/src/benchmark/benchmark_utils.cpp
)
target_link_libraries(test_vector_erase PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_vector_erase PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
# Boost.Test requires exceptions and RTTI - override ULL flags in Release
target_compile_options(test_vector_erase PRIVATE 
    $<$<CONFIG:Release>:-fexceptions -frtti>
)
add_test(NAME VectorEraseTests COMMAND test_vector_erase --log_level=test_suite)

# ============================================================
# Test for Example 3: Container benchmark functions
# ============================================================
add_executable(test_container_lookup 
    test_task3.cpp 
    ${CMAKE_SOURCE_DIR}/src/examples/container_lookup/container_benchmark.cpp
    ${CMAKE_SOURCE_DIR}/src/benchmark/benchmark_utils.cpp
)
target_link_libraries(test_container_lookup PRIVATE 
    Boost::unit_test_framework 
    Threads::Threads
)
target_include_directories(test_container_lookup PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
# Boost.Test requires exceptions and RTTI - override ULL flags in Release
target_compile_options(test_container_lookup PRIVATE 
    $<$<CONFIG:Release>:-fexceptions -frtti>
)
add_test(NAME ContainerLookupTests COMMAND test_container_lookup --log_level=test_suite)
