#include "bench_api.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
static int crypt(int decrypt,const EVP_CIPHER *cipher,uint8_t *out,uint8_t *tag_out,const uint8_t *in,size_t n,const uint8_t *tag_in,const uint8_t *ad,size_t a,const uint8_t *nonce,const uint8_t *key){
    if(n>(size_t)INT_MAX || a>(size_t)INT_MAX)return 2;
    if((n && (!out || !in)) || (a && !ad) || !nonce || !key || (decrypt?!tag_in:!tag_out))return 1;
    uint8_t *tmp=n?malloc(n):NULL,final[16]={0},local_tag[16]={0};
    if(n && !tmp)return 5;
    EVP_CIPHER_CTX *ctx=EVP_CIPHER_CTX_new();int status=1,len=0,final_len=0;
    if(!ctx){status=5;goto done;}
    if(EVP_CipherInit_ex(ctx,cipher,NULL,NULL,NULL,!decrypt)!=1 || EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_AEAD_SET_IVLEN,12,NULL)!=1 || EVP_CipherInit_ex(ctx,NULL,NULL,key,nonce,!decrypt)!=1)goto done;
    if(a && EVP_CipherUpdate(ctx,NULL,&len,ad,(int)a)!=1)goto done;
    if(n && (EVP_CipherUpdate(ctx,tmp,&len,in,(int)n)!=1 || len!=(int)n))goto done;
    if(decrypt && EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_AEAD_SET_TAG,16,(void *)tag_in)!=1)goto done;
    if(EVP_CipherFinal_ex(ctx,final,&final_len)!=1){status=decrypt?7:1;goto done;}
    if(final_len!=0)goto done;
    if(!decrypt && EVP_CIPHER_CTX_ctrl(ctx,EVP_CTRL_AEAD_GET_TAG,16,local_tag)!=1)goto done;
    if(n)memcpy(out,tmp,n);
    if(!decrypt)memcpy(tag_out,local_tag,16);
    status=0;
done:
    if(tmp){OPENSSL_cleanse(tmp,n);free(tmp);}OPENSSL_cleanse(final,16);OPENSSL_cleanse(local_tag,16);EVP_CIPHER_CTX_free(ctx);return status;
}
#define WRAPPERS(prefix,getter) \
static int prefix##_enc(uint8_t *c,uint8_t *t,const uint8_t *p,size_t n,const uint8_t *ad,size_t a,const uint8_t *nonce,const uint8_t *key){return crypt(0,getter(),c,t,p,n,NULL,ad,a,nonce,key);} \
static int prefix##_dec(uint8_t *p,const uint8_t *c,size_t n,const uint8_t *t,const uint8_t *ad,size_t a,const uint8_t *nonce,const uint8_t *key){return crypt(1,getter(),p,NULL,c,n,t,ad,a,nonce,key);}
WRAPPERS(aes,EVP_aes_128_gcm)
WRAPPERS(chacha,EVP_chacha20_poly1305)
const bench_api *bench_baseline(const char *name){
    static const bench_api aes={"AES-128-GCM",16,12,aes_enc,aes_dec},chacha={"ChaCha20-Poly1305",32,12,chacha_enc,chacha_dec};
    if(strcmp(name,aes.name)==0)return &aes;
    if(strcmp(name,chacha.name)==0)return &chacha;
    return NULL;
}
