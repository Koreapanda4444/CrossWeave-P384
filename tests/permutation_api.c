#include "crossweave.h"
#include "test_io.h"
int main(void) {
    typedef crossweave_status (*api)(uint8_t*,size_t,const uint8_t*,size_t);
    api functions[2]={crossweave_p384_12,crossweave_p384_16};
    const char *kat[2]={
      "f98e26352d066afa483b96a5fad30f0963895e163d529708102e9e8728393b8bb113373987dbf0066981e2fb0ffc2630",
      "59e853991a8c3b868074ee51104a750d54ca24a15315468d72233a7ed2126b5110b0ef086692e1cc7ba01d0648b15c8f"};
    for(unsigned j=0;j<2;++j) {
        uint8_t in[48]={0},out[96],before[96],expected[48];
        CHECK(unhex(kat[j],expected,48)==48);
        memset(out,0xa5,sizeof out); memcpy(before,out,sizeof out);
        CHECK(functions[j](out,47,in,48)==CROSSWEAVE_ECAPACITY);
        CHECK(functions[j](NULL,0,NULL,47)==CROSSWEAVE_ELENGTH);
        CHECK(functions[j](NULL,47,NULL,48)==CROSSWEAVE_ECAPACITY);
        CHECK(functions[j](out,96,NULL,48)==CROSSWEAVE_EINVAL);
        CHECK(functions[j](NULL,48,in,48)==CROSSWEAVE_EINVAL);
        CHECK(functions[j](out,48,(void *)(UINTPTR_MAX-20u),48)==CROSSWEAVE_EINVAL);
        CHECK(functions[j]((void *)(UINTPTR_MAX-20u),48,in,48)==CROSSWEAVE_EINVAL);
        CHECK(functions[j](out+1,48,out,48)==CROSSWEAVE_EOVERLAP);
        CHECK(functions[j](out,48,out+1,48)==CROSSWEAVE_EOVERLAP);
        CHECK(memcmp(out,before,sizeof out)==0);
        CHECK(functions[j](out,SIZE_MAX,in,48)==CROSSWEAVE_OK);
        CHECK(memcmp(out,expected,48)==0 && memcmp(out+48,before+48,48)==0);
        memset(out,0,48); CHECK(functions[j](out,96,out,48)==CROSSWEAVE_OK);
        CHECK(memcmp(out,expected,48)==0 && memcmp(out+48,before+48,48)==0);
        memset(out,0,96); CHECK(functions[j](out+48,48,out,48)==CROSSWEAVE_OK);
        CHECK(memcmp(out+48,expected,48)==0);
    }
    return 0;
}
