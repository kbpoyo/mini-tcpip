
#include "dbg.h"
#include "mblock.h"
#include "sys_plat.h"

#define Block_Cnt 0xffff
#define Block_size 1024

static mblock_t blist;

static size_t thread1_block_cnt = 0;
static size_t thread2_block_cnt = 0;

void thread1_entry(void *arg) {
  for (int i = 0; i < Block_Cnt; i++) {
    void *p = mblock_alloc(&blist, -1);
    if (p) {
      thread1_block_cnt++;
    }
  }
}

void thread2_entry(void *arg) {
  for (int i = 0; i < Block_Cnt; i++) {
    void *p = mblock_alloc(&blist, -1);
    if (p) {
      thread2_block_cnt++;
    }
  }
}

int main(void) {
  static uint8_t buf[Block_Cnt][Block_size];
  mblock_init(&blist, buf, Block_size, Block_Cnt, NLOCKER_THREAD);

  sys_thread_t thread1 = sys_thread_create(thread1_entry, "aaaa");
  sys_thread_t thread2 = sys_thread_create(thread2_entry, "bbbb");

  sys_thread_join(thread1);
  sys_thread_join(thread2);

  plat_printf("thread1_block_cnt = %d, thread2_block_cnt = %d\n", thread1_block_cnt, thread2_block_cnt);

  return (thread1_block_cnt + thread2_block_cnt) == Block_Cnt ? 0 : 1;
}
