# CMake generated Testfile for 
# Source directory: C:/Users/kbpoyo/Desktop/mini-tcpip/test
# Build directory: C:/Users/kbpoyo/Desktop/mini-tcpip/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[test1]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test1.exe")
set_tests_properties([=[test1]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;36;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[send_pocket]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/send_pocket.exe")
set_tests_properties([=[send_pocket]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;41;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[recv_pocket]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/recv_pocket.exe")
set_tests_properties([=[recv_pocket]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;46;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[multi_thread]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/multi_thread.exe")
set_tests_properties([=[multi_thread]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;52;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[circular_queue]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/circular_queue.exe")
set_tests_properties([=[circular_queue]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;56;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_tcp_echo_client]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test_tcp_echo_client.exe")
set_tests_properties([=[test_tcp_echo_client]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;61;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_tcp_echo_server]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test_tcp_echo_server.exe")
set_tests_properties([=[test_tcp_echo_server]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;66;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_mblock]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test_mblock.exe")
set_tests_properties([=[test_mblock]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;71;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_pktbuf]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test_pktbuf.exe")
set_tests_properties([=[test_pktbuf]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;76;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
add_test([=[test_ping]=] "C:/Users/kbpoyo/Desktop/mini-tcpip/build/test/test_ping.exe")
set_tests_properties([=[test_ping]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;81;add_test;C:/Users/kbpoyo/Desktop/mini-tcpip/test/CMakeLists.txt;0;")
