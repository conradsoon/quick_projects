cmake_minimum_required(VERSION 3.10)
project(quick_roaring_bitmap)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/barking_bitmap_tests.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

FetchContent_MakeAvailable(googletest)

include_directories(include)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} gtest_main)

add_test(NAME example_test COMMAND ${PROJECT_NAME})
