#include "crossweave_internal.h"
#include <stdlib.h>
void *cw_alloc(size_t n) { return malloc(n); }
void cw_free(void *p, size_t n) {
    cw_clean(p,n); free(p);
}
