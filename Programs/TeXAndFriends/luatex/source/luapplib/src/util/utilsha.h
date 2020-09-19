/* sha2 implementation excerpted from code by Aaron D. Gifford */

#ifndef UTIL_SHA_H
#define UTIL_SHA_H

#include <stddef.h>
#include <stdint.h>
#include "utildecl.h"

#define SHA256_BLOCK_LENGTH 64
#define SHA256_DIGEST_LENGTH 32
#define SHA256_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH 128
#define SHA384_DIGEST_LENGTH 48
#define SHA384_STRING_LENGTH (SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH 128
#define SHA512_DIGEST_LENGTH 64
#define SHA512_STRING_LENGTH (SHA512_DIGEST_LENGTH * 2 + 1)

//#define sha256_state sha256_state_t
//#define sha384_state sha384_state_t
//#define sha512_state sha512_state_t

typedef struct {
  uint32_t  words[8];
  uint64_t  bitcount;
  union {
    uint8_t buffer[SHA256_BLOCK_LENGTH];
    uint32_t buffer32[SHA256_BLOCK_LENGTH / sizeof(uint32_t)];
    uint64_t buffer64[SHA256_BLOCK_LENGTH / sizeof(uint64_t)];
  };
} sha256_state;

typedef struct {
  uint64_t  words[8];
  uint64_t  bitcount[2];
  union {
    uint8_t buffer[SHA512_BLOCK_LENGTH];
    uint64_t buffer64[SHA512_BLOCK_LENGTH / sizeof(uint64_t)];
  };
} sha512_state;

typedef sha512_state sha384_state;

enum {
  SHA_BYTES = 0,
  SHA_UCHEX = (1<<0),
  SHA_LCHEX = (1<<1)
};

#define SHA_DEFAULT SHA_BYTES
#define SHA_HEX (SHA_UCHEX|SHA_LCHEX)

UTILAPI sha256_state * sha256_digest_init (sha256_state *state);
UTILAPI sha384_state * sha384_digest_init (sha384_state *state);
UTILAPI sha512_state * sha512_digest_init (sha512_state *state);

UTILAPI void sha256_digest_add (sha256_state *state, const void *data, size_t size);
UTILAPI void sha384_digest_add (sha384_state *state, const void *data, size_t size);
UTILAPI void sha512_digest_add (sha512_state *state, const void *data, size_t size);

UTILAPI void sha256_digest_get (sha256_state *state, uint8_t digest[], int flags);
UTILAPI void sha384_digest_get (sha384_state *state, uint8_t digest[], int flags);
UTILAPI void sha512_digest_get (sha512_state *state, uint8_t digest[], int flags);

UTILAPI void sha256_digest (const void *data, size_t size, uint8_t digest[], int flags);
UTILAPI void sha384_digest (const void *data, size_t size, uint8_t digest[], int flags);
UTILAPI void sha512_digest (const void *data, size_t size, uint8_t digest[], int flags);

UTILAPI int sha256_digest_add_file (sha256_state *state, const char *filename);
UTILAPI int sha256_digest_file (const char *filename, uint8_t digest[], int flags);

UTILAPI int sha384_digest_add_file (sha384_state *state, const char *filename);
UTILAPI int sha384_digest_file (const char *filename, uint8_t digest[], int flags);

UTILAPI int sha512_digest_add_file (sha512_state *state, const char *filename);
UTILAPI int sha512_digest_file (const char *filename, uint8_t digest[], int flags);

#endif
