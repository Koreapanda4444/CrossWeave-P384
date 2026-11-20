#include "crossweave_internal.h"
#include "check.h"
#include <openssl/crypto.h>
#include <string.h>
/* Only the residual AD XOR is deliberately omitted in this research control. */
static void bad_ad(uint8_t s[48],const uint8_t *ad,size_t n){
    while(n>=16){cw_ad_full(s,ad);ad+=16;n-=16;}
    s[n]^=0x80;s[47]^=0x12;cw_permute(s,12);
}
static void bad_encrypt(uint8_t *c,uint8_t tag[16],const uint8_t *p,size_t n,const uint8_t *ad,size_t a,const uint8_t key[16],const uint8_t nonce[16]){
    uint8_t s[48];cw_initialize(s,key,nonce);bad_ad(s,ad,a);cw_message(s,c,p,n,0);cw_finalize(s,tag,key);cw_clean(s,48);
}
static int bad_accept(const uint8_t *c,const uint8_t tag[16],size_t n,const uint8_t *ad,size_t a,const uint8_t key[16],const uint8_t nonce[16]){
    uint8_t s[48],p[64],t[16];CHECK(n<=64);cw_initialize(s,key,nonce);bad_ad(s,ad,a);cw_message(s,p,c,n,1);cw_finalize(s,t,key);
    int ok=CRYPTO_memcmp(t,tag,16)==0;cw_clean(s,48);cw_clean(p,64);cw_clean(t,16);return ok;
}
int main(void){
    const uint8_t msg[]="one valid encryption query";const size_t n=sizeof msg-1;
    uint8_t key[16]={0},nonce[16]={0},ad[33]={0},badc[64],badt[16],c[64],t[16],out[64],before[64];
    const size_t lengths[5]={1,15,17,31,33};
    for(unsigned j=0;j<5;++j){size_t a=lengths[j];memset(ad,0,33);
        bad_encrypt(badc,badt,msg,n,ad,a,key,nonce);CHECK(bad_accept(badc,badt,n,ad,a,key,nonce));
        CHECK(crossweave_encrypt(c,64,t,16,msg,n,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(crossweave_decrypt(out,64,c,n,t,16,ad,a,nonce,16,key,16)==CROSSWEAVE_OK);
        ad[a-1]=1;CHECK(bad_accept(badc,badt,n,ad,a,key,nonce));
        memset(out,0xa5,64);memcpy(before,out,64);
        CHECK(crossweave_decrypt(out,64,c,n,t,16,ad,a,nonce,16,key,16)==CROSSWEAVE_EAUTH);
        CHECK(memcmp(out,before,64)==0);
    }
    puts("{\"status\":\"complete\",\"classification\":\"deliberately omitted residual AD control\",\"ad_lengths\":[1,15,17,31,33],\"queries_per_control\":1,\"control_accepts_changed_ad\":true,\"official_rejects_changed_ad\":true}");return 0;
}
