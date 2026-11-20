#ifndef CW_BENCH_API_H
#define CW_BENCH_API_H
#include <stddef.h>
#include <stdint.h>
typedef int (*bench_encrypt_fn)(uint8_t *,uint8_t *,const uint8_t *,size_t,const uint8_t *,size_t,const uint8_t *,const uint8_t *);
typedef int (*bench_decrypt_fn)(uint8_t *,const uint8_t *,size_t,const uint8_t *,const uint8_t *,size_t,const uint8_t *,const uint8_t *);
typedef struct {const char *name;size_t key_bytes,nonce_bytes;bench_encrypt_fn encrypt;bench_decrypt_fn decrypt;} bench_api;
const bench_api *bench_baseline(const char *name);
#endif
