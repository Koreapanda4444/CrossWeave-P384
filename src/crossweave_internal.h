#ifndef CW_INTERNAL_H
#define CW_INTERNAL_H
#include "crossweave.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && sizeof(uint64_t) == 8, "byte/lane widths");
typedef struct { uintptr_t first, end; } cw_range;
static inline uint64_t cw_load64(const uint8_t *p) {
    uint64_t x = 0;
    for (unsigned i = 0; i < 8; ++i) x |= (uint64_t)p[i] << (8u * i);
    return x;
}
static inline void cw_store64(uint8_t *p, uint64_t x) {
    for (unsigned i = 0; i < 8; ++i) p[i] = (uint8_t)(x >> (8u * i));
}
static inline uint64_t cw_rol(uint64_t x, unsigned n) {
    n &= 63u;
    return (x << n) | (x >> ((64u - n) & 63u));
}
static inline uint64_t cw_ror(uint64_t x, unsigned n) {
    n &= 63u;
    return (x >> n) | (x << ((64u - n) & 63u));
}
static inline int cw_make_range(const void *p, size_t n, cw_range *r) {
    uintptr_t a = (uintptr_t)p;
    if (n > UINTPTR_MAX - a) return 0;
    r->first = a;
    r->end = a + n;
    return 1;
}
static inline int cw_overlap(cw_range a, cw_range b) {
    return a.first != a.end && b.first != b.end && a.first < b.end && b.first < a.end;
}
void cw_clean(void *p, size_t n);
extern const uint64_t cw_rc[16][2];
extern const uint8_t cw_iv[16];
uint64_t cw_local(uint64_t a, uint64_t b, unsigned r, unsigned i);
void cw_lane_update(uint64_t s[6], unsigned r, unsigned i);
void cw_updates(uint64_t s[6], unsigned r);
void cw_round(uint64_t s[6], unsigned r);
void cw_permute(uint8_t bytes[48], unsigned rounds);
void cw_initialize(uint8_t s[48], const uint8_t key[16], const uint8_t nonce[16]);
void cw_ad_full(uint8_t s[48], const uint8_t block[16]);
void cw_ad_tail(uint8_t s[48], const uint8_t *tail, size_t n);
void cw_absorb_ad(uint8_t s[48], const uint8_t *ad, size_t n);
#endif
