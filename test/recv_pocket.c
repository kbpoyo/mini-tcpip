#include <stdio.h>
#include "sys_plat.h"

int main (void) {
	//以下是测试代码，可以删掉
	//打开物理网卡，设置好硬件地址
	
	static const uint8_t netdev0_hwaddr[] = { 0x00, 0x50, 0x56, 0xc0, 0x00, 0x11 };
	pcap_t* pcap = pcap_device_open("192.168.74.1", netdev0_hwaddr);

	while (pcap) {
		static uint8_t buf[1048];

		static int counter = 0;
		struct pcap_pkthdr *pkthdr;
		const uint8_t *pkt_data;

		plat_printf("begin test: %d\n", counter++);
		
		// 接收数据包
		if(pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
			continue;
		}

		int len = pkthdr->len > sizeof(buf) ? sizeof(buf) : pkthdr->len;
		plat_memcpy(buf, pkt_data, len);

		buf[0] = 0x55;
		buf[1] = 0xaa;


		if (pcap_inject(pcap, buf, len) == -1) {
			plat_printf("pcap send: send packet failed %s\n", pcap_geterr(pcap));
			break;
		}
	}


	return 0;
}