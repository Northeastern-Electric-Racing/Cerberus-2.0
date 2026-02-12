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

# NER Include toolchain file

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Colors
string(ASCII 27 Esc)
set(ColorReset "${Esc}[m")
set(ColorBold  "${Esc}[1m")
set(Red         "${Esc}[31m")
set(Green       "${Esc}[32m")
set(Yellow      "${Esc}[33m")
set(Blue        "${Esc}[34m")
set(Magenta     "${Esc}[35m")
set(Cyan        "${Esc}[36m")
set(White       "${Esc}[37m")
set(BoldRed     "${Esc}[1;31m")
set(BoldGreen   "${Esc}[1;32m")
set(BoldYellow  "${Esc}[1;33m")
set(BoldBlue    "${Esc}[1;34m")

# Display ASCII art at start of build
if(EXISTS "${CMAKE_SOURCE_DIR}/art")
    file(READ "${CMAKE_SOURCE_DIR}/art" ASCII_ART)
    message("${BoldYellow}${ASCII_ART}${ColorReset}")
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

# Make it so floats can be printed
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -u _printf_float)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Embedded-Base ThreadX Utils
    "../Drivers/Embedded-Base/threadX/src/u_tx_can.c"
    "../Drivers/Embedded-Base/threadX/src/u_tx_debug.c"
    "../Drivers/Embedded-Base/threadX/src/u_tx_mutex.c"
    "../Drivers/Embedded-Base/threadX/src/u_tx_queues.c"
    "../Drivers/Embedded-Base/threadX/src/u_tx_threads.c"
    "../Drivers/Embedded-Base/threadX/src/u_tx_timers.c"

    # Embedded-Base NetX Utils
    "../Drivers/Embedded-Base/NetX/src/u_nx_debug.c"
    "../Drivers/Embedded-Base/NetX/src/u_nx_ethernet.c"
    "../Drivers/Embedded-Base/NetX/src/nxd_ptp_client.c"

    # Misc Embedded-Base Stuff (drivers, middleware, utils, etc.)
    "../Drivers/Embedded-Base/general/src/lan8670.c"
    "../Drivers/Embedded-Base/general/src/sht30.c"
    "../Drivers/Embedded-Base/general/src/lsm6dso_reg.c"
    "../Drivers/Embedded-Base/general/src/lsm6dso.c"
    "./Drivers/Embedded-Base/general/src/lsm6dsv_reg.c"
    "../Drivers/Embedded-Base/platforms/stm32h563/src/fdcan.c"
    "./Drivers/Embedded-Base/middleware/src/bitstream.c"
    "./Drivers/Embedded-Base/middleware/src/debounce.c"
    "./Drivers/Embedded-Base/middleware/src/timer.c"
    "./Drivers/Embedded-Base/middleware/src/c_utils.c"
    "./Drivers/Embedded-Base/middleware/src/serial.c"

    # Cerberus-2.0 Source Files
    "./Core/Src/u_threads.c"
    "./Core/Src/u_statemachine.c"
    "./Core/Src/u_queues.c"
    "./Core/Src/u_can.c"
    "./Core/Src/can_messages_tx.c"
    "./Core/Src/u_faults.c"
    "./Core/Src/u_mutexes.c"
    "./Core/Src/u_efuses.c"
    "./Core/Src/u_rtds.c"
    "./Core/Src/u_pedals.c"
    "./Core/Src/u_bms.c"
    "./Core/Src/u_dti.c"
    "./Core/Src/u_adc.c"
    "./Core/Src/u_peripherals.c"
    "./Core/Src/u_tsms.c"
    "./Core/Src/u_lightning.c"
    "./Core/Src/u_debug.c"
    "./Core/Src/u_ethernet.c"
)

# Disable Logging in certain files
set_source_files_properties(
    #"./Core/Src/u_inbox.c"
    #"./Core/Src/u_threads.c"
    #"./Core/Src/u_statemachine.c"
    #"./Core/Src/u_queues.c"
    #"./Core/Src/u_can.c"
    #"./Core/Src/u_faults.c"
    #"./Core/Src/u_mutexes.c"
    #"./Core/Src/u_efuses.c"
    #"./Core/Src/u_rtds.c"
    #"./Core/Src/u_pedals.c"
    #"./Core/Src/u_bms.c"
    #"./Core/Src/u_dti.c"
    #"./Core/Src/u_adc.c"
    #"./Core/Src/u_peripherals.c"
    #"./Core/Src/u_tsms.c"

    PROPERTIES COMPILE_DEFINITIONS "NO_LOG"
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    "./Drivers/Embedded-Base/general/include/"
    "./Drivers/Embedded-Base/middleware/include/"
    "./Drivers/Embedded-Base/platforms/stm32h563/include/"
    "./Drivers/Embedded-Base/threadX/inc/"
    "./Drivers/Embedded-Base/NetX/inc/"
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
