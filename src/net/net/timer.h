/**
 * @file timer.h
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 定时器模块
 * @version 0.1
 * @date 2024-05-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "net_cfg.h"
#include "net_err.h"
#include "nlist.h"

#define TIMER_NAME_SIZE 32         // 定时器名字长度
#define NET_TIMER_RELOAD (1 << 0)  // 重复定时器

struct _net_timer_t;

// 定时器回调函数
typedef void (*timer_handle_t)(struct _net_timer_t *timer, void *arg);

/**
 * @brief 定时器结构
 *
 */
typedef struct _net_timer_t {
  char name[TIMER_NAME_SIZE];
  nlist_node_t node;      // 定时器节点
  int flags;              // 定时器标志位
  int curr_ticks;         // 当前定时器的ticks, (ms/ticks)
  int reload_ticks;       // 定时器的重载ticks
  timer_handle_t handle;  // 定时器回调函数
  void *arg;              // 回调函数参数

} net_timer_t;

net_err_t net_timer_module_init(void);

net_err_t net_timer_add(net_timer_t *timer, const char *name, timer_handle_t handle, void *arg, int ms, int flags);

#endif  // TIMER_H