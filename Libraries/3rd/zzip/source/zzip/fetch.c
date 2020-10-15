
/*
 * These routines are helpers - note that this file gets linked into all the
 * zip-access variants that refer to <zzip/format.h>. On the x86 platform the
 * actual definitions will be empty - fetching is done on native machine-level
 *
 * Author:
 *      Guido Draheim <guidod@gmx.de>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */

#include <zzip/fetch.h>

/* ------------------------- fetch helpers --------------------------------- */

/** internal.
 * Make 32 bit value in host byteorder from little-endian mapped octet-data
 * (works also on machines which SIGBUS on misaligned data access (eg. 68000))
 */
uint32_t
__zzip_get32(unsigned char *s)
{
#if defined __ZZIP_GET32
    return __ZZIP_GET32(s);
#else
    return ((uint32_t) s[3] << 24) | ((uint32_t) s[2] << 16)
        | ((uint32_t) s[1] << 8) | ((uint32_t) s[0]);
#endif
}

/** => __zzip_get32
 * This function does the same for a 16 bit value.
 */
uint16_t
__zzip_get16(unsigned char *s)
{
#if defined __ZZIP_GET16
    return __ZZIP_GET16(s);
#else
    return ((uint16_t) s[1] << 8) | ((uint16_t) s[0]);
#endif
}

/** => __zzip_get32
 * This function does the same for an off64_t value.
 */
uint64_t
__zzip_get64(unsigned char *s)
{
#ifdef __GNUC__
    /* *INDENT-OFF* */
    register uint64_t v
       = s[7]; v <<= 8;
    v |= s[6]; v <<= 8;
    v |= s[5]; v <<= 8;
    v |= s[4]; v <<= 8;
    v |= s[3]; v <<= 8;
    v |= s[2]; v <<= 8;
    v |= s[1]; v <<= 8;
    v |= s[0]; return v;
    /* *INDENT-ON* */
#else
    return ((uint64_t) s[7] << 56) | ((uint64_t) s[6] << 48)
        | ((uint64_t) s[5] << 40) | ((uint64_t) s[4] << 32)
        | ((uint64_t) s[3] << 24) | ((uint64_t) s[2] << 16)
        | ((uint64_t) s[1] << 8) | ((uint64_t) s[0]);
#endif
}

/** => __zzip_get32
 * This function pushes a 32bit value at the specified address
 */
void
__zzip_set32(unsigned char *s, uint32_t v)
{
#if defined __ZZIP_SET32
    return __ZZIP_SET32(s, v);
#else
    /* *INDENT-OFF* */
    s[0] = (unsigned char) (v); v >>= 8;
    s[1] = (unsigned char) (v); v >>= 8;
    s[2] = (unsigned char) (v); v >>= 8;
    s[3] = (unsigned char) (v);
    /* *INDENT-ON* */
#endif
}

/** => __zzip_get32
 * This function does the same for a 16 bit value.
 */
void
__zzip_set16(unsigned char *s, uint16_t v)
{
#if defined __ZZIP_SET16
    return __ZZIP_SET16(s, v);
#else
    /* *INDENT-OFF* */
    s[0] = (unsigned char) (v); v >>= 8;
    s[1] = (unsigned char) (v);
    /* *INDENT-ON* */
#endif
}

/** => __zzip_get32
 * This function pushes a off64_t value at the specified address
 */
void
__zzip_set64(unsigned char *s, uint64_t v)
{
    /* *INDENT-OFF* */
    s[0] = (unsigned char) (v); v >>= 8;
    s[1] = (unsigned char) (v); v >>= 8;
    s[2] = (unsigned char) (v); v >>= 8;
    s[3] = (unsigned char) (v); v >>= 8;
    s[4] = (unsigned char) (v); v >>= 8;
    s[5] = (unsigned char) (v); v >>= 8;
    s[6] = (unsigned char) (v); v >>= 8;
    s[7] = (unsigned char) (v);
    /* *INDENT-ON* */
}
