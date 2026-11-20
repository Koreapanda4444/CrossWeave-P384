#include "bench_api.h"
#include "test_io.h"
#include <limits.h>
int main(void){
    const char *names[2]={"AES-128-GCM","ChaCha20-Poly1305"};
    const size_t lengths[7]={0,1,15,16,17,65,1024};
    for(unsigned alg=0;alg<2;++alg){const bench_api *api=bench_baseline(names[alg]);CHECK(api);
        uint8_t key[32]={0},nonce[12]={0},ad[33],p[1024],c[1024],out[1024],before[1024],tag[16];
        for(size_t i=0;i<1024;++i){p[i]=(uint8_t)i;}
        for(size_t i=0;i<33;++i){ad[i]=(uint8_t)i;}
        for(unsigned j=0;j<7;++j){size_t n=lengths[j];nonce[0]=(uint8_t)j;
            CHECK(api->encrypt(c,tag,p,n,ad,33,nonce,key)==0);CHECK(api->decrypt(out,c,n,tag,ad,33,nonce,key)==0);CHECK(memcmp(out,p,n)==0);
            tag[15]^=1;memset(out,0xa5,sizeof out);memcpy(before,out,1024);
            CHECK(api->decrypt(out,c,n,tag,ad,33,nonce,key)==7);CHECK(memcmp(out,before,1024)==0);tag[15]^=1;
            if(n){c[0]^=1;CHECK(api->decrypt(out,c,n,tag,ad,33,nonce,key)==7);CHECK(memcmp(out,before,1024)==0);c[0]^=1;}
        }
        CHECK(api->encrypt(c,tag,p,(size_t)INT_MAX+1u,ad,33,nonce,key)==2);
        CHECK(api->encrypt(c,tag,p,1,ad,(size_t)INT_MAX+1u,nonce,key)==2);
    }
    const bench_api *aes=bench_baseline(names[0]);uint8_t key[16]={0},nonce[12]={0},p[16]={0},c[16],tag[16],expected[16];
    CHECK(aes->encrypt(c,tag,p,16,NULL,0,nonce,key)==0);
    CHECK(unhex("0388dace60b6a392f328c2b971b2fe78",expected,16)==16 && memcmp(c,expected,16)==0);
    CHECK(unhex("ab6e47d42cec13bdf53a67b21257bddf",expected,16)==16 && memcmp(tag,expected,16)==0);
    CHECK(bench_baseline("unknown")==NULL);puts("OpenSSL baselines: KAT, boundary round trips, tamper/atomic failures, EVP int bounds");return 0;
}
