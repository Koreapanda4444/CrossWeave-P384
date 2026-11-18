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
