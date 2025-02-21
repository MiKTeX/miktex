/*
 * Declarations of functions and data types used for SHA1 sum
 * library functions.
 *
 * Copyright (C) 2015-2025 The Gregorio Project (see CONTRIBUTORS.md)
 * Copyright (C) 2000-2001, 2003, 2005-2006, 2008-2014 Free Software
 * Foundation, Inc.
 *
 * This file is part of Gregorio.
 * This file has been adapted from GNU coreutils to fit Gregorio.
 * 
 * Gregorio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gregorio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHA1_H
#define SHA1_H 1

#include <stddef.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#define SHA1_DIGEST_SIZE 20

/* Structure to save state of computation between the single steps.  */
struct sha1_ctx {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint32_t E;

    uint32_t total[2];
    uint32_t buflen;
    uint32_t buffer[32];
};

/* Initialize structure containing state of computation. */
extern void sha1_init_ctx(struct sha1_ctx *ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
extern void sha1_process_block(const void *buffer, size_t len,
        struct sha1_ctx *ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
extern void sha1_process_bytes(const void *buffer, size_t len,
        struct sha1_ctx *ctx);

/* Process the remaining bytes in the buffer and put result from CTX
   in first 20 bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.  */
extern void *sha1_finish_ctx(struct sha1_ctx *ctx, void *resbuf);

/* Put result from CTX in first 20 bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.  */
extern void *sha1_read_ctx(const struct sha1_ctx *ctx, void *resbuf);

/* Compute SHA1 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
extern void *sha1_buffer(const char *buffer, size_t len, void *resblock);

#endif
