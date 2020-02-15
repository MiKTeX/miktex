#ifndef UTIL_NUMBER_H
#define UTIL_NUMBER_H

#include <stddef.h> // for size_t

#include "utilplat.h"
#include "utildecl.h"

#if defined(__cplusplus) && defined(_MSC_VER)
// int*_t types are in standard in msvc++
#else
#  include <stdint.h>
#endif

/* 'long' isn't long for msvc64/mingw64, we need a type for machine word */

#if defined(_WIN64) || defined(__MINGW32__)
#  define INT64F "%I64d"
#  define UINT64F "%I64u"
#else
#  define INT64F "%lld"
#  define UINT64F "%llu"
#endif

#if defined(MSVC64)
#  define INTLW_IS_INT64
#  define intlw_t int64_t
#  define uintlw_t uint64_t
#  define INTLW(N) N##I64
#  define UINTLW(N) N##UI64
#  define INTLWF INT64F
#  define UINTLWF UINT64F
#elif defined(__MINGW64__)
#  define INTLW_IS_INT64
#  define intlw_t int64_t
#  define uintlw_t uint64_t
#  define INTLW(N) N##LL
#  define UINTLW(N) N##ULL
#  define INTLWF INT64F
#  define UINTLWF UINT64F
#else // 32bit or sane 64bit (LP64)
#  define INTLW_IS_LONG
#  define intlw_t long
#  define uintlw_t unsigned long
#  define INTLW(N) N##L
#  define UINTLW(N) N##UL
#  define INTLWF "%ld"
#  define UINTLWF "%lu"
#endif

// ssize_t is missing in MSVC, but defining it is risky; some environments (eg. python) typedefs ssize_t on its own way..
// #if defined(MSVC64)
// #  define ssize_t int32_t
// #else
// #  if defined(MSVC32)
// #    define ssize_t int64_t
// #  endif
// #endif

/* basic constants */

#define MAX_RADIX 36
#define MAX_INTEGER_DIGITS 65 /* 64-bit number in binary form plus '\0' */
#define MAX_ROMAN_DIGITS 128  /* to handle romannumeral of short int (up to 65 leading 'M') */
#define MAX_NUMBER_DIGITS 512
#define NUMBER_BUFFER_SIZE MAX_NUMBER_DIGITS

#define base36_uc_alphabet "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define base36_lc_alphabet "0123456789abcdefghijklmnopqrstuvwxyz"

#define base26_uc_alphabet "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define base26_lc_alphabet "abcdefghijklmnopqrstuvwxyz"
extern const int base26_lookup[];

#define base36_lc_palindrome "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"
#define base36_uc_palindrome "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

extern const int base36_lookup[];

#define base10_palindrome "9876543210123456789"
#define base10_alphabet "0123456789"
extern const int base10_lookup[];

#define base16_uc_alphabet "0123456789ABCDEF"
#define base16_lc_alphabet "0123456789abcdef"
extern const int base16_lookup[];

#define base16_uc_digit1(c) base16_uc_alphabet[(c)>>4]
#define base16_uc_digit2(c) base16_uc_alphabet[(c)&15]
#define base16_lc_digit1(c) base16_lc_alphabet[(c)>>4]
#define base16_lc_digit2(c) base16_lc_alphabet[(c)&15]

#define base8_digit(c)  ((unsigned)(c - '0') <= (unsigned)('7' - '0'))
#define base8_value(c)  (base8_digit(c) ? (c) - '0' : -1)

#define base10_digit(c) ((unsigned)(c - '0') <= (unsigned)('9' - '0'))
#define base10_value(c) (base10_lookup[(uint8_t)(c)])

#define base16_digit(c) (base16_lookup[(uint8_t)(c)] >= 0)
#define base16_value(c) (base16_lookup[(uint8_t)(c)])

#define base26_digit(c) (base26_lookup[(uint8_t)(c)] >= 0)
#define base26_value(c) (base26_lookup[(uint8_t)(c)])

#define base36_digit(c) (base36_lookup[(uint8_t)(c)] >= 0)
#define base36_value(c) (base36_lookup[(uint8_t)(c)])

//#define base_digit(c, radix) ((unsigned)(base36_lookup[c]) < (unsigned)(radix))
//#define base_value(c, radix) (base_digit(c, radix) ? base36_lookup[c] : -1)

UTILDEF extern char util_number_buffer[NUMBER_BUFFER_SIZE];

/* integer from string; return a pointer to character next to the last digit */

UTILAPI const char * string_to_int32 (const char *s, int32_t *number);
UTILAPI const char * string_to_slong (const char *s, long *number);
UTILAPI const char * string_to_int64 (const char *s, int64_t *number);

UTILAPI const char * string_to_uint32 (const char *s, uint32_t *number);
UTILAPI const char * string_to_ulong (const char *s, unsigned long *number);
UTILAPI const char * string_to_usize (const char *s, size_t *number);
UTILAPI const char * string_to_uint64 (const char *s, uint64_t *number);

UTILAPI const char * radix_to_int32 (const char *s, int32_t *number, int radix);
UTILAPI const char * radix_to_slong (const char *s, long *number, int radix);
UTILAPI const char * radix_to_int64 (const char *s, int64_t *number, int radix);

UTILAPI const char * radix_to_uint32 (const char *s, uint32_t *number, int radix);
UTILAPI const char * radix_to_ulong (const char *s, unsigned long *number, int radix);
UTILAPI const char * radix_to_usize (const char *s, size_t *number, int radix);
UTILAPI const char * radix_to_uint64 (const char *s, uint64_t *number, int radix);

UTILAPI const char * alpha_to_uint32 (const char *s, uint32_t *number);
UTILAPI const char * alpha_to_ulong (const char *s, unsigned long *number);
UTILAPI const char * alpha_to_usize (const char *s, size_t *number);
UTILAPI const char * alpha_to_uint64 (const char *s, uint64_t *number);

/* integer to string */

UTILAPI char * int32_as_string (int32_t number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * slong_as_string (long number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * int64_as_string (int64_t number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);

#define int32_to_string(number, psize) int32_as_string(number, util_number_buffer, psize)
#define slong_to_string(number, psize) slong_as_string(number, util_number_buffer, psize)
#define int64_to_string(number, psize) int64_as_string(number, util_number_buffer, psize)

UTILAPI char * uint32_as_string (uint32_t number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * ulong_as_string (unsigned long number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * usize_as_string  (size_t   number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * uint64_as_string (uint64_t number, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);

#define uint32_to_string(number, psize) uint32_as_string(number, util_number_buffer, psize)
#define ulong_to_string(number, psize) ulong_as_string(number, util_number_buffer, psize)
#define usize_to_string(number, psize)  usize_as_string(number, util_number_buffer, psize)
#define uint64_to_string(number, psize) uint64_as_string(number, util_number_buffer, psize)

UTILAPI char * int32_as_radix (int32_t number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * slong_as_radix (long number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * int64_as_radix (int64_t number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);

#define int32_to_radix(number, radix, uc, psize) int32_as_radix(number, radix, uc, util_number_buffer, psize)
#define slong_to_radix(number, radix, uc, psize) slong_as_radix(number, radix, uc, util_number_buffer, psize)
#define int64_to_radix(number, radix, uc, psize) int64_as_radix(number, radix, uc, util_number_buffer, psize)

UTILAPI char * uint32_as_radix (uint32_t number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * ulong_as_radix (unsigned long number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * usize_as_radix  (size_t   number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * uint64_as_radix (uint64_t number, int radix, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);

#define uint32_to_radix(number, radix, uc, psize) uint32_as_radix(number, radix, uc, util_number_buffer, psize)
#define ulong_to_radix(number, radix, uc, psize) ulong_as_radix(number, radix, uc, util_number_buffer, psize)
#define usize_to_radix(number, radix, uc, psize)  usize_as_radix(number, radix, uc, util_number_buffer, psize)
#define uint64_to_radix(number, radix, uc, psize) uint64_as_radix(number, radix, uc, util_number_buffer, psize)

UTILAPI char * uint32_as_alpha (uint32_t number, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * ulong_as_alpha (unsigned long number, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * usize_as_alpha  (size_t   number, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);
UTILAPI char * uint64_as_alpha (uint64_t number, int uc, char ibuf[MAX_INTEGER_DIGITS], size_t *psize);

#define uint32_to_alpha(number, uc, psize) uint32_as_alpha(number, uc, util_number_buffer, psize)
#define ulong_to_alpha(number, uc, psize) ulong_as_alpha(number, uc, util_number_buffer, psize)
#define usize_to_alpha(number, uc, psize)  usize_as_alpha(number, uc, util_number_buffer, psize)
#define uint64_to_alpha(number, uc, psize) uint64_as_alpha(number, uc, util_number_buffer, psize)

#if defined(INTLW_IS_INT64)

#  define string_to_intlw(s, number) string_to_int64(s, number)
#  define string_to_uintlw(s, number) string_to_uint64(s, number)

#  define radix_to_intlw(s, number, radix) radix_to_int64(s, number, radix)
#  define radix_to_uintlw(s, number, radix) radix_to_uint64(s, number, radix)

#  define alpha_to_uintlw(s, number) alpha_to_uint64(s, number)

#  define intlw_as_string(number, ibuf, psize) int64_as_string(number, ibuf, psize)
#  define uintlw_as_string(number, ibuf, psize) uint64_as_string(number, ibuf, psize)

#  define intlw_to_string(number, psize) int64_to_string(number, psize)
#  define uintlw_to_string(number, psize) uint64_to_string(number, psize)

#  define intlw_as_radix(number, radix, uc, ibuf, psize) int64_as_radix(number, radix, uc, ibuf, psize)
#  define uintlw_as_radix(number, radix, uc, ibuf, psize) uint64_as_radix(number, radix, uc, ibuf, psize)

#  define intlw_to_radix(number, radix, uc, psize) int64_to_radix(number, radix, uc, psize)
#  define uintlw_to_radix(number, radix, uc, psize) uint64_to_radix(number, radix, uc, psize)

#  define uintlw_as_alpha(number, uc, ibuf, psize) uint64_as_alpha(number, uc, ibuf, psize)
#  define uintlw_to_alpha(number, uc, psize) uint64_to_alpha(number, uc, ibuf, psize)

#elif defined(INTLW_IS_LONG)

#  define string_to_intlw(s, number) string_to_slong(s, number)
#  define string_to_uintlw(s, number) string_to_ulong(s, number)

#  define radix_to_intlw(s, number, radix) radix_to_slong(s, number, radix)
#  define radix_to_uintlw(s, number, radix) radix_to_ulong(s, number, radix)

#  define alpha_to_uintlw(s, number) alpha_to_ulong(s, number)

#  define intlw_as_string(number, ibuf, psize) slong_as_string(number, ibuf, psize)
#  define uintlw_as_string(number, ibuf, psize) ulong_as_string(number, ibuf, psize)

#  define intlw_to_string(number, psize) slong_to_string(number, psize)
#  define uintlw_to_string(number, psize) ulong_to_string(number, psize)

#  define intlw_as_radix(number, radix, uc, ibuf, psize) slong_as_radix(number, radix, uc, ibuf, psize)
#  define uintlw_as_radix(number, radix, uc, ibuf, psize) ulong_as_radix(number, radix, uc, ibuf, psize)

#  define intlw_to_radix(number, radix, uc, psize) slong_to_radix(number, radix, uc, psize)
#  define uintlw_to_radix(number, radix, uc, psize) ulong_to_radix(number, radix, uc, psize)

#  define uintlw_as_alpha(number, uc, ibuf, psize) ulong_as_alpha(number, uc, ibuf, psize)
#  define uintlw_to_alpha(number, uc, psize) ulong_to_alpha(number, uc, ibuf, psize)

#endif

/* a..z, aa..zz, aaa..zzz (limited to uint16_t, valid for N <= buffer_size * 26) */

UTILAPI const char * alphan_to_uint16 (const char *s, uint16_t *number);
UTILAPI char * uint16_as_alphan (uint16_t number, int uc, char ibuf[], size_t size, size_t *psize);
#define uint16_to_alphan(number, uc, psize) uint16_as_alphan(number, uc, util_number_buffer, NUMBER_BUFFER_SIZE, psize)

/* roman numeral (limited to uint16_t) */

UTILAPI const char * roman_to_uint16 (const char *s, uint16_t *number);
UTILAPI char * uint16_as_roman (uint16_t number, int uc, char ibuf[MAX_ROMAN_DIGITS], size_t *psize);
#define uint16_to_roman(number, uc, psize) uint16_as_roman(number, uc, util_number_buffer, psize)

/* double/float  to string */

UTILAPI char * double_as_string (double number, int digits, char nbuf[MAX_NUMBER_DIGITS], size_t *psize);
#define double_to_string(number, digits, psize) double_as_string(number, digits, util_number_buffer, psize)

UTILAPI char * float_as_string (float number, int digits, char nbuf[MAX_NUMBER_DIGITS], size_t *psize);
#define float_to_string(number, digits, psize) float_as_string(number, digits, util_number_buffer, psize)

/* string to double/float */

UTILAPI const char * string_to_double (const char *s, double *number);
UTILAPI const char * string_to_float  (const char *s, float *number);

/* convenience form accepting comma among a dot, with not exp notation (eg. pdf) */

UTILAPI const char * convert_to_double (const char *s, double *number);
UTILAPI const char * convert_to_float  (const char *s, float *number);

/* binary data parsers helpers */

#if 0 // masking gives more overactive warnings
#define get_number_byte1(n) ((n) & 0x000000ffu)
#define get_number_byte2(n) (((n) & 0x0000ff00u) >> 8)
#define get_number_byte3(n) (((n) & 0x00ff0000u) >> 16)
#define get_number_byte4(n) (((n) & 0xff000000u) >> 24)
#define get_number_byte5(n) (((n) & 0x000000ff00000000ull) >> 32)
#define get_number_byte6(n) (((n) & 0x0000ff0000000000ull) >> 40)
#define get_number_byte7(n) (((n) & 0x00ff000000000000ull) >> 48)
#define get_number_byte8(n) (((n) & 0xff00000000000000ull) >> 56)
#else
#define get_number_byte1(n) ((n) & 0xff)
#define get_number_byte2(n) (((n) >> 8) & 0xff)
#define get_number_byte3(n) (((n) >> 16) & 0xff)
#define get_number_byte4(n) (((n) >> 24) & 0xff)
#define get_number_byte5(n) (((n) >> 32) & 0xff)
#define get_number_byte6(n) (((n) >> 40) & 0xff)
#define get_number_byte7(n) (((n) >> 48) & 0xff)
#define get_number_byte8(n) (((n) >> 56) & 0xff)
#endif

#define get_number_bytes_be1(n, b) (b[0] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be2(n, b) (b[0] = (uint8_t)get_number_byte2(n), b[1] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be3(n, b) (b[0] = (uint8_t)get_number_byte3(n), b[1] = (uint8_t)get_number_byte2(n), b[2] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be4(n, b) (b[0] = (uint8_t)get_number_byte4(n), b[1] = (uint8_t)get_number_byte3(n), b[2] = (uint8_t)get_number_byte2(n), b[3] = (uint8_t)get_number_byte1(n))

#define get_number_bytes_be5(n, b) (b[0] = (uint8_t)get_number_byte5(n), b[1] = (uint8_t)get_number_byte4(n), b[2] = (uint8_t)get_number_byte3(n), b[3] = (uint8_t)get_number_byte2(n), \
                                    b[4] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be6(n, b) (b[0] = (uint8_t)get_number_byte6(n), b[1] = (uint8_t)get_number_byte5(n), b[2] = (uint8_t)get_number_byte4(n), b[3] = (uint8_t)get_number_byte3(n), \
                                    b[4] = (uint8_t)get_number_byte2(n), b[5] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be7(n, b) (b[0] = (uint8_t)get_number_byte7(n), b[1] = (uint8_t)get_number_byte6(n), b[2] = (uint8_t)get_number_byte5(n), b[3] = (uint8_t)get_number_byte4(n), \
                                    b[4] = (uint8_t)get_number_byte3(n), b[5] = (uint8_t)get_number_byte2(n), b[6] = (uint8_t)get_number_byte1(n))
#define get_number_bytes_be8(n, b) (b[0] = (uint8_t)get_number_byte8(n), b[1] = (uint8_t)get_number_byte7(n), b[2] = (uint8_t)get_number_byte6(n), b[3] = (uint8_t)get_number_byte5(n), \
                                    b[4] = (uint8_t)get_number_byte4(n), b[5] = (uint8_t)get_number_byte3(n), b[6] = (uint8_t)get_number_byte2(n), b[7] = (uint8_t)get_number_byte1(n))

#define read_uint16be_as(s, int_type) ((int_type)((s[0]<<8)|s[1]))
#define read_uint32be_as(s, int_type) ((int_type)((s[0]<<24)|(s[1]<<16)|(s[2]<<8)|s[3]))

#define read_uint16le_as(s, int_type) ((int_type)((s[1]<<8)|s[0]))
#define read_uint32le_as(s, int_type) ((int_type)((s[3]<<24)|(s[2]<<16)|(s[1]<<8)|s[0]))

#define read_uint16_native(s) (*((uint16_t *)(s)))
#define read_uint32_native(s) (*((uint32_t *)(s)))
#define read_int16_native(s)  (*((int16_t *)(s)))
#define read_int32_native(s)  (*((int32_t *)(s)))

#define scan_uint16be_as(s, int_type) (s += 2, (int_type)((s[-2]<<8)|s[-1]))
#define scan_uint32be_as(s, int_type) (s += 4, (int_type)((s[-4]<<24)|(s[-3]<<16)|(s[-2]<<8)|s[-1]))

#define scan_uint16le_as(s, int_type) (s += 2, (int_type)((s[-1]<<8)|s[-2]))
#define scan_uint32le_as(s, int_type) (s += 4, (int_type)((s[-1]<<24)|(s[-2]<<16)|(s[-3]<<8)|s[-4]))

#define scan_uint16_native(s) (s += 2, read_uint16_native(s-2))
#define scan_uint32_native(s) (s += 4, read_uint32_native(s-4))
#define scan_int16_native(s)  (s += 2, read_int16_native(s-2))
#define scan_int32_native(s)  (s += 4, read_int32_native(s-4))

#define read_fixed16_16_as(s, float_type)  (((float_type)read_uint32be_as(s, signed int))/(1<<16))
#define read_fixed2_14_as(s, float_type)  (((float_type)read_uint16be_as(s, signed short))/(1<<14))

#define scan_fixed16_16_as(s, float_type) (((float_type)scan_uint32be_as(s, signed int))/(1<<16))
#define scan_fixed2_14_as(s, float_type) (((float_type)scan_uint16be_as(s, signed short))/(1<<14))

/* internal procedures */

#define _scan_sign(c, sign, next) \
  do { if (c == '-') { sign = 1; c = next; } else if (c == '+') { sign = 0; c = next; } else sign = 0; } while (0)

#define integer_multiplied10(number) (((number) << 1) + ((number) << 3))

#define _scan_integer(c, number, next) \
  for (number = 0; base10_digit(c); number = integer_multiplied10(number) + (c - '0'), c = next)
#define _scan_radix(c, number, radix, next) \
  for (number = 0; (c = base36_value(c)) >= 0 && c < radix; number = number * radix + c, c = next)

#define _read_integer(c, number, next) \
  for (number = c - '0', c = next; base10_digit(c); number = integer_multiplied10(number) + (c - '0'), c = next)
#define _read_radix(c, number, radix, next) \
  for (number = c - '0', c = next; (c = base36_value(c)) >= 0 && c < radix; number = number * radix + c, c = next)

/* rationals */

#define _scan_decimal(c, number, next) \
  for (number = 0; base10_digit(c); number = number*10 + (c - '0'), c = next)
#define _scan_fraction(c, number, exponent10, next) \
  for (exponent10 = 0; base10_digit(c); --exponent10, number = number*10 + (c - '0'), c = next)

#define _scan_exponent10(c, exponent10, next) \
  do { \
    int eexponent10, eexpsign; \
    _scan_sign(c, eexpsign, next); \
    _scan_integer(c, eexponent10, next); \
    if (eexpsign) \
      exponent10 -= eexponent10; \
    else \
      exponent10 += eexponent10; \
  } while(0)

#if 0

// kept just for sentiment ;)

extern const double double_binary_power10[];
extern const float float_binary_power10[];
extern const double double_binary_negpower10[];
extern const float float_binary_negpower10[];

#define double_negative_exp10(number, exponent) \
{ const double *bp10; int e = ((exponent) < 511 ? 511 : -(exponent)); \
  for (bp10 = double_binary_negpower10; e > 0; e >>= 1, ++bp10) \
    if (e & 1) number *= *bp10; }

#define float_negative_exp10(number, exponent) \
{ const float *bp10; int e = ((exponent) < 64 ? 64 : -(exponent)); \
  for (bp10 = float_binary_negpower10; e > 0; e >>= 1, ++bp10) \
    if (e & 1) number *= *bp10; }

#define double_positive_exp10(number, exponent) \
{ const double *bp10; int e = ((exponent) > 511 ? 511 : (exponent)); \
  for (bp10 = double_binary_power10; e > 0; e >>= 1, ++bp10) \
    if (e & 1) number *= *bp10; }

#define float_positive_exp10(number, exponent) \
{ const float *bp10; int e = ((exponent) > 64 ? 64 : (exponent)); \
  for (bp10 = double_binary_power10; e > 0; e >>= 1, ++bp10) \
    if (e & 1) number *= *bp10; }

#define double_exp10(number, exponent) \
  if ((exponent) < 0) double_negative_exp10(number, exponent) else if ((exponent) > 0) double_positive_exp10(number, exponent)

#define float_exp10(number, exponent) \
  if ((exponent) < 0) float_negative_exp10(number, exponent) else if ((exponent) > 0) float_positive_exp10(number, exponent)

#else

extern const double double_decimal_power10[];
extern const float float_decimal_power10[];
extern const double double_decimal_negpower10[];
extern const float float_decimal_negpower10[];

#define double_negative_exp10(number, exponent) ((number) *= double_decimal_negpower10[(exponent) < -308 ? 308 : -(exponent)])
#define double_positive_exp10(number, exponent) ((number) *= double_decimal_power10[(exponent) > 308 ? 308 : (exponent)])

#define float_negative_exp10(number, exponent) ((number) *= float_decimal_negpower10[(exponent) < -38 ? 38 : -(exponent)])
#define float_positive_exp10(number, exponent) ((number) *= float_decimal_power10[(exponent) > 38 ? 38 : (exponent)])

#define double_exp10(number, exponent) ((void)(((exponent) < 0 && double_negative_exp10(number, exponent)) || (((exponent) > 0 && double_positive_exp10(number, exponent)))))
#define float_exp10(number, exponent) ((void)(((exponent) < 0 && float_negative_exp10(number, exponent)) || (((exponent) > 0 && float_positive_exp10(number, exponent)))))

#endif

/* pretty common stuff */

#define bytes_to_hex(input, size, output) bytes_to_hex_lc(input, size, output)
UTILAPI size_t bytes_to_hex_lc (const void *input, size_t size, uint8_t *output);
UTILAPI size_t bytes_to_hex_uc (const void *input, size_t size, uint8_t *output);
UTILAPI size_t hex_to_bytes (const void *input, size_t size, uint8_t *output);
UTILAPI void print_as_hex (const void *input, size_t bytes);

#endif