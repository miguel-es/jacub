# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/local/src/robot/jacub/modules/iDevER

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/local/src/robot/jacub/modules/iDevER

# Include any dependencies generated for this target.
include CMakeFiles/iDevER.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/iDevER.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/iDevER.dir/flags.make

CMakeFiles/iDevER.dir/src/iDevER.o: CMakeFiles/iDevER.dir/flags.make
CMakeFiles/iDevER.dir/src/iDevER.o: src/iDevER.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/local/src/robot/jacub/modules/iDevER/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/iDevER.dir/src/iDevER.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/iDevER.dir/src/iDevER.o -c /usr/local/src/robot/jacub/modules/iDevER/src/iDevER.cpp

CMakeFiles/iDevER.dir/src/iDevER.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iDevER.dir/src/iDevER.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/local/src/robot/jacub/modules/iDevER/src/iDevER.cpp > CMakeFiles/iDevER.dir/src/iDevER.i

CMakeFiles/iDevER.dir/src/iDevER.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iDevER.dir/src/iDevER.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/local/src/robot/jacub/modules/iDevER/src/iDevER.cpp -o CMakeFiles/iDevER.dir/src/iDevER.s

CMakeFiles/iDevER.dir/src/iDevER.o.requires:

.PHONY : CMakeFiles/iDevER.dir/src/iDevER.o.requires

CMakeFiles/iDevER.dir/src/iDevER.o.provides: CMakeFiles/iDevER.dir/src/iDevER.o.requires
	$(MAKE) -f CMakeFiles/iDevER.dir/build.make CMakeFiles/iDevER.dir/src/iDevER.o.provides.build
.PHONY : CMakeFiles/iDevER.dir/src/iDevER.o.provides

CMakeFiles/iDevER.dir/src/iDevER.o.provides.build: CMakeFiles/iDevER.dir/src/iDevER.o


# Object files for target iDevER
iDevER_OBJECTS = \
"CMakeFiles/iDevER.dir/src/iDevER.o"

# External object files for target iDevER
iDevER_EXTERNAL_OBJECTS =

iDevER: CMakeFiles/iDevER.dir/src/iDevER.o
iDevER: CMakeFiles/iDevER.dir/build.make
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_dev.so.2.3.72
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_name.so.2.3.72
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_init.so.2.3.72
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_math.so.2.3.72
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_sig.so.2.3.72
iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_OS.so.2.3.72
iDevER: CMakeFiles/iDevER.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/local/src/robot/jacub/modules/iDevER/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable iDevER"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iDevER.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/iDevER.dir/build: iDevER

.PHONY : CMakeFiles/iDevER.dir/build

# Object files for target iDevER
iDevER_OBJECTS = \
"CMakeFiles/iDevER.dir/src/iDevER.o"

# External object files for target iDevER
iDevER_EXTERNAL_OBJECTS =

CMakeFiles/CMakeRelink.dir/iDevER: CMakeFiles/iDevER.dir/src/iDevER.o
CMakeFiles/CMakeRelink.dir/iDevER: CMakeFiles/iDevER.dir/build.make
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_dev.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_name.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_init.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_math.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_sig.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: /usr/local/src/robot/yarp/build/lib/libYARP_OS.so.2.3.72
CMakeFiles/CMakeRelink.dir/iDevER: CMakeFiles/iDevER.dir/relink.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/local/src/robot/jacub/modules/iDevER/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable CMakeFiles/CMakeRelink.dir/iDevER"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iDevER.dir/relink.txt --verbose=$(VERBOSE)

# Rule to relink during preinstall.
CMakeFiles/iDevER.dir/preinstall: CMakeFiles/CMakeRelink.dir/iDevER

.PHONY : CMakeFiles/iDevER.dir/preinstall

CMakeFiles/iDevER.dir/requires: CMakeFiles/iDevER.dir/src/iDevER.o.requires

.PHONY : CMakeFiles/iDevER.dir/requires

CMakeFiles/iDevER.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/iDevER.dir/cmake_clean.cmake
.PHONY : CMakeFiles/iDevER.dir/clean

CMakeFiles/iDevER.dir/depend:
	cd /usr/local/src/robot/jacub/modules/iDevER && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/local/src/robot/jacub/modules/iDevER /usr/local/src/robot/jacub/modules/iDevER /usr/local/src/robot/jacub/modules/iDevER /usr/local/src/robot/jacub/modules/iDevER /usr/local/src/robot/jacub/modules/iDevER/CMakeFiles/iDevER.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/iDevER.dir/depend

