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
include test/CMakeFiles/test1.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/test1.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test1.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test1.dir/flags.make

test/CMakeFiles/test1.dir/test.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/test.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/test.c.obj: F:/code/c++code/mini-tcpip/test/test.c
test/CMakeFiles/test1.dir/test.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/test1.dir/test.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/test.c.obj -MF CMakeFiles\test1.dir\test.c.obj.d -o CMakeFiles\test1.dir\test.c.obj -c F:\code\c++code\mini-tcpip\test\test.c

test/CMakeFiles/test1.dir/test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/test.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\test\test.c > CMakeFiles\test1.dir\test.c.i

test/CMakeFiles/test1.dir/test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/test.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\test\test.c -o CMakeFiles\test1.dir\test.c.s

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj: F:/code/c++code/mini-tcpip/src/app/echo/tcp_echo_client.c
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj -MF CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_client.c.obj.d -o CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_client.c.obj -c F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c > CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_client.c.i

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c -o CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_client.c.s

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj: F:/code/c++code/mini-tcpip/src/app/echo/tcp_echo_server.c
test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj -MF CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_server.c.obj.d -o CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_server.c.obj -c F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c > CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_server.c.i

test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c -o CMakeFiles\test1.dir\__\src\app\echo\tcp_echo_server.c.s

test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj: F:/code/c++code/mini-tcpip/src/net/src/dbg.c
test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\dbg.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\dbg.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\dbg.c

test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/dbg.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\dbg.c > CMakeFiles\test1.dir\__\src\net\src\dbg.c.i

test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/dbg.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\dbg.c -o CMakeFiles\test1.dir\__\src\net\src\dbg.c.s

test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj: F:/code/c++code/mini-tcpip/src/net/src/exmsg.c
test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\exmsg.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\exmsg.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\exmsg.c

test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/exmsg.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\exmsg.c > CMakeFiles\test1.dir\__\src\net\src\exmsg.c.i

test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/exmsg.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\exmsg.c -o CMakeFiles\test1.dir\__\src\net\src\exmsg.c.s

test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj: F:/code/c++code/mini-tcpip/src/net/src/fixq.c
test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\fixq.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\fixq.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\fixq.c

test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/fixq.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\fixq.c > CMakeFiles\test1.dir\__\src\net\src\fixq.c.i

test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/fixq.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\fixq.c -o CMakeFiles\test1.dir\__\src\net\src\fixq.c.s

test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj: F:/code/c++code/mini-tcpip/src/net/src/mblock.c
test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\mblock.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\mblock.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\mblock.c

test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/mblock.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\mblock.c > CMakeFiles\test1.dir\__\src\net\src\mblock.c.i

test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/mblock.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\mblock.c -o CMakeFiles\test1.dir\__\src\net\src\mblock.c.s

test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj: F:/code/c++code/mini-tcpip/src/net/src/net.c
test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\net.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\net.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\net.c

test/CMakeFiles/test1.dir/__/src/net/src/net.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/net.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\net.c > CMakeFiles\test1.dir\__\src\net\src\net.c.i

test/CMakeFiles/test1.dir/__/src/net/src/net.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/net.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\net.c -o CMakeFiles\test1.dir\__\src\net\src\net.c.s

test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj: F:/code/c++code/mini-tcpip/src/net/src/nlist.c
test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\nlist.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\nlist.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\nlist.c

test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/nlist.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\nlist.c > CMakeFiles\test1.dir\__\src\net\src\nlist.c.i

test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/nlist.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\nlist.c -o CMakeFiles\test1.dir\__\src\net\src\nlist.c.s

test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj: F:/code/c++code/mini-tcpip/src/net/src/nlocker.c
test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\nlocker.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\nlocker.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\nlocker.c

test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/nlocker.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\nlocker.c > CMakeFiles\test1.dir\__\src\net\src\nlocker.c.i

test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/nlocker.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\nlocker.c -o CMakeFiles\test1.dir\__\src\net\src\nlocker.c.s

test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj: F:/code/c++code/mini-tcpip/src/net/src/pktbuf.c
test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building C object test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj -MF CMakeFiles\test1.dir\__\src\net\src\pktbuf.c.obj.d -o CMakeFiles\test1.dir\__\src\net\src\pktbuf.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\pktbuf.c

test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\pktbuf.c > CMakeFiles\test1.dir\__\src\net\src\pktbuf.c.i

test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\pktbuf.c -o CMakeFiles\test1.dir\__\src\net\src\pktbuf.c.s

test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj: F:/code/c++code/mini-tcpip/src/plat/net_plat.c
test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building C object test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj -MF CMakeFiles\test1.dir\__\src\plat\net_plat.c.obj.d -o CMakeFiles\test1.dir\__\src\plat\net_plat.c.obj -c F:\code\c++code\mini-tcpip\src\plat\net_plat.c

test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/plat/net_plat.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\net_plat.c > CMakeFiles\test1.dir\__\src\plat\net_plat.c.i

test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/plat/net_plat.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\net_plat.c -o CMakeFiles\test1.dir\__\src\plat\net_plat.c.s

test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj: F:/code/c++code/mini-tcpip/src/plat/netif_pcap.c
test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building C object test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj -MF CMakeFiles\test1.dir\__\src\plat\netif_pcap.c.obj.d -o CMakeFiles\test1.dir\__\src\plat\netif_pcap.c.obj -c F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c

test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c > CMakeFiles\test1.dir\__\src\plat\netif_pcap.c.i

test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c -o CMakeFiles\test1.dir\__\src\plat\netif_pcap.c.s

test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/test1.dir/flags.make
test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/test1.dir/includes_C.rsp
test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj: F:/code/c++code/mini-tcpip/src/plat/sys_plat.c
test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/test1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building C object test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj -MF CMakeFiles\test1.dir\__\src\plat\sys_plat.c.obj.d -o CMakeFiles\test1.dir\__\src\plat\sys_plat.c.obj -c F:\code\c++code\mini-tcpip\src\plat\sys_plat.c

test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/test1.dir/__/src/plat/sys_plat.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\sys_plat.c > CMakeFiles\test1.dir\__\src\plat\sys_plat.c.i

test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/test1.dir/__/src/plat/sys_plat.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\sys_plat.c -o CMakeFiles\test1.dir\__\src\plat\sys_plat.c.s

# Object files for target test1
test1_OBJECTS = \
"CMakeFiles/test1.dir/test.c.obj" \
"CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj" \
"CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/net.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj" \
"CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj" \
"CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj" \
"CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj" \
"CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj"

# External object files for target test1
test1_EXTERNAL_OBJECTS =

test/test1.exe: test/CMakeFiles/test1.dir/test.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_client.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/app/echo/tcp_echo_server.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/dbg.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/exmsg.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/fixq.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/mblock.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/net.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/nlist.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/nlocker.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/net/src/pktbuf.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/plat/net_plat.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/plat/netif_pcap.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/__/src/plat/sys_plat.c.obj
test/test1.exe: test/CMakeFiles/test1.dir/build.make
test/test1.exe: test/CMakeFiles/test1.dir/linkLibs.rsp
test/test1.exe: test/CMakeFiles/test1.dir/objects1.rsp
test/test1.exe: test/CMakeFiles/test1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Linking C executable test1.exe"
	cd /d F:\code\c++code\mini-tcpip\build\test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\test1.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test1.dir/build: test/test1.exe
.PHONY : test/CMakeFiles/test1.dir/build

test/CMakeFiles/test1.dir/clean:
	cd /d F:\code\c++code\mini-tcpip\build\test && $(CMAKE_COMMAND) -P CMakeFiles\test1.dir\cmake_clean.cmake
.PHONY : test/CMakeFiles/test1.dir/clean

test/CMakeFiles/test1.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\code\c++code\mini-tcpip F:\code\c++code\mini-tcpip\test F:\code\c++code\mini-tcpip\build F:\code\c++code\mini-tcpip\build\test F:\code\c++code\mini-tcpip\build\test\CMakeFiles\test1.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/test1.dir/depend

