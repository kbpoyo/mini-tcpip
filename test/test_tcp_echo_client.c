
#include <stdint.h>
#include <stdio.h>

#include "sys_plat.h"
#include "echo/tcp_echo_client.h"

int main(void) {
//   tcp_echo_client_start(friend0_ip, 5000);
  tcp_echo_client_start("127.0.0.1", 5000);


  return 0;
}