# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.27

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\code\c++code\mini-tcpip

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\code\c++code\mini-tcpip\build

# Include any dependencies generated for this target.
include CMakeFiles/net.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/net.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/net.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/net.dir/flags.make

CMakeFiles/net.dir/src/app/test/main.c.obj: CMakeFiles/net.dir/flags.make
CMakeFiles/net.dir/src/app/test/main.c.obj: CMakeFiles/net.dir/includes_C.rsp
CMakeFiles/net.dir/src/app/test/main.c.obj: F:/code/c++code/mini-tcpip/src/app/test/main.c
CMakeFiles/net.dir/src/app/test/main.c.obj: CMakeFiles/net.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/net.dir/src/app/test/main.c.obj"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/net.dir/src/app/test/main.c.obj -MF CMakeFiles\net.dir\src\app\test\main.c.obj.d -o CMakeFiles\net.dir\src\app\test\main.c.obj -c F:\code\c++code\mini-tcpip\src\app\test\main.c

CMakeFiles/net.dir/src/app/test/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/net.dir/src/app/test/main.c.i"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\app\test\main.c > CMakeFiles\net.dir\src\app\test\main.c.i

CMakeFiles/net.dir/src/app/test/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/net.dir/src/app/test/main.c.s"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\app\test\main.c -o CMakeFiles\net.dir\src\app\test\main.c.s

CMakeFiles/net.dir/src/plat/sys_plat.c.obj: CMakeFiles/net.dir/flags.make
CMakeFiles/net.dir/src/plat/sys_plat.c.obj: CMakeFiles/net.dir/includes_C.rsp
CMakeFiles/net.dir/src/plat/sys_plat.c.obj: F:/code/c++code/mini-tcpip/src/plat/sys_plat.c
CMakeFiles/net.dir/src/plat/sys_plat.c.obj: CMakeFiles/net.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/net.dir/src/plat/sys_plat.c.obj"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/net.dir/src/plat/sys_plat.c.obj -MF CMakeFiles\net.dir\src\plat\sys_plat.c.obj.d -o CMakeFiles\net.dir\src\plat\sys_plat.c.obj -c F:\code\c++code\mini-tcpip\src\plat\sys_plat.c

CMakeFiles/net.dir/src/plat/sys_plat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/net.dir/src/plat/sys_plat.c.i"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\sys_plat.c > CMakeFiles\net.dir\src\plat\sys_plat.c.i

CMakeFiles/net.dir/src/plat/sys_plat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/net.dir/src/plat/sys_plat.c.s"
	"F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\sys_plat.c -o CMakeFiles\net.dir\src\plat\sys_plat.c.s

# Object files for target net
net_OBJECTS = \
"CMakeFiles/net.dir/src/app/test/main.c.obj" \
"CMakeFiles/net.dir/src/plat/sys_plat.c.obj"

# External object files for target net
net_EXTERNAL_OBJECTS =

net.exe: CMakeFiles/net.dir/src/app/test/main.c.obj
net.exe: CMakeFiles/net.dir/src/plat/sys_plat.c.obj
net.exe: CMakeFiles/net.dir/build.make
net.exe: CMakeFiles/net.dir/linkLibs.rsp
net.exe: CMakeFiles/net.dir/objects1.rsp
net.exe: CMakeFiles/net.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable net.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\net.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/net.dir/build: net.exe
.PHONY : CMakeFiles/net.dir/build

CMakeFiles/net.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\net.dir\cmake_clean.cmake
.PHONY : CMakeFiles/net.dir/clean

CMakeFiles/net.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\code\c++code\mini-tcpip F:\code\c++code\mini-tcpip F:\code\c++code\mini-tcpip\build F:\code\c++code\mini-tcpip\build F:\code\c++code\mini-tcpip\build\CMakeFiles\net.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/net.dir/depend

