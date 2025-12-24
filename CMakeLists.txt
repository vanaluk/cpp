cmake_minimum_required(VERSION 3.15)
project(CppInterviewDemo VERSION 1.0.0 LANGUAGES CXX)

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

# Compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

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

# Source files
set(SOURCES
    src/main.cpp
    src/task1_weak_ptr/custom_weak_ptr.cpp
    src/task2_vector_erase/vector_erase.cpp
    src/task3_mapping/container_benchmark.cpp
    src/benchmark/benchmark_utils.cpp
)

set(HEADERS
    src/task1_weak_ptr/custom_weak_ptr.hpp
    src/task2_vector_erase/vector_erase.hpp
    src/task3_mapping/container_benchmark.hpp
    src/benchmark/benchmark_utils.hpp
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Boost::system
    Threads::Threads
)

# Pybind11 module
if(BUILD_PYBIND11)
    pybind11_add_module(cpp_interview_bindings
        src/bindings/bindings.cpp
        src/task1_weak_ptr/custom_weak_ptr.cpp
        src/task2_vector_erase/vector_erase.cpp
        src/task3_mapping/container_benchmark.cpp
        src/benchmark/benchmark_utils.cpp
    )
    target_link_libraries(cpp_interview_bindings PRIVATE 
        Boost::system
        Threads::Threads
    )
    set_target_properties(cpp_interview_bindings PROPERTIES
        CXX_VISIBILITY_PRESET "hidden"
        VISIBILITY_INLINES_HIDDEN 1
    )
endif()

# Boost.Asio server
if(BUILD_ASIO_SERVER)
    add_executable(asio_server 
        src/asio_server/server.cpp
        src/task1_weak_ptr/custom_weak_ptr.cpp
        src/task2_vector_erase/vector_erase.cpp
        src/task3_mapping/container_benchmark.cpp
        src/benchmark/benchmark_utils.cpp
    )
    target_link_libraries(asio_server PRIVATE 
        Boost::system
        Threads::Threads
    )
    target_include_directories(asio_server PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
if(BUILD_PYBIND11)
    install(TARGETS cpp_interview_bindings DESTINATION python)
endif()
if(BUILD_ASIO_SERVER)
    install(TARGETS asio_server DESTINATION bin)
endif()
