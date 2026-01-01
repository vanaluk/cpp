cmake_minimum_required(VERSION 3.15)
project(CppBenchmarkKit VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYBIND11 "Build pybind11 bindings" ON)
option(BUILD_ASIO_SERVER "Build Boost.Asio server" ON)
option(BUILD_EXAMPLES "Build example benchmarks" ON)

# Compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Ultra low latency Release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -march=native")           # CPU-specific optimizations
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -mtune=native")           # Tune for current CPU
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto")                   # Link-time optimization
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -ffast-math")             # Aggressive FP optimizations
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -funroll-loops")          # Unroll loops
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fomit-frame-pointer")    # Free up register
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fno-exceptions")         # Disable exceptions (ULL critical)
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fno-rtti")               # Disable RTTI
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fvisibility=hidden")     # Hide symbols by default
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fvisibility-inlines-hidden")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fno-stack-protector")    # Disable stack protection
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fno-plt")                # Direct function calls
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fprefetch-loop-arrays")  # Prefetch hints
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -falign-functions=32")    # Cache-line alignment
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -falign-loops=32")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG")                # Disable assertions

# LTO for linker
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -fuse-linker-plugin")

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# PostgreSQL client library (libpq)
find_package(PostgreSQL QUIET)
if(NOT PostgreSQL_FOUND)
    # Fallback: find libpq manually
    find_library(LIBPQ_LIBRARY NAMES pq libpq)
    find_path(LIBPQ_INCLUDE_DIR libpq-fe.h
        PATHS /usr/include /usr/local/include /usr/include/postgresql)
    if(LIBPQ_LIBRARY AND LIBPQ_INCLUDE_DIR)
        set(PostgreSQL_FOUND TRUE)
        set(PostgreSQL_LIBRARIES ${LIBPQ_LIBRARY})
        set(PostgreSQL_INCLUDE_DIRS ${LIBPQ_INCLUDE_DIR})
        message(STATUS "Found libpq: ${LIBPQ_LIBRARY}")
    else()
        message(WARNING "libpq not found - asio_server will not have PostgreSQL support")
    endif()
endif()

# Pybind11 (optional)
if(BUILD_PYBIND11)
    # Find Python 3.8+ explicitly to avoid using old Python versions
    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter Development)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    
    find_package(pybind11 QUIET)
    if(NOT pybind11_FOUND)
        message(STATUS "pybind11 not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.11.1
        )
        FetchContent_MakeAvailable(pybind11)
    endif()
endif()

# =============================================================================
# Core library (header-only, but we define it for include paths)
# =============================================================================
add_library(benchmark_kit_core INTERFACE)
target_include_directories(benchmark_kit_core INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

# =============================================================================
# Example implementations
# =============================================================================
set(EXAMPLE_SOURCES
    src/examples/weak_ptr/custom_weak_ptr.cpp
    src/examples/vector_erase/vector_erase.cpp
    src/examples/container_lookup/container_benchmark.cpp
)

set(EXAMPLE_HEADERS
    src/examples/weak_ptr/custom_weak_ptr.hpp
    src/examples/vector_erase/vector_erase.hpp
    src/examples/container_lookup/container_benchmark.hpp
)

# =============================================================================
# Main executable (demo)
# =============================================================================
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${EXAMPLE_SOURCES}
    src/benchmark/benchmark_utils.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Boost::system
    Threads::Threads
)

# Legacy executable name (for backward compatibility)
add_custom_target(CppInterviewDemo ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_BINARY_DIR}/CppInterviewDemo
    DEPENDS ${PROJECT_NAME}
)

# =============================================================================
# Pybind11 module (requires exceptions and RTTI, override ULL flags)
# =============================================================================
if(BUILD_PYBIND11)
    pybind11_add_module(benchmark_kit_bindings
        src/bindings/bindings.cpp
        ${EXAMPLE_SOURCES}
        src/benchmark/benchmark_utils.cpp
    )
    target_include_directories(benchmark_kit_bindings PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(benchmark_kit_bindings PRIVATE 
        Boost::system
        Threads::Threads
    )
    # pybind11 requires exceptions and RTTI - override ULL flags
    target_compile_options(benchmark_kit_bindings PRIVATE
        $<$<CONFIG:Release>:-fexceptions -frtti>
    )
    set_target_properties(benchmark_kit_bindings PROPERTIES
        CXX_VISIBILITY_PRESET "hidden"
        VISIBILITY_INLINES_HIDDEN 1
    )

    # Legacy binding name (for backward compatibility with Python scripts)
    add_custom_target(cpp_interview_bindings ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:benchmark_kit_bindings>
            ${CMAKE_BINARY_DIR}/cpp_interview_bindings$<TARGET_PROPERTY:benchmark_kit_bindings,SUFFIX>
        DEPENDS benchmark_kit_bindings
    )
endif()

# =============================================================================
# Boost.Asio server (requires exceptions for error handling)
# =============================================================================
if(BUILD_ASIO_SERVER)
    # Check if new server location exists, otherwise use legacy
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/server/server.cpp")
        set(SERVER_SOURCE src/server/server.cpp)
    else()
        set(SERVER_SOURCE src/asio_server/server.cpp)
    endif()

    add_executable(benchmark_server
        ${SERVER_SOURCE}
        ${EXAMPLE_SOURCES}
        src/benchmark/benchmark_utils.cpp
    )
    target_include_directories(benchmark_server PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(benchmark_server PRIVATE 
        Boost::system
        Threads::Threads
    )
    # Boost.Asio and server error handling require exceptions
    target_compile_options(benchmark_server PRIVATE
        $<$<CONFIG:Release>:-fexceptions>
    )
    # Link PostgreSQL if available
    if(PostgreSQL_FOUND)
        target_link_libraries(benchmark_server PRIVATE ${PostgreSQL_LIBRARIES})
        target_include_directories(benchmark_server PRIVATE ${PostgreSQL_INCLUDE_DIRS})
        target_compile_definitions(benchmark_server PRIVATE HAS_POSTGRESQL=1)
    endif()

    # Legacy executable name (for backward compatibility)
    add_custom_target(asio_server ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:benchmark_server>
            ${CMAKE_BINARY_DIR}/asio_server
        DEPENDS benchmark_server
    )
endif()

# =============================================================================
# Examples (optional standalone benchmarks)
# =============================================================================
if(BUILD_EXAMPLES)
    # Sorting benchmark example
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/examples/sorting/sorting_benchmark.cpp")
        add_executable(sorting_benchmark
            src/examples/sorting/sorting_benchmark.cpp
        )
        target_include_directories(sorting_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)
        target_link_libraries(sorting_benchmark PRIVATE Threads::Threads)
    endif()
endif()

# =============================================================================
# Tests
# =============================================================================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
if(BUILD_PYBIND11)
    install(TARGETS benchmark_kit_bindings DESTINATION python)
endif()
if(BUILD_ASIO_SERVER)
    install(TARGETS benchmark_server DESTINATION bin)
endif()

# Install core headers
install(DIRECTORY src/core/
    DESTINATION include/benchmark_kit
    FILES_MATCHING PATTERN "*.hpp"
)
