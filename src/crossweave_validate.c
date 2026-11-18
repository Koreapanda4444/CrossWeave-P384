#include "crossweave_internal.h"
crossweave_status cw_validate_aead(int decrypt, uint8_t *out, size_t cap,
        const uint8_t *in, size_t n, const uint8_t *tag, size_t tag_extent,
        const uint8_t *ad, size_t a, const uint8_t *nonce, size_t nonce_n,
        const uint8_t *key, size_t key_n) {
    if(key_n!=16u || nonce_n!=16u || (decrypt && tag_extent!=16u) ||
       n>CROSSWEAVE_MAX_INPUT_BYTES || a>CROSSWEAVE_MAX_INPUT_BYTES)
        return CROSSWEAVE_ELENGTH;
    if(cap<n || (!decrypt && tag_extent<16u)) return CROSSWEAVE_ECAPACITY;
    if(key==NULL || nonce==NULL || tag==NULL || (ad==NULL && a!=0) ||
       (in==NULL && n!=0) || (out==NULL && (n!=0 || cap!=0)))
        return CROSSWEAVE_EINVAL;
    const void *p[6]={out,in,tag,ad,nonce,key};
    const size_t sizes[6]={n,n,16,a,16,16};
    cw_range ranges[6];
    for(unsigned i=0;i<6;++i)
        if(!cw_make_range(p[i],sizes[i],&ranges[i])) return CROSSWEAVE_EINVAL;
    if(out!=in && cw_overlap(ranges[0],ranges[1])) return CROSSWEAVE_EOVERLAP;
    for(unsigned i=2;i<6;++i)
        if(cw_overlap(ranges[0],ranges[i])) return CROSSWEAVE_EOVERLAP;
    if(!decrypt) {
        for(unsigned i=1;i<6;++i)
            if(i!=2u && cw_overlap(ranges[2],ranges[i])) return CROSSWEAVE_EOVERLAP;
    }
    return CROSSWEAVE_OK;
}
