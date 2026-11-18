#include "crossweave.h"
#include "test_io.h"
int main(int argc, char **argv) {
    CHECK(argc==5);
    uint8_t key[16],nonce[16],ad[1024],p[1024],c[1056],tag[32],out[1056],inplace[1056];
    CHECK(unhex(argv[1],key,16)==16 && unhex(argv[2],nonce,16)==16);
    size_t a=unhex(argv[3],ad,sizeof ad),m=unhex(argv[4],p,sizeof p);
    memset(c,0xa5,sizeof c); memset(tag,0xb6,sizeof tag); memset(out,0xc7,sizeof out);
    CHECK(crossweave_encrypt(c,sizeof c,tag,sizeof tag,p,m,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(crossweave_decrypt(out,sizeof out,c,m,tag,16,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(out,p,m)==0);
    for(size_t i=m;i<sizeof c;++i) CHECK(c[i]==0xa5 && out[i]==0xc7);
    for(size_t i=16;i<sizeof tag;++i) CHECK(tag[i]==0xb6);
    memcpy(inplace,p,m);
    uint8_t tag2[16];
    CHECK(crossweave_encrypt(inplace,sizeof inplace,tag2,16,inplace,m,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(c,inplace,m)==0 && memcmp(tag,tag2,16)==0);
    CHECK(crossweave_decrypt(inplace,sizeof inplace,inplace,m,tag2,16,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(inplace,p,m)==0);
    printhex(c,m); printhex(tag,16); printhex(out,m); return 0;
}
