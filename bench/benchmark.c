#include "crossweave.h"
#include "bench_api.h"
#include "check.h"
#include <openssl/rand.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif
static volatile uint64_t sink;
static double now(void){
#ifdef _WIN32
    LARGE_INTEGER value,freq;CHECK(QueryPerformanceCounter(&value) && QueryPerformanceFrequency(&freq));return (double)value.QuadPart/(double)freq.QuadPart;
#else
    struct timespec ts;CHECK(clock_gettime(CLOCK_MONOTONIC,&ts)==0);return (double)ts.tv_sec+(double)ts.tv_nsec/1e9;
#endif
}
static int enc(uint8_t *c,uint8_t *t,const uint8_t *p,size_t n,const uint8_t *ad,size_t a,const uint8_t *nonce,const uint8_t *key){return (int)crossweave_encrypt(c,n,t,16,p,n,ad,a,nonce,16,key,16);}
static int dec(uint8_t *p,const uint8_t *c,size_t n,const uint8_t *t,const uint8_t *ad,size_t a,const uint8_t *nonce,const uint8_t *key){return (int)crossweave_decrypt(p,n,c,n,t,16,ad,a,nonce,16,key,16);}
static const bench_api cw={"CrossWeave",16,16,enc,dec};
static void nonce(uint8_t *p,size_t n,uint64_t value){memset(p,0,n);for(unsigned i=0;i<8;++i)p[i]=(uint8_t)(value>>(8u*i));}
static void numbers(const double values[5]){putchar('[');for(unsigned i=0;i<5;++i){printf("%s%.3f",i?",":"",values[i]);}putchar(']');}
int main(int argc,char **argv){
    CHECK(argc==4);const char *algorithm=argv[1];char *end;
    unsigned long ad_raw=strtoul(argv[2],&end,10);CHECK(!*end && ad_raw<=1024);
    unsigned long msg_raw=strtoul(argv[3],&end,10);CHECK(!*end && msg_raw<=1048576);
    size_t a=(size_t)ad_raw,n=(size_t)msg_raw;
    double encryption[5],decryption[5],failure[5];
    if(strcmp(algorithm,"P12")==0 || strcmp(algorithm,"P16")==0){
        uint8_t s[48]={0};size_t iterations=8192;
        crossweave_status (*permutation)(uint8_t *,size_t,const uint8_t *,size_t)=strcmp(algorithm,"P12")==0?crossweave_p384_12:crossweave_p384_16;
        for(unsigned warm=0;warm<32;++warm)CHECK(permutation(s,48,s,48)==CROSSWEAVE_OK);
        for(unsigned sample=0;sample<5;++sample){double start=now();
            for(size_t i=0;i<iterations;++i){CHECK(permutation(s,48,s,48)==CROSSWEAVE_OK);sink+=(uint64_t)s[0];}
            encryption[sample]=(now()-start)*1e9/(double)iterations;
        }
        printf("{\"algorithm\":\"%s\",\"iterations\":%zu,\"latency_ns\":",algorithm,iterations);numbers(encryption);printf(",\"checksum\":%llu}\n",(unsigned long long)sink);return 0;
    }
    const bench_api *api=strcmp(algorithm,"CrossWeave")==0?&cw:bench_baseline(algorithm);CHECK(api!=NULL);
    uint8_t key[32],tag[16],badtag[16];CHECK(RAND_bytes(key,(int)api->key_bytes)==1);
    uint8_t *p=malloc(n?n:1),*c=malloc(n?n:1),*out=malloc(n?n:1),*ad=malloc(a?a:1);CHECK(p&&c&&out&&ad);
    for(size_t i=0;i<n;++i){p[i]=(uint8_t)i;}
    for(size_t i=0;i<a;++i){ad[i]=(uint8_t)(i*3u);}
    size_t unit=n+a;if(unit<16)unit=16;size_t iterations=4194304u/unit;if(iterations<4)iterations=4;if(iterations>2048)iterations=2048;
    uint8_t *nonces=malloc(iterations*api->nonce_bytes);CHECK(nonces);uint64_t counter=1;uint8_t warmnonce[16];
    for(unsigned warm=0;warm<8;++warm){nonce(warmnonce,api->nonce_bytes,counter++);CHECK(api->encrypt(c,tag,p,n,ad,a,warmnonce,key)==0);CHECK(api->decrypt(out,c,n,tag,ad,a,warmnonce,key)==0);memcpy(badtag,tag,16);badtag[0]^=1;CHECK(api->decrypt(out,c,n,badtag,ad,a,warmnonce,key)==7);}
    for(unsigned sample=0;sample<5;++sample){
        for(size_t i=0;i<iterations;++i){CHECK(counter!=UINT64_MAX);nonce(nonces+i*api->nonce_bytes,api->nonce_bytes,counter++);}
        double start=now();for(size_t i=0;i<iterations;++i){CHECK(api->encrypt(c,tag,p,n,ad,a,nonces+i*api->nonce_bytes,key)==0);sink+=(uint64_t)tag[0]+(n?c[0]:0u);}encryption[sample]=(now()-start)*1e9/(double)iterations;
        const uint8_t *last=nonces+(iterations-1)*api->nonce_bytes;
        start=now();for(size_t i=0;i<iterations;++i){CHECK(api->decrypt(out,c,n,tag,ad,a,last,key)==0);sink+=(uint64_t)(n?out[n-1]:tag[0]);}decryption[sample]=(now()-start)*1e9/(double)iterations;CHECK(memcmp(out,p,n)==0);
        memcpy(badtag,tag,16);badtag[0]^=1;memset(out,0xa5,n);
        start=now();for(size_t i=0;i<iterations;++i){CHECK(api->decrypt(out,c,n,badtag,ad,a,last,key)==7);sink+=7u;}failure[sample]=(now()-start)*1e9/(double)iterations;
        for(size_t i=0;i<n;++i)CHECK(out[i]==0xa5);
    }
    printf("{\"algorithm\":\"%s\",\"key_bytes\":%zu,\"nonce_bytes\":%zu,\"ad_bytes\":%zu,\"message_bytes\":%zu,\"iterations\":%zu,\"temporary_heap_bytes\":%zu,\"encrypt_ns\":",api->name,api->key_bytes,api->nonce_bytes,a,n,iterations,n);numbers(encryption);
    printf(",\"decrypt_ns\":");numbers(decryption);printf(",\"failure_ns\":");numbers(failure);printf(",\"failure_output_unchanged\":true,\"checksum\":%llu}\n",(unsigned long long)sink);
    free(nonces);free(ad);free(out);free(c);free(p);return 0;
}
