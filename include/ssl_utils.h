#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* init_server_ssl(const char *cert_file, const char *key_file);
SSL_CTX* init_client_ssl(void);
void cleanup_ssl(SSL_CTX *ctx);
void handle_ssl_error(void);

#endif // SSL_UTILS_H
