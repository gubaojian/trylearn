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
CMAKE_SOURCE_DIR = /Users/furture/CLionProjects/Tson

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/furture/CLionProjects/Tson/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/TsonProject.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/TsonProject.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TsonProject.dir/flags.make

CMakeFiles/TsonProject.dir/main.cpp.o: CMakeFiles/TsonProject.dir/flags.make
CMakeFiles/TsonProject.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/TsonProject.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/TsonProject.dir/main.cpp.o -c /Users/furture/CLionProjects/Tson/main.cpp

CMakeFiles/TsonProject.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TsonProject.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/furture/CLionProjects/Tson/main.cpp > CMakeFiles/TsonProject.dir/main.cpp.i

CMakeFiles/TsonProject.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TsonProject.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/furture/CLionProjects/Tson/main.cpp -o CMakeFiles/TsonProject.dir/main.cpp.s

CMakeFiles/TsonProject.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/TsonProject.dir/main.cpp.o.requires

CMakeFiles/TsonProject.dir/main.cpp.o.provides: CMakeFiles/TsonProject.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/TsonProject.dir/build.make CMakeFiles/TsonProject.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/TsonProject.dir/main.cpp.o.provides

CMakeFiles/TsonProject.dir/main.cpp.o.provides.build: CMakeFiles/TsonProject.dir/main.cpp.o


CMakeFiles/TsonProject.dir/tson.c.o: CMakeFiles/TsonProject.dir/flags.make
CMakeFiles/TsonProject.dir/tson.c.o: ../tson.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/TsonProject.dir/tson.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TsonProject.dir/tson.c.o   -c /Users/furture/CLionProjects/Tson/tson.c

CMakeFiles/TsonProject.dir/tson.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TsonProject.dir/tson.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/furture/CLionProjects/Tson/tson.c > CMakeFiles/TsonProject.dir/tson.c.i

CMakeFiles/TsonProject.dir/tson.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TsonProject.dir/tson.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/furture/CLionProjects/Tson/tson.c -o CMakeFiles/TsonProject.dir/tson.c.s

CMakeFiles/TsonProject.dir/tson.c.o.requires:

.PHONY : CMakeFiles/TsonProject.dir/tson.c.o.requires

CMakeFiles/TsonProject.dir/tson.c.o.provides: CMakeFiles/TsonProject.dir/tson.c.o.requires
	$(MAKE) -f CMakeFiles/TsonProject.dir/build.make CMakeFiles/TsonProject.dir/tson.c.o.provides.build
.PHONY : CMakeFiles/TsonProject.dir/tson.c.o.provides

CMakeFiles/TsonProject.dir/tson.c.o.provides.build: CMakeFiles/TsonProject.dir/tson.c.o


CMakeFiles/TsonProject.dir/tsonjsc.c.o: CMakeFiles/TsonProject.dir/flags.make
CMakeFiles/TsonProject.dir/tsonjsc.c.o: ../tsonjsc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/TsonProject.dir/tsonjsc.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TsonProject.dir/tsonjsc.c.o   -c /Users/furture/CLionProjects/Tson/tsonjsc.c

CMakeFiles/TsonProject.dir/tsonjsc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TsonProject.dir/tsonjsc.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/furture/CLionProjects/Tson/tsonjsc.c > CMakeFiles/TsonProject.dir/tsonjsc.c.i

CMakeFiles/TsonProject.dir/tsonjsc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TsonProject.dir/tsonjsc.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/furture/CLionProjects/Tson/tsonjsc.c -o CMakeFiles/TsonProject.dir/tsonjsc.c.s

CMakeFiles/TsonProject.dir/tsonjsc.c.o.requires:

.PHONY : CMakeFiles/TsonProject.dir/tsonjsc.c.o.requires

CMakeFiles/TsonProject.dir/tsonjsc.c.o.provides: CMakeFiles/TsonProject.dir/tsonjsc.c.o.requires
	$(MAKE) -f CMakeFiles/TsonProject.dir/build.make CMakeFiles/TsonProject.dir/tsonjsc.c.o.provides.build
.PHONY : CMakeFiles/TsonProject.dir/tsonjsc.c.o.provides

CMakeFiles/TsonProject.dir/tsonjsc.c.o.provides.build: CMakeFiles/TsonProject.dir/tsonjsc.c.o


CMakeFiles/TsonProject.dir/data.c.o: CMakeFiles/TsonProject.dir/flags.make
CMakeFiles/TsonProject.dir/data.c.o: ../data.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/TsonProject.dir/data.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TsonProject.dir/data.c.o   -c /Users/furture/CLionProjects/Tson/data.c

CMakeFiles/TsonProject.dir/data.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TsonProject.dir/data.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/furture/CLionProjects/Tson/data.c > CMakeFiles/TsonProject.dir/data.c.i

CMakeFiles/TsonProject.dir/data.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TsonProject.dir/data.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/furture/CLionProjects/Tson/data.c -o CMakeFiles/TsonProject.dir/data.c.s

CMakeFiles/TsonProject.dir/data.c.o.requires:

.PHONY : CMakeFiles/TsonProject.dir/data.c.o.requires

CMakeFiles/TsonProject.dir/data.c.o.provides: CMakeFiles/TsonProject.dir/data.c.o.requires
	$(MAKE) -f CMakeFiles/TsonProject.dir/build.make CMakeFiles/TsonProject.dir/data.c.o.provides.build
.PHONY : CMakeFiles/TsonProject.dir/data.c.o.provides

CMakeFiles/TsonProject.dir/data.c.o.provides.build: CMakeFiles/TsonProject.dir/data.c.o


CMakeFiles/TsonProject.dir/tsonjs.cpp.o: CMakeFiles/TsonProject.dir/flags.make
CMakeFiles/TsonProject.dir/tsonjs.cpp.o: ../tsonjs.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/TsonProject.dir/tsonjs.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/TsonProject.dir/tsonjs.cpp.o -c /Users/furture/CLionProjects/Tson/tsonjs.cpp

CMakeFiles/TsonProject.dir/tsonjs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TsonProject.dir/tsonjs.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/furture/CLionProjects/Tson/tsonjs.cpp > CMakeFiles/TsonProject.dir/tsonjs.cpp.i

CMakeFiles/TsonProject.dir/tsonjs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TsonProject.dir/tsonjs.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/furture/CLionProjects/Tson/tsonjs.cpp -o CMakeFiles/TsonProject.dir/tsonjs.cpp.s

CMakeFiles/TsonProject.dir/tsonjs.cpp.o.requires:

.PHONY : CMakeFiles/TsonProject.dir/tsonjs.cpp.o.requires

CMakeFiles/TsonProject.dir/tsonjs.cpp.o.provides: CMakeFiles/TsonProject.dir/tsonjs.cpp.o.requires
	$(MAKE) -f CMakeFiles/TsonProject.dir/build.make CMakeFiles/TsonProject.dir/tsonjs.cpp.o.provides.build
.PHONY : CMakeFiles/TsonProject.dir/tsonjs.cpp.o.provides

CMakeFiles/TsonProject.dir/tsonjs.cpp.o.provides.build: CMakeFiles/TsonProject.dir/tsonjs.cpp.o


# Object files for target TsonProject
TsonProject_OBJECTS = \
"CMakeFiles/TsonProject.dir/main.cpp.o" \
"CMakeFiles/TsonProject.dir/tson.c.o" \
"CMakeFiles/TsonProject.dir/tsonjsc.c.o" \
"CMakeFiles/TsonProject.dir/data.c.o" \
"CMakeFiles/TsonProject.dir/tsonjs.cpp.o"

# External object files for target TsonProject
TsonProject_EXTERNAL_OBJECTS =

TsonProject: CMakeFiles/TsonProject.dir/main.cpp.o
TsonProject: CMakeFiles/TsonProject.dir/tson.c.o
TsonProject: CMakeFiles/TsonProject.dir/tsonjsc.c.o
TsonProject: CMakeFiles/TsonProject.dir/data.c.o
TsonProject: CMakeFiles/TsonProject.dir/tsonjs.cpp.o
TsonProject: CMakeFiles/TsonProject.dir/build.make
TsonProject: CMakeFiles/TsonProject.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable TsonProject"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TsonProject.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TsonProject.dir/build: TsonProject

.PHONY : CMakeFiles/TsonProject.dir/build

CMakeFiles/TsonProject.dir/requires: CMakeFiles/TsonProject.dir/main.cpp.o.requires
CMakeFiles/TsonProject.dir/requires: CMakeFiles/TsonProject.dir/tson.c.o.requires
CMakeFiles/TsonProject.dir/requires: CMakeFiles/TsonProject.dir/tsonjsc.c.o.requires
CMakeFiles/TsonProject.dir/requires: CMakeFiles/TsonProject.dir/data.c.o.requires
CMakeFiles/TsonProject.dir/requires: CMakeFiles/TsonProject.dir/tsonjs.cpp.o.requires

.PHONY : CMakeFiles/TsonProject.dir/requires

CMakeFiles/TsonProject.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/TsonProject.dir/cmake_clean.cmake
.PHONY : CMakeFiles/TsonProject.dir/clean

CMakeFiles/TsonProject.dir/depend:
	cd /Users/furture/CLionProjects/Tson/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/furture/CLionProjects/Tson /Users/furture/CLionProjects/Tson /Users/furture/CLionProjects/Tson/cmake-build-debug /Users/furture/CLionProjects/Tson/cmake-build-debug /Users/furture/CLionProjects/Tson/cmake-build-debug/CMakeFiles/TsonProject.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TsonProject.dir/depend

