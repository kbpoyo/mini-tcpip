#include <stdint.h>
#include <stdio.h>

#include "sys_plat.h"
#include "echo/tcp_echo_server.h"

int main(void) {
  tcp_echo_server_start(5000);


  return 0;
}