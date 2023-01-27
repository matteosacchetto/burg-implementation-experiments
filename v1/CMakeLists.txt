cmake_minimum_required(VERSION 3.14)

# Project name and version
project(burg VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set common C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

# Build type
message("-- ${CMAKE_BUILD_TYPE} build")
message("-- ${CMAKE_SYSTEM_PROCESSOR} architecture")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
endif()

# Activate warnings
if (MSVC)
  # Warning level 4
  add_compile_options(/W4)
else()
  # Lots of warnings
  add_compile_options(-Wall -Wextra -pedantic)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# [[ Build executable ]]
# Sources
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link additional libraries
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)