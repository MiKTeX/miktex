/* -*- related-file-name: "../../liblcdf/md5.c" -*-
 * md5.h - MD5 Message-Digest Algorithm
 *	Copyright (C) 1995, 1996, 1998, 1999 Free Software Foundation, Inc.
 *
 * according to the definition of MD5 in RFC 1321 from April 1992.
 * NOTE: This is *not* the same file as the one from glibc
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

#ifndef LCDF_MD5_H
#define LCDF_MD5_H
#ifdef __cplusplus
extern "C" {
#endif

#define MD5_DIGEST_SIZE		16
#define MD5_TEXT_DIGEST_SIZE	26 /* + 1 for the terminating NUL */

typedef struct {  /* Hmm, should be private */
    uint32_t A,B,C,D;
    uint32_t nblocks;
    unsigned char buf[64];
    int  count;
    int  finalized;
} MD5_CONTEXT;

void md5_init(MD5_CONTEXT *ctx);
void md5_update(MD5_CONTEXT *hd, const unsigned char *inbuf, size_t inlen);
void md5_final(unsigned char digest[MD5_DIGEST_SIZE], MD5_CONTEXT *ctx);
void md5_final_text(char *text_digest, MD5_CONTEXT *ctx);

#ifdef __cplusplus
}
#endif
#endif /* LCDF_MD5_H */
