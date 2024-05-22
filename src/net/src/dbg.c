#include "dbg.h"

#include <stdarg.h>

#include "net_sys.h"

/**
 * @brief 按调试等级打印调试信息
 *
 * @param file
 * @param func
 * @param line
 * @param fmt
 * @param ...
 */
void dbg_print(int module_level, int default_level, const char *file,
               const char *func, int line, const char *fmt, ...) {
    // 定义调试信息标题
    static const char *title[] = {
        [DBG_LEVEL_NONE] = "none",
        [DBG_LEVEL_ERROR] = DBG_STYLE_ERROR "error",
        [DBG_LEVEL_WARN] = DBG_STYLE_WARN "waring",
        [DBG_LEVEL_INFO] = DBG_STYLE_INFO "info",
    };
    
  if (module_level >= default_level) {
    const char *end = file + plat_strlen(file);
    while (end >= file) {
      if (*end == '/' || *end == '\\') {
        break;
      }
      end--;
    }
    end++;

    plat_printf("%s(%s-%s-%d):", title[default_level], end, func, line);

    char str_buf[256];
    va_list args;
    va_start(args, fmt);
    plat_vsprintf(str_buf, fmt, args);
    plat_printf("%s\n" DBG_STYLE_RESET, str_buf);
    va_end(args);
  }
}


