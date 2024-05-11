# CMake generated Testfile for 
# Source directory: F:/code/c++code/mini-tcpip/test
# Build directory: F:/code/c++code/mini-tcpip/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[test1]=] "F:/code/c++code/mini-tcpip/build/test/test1.exe")
set_tests_properties([=[test1]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;30;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[send_pocket]=] "F:/code/c++code/mini-tcpip/build/test/send_pocket.exe")
set_tests_properties([=[send_pocket]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;35;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[recv_pocket]=] "F:/code/c++code/mini-tcpip/build/test/recv_pocket.exe")
set_tests_properties([=[recv_pocket]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;40;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[multi_thread]=] "F:/code/c++code/mini-tcpip/build/test/multi_thread.exe")
set_tests_properties([=[multi_thread]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;46;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[circular_queue]=] "F:/code/c++code/mini-tcpip/build/test/circular_queue.exe")
set_tests_properties([=[circular_queue]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;50;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_tcp_echo_client]=] "F:/code/c++code/mini-tcpip/build/test/test_tcp_echo_client.exe")
set_tests_properties([=[test_tcp_echo_client]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;55;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_tcp_echo_server]=] "F:/code/c++code/mini-tcpip/build/test/test_tcp_echo_server.exe")
set_tests_properties([=[test_tcp_echo_server]=] PROPERTIES  _BACKTRACE_TRIPLES "F:/code/c++code/mini-tcpip/test/CMakeLists.txt;60;add_test;F:/code/c++code/mini-tcpip/test/CMakeLists.txt;0;")
