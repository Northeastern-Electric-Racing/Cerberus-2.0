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
CMAKE_SOURCE_DIR = /home/app

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/app/build

# Utility rule file for Cerberus-2.0_Boot.

# Include any custom commands dependencies for this target.
include CMakeFiles/Cerberus-2.0_Boot.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Cerberus-2.0_Boot.dir/progress.make

CMakeFiles/Cerberus-2.0_Boot: CMakeFiles/Cerberus-2.0_Boot-complete

CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-mkdir
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build
CMakeFiles/Cerberus-2.0_Boot-complete: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Completed 'Cerberus-2.0_Boot'"
	/usr/bin/cmake -E make_directory /home/app/build/CMakeFiles
	/usr/bin/cmake -E touch /home/app/build/CMakeFiles/Cerberus-2.0_Boot-complete
	/usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-done

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install:
.PHONY : Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build:
.PHONY : Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install:
.PHONY : Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Performing build step for 'Cerberus-2.0_Boot'"
	cd /home/app/Boot/build && $(MAKE)

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure: Boot/tmp/Cerberus-2.0_Boot-cfgcmd.txt
Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Performing configure step for 'Cerberus-2.0_Boot'"
	cd /home/app/Boot/build && /usr/bin/cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE= "-GUnix Makefiles" -S /home/app/Boot -B /home/app/Boot/build
	cd /home/app/Boot/build && /usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-source_dirinfo.txt
Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-mkdir
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "No download step for 'Cerberus-2.0_Boot'"
	/usr/bin/cmake -E echo_append
	/usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "No install step for 'Cerberus-2.0_Boot'"
	cd /home/app/Boot/build && /usr/bin/cmake -E echo_append

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build:
.PHONY : Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-mkdir:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Creating directories for 'Cerberus-2.0_Boot'"
	/usr/bin/cmake -Dcfgdir= -P /home/app/build/Boot/tmp/Cerberus-2.0_Boot-mkdirs.cmake
	/usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-mkdir

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch-info.txt
Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "No patch step for 'Cerberus-2.0_Boot'"
	/usr/bin/cmake -E echo_append
	/usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch

Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update-info.txt
Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "No update step for 'Cerberus-2.0_Boot'"
	/usr/bin/cmake -E echo_append
	/usr/bin/cmake -E touch /home/app/build/Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update

Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-build
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-configure
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-download
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-install
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-mkdir
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-patch
Cerberus-2.0_Boot: Boot/src/Cerberus-2.0_Boot-stamp/Cerberus-2.0_Boot-update
Cerberus-2.0_Boot: CMakeFiles/Cerberus-2.0_Boot
Cerberus-2.0_Boot: CMakeFiles/Cerberus-2.0_Boot-complete
Cerberus-2.0_Boot: CMakeFiles/Cerberus-2.0_Boot.dir/build.make
.PHONY : Cerberus-2.0_Boot

# Rule to build all files generated by this target.
CMakeFiles/Cerberus-2.0_Boot.dir/build: Cerberus-2.0_Boot
.PHONY : CMakeFiles/Cerberus-2.0_Boot.dir/build

CMakeFiles/Cerberus-2.0_Boot.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Cerberus-2.0_Boot.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Cerberus-2.0_Boot.dir/clean

CMakeFiles/Cerberus-2.0_Boot.dir/depend:
	cd /home/app/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/app /home/app /home/app/build /home/app/build /home/app/build/CMakeFiles/Cerberus-2.0_Boot.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Cerberus-2.0_Boot.dir/depend

