# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/furture/code/trylearn/CssParser

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/furture/code/trylearn/CssParser/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/CssParser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/CssParser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/CssParser.dir/flags.make

CMakeFiles/CssParser.dir/main.c.o: CMakeFiles/CssParser.dir/flags.make
CMakeFiles/CssParser.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/code/trylearn/CssParser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/CssParser.dir/main.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/CssParser.dir/main.c.o   -c /Users/furture/code/trylearn/CssParser/main.c

CMakeFiles/CssParser.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/CssParser.dir/main.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/furture/code/trylearn/CssParser/main.c > CMakeFiles/CssParser.dir/main.c.i

CMakeFiles/CssParser.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/CssParser.dir/main.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/furture/code/trylearn/CssParser/main.c -o CMakeFiles/CssParser.dir/main.c.s

CMakeFiles/CssParser.dir/main.c.o.requires:

.PHONY : CMakeFiles/CssParser.dir/main.c.o.requires

CMakeFiles/CssParser.dir/main.c.o.provides: CMakeFiles/CssParser.dir/main.c.o.requires
	$(MAKE) -f CMakeFiles/CssParser.dir/build.make CMakeFiles/CssParser.dir/main.c.o.provides.build
.PHONY : CMakeFiles/CssParser.dir/main.c.o.provides

CMakeFiles/CssParser.dir/main.c.o.provides.build: CMakeFiles/CssParser.dir/main.c.o


# Object files for target CssParser
CssParser_OBJECTS = \
"CMakeFiles/CssParser.dir/main.c.o"

# External object files for target CssParser
CssParser_EXTERNAL_OBJECTS =

CssParser: CMakeFiles/CssParser.dir/main.c.o
CssParser: CMakeFiles/CssParser.dir/build.make
CssParser: CMakeFiles/CssParser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/furture/code/trylearn/CssParser/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable CssParser"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CssParser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/CssParser.dir/build: CssParser

.PHONY : CMakeFiles/CssParser.dir/build

CMakeFiles/CssParser.dir/requires: CMakeFiles/CssParser.dir/main.c.o.requires

.PHONY : CMakeFiles/CssParser.dir/requires

CMakeFiles/CssParser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CssParser.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CssParser.dir/clean

CMakeFiles/CssParser.dir/depend:
	cd /Users/furture/code/trylearn/CssParser/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/furture/code/trylearn/CssParser /Users/furture/code/trylearn/CssParser /Users/furture/code/trylearn/CssParser/cmake-build-debug /Users/furture/code/trylearn/CssParser/cmake-build-debug /Users/furture/code/trylearn/CssParser/cmake-build-debug/CMakeFiles/CssParser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/CssParser.dir/depend

