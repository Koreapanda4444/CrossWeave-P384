#include "crossweave_internal.h"
#include <string.h>
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
