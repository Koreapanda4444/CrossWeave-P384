#include "crossweave_internal.h"
#include "permutation_test.h"
#include "test_io.h"
static void hex(const uint8_t *p,size_t n){putchar('"');for(size_t i=0;i<n;++i)printf("%02x",(unsigned)p[i]);putchar('"');}
static void event(const uint8_t before[48],const uint8_t after[48],size_t n,int *first){
    uint8_t pre[48];memcpy(pre,after,48);cw_test_inverse(pre,12);
    printf("%s{\"length\":%zu,\"domain\":%u,\"before\":",*first?"":",",n,(unsigned)(before[47]^pre[47]));*first=0;
    hex(before,48);printf(",\"pre\":");hex(pre,48);printf(",\"after\":");hex(after,48);putchar('}');
}
int main(void){char line[140];
    while(fgets(line,sizeof line,stdin)!=NULL){
        line[strcspn(line,"\r\n")]='\0';char *comma=strchr(line,',');CHECK(comma && !strchr(comma+1,','));*comma='\0';
        uint8_t ad[33],p[33],c[33],tag[16],s[48],before[48],key[16]={0},nonce[16]={0};
        size_t a=unhex(line,ad,33),n=unhex(comma+1,p,33);cw_initialize(s,key,nonce);int first=1;
        printf("{\"events\":[");size_t offset=0;
        for(;offset+16<=a;offset+=16){memcpy(before,s,48);cw_ad_full(s,ad+offset);event(before,s,16,&first);}
        memcpy(before,s,48);cw_ad_tail(s,ad+offset,a-offset);event(before,s,a-offset,&first);
        offset=0;
        for(;offset+16<=n;offset+=16){memcpy(before,s,48);cw_message_block(s,c+offset,p+offset,16,0,1);event(before,s,16,&first);}
        memcpy(before,s,48);cw_message_block(s,c+offset,p+offset,n-offset,0,0);event(before,s,n-offset,&first);
        cw_finalize(s,tag,key);printf("],\"ciphertext\":");hex(c,n);printf(",\"tag\":");hex(tag,16);puts("}");
    }
    CHECK(!ferror(stdin) && !ferror(stdout));return 0;
}
