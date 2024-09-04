#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>


static log_level_t current_log_level = LOG_INFO;
static const char *log_level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

// 获取文件名
const char *basename(const char *path) {
    const char *basename = strrchr(path, '/');
    return basename ? basename + 1 : path;
}

// 设置日志级别
void set_log_level(log_level_t level) {
    current_log_level = level;
}

void log_message(log_level_t level,const char* file,int line,const char* func, const char *format, ...) {
    if (level < current_log_level) {
        return;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char time_buffer[32];
    snprintf(time_buffer, sizeof(time_buffer), "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
        localtime(&tv.tv_sec)->tm_year + 1900,
        localtime(&tv.tv_sec)->tm_mon + 1,
        localtime(&tv.tv_sec)->tm_mday,
        localtime(&tv.tv_sec)->tm_hour,
        localtime(&tv.tv_sec)->tm_min,
        localtime(&tv.tv_sec)->tm_sec,
        tv.tv_usec);

    printf("[%s][%s][%s:%d][%s] ", time_buffer, log_level_names[level], basename(file),line,func);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}
