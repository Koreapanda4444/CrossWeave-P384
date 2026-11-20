#include "crossweave.h"
#include "check.h"
#include <string.h>
static void guards(const uint8_t *p,size_t n){for(size_t i=0;i<16;++i)CHECK(p[i]==0xa5 && p[n+16+i]==0xa5);}
int main(void){
    const size_t max=CROSSWEAVE_MAX_INPUT_BYTES;
    uint8_t *pa=malloc(max+32),*ca=malloc(max+32),*oa=malloc(max+32),*aa=malloc(max+32);
    CHECK(pa && ca && oa && aa);memset(pa,0xa5,max+32);memset(ca,0xa5,max+32);memset(oa,0xa5,max+32);memset(aa,0xa5,max+32);
    uint8_t *p=pa+16,*c=ca+16,*out=oa+16,*ad=aa+16,key[16]={0},nonce[16]={0},tag[16];
    for(size_t i=0;i<max;++i){p[i]=(uint8_t)i;ad[i]=(uint8_t)(i*3u);}
    CHECK(crossweave_encrypt(c,max,tag,16,p,max,ad,max,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(crossweave_decrypt(out,max,c,max,tag,16,ad,max,nonce,16,key,16)==CROSSWEAVE_OK);
    CHECK(memcmp(out,p,max)==0);guards(pa,max);guards(ca,max);guards(oa,max);guards(aa,max);
    for(unsigned j=1;j<=100;++j){
        size_t n=(j*31u)%1025u;nonce[0]=(uint8_t)j;
        CHECK(crossweave_encrypt(c+1,max-1,tag,16,p+1,n,ad+1,n,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(crossweave_decrypt(out+1,max-1,c+1,n,tag,16,ad+1,n,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(memcmp(out+1,p+1,n)==0);
    }
    uint8_t before[32];memcpy(before,out,32);
    CHECK(crossweave_encrypt(out,32,tag,16,(void *)(uintptr_t)1,max+1,ad,0,nonce,16,key,16)==CROSSWEAVE_ELENGTH);
    CHECK(crossweave_decrypt(out,32,(void *)(uintptr_t)1,max+1,tag,16,ad,0,nonce,16,key,16)==CROSSWEAVE_ELENGTH);
    CHECK(crossweave_encrypt(out,32,tag,16,p,1,(void *)(uintptr_t)1,max+1,nonce,16,key,16)==CROSSWEAVE_ELENGTH);
    CHECK(memcmp(before,out,32)==0);
    free(pa);free(ca);free(oa);free(aa);
    puts("Actual 2^24-byte AD and message round trip, guards, 100 unaligned calls, oversized inputs rejected before reads");return 0;
}
