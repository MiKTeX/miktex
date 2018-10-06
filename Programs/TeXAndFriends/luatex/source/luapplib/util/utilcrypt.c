
#include "utilmem.h"
#include "utilcrypt.h"
#include "utilcryptdef.h"
#include "utilmd5.h"

/* rc4 */

/*
Initializer arguments:
- state - crypt state
- map - a space for rc4 bytes map; may be left NULL in which case will be allocated
- vkey - crypt key; may be left NULL iff map is provided and properly initialized
- keylength - the length of crypt key (from 5 to 16 bytes)
*/

rc4_state * rc4_state_initialize (rc4_state *state, rc4_map *map, const void *vkey, size_t keylength)
{
  int i, j;
  uint8_t tmp;
  const uint8_t *key;
  key = (const uint8_t *)vkey;
  if (keylength == 0 || keylength > 256)
    return NULL;
  state->flags = 0;
  if (map != NULL)
  {
    state->map = map;
  }
  else
  {
    state->map = (rc4_map *)util_malloc(sizeof(rc4_map));
    state->flags |= RC4_STATE_ALLOC;
  }

  if (key != NULL)
  {
    for (i = 0; i < 256; ++i)
      state->smap[i] = (uint8_t)i;
    for (i = 0, j = 0; i < 256; ++i)
    {
      j = (j + state->smap[i] + key[i % keylength]) & 255;
      tmp = state->smap[i];
      state->smap[i] = state->smap[j];
      state->smap[j] = tmp;
    }
  }
  state->i = 0;
  state->j = 0;
  state->flush = 0; /* caller is responsible to override if necessary */
  return state;
}

void rc4_map_save (rc4_state *state, rc4_map *map)
{
  memcpy(map, state->map, sizeof(rc4_map));
}

void rc4_map_restore (rc4_state *state, rc4_map *map)
{
  memcpy(state->map, map, sizeof(rc4_map));
  //state->flags = 0;
  //state->flush = 0;
  state->i = 0;
  state->j = 0;
}

static uint8_t rc4_next_random_byte (rc4_state *state)
{
  uint8_t tmp;
  state->i = (state->i + 1) & 255;
  state->j = (state->j + state->smap[state->i]) & 255;
  tmp = state->smap[state->i];
  state->smap[state->i] = state->smap[state->j];
  state->smap[state->j] = tmp;
  return state->smap[(state->smap[state->i] + state->smap[state->j]) & 255];
}

iof_status rc4_crypt_state (iof *I, iof *O, rc4_state *state)
{
  uint8_t r;
  int c;
  while (iof_ensure(O, 1))
  {
    if ((c = iof_get(I)) < 0)
      return c == IOFERR ? IOFERR : (state->flush ? IOFEOF : IOFEMPTY);
    r = rc4_next_random_byte(state);
    //r = r ^ ((uint8_t)c);
    //iof_set(O, r);
    iof_set(O, r ^ ((uint8_t)c));
  }
  return IOFFULL;
}

iof_status rc4_crypt (iof *I, iof *O, const void *key, size_t keylength)
{
  int ret;
  rc4_state state;
  rc4_map map;
  if (rc4_state_initialize(&state, &map, key, keylength) == NULL)
    return IOFERR;
  state.flush = 1;
  ret = rc4_crypt_state(I, O, &state);
  rc4_state_close(&state);
  return ret;
}

/*
Variants that operates on c-strings can worn inplace, so output and input can be the same address.
Variant that takes rc4_state pointer expects the state properly initialized. Keep in mind
the crypt procedure modifies rc4 bytes map. All returns the size of encrypted/decrypted
data, which is the same as input data length for rc4.
*/

size_t rc4_crypt_data (const void *input, size_t length, void *output, const void *key, size_t keylength)
{
  rc4_state state;
  rc4_map map;
  if (rc4_state_initialize(&state, &map, key, keylength) == NULL)
    return 0;
  return rc4_crypt_state_data(&state, input, length, output);
  // no need to call rc4_state_close()
}

size_t rc4_crypt_state_data (rc4_state *state, const void *input, size_t length, void *output)
{ /* state assumed to be initialized and with the proper state of smap */
  const uint8_t *inp;
  uint8_t r, *out;
  size_t size;
  inp = (const uint8_t *)input;
  out = (uint8_t *)output;
  for (size = 0; size < length; ++size, ++inp, ++out)
  {
    r = rc4_next_random_byte(state);
    *out = r ^ *inp;
  }
  return length;
}

void rc4_state_close (rc4_state *state)
{
  if (state->smap != NULL && (state->flags & RC4_STATE_ALLOC))
  {
    util_free(state->smap);
    state->smap = NULL;
  }
}

/* aes; parts of code excerpted from https://github.com/kokke/tiny-AES128-C */

static const uint8_t sbox[256] =   {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const uint8_t rsbox[256] =
{ 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

/*
The round constant word array, rcon[i], contains the values given by
x to th e power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
Note that i starts at 1, not 0).
*/

static const uint8_t rcon[255] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
  0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
  0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
  0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
  0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
  0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
  0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
  0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
  0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
  0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
  0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
  0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
  0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
  0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
  0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
  0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb  };

/* block copying */

#define aes_copy_block(output, input) memcpy(output, input, 16)

static void aes_copy_cbc (uint8_t *data, const uint8_t *input)
{
  uint8_t i;
  for (i = 0; i < 16; ++i)
    data[i] ^= input[i];
}

static void aes_copy_xor (uint8_t *data, const uint8_t *input, const uint8_t *iv)
{
  uint8_t i;
  for (i = 0; i < 16; ++i)
    data[i] = input[i] ^ iv[i];
}

/* key expansion */

#define AES_COLUMNS 4 // constant in aes

static void key_expansion (aes_state *state, const uint8_t *key)
{
  uint32_t i, j, k;
  uint8_t t[4];
  uint8_t *keydata, keywords, columns;

  keywords = (uint8_t)(state->keylength >> 2);
  keydata = (uint8_t *)state->keyblock;

  /* the first round key is the key itself */
  for(i = 0; i < keywords; ++i)
  {
    keydata[(i * 4) + 0] = key[(i * 4) + 0];
    keydata[(i * 4) + 1] = key[(i * 4) + 1];
    keydata[(i * 4) + 2] = key[(i * 4) + 2];
    keydata[(i * 4) + 3] = key[(i * 4) + 3];
  }

  /* others derived from the first */

  for(columns = AES_COLUMNS * (state->rounds + 1); i < columns; ++i)
  {
    for(j = 0; j < 4; ++j)
      t[j] = keydata[(i - 1) * 4 + j];
    if (i % keywords == 0)
    {
      /* rotate the 4 bytes in a word to the left once; [a0,a1,a2,a3] becomes [a1,a2,a3,a0] */
      k = t[0];
      t[0] = t[1];
      t[1] = t[2];
      t[2] = t[3];
      t[3] = k;

      /* take a four-byte input word and apply the S-box to each of the four bytes to produce an output word */
      t[0] = sbox[t[0]];
      t[1] = sbox[t[1]];
      t[2] = sbox[t[2]];
      t[3] = sbox[t[3]];

      t[0] =  t[0] ^ rcon[i / keywords];
    }
    else if (keywords > 6 && i % keywords == 4)
    {
      t[0] = sbox[t[0]];
      t[1] = sbox[t[1]];
      t[2] = sbox[t[2]];
      t[3] = sbox[t[3]];
    }
    keydata[i * 4 + 0] = keydata[(i - keywords) * 4 + 0] ^ t[0];
    keydata[i * 4 + 1] = keydata[(i - keywords) * 4 + 1] ^ t[1];
    keydata[i * 4 + 2] = keydata[(i - keywords) * 4 + 2] ^ t[2];
    keydata[i * 4 + 3] = keydata[(i - keywords) * 4 + 3] ^ t[3];
  }

}

/*
An original implementation uses no private buffers except a keyblock. We need private buffers to
keep a CBC vector between calls and to be able to read input data not necessarily in 16-bytes blocks.
Encrypter would actually require only one such buffer, as CBC vector is applied on input data before
the actual cipher procedure. And CBC for the next chunk is simply the output from the previous.
Decrypter, however, applies the cipher first, then applies CBC to the output with a buffered init
vector, and the vector for the next call is the row input before cipher. Hence we need two 16-bytes
buffers for decrypter.
*/

/*
aes_state * aes_state_initialize_ecb (aes_state *State, uint8_t *keyblock, const uint8_t *key)
{
  state->flags = 0;

  state->flags |= AES_ECB_MODE;

  if (keyblock == NULL)
  {
    keyblock = util_malloc(sizeof(aes_keyblock));
    state->flags |= AES_STATE_ALLOC;
  }
  state->keyblock = keyblock;
  key_expansion(state, key);
  state->flush = 0;
  return state;
}
*/

void aes_pdf_mode (aes_state *state)
{
  state->flags |= AES_INLINE_IV;
  state->flags &= ~AES_NULL_PADDING;
}

/*
Initialize arguments:
- state - crypt state
- keyblock - a space for aes key expansion; can be left NULL in which case will be allocated
- key - crypt key; can be left NULL iff keyblock is given and properly initialized
- keylength - the length of the key (16 or 32 bytes)
- iv - 16-bytes CBC initialization vector;
  - if left NULL for encoder, one is generated and stored as state->iv
  - can also be left NULL for decorer, but then AES_INLINE_IV must be set, as this informs decoder to take
    an initialization vector from the beginning of the encrypted stream

At the first approach, an initialization vector was copied to state block during initialization and encoders
assumed that the state block is the current initialization vector. This simplifies encrypting procedure,
as the output from every 16-bytes chunk encryption is an initialization vector for the next chunk. However,
it makes api usage cumbersome, as the user has to know that iv may need to be copied to state block
before each call.
*/

static int aes_key_length (aes_state *state, size_t keylength)
{
  state->keylength = keylength;
  switch (keylength)
  {
    case 16:
      state->rounds = 10;
      break;
    case 24:
      state->rounds = 12;
      break;
    case 32:
      state->rounds = 14;
      break;
    default:
      return 0;
  }
  return 1;
}

aes_state * aes_encode_initialize (aes_state *state, aes_keyblock *keyblock, const void *key, size_t keylength, const void *iv)
{
  state->flags = 0;
  if (!aes_key_length(state, keylength))
    return NULL;
  if (iv != NULL)
    aes_copy_block(state->iv, iv);
  else
    aes_generate_iv(state->iv);
  state->flags |= AES_HAS_IV;

  if (keyblock == NULL)
  {
    keyblock = (aes_keyblock *)util_malloc(sizeof(aes_keyblock));
    state->flags |= AES_STATE_ALLOC;
  }
  state->keyblock = keyblock;
  if (key != NULL) /* if NULL we assume keyblock is given and already expanded */
    key_expansion(state, (const uint8_t *)key);
  state->flush = 0;
  return state;
}

aes_state * aes_decode_initialize (aes_state *state, aes_keyblock *keyblock, const void *key, size_t keylength, const void *iv)
{
  state->flags = 0;
  if (!aes_key_length(state, keylength))
    return NULL;
  if (iv != NULL)
  {
    aes_copy_block(state->iv, iv);
    state->flags |= AES_HAS_IV;
  }
  /* else if AES_INLINE_IV flag is set will be read from input */

  if (keyblock == NULL)
  {
    keyblock = (aes_keyblock *)util_malloc(sizeof(aes_keyblock));
    state->flags |= AES_STATE_ALLOC;
  }
  state->keyblock = keyblock;
  if (key != NULL) /* otherwise keyblock is assumed present and properly initialized */
    key_expansion(state, (const uint8_t *)key);
  state->flush = 0;
  return state;
}

void aes_state_close (aes_state *state)
{
  if (state->keyblock != NULL && (state->flags & AES_STATE_ALLOC))
    util_free(state->keyblock);
}

/* add round key */

static void aes_round_key (aes_block block, aes_block keyblock)
{
  uint8_t i, j;
  for(i = 0; i < 4; ++i)
    for(j = 0; j < 4; ++j)
      block[i][j] ^= keyblock[i][j];
}

#define aes_add_key(block, keyblock, round) aes_round_key(block, (*keyblock)[round])

/* substitution */

static void aes_encode_sub (aes_block block)
{
  uint8_t i, j, v;
  for(i = 0; i < 4; ++i)
    for(j = 0; j < 4; ++j)
      v = block[i][j], block[i][j] = sbox[v];
}

/* rows shift; the row index is the shift offset, the first order is not shifted */

static void aes_encode_shift (aes_block block)
{
  uint8_t tmp;

  /* 1st row rotated once */
  tmp = block[0][1];
  block[0][1] = block[1][1];
  block[1][1] = block[2][1];
  block[2][1] = block[3][1];
  block[3][1] = tmp;

  /* 2nd row rotated twice */
  tmp = block[0][2];
  block[0][2] = block[2][2];
  block[2][2] = tmp;
  tmp = block[1][2];
  block[1][2] = block[3][2];
  block[3][2] = tmp;

  /* 3rd row rotated 3 times */
  tmp = block[0][3];
  block[0][3] = block[3][3];
  block[3][3] = block[2][3];
  block[2][3] = block[1][3];
  block[1][3] = tmp;
}

static uint8_t xtime (uint8_t x)
{
  return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

/* mix columns */

static void aes_encode_mix (aes_block block)
{
  uint8_t i, tmp, tm, t;

  for(i = 0; i < 4; ++i)
  {
    t   = block[i][0];
    tmp = block[i][0] ^ block[i][1] ^ block[i][2] ^ block[i][3] ;
    tm  = block[i][0] ^ block[i][1]; tm = xtime(tm); block[i][0] ^= tm ^ tmp;
    tm  = block[i][1] ^ block[i][2]; tm = xtime(tm); block[i][1] ^= tm ^ tmp;
    tm  = block[i][2] ^ block[i][3]; tm = xtime(tm); block[i][2] ^= tm ^ tmp;
    tm  = block[i][3] ^ t ;          tm = xtime(tm); block[i][3] ^= tm ^ tmp;
  }
}

/* multiply is used to multiply numbers in the field GF(2^8) */

#define multiply(x, y)                                \
      (  ((y & 1) * x) ^                              \
      ((y>>1 & 1) * xtime(x)) ^                       \
      ((y>>2 & 1) * xtime(xtime(x))) ^                \
      ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^         \
      ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))   \

/* mix columns */

static void aes_decode_mix (aes_block block)
{
  int i;
  uint8_t a, b, c, d;

  for(i = 0; i < 4; ++i)
  {
    a = block[i][0];
    b = block[i][1];
    c = block[i][2];
    d = block[i][3];
    block[i][0] = multiply(a, 0x0e) ^ multiply(b, 0x0b) ^ multiply(c, 0x0d) ^ multiply(d, 0x09);
    block[i][1] = multiply(a, 0x09) ^ multiply(b, 0x0e) ^ multiply(c, 0x0b) ^ multiply(d, 0x0d);
    block[i][2] = multiply(a, 0x0d) ^ multiply(b, 0x09) ^ multiply(c, 0x0e) ^ multiply(d, 0x0b);
    block[i][3] = multiply(a, 0x0b) ^ multiply(b, 0x0d) ^ multiply(c, 0x09) ^ multiply(d, 0x0e);
  }
}

/* inverse substitution */

static void aes_decode_sub (aes_block block)
{
  uint8_t i, j, v;
  for(i = 0; i < 4; ++i)
    for(j = 0; j < 4; ++j)
      v = block[i][j], block[i][j] = rsbox[v];
}

/* inverse shift rows */

static void aes_decode_shift (aes_block block)
{
  uint8_t tmp;

  /* 1st row rotated once right */
  tmp = block[3][1];
  block[3][1] = block[2][1];
  block[2][1] = block[1][1];
  block[1][1] = block[0][1];
  block[0][1] = tmp;

  /* 2st row rotated twice right */
  tmp = block[0][2];
  block[0][2] = block[2][2];
  block[2][2] = tmp;
  tmp = block[1][2];
  block[1][2] = block[3][2];
  block[3][2] = tmp;

  /* 3rd row rotated 3 times right */
  tmp = block[0][3];
  block[0][3] = block[1][3];
  block[1][3] = block[2][3];
  block[2][3] = block[3][3];
  block[3][3] = tmp;
}

/* aes block encoder */

static void aes_encode_cipher (aes_state *state)
{
  uint8_t round;
  aes_add_key(state->block, state->keyblock, 0);
  for (round = 1; round < state->rounds; ++round)
  {
    aes_encode_sub(state->block);
    aes_encode_shift(state->block);
    aes_encode_mix(state->block);
    aes_add_key(state->block, state->keyblock, round);
  }
  aes_encode_sub(state->block);
  aes_encode_shift(state->block);
  aes_add_key(state->block, state->keyblock, state->rounds);
}

/* aes block decoder */

static void aes_decode_cipher (aes_state *state)
{
  uint8_t round;
  aes_add_key(state->block, state->keyblock, state->rounds);
  for(round = state->rounds - 1; round > 0; --round)
  {
    aes_decode_shift(state->block);
    aes_decode_sub(state->block);
    aes_add_key(state->block, state->keyblock, round);
    aes_decode_mix(state->block);
  }
  aes_decode_shift(state->block);
  aes_decode_sub(state->block);
  aes_add_key(state->block, state->keyblock, 0);
}

/* tail block padding; RFC 2898, PKCS #5: Password-Based Cryptography Specification Version 2.0; pdf spec p. 119 */

#define aes_padding(state) ((state->flags & AES_NULL_PADDING) == 0)

static void aes_put_padding (aes_state *state, uint8_t length)
{
  uint8_t pad;
  pad = (aes_padding(state)) ? 16 - length : 0;
  for (; length < 16; ++length)
    state->data[length] = state->iv[length] ^ pad;
}

static int aes_remove_padding (aes_state *state, uint8_t *data, uint8_t *length)
{
  uint8_t pad;
  *length = 16; /* block length 16 means leave intact */
  if (aes_padding(state))
  {
    pad = data[16 - 1];
    if (pad > 16)
      return IOFERR;
    for ( ; *length > 16 - pad; --(*length))
      if (data[*length - 1] != pad)
        return IOFERR;
  }
  else
  {
    for ( ; *length > 0; --(*length))
      if (data[*length - 1] != '\0')
        break;
  }
  return IOFEOF;
}

/* aes codec */

/* make the cipher on input xor-ed with iv, save the output as a new iv, write the output */
#define aes_encode_output(state, output) \
  (aes_encode_cipher(state), aes_copy_block(state->iv, state->data), aes_copy_block(output, state->data), output += 16)

iof_status aes_encode_state (iof *I, iof *O, aes_state *state)
{
  int c;

  if (!(state->flags & AES_HAS_IV)) // weird
    return IOFERR;
  if ((state->flags & AES_INLINE_IV) && !(state->flags & AES_CONTINUE))
  { /* write iv at the beginning of encrypted data */
    if (!iof_ensure(O, 16))
      return IOFFULL;
    aes_copy_block(O->pos, state->iv);
    O->pos += 16;
    state->flags |= AES_CONTINUE;
  }
  while (iof_ensure(O, 16))
  {
    while (state->buffered < 16)
    {
      if ((c = iof_get(I)) != IOFEOF)
      { /* get input byte XORed with iv */
        state->data[state->buffered] = state->iv[state->buffered] ^ ((uint8_t)c);
        ++state->buffered;
      }
      else
      {
        if (state->flush)
        {
          if (state->buffered > 0 || aes_padding(state))
          { /* pad the last input chunk; for input divisable by 16, add 16 bytes 0x0f */
            aes_put_padding(state, state->buffered);
            state->buffered = 16;
            aes_encode_output(state, O->pos);
          }
          return IOFEOF;
        }
        else
          return IOFEMPTY;
      }
    }
    aes_encode_output(state, O->pos);
    state->buffered = 0;
  }
  return IOFFULL;
}

/* write iv to the output, save the raw input just buffered as iv for the next chunk, make the cipher, write out xoring with iv */
#define aes_decode_output(state, output) \
  (aes_copy_block(output, state->iv), aes_copy_block(state->iv, state->data), aes_decode_cipher(state), aes_copy_cbc(output, state->data), output += 16)

iof_status aes_decode_state (iof *I, iof *O, aes_state *state)
{
  int c, ret;
  uint8_t lastlength;

  if ((state->flags & AES_INLINE_IV) && !(state->flags & AES_CONTINUE))
  {
    while (state->buffered < 16)
    {
      if ((c = iof_get(I)) != IOFEOF)
        state->iv[state->buffered++] = (uint8_t)c;
      else
        return state->flush ? IOFERR : IOFEMPTY;
    }
    state->flags |= AES_CONTINUE|AES_HAS_IV;
    state->buffered = 0;
  }
  while (iof_ensure(O, 16))
  {
    while (state->buffered < 16)
    {
      if ((c = iof_get(I)) != IOFEOF)
        state->data[state->buffered++] = (uint8_t)c;
      else
        return state->flush ? IOFERR : IOFEMPTY;
    }
    aes_decode_output(state, O->pos);
    if (state->flush)
    { /* we have to check for EOF here, to remove eventual padding */
      if ((c = iof_get(I)) < 0)
      { /* end of input at 16-bytes boundary; remove padding and quit */
        ret = aes_remove_padding(state, O->pos - 16, &lastlength);
        O->pos -= 16 - lastlength;
        return ret;
      }
      else
      { /* beginning of the next block */
        state->buffered = 1;
        state->data[0] = (uint8_t)c;
      }
    }
    else
      state->buffered = 0;
  }
  return IOFFULL;
}

/* variants that works on c-strings; can work inplace (output==input) except encoder in pdf flavour */

/*
Codecs operating on c-string can generally work inplace (output==input), except encoder with AES_INLINE_IV flag set,
which outputs 16 bytes of initialization vector at the beginning of encrypted data. All return the size of encrypted/decrypted
data. Encoders output is the original length padded to a complete 16 bytes (plus eventual 16 bytes of initialization
vector, if AES_INLINE_IV is used). Default padding is unambiguously removed during decryption. AES_NULL_PADDING flag
forces using (ambiguous) NULL-byte padding, only if input length module 16 is greater then zero.

An input data is supposed to be a complete data to be encrypted or decrypted. It is possible, however, to use those
codecs for scaterred data chunks by manipulating AES_INLINE_IV, AES_NULL_PADDING, AES_CONTINUE flags and data length.
Caller may assume that c-string codecs do not modify state flags.

Encoder could actually be optimized by writing an initialization vector to a state block once. After every chunk encryption,
the output is the initialization vector for the next chunk. Since we use c-string codec variants on short strings,
the gain is neglectable in comparison with the weight of the aes crypt procedure.
*/

size_t aes_encode_data (const void *input, size_t length, void *output, const void *key, size_t keylength, const void *iv, int flags)
{
  aes_state state;
  aes_keyblock keyblock;

  if (aes_encode_initialize(&state, &keyblock, key, keylength, iv) == NULL)
    return 0;
  state.flags |= flags;
  return aes_encode_state_data(&state, input, length, output);
  // aes_state_close(&state);
}

size_t aes_encode_state_data (aes_state *state, const void *input, size_t length, void *output)
{
  const uint8_t *inp;
  uint8_t *out, tail, t;
  size_t size;

  inp = (const uint8_t *)input;
  out = (uint8_t *)output;

  if (!(state->flags & AES_HAS_IV))
      return 0;
  if ((state->flags & AES_INLINE_IV) && !(state->flags & AES_CONTINUE))
  {
    aes_copy_block(out, state->iv);
    out += 16;
  }
  // state->flags |= AES_CONTINUE; // do not modify state flags

  for (size = 0; size + 16 <= length; size += 16)
  {
    aes_copy_xor(state->data, inp, state->iv);
    aes_encode_output(state, out);
    inp += 16;
  }

  if ((tail = (length % 16)) > 0 || aes_padding(state))
  {
    for (t = 0; t < tail; ++t)
      state->data[t] = inp[t] ^ state->iv[t];
    aes_put_padding(state, tail);
    aes_encode_output(state, out);
    size += 16;
  }
  if (state->flags & AES_INLINE_IV)
    size += 16; /* iv written at the beginning of encoded data */

  return size;
}

size_t aes_decode_data (const void *input, size_t length, void *output, const void *key, size_t keylength, const void *iv, int flags)
{
  aes_state state;
  aes_keyblock keyblock;

  if (aes_decode_initialize(&state, &keyblock, key, keylength, iv) == NULL)
    return 0;
  state.flags |= flags;
  return aes_decode_state_data(&state, input, length, output);
  // aes_state_close(&state);
}

size_t aes_decode_state_data (aes_state *state, const void *input, size_t length, void *output)
{
  const uint8_t *inp;
  uint8_t *out, lastlength;
  size_t size;

  inp = (const uint8_t *)input;
  out = (uint8_t *)output;

  if ((state->flags & AES_INLINE_IV) && !(state->flags & AES_CONTINUE))
  {
    aes_copy_block(state->iv, inp);
    // state->flags |= AES_HAS_IV; // do not modify state flags
    inp += 16;
    length = length >= 16 ? length - 16 : 0;
  }
  else if (!(state->flags & AES_HAS_IV))
    return 0;
  // state->flags |= AES_CONTINUE; // do not modify state flags
  for (size = 0; size + 16 <= length; size += 16)
  {
    aes_copy_block(state->data, inp);
    aes_decode_output(state, out);
    inp += 16;
  }

  if (size >= 16)
  {
    aes_remove_padding(state, out - 16, &lastlength);
    size = size - 16 + lastlength;
  }

  return size;
}

/*
pseudo-random bytes chain exceprted from eexec; not expected to have strong cryptographic properties
we only expect that it is (reasonably) unique and different for each call (not only function call, but also
a program call). A current trick with mangling pointer value gives satisfactory results, generally different
for every function call and a programm call. Note that the pseudo-input bytes starts from some inner address
bits, as they vary better; without that, the first byte tends to be "lazy".
*/

void random_bytes (uint8_t *output, size_t size)
{
  size_t i;
  uint8_t p;
  static uint16_t k = 55665;
  for (i = 0; i < size; ++i)
  {
    p = ((uint8_t *)(&output))[(i + 2) % sizeof(uint8_t *)] ^ (uint8_t)size; // pseudo input byte ;)
    k = (((p + k) * 52845 + 22719) & 65535); // xor-ed with pseudo-random sequence (kept between calls)
    output[i] = p ^ (k >> 8);
  }
}

void aes_generate_iv (uint8_t output[16])
{
  random_bytes(output, 16);
}

/* filters */

// rc4 decoder function

static size_t rc4_decoder (iof *F, iof_mode mode)
{
  rc4_state *state;
  iof_status status;
  size_t tail;

  state = iof_filter_state(rc4_state *, F);
  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      do {
        status = rc4_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "rc4", status);
    case IOFCLOSE:
      rc4_state_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// rc4 encoder function

static size_t rc4_encoder (iof *F, iof_mode mode)
{
  rc4_state *state;
  iof_status status;

  state = iof_filter_state(rc4_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = rc4_encode_state(F, F->next, state);
      return iof_encoder_retval(F, "rc4", status);
    case IOFCLOSE:
      if (!state->flush)
        rc4_encoder(F, IOFFLUSH);
      rc4_state_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// aes decoder function

static size_t aes_decoder (iof *F, iof_mode mode)
{
  aes_state *state;
  iof_status status;
  size_t tail;

  state = iof_filter_state(aes_state *, F);
  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      do {
        status = aes_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "aes", status);
    case IOFCLOSE:
      aes_state_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// aes encoder function

static size_t aes_encoder (iof *F, iof_mode mode)
{
  aes_state *state;
  iof_status status;

  state = iof_filter_state(aes_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = aes_encode_state(F, F->next, state);
      return iof_encoder_retval(F, "aes", status);
    case IOFCLOSE:
      if (!state->flush)
        aes_encoder(F, IOFFLUSH);
      aes_state_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

iof * iof_filter_rc4_decoder (iof *N, const void *key, size_t keylength)
{
  iof *I;
  rc4_state *state;

  I = iof_filter_reader(rc4_decoder, sizeof(rc4_state), &state);
  iof_setup_next(I, N);
  if (rc4_state_init(state, key, keylength) == NULL)
  {
    iof_discard(I);
    return NULL;
  }
  state->flush = 1;
  return I;
}

iof * iof_filter_rc4_encoder (iof *N, const void *key, size_t keylength)
{
  iof *O;
  rc4_state *state;

  O = iof_filter_writer(rc4_encoder, sizeof(rc4_state), &state);
  iof_setup_next(O, N);
  if (rc4_state_init(state, key, keylength) == NULL)
  {
    iof_discard(O);
    return NULL;
  }
  // state->flush = 1;
  return O;
}

/* aes crypt filters */

iof * iof_filter_aes_decoder (iof *N, const void *key, size_t keylength)
{
  iof *I;
  aes_state *state;

  I = iof_filter_reader(aes_decoder, sizeof(aes_state), &state);
  iof_setup_next(I, N);
  if (aes_decode_init(state, key, keylength) == NULL)
  {
    iof_discard(I);
    return NULL;
  }
  aes_pdf_mode(state);
  state->flush = 1;
  return I;
}

iof * iof_filter_aes_encoder (iof *N, const void *key, size_t keylength)
{
  iof *O;
  aes_state *state;

  O = iof_filter_writer(aes_encoder, sizeof(aes_state), &state);
  iof_setup_next(O, N);
  if (aes_encode_init(state, key, keylength) == NULL)
  {
    iof_discard(O);
    return NULL;
  }
  aes_pdf_mode(state);
  // state->flush = 1;
  return O;
}

/* test */

/*
static void show (void *p, size_t size, uint8_t round, uint8_t sym)
{
  uint8_t i;
  printf("%c%c:", round, sym);
  for (i = 0; i < size; ++i)
    printf("%02x", ((uint8_t *)p)[i]);
  printf("\n");
}

void aes_test (void)
{
  const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  const uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
  const uint8_t inp[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
  const uint8_t out[]  = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
    0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
    0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7 };

  uint8_t input[64], output[64];
  size_t inpsize, outsize;
  int flags = AES_NULL_PADDING;

	////////////////////////////////////////////////////////////////////////////

//#define ENCODETO output
#define ENCODETO input // inplace

  inpsize = 64;
  memcpy(input, inp, inpsize);
	show(input, inpsize, '>', '>');
  outsize = aes_encode_data(input, inpsize, ENCODETO, key, 16, iv, flags);
  show(ENCODETO, outsize, '<', '<');
  if (outsize == inpsize && memcmp(ENCODETO, out, outsize) == 0)
    printf("ENCODER SUCCESS\n");
  else
    printf("ENCODER FAILURE\n");

	////////////////////////////////////////////////////////////////////////////

//#define DECODETO input
#define DECODETO output // in place

  outsize = 64;
  memcpy(output, out, outsize);
  show(output, outsize, '<', '<');
	inpsize = aes_decode_data(output, outsize, DECODETO, key, 16, iv, flags);
  show(DECODETO, inpsize, '>', '>');
  if (inpsize == outsize && memcmp(DECODETO, inp, inpsize) == 0)
    printf("DECODER SUCCESS\n");
  else
    printf("DECODER FAILURE\n");
}
*/

/*
Some example vectors

================================ AES ECB 128-bit encryption mode ================================

Encryption key: 2b7e151628aed2a6abf7158809cf4f3c

Test vector                      Cipher text
6bc1bee22e409f96e93d7e117393172a 3ad77bb40d7a3660a89ecaf32466ef97
ae2d8a571e03ac9c9eb76fac45af8e51 f5d3d58503b9699de785895a96fdbaaf
30c81c46a35ce411e5fbc1191a0a52ef 43b1cd7f598ece23881b00e3ed030688
f69f2445df4f9b17ad2b417be66c3710 7b0c785e27e8ad3f8223207104725dd4


================================ AES ECB 192-bit encryption mode ================================

Encryption key: 8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b

Test vector                       Cipher text
6bc1bee22e409f96e93d7e117393172a  bd334f1d6e45f25ff712a214571fa5cc
ae2d8a571e03ac9c9eb76fac45af8e51  974104846d0ad3ad7734ecb3ecee4eef
30c81c46a35ce411e5fbc1191a0a52ef  ef7afd2270e2e60adce0ba2face6444e
f69f2445df4f9b17ad2b417be66c3710  9a4b41ba738d6c72fb16691603c18e0e


================================ AES ECB 256-bit encryption mode ================================

Encryption key: 603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4

Test vector                       Cipher text
6bc1bee22e409f96e93d7e117393172a  f3eed1bdb5d2a03c064b5a7e3db181f8
ae2d8a571e03ac9c9eb76fac45af8e51  591ccb10d410ed26dc5ba74a31362870
30c81c46a35ce411e5fbc1191a0a52ef  b6ed21b99ca6f4f9f153e7b1beafed1d
f69f2445df4f9b17ad2b417be66c3710  23304b7a39f9f3ff067d8d8f9e24ecc7

================================ AES CBC 128-bit encryption mode ================================

Encryption key: 2b7e151628aed2a6abf7158809cf4f3c

Initialization vector             Test vector                       Cipher text
000102030405060708090A0B0C0D0E0F  6bc1bee22e409f96e93d7e117393172a  7649abac8119b246cee98e9b12e9197d
7649ABAC8119B246CEE98E9B12E9197D  ae2d8a571e03ac9c9eb76fac45af8e51  5086cb9b507219ee95db113a917678b2
5086CB9B507219EE95DB113A917678B2  30c81c46a35ce411e5fbc1191a0a52ef  73bed6b8e3c1743b7116e69e22229516
73BED6B8E3C1743B7116E69E22229516  f69f2445df4f9b17ad2b417be66c3710  3ff1caa1681fac09120eca307586e1a7

================================ AES CBC 192-bit encryption mode ================================

Encryption key: 8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b

Initialization vector             Test vector                       Cipher text
000102030405060708090A0B0C0D0E0F  6bc1bee22e409f96e93d7e117393172a  4f021db243bc633d7178183a9fa071e8
4F021DB243BC633D7178183A9FA071E8  ae2d8a571e03ac9c9eb76fac45af8e51  b4d9ada9ad7dedf4e5e738763f69145a
B4D9ADA9AD7DEDF4E5E738763F69145A  30c81c46a35ce411e5fbc1191a0a52ef  571b242012fb7ae07fa9baac3df102e0
571B242012FB7AE07FA9BAAC3DF102E0  f69f2445df4f9b17ad2b417be66c3710  08b0e27988598881d920a9e64f5615cd

================================ AES CBC 256-bit encryption mode ================================

Encryption key: 603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4

Initialization vector             Test vector                       Cipher text
000102030405060708090A0B0C0D0E0F  6bc1bee22e409f96e93d7e117393172a  f58c4c04d6e5f1ba779eabfb5f7bfbd6
F58C4C04D6E5F1BA779EABFB5F7BFBD6  ae2d8a571e03ac9c9eb76fac45af8e51  9cfc4e967edb808d679f777bc6702c7d
9CFC4E967EDB808D679F777BC6702C7D  30c81c46a35ce411e5fbc1191a0a52ef  39f23369a9d9bacfa530e26304231461
39F23369A9D9BACFA530E26304231461  f69f2445df4f9b17ad2b417be66c3710  b2eb05e2c39be9fcda6c19078c6a9d1b
*/