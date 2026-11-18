#ifndef CROSSWEAVE_H
#define CROSSWEAVE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define CROSSWEAVE_KEY_BYTES 16u
#define CROSSWEAVE_NONCE_BYTES 16u
#define CROSSWEAVE_TAG_BYTES 16u
#define CROSSWEAVE_RATE_BYTES 16u
#define CROSSWEAVE_STATE_BYTES 48u
#define CROSSWEAVE_MAX_INPUT_BYTES ((size_t)16777216)
typedef enum {
    CROSSWEAVE_OK = 0, CROSSWEAVE_EINVAL = 1, CROSSWEAVE_ELENGTH = 2,
    CROSSWEAVE_ECAPACITY = 3, CROSSWEAVE_EOVERLAP = 4,
    CROSSWEAVE_ENOMEM = 5, CROSSWEAVE_EAUTH = 7
} crossweave_status;
crossweave_status crossweave_p384_12(uint8_t *out, size_t out_capacity,
    const uint8_t *in, size_t in_len);
crossweave_status crossweave_p384_16(uint8_t *out, size_t out_capacity,
    const uint8_t *in, size_t in_len);
#ifdef __cplusplus
}
#endif
#endif
