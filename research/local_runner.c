#include "crossweave_internal.h"
#include "check.h"
#include "binary_io.h"
#include <stdlib.h>
int main(int argc,char **argv){
    research_binary_stdin();CHECK(argc==4);char *end;unsigned long r=strtoul(argv[1],&end,10);CHECK(!*end && r<16);
    unsigned long i=strtoul(argv[2],&end,10);CHECK(!*end && i<6);
    unsigned long n=strtoul(argv[3],&end,10);CHECK(!*end && n>0 && n<=16384);
    const uint64_t m=UINT64_C(1)<<63;size_t ah[2]={0},bh[2]={0},bothh[4]={0};
    for(unsigned long j=0;j<n;++j){uint8_t bytes[16];CHECK(fread(bytes,1,16,stdin)==16);
        uint64_t a=cw_load64(bytes),b=cw_load64(bytes+8),base=cw_local(a,b,(unsigned)r,(unsigned)i);
        uint64_t da=cw_local(a^m,b,(unsigned)r,(unsigned)i)^base,db=cw_local(a,b^m,(unsigned)r,(unsigned)i)^base;
        unsigned alpha=(7u*(unsigned)i+3u*(unsigned)r)%63u+1u,beta=(13u*(unsigned)i+5u*(unsigned)r)%63u+1u;
        CHECK(da==(cw_rol(m,alpha)^m^(m&cw_rol(b,11))));
        CHECK(db==(cw_rol(m,beta)^m^(a&cw_rol(m,11))));
        uint64_t both=cw_local(a^m,b^m,(unsigned)r,(unsigned)i)^base;CHECK(both==(da^db));
        ++ah[(da^(cw_rol(m,alpha)^m))!=0];++bh[(db^(cw_rol(m,beta)^m))!=0];
        if(r==0 && i==0){CHECK((both&~(m|UINT64_C(1024)))==0);++bothh[((both&m)!=0?2u:0u)+((both&1024u)!=0?1u:0u)];}
    }
    CHECK(fgetc(stdin)==EOF && !ferror(stdin));
    printf("{\"a\":[%zu,%zu],\"b\":[%zu,%zu],\"both\":[%zu,%zu,%zu,%zu]}\n",ah[0],ah[1],bh[0],bh[1],bothh[0],bothh[1],bothh[2],bothh[3]);return 0;
}
