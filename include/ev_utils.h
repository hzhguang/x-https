#ifndef EV_UTILS_H
#define EV_UTILS_H

#include <ev.h>

// 初始化libev事件循环
struct ev_loop *init_event_loop();

// 停止libev事件循环
void stop_event_loop(struct ev_loop *loop);

#endif // EV_UTILS_H
