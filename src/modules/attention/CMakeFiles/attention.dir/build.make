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
include CMakeFiles/attention.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/attention.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/attention.dir/flags.make

CMakeFiles/attention.dir/src/AttentionModule.o: CMakeFiles/attention.dir/flags.make
CMakeFiles/attention.dir/src/AttentionModule.o: src/AttentionModule.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/attention.dir/src/AttentionModule.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/attention.dir/src/AttentionModule.o -c /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp

CMakeFiles/attention.dir/src/AttentionModule.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/attention.dir/src/AttentionModule.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp > CMakeFiles/attention.dir/src/AttentionModule.i

CMakeFiles/attention.dir/src/AttentionModule.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/attention.dir/src/AttentionModule.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/AttentionModule.cpp -o CMakeFiles/attention.dir/src/AttentionModule.s

CMakeFiles/attention.dir/src/AttentionModule.o.requires:

.PHONY : CMakeFiles/attention.dir/src/AttentionModule.o.requires

CMakeFiles/attention.dir/src/AttentionModule.o.provides: CMakeFiles/attention.dir/src/AttentionModule.o.requires
	$(MAKE) -f CMakeFiles/attention.dir/build.make CMakeFiles/attention.dir/src/AttentionModule.o.provides.build
.PHONY : CMakeFiles/attention.dir/src/AttentionModule.o.provides

CMakeFiles/attention.dir/src/AttentionModule.o.provides.build: CMakeFiles/attention.dir/src/AttentionModule.o


CMakeFiles/attention.dir/src/main.o: CMakeFiles/attention.dir/flags.make
CMakeFiles/attention.dir/src/main.o: src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/attention.dir/src/main.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/attention.dir/src/main.o -c /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/main.cpp

CMakeFiles/attention.dir/src/main.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/attention.dir/src/main.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/main.cpp > CMakeFiles/attention.dir/src/main.i

CMakeFiles/attention.dir/src/main.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/attention.dir/src/main.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/miguel/eclipse-workspace/jacub/src/modules/attention/src/main.cpp -o CMakeFiles/attention.dir/src/main.s

CMakeFiles/attention.dir/src/main.o.requires:

.PHONY : CMakeFiles/attention.dir/src/main.o.requires

CMakeFiles/attention.dir/src/main.o.provides: CMakeFiles/attention.dir/src/main.o.requires
	$(MAKE) -f CMakeFiles/attention.dir/build.make CMakeFiles/attention.dir/src/main.o.provides.build
.PHONY : CMakeFiles/attention.dir/src/main.o.provides

CMakeFiles/attention.dir/src/main.o.provides.build: CMakeFiles/attention.dir/src/main.o


CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o: CMakeFiles/attention.dir/flags.make
CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o: /home/miguel/eclipse-workspace/jacub/src/utils/jutils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o -c /home/miguel/eclipse-workspace/jacub/src/utils/jutils.cpp

CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/miguel/eclipse-workspace/jacub/src/utils/jutils.cpp > CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.i

CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/miguel/eclipse-workspace/jacub/src/utils/jutils.cpp -o CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.s

CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.requires:

.PHONY : CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.requires

CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.provides: CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.requires
	$(MAKE) -f CMakeFiles/attention.dir/build.make CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.provides.build
.PHONY : CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.provides

CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.provides.build: CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o


# Object files for target attention
attention_OBJECTS = \
"CMakeFiles/attention.dir/src/AttentionModule.o" \
"CMakeFiles/attention.dir/src/main.o" \
"CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o"

# External object files for target attention
attention_EXTERNAL_OBJECTS =

attention: CMakeFiles/attention.dir/src/AttentionModule.o
attention: CMakeFiles/attention.dir/src/main.o
attention: CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o
attention: CMakeFiles/attention.dir/build.make
attention: /usr/lib/x86_64-linux-gnu/libYARP_init.so.3.1.0
attention: /usr/lib/x86_64-linux-gnu/libYARP_dev.so.3.1.0
attention: /usr/lib/x86_64-linux-gnu/libYARP_math.so.3.1.0
attention: /usr/lib/x86_64-linux-gnu/libYARP_sig.so.3.1.0
attention: /usr/lib/x86_64-linux-gnu/libYARP_OS.so.3.1.0
attention: CMakeFiles/attention.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable attention"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/attention.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/attention.dir/build: attention

.PHONY : CMakeFiles/attention.dir/build

CMakeFiles/attention.dir/requires: CMakeFiles/attention.dir/src/AttentionModule.o.requires
CMakeFiles/attention.dir/requires: CMakeFiles/attention.dir/src/main.o.requires
CMakeFiles/attention.dir/requires: CMakeFiles/attention.dir/home/miguel/eclipse-workspace/jacub/src/utils/jutils.o.requires

.PHONY : CMakeFiles/attention.dir/requires

CMakeFiles/attention.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/attention.dir/cmake_clean.cmake
.PHONY : CMakeFiles/attention.dir/clean

CMakeFiles/attention.dir/depend:
	cd /home/miguel/eclipse-workspace/jacub/src/modules/attention && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention /home/miguel/eclipse-workspace/jacub/src/modules/attention/CMakeFiles/attention.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/attention.dir/depend
