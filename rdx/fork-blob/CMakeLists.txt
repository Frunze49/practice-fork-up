cmake_minimum_required(VERSION 3.13)
project(hello_s3)

set(CMAKE_TOOLCHAIN_FILE "/home/user/practice-fork-up/rdx/fork-blob/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Ручное указание путей
set(AWS_SDK_INCLUDE_DIR "/home/user/practice-fork-up/rdx/fork-blob/vcpkg/installed/x64-linux/include")
set(AWS_SDK_LIB_DIR "/home/user/practice-fork-up/rdx/fork-blob/vcpkg/installed/x64-linux/lib")

include_directories(${AWS_SDK_INCLUDE_DIR})
link_directories(${AWS_SDK_LIB_DIR})

add_executable(${PROJECT_NAME} hello_s3.cpp)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(aws-cpp-sdk-core REQUIRED)
find_package(aws-cpp-sdk-s3 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    aws-cpp-sdk-s3
    aws-cpp-sdk-core
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    pthread
    dl
)