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
include test/CMakeFiles/recv_pocket.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/recv_pocket.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/recv_pocket.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/recv_pocket.dir/flags.make

test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj: F:/code/c++code/mini-tcpip/test/recv_pocket.c
test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj -MF CMakeFiles\recv_pocket.dir\recv_pocket.c.obj.d -o CMakeFiles\recv_pocket.dir\recv_pocket.c.obj -c F:\code\c++code\mini-tcpip\test\recv_pocket.c

test/CMakeFiles/recv_pocket.dir/recv_pocket.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/recv_pocket.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\test\recv_pocket.c > CMakeFiles\recv_pocket.dir\recv_pocket.c.i

test/CMakeFiles/recv_pocket.dir/recv_pocket.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/recv_pocket.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\test\recv_pocket.c -o CMakeFiles\recv_pocket.dir\recv_pocket.c.s

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj: F:/code/c++code/mini-tcpip/src/app/echo/tcp_echo_client.c
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_client.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_client.c.obj -c F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c > CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_client.c.i

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_client.c -o CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_client.c.s

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj: F:/code/c++code/mini-tcpip/src/app/echo/tcp_echo_server.c
test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_server.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_server.c.obj -c F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c > CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_server.c.i

test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\app\echo\tcp_echo_server.c -o CMakeFiles\recv_pocket.dir\__\src\app\echo\tcp_echo_server.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj: F:/code/c++code/mini-tcpip/src/net/src/dbg.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\dbg.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\dbg.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\dbg.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\dbg.c > CMakeFiles\recv_pocket.dir\__\src\net\src\dbg.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\dbg.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\dbg.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj: F:/code/c++code/mini-tcpip/src/net/src/exmsg.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\exmsg.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\exmsg.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\exmsg.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\exmsg.c > CMakeFiles\recv_pocket.dir\__\src\net\src\exmsg.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\exmsg.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\exmsg.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj: F:/code/c++code/mini-tcpip/src/net/src/fixq.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\fixq.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\fixq.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\fixq.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\fixq.c > CMakeFiles\recv_pocket.dir\__\src\net\src\fixq.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\fixq.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\fixq.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj: F:/code/c++code/mini-tcpip/src/net/src/mblock.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\mblock.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\mblock.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\mblock.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\mblock.c > CMakeFiles\recv_pocket.dir\__\src\net\src\mblock.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\mblock.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\mblock.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj: F:/code/c++code/mini-tcpip/src/net/src/net.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\net.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\net.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\net.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\net.c > CMakeFiles\recv_pocket.dir\__\src\net\src\net.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\net.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\net.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj: F:/code/c++code/mini-tcpip/src/net/src/nlist.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\nlist.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\nlist.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\nlist.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\nlist.c > CMakeFiles\recv_pocket.dir\__\src\net\src\nlist.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\nlist.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\nlist.c.s

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj: F:/code/c++code/mini-tcpip/src/net/src/nlocker.c
test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\net\src\nlocker.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\net\src\nlocker.c.obj -c F:\code\c++code\mini-tcpip\src\net\src\nlocker.c

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\net\src\nlocker.c > CMakeFiles\recv_pocket.dir\__\src\net\src\nlocker.c.i

test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\net\src\nlocker.c -o CMakeFiles\recv_pocket.dir\__\src\net\src\nlocker.c.s

test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj: F:/code/c++code/mini-tcpip/src/plat/net_plat.c
test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\plat\net_plat.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\plat\net_plat.c.obj -c F:\code\c++code\mini-tcpip\src\plat\net_plat.c

test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\net_plat.c > CMakeFiles\recv_pocket.dir\__\src\plat\net_plat.c.i

test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\net_plat.c -o CMakeFiles\recv_pocket.dir\__\src\plat\net_plat.c.s

test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj: F:/code/c++code/mini-tcpip/src/plat/netif_pcap.c
test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\plat\netif_pcap.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\plat\netif_pcap.c.obj -c F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c

test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c > CMakeFiles\recv_pocket.dir\__\src\plat\netif_pcap.c.i

test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\netif_pcap.c -o CMakeFiles\recv_pocket.dir\__\src\plat\netif_pcap.c.s

test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/recv_pocket.dir/flags.make
test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/recv_pocket.dir/includes_C.rsp
test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj: F:/code/c++code/mini-tcpip/src/plat/sys_plat.c
test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj: test/CMakeFiles/recv_pocket.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building C object test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj -MF CMakeFiles\recv_pocket.dir\__\src\plat\sys_plat.c.obj.d -o CMakeFiles\recv_pocket.dir\__\src\plat\sys_plat.c.obj -c F:\code\c++code\mini-tcpip\src\plat\sys_plat.c

test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.i"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\code\c++code\mini-tcpip\src\plat\sys_plat.c > CMakeFiles\recv_pocket.dir\__\src\plat\sys_plat.c.i

test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.s"
	cd /d F:\code\c++code\mini-tcpip\build\test && "F:\software\work_space\Toolbox _APP\CLion Nova\bin\mingw\bin\gcc.exe" $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\code\c++code\mini-tcpip\src\plat\sys_plat.c -o CMakeFiles\recv_pocket.dir\__\src\plat\sys_plat.c.s

# Object files for target recv_pocket
recv_pocket_OBJECTS = \
"CMakeFiles/recv_pocket.dir/recv_pocket.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj" \
"CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj"

# External object files for target recv_pocket
recv_pocket_EXTERNAL_OBJECTS =

test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/recv_pocket.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_client.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/app/echo/tcp_echo_server.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/dbg.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/exmsg.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/fixq.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/mblock.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/net.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlist.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/net/src/nlocker.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/plat/net_plat.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/plat/netif_pcap.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/__/src/plat/sys_plat.c.obj
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/build.make
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/linkLibs.rsp
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/objects1.rsp
test/recv_pocket.exe: test/CMakeFiles/recv_pocket.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=F:\code\c++code\mini-tcpip\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Linking C executable recv_pocket.exe"
	cd /d F:\code\c++code\mini-tcpip\build\test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\recv_pocket.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/recv_pocket.dir/build: test/recv_pocket.exe
.PHONY : test/CMakeFiles/recv_pocket.dir/build

test/CMakeFiles/recv_pocket.dir/clean:
	cd /d F:\code\c++code\mini-tcpip\build\test && $(CMAKE_COMMAND) -P CMakeFiles\recv_pocket.dir\cmake_clean.cmake
.PHONY : test/CMakeFiles/recv_pocket.dir/clean

test/CMakeFiles/recv_pocket.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\code\c++code\mini-tcpip F:\code\c++code\mini-tcpip\test F:\code\c++code\mini-tcpip\build F:\code\c++code\mini-tcpip\build\test F:\code\c++code\mini-tcpip\build\test\CMakeFiles\recv_pocket.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/recv_pocket.dir/depend

