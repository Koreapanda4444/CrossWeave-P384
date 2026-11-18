#include "crossweave_internal.h"
uint64_t cw_local(uint64_t a, uint64_t b, unsigned r, unsigned i) {
    unsigned alpha=(7u*i+3u*r)%63u+1u, beta=(13u*i+5u*r)%63u+1u;
    return cw_rol(a,alpha)^cw_rol(b,beta)^(a&cw_rol(b,11))^(a+b);
}
void cw_lane_update(uint64_t s[6], unsigned r, unsigned i) {
    s[i]^=cw_local(s[(i+1u)%6u],s[(i+3u)%6u],r,i);
}
void cw_updates(uint64_t s[6], unsigned r) {
    static const unsigned order[6]={0,2,4,1,3,5};
    for(unsigned k=0;k<6;++k) cw_lane_update(s,r,order[k]);
}
void cw_round(uint64_t s[6], unsigned r) {
    static const unsigned perm[6]={2,5,1,4,0,3}, rot[6]={1,7,19,31,43,57};
    uint64_t t[6];
    cw_updates(s,r); s[0]^=cw_rc[r][0]; s[3]^=cw_rc[r][1];
    for(unsigned i=0;i<6;++i) t[i]=cw_rol(s[perm[i]],rot[i]);
    for(unsigned i=0;i<6;++i) s[i]=t[i];
    cw_clean(t,sizeof t);
}
void cw_permute(uint8_t bytes[48], unsigned rounds) {
    uint64_t s[6];
    for(unsigned i=0;i<6;++i) s[i]=cw_load64(bytes+8u*i);
    for(unsigned r=0;r<rounds;++r) cw_round(s,r);
    for(unsigned i=0;i<6;++i) cw_store64(bytes+8u*i,s[i]);
    cw_clean(s,sizeof s);
}

static crossweave_status fixed_permutation(uint8_t *out, size_t cap,
        const uint8_t *in, size_t len, unsigned rounds) {
    cw_range output,input;
    if(len!=48u) return CROSSWEAVE_ELENGTH;
    if(cap<48u) return CROSSWEAVE_ECAPACITY;
    if(out==NULL || in==NULL) return CROSSWEAVE_EINVAL;
    if(!cw_make_range(out,48,&output) || !cw_make_range(in,48,&input))
        return CROSSWEAVE_EINVAL;
    if(out!=in && cw_overlap(output,input)) return CROSSWEAVE_EOVERLAP;
    uint8_t tmp[48];
    for(size_t i=0;i<48;++i) tmp[i]=in[i];
    cw_permute(tmp,rounds);
    for(size_t i=0;i<48;++i) out[i]=tmp[i];
    cw_clean(tmp,sizeof tmp); return CROSSWEAVE_OK;
}
crossweave_status crossweave_p384_12(uint8_t *out, size_t cap, const uint8_t *in, size_t len) {
    return fixed_permutation(out,cap,in,len,12);
}
crossweave_status crossweave_p384_16(uint8_t *out, size_t cap, const uint8_t *in, size_t len) {
    return fixed_permutation(out,cap,in,len,16);
}
