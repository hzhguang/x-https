#ifndef __EVENT_CALLBACK_H__
#define __EVENT_CALLBACK_H__

#include <openssl/ssl.h>
#include <ev.h>
#include <llhttp.h>
#include <pthread.h>

#define BUFFER_SIZE 4096
#define INITIAL_BUFFER_SIZE 4096

typedef void (*data_callback_t)(void *client, const char *data, size_t length);
typedef void (*connect_callback_t)(void *client);
typedef void (*disconnect_callback_t)(void *client);
typedef void (*error_callback_t)(void *client, const char *error_message);

// 客户端回调结构体
typedef struct {
    data_callback_t on_data_received;
    connect_callback_t on_connected;
    disconnect_callback_t on_disconnected;
    error_callback_t on_error;  // 新增的异常回调
} event_callbacks;


struct st_client {
    struct ev_io io;
    int client_fd;
    SSL *ssl;
    llhttp_t parser;
    llhttp_settings_t settings;
    event_callbacks *callbacks;
}st_client_t;


typedef struct st_server_params{
    SSL_CTX *ctx;
    event_callbacks *callbacks;
} st_server_params_t; 

// typedef struct st_client {
//     struct ev_io io;
//     int client_fd;
//     SSL *ssl;
//     event_callbacks *callbacks;
//     llhttp_t parser;
//     char *buffer;
//     size_t buffer_size;
//     size_t buffer_offset;
//     pthread_mutex_t mutex;
//     pthread_cond_t cond;
//     bool response_received;
//     char response_buffer[BUFFER_SIZE];
//     size_t response_length;
//     struct memory_pool_t *pool;
// } st_client_t;

// HTTP 请求方法
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE
} http_method;

#endif