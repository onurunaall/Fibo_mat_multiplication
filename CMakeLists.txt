cmake_minimum_required(VERSION 3.10)
project(FibonacciProject VERSION 1.0 LANGUAGES CXX)

# Set C++ standard to C++11 and optimization flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# List all source files for the project
set(SOURCES
    main.cpp
    fibonacci.cpp
    fastexp2d.cpp
    utils.cpp
    eval.cpp
)

# Create the executable target
add_executable(fib_app ${SOURCES})

# Include current directory for header files
target_include_directories(fib_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
