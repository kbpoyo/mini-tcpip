# cmake最低版本要求
cmake_minimum_required(VERSION 3.22.1)

# 设置工程的名字，生成的可执行文件也将使用该名称，如net.exe
project(net)
set(CMAKE_C_STANDARD 99)

LINK_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/npcap/Lib/x64          # win64使用
    #${PROJECT_SOURCE_DIR}/lib/npcap/Lib/             # win32使用
)


#使用visual studio 时定义该宏，以消除不安全函数警告
# add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#使用GCC时，Enable GCC debug
add_compile_options(-g)
# add_compile_options(-Wall)



# 头文件搜索路径，使得c文件中可以直接使用#include "xxx.h"，而不必加前缀
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/npcap/Include          # pcap路径
        ${PROJECT_SOURCE_DIR}/src/net/net 
        ${PROJECT_SOURCE_DIR}/src/plat
        ${PROJECT_SOURCE_DIR}/src/app
)

# 搜索src下面的所有源文件，生成文件列表
file(GLOB_RECURSE SOURCE_LIST "src/*.c" "src/*.h")

# 将源文件列表加入到工程中
add_executable(${PROJECT_NAME} ${SOURCE_LIST})

# 与一些库进行编译链接，生成最终的程序
# 针对网络的平台配置
add_definitions(-DNET_DRIVER_PCAP)    # use pcap

message(STATUS "current platform: ${CMAKE_HOST_SYSTEM_NAME}")
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-DSYS_PLAT_WINDOWS)
    set(LINK_LIBS_LIST wpcap packet Ws2_32)
    target_link_libraries(${PROJECT_NAME} ${LINK_LIBS_LIST})
else()
    # Linux和Mac上的特定配置
    add_definitions(-DSYS_PLAT_LINUX)
    target_link_libraries(${PROJECT_NAME} pthread pcap)
endif()


enable_testing()
add_subdirectory(test)


