#ifndef UTIL_CRYPT_H
#define UTIL_CRYPT_H

#include <stdint.h>
#include <stddef.h>
#include "utiliof.h"

#ifndef UTIL_CRYPT_TIME
#  define UTIL_CRYPT_TIME 0
#endif

/* RC4 */

typedef uint8_t rc4_map[256];

typedef struct rc4_state rc4_state;

#define RC4_STATE_ALLOC (1<<0)

UTILAPI rc4_state * rc4_state_initialize (rc4_state *state, rc4_map *map, const void *vkey, size_t keylength);
#define rc4_state_init(state, vkey, keylength) rc4_state_initialize(state, NULL, vkey, keylength)
UTILAPI void rc4_map_save (rc4_state *state, rc4_map *map);
UTILAPI void rc4_map_restore (rc4_state *state, rc4_map *map);

/* Codecs operating on iof */

UTILAPI iof_status rc4_crypt_state (iof *I, iof *O, rc4_state *state);
#define rc4_encode_state(I, O, state) rc4_crypt_state(I, O, state)
#define rc4_decode_state(I, O, state) rc4_crypt_state(I, O, state)

UTILAPI iof_status rc4_crypt (iof *I, iof *O, const void *key, size_t length);
#define rc4_encode(I, O) rc4_crypt(I, O, key, length)
#define rc4_decode(I, O) rc4_crypt(I, O, key, length)

UTILAPI size_t rc4_crypt_data (const void *input, size_t length, void *output, const void *key, size_t keylength);
UTILAPI size_t rc4_crypt_state_data (rc4_state *state, const void *input, size_t length, void *output);
#define rc4_encode_data(input, length, output, key, keylength) rc4_crypt_data(input, length, output, key, keylength)
#define rc4_decode_data(input, length, output, key, keylength) rc4_crypt_data(input, length, output, key, keylength)
#define rc4_encode_state_data(state, input, length, output) rc4_crypt_state_data(state, input, length, output)
#define rc4_decode_state_data(state, input, length, output) rc4_crypt_state_data(state, input, length, output)

UTILAPI void rc4_state_close (rc4_state *state);

/* AES */

typedef uint8_t aes_block[4][4];
typedef aes_block aes_keyblock[15]; // aes128 - 10+1, aes192 - 12+1, aes256 - 14+1

typedef struct aes_state aes_state;

#define AES_STATE_ALLOC (1<<0)
//#define AES_ECB_MODE (1<<2)
#define AES_HAS_IV (1<<3)
#define AES_INLINE_IV (1<<4)
#define AES_CONTINUE (1<<5)
#define AES_NULL_PADDING (1<<6)

UTILAPI void aes_pdf_mode (aes_state *state);
//UTILAPI aes_state * aes_state_initialize_ecb (aes_state *State, uint8_t *roundkey, const uint8_t *key);
UTILAPI aes_state * aes_encode_initialize (aes_state *state, aes_keyblock *keyblock, const void *key, size_t keylength, const void *iv);
UTILAPI aes_state * aes_decode_initialize (aes_state *state, aes_keyblock *keyblock, const void *key, size_t keylength, const void *iv);
#define aes_encode_init(state, key, keylength) aes_encode_initialize(state, NULL, key, keylength, NULL)
#define aes_decode_init(state, key, keylength) aes_decode_initialize(state, NULL, key, keylength, NULL)

UTILAPI void aes_state_close (aes_state *state);

/* Codecs operating on iof */

UTILAPI iof_status aes_encode_state (iof *I, iof *O, aes_state *state);
UTILAPI iof_status aes_decode_state (iof *I, iof *O, aes_state *state);

UTILAPI size_t aes_encode_data (const void *input, size_t length, void *output, const void *key, size_t keylength, const void *iv, int flags);
UTILAPI size_t aes_encode_state_data (aes_state *state, const void *input, size_t length, void *output);
UTILAPI size_t aes_decode_data (const void *input, size_t length, void *output, const void *key, size_t keylength, const void *iv, int flags);
UTILAPI size_t aes_decode_state_data (aes_state *state, const void *input, size_t length, void *output);

/* random bytes generator */

UTILAPI void random_bytes (uint8_t *output, size_t size);
UTILAPI void aes_generate_iv (uint8_t output[16]);

/* filters */

iof * iof_filter_rc4_decoder (iof *N, const void *key, size_t keylength);
iof * iof_filter_rc4_encoder (iof *N, const void *key, size_t keylength);

iof * iof_filter_aes_decoder (iof *N, const void *key, size_t keylength);
iof * iof_filter_aes_encoder (iof *N, const void *key, size_t keylength);

#endif