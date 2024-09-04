#include "https_server.h"
#include "log.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

// 数据接收回调
void on_data_received(void* client, const char *data, size_t length) {
    log_debug("data received: %.*s", (int)length, data);
    
    // 向客户端发送响应数据
    int status_code = 200;
    const char* status_message = "OK";
    const char* body = "hello,client!";
    bool result = send_response_to_client(client, status_code, status_message, body);
    if (!result) {
        log_error("failed to send data to client");
    }
}

// 连接建立回调
void on_connected(void* client) {
    log_debug("client connected");
}

// 连接断开回调
void on_disconnected(void* client) {
    log_debug("client disconnected");
}

// 异常回调
void on_error(void* client, const char *error_message) {
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
    const char* cert_file = get_config_value(&config, "ssl", "cert");
    const char* key_file = get_config_value(&config, "ssl", "key");

    printf("log_level:%d,ssl_port:%d,cert_file:%s,key_file:%s \r\n",log_level,ssl_port,cert_file,key_file);

    set_log_level(log_level);


    event_callbacks callbacks = {
        .on_data_received = on_data_received,
        .on_connected = on_connected,
        .on_disconnected = on_disconnected,
        .on_error = on_error  // 设置异常回调
    };

    if (!start_https_server(cert_file, key_file, ssl_port, &callbacks)) {
        log_error("failed to start https server");
        free_config(&config);
        return 1;
    }

    free_config(&config);
    return 0;
}
