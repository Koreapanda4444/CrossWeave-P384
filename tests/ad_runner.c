#include "crossweave_internal.h"
#include "test_io.h"
int main(int argc, char **argv) {
    CHECK(argc==2); char *end; unsigned long len=strtoul(argv[1],&end,10);
    CHECK(*end=='\0' && len<=1024);
    uint8_t key[16],nonce[16],ad[1024],state[48],whole[48];
    for(size_t i=0;i<16;++i) { key[i]=(uint8_t)i; nonce[i]=(uint8_t)(i+16); }
    for(size_t i=0;i<(size_t)len;++i) ad[i]=(uint8_t)i;
    cw_initialize(state,key,nonce); memcpy(whole,state,48); printhex(state,48);
    size_t offset=0;
    for(;offset+16u<=(size_t)len;offset+=16u) { cw_ad_full(state,ad+offset); printhex(state,48); }
    cw_ad_tail(state,ad+offset,(size_t)len-offset); printhex(state,48);
    cw_absorb_ad(whole,len==0?NULL:ad,(size_t)len); CHECK(memcmp(state,whole,48)==0);
    return 0;
}
