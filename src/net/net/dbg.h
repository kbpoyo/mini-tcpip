#ifndef DBG_H
#define DBG_H

#include "net_cfg.h"

// ascii转译序列定义调试信息颜色
#define DBG_STYLE_ERROR "\033[31m"
#define DBG_STYLE_WARN "\033[33m"
#define DBG_STYLE_INFO "\033[32m"
#define DBG_STYLE_RESET "\033[0m"

// 定义调试等级
#define DBG_LEVEL_NONE  0
#define DBG_LEVEL_ERROR 1
#define DBG_LEVEL_WARN  2
#define DBG_LEVEL_INFO  3

void dbg_print(int module_level, int default_level, const char *file, const char *func, int len, const char *fmt, ...);

#define dbg_info(module, fmt, ...) dbg_print(module, DBG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_warning(module, fmt, ...) dbg_print(module, DBG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_error(module, fmt, ...) dbg_print(module, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define dbg_assert(expr, msg) { \
    if (!(expr)) { \
        dbg_error(DBG_LEVEL_ERROR, "assert ("#expr ") failed: "msg); \
        while(1); \
    } \
}

#define DBG_DISP_ENABLED(module) (module >= DBG_LEVEL_INFO)


#endif // DBG_H