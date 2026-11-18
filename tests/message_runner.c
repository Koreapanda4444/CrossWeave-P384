#include "crossweave_internal.h"
#include "test_io.h"
int main(int argc, char **argv) {
    CHECK(argc==3); char *end; unsigned long a=strtoul(argv[1],&end,10);
    CHECK(*end=='\0' && a<=1024); unsigned long m=strtoul(argv[2],&end,10);
    CHECK(*end=='\0' && m<=1024);
    uint8_t key[16],nonce[16],ad[1024],p[1024],c[1024],dec[1024],tag[16],tag2[16],s[48],base[48],other[48];
    for(size_t i=0;i<16;++i) {key[i]=(uint8_t)i;nonce[i]=(uint8_t)(i+16);}
    for(size_t i=0;i<(size_t)a;++i) ad[i]=(uint8_t)i;
    for(size_t i=0;i<(size_t)m;++i) p[i]=(uint8_t)(7u*i+3u);
    cw_initialize(base,key,nonce); cw_absorb_ad(base,ad,(size_t)a);
    memcpy(s,base,48); cw_message(s,c,p,(size_t)m,0); cw_finalize(s,tag,key);
    printhex(c,(size_t)m); printhex(tag,16);
    memcpy(other,base,48); size_t offset=0;
    for(;offset+16u<=(size_t)m;offset+=16u) {
        cw_message_block(other,dec+offset,c+offset,16,1,1); printhex(other,48);
    }
    cw_message_block(other,dec+offset,c+offset,(size_t)m-offset,1,0); printhex(other,48);
    CHECK(memcmp(dec,p,(size_t)m)==0); cw_finalize(other,tag2,key); printhex(other,48);
    CHECK(memcmp(tag,tag2,16)==0 && memcmp(s,other,48)==0);
    memcpy(other,base,48); cw_message(other,dec,c,(size_t)m,1); cw_finalize(other,tag2,key);
    CHECK(memcmp(dec,p,(size_t)m)==0 && memcmp(s,other,48)==0);
    return 0;
}
