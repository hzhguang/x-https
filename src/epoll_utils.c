#include "epoll_utils.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int create_epoll_instance(void) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        log_error("epoll_create1");
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

void add_socket_to_epoll(int epoll_fd, int socket_fd, uint32_t events) {
    log_debug("%d, %d , %d", epoll_fd,socket_fd, events);
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        log_error("epoll_ctl: add_socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void modify_socket_in_epoll(int epoll_fd, int socket_fd, uint32_t events) {
    log_debug("%d, %d , %d", epoll_fd,socket_fd, events);
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &ev) == -1) {
        log_error("epoll_ctl: modify_socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void remove_socket_from_epoll(int epoll_fd, int socket_fd) {
    log_debug("%d, %d ", epoll_fd,socket_fd);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, NULL) == -1) {
        log_error("epoll_ctl: remove_socket");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    close(socket_fd);
}
