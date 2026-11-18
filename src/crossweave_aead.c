#include "crossweave_internal.h"
#include <string.h>
#include <openssl/crypto.h>
void cw_initialize(uint8_t s[48], const uint8_t key[16], const uint8_t nonce[16]) {
    memcpy(s,nonce,16); memcpy(s+16,key,16); memcpy(s+32,cw_iv,16);
    cw_permute(s,16);
    for(size_t j=0;j<16;++j) s[16+j]^=key[j];
}
void cw_ad_full(uint8_t s[48], const uint8_t block[16]) {
    for(size_t j=0;j<16;++j) s[j]^=block[j];
    s[47]^=0x11; cw_permute(s,12);
}
void cw_ad_tail(uint8_t s[48], const uint8_t *tail, size_t n) {
    for(size_t j=0;j<n;++j) s[j]^=tail[j];
    s[n]^=0x80; s[47]^=0x12; cw_permute(s,12);
}
void cw_absorb_ad(uint8_t s[48], const uint8_t *ad, size_t n) {
    while(n>=16u) { cw_ad_full(s,ad); ad+=16; n-=16u; }
    cw_ad_tail(s,ad,n);
}
void cw_message_block(uint8_t s[48], uint8_t *out, const uint8_t *in, size_t n, int decrypt, int full) {
    for(size_t j=0;j<n;++j) {
        uint8_t input=in[j], output=(uint8_t)(input^s[j]);
        out[j]=output; s[j]=decrypt?input:output;
    }
    if(!full) s[n]^=0x80;
    s[47]^=full?0x21u:0x22u; cw_permute(s,12);
}
void cw_message(uint8_t s[48], uint8_t *out, const uint8_t *in, size_t n, int decrypt) {
    while(n>=16u) { cw_message_block(s,out,in,16,decrypt,1); in+=16; out+=16; n-=16u; }
    cw_message_block(s,out,in,n,decrypt,0);
}
void cw_finalize(uint8_t s[48], uint8_t tag[16], const uint8_t key[16]) {
    for(size_t j=0;j<16;++j) s[16+j]^=key[j];
    s[47]^=0x31; cw_permute(s,16);
    for(size_t j=0;j<16;++j) tag[j]=(uint8_t)(s[j]^key[j]);
}

crossweave_status crossweave_encrypt(uint8_t *ciphertext, size_t ciphertext_capacity,
        uint8_t *tag, size_t tag_capacity, const uint8_t *plaintext, size_t plaintext_len,
        const uint8_t *ad, size_t ad_len, const uint8_t *nonce, size_t nonce_len,
        const uint8_t *key, size_t key_len) {
    crossweave_status status=cw_validate_aead(0,ciphertext,ciphertext_capacity,
        plaintext,plaintext_len,tag,tag_capacity,ad,ad_len,nonce,nonce_len,key,key_len);
    if(status!=CROSSWEAVE_OK) return status;
    uint8_t *tmp=NULL;
    if(plaintext_len!=0) { tmp=cw_alloc(plaintext_len); if(tmp==NULL) return CROSSWEAVE_ENOMEM; }
    uint8_t s[48],local_tag[16];
    cw_initialize(s,key,nonce); cw_absorb_ad(s,ad,ad_len);
    cw_message(s,tmp,plaintext,plaintext_len,0); cw_finalize(s,local_tag,key);
    if(plaintext_len!=0) memcpy(ciphertext,tmp,plaintext_len);
    memcpy(tag,local_tag,16);
    cw_free(tmp,plaintext_len); cw_clean(local_tag,sizeof local_tag); cw_clean(s,sizeof s);
    return CROSSWEAVE_OK;
}

crossweave_status crossweave_decrypt(uint8_t *plaintext, size_t plaintext_capacity,
        const uint8_t *ciphertext, size_t ciphertext_len, const uint8_t *tag, size_t tag_len,
        const uint8_t *ad, size_t ad_len, const uint8_t *nonce, size_t nonce_len,
        const uint8_t *key, size_t key_len) {
    crossweave_status status=cw_validate_aead(1,plaintext,plaintext_capacity,
        ciphertext,ciphertext_len,tag,tag_len,ad,ad_len,nonce,nonce_len,key,key_len);
    if(status!=CROSSWEAVE_OK) return status;
    uint8_t *tmp=NULL;
    if(ciphertext_len!=0) { tmp=cw_alloc(ciphertext_len); if(tmp==NULL) return CROSSWEAVE_ENOMEM; }
    uint8_t s[48],local_tag[16];
    cw_initialize(s,key,nonce); cw_absorb_ad(s,ad,ad_len);
    cw_message(s,tmp,ciphertext,ciphertext_len,1); cw_finalize(s,local_tag,key);
    status=CRYPTO_memcmp(tag,local_tag,16)==0?CROSSWEAVE_OK:CROSSWEAVE_EAUTH;
    if(status==CROSSWEAVE_OK && ciphertext_len!=0) memcpy(plaintext,tmp,ciphertext_len);
    cw_free(tmp,ciphertext_len); cw_clean(local_tag,sizeof local_tag); cw_clean(s,sizeof s);
    return status;
}
