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
CMAKE_SOURCE_DIR = /usr/local/src/robot/jacub/modules/world

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/local/src/robot/jacub/modules/world

# Include any dependencies generated for this target.
include CMakeFiles/jworld.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/jworld.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/jworld.dir/flags.make

CMakeFiles/jworld.dir/src/JWorld.o: CMakeFiles/jworld.dir/flags.make
CMakeFiles/jworld.dir/src/JWorld.o: src/JWorld.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/local/src/robot/jacub/modules/world/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/jworld.dir/src/JWorld.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/jworld.dir/src/JWorld.o -c /usr/local/src/robot/jacub/modules/world/src/JWorld.cpp

CMakeFiles/jworld.dir/src/JWorld.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jworld.dir/src/JWorld.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/local/src/robot/jacub/modules/world/src/JWorld.cpp > CMakeFiles/jworld.dir/src/JWorld.i

CMakeFiles/jworld.dir/src/JWorld.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jworld.dir/src/JWorld.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/local/src/robot/jacub/modules/world/src/JWorld.cpp -o CMakeFiles/jworld.dir/src/JWorld.s

CMakeFiles/jworld.dir/src/JWorld.o.requires:

.PHONY : CMakeFiles/jworld.dir/src/JWorld.o.requires

CMakeFiles/jworld.dir/src/JWorld.o.provides: CMakeFiles/jworld.dir/src/JWorld.o.requires
	$(MAKE) -f CMakeFiles/jworld.dir/build.make CMakeFiles/jworld.dir/src/JWorld.o.provides.build
.PHONY : CMakeFiles/jworld.dir/src/JWorld.o.provides

CMakeFiles/jworld.dir/src/JWorld.o.provides.build: CMakeFiles/jworld.dir/src/JWorld.o


# Object files for target jworld
jworld_OBJECTS = \
"CMakeFiles/jworld.dir/src/JWorld.o"

# External object files for target jworld
jworld_EXTERNAL_OBJECTS =

jworld: CMakeFiles/jworld.dir/src/JWorld.o
jworld: CMakeFiles/jworld.dir/build.make
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_dev.so.2.3.72
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_name.so.2.3.72
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_init.so.2.3.72
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_math.so.2.3.72
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_sig.so.2.3.72
jworld: /usr/local/src/robot/yarp/build/lib/libYARP_OS.so.2.3.72
jworld: CMakeFiles/jworld.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/local/src/robot/jacub/modules/world/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable jworld"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/jworld.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/jworld.dir/build: jworld

.PHONY : CMakeFiles/jworld.dir/build

CMakeFiles/jworld.dir/requires: CMakeFiles/jworld.dir/src/JWorld.o.requires

.PHONY : CMakeFiles/jworld.dir/requires

CMakeFiles/jworld.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/jworld.dir/cmake_clean.cmake
.PHONY : CMakeFiles/jworld.dir/clean

CMakeFiles/jworld.dir/depend:
	cd /usr/local/src/robot/jacub/modules/world && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/local/src/robot/jacub/modules/world /usr/local/src/robot/jacub/modules/world /usr/local/src/robot/jacub/modules/world /usr/local/src/robot/jacub/modules/world /usr/local/src/robot/jacub/modules/world/CMakeFiles/jworld.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/jworld.dir/depend
