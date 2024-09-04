#ifndef __EPOLL_UTILS_H__
#define __EPOLL_UTILS_H__

#include <sys/epoll.h>

int create_epoll_instance(void);
void add_socket_to_epoll(int epoll_fd, int socket_fd, uint32_t events);
void modify_socket_in_epoll(int epoll_fd, int socket_fd, uint32_t events);
void remove_socket_from_epoll(int epoll_fd, int socket_fd);

#endif // __EPOLL_UTILS_H__
