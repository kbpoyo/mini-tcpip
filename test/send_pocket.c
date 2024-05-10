#include <stdio.h>
#include "sys_plat.h"

int main (void) {
	// 打开物理网卡，设置好硬件地址
	static const uint8_t netdev0_hwaddr[] = { 0x00, 0x50, 0x56, 0xc0, 0x00, 0x11 };
	pcap_t* pcap = pcap_device_open("192.168.74.1", netdev0_hwaddr);

	int counter = 0;
	while (pcap && counter <= 30) {
		static uint8_t buf[1048];


		plat_printf("begin test: %d\n", counter++);
		
        for (int i = 0; i < sizeof(buf); i++) {
            buf[i] = i;
        }


		if (pcap_inject(pcap, buf, sizeof(buf)) == -1) {
			plat_printf("pcap send: send packet failed %s\n", pcap_geterr(pcap));
			return 1;
		}

		sys_sleep(100);
	}

	return 0;
}