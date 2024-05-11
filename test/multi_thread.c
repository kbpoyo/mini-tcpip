#include <stdio.h>

#include "sys_plat.h"

static sys_mutex_t mutex = SYS_MUTEx_INVALID;

int count = 0;


void thread1_entry(void *arg) {
	for (int i = 0; i < 1000000; i++) {
		sys_mutex_lock(mutex);
		count++;
		sys_mutex_unlock(mutex);
	}


    sys_thread_exit(0);
}

void thread2_entry(void *arg) {
	for (int i = 0; i < 1000000; i++) {
		sys_mutex_lock(mutex);
		count--;
		sys_mutex_unlock(mutex);
	}
    sys_thread_exit(0);
}

int main(void) {
	mutex = sys_mutex_create();

	sys_thread_t thread1 = sys_thread_create(thread1_entry, "aaaa");
	sys_thread_t thread2 = sys_thread_create(thread2_entry, "bbbb");

    sys_thread_join(thread1);
    sys_thread_join(thread2);

    plat_printf("count = %d\n", count);
    return count;
}