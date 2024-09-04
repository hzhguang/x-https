#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>

SSL_CTX* init_client_ssl(void) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Unable to create SSL context\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

int main(){
    SSL_CTX* ctx =init_client_ssl();
    printf("%p \r\n",ctx);
    return 0;
}