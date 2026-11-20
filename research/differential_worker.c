#include "crossweave_internal.h"
#include "check.h"
#include "binary_io.h"
#include <string.h>
static void delta(uint8_t d[48],unsigned index){
    static const unsigned bits[8]={0,1,7,15,31,32,47,63};memset(d,0,48);
    if(index<48){unsigned b=(index/8u)*64u+bits[index%8u];d[b/8u]=(uint8_t)(1u<<(b%8u));}
    else if(index<54)memset(d+(index-48u)*8u,255,8);
    else memset(d,255,48);
}
int main(int argc,char **argv){
    research_binary_stdin();CHECK(argc==4);char *end;unsigned long rounds=strtoul(argv[1],&end,10);CHECK(!*end && rounds<=16);
    unsigned long index=strtoul(argv[2],&end,10);CHECK(!*end && index<55);
    unsigned long n=strtoul(argv[3],&end,10);CHECK(!*end && n>0 && n<=16384);
    size_t counts[384]={0};uint8_t mask[48],a[48],b[48];delta(mask,(unsigned)index);
    for(unsigned long j=0;j<n;++j){CHECK(fread(a,1,48,stdin)==48);for(size_t k=0;k<48;++k)b[k]=(uint8_t)(a[k]^mask[k]);
        cw_permute(a,(unsigned)rounds);cw_permute(b,(unsigned)rounds);
        for(size_t k=0;k<48;++k){unsigned x=(unsigned)(a[k]^b[k]);for(unsigned bit=0;bit<8;++bit)counts[8*k+bit]+=(x>>bit)&1u;}
    }
    CHECK(fgetc(stdin)==EOF && !ferror(stdin));putchar('[');
    for(size_t i=0;i<384;++i){printf("%s%zu",i?",":"",counts[i]);}
    puts("]");return 0;
}
