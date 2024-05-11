# cmake最低版本要求
cmake_minimum_required(VERSION 3.22.1)

# 设置工程的名字，生成的可执行文件也将使用该名称，如net.exe
project(test)


message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

list(REMOVE_ITEM SOURCE_LIST ${CMAKE_SOURCE_DIR}/src/app/main.c)

# 将源文件列表加入到工程中
add_executable(test1 "test.c" ${SOURCE_LIST})
add_executable(send_pocket "send_pocket.c" ${SOURCE_LIST})
add_executable(recv_pocket "recv_pocket.c" ${SOURCE_LIST})
add_executable(multi_thread "multi_thread.c" ${SOURCE_LIST})
add_executable(circular_queue "circular_queue.c" ${SOURCE_LIST})
add_executable(test_tcp_echo_client "test_tcp_echo_client.c" ${SOURCE_LIST})
add_executable(test_tcp_echo_server "test_tcp_echo_server.c" ${SOURCE_LIST})

target_link_libraries(test1 ${LINK_LIBS_LIST})
target_link_libraries(send_pocket ${LINK_LIBS_LIST})
target_link_libraries(recv_pocket ${LINK_LIBS_LIST})
target_link_libraries(multi_thread ${LINK_LIBS_LIST})
target_link_libraries(circular_queue ${LINK_LIBS_LIST})
target_link_libraries(test_tcp_echo_client ${LINK_LIBS_LIST})
target_link_libraries(test_tcp_echo_server ${LINK_LIBS_LIST})

add_test(
  NAME test1
  COMMAND $<TARGET_FILE:test1>
)

add_test(
  NAME send_pocket
  COMMAND $<TARGET_FILE:send_pocket>
)

add_test(
  NAME recv_pocket
  COMMAND $<TARGET_FILE:recv_pocket>
)
set_tests_properties(recv_pocket PROPERTIES TIMEOUT 30)

add_test(
  NAME multi_thread
  COMMAND $<TARGET_FILE:multi_thread>
)
add_test(
  NAME circular_queue
  COMMAND $<TARGET_FILE:circular_queue>
)

add_test(
  NAME test_tcp_echo_client
  COMMAND $<TARGET_FILE:test_tcp_echo_client>
)

add_test(
  NAME test_tcp_echo_server
  COMMAND $<TARGET_FILE:test_tcp_echo_server>
)