/* -*- mode: c; c-basic-offset: 8; related-file-name: "../include/lcdf/md5.h" -*- */
/* md5.c - MD5 Message-Digest Algorithm
 *	Copyright (C) 1995, 1996, 1998, 1999 Free Software Foundation, Inc.
 *
 * according to the definition of MD5 in RFC 1321 from April 1992.
 * NOTE: This is *not* the same file as the one from glibc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */
/* heavily modified for GnuPG by <werner.koch@guug.de> */
/* modified again for Sylpheed by <wk@gnupg.org> 2001-02-11 */
/* modified again for LCDF by Eddie Kohler <kohler@icir.org> */


/* Test values:
 * ""                  D4 1D 8C D9 8F 00 B2 04  E9 80 09 98 EC F8 42 7E
 * "a"                 0C C1 75 B9 C0 F1 B6 A8  31 C3 99 E2 69 77 26 61
 * "abc                90 01 50 98 3C D2 4F B0  D6 96 3F 7D 28 E1 7F 72
 * "message digest"    F9 6B 69 7D 7C B7 93 8D  52 5A 2F 31 AA F1 61 D0
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <lcdf/inttypes.h>
#include <lcdf/md5.h>

#ifdef __cplusplus
extern "C" {
#endif


/****************
 * Rotate a 32 bit integer by n bits
 */
#if defined(__GNUC__) && defined(__i386__)
static inline uint32_t
rol( uint32_t x, int n)
{
	__asm__("roll %%cl,%0"
		:"=r" (x)
		:"0" (x),"c" (n));
	return x;
}
#else
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
#endif


void
md5_init(MD5_CONTEXT *ctx)
{
	ctx->A = 0x67452301;
	ctx->B = 0xefcdab89;
	ctx->C = 0x98badcfe;
	ctx->D = 0x10325476;

	ctx->nblocks = 0;
	ctx->count = 0;
	ctx->finalized = 0;
}

/* These are the four functions used in the four steps of the MD5 algorithm
   and defined in the RFC 1321.  The first function is a little bit optimized
   (as found in Colin Plumbs public domain implementation).  */
/* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))


/****************
 * transform n*64 bytes
 */
static void
transform(MD5_CONTEXT *ctx, const unsigned char *data)
{
	uint32_t correct_words[16];
	uint32_t A = ctx->A;
	uint32_t B = ctx->B;
	uint32_t C = ctx->C;
	uint32_t D = ctx->D;
	uint32_t *cwp = correct_words;

#if WORDS_BIGENDIAN
	{
		int i;
		unsigned char *p2, *p1;

		for (i = 0, p1 = data, p2 = (unsigned char*)correct_words;
		     i < 16; i++, p2 += 4) {
			p2[3] = *p1++;
			p2[2] = *p1++;
			p2[1] = *p1++;
			p2[0] = *p1++;
		}
	}
#elif WORDS_BIGENDIAN_SET
	memcpy(correct_words, data, 64);
#else
# error "WORDS_BIGENDIAN has not been set!"
#endif


#define OP(a, b, c, d, s, T)				\
	do {						\
		a += FF (b, c, d) + (*cwp++) + T;	\
		a = rol(a, s);				\
		a += b;					\
	} while (0)

	/* Before we start, one word about the strange constants.
	   They are defined in RFC 1321 as

	   T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64
	 */

	/* Round 1.  */
	OP (A, B, C, D,  7, 0xd76aa478);
	OP (D, A, B, C, 12, 0xe8c7b756);
	OP (C, D, A, B, 17, 0x242070db);
	OP (B, C, D, A, 22, 0xc1bdceee);
	OP (A, B, C, D,  7, 0xf57c0faf);
	OP (D, A, B, C, 12, 0x4787c62a);
	OP (C, D, A, B, 17, 0xa8304613);
	OP (B, C, D, A, 22, 0xfd469501);
	OP (A, B, C, D,  7, 0x698098d8);
	OP (D, A, B, C, 12, 0x8b44f7af);
	OP (C, D, A, B, 17, 0xffff5bb1);
	OP (B, C, D, A, 22, 0x895cd7be);
	OP (A, B, C, D,  7, 0x6b901122);
	OP (D, A, B, C, 12, 0xfd987193);
	OP (C, D, A, B, 17, 0xa679438e);
	OP (B, C, D, A, 22, 0x49b40821);

#undef OP
#define OP(f, a, b, c, d, k, s, T)  \
	do {							\
		a += f (b, c, d) + correct_words[k] + T;	\
		a = rol(a, s);					\
		a += b;					\
	} while (0)

	/* Round 2.  */
	OP (FG, A, B, C, D,  1,  5, 0xf61e2562);
	OP (FG, D, A, B, C,  6,  9, 0xc040b340);
	OP (FG, C, D, A, B, 11, 14, 0x265e5a51);
	OP (FG, B, C, D, A,  0, 20, 0xe9b6c7aa);
	OP (FG, A, B, C, D,  5,  5, 0xd62f105d);
	OP (FG, D, A, B, C, 10,  9, 0x02441453);
	OP (FG, C, D, A, B, 15, 14, 0xd8a1e681);
	OP (FG, B, C, D, A,  4, 20, 0xe7d3fbc8);
	OP (FG, A, B, C, D,  9,  5, 0x21e1cde6);
	OP (FG, D, A, B, C, 14,  9, 0xc33707d6);
	OP (FG, C, D, A, B,  3, 14, 0xf4d50d87);
	OP (FG, B, C, D, A,  8, 20, 0x455a14ed);
	OP (FG, A, B, C, D, 13,  5, 0xa9e3e905);
	OP (FG, D, A, B, C,  2,  9, 0xfcefa3f8);
	OP (FG, C, D, A, B,  7, 14, 0x676f02d9);
	OP (FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

	/* Round 3.  */
	OP (FH, A, B, C, D,  5,  4, 0xfffa3942);
	OP (FH, D, A, B, C,  8, 11, 0x8771f681);
	OP (FH, C, D, A, B, 11, 16, 0x6d9d6122);
	OP (FH, B, C, D, A, 14, 23, 0xfde5380c);
	OP (FH, A, B, C, D,  1,  4, 0xa4beea44);
	OP (FH, D, A, B, C,  4, 11, 0x4bdecfa9);
	OP (FH, C, D, A, B,  7, 16, 0xf6bb4b60);
	OP (FH, B, C, D, A, 10, 23, 0xbebfbc70);
	OP (FH, A, B, C, D, 13,  4, 0x289b7ec6);
	OP (FH, D, A, B, C,  0, 11, 0xeaa127fa);
	OP (FH, C, D, A, B,  3, 16, 0xd4ef3085);
	OP (FH, B, C, D, A,  6, 23, 0x04881d05);
	OP (FH, A, B, C, D,  9,  4, 0xd9d4d039);
	OP (FH, D, A, B, C, 12, 11, 0xe6db99e5);
	OP (FH, C, D, A, B, 15, 16, 0x1fa27cf8);
	OP (FH, B, C, D, A,  2, 23, 0xc4ac5665);

	/* Round 4.  */
	OP (FI, A, B, C, D,  0,  6, 0xf4292244);
	OP (FI, D, A, B, C,  7, 10, 0x432aff97);
	OP (FI, C, D, A, B, 14, 15, 0xab9423a7);
	OP (FI, B, C, D, A,  5, 21, 0xfc93a039);
	OP (FI, A, B, C, D, 12,  6, 0x655b59c3);
	OP (FI, D, A, B, C,  3, 10, 0x8f0ccc92);
	OP (FI, C, D, A, B, 10, 15, 0xffeff47d);
	OP (FI, B, C, D, A,  1, 21, 0x85845dd1);
	OP (FI, A, B, C, D,  8,  6, 0x6fa87e4f);
	OP (FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
	OP (FI, C, D, A, B,  6, 15, 0xa3014314);
	OP (FI, B, C, D, A, 13, 21, 0x4e0811a1);
	OP (FI, A, B, C, D,  4,  6, 0xf7537e82);
	OP (FI, D, A, B, C, 11, 10, 0xbd3af235);
	OP (FI, C, D, A, B,  2, 15, 0x2ad7d2bb);
	OP (FI, B, C, D, A,  9, 21, 0xeb86d391);

	/* Put checksum in context given as argument.  */
	ctx->A += A;
	ctx->B += B;
	ctx->C += C;
	ctx->D += D;
}



/* The routine updates the message-digest context to
 * account for the presence of each of the characters inBuf[0..inLen-1]
 * in the message whose digest is being computed.
 */
void
md5_update(MD5_CONTEXT *hd, const unsigned char *inbuf, size_t inlen)
{
	if (hd->count == 64) { /* flush the buffer */
		transform( hd, hd->buf );
		hd->count = 0;
		hd->nblocks++;
	}
	if (!inbuf)
		return;
	if (hd->count) {
		for (; inlen && hd->count < 64; inlen--)
			hd->buf[hd->count++] = *inbuf++;
		md5_update(hd, NULL, 0);
		if (!inlen)
			return;
	}

	while (inlen >= 64) {
		transform(hd, inbuf);
		hd->count = 0;
		hd->nblocks++;
		inlen -= 64;
		inbuf += 64;
	}

	for (; inlen && hd->count < 64; inlen--)
		hd->buf[hd->count++] = *inbuf++;
}



/* The routine final terminates the message-digest computation and
 * ends with the desired message digest in mdContext->digest[0...15].
 * The handle is prepared for a new MD5 cycle.
 * Returns 16 bytes representing the digest.
 */

static void
do_final(MD5_CONTEXT *hd)
{
	uint32_t t, msb, lsb;
	unsigned char *p;

	md5_update(hd, NULL, 0); /* flush */

	msb = 0;
	t = hd->nblocks;
	if ((lsb = t << 6) < t) /* multiply by 64 to make a byte count */
		msb++;
	msb += t >> 26;
	t = lsb;
	if ((lsb = t + hd->count) < t) /* add the count */
		msb++;
	t = lsb;
	if ((lsb = t << 3) < t) /* multiply by 8 to make a bit count */
		msb++;
	msb += t >> 29;

	if (hd->count < 56) { /* enough room */
		hd->buf[hd->count++] = 0x80; /* pad */
		while(hd->count < 56)
			hd->buf[hd->count++] = 0;  /* pad */
	} else { /* need one extra block */
		hd->buf[hd->count++] = 0x80; /* pad character */
		while (hd->count < 64)
			hd->buf[hd->count++] = 0;
		md5_update(hd, NULL, 0);  /* flush */
		memset(hd->buf, 0, 56); /* fill next block with zeroes */
	}

	/* append the 64 bit count */
	hd->buf[56] = lsb      ;
	hd->buf[57] = lsb >>  8;
	hd->buf[58] = lsb >> 16;
	hd->buf[59] = lsb >> 24;
	hd->buf[60] = msb      ;
	hd->buf[61] = msb >>  8;
	hd->buf[62] = msb >> 16;
	hd->buf[63] = msb >> 24;
	transform(hd, hd->buf);

	p = hd->buf;
#if WORDS_BIGENDIAN
#define X(a) do { *p++ = hd->a      ; *p++ = hd->a >> 8;      \
		  *p++ = hd->a >> 16; *p++ = hd->a >> 24; } while(0)
#elif WORDS_BIGENDIAN_SET
	/*#define X(a) do { *(uint32_t*)p = hd->##a ; p += 4; } while(0)*/
	/* Unixware's cpp doesn't like the above construct so we do it his way:
	 * (reported by Allan Clark) */
#define X(a) do { *(uint32_t*)p = (*hd).a ; p += 4; } while(0)
#else
# error "WORDS_BIGENDIAN has not been set!"
#endif
	X(A);
	X(B);
	X(C);
	X(D);
#undef X
	hd->finalized = 1;
}

void
md5_final(unsigned char *digest, MD5_CONTEXT *ctx)
{
	if (!ctx->finalized)
		do_final(ctx);
	memcpy(digest, ctx->buf, 16);
}

void
md5_final_text(char *buf, MD5_CONTEXT *ctx)
{
	static const char *chars = "abcdefghijklmnopqrstuvwxyz234567";
	int bit;
	if (!ctx->finalized)
		do_final(ctx);
	for (bit = 0; bit < 16*8; bit += 5) {
		int first_char = bit / 8;
		int val = ctx->buf[first_char] >> (bit % 8);
		if (bit + 8 > (first_char + 1) * 8 && first_char < 15)
			val += ctx->buf[first_char + 1] << (8 - (bit % 8));
		*buf++ = chars[val & 0x1F];
	}
	*buf++ = 0;
}


#ifdef __cplusplus
}
#endif
