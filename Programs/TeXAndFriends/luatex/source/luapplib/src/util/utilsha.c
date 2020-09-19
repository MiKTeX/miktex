/* sha2 implementation excerpted from code by Aaron D. Gifford */

/*
 * AUTHOR: Aaron D. Gifford - http://www.aarongifford.com/
 *
 * Copyright (c) 2000-2001, Aaron D. Gifford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.c,v 1.1 2001/11/08 00:01:51 adg Exp adg $
 */

#include <stdio.h>  /* FILE */
#include <string.h> /* memcpy()/memset() or bcopy()/bzero() */
//#include <assert.h> /* assert() */
#include "utilsha.h"

/*
 * UNROLLED TRANSFORM LOOP NOTE:
 * You can define SHA2_UNROLL_TRANSFORM to use the unrolled transform
 * loop version for the hash transform rounds (defined using macros
 * later in this file).  Either define on the command line, for example:
 *
 *   cc -DSHA2_UNROLL_TRANSFORM -o sha2 sha2.c sha2prog.c
 *
 * or define below:
 *
 *   #define SHA2_UNROLL_TRANSFORM
 *
 */

/*** SHA-256/384/512 Machine Architecture Definitions *****************/
/*
 * BYTE_ORDER NOTE:
 *
 * Please make sure that your system defines BYTE_ORDER.  If your
 * architecture is little-endian, make sure it also defines
 * LITTLE_ENDIAN and that the two (BYTE_ORDER and LITTLE_ENDIAN) are
 * equivilent.
 *
 * If your system does not define the above, then you can do so by
 * hand like this:
 *
 *   #define LITTLE_ENDIAN 1234
 *   #define BIG_ENDIAN    4321
 *
 * And for little-endian machines, add:
 *
 *   #define BYTE_ORDER LITTLE_ENDIAN
 *
 * Or for big-endian machines:
 *
 *   #define BYTE_ORDER BIG_ENDIAN
 *
 * The FreeBSD machine this was written on defines BYTE_ORDER
 * appropriately by including <sys/types.h> (which in turn includes
 * <machine/endian.h> where the appropriate definitions are actually
 * made).
 */

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

//#if !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
//#error Define BYTE_ORDER to be equal to either LITTLE_ENDIAN or BIG_ENDIAN
//#endif

/*
 * Define the following sha2_* types to types of the correct length on
 * the native archtecture.   Most BSD systems and Linux define u_intXX_t
 * types.  Machines with very recent ANSI C headers, can use the
 * uintXX_t definintions from inttypes.h by defining SHA2_USE_INTTYPES_H
 * during compile or in the sha.h header file.
 *
 * Machines that support neither u_intXX_t nor inttypes.h's uintXX_t
 * will need to define these three typedefs below (and the appropriate
 * ones in sha.h too) by hand according to their system architecture.
 *
 * Thank you, Jun-ichiro itojun Hagino, for suggesting using u_intXX_t
 * types and pointing out recent ANSI C support for uintXX_t in inttypes.h.
 *
 * PJ: replace by uintX_t
 */

//typedef uint8_t  sha2_byte; /* Exactly 1 byte */
//typedef uint32_t sha2_word32; /* Exactly 4 bytes */
//typedef uint64_t sha2_word64; /* Exactly 8 bytes */

/*** SHA-256/384/512 Various Length Definitions ***********************/
/* NOTE: Most of these are in header */
#define SHA256_SHORT_BLOCK_LENGTH (SHA256_BLOCK_LENGTH - 8)
#define SHA384_SHORT_BLOCK_LENGTH (SHA384_BLOCK_LENGTH - 16)
#define SHA512_SHORT_BLOCK_LENGTH (SHA512_BLOCK_LENGTH - 16)


/*** ENDIAN REVERSAL MACROS *******************************************/
#if BYTE_ORDER == LITTLE_ENDIAN
#define REVERSE32(w, x) { \
  uint32_t tmp = (w); \
  tmp = (tmp >> 16) | (tmp << 16); \
  (x) = ((tmp & 0xff00ff00UL) >> 8) | ((tmp & 0x00ff00ffUL) << 8); \
}
#define REVERSE64(w, x) { \
  uint64_t tmp = (w); \
  tmp = (tmp >> 32) | (tmp << 32); \
  tmp = ((tmp & 0xff00ff00ff00ff00ULL) >> 8) | \
        ((tmp & 0x00ff00ff00ff00ffULL) << 8); \
  (x) = ((tmp & 0xffff0000ffff0000ULL) >> 16) | \
        ((tmp & 0x0000ffff0000ffffULL) << 16); \
}
#endif /* BYTE_ORDER == LITTLE_ENDIAN */

/*
 * Macro for incrementally adding the unsigned 64-bit integer n to the
 * unsigned 128-bit integer (represented using a two-element array of
 * 64-bit words):
 */
#define ADDINC128(w,n) { \
  (w)[0] += (uint64_t)(n); \
  if ((w)[0] < (n)) { \
    (w)[1]++; \
  } \
}

#define MEMSET_BZERO(p,l) memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l) memcpy((d), (s), (l))

/*** THE SIX LOGICAL FUNCTIONS ****************************************/
/*
 * Bit shifting and rotation (used by the six SHA-XYZ logical functions:
 *
 *   NOTE:  The naming of R and S appears backwards here (R is a SHIFT and
 *   S is a ROTATION) because the SHA-256/384/512 description document
 *   (see http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf) uses this
 *   same "backwards" definition.
 */
/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define R(b,x)   ((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define S32(b,x) (((x) >> (b)) | ((x) << (32 - (b))))
/* 64-bit Rotate-right (used in SHA-384 and SHA-512): */
#define S64(b,x) (((x) >> (b)) | ((x) << (64 - (b))))

/* Two of six logical functions used in SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Four of six logical functions used in SHA-256: */
#define Sigma0_256(x) (S32(2,  (x)) ^ S32(13, (x)) ^ S32(22, (x)))
#define Sigma1_256(x) (S32(6,  (x)) ^ S32(11, (x)) ^ S32(25, (x)))
#define sigma0_256(x) (S32(7,  (x)) ^ S32(18, (x)) ^ R(3 ,   (x)))
#define sigma1_256(x) (S32(17, (x)) ^ S32(19, (x)) ^ R(10,   (x)))

/* Four of six logical functions used in SHA-384 and SHA-512: */
#define Sigma0_512(x) (S64(28, (x)) ^ S64(34, (x)) ^ S64(39, (x)))
#define Sigma1_512(x) (S64(14, (x)) ^ S64(18, (x)) ^ S64(41, (x)))
#define sigma0_512(x) (S64( 1, (x)) ^ S64( 8, (x)) ^ R( 7,   (x)))
#define sigma1_512(x) (S64(19, (x)) ^ S64(61, (x)) ^ R( 6,   (x)))

static void sha512_last (sha512_state *state);
static void sha256_transform (sha256_state *state, const uint32_t idata[16]);
static void sha512_transform (sha512_state *state, const uint64_t idata[16]);

/*** SHA-XYZ INITIAL HASH VALUES AND CONSTANTS ************************/
/* Hash constant words K for SHA-256: */
static const uint32_t K256[64] = {
  0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
  0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
  0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
  0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
  0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
  0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
  0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
  0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
  0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
  0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
  0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
  0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
  0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
  0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
  0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
  0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Initial hash value H for SHA-256: */
static const uint32_t sha256_initial_hash_value[8] = {
  0x6a09e667UL,
  0xbb67ae85UL,
  0x3c6ef372UL,
  0xa54ff53aUL,
  0x510e527fUL,
  0x9b05688cUL,
  0x1f83d9abUL,
  0x5be0cd19UL
};

/* Hash constant words K for SHA-384 and SHA-512: */
static const uint64_t K512[80] = {
  0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
  0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
  0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
  0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
  0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
  0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
  0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
  0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
  0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
  0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
  0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
  0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
  0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
  0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
  0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
  0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
  0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
  0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
  0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
  0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
  0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
  0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
  0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
  0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
  0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
  0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
  0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
  0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
  0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
  0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
  0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
  0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
  0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
  0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
  0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
  0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
  0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
  0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
  0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
  0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

/* Initial hash value H for SHA-384 */
static const uint64_t sha384_initial_hash_value[8] = {
  0xcbbb9d5dc1059ed8ULL,
  0x629a292a367cd507ULL,
  0x9159015a3070dd17ULL,
  0x152fecd8f70e5939ULL,
  0x67332667ffc00b31ULL,
  0x8eb44a8768581511ULL,
  0xdb0c2e0d64f98fa7ULL,
  0x47b5481dbefa4fa4ULL
};

/* Initial hash value H for SHA-512 */
static const uint64_t sha512_initial_hash_value[8] = {
  0x6a09e667f3bcc908ULL,
  0xbb67ae8584caa73bULL,
  0x3c6ef372fe94f82bULL,
  0xa54ff53a5f1d36f1ULL,
  0x510e527fade682d1ULL,
  0x9b05688c2b3e6c1fULL,
  0x1f83d9abfb41bd6bULL,
  0x5be0cd19137e2179ULL
};

/*** SHA-256: *********************************************************/
sha256_state * sha256_digest_init (sha256_state *state) 
{
  MEMCPY_BCOPY(state->words, sha256_initial_hash_value, SHA256_DIGEST_LENGTH);
  MEMSET_BZERO(state->buffer, SHA256_BLOCK_LENGTH);
  state->bitcount = 0;
  return state;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* Unrolled SHA-256 round macros: */

#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND256_0_TO_15(v, a, b, c, d, e, f, g, h) \
  REVERSE32(v, W256[j]); \
  T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + W256[j]; \
  (d) += T1; \
  (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c))

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256_0_TO_15(v, a, b, c, d, e, f, g, h) \
  T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + (W256[j] = v); \
  (d) += T1; \
  (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c))

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND256(a, b, c, d, e, f, g, h) \
  s0 = W256[(j+1)&0x0f]; \
  s0 = sigma0_256(s0); \
  s1 = W256[(j+14)&0x0f]; \
  s1 = sigma1_256(s1); \
  T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0); \
  (d) += T1; \
  (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c))

static void sha256_transform (sha256_state *state, const uint32_t idata[16]) {
  uint32_t a, b, c, d, e, f, g, h, s0, s1;
  uint32_t T1, *W256, v;
  int j;

  W256 = state->buffer32;

  /* Initialize registers with the prev. intermediate value */
  a = state->words[0];
  b = state->words[1];
  c = state->words[2];
  d = state->words[3];
  e = state->words[4];
  f = state->words[5];
  g = state->words[6];
  h = state->words[7];

  j = 0;
  do {
    /* Rounds 0 to 15 (unrolled): */
    v = idata[j]; ROUND256_0_TO_15(v, a, b, c, d, e, f, g, h); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, h, a, b, c, d, e, f, g); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, g, h, a, b, c, d, e, f); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, f, g, h, a, b, c, d, e); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, e, f, g, h, a, b, c, d); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, d, e, f, g, h, a, b, c); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, c, d, e, f, g, h, a, b); ++j;
    v = idata[j]; ROUND256_0_TO_15(v, b, c, d, e, f, g, h, a); ++j;
  } while (j < 16);

  /* Now for the remaining rounds to 64: */
  do {
    ROUND256(a, b, c, d, e, f, g, h); ++j;
    ROUND256(h, a, b, c, d, e, f, g); ++j;
    ROUND256(g, h, a, b, c, d, e, f); ++j;
    ROUND256(f, g, h, a, b, c, d, e); ++j;
    ROUND256(e, f, g, h, a, b, c, d); ++j;
    ROUND256(d, e, f, g, h, a, b, c); ++j;
    ROUND256(c, d, e, f, g, h, a, b); ++j;
    ROUND256(b, c, d, e, f, g, h, a); ++j;
  } while (j < 64);

  /* Compute the current intermediate hash value */
  state->words[0] += a;
  state->words[1] += b;
  state->words[2] += c;
  state->words[3] += d;
  state->words[4] += e;
  state->words[5] += f;
  state->words[6] += g;
  state->words[7] += h;
}

#else /* SHA2_UNROLL_TRANSFORM */

static void sha256_transform (sha256_state *state, const uint32_t idata[16]) {
  uint32_t a, b, c, d, e, f, g, h, s0, s1;
  uint32_t T1, T2, *W256, v;
  int j;

  W256 = state->buffer32;

  /* Initialize registers with the prev. intermediate value */
  a = state->words[0];
  b = state->words[1];
  c = state->words[2];
  d = state->words[3];
  e = state->words[4];
  f = state->words[5];
  g = state->words[6];
  h = state->words[7];

  j = 0;
  do {
    v = idata[j];
#if BYTE_ORDER == LITTLE_ENDIAN
    /* Copy data while converting to host byte order */
    REVERSE32(v, W256[j]);
    /* Apply the SHA-256 compression function to update a..h */
    T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
    /* Apply the SHA-256 compression function to update a..h with copy */
    T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = v);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
    T2 = Sigma0_256(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;

    j++;
  } while (j < 16);

  do {
    /* Part of the message block expansion: */
    s0 = W256[(j+1)&0x0f];
    s0 = sigma0_256(s0);
    s1 = W256[(j+14)&0x0f];
    s1 = sigma1_256(s1);

    /* Apply the SHA-256 compression function to update a..h */
    T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
    T2 = Sigma0_256(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;

    j++;
  } while (j < 64);

  /* Compute the current intermediate hash value */
  state->words[0] += a;
  state->words[1] += b;
  state->words[2] += c;
  state->words[3] += d;
  state->words[4] += e;
  state->words[5] += f;
  state->words[6] += g;
  state->words[7] += h;
}

#endif /* SHA2_UNROLL_TRANSFORM */

/* PJ: alignment-safe version */

#define data_aligned4(data) (((data - (const uint8_t *)(0UL)) & 3) == 0)
#define data_aligned8(data) (((data - (const uint8_t *)(0ULL)) & 7) == 0)

static void sha256_transform_aligned (sha256_state *state, const uint8_t *data) {
  if (data_aligned4(data)) 
  {
    sha256_transform(state, (const uint32_t *)((const void *)data)); // alignment ok
  } 
  else 
  {
    uint32_t idata[16];
    memcpy(&idata[0], data, 16 * sizeof(uint32_t));
    sha256_transform(state, idata);
  }
}

void sha256_digest_add (sha256_state *state, const void *vdata, size_t len) 
{
  unsigned int freespace, usedspace;
  const uint8_t *data;

  if (len == 0) /* Calling with no data is valid - we do nothing */  
    return;

  data = (const uint8_t *)vdata;

  usedspace = (state->bitcount >> 3) % SHA256_BLOCK_LENGTH;
  if (usedspace > 0) 
  {
    /* Calculate how much free space is available in the buffer */
    freespace = SHA256_BLOCK_LENGTH - usedspace;

    if (len >= freespace) 
    {
      /* Fill the buffer completely and process it */
      MEMCPY_BCOPY(&state->buffer[usedspace], data, freespace);
      state->bitcount += freespace << 3;
      len -= freespace;
      data += freespace;
      sha256_transform(state, state->buffer32);
    }
    else 
    {
      /* The buffer is not yet full */
      MEMCPY_BCOPY(&state->buffer[usedspace], data, len);
      state->bitcount += len << 3;
      return;
    }
  }
  while (len >= SHA256_BLOCK_LENGTH) 
  {
    /* Process as many complete blocks as we can */
    sha256_transform_aligned(state, data);

    state->bitcount += SHA256_BLOCK_LENGTH << 3;
    len -= SHA256_BLOCK_LENGTH;
    data += SHA256_BLOCK_LENGTH;
  }
  if (len > 0) 
  {
    /* There's left-overs, so save 'em */
    MEMCPY_BCOPY(state->buffer, data, len);
    state->bitcount += len << 3;
  }
}

static void digest_hex (uint8_t digest[], const void *data, size_t size, int flags);

void sha256_digest_get (sha256_state *state, uint8_t digest[], int flags) {
  unsigned int usedspace;

  usedspace = (state->bitcount >> 3) % SHA256_BLOCK_LENGTH;
#if BYTE_ORDER == LITTLE_ENDIAN
  /* Convert FROM host byte order */
  REVERSE64(state->bitcount,state->bitcount);
#endif
  if (usedspace > 0) 
  {
    /* Begin padding with a 1 bit: */
    state->buffer[usedspace++] = 0x80;

    if (usedspace <= SHA256_SHORT_BLOCK_LENGTH) {
      /* Set-up for the last transform: */
      MEMSET_BZERO(&state->buffer[usedspace], SHA256_SHORT_BLOCK_LENGTH - usedspace);
    } else {
      if (usedspace < SHA256_BLOCK_LENGTH) {
        MEMSET_BZERO(&state->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
      }
      /* Do second-to-last transform: */
      sha256_transform(state, state->buffer32);

      /* And set-up for the last transform: */
      MEMSET_BZERO(state->buffer, SHA256_SHORT_BLOCK_LENGTH);
    }
  } 
  else 
  {
    /* Set-up for the last transform: */
    MEMSET_BZERO(state->buffer, SHA256_SHORT_BLOCK_LENGTH);

    /* Begin padding with a 1 bit: */
    *state->buffer = 0x80;
  }
  /* Set the bit count: */
  //*(uint64_t*)&state->buffer[SHA256_SHORT_BLOCK_LENGTH] = state->bitcount; // aliasing violation warning
  state->buffer64[SHA256_SHORT_BLOCK_LENGTH / sizeof(uint64_t)] = state->bitcount;

  /* Final transform: */
  sha256_transform(state, state->buffer32);

#if BYTE_ORDER == LITTLE_ENDIAN
  {
    /* Convert TO host byte order */
    int j;
    for (j = 0; j < 8; j++) 
    {
      REVERSE32(state->words[j], state->words[j]);
    }
  }
#endif
  if (flags & SHA_HEX)
    digest_hex(digest, state->words, SHA256_DIGEST_LENGTH, flags);
  else
    memcpy(digest, state->words, SHA256_DIGEST_LENGTH);
}

/*** SHA-512: *********************************************************/
sha512_state * sha512_digest_init (sha512_state *state) 
{
  MEMCPY_BCOPY(state->words, sha512_initial_hash_value, SHA512_DIGEST_LENGTH);
  MEMSET_BZERO(state->buffer, SHA512_BLOCK_LENGTH);
  state->bitcount[0] = 0;
  state->bitcount[1] = 0;
  return state;
}

#ifdef SHA2_UNROLL_TRANSFORM

/* PJ: ++ operations moved out of macros! */

/* Unrolled SHA-512 round macros: */
#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND512_0_TO_15(v, a, b, c, d, e, f, g, h) \
  REVERSE64(v, W512[j]); \
  T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + W512[j]; \
  (d) += T1; \
  (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c))

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512_0_TO_15(v, a, b, c, d, e, f, g, h) \
  T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + (W512[j] = v); \
  (d) += T1; \
  (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c))

#endif /* BYTE_ORDER == LITTLE_ENDIAN */

#define ROUND512(a, b, c, d, e, f, g, h) \
  s0 = W512[(j+1)&0x0f]; \
  s0 = sigma0_512(s0); \
  s1 = W512[(j+14)&0x0f]; \
  s1 = sigma1_512(s1); \
  T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0); \
  (d) += T1; \
  (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c))

static void sha512_transform (sha512_state *state, const uint64_t idata[16]) 
{
  uint64_t a, b, c, d, e, f, g, h, s0, s1;
  uint64_t T1, *W512, v;
  int j;

  W512 = state->buffer64;

  /* Initialize registers with the prev. intermediate value */
  a = state->words[0];
  b = state->words[1];
  c = state->words[2];
  d = state->words[3];
  e = state->words[4];
  f = state->words[5];
  g = state->words[6];
  h = state->words[7];

  j = 0;
  do {
    v = idata[j]; ROUND512_0_TO_15(v, a, b, c, d, e, f, g, h); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, h, a, b, c, d, e, f, g); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, g, h, a, b, c, d, e, f); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, f, g, h, a, b, c, d, e); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, e, f, g, h, a, b, c, d); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, d, e, f, g, h, a, b, c); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, c, d, e, f, g, h, a, b); ++j;
    v = idata[j]; ROUND512_0_TO_15(v, b, c, d, e, f, g, h, a); ++j;
  } while (j < 16);

  /* Now for the remaining rounds up to 79: */
  do {
    ROUND512(a, b, c, d, e, f, g, h); ++j;
    ROUND512(h, a, b, c, d, e, f, g); ++j;
    ROUND512(g, h, a, b, c, d, e, f); ++j;
    ROUND512(f, g, h, a, b, c, d, e); ++j;
    ROUND512(e, f, g, h, a, b, c, d); ++j;
    ROUND512(d, e, f, g, h, a, b, c); ++j;
    ROUND512(c, d, e, f, g, h, a, b); ++j;
    ROUND512(b, c, d, e, f, g, h, a); ++j;
  } while (j < 80);

  /* Compute the current intermediate hash value */
  state->words[0] += a;
  state->words[1] += b;
  state->words[2] += c;
  state->words[3] += d;
  state->words[4] += e;
  state->words[5] += f;
  state->words[6] += g;
  state->words[7] += h;
}

#else /* SHA2_UNROLL_TRANSFORM */

static void sha512_transform (sha512_state *state, const uint64_t idata[16]) 
{
  uint64_t a, b, c, d, e, f, g, h, s0, s1;
  uint64_t T1, T2, *W512, v;
  int j;

  W512 = state->buffer64;

  /* Initialize registers with the prev. intermediate value */
  a = state->words[0];
  b = state->words[1];
  c = state->words[2];
  d = state->words[3];
  e = state->words[4];
  f = state->words[5];
  g = state->words[6];
  h = state->words[7];

  j = 0;
  do {
    v = idata[j];
#if BYTE_ORDER == LITTLE_ENDIAN
    /* Convert TO host byte order */
    REVERSE64(v, W512[j]);
    /* Apply the SHA-512 compression function to update a..h */
    T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
#else /* BYTE_ORDER == LITTLE_ENDIAN */
    /* Apply the SHA-512 compression function to update a..h with copy */
    T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j] = v);
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
    T2 = Sigma0_512(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;

    j++;
  } while (j < 16);

  do {
    /* Part of the message block expansion: */
    s0 = W512[(j+1)&0x0f];
    s0 = sigma0_512(s0);
    s1 = W512[(j+14)&0x0f];
    s1 = sigma1_512(s1);

    /* Apply the SHA-512 compression function to update a..h */
    T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0);
    T2 = Sigma0_512(a) + Maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;

    j++;
  } while (j < 80);

  /* Compute the current intermediate hash value */
  state->words[0] += a;
  state->words[1] += b;
  state->words[2] += c;
  state->words[3] += d;
  state->words[4] += e;
  state->words[5] += f;
  state->words[6] += g;
  state->words[7] += h;
}

#endif /* SHA2_UNROLL_TRANSFORM */

static void sha512_transform_aligned (sha512_state *state, const uint8_t *data) 
{
  if (data_aligned8(data)) 
  {
    sha512_transform(state, (const uint64_t *)((const void *)data)); // alignment ok
  } 
  else 
  {
    uint64_t idata[16];
    memcpy(&idata[0], data, 16 * sizeof(uint64_t));
    sha512_transform(state, idata);
  }
}

void sha512_digest_add (sha512_state *state, const void *vdata, size_t len) 
{
  unsigned int freespace, usedspace;
  const uint8_t *data;

  if (len == 0) /* Calling with no data is valid - we do nothing */
    return;

  /* Sanity check: */
  data = (const uint8_t *)vdata;

  usedspace = (state->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH;
  if (usedspace > 0) 
  {
    /* Calculate how much free space is available in the buffer */
    freespace = SHA512_BLOCK_LENGTH - usedspace;

    if (len >= freespace) 
    {
      /* Fill the buffer completely and process it */
      MEMCPY_BCOPY(&state->buffer[usedspace], data, freespace);
      ADDINC128(state->bitcount, freespace << 3);
      len -= freespace;
      data += freespace;
      sha512_transform(state, state->buffer64);
    } 
    else 
    {
      /* The buffer is not yet full */
      MEMCPY_BCOPY(&state->buffer[usedspace], data, len);
      ADDINC128(state->bitcount, len << 3);
      return;
    }
  }
  while (len >= SHA512_BLOCK_LENGTH) 
  {
    /* Process as many complete blocks as we can */
    sha512_transform_aligned(state, data);

    ADDINC128(state->bitcount, SHA512_BLOCK_LENGTH << 3);
    len -= SHA512_BLOCK_LENGTH;
    data += SHA512_BLOCK_LENGTH;
  }
  if (len > 0) 
  {
    /* There's left-overs, so save 'em */
    MEMCPY_BCOPY(state->buffer, data, len);
    ADDINC128(state->bitcount, len << 3);
  }
}

static void sha512_last (sha512_state *state) 
{
  unsigned int usedspace;

  usedspace = (state->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH;
#if BYTE_ORDER == LITTLE_ENDIAN
  /* Convert FROM host byte order */
  REVERSE64(state->bitcount[0],state->bitcount[0]);
  REVERSE64(state->bitcount[1],state->bitcount[1]);
#endif
  if (usedspace > 0) 
  {
    /* Begin padding with a 1 bit: */
    state->buffer[usedspace++] = 0x80;

    if (usedspace <= SHA512_SHORT_BLOCK_LENGTH) {
      /* Set-up for the last transform: */
      MEMSET_BZERO(&state->buffer[usedspace], SHA512_SHORT_BLOCK_LENGTH - usedspace);
    } else {
      if (usedspace < SHA512_BLOCK_LENGTH) {
        MEMSET_BZERO(&state->buffer[usedspace], SHA512_BLOCK_LENGTH - usedspace);
      }
      /* Do second-to-last transform: */
      sha512_transform(state, state->buffer64);

      /* And set-up for the last transform: */
      //MEMSET_BZERO(state->buffer, SHA512_BLOCK_LENGTH - 2); // seems a typo, we overwrite last 16 bytes below
      MEMSET_BZERO(state->buffer, SHA512_SHORT_BLOCK_LENGTH);
    }
  } 
  else 
  {
    /* Prepare for final transform: */
    MEMSET_BZERO(state->buffer, SHA512_SHORT_BLOCK_LENGTH);

    /* Begin padding with a 1 bit: */
    *state->buffer = 0x80;
  }
  /* Store the length of input data (in bits): */
  //*(uint64_t*)&state->buffer[SHA512_SHORT_BLOCK_LENGTH] = state->bitcount[1]; // aliasing violation warning
  //*(uint64_t*)&state->buffer[SHA512_SHORT_BLOCK_LENGTH+8] = state->bitcount[0];
  state->buffer64[SHA512_SHORT_BLOCK_LENGTH / sizeof(uint64_t)] = state->bitcount[1];
  state->buffer64[SHA512_SHORT_BLOCK_LENGTH / sizeof(uint64_t) + 1] = state->bitcount[0];

  /* Final transform: */
  sha512_transform(state, state->buffer64);
}

void sha512_digest_get (sha512_state *state, uint8_t digest[], int flags) 
{
  /* If no digest buffer is passed, we don't bother doing this: */
  sha512_last(state);

  /* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
  {
    /* Convert TO host byte order */
    int j;
    for (j = 0; j < 8; j++) 
    {
      REVERSE64(state->words[j], state->words[j]);
    }
  }
#endif
  if (flags & SHA_HEX)
    digest_hex(digest, state->words, SHA512_DIGEST_LENGTH, flags);
  else
    memcpy(digest, state->words, SHA512_DIGEST_LENGTH);
}

/*** SHA-384: *********************************************************/
sha384_state * sha384_digest_init (sha384_state *state) 
{
  MEMCPY_BCOPY(state->words, sha384_initial_hash_value, SHA512_DIGEST_LENGTH);
  MEMSET_BZERO(state->buffer, SHA384_BLOCK_LENGTH);
  state->bitcount[0] = state->bitcount[1] = 0;
  return state;
}

void sha384_digest_add (sha384_state *state, const void *data, size_t len) 
{
  sha512_digest_add((sha512_state *)state, data, len);
}

void sha384_digest_get (sha384_state *state, uint8_t digest[], int flags) 
{
  sha512_last((sha512_state *)state);

  /* Save the hash data for output: */
#if BYTE_ORDER == LITTLE_ENDIAN
  {
    /* Convert TO host byte order */
    int j;
    for (j = 0; j < 6; j++) 
    {
      REVERSE64(state->words[j], state->words[j]);
    }
  }
#endif
  if (flags & SHA_HEX)
    digest_hex(digest, state->words, SHA384_DIGEST_LENGTH, flags);
  else
    memcpy(digest, state->words, SHA384_DIGEST_LENGTH);
}

/* hex output */

static void digest_hex (uint8_t digest[], const void *data, size_t size, int flags)
{
  const char *alphabet;
  const uint8_t *bytes;
  size_t i;

  bytes = (const uint8_t *)data;
  alphabet = (flags & SHA_LCHEX) ? "0123456789abcdef" : "0123456789ABCDEF";
  for (i = 0; i < size; ++i, ++bytes)
  {
    *digest++ = (uint8_t)alphabet[(*bytes) >> 4];
    *digest++ = (uint8_t)alphabet[(*bytes) & 15];
  }
  *digest = 0;
}

/* string checksum */

void sha256_digest (const void *data, size_t size, uint8_t digest[], int flags)
{
  sha256_state state;
  sha256_digest_init(&state);
  sha256_digest_add(&state, data, size);
  sha256_digest_get(&state, digest, flags);
}

void sha384_digest (const void *data, size_t size, uint8_t digest[], int flags)
{
  sha384_state state;
  sha384_digest_init(&state);
  sha384_digest_add(&state, data, size);
  sha384_digest_get(&state, digest, flags);
}

void sha512_digest (const void *data, size_t size, uint8_t digest[], int flags)
{
  sha512_state state;
  sha512_digest_init(&state);
  sha512_digest_add(&state, data, size);
  sha512_digest_get(&state, digest, flags);
}

/* file checksum */

#define DIGEST_BUFFER_SIZE 4096

int sha256_digest_add_file (sha256_state *state, const char *filename)
{
  FILE *fh;
  uint8_t buffer[DIGEST_BUFFER_SIZE];
  size_t read;

  if ((fh = fopen(filename, "rb")) == NULL)
    return 0;
  do {
    read = fread(buffer, 1, DIGEST_BUFFER_SIZE, fh);
    sha256_digest_add(state, buffer, read);
  } while (read == DIGEST_BUFFER_SIZE);
  fclose(fh);
  return 1;
}

int sha256_digest_file (const char *filename, uint8_t digest[], int flags)
{
  sha256_state state;

  sha256_digest_init(&state);
  if (sha256_digest_add_file(&state, filename))
  {
    sha256_digest_get(&state, digest, flags);
    return 1;
  }
  return 0;
}

int sha384_digest_add_file (sha384_state *state, const char *filename)
{
  FILE *fh;
  uint8_t buffer[DIGEST_BUFFER_SIZE];
  size_t read;

  if ((fh = fopen(filename, "rb")) == NULL)
    return 0;
  do {
    read = fread(buffer, 1, DIGEST_BUFFER_SIZE, fh);
    sha384_digest_add(state, buffer, read);
  } while (read == DIGEST_BUFFER_SIZE);
  fclose(fh);
  return 1;
}

int sha384_digest_file (const char *filename, uint8_t digest[], int flags)
{
  sha384_state state;

  sha384_digest_init(&state);
  if (sha384_digest_add_file(&state, filename))
  {
    sha384_digest_get(&state, digest, flags);
    return 1;
  }
  return 0;
}

int sha512_digest_add_file (sha512_state *state, const char *filename)
{
  FILE *fh;
  uint8_t buffer[DIGEST_BUFFER_SIZE];
  size_t read;

  if ((fh = fopen(filename, "rb")) == NULL)
    return 0;
  do {
    read = fread(buffer, 1, DIGEST_BUFFER_SIZE, fh);
    sha512_digest_add(state, buffer, read);
  } while (read == DIGEST_BUFFER_SIZE);
  fclose(fh);
  return 1;
}

int sha512_digest_file (const char *filename, uint8_t digest[], int flags)
{
  sha512_state state;

  sha512_digest_init(&state);
  if (sha512_digest_add_file(&state, filename))
  {
    sha512_digest_get(&state, digest, flags);
    return 1;
  }
  return 0;
}
