#ifndef CW_FAULT_SUPPORT_H
#define CW_FAULT_SUPPORT_H
#include <stddef.h>
typedef struct {
    size_t allocations,frees,active,bytes,wipes;
    size_t last[2];
} fault_stats;
void fault_reset(int fail);
fault_stats fault_get(void);
#endif
