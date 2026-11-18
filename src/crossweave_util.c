#include "crossweave_internal.h"
#include <openssl/crypto.h>
void cw_clean(void *p, size_t n) {
    if (n != 0) OPENSSL_cleanse(p, n);
}
