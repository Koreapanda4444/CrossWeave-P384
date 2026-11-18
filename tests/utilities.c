#include "crossweave_internal.h"
#include "check.h"
#include <string.h>
int main(void) {
    uint8_t bytes[12]; memset(bytes, 0xa5, sizeof bytes);
    uint64_t x = UINT64_C(0x0123456789abcdef);
    cw_store64(bytes + 1, x);
    CHECK(cw_load64(bytes + 1) == x && bytes[1] == 0xef && bytes[8] == 1);
    CHECK(bytes[0] == 0xa5 && bytes[9] == 0xa5);
    CHECK(cw_rol(x,0) == x && cw_rol(x,64) == x);
    CHECK(cw_rol(x,1) == UINT64_C(0x02468acf13579bde));
    CHECK(cw_ror(cw_rol(x,63),63) == x);
    cw_range a,b,c;
    CHECK(cw_make_range(bytes,8,&a)); CHECK(cw_make_range(bytes+8,4,&b));
    CHECK(!cw_overlap(a,b)); CHECK(cw_make_range(bytes+7,1,&c));
    CHECK(cw_overlap(a,c)); CHECK(cw_make_range(bytes+1,0,&c));
    CHECK(!cw_overlap(a,c)); CHECK(cw_make_range(NULL,0,&c));
    CHECK(!cw_make_range((void *)(UINTPTR_MAX-3u),4,&c));
    cw_clean(bytes,sizeof bytes); for(size_t i=0;i<sizeof bytes;++i) CHECK(bytes[i]==0);
    cw_clean(NULL,0); return 0;
}
