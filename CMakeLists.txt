cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Cerberus-2.0)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Display ASCII art at start of build
if(EXISTS "${CMAKE_SOURCE_DIR}/art")
    file(READ "${CMAKE_SOURCE_DIR}/art" ASCII_ART)
    message("${ASCII_ART}")
endif()

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    "../Drivers/Embedded-Base/general/src/lan8670.c"
    "./Core/Src/u_ethernet.c"
    "./Core/Src/u_inbox.c"
    "./Core/Src/u_threads.c"
    "./Core/Src/u_statemachine.c"
    "./Core/Src/u_queues.c"
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    "./Drivers/Embedded-Base/general/include/"
    "./Drivers/Embedded-Base/middleware/include/"
    "./NetXDuo"
    "./Core/Inc/"
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Compiler options
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wno-unused-parameter # u_TODO - this doesn't seem to be working
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
