#include <limits.h>
#include <stdint.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include "check.h"
_Static_assert(CHAR_BIT == 8, "8-bit bytes required");
_Static_assert(sizeof(uint64_t) == 8, "64-bit lanes required");
_Static_assert(sizeof(uintptr_t) >= sizeof(void *), "uintptr_t required");
int main(void) {
    CHECK(OpenSSL_version_num() >= 0x30000000UL);
    puts(OpenSSL_version(OPENSSL_VERSION));
    return 0;
}
