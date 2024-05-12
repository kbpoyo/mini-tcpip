#include <stdint.h>
#include <stdio.h>

#include "sys_plat.h"

static sys_sem_t sem_read = SYS_SEM_INVALID;
static sys_sem_t sem_write = SYS_SEM_INVALID;
static sys_mutex_t mutex = SYS_MUTEX_INVALID;

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

    sys_sleep(5);
  }
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

  sys_thread_t thread1 = sys_thread_create(thread1_entry, "aaaa");
  sys_thread_t thread2 = sys_thread_create(thread2_entry, "bbbb");

  sys_thread_join(thread1);
  sys_thread_join(thread2);

  plat_printf("data_cnt = %d\n", data_cnt);
  return data_cnt;
}