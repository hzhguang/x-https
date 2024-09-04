#ifndef TCP_UTILS_H
#define TCP_UTILS_H

int create_server_socket(int port);
int create_client_socket(const char *hostname, int port);
void set_non_blocking(int fd);

#endif // TCP_UTILS_H
