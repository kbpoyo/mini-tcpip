#include <stdint.h>
#include <stdio.h>

#include "sys_plat.h"

static sys_sem_t sem_read = SYS_SEM_INVALID;
static sys_sem_t sem_write = SYS_SEM_INVALID;
static sys_mutex_t mutex = SYS_MUTEx_INVALID;

#define Buffer_Size 100
static char buffer[Buffer_Size];
static int write_index = 0;
static int read_index = 0;
static int data_cnt = 0;

void thread1_entry(void *arg) {
  for (int i = 0; i < 2 * sizeof(buffer); i++) {
    sys_sem_wait(sem_read, 0);
    char data = buffer[read_index++];
    read_index %= Buffer_Size;

    sys_mutex_lock(mutex);
    data_cnt--;
    sys_mutex_unlock(mutex);
    
    plat_printf("thread1 read data = %d\n", (uint8_t)data);
    sys_sem_notify(sem_write);

    sys_sleep(10);
  }

  plat_printf("data_cnt = %d\n", data_cnt);
}

void thread2_entry(void *arg) {
  for (int i = 0; i < 2 * sizeof(buffer); i++) {
    sys_sem_wait(sem_write, 0);
    buffer[write_index++] = i;
    write_index %= Buffer_Size;
    
    sys_mutex_lock(mutex);
    data_cnt++;
    sys_mutex_unlock(mutex);

    plat_printf("thread2 write data = %d\n", i);
    sys_sem_notify(sem_read);
  }
}

int main(void) {
  sem_read = sys_sem_create(0);
  sem_write = sys_sem_create(Buffer_Size);
  mutex = sys_mutex_create();

  sys_thread_create(thread1_entry, "aaaa");
  sys_thread_create(thread2_entry, "bbbb");

  // 打开物理网卡，设置好硬件地址
  static const uint8_t netdev0_hwaddr[] = {0x00, 0x50, 0x56, 0xc0, 0x00, 0x11};
  pcap_t *pcap = pcap_device_open("192.168.74.1", netdev0_hwaddr);

  while (pcap) {
    static uint8_t buf[1048];

    static int counter = 0;
    struct pcap_pkthdr *pkthdr;
    const uint8_t *pkt_data;

    plat_printf("begin test: %d\n", counter++);

    // 接收数据包
    if (pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
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