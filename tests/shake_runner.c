#include "test_io.h"
#include "shake.h"
#include <openssl/err.h>
int main(int argc, char **argv) {
    uint8_t input[1024], out[512];
    CHECK(argc == 3); size_t n=unhex(argv[2],input,sizeof input);
    char *end; unsigned long want=strtoul(argv[1],&end,10);
    CHECK(*end == '\0' && want <= sizeof out);
    CHECK(!test_xof(NULL,input,n,out,8)); ERR_clear_error();
    CHECK(!test_xof(EVP_sha256(),input,n,out,8)); ERR_clear_error();
    CHECK(test_xof(EVP_shake256(),input,n,out,(size_t)want));
    printhex(out,(size_t)want); return 0;
}
