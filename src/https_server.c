#include "https_server.h"
#include "ssl_utils.h"
#include "tcp_utils.h"
#include "ev_utils.h"
#include "log.h"
#include <openssl/ssl.h>
#include <ev.h>
#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_LEN 4096

// 处理SSL错误
static void handle_error(struct st_client *client, const char *context) {
    int err = SSL_get_error(client->ssl, -1);
    const char *error_message = "unknown ssl error";
    switch (err) {
        case SSL_ERROR_SSL:
            error_message = "ssl library error";
            break;
        case SSL_ERROR_SYSCALL:
            error_message = "system call error";
            break;
        case SSL_ERROR_ZERO_RETURN:
            error_message = "ssl connection closed";
            break;
        default:
            error_message = context;
            break;
    }
    log_error("%s", error_message);
    if (client->callbacks && client->callbacks->on_error) {
        client->callbacks->on_error(client, error_message);
    }
}

static size_t build_http_response(char *buffer, size_t buffer_size, int status_code, const char *status_message, const char *body) {
    const char *headers = "Content-Type: text/plain\r\nConnection: keep-alive\r\n";
    size_t body_length = strlen(body);
    size_t total_length = snprintf(buffer, buffer_size,
        "HTTP/1.1 %d %s\r\n"
        "%s"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        status_code, status_message, headers, body_length, body);

    if (total_length >= buffer_size) {
        return total_length;
    }
    return total_length;
}

int on_message_begin(llhttp_t *parser) {
    log_debug("parse start");
    return 0;
}
int on_url(llhttp_t *parser, const char* at, size_t length) {
    char url[MAX_LEN];
    strncpy(url, at, length);
    url[length] = '\0';
    log_debug("url: %s", url);
    return 0;
}
int on_header_field(llhttp_t *parser, const char* at, size_t length) {
    char header_field[MAX_LEN];
    strncpy(header_field, at, length);
    header_field[length] = '\0';
    log_debug("head field: %s", header_field);
    return 0;
}

// HTTP message parsing callbacks
int on_header_value(llhttp_t *parser, const char* at, size_t length) {
 char header_value[MAX_LEN];
    strncpy(header_value, at, length);
    header_value[length] = '\0';
    log_debug("head value: %s", header_value);
    return 0;
}

int on_headers_complete(llhttp_t *parser) {
    log_debug("on_headers_complete, major: %d, major: %d, keep-alive: %d, upgrade: %d", parser->http_major, parser->http_minor, llhttp_should_keep_alive(parser), parser->upgrade);
    return 0;
}

int on_body(llhttp_t *parser, const char *at, size_t length) {
    log_debug("%s, %ld",at,length);
    struct st_client *client = (struct st_client *)parser->data;
    if (client->callbacks && client->callbacks->on_data_received) {
        client->callbacks->on_data_received(client, at, length);
    }
    return 0;
}

int on_message_complete(llhttp_t *parser) {
    log_debug("message complete");
    return 0;
}

static void on_read(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct st_client *client = (struct st_client *)w->data;
    char buffer[4096];

    log_debug("loop:%p,io:%p,client:%p,parser:%p",loop,w,client,&client->parser);
    int read = SSL_read(client->ssl, buffer, sizeof(buffer));
    if (read <= 0) {
        if (client->callbacks && client->callbacks->on_disconnected) {
            client->callbacks->on_disconnected(client);
        }
        handle_error(client, "ssl read failed");
        ev_io_stop(loop, w);
        log_debug("errno:%d, err:%s", errno,strerror(errno));
        if (SSL_shutdown(client->ssl) < 0) {
            handle_error(client, "ssl read failed");
        }
        SSL_free(client->ssl);
        close(w->fd);
        free(client);
    } else {
        buffer[read] = '\0';
        log_debug("buffer:%s,length:%d",buffer,read);
        llhttp_errno_t err = llhttp_execute(&client->parser, buffer, read);
        if (err != HPE_OK) {
            log_error("llhttp error: %s", llhttp_errno_name(err));
        }
        log_debug("llhttp_execute %d",err);
    }
}

static void on_client_accept(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(w->fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        log_error("accept");
        return;
    }

    struct st_client *client = malloc(sizeof(struct st_client));
    if (!client) {
        log_error("malloc");
        close(client_fd);
        return;
    }
    client->client_fd = client_fd;

    struct st_server_params *server_data = (struct st_server_params*)w->data;
    client->ssl = SSL_new(server_data->ctx);
    if (!client->ssl) {
        log_error("SSL_new");
        close(client_fd);
        free(client);
        return;
    }

    llhttp_settings_init(&client->settings);
    client->settings.on_message_begin = on_message_begin;
    client->settings.on_url = on_url;
    client->settings.on_header_field = on_header_field;
    client->settings.on_header_value = on_header_value;
    client->settings.on_headers_complete = on_headers_complete;
    client->settings.on_body = on_body;
    client->settings.on_message_complete = on_message_complete;
    llhttp_init(&client->parser, HTTP_REQUEST, &client->settings);
    client->parser.data = client;

    SSL_set_fd(client->ssl, client_fd);
    if (SSL_accept(client->ssl) <= 0) {
        handle_error(client, "SSL accept failed");
        SSL_free(client->ssl);
        close(client_fd);
        free(client);
        return;
    }

    client->callbacks = server_data->callbacks;
    log_debug("new client,client_fd:%d,client:%p,ssl:%p,event_callbacks:%p,parser:%p",client_fd,client,client->ssl,client->callbacks,&client->parser);

    if (client->callbacks && client->callbacks->on_connected) {
        client->callbacks->on_connected(client);
    }

    client->io.data = client;

    ev_io_init(&client->io, on_read, client_fd, EV_READ);
    ev_io_start(loop, &client->io);
}

// 数据发送接口
bool send_data_to_client(struct st_client *client, const char *data, size_t length) {
    log_debug("client:%p,data:%s,data length:%ld", client, data, length);
    int result = SSL_write(client->ssl, data, length);
    if (result <= 0) {
        int err = SSL_get_error(client->ssl, result);
        const char *error_message = "Unknown SSL write error";
        switch (err) {
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
                // The operation did not complete; try again later.
                return false;
            default:
                error_message = "SSL write failed";
                break;
        }
        if (client->callbacks && client->callbacks->on_error) {
            client->callbacks->on_error(client, error_message);
        }
        return false;
    }
    return true;
}

bool send_response_to_client(struct st_client *client, int status_code, const char *status_message, const char *body){
    char response_buffer[4096];
    size_t response_length = build_http_response(response_buffer, sizeof(response_buffer), status_code, status_message, body);
    return send_data_to_client(client, response_buffer, response_length);
}

bool start_https_server(const char *cert_file, const char *key_file, int port, event_callbacks *callbacks) {
    signal(SIGPIPE, SIG_IGN);
    SSL_CTX *ctx = init_server_ssl(cert_file, key_file);
    if (!ctx) {
        return false;
    }

    int server_fd = create_server_socket(port);
    if (server_fd < 0) {
        cleanup_ssl(ctx);
        return false;
    }

    struct ev_loop *loop = init_event_loop();
    struct ev_io io_accept;

    // Create a struct to hold both the SSL context and the callbacks
    struct st_server_params *server_data = malloc(sizeof(st_server_params_t));

    if (!server_data) {
        log_error("malloc");
        close(server_fd);
        cleanup_ssl(ctx);
        return false;
    }

    server_data->ctx = ctx;
    server_data->callbacks = callbacks;
    log_debug("loop:%p,server_fd:%d,ctx:%p",loop,server_fd,ctx);

    ev_io_init(&io_accept, on_client_accept, server_fd, EV_READ);
    io_accept.data = server_data;

    ev_io_start(loop, &io_accept);
    ev_run(loop, 0);

    close(server_fd);
    cleanup_ssl(ctx);
    free(server_data); // Don't forget to free the allocated memory
    return true;
}
