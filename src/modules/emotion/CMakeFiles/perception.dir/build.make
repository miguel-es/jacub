# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/miguel/eclipse-workspace/jacub/src/modules/attention

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/miguel/eclipse-workspace/jacub/src/modules/attention

# Include any dependencies generated for this target.
include CMakeFiles/perception.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/perception.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/perception.dir/flags.make

CMakeFiles/perception.dir/src/AttentionModule.o: CMakeFiles/perception.dir/flags.make
CMakeFiles/perception.dir/src/AttentionModule.o: src/AttentionModule.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/perception.dir/src/AttentionModule.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/perception.dir/src/AttentionModule.o -c /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp

CMakeFiles/perception.dir/src/AttentionModule.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/perception.dir/src/AttentionModule.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp > CMakeFiles/perception.dir/src/AttentionModule.i

CMakeFiles/perception.dir/src/AttentionModule.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/perception.dir/src/AttentionModule.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp -o CMakeFiles/perception.dir/src/AttentionModule.s

CMakeFiles/perception.dir/src/AttentionModule.o.requires:

.PHONY : CMakeFiles/perception.dir/src/AttentionModule.o.requires

CMakeFiles/perception.dir/src/AttentionModule.o.provides: CMakeFiles/perception.dir/src/AttentionModule.o.requires
	$(MAKE) -f CMakeFiles/perception.dir/build.make CMakeFiles/perception.dir/src/AttentionModule.o.provides.build
.PHONY : CMakeFiles/perception.dir/src/AttentionModule.o.provides

CMakeFiles/perception.dir/src/AttentionModule.o.provides.build: CMakeFiles/perception.dir/src/AttentionModule.o


# Object files for target perception
perception_OBJECTS = \
"CMakeFiles/perception.dir/src/AttentionModule.o"

# External object files for target perception
perception_EXTERNAL_OBJECTS =

perception: CMakeFiles/perception.dir/src/AttentionModule.o
perception: CMakeFiles/perception.dir/build.make
perception: /usr/lib/x86_64-linux-gnu/libYARP_init.so.3.1.0
perception: /usr/lib/x86_64-linux-gnu/libYARP_dev.so.3.1.0
perception: /usr/lib/x86_64-linux-gnu/libYARP_math.so.3.1.0
perception: /usr/lib/x86_64-linux-gnu/libYARP_sig.so.3.1.0
perception: /usr/lib/x86_64-linux-gnu/libYARP_OS.so.3.1.0
perception: CMakeFiles/perception.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable perception"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/perception.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/perception.dir/build: perception

.PHONY : CMakeFiles/perception.dir/build

CMakeFiles/perception.dir/requires: CMakeFiles/perception.dir/src/AttentionModule.o.requires

.PHONY : CMakeFiles/perception.dir/requires

CMakeFiles/perception.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/perception.dir/cmake_clean.cmake
.PHONY : CMakeFiles/perception.dir/clean

CMakeFiles/perception.dir/depend:
	cd /home/miguel/eclipse-workspace/jacub/src/modules/attention && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles/perception.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/perception.dir/depend
