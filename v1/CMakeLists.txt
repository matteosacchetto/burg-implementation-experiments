cmake_minimum_required(VERSION 3.14)

# Project name and version
project(burg VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Enable fast math
if(NATIVE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

# Enable fast math
if(FAST_MATH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
endif()

# Enable achitecture based flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") # Linux
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") # Raspberry Pi
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=cortex-a72")
endif()

# Build type
message("-- ${CMAKE_BUILD_TYPE} build")
message("-- ${CMAKE_SYSTEM_PROCESSOR} architecture")
message("-- ${CMAKE_SYSTEM_NAME}")
message("-- ${CMAKE_CXX_COMPILER}${CMAKE_CXX_FLAGS}")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
endif()

# Activate warnings
if(MSVC)
  # Warning level 4
  add_compile_options(/W4)
else()
  # Lots of warnings
  add_compile_options(-Wall -Wextra -pedantic)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_GetProperties(json)

if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR})
endif()
include_directories(${json_SOURCE_DIR}/include)

if(DATA_TYPE STREQUAL "DOUBLE")
  message("-- Using data-type=double")
  add_definitions(-DUSE_DOUBLE)
elseif(DATA_TYPE STREQUAL "LONG_DOUBLE")
  message("-- Using data-type=long double")
  add_definitions(-DUSE_LONG_DOUBLE)
else()
  message("-- Using data-type=double")
endif()

# Compilation definitions
if(BURG STREQUAL "BASIC")
  message("-- Using BURG basic implementation")
  add_definitions(-DBURG_BASIC)
elseif(BURG STREQUAL "OPT_DEN")
  message("-- Using BURG optimized den implementation")
  add_definitions(-DBURG_OPT_DEN)
elseif(BURG STREQUAL "OPT_DEN_SQRT")
  message("-- Using BURG optimized den sqrt implementation")
  add_definitions(-DBURG_OPT_DEN_SQRT)
elseif(BURG STREQUAL "COMP_BASIC")
  message("-- Using BURG compensated basic implementation")
  add_definitions(-DBURG_COMP_BASIC)
elseif(BURG STREQUAL "COMP_OPT_DEN")
  message("-- Using BURG compensated optimized den implementation")
  add_definitions(-DBURG_COMP_OPT_DEN)
elseif(BURG STREQUAL "COMP_OPT_DEN_SQRT")
  message("-- Using BURG compensated optimized den sqrt implementation")
  add_definitions(-DBURG_COMP_OPT_DEN_SQRT)
else()
  message("-- Using BURG basic implementation")
endif()

# [[ Build executable ]]
# Add the executable
add_executable(${PROJECT_NAME} src/main.cpp src/timer.cpp src/tty.cpp)
add_executable(${PROJECT_NAME}-error src/main-error.cpp src/timer.cpp src/tty.cpp)


# Link additional libraries
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME}-error PRIVATE nlohmann_json::nlohmann_json)