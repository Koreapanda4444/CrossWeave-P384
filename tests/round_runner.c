#include "crossweave_internal.h"
#include "test_io.h"
int main(int argc, char **argv) {
    uint8_t bytes[48]; CHECK(argc==4);
    char *end; unsigned long r=strtoul(argv[2],&end,10);
    CHECK(*end=='\0' && r<=16 && unhex(argv[3],bytes,48)==48);
    if(strcmp(argv[1],"p")==0) cw_permute(bytes,(unsigned)r);
    else {
        uint64_t s[6]; CHECK(strcmp(argv[1],"round")==0 && r<16);
        for(unsigned i=0;i<6;++i) s[i]=cw_load64(bytes+8u*i);
        cw_round(s,(unsigned)r);
        for(unsigned i=0;i<6;++i) cw_store64(bytes+8u*i,s[i]);
    }
    printhex(bytes,48); return 0;
}
