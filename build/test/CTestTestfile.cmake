# CMake generated Testfile for 
# Source directory: F:/code/c++code/mini-tcpip/test
# Build directory: F:/code/c++code/mini-tcpip/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[test1]=] "F:/code/c++code/mini-tcpip/build/test/test1.exe")
set_tests_properties([=[test1]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;22;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[send_pocket]=] "F:/code/c++code/mini-tcpip/build/test/send_pocket.exe")
set_tests_properties([=[send_pocket]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;27;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[recv_pocket]=] "F:/code/c++code/mini-tcpip/build/test/recv_pocket.exe")
set_tests_properties([=[recv_pocket]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;32;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
