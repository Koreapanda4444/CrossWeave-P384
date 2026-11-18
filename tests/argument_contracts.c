#include "crossweave.h"
#include "check.h"
#include <string.h>

typedef struct {
    uint8_t *out,*tag;
    const uint8_t *in,*ad,*nonce,*key;
    size_t cap,n,tag_n,a,nonce_n,key_n;
} args;
static uint8_t arena[2048];
static size_t cases;
static args base(void) {
    for(size_t i=0;i<sizeof arena;++i) arena[i]=(uint8_t)(i*11u+7u);
    args q={arena+256,arena+768,arena+512,arena+1024,arena+1280,arena+1536,64,33,16,33,16,16};
    return q;
}
static void verify(int decrypt,args q,crossweave_status want) {
    uint8_t before[sizeof arena]; memcpy(before,arena,sizeof arena);
    crossweave_status got=decrypt?
        crossweave_decrypt(q.out,q.cap,q.in,q.n,q.tag,q.tag_n,q.ad,q.a,q.nonce,q.nonce_n,q.key,q.key_n):
        crossweave_encrypt(q.out,q.cap,q.tag,q.tag_n,q.in,q.n,q.ad,q.a,q.nonce,q.nonce_n,q.key,q.key_n);
    if(got!=want) { fprintf(stderr,"case %zu, decrypt=%d: got %d expected %d\n",cases,decrypt,got,want); exit(1); }
    for(size_t i=0;i<sizeof arena;++i) {
        uintptr_t pos=(uintptr_t)(arena+i),out=(uintptr_t)q.out,tag=(uintptr_t)q.tag;
        int changed=got==CROSSWEAVE_OK && ((pos>=out && pos-out<q.n) ||
                    (!decrypt && pos>=tag && pos-tag<16u));
        if(!changed) CHECK(arena[i]==before[i]);
    }
    ++cases;
}
int main(void) {
    for(int d=0;d<2;++d) {
        args q;
        crossweave_status accepted=d?CROSSWEAVE_EAUTH:CROSSWEAVE_OK;
        for(size_t j=0;j<4;++j) {
            const size_t wrong[4]={0,15,17,SIZE_MAX};
            q=base();q.key_n=wrong[j];q.key=NULL;q.cap=0;verify(d,q,CROSSWEAVE_ELENGTH);
            q=base();q.nonce_n=wrong[j];q.nonce=NULL;verify(d,q,CROSSWEAVE_ELENGTH);
            if(d) {q=base();q.tag_n=wrong[j];q.tag=NULL;verify(d,q,CROSSWEAVE_ELENGTH);}
        }
        q=base();q.n=CROSSWEAVE_MAX_INPUT_BYTES+1u;q.in=NULL;q.out=NULL;q.cap=0;verify(d,q,CROSSWEAVE_ELENGTH);
        q=base();q.a=CROSSWEAVE_MAX_INPUT_BYTES+1u;q.ad=NULL;verify(d,q,CROSSWEAVE_ELENGTH);
        q=base();q.n=SIZE_MAX;q.in=(void *)UINTPTR_MAX;verify(d,q,CROSSWEAVE_ELENGTH);
        q=base();q.cap=32;q.out=NULL;q.key=NULL;verify(d,q,CROSSWEAVE_ECAPACITY);
        if(!d) {q=base();q.tag_n=15;q.tag=NULL;q.out=NULL;verify(d,q,CROSSWEAVE_ECAPACITY);}
        for(unsigned field=0;field<6;++field) {
            q=base();
            switch(field) {
                case 0:q.out=NULL;break;case 1:q.in=NULL;break;case 2:q.tag=NULL;break;
                case 3:q.ad=NULL;break;case 4:q.nonce=NULL;break;default:q.key=NULL;break;
            }
            verify(d,q,CROSSWEAVE_EINVAL);
            q=base(); const uint8_t *bad=(const uint8_t *)(UINTPTR_MAX-7u);
            switch(field) {
                case 0:q.out=(uint8_t *)bad;break;case 1:q.in=bad;break;case 2:q.tag=(uint8_t *)bad;break;
                case 3:q.ad=bad;break;case 4:q.nonce=bad;break;default:q.key=bad;break;
            }
            verify(d,q,CROSSWEAVE_EINVAL);
        }
        q=base();q.out=(uint8_t *)q.in+1;q.key=(void *)(UINTPTR_MAX-7u);verify(d,q,CROSSWEAVE_EINVAL);
        q=base();q.out=(uint8_t *)q.in+1;q.key=NULL;verify(d,q,CROSSWEAVE_EINVAL);
        for(int delta=-32;delta<=32;++delta) {
            q=base();q.out=(uint8_t *)q.in+delta;
            verify(d,q,delta==0?accepted:CROSSWEAVE_EOVERLAP);
        }
        q=base();q.out=(uint8_t *)q.in+33;verify(d,q,accepted);
        q=base();q.out=(uint8_t *)q.in-33;verify(d,q,accepted);
        for(unsigned field=0;field<4;++field) {
            for(int delta=-32;delta<=(field==1?32:15);++delta) {
                q=base(); const uint8_t *read=field==0?q.tag:field==1?q.ad:field==2?q.nonce:q.key;
                q.out=(uint8_t *)read+delta;verify(d,q,CROSSWEAVE_EOVERLAP);
            }
        }
        if(!d) {
            for(unsigned field=0;field<4;++field) {
                for(int delta=-15;delta<=(field<2?32:15);++delta) {
                    q=base();const uint8_t *read=field==0?q.in:field==1?q.ad:field==2?q.nonce:q.key;
                    q.tag=(uint8_t *)read+delta;verify(0,q,CROSSWEAVE_EOVERLAP);
                }
            }
        } else {
            for(unsigned field=0;field<4;++field) {
                q=base();q.tag=(uint8_t *)(field==0?q.in:field==1?q.ad:field==2?q.nonce:q.key);
                verify(1,q,CROSSWEAVE_EAUTH);
            }
        }
        q=base();q.key=q.nonce;verify(d,q,accepted);
        q=base();q.nonce=q.ad;verify(d,q,accepted);
        q=base();q.in=q.ad;verify(d,q,accepted);
        q=base();q.out=(uint8_t *)q.in;q.ad=q.in;verify(d,q,CROSSWEAVE_EOVERLAP);
        q=base();q.tag=q.out+q.n;q.cap=128;verify(d,q,accepted);
        q=base();q.tag=q.out+q.n-1;verify(d,q,CROSSWEAVE_EOVERLAP);
        q=base();q.tag=q.out-15;verify(d,q,CROSSWEAVE_EOVERLAP);
        q=base();q.ad=NULL;q.a=0;verify(d,q,accepted);
        q=base();q.n=0;q.in=NULL;q.out=NULL;q.cap=0;verify(d,q,accepted);
        q=base();q.n=0;q.in=NULL;q.out=NULL;q.cap=1;verify(d,q,CROSSWEAVE_EINVAL);
        q=base();q.n=0;q.in=NULL;q.cap=0;verify(d,q,accepted);
        q=base();q.n=0;q.in=NULL;q.out=q.tag;verify(d,q,accepted);
        q=base();q.n=0;q.out=(uint8_t *)q.key;verify(d,q,accepted);
        q=base();q.a=0;q.out=(uint8_t *)q.ad;verify(d,q,accepted);
    }
    printf("%zu argument/overlap/precedence cases; entire arena failure invariance\n",cases);
    return 0;
}
