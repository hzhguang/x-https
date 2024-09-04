#include "https_client.h"
#include "log.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

// 数据接收回调
void on_data_received(void *client, const char *data, size_t length) {
    log_debug("data received: %.*s", (int)length, data);

    const char *body = "hello,server!";
    bool result = send_http_request(client, HTTP_METHOD_GET, "/hello", body, strlen(body));
    if (!result) {
        log_error("failed to send data to server");
    }
}

// 连接成功回调
void on_connected(void *client) {
    log_debug("connected to server");
    
    const char *body = "hello,server!";
    bool result = send_http_request(client, HTTP_METHOD_GET, "/hello", body, strlen(body));
    if (!result) {
        log_error("failed to send data to server");
    }
}

// 连接断开回调
void on_disconnected(void *client) {
    log_debug("disconnected from server");
}

// 异常回调
void on_error(void *client, const char *error_message) {
    log_error("error: %s", error_message);
}

int main() {
    st_config_file_t config;
    if (parse_config("config.txt", &config) != 0) {
        printf("parse config file failed");
        return 0;
    }
    int log_level = atoi(get_config_value(&config, "log", "level"));
    int ssl_port = atoi(get_config_value(&config, "ssl", "port"));
    const char* host = get_config_value(&config, "ssl", "host");

    printf("log_level:%d,ssl_port:%d,host:%s \r\n",log_level,ssl_port,host);

    set_log_level(log_level);
    event_callbacks callbacks = {
        .on_data_received = on_data_received,
        .on_connected = on_connected,
        .on_disconnected = on_disconnected,
        .on_error = on_error  // 设置异常回调
    };

    if (!start_https_client(host, ssl_port, &callbacks)) {
        log_error("failed to start https client");
        free_config(&config);
        return 1;
    }

    free_config(&config);
    return 0;
}
