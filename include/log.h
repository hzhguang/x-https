#ifndef __LOG_H__
#define __LOG_H__

// 日志级别枚举
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

void set_log_level(log_level_t level);
void log_message(log_level_t level,const char* file,int line,const char* func, const char *format, ...);

// 日志宏
#define log_debug(fmt, ...) log_message(LOG_DEBUG,__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_message(LOG_INFO,__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_message(LOG_WARN,__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_message(LOG_ERROR,__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif