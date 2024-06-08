/**
 * @file timer.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 定时器模块
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "timer.h"

#include "dbg.h"
#include "net_sys.h"

static nlist_t timer_list;  // 定时器链表

#if DBG_DISP_ENABLED(DBG_TIMER)
/**
 * @brief 遍历显示定时器链表
 *
 */
static void display_timer_list(void) {
  plat_printf("--------------timer list--------------\n");
  nlist_node_t *node;
  net_timer_t *timer;

  int index = 0;
  nlist_for_each(node, &timer_list) {
    timer = nlist_entry(node, net_timer_t, node);
    plat_printf(
        "%d: %s, handle=%p, curr_ticks=%dms, reload_ticks=%dms, "
        "flag_reload=%d\n",
        index, timer->name, timer->handle, timer->curr_ticks,
        timer->reload_ticks, timer->flags & NET_TIMER_RELOAD ? 1 : 0);
  }

  plat_printf("-----------timer list end------------\n");
}

#else
#define display_timer_list()
#endif

/**
 * @brief 初始化定时器模块
 *
 * @return net_err_t
 */
net_err_t net_timer_module_init(void) {
  dbg_info(DBG_TIMER, "init timer module....");

  nlist_init(&timer_list);  // 初始化定时器链表

  dbg_info(DBG_TIMER, "init timer module ok.");
  return NET_ERR_OK;
}

static void insert_timer(net_timer_t *timer) {
  nlist_node_t *node = (nlist_node_t *)0;
  net_timer_t *curr_timer = (net_timer_t *)0;

  // 遍历链表，找到合适的位置插入
  nlist_for_each(node, &timer_list) {
    curr_timer = nlist_entry(node, net_timer_t, node);

    if (timer->curr_ticks > curr_timer->curr_ticks) {
      // 更新timer的curr_ticks
      timer->curr_ticks -= curr_timer->curr_ticks;
    } else if (timer->curr_ticks == curr_timer->curr_ticks) {
      // 更新timer的curr_ticks
      timer->curr_ticks = 0;
      // 将timer插入到curr_timer之后
      nlist_insert_after(&timer_list, &curr_timer->node, &timer->node);
      break;
    } else {
      // 将timer插入到curr_timer之前
      nlist_insert_before(&timer_list, &curr_timer->node, &timer->node);
      break;
    }
  }

  if (!node) {  // 遍历完链表，定时器需要插入到链表末尾
    nlist_insert_last(&timer_list, &timer->node);
  } else {  // 遍历到链表中间，需要更新后续定时器的curr_ticks
    while (curr_timer && timer->curr_ticks > 0) {
      curr_timer->curr_ticks -= timer->curr_ticks;

      curr_timer =
          nlist_entry(nlist_node_next(&curr_timer->node), net_timer_t, node);
    }
  }
}

/**
 * @brief 添加一个定时器
 *
 * @param timer
 * @param name
 * @param handle
 * @param arg
 * @param ms
 * @param flags
 * @return net_err_t
 */
net_err_t net_timer_add(net_timer_t *timer, const char *name,
                        timer_handle_t handle, void *arg, int ms, int flags) {
  dbg_info(DBG_TIMER, "add timer %s, ms=%d, flags=%d", name, ms, flags);

  if (timer == (net_timer_t *)0 || name == (char *)0 ||
      handle == (timer_handle_t)0 || ms < 0) {
    return NET_ERR_PARAM;
  }

  // 初始化定时器
  plat_strncpy(timer->name, name, TIMER_NAME_SIZE);
  timer->name[TIMER_NAME_SIZE - 1] = '\0';
  timer->flags = flags;
  timer->curr_ticks = ms;
  timer->reload_ticks = ms;
  timer->handle = handle;
  timer->arg = arg;

  // 加入定时器链表
  insert_timer(timer);

  display_timer_list();

  return NET_ERR_OK;
}