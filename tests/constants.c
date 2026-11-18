#include "crossweave_internal.h"
#include "test_io.h"
#include "shake.h"
int main(void) {
    uint8_t result[16], le[8]; char text[64];
    CHECK(cw_rc[0][0] == UINT64_C(0xcab26712b1223857));
    CHECK(cw_rc[0][1] == UINT64_C(0x52cf70dd96cd3c07));
    for(unsigned r=0;r<16;++r) for(unsigned j=0;j<2;++j) {
        int len=snprintf(text,sizeof text,"cw-p384/round-%u/lane-%u",r,j*3u);
        CHECK(len>0 && (size_t)len<sizeof text);
        CHECK(test_xof(EVP_shake256(),text,(size_t)len,result,8));
        cw_store64(le,cw_rc[r][j]); CHECK(memcmp(le,result,8)==0);
        CHECK(test_xof(EVP_shake256(),text,(size_t)len+1u,result,8));
        CHECK(memcmp(le,result,8)!=0); printhex(le,8);
    }
    CHECK(test_xof(EVP_shake256(),"CrossWeave/IV",sizeof("CrossWeave/IV")-1,result,16));
    CHECK(memcmp(cw_iv,result,16)==0); printhex(cw_iv,16);
    CHECK(test_xof(EVP_shake256(),"CrossWeave/IV",sizeof("CrossWeave/IV"),result,16));
    CHECK(memcmp(cw_iv,result,16)!=0);
    CHECK(test_xof(EVP_shake256(),"CrossWeave/iv",sizeof("CrossWeave/iv")-1,result,16));
    CHECK(memcmp(cw_iv,result,16)!=0); return 0;
}
