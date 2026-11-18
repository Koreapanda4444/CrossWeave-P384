#include "crossweave.h"
#include "test_io.h"
int main(void) {
    uint8_t key[16]={0},nonce[16],ad[17],p[64],c[64],tag[32],expected[17],expected_tag[16];
    for(size_t i=0;i<17;++i) ad[i]=p[i]=(uint8_t)i;
    for(size_t i=0;i<16;++i) nonce[i]=(uint8_t)i;
    CHECK(unhex("d9284e96da9b49763c268a2abc39abf2c0",expected,17)==17);
    CHECK(unhex("c1157d9abecbade2ca62f2611adc0c4a",expected_tag,16)==16);
    memset(c,0xa5,sizeof c); memset(tag,0xb6,sizeof tag);
    CHECK(crossweave_encrypt(c,64,tag,32,p,17,ad,17,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(c,expected,17)==0 && memcmp(tag,expected_tag,16)==0);
    for(size_t i=17;i<64;++i) CHECK(c[i]==0xa5);
    for(size_t i=16;i<32;++i) CHECK(tag[i]==0xb6);
    CHECK(crossweave_encrypt(p,64,tag,32,p,17,ad,17,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(p,expected,17)==0 && memcmp(tag,expected_tag,16)==0);
    uint8_t before[64],oldtag[32]; memcpy(before,c,64); memcpy(oldtag,tag,32);
    CHECK(crossweave_encrypt(c,16,tag,32,p,17,ad,17,nonce,16,key,16)==CROSSWEAVE_ECAPACITY);
    CHECK(crossweave_encrypt(c,64,tag,15,p,17,ad,17,nonce,16,key,16)==CROSSWEAVE_ECAPACITY);
    CHECK(crossweave_encrypt(c+1,63,tag,32,c,17,ad,17,nonce,16,key,16)==CROSSWEAVE_EOVERLAP);
    CHECK(crossweave_encrypt(c,64,tag,32,p,17,ad,17,nonce,16,NULL,16)==CROSSWEAVE_EINVAL);
    CHECK(memcmp(c,before,64)==0 && memcmp(tag,oldtag,32)==0);
    CHECK(unhex("f38f855b4280efb6c827a2361a03979e",key,16)==16); memset(nonce,0,16);
    CHECK(crossweave_encrypt(NULL,0,tag,32,NULL,0,NULL,0,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(unhex("7c7d20f684fe8f6d80ae7d20c78cc3cf",expected_tag,16)==16);
    CHECK(memcmp(tag,expected_tag,16)==0); return 0;
}
