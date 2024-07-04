/**
 * @file timer.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 定时器模块, 采用定时器链表, 按照定时器的定时时长由低到高插入链表,
 * 支持定时器重载
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "timer.h"

#include "dbg.h"
#include "net_sys.h"

static nlist_t timer_list;     // 定时器工作链表
static nlist_t overtime_list;  // 定时器超时链表

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

/**
 * @brief 按照定时时长插入定时器链表，且在插入后，
 * 每个定时器的定时时长为当前定时器及之前的定时器的curr_ticks之和。
 * 
 * (timer_A, 10s) -> (timer_B, 20s) -> (timer_C, 0s) -> (timer_D, 30s)
 * 
 * timer_A定时时长为10s, timer_B和timer_C定时时长为30s, timer_D定时时长为60s
 * 
 * @param timer 
 */
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
      // 更新curr_timer的curr_ticks
      curr_timer->curr_ticks -= timer->curr_ticks;
      break;
    }
  }

  if (!node) {  // 遍历完链表，证明定时器无法插入到链表中间,
                // 需将定时器需要插入到链表末尾
    nlist_insert_last(&timer_list, &timer->node);
  }
}

/**
 * @brief 添加一个定时器
 *
 * @param timer 定时器对象
 * @param name 定时器名称
 * @param handle 定时器回调函数
 * @param arg 回调函数参数
 * @param ms 定时时长
 * @param flags 定时器标志
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
  if (flags & NET_TIMER_ACTIVE) {  // 激活定时器
    insert_timer(timer);
  }

  display_timer_list();

  return NET_ERR_OK;
}

/**
 * @brief 删除一个定时器
 *
 * @param timer
 */
void net_timer_remove(net_timer_t *timer) {
  // 定时器为空或者未激活
  if (!timer || !(timer->flags & NET_TIMER_ACTIVE)) {
    dbg_warning(DBG_TIMER, "remove timer failed: timer is null or not active");
    return;
  }

  dbg_info(DBG_TIMER, "remove timer %s", timer->name);

  // 获取timer的后一个定时器
  net_timer_t *next_timer =
      nlist_entry(nlist_node_next(&timer->node), net_timer_t, node);

  // 从链表中删除timer
  nlist_remove(&timer_list, &timer->node);
  timer->flags &= ~NET_TIMER_ACTIVE;  // 清除激活标志

  // 更新next_timer的curr_ticks
  if (next_timer) {
    next_timer->curr_ticks += timer->curr_ticks;
  }

  display_timer_list();
}

/**
 * @brief 扫描定时器链表，检查是否有定时器需要触发
 *
 * @param diff_ms 与上次扫描的时间间隔
 * @return net_err_t
 */
net_err_t net_timer_check_tmo(int diff_ms) {
  if (diff_ms <= 0) {
    return NET_ERR_PARAM;
  }

  nlist_node_t *node = nlist_first(&timer_list);
  net_timer_t *timer = (net_timer_t *)0;

  // 遍历定时器链表
  while (node) {
    timer = nlist_entry(node, net_timer_t, node);
    node = nlist_node_next(node);

    if (timer->curr_ticks <= diff_ms) {  // timer需要触发
      diff_ms -= timer->curr_ticks;  // 更新时间间隔, 以便继续扫描后面的定时器

      // 定时器超时, 移除定时器并加入超时链表
      timer->curr_ticks = 0;
      net_timer_remove(timer);
      nlist_insert_last(&overtime_list, &timer->node);

    } else {  // timer及后面的定时器不需要触发
      timer->curr_ticks -= diff_ms;
      break;
    }
  }

  // 遍历超时链表, 调用定时器回调函数
  while ((node = nlist_remove_first(&overtime_list)) != (nlist_node_t *)0) {
    timer = nlist_entry(node, net_timer_t, node);
    timer->handle(timer, timer->arg);

    if (timer->flags & NET_TIMER_RELOAD) {  // 定时器支持重载
      timer->curr_ticks = timer->reload_ticks;
      timer->flags |= NET_TIMER_ACTIVE;
      insert_timer(timer);
    }
  }

  display_timer_list();
  return NET_ERR_OK;
}

/**
 * @brief 获取定时器链表中第一个定时器的剩余时间
 *
 * @return int
 */
int net_timer_first_tmo(void) {
  net_timer_t *timer = nlist_entry(nlist_first(&timer_list), net_timer_t, node);

  return timer ? timer->curr_ticks : 0;
}