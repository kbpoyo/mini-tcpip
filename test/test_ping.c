#include "ping/ping.h"

int main(void) {

    ping_t ping;

    ping_run(&ping, "192.168.3.159", 1024, 4, 1000);


    return 0;
}