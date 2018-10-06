#ifndef UTIL_NUMBER_H
#define UTIL_NUMBER_H

#include <stddef.h> // for size_t

#include "utilplat.h"
#include "utildecl.h"

/* since 'long' isn't long for msvc64/mingw64, we need a type for machine word */

#if !defined(__cplusplus) || !defined(_MSC_VER)
#  include <stdint.h> // int*_t types are in standard in msvc++
#endif

//#if defined(MSVC64) || defined(__MINGW64__) || defined(__x86_64__) || UINTPTR_MAX > 0xffffffff
//#  define BIT64
//#else
//#  define BIT64
//#endif

#if defined(_WIN64) || defined(__MINGW32__)
#  define INT64F "%I64d"
#  define UINT64F "%I64u"
#else
#  define INT64F "%lld"
#  define UINT64F "%llu"
#endif

#if defined(MSVC64)
#  define intlw_t int64_t
#  define uintlw_t uint64_t
#  define INTLW(N) N##I64
#  define UINTLW(N) N##UI64
#  define INTLWF INT64F
#  define UINTLWF UINT64F
#elif defined(__MINGW64__)
#  define intlw_t int64_t
#  define uintlw_t uint64_t
#  define INTLW(N) N##LL
#  define UINTLW(N) N##ULL
#  define INTLWF INT64F
#  define UINTLWF UINT64F
#else // 32bit or sane 64bit (LP64)
#  define intlw_t long
#  define uintlw_t size_t /*unsigned long*/
#  define INTLW(N) N##L
#  define UINTLW(N) N##UL
#  define INTLWF "%ld"
#  define UINTLWF "%lu"
#endif

/* basic constants */

#define MAX_RADIX 36
// #define MAX_INTEGER_DIGITS 65 /* 64-bit number in binary form plus '\0' */
#define MAX_INTEGER_DIGITS 128 // to handle romannumeral of short int

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
#define base10_value(c) (base10_lookup[(uint8_t)c])              

#define base16_digit(c) (base16_lookup[(uint8_t)c] >= 0)
#define base16_value(c) (base16_lookup[(uint8_t)c])

#define base26_digit(c) (base26_lookup[(uint8_t)c] >= 0)
#define base26_value(c) (base26_lookup[(uint8_t)c])

#define base36_digit(c) (base36_lookup[(uint8_t)c] >= 0)
#define base36_value(c) (base36_lookup[(uint8_t)c])

//#define base_digit(c, radix) ((unsigned)(base36_lookup[c]) < (unsigned)(radix))
//#define base_value(c, radix) (base_digit(c, radix) ? base36_lookup[c] : -1)

/* integer from string; return a pointer to character next to the last digit */

UTILAPI const char * string_to_int32 (const char *s, int32_t *number);
UTILAPI const char * string_to_intlw (const char *s, intlw_t *number);
UTILAPI const char * string_to_int64 (const char *s, int64_t *number);

UTILAPI const char * string_to_uint32 (const char *s, uint32_t *number);
UTILAPI const char * string_to_uintlw (const char *s, uintlw_t *number);
UTILAPI const char * string_to_uint64 (const char *s, uint64_t *number);

UTILAPI const char * radix_to_int32 (const char *s, int32_t *number, int radix);
UTILAPI const char * radix_to_intlw (const char *s, intlw_t *number, int radix);
UTILAPI const char * radix_to_int64 (const char *s, int64_t *number, int radix);

UTILAPI const char * radix_to_uint32 (const char *s, uint32_t *number, int radix);
UTILAPI const char * radix_to_uintlw (const char *s, uintlw_t *number, int radix);
UTILAPI const char * radix_to_uint64 (const char *s, uint64_t *number, int radix);

UTILAPI const char * roman_to_uint16 (const char *s, uint16_t *number);

UTILAPI const char * alpha_to_uint32 (const char *s, uint32_t *number);
UTILAPI const char * alpha_to_uintlw (const char *s, uintlw_t *number);
UTILAPI const char * alpha_to_uint64 (const char *s, uint64_t *number);

UTILAPI const char * alphan_to_uint32 (const char *s, uint32_t *number);
UTILAPI const char * alphan_to_uintlw (const char *s, uintlw_t *number);
UTILAPI const char * alphan_to_uint64 (const char *s, uintlw_t *number);

/*
integer to string; return a pointer to null-terminated static const string
same but also stores pointer to trailing null (to be used for firther formatting)
*/

UTILAPI char * int32_as_string (int32_t number, char **e);
UTILAPI char * intlw_as_string (intlw_t number, char **e);
UTILAPI char * int64_as_string (int64_t number, char **e);

UTILAPI char * uint32_as_string (uint32_t number, char **e);
UTILAPI char * uintlw_as_string (uintlw_t number, char **e);
UTILAPI char * uint64_as_string (uint64_t number, char **e);

#define int32_to_string(number) int32_as_string(number, NULL)
#define intlw_to_string(number) intlw_as_string(number, NULL)
#define int64_to_string(number) int64_as_string(number, NULL)

#define uint32_to_string(number) uint32_as_string(number, NULL)
#define uintlw_to_string(number) uintlw_as_string(number, NULL)
#define uint64_to_string(number) uint64_as_string(number, NULL)

UTILAPI char * int32_as_radix (int32_t number, int radix, char **e);
UTILAPI char * intlw_as_radix (intlw_t number, int radix, char **e);
UTILAPI char * int64_as_radix (int64_t number, int radix, char **e);

UTILAPI char * uint32_as_radix (uint32_t number, int radix, char **e);
UTILAPI char * uintlw_as_radix (uintlw_t number, int radix, char **e);
UTILAPI char * uint64_as_radix (uint64_t number, int radix, char **e);

#define int32_to_radix(number, radix) int32_as_radix(number, radix, NULL)
#define intlw_to_radix(number, radix) intlw_as_radix(number, radix, NULL)
#define int64_to_radix(number, radix) int64_as_radix(number, radix, NULL)

#define uint32_to_radix(number, radix) uint32_as_radix(number, radix, NULL)
#define uintlw_to_radix(number, radix) uintlw_as_radix(number, radix, NULL)
#define uint64_to_radix(number, radix) uint64_as_radix(number, radix, NULL)

UTILAPI char * uint32_as_alpha_uc (uint32_t number, char **e);
UTILAPI char * uint32_as_alpha_lc (uint32_t number, char **e);
UTILAPI char * uintlw_as_alpha_uc (uintlw_t number, char **e);
UTILAPI char * uintlw_as_alpha_lc (uintlw_t number, char **e);
UTILAPI char * uint64_as_alpha_uc (uint64_t number, char **e);
UTILAPI char * uint64_as_alpha_lc (uint64_t number, char **e);

#define uint32_to_alpha_uc(number) uint32_as_alpha_uc(number, NULL)
#define uint32_to_alpha_lc(number) uint32_as_alpha_lc(number, NULL)
#define uintlw_to_alpha_uc(number) uintlw_as_alpha_uc(number, NULL)
#define uintlw_to_alpha_lc(number) uintlw_as_alpha_lc(number, NULL)
#define uint64_to_alpha_uc(number) uint64_as_alpha_uc(number, NULL)
#define uint64_to_alpha_lc(number) uint64_as_alpha_lc(number, NULL)

UTILAPI char * uint32_as_alphan_uc (uint32_t number, char **e);
UTILAPI char * uint32_as_alphan_lc (uint32_t number, char **e);
UTILAPI char * uintlw_as_alphan_uc (uintlw_t number, char **e);
UTILAPI char * uintlw_as_alphan_lc (uintlw_t number, char **e);
UTILAPI char * uint64_as_alphan_uc (uint64_t number, char **e);
UTILAPI char * uint64_as_alphan_lc (uint64_t number, char **e);

#define uint32_to_alphan_uc(number) uint32_as_alpha_uc(number, NULL)
#define uint32_to_alphan_lc(number) uint32_as_alpha_lc(number, NULL)
#define uintlw_to_alphan_uc(number) uintlw_as_alpha_uc(number, NULL)
#define uintlw_to_alphan_lc(number) uintlw_as_alpha_lc(number, NULL)
#define uint64_to_alphan_uc(number) uint64_as_alpha_uc(number, NULL)
#define uint64_to_alphan_lc(number) uint64_as_alpha_lc(number, NULL)

/* roman numeral (limited to uint16_t) */

UTILAPI char * uint16_as_roman_uc (uint16_t number, char **e);
UTILAPI char * uint16_as_roman_lc (uint16_t number, char **e);

#define uint16_to_roman_uc(number) uint16_as_roman_uc(number, NULL)
#define uint16_to_roman_lc(number) uint16_as_roman_lc(number, NULL)

#define uint16_as_roman(number) uint16_as_roman_uc(number)
#define uint16_to_roman(number) uint16_to_roman_uc(number)

/* double/float  to string */

UTILAPI char * double_to_string (double number, int digits);
UTILAPI char * float_to_string (float number, int digits);

UTILAPI char * double_as_string (double number, int digits, char **r, char **e);
UTILAPI char * float_as_string (float number, int digits, char **r, char **e);

/* string to double/float */

UTILAPI const char * string_to_double (const char *s, double *number);
UTILAPI const char * string_to_float  (const char *s, float *number);

/* convenience form accepting comma among a dot, with not exp notation (eg. pdf) */

UTILAPI const char * convert_to_double (const char *s, double *number);
UTILAPI const char * convert_to_float  (const char *s, float *number);

/* binary data parsers helpers */

#define get_byte1(i) ((i)&255)
#define get_byte2(i) (((i)>>8)&255)
#define get_byte3(i) (((i)>>16)&255)
#define get_byte4(i) (((i)>>24)&255)

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