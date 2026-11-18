#include "crossweave_internal.h"
#include "fault_support.h"
#include "check.h"
#include <openssl/crypto.h>
#include <stdlib.h>
#include <string.h>
static fault_stats stats;
static int fail_allocation;
void fault_reset(int fail) {
    CHECK(stats.active==0);
    memset(&stats,0,sizeof stats); fail_allocation=fail;
}
fault_stats fault_get(void) { return stats; }
void *cw_alloc(size_t n) {
    CHECK(n!=0); ++stats.allocations;
    if(fail_allocation) return NULL;
    void *p=malloc(n); CHECK(p!=NULL);
    memset(p,0xa5,n); ++stats.active; stats.bytes+=n; return p;
}
void cw_clean(void *p, size_t n) {
    if(n!=0) OPENSSL_cleanse(p,n);
    const volatile uint8_t *bytes=p;
    for(size_t i=0;i<n;++i) CHECK(bytes[i]==0);
    stats.last[0]=stats.last[1]; stats.last[1]=n; ++stats.wipes;
}
void cw_free(void *p, size_t n) {
    if(p==NULL) { CHECK(n==0); return; }
    CHECK(stats.active==1); cw_clean(p,n);
    ++stats.frees; --stats.active; free(p);
}
