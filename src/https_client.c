#include "https_client.h"
#include "ssl_utils.h"
#include "tcp_utils.h"
#include "ev_utils.h"
#include "log.h"
#include <llhttp.h>
#include <openssl/ssl.h>
#include <ev.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_LEN 4096

static size_t build_http_request(char *buffer, size_t buffer_size, http_method method, const char *path, const char *body, size_t body_length) {
    static const char* method_array[] = {"GET", "POST", "DELETE", "PUT"};
    const char* method_str = method_array[method];
    size_t length = snprintf(buffer, buffer_size,
             "%s %s HTTP/1.1\r\n"
             "Host: localhost\r\n"
             "Connection: keep-alive\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             method_str, path, body_length, body);
    return length;
}

// Handle SSL errors
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
    if (client->callbacks && client->callbacks->on_error) {
        client->callbacks->on_error(client, error_message);
    }
}

static int on_message_begin(llhttp_t *parser) {
    log_debug("parse start");
    return 0;
}

static int on_url(llhttp_t *parser, const char* at, size_t length) {
    char url[MAX_LEN];
    strncpy(url, at, length);
    url[length] = '\0';
    log_debug("url: %s", url);
    return 0;
}

int on_status(llhttp_t* parser, const char* at, size_t length)
{
    char status[MAX_LEN];
    strncpy(status, at, length);
    status[length] = '\0';
    log_debug("status: %s", status);
    return 0;
}

static int on_header_field(llhttp_t *parser, const char* at, size_t length) {
    char header_field[MAX_LEN];
    strncpy(header_field, at, length);
    header_field[length] = '\0';
    log_debug("head field: %s", header_field);
    return 0;
}

// HTTP message parsing callbacks
static int on_header_value(llhttp_t *parser, const char* at, size_t length) {
 char header_value[MAX_LEN];
    strncpy(header_value, at, length);
    header_value[length] = '\0';
    log_debug("head value: %s", header_value);
    return 0;
}

static int on_headers_complete(llhttp_t *parser) {
    log_debug("on_headers_complete, major: %d, major: %d, keep-alive: %d, upgrade: %d", parser->http_major, parser->http_minor, llhttp_should_keep_alive(parser), parser->upgrade);
    return 0;
}

static int on_body(llhttp_t *parser, const char *at, size_t length) {
    log_debug("%s, %ld",at,length);
    struct st_client *client = (struct st_client *)parser->data;
    if (client->callbacks && client->callbacks->on_data_received) {
        client->callbacks->on_data_received(client, at, length);
    }
    return 0;
}

static int on_message_complete(llhttp_t *parser) {
    log_debug("message complete");
    return 0;
}

// Read data from server
static void on_read(struct ev_loop *loop, struct ev_io *w, int revents) {
    struct st_client *client = (struct st_client *)w->data;
    char buffer[4096];
    int read = SSL_read(client->ssl, buffer, sizeof(buffer));
    if (read <= 0) {
        if (client->callbacks && client->callbacks->on_disconnected) {
            client->callbacks->on_disconnected(client);
        }
        handle_error(client, "ssl read failed");
        ev_io_stop(loop, w);
    } else {
        buffer[read] = '\0';
        log_debug("buffer:%s,length:%d",buffer,read);
        // Feed the data to llhttp parser
        llhttp_errno_t err = llhttp_execute(&client->parser, buffer, read);
        if (err != HPE_OK) {
            log_error("llhttp error: %s", llhttp_errno_name(err));
        }
    }
}

// Send data to server
bool send_data_to_server(struct st_client *client, const char *data, size_t length) {
    log_debug("data:%s, length:%ld, client:%p", data, length, client);

    int result = SSL_write(client->ssl, data, length);
    if (result <= 0) {
        int err = SSL_get_error(client->ssl, result);
        const char *error_message = "unknown ssl write error";
        switch (err) {
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
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

// Send HTTP request
bool send_http_request(struct st_client* client, http_method method, const char *path, const char *body, size_t body_length) {
    char request_buffer[4096];
    int request_length = build_http_request(request_buffer, sizeof(request_buffer), method, path, body, body_length);
    return send_data_to_server(client, request_buffer, request_length);
}

// Start HTTPS client
bool start_https_client(const char *host, int port, event_callbacks *callbacks) {
    signal(SIGPIPE, SIG_IGN);
    SSL_CTX *ctx = init_client_ssl();
    if (!ctx) {
        return false;
    }

    int client_fd = create_client_socket(host, port);
    if (client_fd < 0) {
        cleanup_ssl(ctx);
        return false;
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        log_error("SSL_new failed");
        close(client_fd);
        cleanup_ssl(ctx);
        return false;
    }

    struct st_client *client = malloc(sizeof(struct st_client));
    if (!client) {
        log_error("malloc failed");
        SSL_free(ssl);
        close(client_fd);
        cleanup_ssl(ctx);
        return false;
    }
    client->client_fd = client_fd;
    client->ssl = ssl;
    client->callbacks = callbacks;

    // Initialize llhttp parser
    llhttp_settings_init(&client->settings);
    client->settings.on_message_begin = on_message_begin;
    client->settings.on_url = on_url;
    client->settings.on_status = on_status;
    client->settings.on_header_field = on_header_field;
    client->settings.on_header_value = on_header_value;
    client->settings.on_headers_complete = on_headers_complete;
    client->settings.on_body = on_body;
    client->settings.on_message_complete = on_message_complete;
    llhttp_init(&client->parser, HTTP_RESPONSE, &client->settings);
    client->parser.data = client;

    SSL_set_fd(ssl, client_fd);
    if (SSL_connect(ssl) <= 0) {
        handle_error(client, "SSL connect failed");
        SSL_free(ssl);
        close(client_fd);
        cleanup_ssl(ctx);
        free(client);
        return false;
    }

    struct ev_loop *loop = init_event_loop();
    struct ev_io io;
    ev_io_init(&io, on_read, client_fd, EV_READ);
    io.data = client;

    log_debug("loop:%p,io:%p,ssl:%p,client:%p,client_fd:%d", loop, &io, ssl, client, client_fd);

    if (client->callbacks && client->callbacks->on_connected) {
        client->callbacks->on_connected(client);
    }

    ev_io_start(loop, &io);
    ev_run(loop, 0);

    ev_io_stop(loop, &io);
    close(client_fd);
    int shutdown = SSL_get_shutdown(client->ssl);
    if(shutdown & SSL_SENT_SHUTDOWN){
        log_info("shutdown request ignored");
    }else{
        SSL_shutdown(client->ssl);
    }
    log_info("ssl:%p , ctx:%p, client:%p, client->ssl:%p",ssl,ctx,client,client->ssl);
    SSL_free(client->ssl);
    cleanup_ssl(ctx);
    free(client);
    return true;
}
