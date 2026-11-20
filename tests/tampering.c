#include "crossweave.h"
#include "check.h"
#include <string.h>
int main(void) {
    uint8_t key[16],nonce[16],ad[33],p[65],c[128],tag[17],out[128],before[128];
    for(size_t i=0;i<16;++i){key[i]=(uint8_t)i;nonce[i]=(uint8_t)(i+16);}
    for(size_t i=0;i<33;++i)ad[i]=(uint8_t)i;
    for(size_t i=0;i<65;++i)p[i]=(uint8_t)i;
    memset(c,0xa5,sizeof c);memset(tag,0xb6,sizeof tag);
    CHECK(crossweave_encrypt(c,128,tag,17,p,65,ad,33,nonce,16,key,16)==CROSSWEAVE_OK);
    uint8_t *fields[5]={key,nonce,ad,c,tag};const size_t sizes[5]={16,16,33,65,16};
    size_t mutations=0;
    for(unsigned f=0;f<5;++f)for(size_t i=0;i<sizes[f];++i)for(unsigned bit=0;bit<8;++bit){
        fields[f][i]^=(uint8_t)(1u<<bit);
        for(unsigned inplace=0;inplace<2;++inplace){
            memset(out,0xc7,sizeof out);if(inplace)memcpy(out,c,65);memcpy(before,out,128);
            CHECK(crossweave_decrypt(out,128,inplace?out:c,65,tag,16,ad,33,nonce,16,key,16)==CROSSWEAVE_EAUTH);
            CHECK(memcmp(out,before,128)==0);
        }
        fields[f][i]^=(uint8_t)(1u<<bit);++mutations;
    }
    CHECK(mutations==1168);
    const size_t wrong[3]={0,15,17};memset(out,0xc7,128);memcpy(before,out,128);
    for(unsigned i=0;i<3;++i){
        CHECK(crossweave_decrypt(out,128,c,65,tag,wrong[i],ad,33,nonce,16,key,16)==CROSSWEAVE_ELENGTH);
        CHECK(memcmp(out,before,128)==0);
    }
    puts("1168 single-bit field mutations rejected, separate/in-place output unchanged; tag lengths 0/15/17 rejected");
    return 0;
}
