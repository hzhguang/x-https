#ifndef HTTPS_SERVER_H
#define HTTPS_SERVER_H

#include <stdbool.h>
#include <openssl/ssl.h>
#include "structs.h"


// 启动HTTPS服务器
bool start_https_server(const char *cert_file, const char *key_file, int port, event_callbacks *callbacks);

// 向客户端发送数据
bool send_data_to_client(struct st_client *client, const char *data, size_t length);

bool send_response_to_client(struct st_client *client, int status_code, const char *status_message, const char *body);

#endif // HTTPS_SERVER_H
