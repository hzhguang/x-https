#ifndef HTTPS_CLIENT_H
#define HTTPS_CLIENT_H

#include <stdbool.h>
#include <openssl/ssl.h>
#include "structs.h"

// 启动HTTPS客户端
bool start_https_client(const char *host, int port, event_callbacks *callbacks);

bool send_data_to_server(struct st_client *client, const char *data, size_t length);

bool send_http_request(struct st_client* client, http_method method, const char *path, const char *body, size_t body_length);

#endif // HTTPS_CLIENT_H
