#include "crossweave.h"
#include "fault_support.h"
#include "check.h"
#include <string.h>
static void complete(size_t n) {
    fault_stats s=fault_get();
    CHECK(s.allocations==(n!=0?1u:0u) && s.frees==s.allocations && s.active==0);
    CHECK(s.bytes==n && s.wipes>=2 && s.last[0]==16 && s.last[1]==48);
}
static void unchanged(const uint8_t *a,const uint8_t *b,size_t n) { CHECK(memcmp(a,b,n)==0); }
int main(void) {
    const size_t lengths[5]={0,1,16,17,65};
    uint8_t key[16]={0},nonce[16]={0},ad[33],p[128],c[128],out[128],tag[32],saved[128],saved_tag[32],badtag[16];
    for(size_t i=0;i<sizeof p;++i) p[i]=(uint8_t)(i*7u+1u);
    for(size_t i=0;i<sizeof ad;++i) ad[i]=(uint8_t)i;
    for(size_t j=0;j<5;++j) {
        size_t n=lengths[j]; nonce[0]=(uint8_t)j;
        memset(c,0x91,sizeof c); memset(tag,0xa2,sizeof tag);
        memcpy(saved,c,sizeof c); memcpy(saved_tag,tag,sizeof tag);
        fault_reset(1);
        crossweave_status rc=crossweave_encrypt(c,128,tag,32,p,n,ad,33,nonce,16,key,16);
        if(n==0) {CHECK(rc==CROSSWEAVE_OK);complete(0);unchanged(c,saved,128);}
        else {
            CHECK(rc==CROSSWEAVE_ENOMEM);unchanged(c,saved,128);unchanged(tag,saved_tag,32);
            fault_stats s=fault_get();CHECK(s.allocations==1 && s.active==0 && s.frees==0 && s.wipes==0);
            memcpy(out,p,128);memcpy(saved,out,128);fault_reset(1);
            CHECK(crossweave_encrypt(out,128,tag,32,out,n,ad,33,nonce,16,key,16)==CROSSWEAVE_ENOMEM);
            unchanged(out,saved,128);unchanged(tag,saved_tag,32);
        }
        fault_reset(0);
        CHECK(crossweave_encrypt(c,128,tag,32,p,n,ad,33,nonce,16,key,16)==CROSSWEAVE_OK);complete(n);
        memcpy(badtag,tag,16);badtag[0]^=1;
        memset(out,0xb3,sizeof out);memcpy(saved,out,128);
        fault_reset(1);
        rc=crossweave_decrypt(out,128,c,n,badtag,16,ad,33,nonce,16,key,16);
        CHECK(rc==(n==0?CROSSWEAVE_EAUTH:CROSSWEAVE_ENOMEM));unchanged(out,saved,128);
        if(n==0) complete(0); else {fault_stats s=fault_get();CHECK(s.allocations==1 && s.active==0 && s.frees==0 && s.wipes==0);}
        for(unsigned inplace=0;inplace<2;++inplace) {
            if(inplace) memcpy(out,c,n); else memset(out,0xb3,128);
            memcpy(saved,out,128);fault_reset(0);
            CHECK(crossweave_decrypt(out,128,inplace?out:c,n,badtag,16,ad,33,nonce,16,key,16)==CROSSWEAVE_EAUTH);
            unchanged(out,saved,128);complete(n);
        }
        fault_reset(0);
        CHECK(crossweave_decrypt(out,128,c,n,tag,16,ad,33,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(memcmp(out,p,n)==0);complete(n);
        fault_reset(1);
        CHECK(crossweave_decrypt(out,128,c,n,tag,15,ad,33,nonce,16,key,16)==CROSSWEAVE_ELENGTH);
        CHECK(fault_get().allocations==0);
        CHECK(crossweave_encrypt(out,128,tag,32,p,n,ad,33,nonce,16,NULL,16)==CROSSWEAVE_EINVAL);
        CHECK(fault_get().allocations==0);
        CHECK(crossweave_encrypt(out,128,out,16,p,n,ad,33,nonce,16,key,16)==(n==0?CROSSWEAVE_OK:CROSSWEAVE_EOVERLAP));
        CHECK(fault_get().allocations==0);
    }
    puts("Allocation/authentication failures: 0/1/16/17/65B; atomic outputs; no malloc(0); clean state/tag/temp; balanced allocations");
    return 0;
}
