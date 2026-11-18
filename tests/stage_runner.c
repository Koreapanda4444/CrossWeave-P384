#include "crossweave_internal.h"
#include "test_io.h"
int main(int argc, char **argv) {
    uint8_t bytes[48]; uint64_t s[6], original[6];
    CHECK(argc==3); char *end; unsigned long r=strtoul(argv[1],&end,10);
    CHECK(*end=='\0' && r<16 && unhex(argv[2],bytes,48)==48);
    for(unsigned i=0;i<6;++i) original[i]=s[i]=cw_load64(bytes+8u*i);
    static const unsigned order[6]={0,2,4,1,3,5};
    for(unsigned k=0;k<6;++k) {
        cw_lane_update(s,(unsigned)r,order[k]);
        for(unsigned i=0;i<6;++i) cw_store64(bytes+8u*i,s[i]);
        printhex(bytes,48);
    }
    cw_updates(original,(unsigned)r);
    CHECK(memcmp(original,s,sizeof s)==0); return 0;
}
