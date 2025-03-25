cmake_minimum_required(VERSION 3.10)
project(ForkUp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find AWS SDK
find_package(AWSSDK REQUIRED COMPONENTS s3)
# Find nlohmann_json (make sure it's installed)
find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(fork-up fork-up.cpp)

target_link_libraries(fork-up 
    PRIVATE 
    ${AWSSDK_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)
