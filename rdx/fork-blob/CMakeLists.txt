cmake_minimum_required(VERSION 3.13)
project(fork-up LANGUAGES CXX)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")  
        set(VCPKG_TARGET_TRIPLET "arm64-osx")
    else()  
        set(VCPKG_TARGET_TRIPLET "x64-osx")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")  
        set(VCPKG_TARGET_TRIPLET "arm64-linux")
    else()  
        set(VCPKG_TARGET_TRIPLET "x64-linux")
    endif()
endif()

message(STATUS "Detected OS: ${CMAKE_SYSTEM_NAME}, Arch: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Using vcpkg triplet: ${VCPKG_TARGET_TRIPLET}")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}")

find_package(aws-cpp-sdk-core REQUIRED)
find_package(aws-cpp-sdk-s3 REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

find_package(GTest CONFIG REQUIRED)

include(CTest)
enable_testing()

set(FORK_UP_SOURCES
    src/fork_up.cpp
    src/s3_client/client.cpp
    src/file_helper/helper.cpp
)

# Create a library for fork_up functionality
add_library(fork_up_lib ${FORK_UP_SOURCES})
target_include_directories(fork_up_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set include directories for the library
target_include_directories(fork_up_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${aws-cpp-sdk-core_INCLUDE_DIRS}
    ${aws-cpp-sdk-s3_INCLUDE_DIRS}
)

# Link dependencies to the library
target_link_libraries(fork_up_lib PUBLIC
    aws-cpp-sdk-s3
    aws-cpp-sdk-core
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    CURL::libcurl
    pthread
    dl
)

# Test for fork_up
add_executable(test_fork_up testsuite/test_fork_up.cpp)
target_link_libraries(test_fork_up PRIVATE
    fork_up_lib
    GTest::gtest
    GTest::gtest_main
)

# Register test with CTest
add_test(NAME fork_up_test COMMAND test_fork_up)
