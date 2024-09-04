#include "ev_utils.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

// 初始化libev事件循环
struct ev_loop *init_event_loop() {
    log_debug("...");
    struct ev_loop *loop =  ev_loop_new(EVBACKEND_EPOLL); // EV_DEFAULT;
    if (!loop) {
        log_error("failed to initialize libev event loop");
        exit(EXIT_FAILURE);
    }
    return loop;
}

// 停止libev事件循环
void stop_event_loop(struct ev_loop *loop) {
    log_debug("...");
    ev_break(loop, EVBREAK_ALL);
}
