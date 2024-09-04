#include "ssl_utils.h"
#include "log.h"
#include <stdio.h>

SSL_CTX* init_server_ssl(const char *cert_file, const char *key_file) {
    log_debug("cert_file:%s,key_file:%s",cert_file,key_file);
    fflush(stdout);

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        log_error("unable to create SSL context");
        exit(EXIT_FAILURE);
    }

    SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM);

    return ctx;
}

SSL_CTX* init_client_ssl(void) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        log_error("unable to create SSL context");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void cleanup_ssl(SSL_CTX *ctx) {
    SSL_CTX_free(ctx);
}

void handle_ssl_error(void) {
    ERR_print_errors_fp(stderr);
}
