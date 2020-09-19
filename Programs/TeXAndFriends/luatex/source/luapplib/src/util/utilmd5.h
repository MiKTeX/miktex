
/* sha2 implementation excerpted from code by Aaron D. Gifford */

#ifndef UTIL_MD5_H
#define UTIL_MD5_H

#include <stdint.h>
#include <stddef.h> // for size_t
#include "utildecl.h"

//#define md5_state md5_state_t

typedef struct {
  uint32_t bitcount[2];
  uint32_t words[4];
  uint8_t buffer[64];
} md5_state;

#define MD5_DIGEST_LENGTH 16
#define MD5_STRING_LENGTH (MD5_DIGEST_LENGTH * 2 + 1)

enum {
  MD5_BYTES = 0,
  MD5_UCHEX = (1<<0),
  MD5_LCHEX = (1<<1)
};

#define MD5_DEFAULT MD5_BYTES
#define MD5_HEX (MD5_UCHEX|MD5_LCHEX)

#ifdef __cplusplus
extern "C"
{
#endif

UTILAPI md5_state * md5_digest_init (md5_state *state);
UTILAPI void md5_digest_add (md5_state *state, const void *input, size_t size);
UTILAPI void md5_digest_get (md5_state *state, uint8_t digest[], int flags);

UTILAPI void md5_digest (const void *input, size_t length, uint8_t digest[], int flags);

UTILAPI int md5_digest_add_file (md5_state *state, const char *filename);
UTILAPI int md5_digest_file (const char *filename, uint8_t digest[], int flags);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif