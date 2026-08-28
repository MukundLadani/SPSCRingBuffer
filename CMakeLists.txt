cmake_minimum_required(VERSION 3.20)
project(SPSCRingBuffer VERSION 1.0 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Create the test executable (only compile the test file)
add_executable(queue_tests tests/test_queue.cpp)

# Tell the test executable where to find SPSCQueue.h
target_include_directories(queue_tests PRIVATE include)

# Link Google Test
target_link_libraries(queue_tests PRIVATE gtest_main)

# Register tests for CTest
include(GoogleTest)
gtest_discover_tests(queue_tests)