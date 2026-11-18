#ifndef CW_TEST_IO_H
#define CW_TEST_IO_H
#include "check.h"
#include <stdint.h>
#include <string.h>
static inline unsigned hex_digit(char c) {
    if(c >= '0' && c <= '9') return (unsigned)(c-'0');
    if(c >= 'a' && c <= 'f') return (unsigned)(c-'a')+10u;
    CHECK(0); return 0;
}
static inline size_t unhex(const char *s, uint8_t *p, size_t cap) {
    size_t n = strlen(s); CHECK(n%2u == 0 && n/2u <= cap);
    for(size_t i=0;i<n/2u;++i) p[i]=(uint8_t)(16u*hex_digit(s[2*i])+hex_digit(s[2*i+1]));
    return n/2u;
}
static inline void printhex(const uint8_t *p, size_t n) {
    for(size_t i=0;i<n;++i) printf("%02x",(unsigned)p[i]);
    putchar('\n');
}
#endif
