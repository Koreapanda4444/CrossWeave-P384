#include "crossweave.h"
#include "test_io.h"
int main(void) {
    uint8_t key[16]={0},nonce[16],ad[17],p[64],c[64],tag[16],before[64];
    for(size_t i=0;i<17;++i) ad[i]=(uint8_t)i;
    for(size_t i=0;i<16;++i) nonce[i]=(uint8_t)i;
    CHECK(unhex("d9284e96da9b49763c268a2abc39abf2c0",c,64)==17);
    CHECK(unhex("c1157d9abecbade2ca62f2611adc0c4a",tag,16)==16);
    memset(p,0xa5,sizeof p); memcpy(before,p,64);
    tag[15]^=0x80;
    CHECK(crossweave_decrypt(p,64,c,17,tag,16,ad,17,nonce,16,key,16)==CROSSWEAVE_EAUTH);
    CHECK(memcmp(p,before,64)==0);
    memcpy(p,c,17); memcpy(before,p,64);
    CHECK(crossweave_decrypt(p,64,p,17,tag,16,ad,17,nonce,16,key,16)==CROSSWEAVE_EAUTH);
    CHECK(memcmp(p,before,64)==0);
    tag[15]^=0x80;
    CHECK(crossweave_decrypt(p,64,c,17,tag,16,ad,17,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(p,ad,17)==0 && memcmp(p+17,before+17,47)==0);
    memcpy(p,c,17);
    CHECK(crossweave_decrypt(p,64,p,17,tag,16,ad,17,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(p,ad,17)==0);
    CHECK(unhex("f38f855b4280efb6c827a2361a03979e",key,16)==16); memset(nonce,0,16);
    CHECK(unhex("7c7d20f684fe8f6d80ae7d20c78cc3cf",tag,16)==16);
    CHECK(crossweave_decrypt(NULL,0,NULL,0,tag,16,NULL,0,nonce,16,key,16)==CROSSWEAVE_OK);
    tag[0]^=1;
    CHECK(crossweave_decrypt(NULL,0,NULL,0,tag,16,NULL,0,nonce,16,key,16)==CROSSWEAVE_EAUTH);
    return 0;
}
