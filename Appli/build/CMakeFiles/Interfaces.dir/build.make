# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/app/Appli

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/app/Appli/build

# Include any dependencies generated for this target.
include CMakeFiles/Interfaces.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Interfaces.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Interfaces.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Interfaces.dir/flags.make

CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj: CMakeFiles/Interfaces.dir/flags.make
CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj: /home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c
CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj: CMakeFiles/Interfaces.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/app/Appli/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj"
	/home/dev/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj -MF CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj.d -o CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj -c /home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c

CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.i"
	/home/dev/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c > CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.i

CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.s"
	/home/dev/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c -o CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.s

Interfaces: CMakeFiles/Interfaces.dir/home/app/Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c.obj
Interfaces: CMakeFiles/Interfaces.dir/build.make
.PHONY : Interfaces

# Rule to build all files generated by this target.
CMakeFiles/Interfaces.dir/build: Interfaces
.PHONY : CMakeFiles/Interfaces.dir/build

CMakeFiles/Interfaces.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Interfaces.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Interfaces.dir/clean

CMakeFiles/Interfaces.dir/depend:
	cd /home/app/Appli/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/app/Appli /home/app/Appli /home/app/Appli/build /home/app/Appli/build /home/app/Appli/build/CMakeFiles/Interfaces.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Interfaces.dir/depend

