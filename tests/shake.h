#ifndef CW_TEST_SHAKE_H
#define CW_TEST_SHAKE_H
#include <openssl/evp.h>
static inline int test_xof(const EVP_MD *md, const void *p, size_t n, unsigned char *out, size_t len) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if(ctx == NULL) return 0;
    int ok = EVP_DigestInit_ex(ctx, md, NULL) == 1 &&
             EVP_DigestUpdate(ctx, p, n) == 1 &&
             EVP_DigestFinalXOF(ctx, out, len) == 1;
    EVP_MD_CTX_free(ctx);
    return ok;
}
#endif
