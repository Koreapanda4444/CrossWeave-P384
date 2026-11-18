#include "crossweave_internal.h"
#include "permutation_test.h"
void cw_test_inverse(uint8_t bytes[48], unsigned rounds) {
    static const unsigned perm[6]={2,5,1,4,0,3}, rot[6]={1,7,19,31,43,57};
    static const unsigned reverse[6]={5,3,1,4,2,0};
    uint64_t s[6], old[6];
    for(unsigned i=0;i<6;++i) s[i]=cw_load64(bytes+8u*i);
    while(rounds>0) {
        unsigned r=--rounds;
        for(unsigned i=0;i<6;++i) old[perm[i]]=cw_ror(s[i],rot[i]);
        old[0]^=cw_rc[r][0]; old[3]^=cw_rc[r][1];
        for(unsigned i=0;i<6;++i) cw_lane_update(old,r,reverse[i]);
        for(unsigned i=0;i<6;++i) s[i]=old[i];
    }
    for(unsigned i=0;i<6;++i) cw_store64(bytes+8u*i,s[i]);
    cw_clean(s,sizeof s); cw_clean(old,sizeof old);
}
