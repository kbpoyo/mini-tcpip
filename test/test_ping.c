#include "ping/ping.h"

int main(void) {

    ping_t ping;

    ping_run(&ping, "192.168.74.3", 1024, 4, 1000);


    return 0;
}