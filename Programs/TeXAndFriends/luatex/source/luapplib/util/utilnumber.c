
#include <math.h> /* for log10() and floor() */
#include <stdio.h> /* for printf() */

#include "utilnumber.h"

const int base10_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

const int base16_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

const int base26_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,-1,-1,-1,-1,-1,
  -1, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

const int base36_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
  -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
  25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

/* integer from string; return a pointer to character next to the last digit */

#define string_scan_sign(s, c, sign) _scan_sign(c, sign, *++s)
#define string_scan_integer(s, c, number) _scan_integer(c, number, *++s)
#define string_scan_radix(s, c, number, radix) _scan_radix(c, number, radix, *++s)
#define string_read_integer(s, c, number) _read_integer(c, number, *++s)
#define string_read_radix(s, c, number, radix) _read_radix(c, number, radix, *++s)

const char * string_to_int32 (const char *s, int32_t *number)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_integer(s, c, *number);
  if (sign) *number = -*number;
  return s;
}

const char * string_to_intlw (const char *s, intlw_t *number)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_integer(s, c, *number);
  if (sign) *number = -*number;
  return s;
}

const char * string_to_int64 (const char *s, int64_t *number)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_integer(s, c, *number);
  if (sign) *number = -*number;
  return s;
}

const char * string_to_uint32 (const char *s, uint32_t *number)
{
  int c = *s;
  string_scan_integer(s, c, *number);
  return s;
}

const char * string_to_uintlw (const char *s, uintlw_t *number)
{
  int c = *s;
  string_scan_integer(s, c, *number);
  return s;
}

const char * string_to_uint64 (const char *s, uint64_t *number)
{
  int c = *s;
  string_scan_integer(s, c, *number);
  return s;
}

const char * radix_to_int32 (const char *s, int32_t *number, int radix)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_radix(s, c, *number, radix);
  if (sign) *number = -*number;
  return s;
}

const char * radix_to_intlw (const char *s, intlw_t *number, int radix)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_radix(s, c, *number, radix);
  if (sign) *number = -*number;
  return s;
}

const char * radix_to_int64 (const char *s, int64_t *number, int radix)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_radix(s, c, *number, radix);
  if (sign) *number = -*number;
  return s;
}

const char * radix_to_uint32 (const char *s, uint32_t *number, int radix)
{
  int c = *s;
  string_scan_radix(s, c, *number, radix);
  return s;
}

const char * radix_to_uintlw (const char *s, uintlw_t *number, int radix)
{
  int c = *s;
  string_scan_radix(s, c, *number, radix);
  return s;
}

const char * radix_to_uint64 (const char *s, uint64_t *number, int radix)
{
  int c = *s;
  string_scan_radix(s, c, *number, radix);
  return s;
}

/* roman to uint16_t */

#define roman1000(c) (c == 'M' || c == 'm')
#define roman500(c)  (c == 'D' || c == 'd')
#define roman100(c)  (c == 'C' || c == 'c')
#define roman50(c)   (c == 'L' || c == 'l')
#define roman10(c)   (c == 'X' || c == 'x')
#define roman5(c)    (c == 'V' || c == 'v')
#define roman1(c)    (c == 'I' || c == 'i')

#define roman100s(p) (roman100(*p) ? (100 + ((++p, roman100(*p)) ? (100 + ((++p, roman100(*p)) ? (++p, 100) : 0)) : 0)) : 0)
#define roman10s(p) (roman10(*p) ? (10 + ((++p, roman10(*p)) ? (10 + ((++p, roman10(*p)) ? (++p, 10) : 0)) : 0)) : 0)
#define roman1s(p) (roman1(*p) ? (1 + ((++p, roman1(*p)) ? (1 + ((++p, roman1(*p)) ? (++p, 1) : 0)) : 0)) : 0)

const char * roman_to_uint16 (const char *s, uint16_t *number)
{
  const char *p;
  /* M */
  for (*number = 0, p = s; roman1000(*p); *number += 1000, ++p);
  /* D C */
  if (roman500(*p))
  {
    ++p;
    *number += 500 + roman100s(p);
  }
  else if (roman100(*p))
  {
    ++p;
    if (roman1000(*p))
    {
      ++p;
      *number += 900;
    }
    else if (roman500(*p))
    {
      ++p;
      *number += 400;
    }
    else
      *number += 100 + roman100s(p);
  }
  /* L X */
  if (roman50(*p))
  {
    ++p;
    *number += 50 + roman10s(p);
  }
  else if (roman10(*p))
  {
    ++p;
    if (roman100(*p))
    {
      ++p;
      *number += 90;
    }
    else if (roman50(*p))
    {
      ++p;
      *number += 40;
    }
    else
      *number += 10 + roman10s(p);
  }
  /* V I */
  if (roman5(*p))
  {
    ++p;
    *number += 5 + roman1s(p);
  }
  else if (roman1(*p))
  {
    ++p;
    if (roman10(*p))
    {
      ++p;
      *number += 9;
    }
    else if (roman5(*p))
    {
      ++p;
      *number += 4;
    }
    else
      *number += 1 + roman1s(p);
  }
  return p;
}

/* integer to string; return a pointer to null-terminated static const string */

static char integer_buffer[MAX_INTEGER_DIGITS] = {'\0'};
#define end_of_integer_buffer (integer_buffer + MAX_INTEGER_DIGITS - 1)

/* writing integers */

#define number_printrev_signed(p, number, quotient) \
  do { \
    quotient = number; number /= 10; \
    *--p = base10_palindrome[9 + (quotient - number*10)]; \
  } while (number); \
  if (quotient < 0) *--p = '-'

#define number_printrev_unsigned(p, number, quotient) \
  do { \
    quotient = number; number /= 10; \
    *--p = (char)(quotient - integer_multiplied10(number)) + '0'; \
  } while (number)

char * int32_as_string (int32_t number, char **e)
{
  char *p;
  int quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed(p, number, quotient);
  return p;
}

char * intlw_as_string (intlw_t number, char **e)
{
  char *p;
  intlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed(p, number, quotient);
  return p;
}

char * int64_as_string (int64_t number, char **e)
{
  char *p;
  int64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed(p, number, quotient);
  return p;
}

char * uint32_as_string (uint32_t number, char **e)
{
  char *p;
  uint32_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned(p, number, quotient);
  return p;
}

char * uintlw_as_string (uintlw_t number, char **e)
{
  char *p;
  uintlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned(p, number, quotient);
  return p;
}

char * uint64_as_string (uint64_t number, char **e)
{
  char *p;
  uint64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned(p, number, quotient);
  return p;
}

/* radix variant */

#define number_printrev_signed_radix_uc(p, number, radix, quotient) \
  do { \
    quotient = number; number /= radix; \
    *--p = base36_uc_palindrome[MAX_RADIX - 1 + (quotient - number*radix)]; \
  } while (number)

#define number_printrev_signed_radix_lc(p, number, radix, quotient) \
  do { \
    quotient = number; number /= radix; \
    *--p = base36_lc_palindrome[MAX_RADIX - 1 + (quotient - number*radix)]; \
  } while (number)

#define number_printrev_signed_radix(p, number, radix, quotient) \
  do { \
    if (radix > 0) { number_printrev_signed_radix_uc(p, number, radix, quotient); } \
    else { radix = -radix; number_printrev_signed_radix_lc(p, number, radix, quotient); } \
    if (quotient < 0) *--p = '-'; \
  } while (0)

#define number_printrev_unsigned_radix_uc(p, number, radix, quotient) \
  do { \
    quotient = number; number /= radix; \
    *--p = base36_uc_alphabet[quotient % radix]; \
  } while (number)

#define number_printrev_unsigned_radix_lc(p, number, radix, quotient) \
  do { \
    quotient = number; number /= radix; \
    *--p = base36_lc_alphabet[quotient % radix]; \
  } while (number)

#define number_printrev_unsigned_radix(p, number, radix, quotient) \
  do { \
    if (radix > 0) { number_printrev_unsigned_radix_uc(p, number, radix, quotient); } \
    else { radix = -radix; number_printrev_unsigned_radix_lc(p, number, radix, quotient); } \
  } while (0)

char * int32_as_radix (int number, int radix, char **e)
{
  char *p;
  int quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed_radix(p, number, radix, quotient);
  return p;
}

char * intlw_as_radix (intlw_t number, int radix, char **e)
{
  char *p;
  intlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed_radix(p, number, radix, quotient);
  return p;
}

char * int64_as_radix (int64_t number, int radix, char **e)
{
  char *p;
  int64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_signed_radix(p, number, radix, quotient);
  return p;
}

char * uint32_as_radix (uint32_t number, int radix, char **e)
{
  char *p;
  uint32_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_radix(p, number, radix, quotient);
  return p;
}

char * uintlw_as_radix (uintlw_t number, int radix, char **e)
{
  char *p;
  uintlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_radix(p, number, radix, quotient);
  return p;
}

char * uint64_as_radix (uint64_t number, int radix, char **e)
{
  char *p;
  uint64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_radix(p, number, radix, quotient);
  return p;
}

/* aaa, aab, aac, ...; unsigned only. 0 gives empty string */

#define string_scan_alpha(s, c, number, radix) \
  for (number = 0, c = *s; (c = base26_value(c)) > 0; number = number * radix + c, c = *++s)

const char * alpha_to_uint32 (const char *s, uint32_t *number)
{
  int c;
  string_scan_alpha(s, c, *number, 26);
  return s;
}

const char * alpha_to_uintlw (const char *s, uintlw_t *number)
{
  int c;
  string_scan_alpha(s, c, *number, 26);
  return s;
}

const char * alpha_to_uint64 (const char *s, uint64_t *number)
{
  int c;
  string_scan_alpha(s, c, *number, 26);
  return s;
}

#define number_printrev_unsigned_alpha_uc(p, number, radix, quotient) \
  while (number > 0) { \
    quotient = --number; number /= radix; \
    *--p = base26_uc_alphabet[quotient % radix]; \
  }

#define number_printrev_unsigned_alpha_lc(p, number, radix, quotient) \
  while (number > 0) { \
    quotient = --number; number /= radix; \
    *--p = base26_lc_alphabet[quotient % radix]; \
  }

char * uint32_as_alpha_uc (uint32_t number, char **e)
{
  char *p;
  uint32_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_uc(p, number, 26, quotient);
  return p;
}

char * uint32_as_alpha_lc (uint32_t number, char **e)
{
  char *p;
  uint32_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_lc(p, number, 26, quotient);
  return p;
}

char * uintlw_as_alpha_uc (uintlw_t number, char **e)
{
  char *p;
  uintlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_uc(p, number, 26, quotient);
  return p;
}

char * uintlw_as_alpha_lc (uintlw_t number, char **e)
{
  char *p;
  uintlw_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_lc(p, number, 26, quotient);
  return p;
}

char * uint64_as_alpha_uc (uint64_t number, char **e)
{
  char *p;
  uint64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_uc(p, number, 26, quotient);
  return p;
}

char * uint64_as_alpha_lc (uint64_t number, char **e)
{
  char *p;
  uint64_t quotient;
  p = end_of_integer_buffer; *p = '\0';
  if (e != NULL) *e = p;
  number_printrev_unsigned_alpha_lc(p, number, 26, quotient);
  return p;
}

/* a variant of alphabetic, a, b, c, ..., z, aa, bb, cc, ..., zz (eg. pdf page labelling) */

#define string_scan_alphan(s, c, number, radix) \
  do { \
    number = 0; \
    if ((c = base26_value(*s)) > 0) { \
      number = c; \
      while (c == base26_value(*++s)) number += radix; \
    }  \
  } while (0)

const char * alphan_to_uint32 (const char *s, uint32_t *number)
{
  int c;
  string_scan_alphan(s, c, *number, 26);
  return s;
}

const char * alphan_to_uintlw (const char *s, uintlw_t *number)
{
  int c;
  string_scan_alphan(s, c, *number, 26);
  return s;
}

const char * alphan_to_uint64 (const char *s, uintlw_t *number)
{
  int c;
  string_scan_alphan(s, c, *number, 26);
  return s;
}

#define number_print_alphan_uc(s, c, number, radix) \
  if (number > 0) { \
    for (c = (--number) % radix, number -= c; ; number -= radix) { \
      *s++ = base26_uc_alphabet[c]; \
       if (number == 0 || p >= end_of_integer_buffer) break; \
    } \
  }

#define number_print_alphan_lc(s, c, number, radix) \
  if (number > 0) { \
    for (c = (--number) % radix, number -= c; ; number -= radix) { \
      *s++ = base26_lc_alphabet[c]; \
       if (number == 0 || p >= end_of_integer_buffer) break; \
    } \
  }

char * uint32_as_alphan_uc (uint32_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_uc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

char * uint32_as_alphan_lc (uint32_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_lc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

char * uintlw_as_alphan_uc (uintlw_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_uc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

char * uintlw_as_alphan_lc (uintlw_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_lc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

char * uint64_as_alphan_uc (uint64_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_uc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

char * uint64_as_alphan_lc (uint64_t number, char **e)
{
  char *p;
  uint8_t c;
  p = integer_buffer;
  number_print_alphan_lc(p, c, number, 26);
  *p = '\0'; if (e != NULL) *e = p;
  return integer_buffer;
}

/* roman numeral */

/* todo: large roman numerals? http://mathforum.org/library/drmath/view/57569.html */

#define base_roman_uc_alphabet "MDCLXVI"
#define base_roman_lc_alphabet "mdclxvi"

static const uint32_t base_roman_values[] = { 1000, 500, 100, 50, 10, 5, 1 };

#define integer_to_roman(p, number, alphabet) \
  { \
    uint32_t k, j, v, u; \
    for (j = 0, v = base_roman_values[0]; number > 0; ) \
    { \
      if (number >= v) \
      { \
       *p++ = alphabet[j]; \
       number -= v; \
       continue; \
      } \
      if (j & 1) \
        k = j + 1; \
      else \
        k = j + 2; \
      u = base_roman_values[k]; \
      if (number + u >= v) \
      { \
        *p++ = alphabet[k]; \
        number += u; \
      } \
      else \
        v = base_roman_values[++j]; \
    } \
  }

char * uint16_as_roman_uc (uint16_t number, char **e)
{
  char *p = integer_buffer;
  integer_to_roman(p, number, base_roman_uc_alphabet);
  if (e != NULL)
    *e = p;
  *p = '\0';
  return integer_buffer;
}

char * uint16_as_roman_lc (uint16_t number, char **e)
{
  char *p = integer_buffer;
  integer_to_roman(p, number, base_roman_lc_alphabet);
  if (e != NULL)
    *e = p;
  *p = '\0';
  return integer_buffer;
}

/* IEEE-754 */

#define BINARY_MODF    1

#define NOT_A_NUMBER_STRING "NaN"
#define INFINITY_STRING "INF"
#define SIGNED_INFINITY 1
#define SIGNED_ZERO 0
#define SIGNED_NOT_A_NUMBER 0
#define RADIX_CHAR '.'

/* double/float to decimal */

typedef struct ieee_double {
  union {
    double number;
    uint64_t bits;
  };
  uint64_t fraction;
  int exponent, sign;
} ieee_double;

typedef struct ieee_float {
  union {
    float number;
    uint32_t bits;
  };
  uint32_t fraction;
  int exponent, sign;
} ieee_float;

#define IEEE_DOUBLE_BIAS           1023
#define IEEE_DOUBLE_MIN_EXPONENT  -1023
#define IEEE_DOUBLE_MAX_EXPONENT  (0x7ff - IEEE_DOUBLE_BIAS)

#define IEEE_FLOAT_BIAS            127
#define IEEE_FLOAT_MIN_EXPONENT   -127
#define IEEE_FLOAT_MAX_EXPONENT   (0xff - IEEE_FLOAT_BIAS)

#define ieee_double_fraction(i) (i & 0x000fffffffffffffull)
#define ieee_double_exponent(i) ((0x7ff & (i >> 52)) - IEEE_DOUBLE_BIAS)
#define ieee_double_sign(ieee_number) ((void)((ieee_number.sign = ieee_number.bits >> 63) && (ieee_number.number = -ieee_number.number)))
#define ieee_double_init(ieee_number, number) \
  ieee_number.number = number, \
  ieee_number.fraction = ieee_double_fraction(ieee_number.bits), \
  ieee_number.exponent = ieee_double_exponent(ieee_number.bits)

#define ieee_float_fraction(i) (i & 0x007fffff)
#define ieee_float_exponent(i) ((0xff & (i >> 23)) - IEEE_FLOAT_BIAS)
#define ieee_float_sign(ieee_number) ((void)((ieee_number.sign = ieee_number.bits >> 31) && (ieee_number.number = -ieee_number.number)))
#define ieee_float_init(ieee_number, number) \
  ieee_number.number = number, \
  ieee_number.fraction = ieee_float_fraction(ieee_number.bits), \
  ieee_number.exponent = ieee_float_exponent(ieee_number.bits)

/* special cases  */

#define ieee_double_is_zero(ieee_number) (ieee_number.number == 0) // || ieee_double_too_small(ieee_number) ?
#define ieee_double_too_small(ieee_number) (ieee_number.exponent == 0 && ieee_number.fraction != 0) // denormalized, implicit fracion bit not set

#define ieee_float_is_zero(ieee_number) (ieee_number.number == 0) // || ieee_float_too_small(ieee_number) ?
#define ieee_float_too_small(ieee_number) (ieee_number.exponent == 0 && ieee_number.fraction != 0)

#define ieee_double_zero_string(ieee_number) (SIGNED_ZERO && ieee_number.sign ? "-0" : "0")
#define ieee_double_infinity_string(ieee_number) (SIGNED_INFINITY && ieee_number.sign ? "-" INFINITY_STRING : INFINITY_STRING)

#define ieee_float_zero_string ieee_double_zero_string
#define ieee_float_infinity_string ieee_double_infinity_string

#define ieee_double_special_case(ieee_number) (ieee_number.exponent == IEEE_DOUBLE_MAX_EXPONENT)
#define ieee_double_special_string(ieee_number) (ieee_number.fraction ? NOT_A_NUMBER_STRING : ieee_double_infinity_string(ieee_number))

#define ieee_float_special_case(ieee_number) (ieee_number.exponent == IEEE_FLOAT_MAX_EXPONENT)
#define ieee_float_special_string(ieee_number) (ieee_number.fraction ? NOT_A_NUMBER_STRING : ieee_float_infinity_string(ieee_number))

#if 0

const double double_binary_power10[] =
{
  1.0e1, 1.0e2, 1.0e4, 1.0e8, 1.0e16, 1.0e32, 1.0e64, 1.0e128, 1.0e256
};

const float float_binary_power10[] =
{
  1.0e1, 1.0e2, 1.0e4, 1.0e8, 1.0e16, 1.0e32
};

const double double_binary_negpower10[] =
{
  1.0e-1, 1.0e-2, 1.0e-4, 1.0e-8, 1.0e-16, 1.0e-32
};

const float float_binary_negpower10[] =
{
  1.0e-1, 1.0e-2, 1.0e-4, 1.0e-8, 1.0e-16, 1.0e-32
};

#else

const double double_decimal_power10[] = {
    1.0e0,   1.0e1,   1.0e2,   1.0e3,   1.0e4,   1.0e5,   1.0e6,   1.0e7,   1.0e8,   1.0e9,
   1.0e10,  1.0e11,  1.0e12,  1.0e13,  1.0e14,  1.0e15,  1.0e16,  1.0e17,  1.0e18,  1.0e19,
   1.0e20,  1.0e21,  1.0e22,  1.0e23,  1.0e24,  1.0e25,  1.0e26,  1.0e27,  1.0e28,  1.0e29,
   1.0e30,  1.0e31,  1.0e32,  1.0e33,  1.0e34,  1.0e35,  1.0e36,  1.0e37,  1.0e38,  1.0e39,
   1.0e40,  1.0e41,  1.0e42,  1.0e43,  1.0e44,  1.0e45,  1.0e46,  1.0e47,  1.0e48,  1.0e49,
   1.0e50,  1.0e51,  1.0e52,  1.0e53,  1.0e54,  1.0e55,  1.0e56,  1.0e57,  1.0e58,  1.0e59,
   1.0e60,  1.0e61,  1.0e62,  1.0e63,  1.0e64,  1.0e65,  1.0e66,  1.0e67,  1.0e68,  1.0e69,
   1.0e70,  1.0e71,  1.0e72,  1.0e73,  1.0e74,  1.0e75,  1.0e76,  1.0e77,  1.0e78,  1.0e79,
   1.0e80,  1.0e81,  1.0e82,  1.0e83,  1.0e84,  1.0e85,  1.0e86,  1.0e87,  1.0e88,  1.0e89,
   1.0e90,  1.0e91,  1.0e92,  1.0e93,  1.0e94,  1.0e95,  1.0e96,  1.0e97,  1.0e98,  1.0e99,
  1.0e100, 1.0e101, 1.0e102, 1.0e103, 1.0e104, 1.0e105, 1.0e106, 1.0e107, 1.0e108, 1.0e109,
  1.0e110, 1.0e111, 1.0e112, 1.0e113, 1.0e114, 1.0e115, 1.0e116, 1.0e117, 1.0e118, 1.0e119,
  1.0e120, 1.0e121, 1.0e122, 1.0e123, 1.0e124, 1.0e125, 1.0e126, 1.0e127, 1.0e128, 1.0e129,
  1.0e130, 1.0e131, 1.0e132, 1.0e133, 1.0e134, 1.0e135, 1.0e136, 1.0e137, 1.0e138, 1.0e139,
  1.0e140, 1.0e141, 1.0e142, 1.0e143, 1.0e144, 1.0e145, 1.0e146, 1.0e147, 1.0e148, 1.0e149,
  1.0e150, 1.0e151, 1.0e152, 1.0e153, 1.0e154, 1.0e155, 1.0e156, 1.0e157, 1.0e158, 1.0e159,
  1.0e160, 1.0e161, 1.0e162, 1.0e163, 1.0e164, 1.0e165, 1.0e166, 1.0e167, 1.0e168, 1.0e169,
  1.0e170, 1.0e171, 1.0e172, 1.0e173, 1.0e174, 1.0e175, 1.0e176, 1.0e177, 1.0e178, 1.0e179,
  1.0e180, 1.0e181, 1.0e182, 1.0e183, 1.0e184, 1.0e185, 1.0e186, 1.0e187, 1.0e188, 1.0e189,
  1.0e190, 1.0e191, 1.0e192, 1.0e193, 1.0e194, 1.0e195, 1.0e196, 1.0e197, 1.0e198, 1.0e199,
  1.0e200, 1.0e201, 1.0e202, 1.0e203, 1.0e204, 1.0e205, 1.0e206, 1.0e207, 1.0e208, 1.0e209,
  1.0e210, 1.0e211, 1.0e212, 1.0e213, 1.0e214, 1.0e215, 1.0e216, 1.0e217, 1.0e218, 1.0e219,
  1.0e220, 1.0e221, 1.0e222, 1.0e223, 1.0e224, 1.0e225, 1.0e226, 1.0e227, 1.0e228, 1.0e229,
  1.0e230, 1.0e231, 1.0e232, 1.0e233, 1.0e234, 1.0e235, 1.0e236, 1.0e237, 1.0e238, 1.0e239,
  1.0e240, 1.0e241, 1.0e242, 1.0e243, 1.0e244, 1.0e245, 1.0e246, 1.0e247, 1.0e248, 1.0e249,
  1.0e250, 1.0e251, 1.0e252, 1.0e253, 1.0e254, 1.0e255, 1.0e256, 1.0e257, 1.0e258, 1.0e259,
  1.0e260, 1.0e261, 1.0e262, 1.0e263, 1.0e264, 1.0e265, 1.0e266, 1.0e267, 1.0e268, 1.0e269,
  1.0e270, 1.0e271, 1.0e272, 1.0e273, 1.0e274, 1.0e275, 1.0e276, 1.0e277, 1.0e278, 1.0e279,
  1.0e280, 1.0e281, 1.0e282, 1.0e283, 1.0e284, 1.0e285, 1.0e286, 1.0e287, 1.0e288, 1.0e289,
  1.0e290, 1.0e291, 1.0e292, 1.0e293, 1.0e294, 1.0e295, 1.0e296, 1.0e297, 1.0e298, 1.0e299,
  1.0e300, 1.0e301, 1.0e302, 1.0e303, 1.0e304, 1.0e305, 1.0e306, 1.0e307, 1.0e308
};

const float float_decimal_power10[] = {
    1.0e0f,   1.0e1f,   1.0e2f,   1.0e3f,   1.0e4f,   1.0e5f,   1.0e6f,   1.0e7f,   1.0e8f,   1.0e9f,
   1.0e10f,  1.0e11f,  1.0e12f,  1.0e13f,  1.0e14f,  1.0e15f,  1.0e16f,  1.0e17f,  1.0e18f,  1.0e19f,
   1.0e20f,  1.0e21f,  1.0e22f,  1.0e23f,  1.0e24f,  1.0e25f,  1.0e26f,  1.0e27f,  1.0e28f,  1.0e29f,
   1.0e30f,  1.0e31f,  1.0e32f,  1.0e33f,  1.0e34f,  1.0e35f,  1.0e36f,  1.0e37f,  1.0e38f
};

const double double_decimal_negpower10[] = {
    1.0e0,   1.0e-1,   1.0e-2,   1.0e-3,   1.0e-4,   1.0e-5,   1.0e-6,   1.0e-7,   1.0e-8,   1.0e-9,
   1.0e-10,  1.0e-11,  1.0e-12,  1.0e-13,  1.0e-14,  1.0e-15,  1.0e-16,  1.0e-17,  1.0e-18,  1.0e-19,
   1.0e-20,  1.0e-21,  1.0e-22,  1.0e-23,  1.0e-24,  1.0e-25,  1.0e-26,  1.0e-27,  1.0e-28,  1.0e-29,
   1.0e-30,  1.0e-31,  1.0e-32,  1.0e-33,  1.0e-34,  1.0e-35,  1.0e-36,  1.0e-37,  1.0e-38,  1.0e-39,
   1.0e-40,  1.0e-41,  1.0e-42,  1.0e-43,  1.0e-44,  1.0e-45,  1.0e-46,  1.0e-47,  1.0e-48,  1.0e-49,
   1.0e-50,  1.0e-51,  1.0e-52,  1.0e-53,  1.0e-54,  1.0e-55,  1.0e-56,  1.0e-57,  1.0e-58,  1.0e-59,
   1.0e-60,  1.0e-61,  1.0e-62,  1.0e-63,  1.0e-64,  1.0e-65,  1.0e-66,  1.0e-67,  1.0e-68,  1.0e-69,
   1.0e-70,  1.0e-71,  1.0e-72,  1.0e-73,  1.0e-74,  1.0e-75,  1.0e-76,  1.0e-77,  1.0e-78,  1.0e-79,
   1.0e-80,  1.0e-81,  1.0e-82,  1.0e-83,  1.0e-84,  1.0e-85,  1.0e-86,  1.0e-87,  1.0e-88,  1.0e-89,
   1.0e-90,  1.0e-91,  1.0e-92,  1.0e-93,  1.0e-94,  1.0e-95,  1.0e-96,  1.0e-97,  1.0e-98,  1.0e-99,
  1.0e-100, 1.0e-101, 1.0e-102, 1.0e-103, 1.0e-104, 1.0e-105, 1.0e-106, 1.0e-107, 1.0e-108, 1.0e-109,
  1.0e-110, 1.0e-111, 1.0e-112, 1.0e-113, 1.0e-114, 1.0e-115, 1.0e-116, 1.0e-117, 1.0e-118, 1.0e-119,
  1.0e-120, 1.0e-121, 1.0e-122, 1.0e-123, 1.0e-124, 1.0e-125, 1.0e-126, 1.0e-127, 1.0e-128, 1.0e-129,
  1.0e-130, 1.0e-131, 1.0e-132, 1.0e-133, 1.0e-134, 1.0e-135, 1.0e-136, 1.0e-137, 1.0e-138, 1.0e-139,
  1.0e-140, 1.0e-141, 1.0e-142, 1.0e-143, 1.0e-144, 1.0e-145, 1.0e-146, 1.0e-147, 1.0e-148, 1.0e-149,
  1.0e-150, 1.0e-151, 1.0e-152, 1.0e-153, 1.0e-154, 1.0e-155, 1.0e-156, 1.0e-157, 1.0e-158, 1.0e-159,
  1.0e-160, 1.0e-161, 1.0e-162, 1.0e-163, 1.0e-164, 1.0e-165, 1.0e-166, 1.0e-167, 1.0e-168, 1.0e-169,
  1.0e-170, 1.0e-171, 1.0e-172, 1.0e-173, 1.0e-174, 1.0e-175, 1.0e-176, 1.0e-177, 1.0e-178, 1.0e-179,
  1.0e-180, 1.0e-181, 1.0e-182, 1.0e-183, 1.0e-184, 1.0e-185, 1.0e-186, 1.0e-187, 1.0e-188, 1.0e-189,
  1.0e-190, 1.0e-191, 1.0e-192, 1.0e-193, 1.0e-194, 1.0e-195, 1.0e-196, 1.0e-197, 1.0e-198, 1.0e-199,
  1.0e-200, 1.0e-201, 1.0e-202, 1.0e-203, 1.0e-204, 1.0e-205, 1.0e-206, 1.0e-207, 1.0e-208, 1.0e-209,
  1.0e-210, 1.0e-211, 1.0e-212, 1.0e-213, 1.0e-214, 1.0e-215, 1.0e-216, 1.0e-217, 1.0e-218, 1.0e-219,
  1.0e-220, 1.0e-221, 1.0e-222, 1.0e-223, 1.0e-224, 1.0e-225, 1.0e-226, 1.0e-227, 1.0e-228, 1.0e-229,
  1.0e-230, 1.0e-231, 1.0e-232, 1.0e-233, 1.0e-234, 1.0e-235, 1.0e-236, 1.0e-237, 1.0e-238, 1.0e-239,
  1.0e-240, 1.0e-241, 1.0e-242, 1.0e-243, 1.0e-244, 1.0e-245, 1.0e-246, 1.0e-247, 1.0e-248, 1.0e-249,
  1.0e-250, 1.0e-251, 1.0e-252, 1.0e-253, 1.0e-254, 1.0e-255, 1.0e-256, 1.0e-257, 1.0e-258, 1.0e-259,
  1.0e-260, 1.0e-261, 1.0e-262, 1.0e-263, 1.0e-264, 1.0e-265, 1.0e-266, 1.0e-267, 1.0e-268, 1.0e-269,
  1.0e-270, 1.0e-271, 1.0e-272, 1.0e-273, 1.0e-274, 1.0e-275, 1.0e-276, 1.0e-277, 1.0e-278, 1.0e-279,
  1.0e-280, 1.0e-281, 1.0e-282, 1.0e-283, 1.0e-284, 1.0e-285, 1.0e-286, 1.0e-287, 1.0e-288, 1.0e-289,
  1.0e-290, 1.0e-291, 1.0e-292, 1.0e-293, 1.0e-294, 1.0e-295, 1.0e-296, 1.0e-297, 1.0e-298, 1.0e-299,
  1.0e-300, 1.0e-301, 1.0e-302, 1.0e-303, 1.0e-304, 1.0e-305, 1.0e-306, 1.0e-307, 1.0e-308
};

const float float_decimal_negpower10[] = {
     1.0e0f,   1.0e-1f,   1.0e-2f,   1.0e-3f,   1.0e-4f,   1.0e-5f,   1.0e-6f,   1.0e-7f,   1.0e-8f,   1.0e-9f,
   1.0e-10f,  1.0e-11f,  1.0e-12f,  1.0e-13f,  1.0e-14f,  1.0e-15f,  1.0e-16f,  1.0e-17f,  1.0e-18f,  1.0e-19f,
   1.0e-20f,  1.0e-21f,  1.0e-22f,  1.0e-23f,  1.0e-24f,  1.0e-25f,  1.0e-26f,  1.0e-27f,  1.0e-28f,  1.0e-29f,
   1.0e-30f,  1.0e-31f,  1.0e-32f,  1.0e-33f,  1.0e-34f,  1.0e-35f,  1.0e-36f,  1.0e-37f,  1.0e-38f
};

#endif

/* scale number by floor(log10(number)) + 1 so that the result is in range [0.1, 1) */

#define ieee_double_exponent10(ieee_number) ((int)floor(log10(ieee_number.number)) + 1)
#define ieee_float_exponent10(ieee_number) ((int)floorf(log10f(ieee_number.number)) + 1) // floorf, log10f ?

#define ieee_double_exp10(ieee_number, exponent10) \
  exponent10 = ieee_double_exponent10(ieee_number); \
  if (exponent10 > 0) { \
    double_negative_exp10(ieee_number.number, -exponent10); \
    ieee_number.fraction = ieee_double_fraction(ieee_number.bits); \
    ieee_number.exponent = ieee_double_exponent(ieee_number.bits); \
  } else if (exponent10 < 0) { \
    double_positive_exp10(ieee_number.number, -exponent10); \
    ieee_number.fraction = ieee_double_fraction(ieee_number.bits); \
    ieee_number.exponent = ieee_double_exponent(ieee_number.bits); \
  }

#define ieee_float_exp10(ieee_number, exponent10) \
  exponent10 = ieee_float_exponent10(ieee_number); \
  if (exponent10 > 0) { \
    float_negative_exp10(ieee_number.number, -exponent10); \
    ieee_number.fraction = ieee_float_fraction(ieee_number.bits); \
    ieee_number.exponent = ieee_float_exponent(ieee_number.bits); \
  } else if (exponent10 < 0) { \
    float_positive_exp10(ieee_number.number, -exponent10); \
    ieee_number.fraction = ieee_float_fraction(ieee_number.bits); \
    ieee_number.exponent = ieee_float_exponent(ieee_number.bits); \
  }

#if BINARY_MODF

/* unhide implicit bit 53, produce 56-bit denormalised fraction (binary exponent already in range [-4, -1]) */

#define ieee_double_denormalize(ieee_number) \
  (ieee_number.exponent == IEEE_DOUBLE_MIN_EXPONENT ? (++ieee_number.exponent, 0) : (ieee_number.fraction |= (1ull<<52))), \
  ieee_number.fraction <<= (ieee_number.exponent + 4)

/* unhide implicit bit 24, produce 27-bit denormalized fraction (binary exponent already in range [-4, -1]) */

#define ieee_float_denormalize(ieee_number) \
  (ieee_number.exponent == IEEE_FLOAT_MIN_EXPONENT ? (++ieee_number.exponent, 0) : (ieee_number.fraction |= (1<<23))), \
  ieee_number.fraction <<= (ieee_number.exponent + 4)

/* turn off significant bits over 56 (integer part), multiply by 10, return new integer part (subsequent decimal digit) */

#define ieee_double_binary_fraction(ieee_number) \
  (ieee_number.fraction &= ((1ull<<56) - 1), \
   ieee_number.fraction = (ieee_number.fraction << 1) + (ieee_number.fraction << 3), \
   ieee_number.fraction >> 56)

/* turn off significant bits over 27 (integer part), multiply by 10, return the integer part (subsequent decimal digit) */

#define ieee_float_binary_fraction(ieee_number) \
  (ieee_number.fraction &= ((1<<27) - 1), \
   ieee_number.fraction = (ieee_number.fraction << 1) + (ieee_number.fraction << 3), \
   ieee_number.fraction >> 27)

#define ieee_double_decimal(ieee_number, exponent10, digits, p) \
  ieee_number_decimal(ieee_double_binary_fraction, ieee_number, exponent10, digits, p)
#define ieee_float_decimal(ieee_number, exponent10, digits, p) \
  ieee_number_decimal(ieee_float_binary_fraction, ieee_number, exponent10, digits, p)
#define ieee_double_decimal_dot(ieee_number, exponent10, digits, p, dot) \
  ieee_number_decimal_dot(ieee_double_binary_fraction, ieee_number, exponent10, digits, p, dot)
#define ieee_float_decimal_dot(ieee_number, exponent10, digits, p, dot) \
  ieee_number_decimal_dot(ieee_float_binary_fraction, ieee_number, exponent10, digits, p, dot)

#else

/* generic method */

#define ieee_double_decimal_fraction(ieee_number, i) (ieee_number.number = modf(10*ieee_number.number, &i), i)
#define ieee_float_decimal_fraction(ieee_number, i) (ieee_number.number = (float)modf(10*ieee_number.number, &i), i) // ???

#define ieee_double_decimal(ieee_number, exponent10, digits, p) \
  ieee_number_decimal(ieee_double_decimal_fraction, ieee_number, exponent10, digits, p)
#define ieee_float_decimal(ieee_number, exponent10, digits, p) \
  ieee_number_decimal(ieee_float_decimal_fraction, ieee_number, exponent10, digits, p)
#define ieee_double_decimal_dot(ieee_number, exponent10, digits, p, dot) \
  ieee_number_decimal_dot(ieee_double_decimal_fraction, ieee_number, exponent10, digits, p, dot)
#define ieee_float_decimal_dot(ieee_number, exponent10, digits, p, dot) \
  ieee_number_decimal_dot(ieee_float_decimal_fraction, ieee_number, exponent10, digits, p, dot)

#endif

#define ieee_number_decimal(method, ieee_number, exponent10, digits, p) \
  ieee_double_denormalize(ieee_number); \
  if (ieee_number.sign) *p++ = '-'; \
  if (exponent10 <= 0) \
    for (*p++ = '0', *p++ = RADIX_CHAR; exponent10 && digits; *p++ = '0', ++exponent10, --digits); \
  else \
  { \
    do { *p++ = '0' + (char)method(ieee_number); } while (--exponent10); \
    *p++ = RADIX_CHAR; \
  } \
  for  ( ; digits && ieee_number.fraction; --digits) \
    *p++ = '0' + (char)method(ieee_number)

#define ieee_number_decimal_dot(method, ieee_number, exponent10, digits, p, dot) \
  ieee_double_denormalize(ieee_number); \
  if (ieee_number.sign) *p++ = '-'; \
  if (exponent10 <= 0) \
  { \
    *p++ = '0'; \
    if (dot != NULL) *dot = p; \
    for (*p++ = RADIX_CHAR; exponent10 && digits; *p++ = '0', ++exponent10, --digits); \
  } \
  else \
  { \
    do { *p++ = '0' + (char)method(ieee_number); } while (--exponent10); \
    if (dot != NULL) *dot = p; \
    *p++ = RADIX_CHAR; \
  } \
  for  ( ; digits && ieee_number.fraction; --digits) \
    *p++ = '0' + (char)method(ieee_number)

/* rounding to nearest integer */

#if BINARY_MODF
/* check if the mantissa has the most significant bit set, means >= 0.5 */
#  define ieee_double_half(ieee_number) (ieee_number.fraction & (1ull<<55))
#  define ieee_float_half(ieee_number) (ieee_number.fraction & (1<<26))
#else
#  define ieee_double_half(ieee_number) (ieee_number.number >= 0.5)
#  define ieee_float_half(ieee_number) (ieee_number.number >= 0.5)
#endif

/* rounding to nearest integer */

#define buffer_ceil(s, p, sign) \
  { \
    while (*--p == '9'); \
    if (*p != RADIX_CHAR) ++*p++; \
    else { \
      char *q; \
      for (q = p - 1; ; --q) { \
        if (*q < '9') { ++*q; break; } \
        *q = '0'; \
        if (q == s) \
          *--s = '1'; \
        else if (sign && q - 1 == s) \
          *s = '1', *--s = '-'; \
      } \
    } \
  }

#define buffer_remove_trailing_zeros(s, p, sign) \
  { \
    while (*--p == '0'); \
    if (*p != RADIX_CHAR) \
      ++p; \
    else if (!SIGNED_ZERO && sign && p - 2 == s && *(p - 1) == '0') \
      p -= 2, *p++ = '0'; \
  }

// if digits parameter was initially less then exponent10, then exponent10 > 0 and ieee_double_half(ieee_number) is irrelevant
#define ieee_double_round(ieee_number, exponent10, s, p) \
  if (exponent10 == 0 && ieee_double_half(ieee_number)) \
    { buffer_ceil(s, p, ieee_number.sign); } \
  else \
    { buffer_remove_trailing_zeros(s, p, ieee_number.sign); }

#define ieee_float_round(ieee_number, exponent10, s, p) \
  if (exponent10 == 0 && ieee_float_half(ieee_number)) \
    { buffer_ceil(s, p, ieee_number.sign); } \
  else \
    { buffer_remove_trailing_zeros(s, p, ieee_number.sign); }

/* double to decimal */

static char number_buffer[512];

#define ieee_copy_special_string(special, p, _p) \
  for (p = (char *)number_buffer, _p = special; ; ++p, ++_p) { \
    if ((*p = *_p) == '\0') break; \
  }

#define ieee_copy_special_string_re(special, p, _p, r, e) \
  for (p = (char *)number_buffer, _p = special; ; ++p, ++_p) { \
    if ((*p = *_p) == '\0') { \
    	if (r != NULL) *r = NULL; \
    	if (e != NULL) *e = p; \
      break; \
    } \
  }

char * double_to_string (double number, int digits)
{
  ieee_double ieee_number;
  int exponent10;
  char *s, *p; const char *_p;
  ieee_double_init(ieee_number, number);
  ieee_double_sign(ieee_number);
  if (ieee_double_is_zero(ieee_number)) // to avoid crash on log10(number)
  {
    ieee_copy_special_string(ieee_double_zero_string(ieee_number), p, _p);
    return (char *)number_buffer;
  }
  if (ieee_double_special_case(ieee_number))
  {
    ieee_copy_special_string(ieee_double_special_string(ieee_number), p, _p);
    return (char *)number_buffer;
  }
  s = p = number_buffer + 1;
  ieee_double_exp10(ieee_number, exponent10);
  ieee_double_decimal(ieee_number, exponent10, digits, p);
  ieee_double_round(ieee_number, exponent10, s, p);
  *p = '\0';
  return s;
}

char * double_as_string (double number, int digits, char **r, char **e)
{
  ieee_double ieee_number;
  int exponent10;
  char *s, *p; const char *_p;
  s = p = number_buffer + 1;
  ieee_double_init(ieee_number, number);
  ieee_double_sign(ieee_number);
  if (ieee_double_is_zero(ieee_number)) // to avoid crash on log10(number)
  {
    ieee_copy_special_string_re(ieee_double_zero_string(ieee_number), p, _p, r, e);
    return (char *)number_buffer;
  }
  if (ieee_double_special_case(ieee_number))
  {
    ieee_copy_special_string_re(ieee_double_special_string(ieee_number), p, _p, r, e);
    return (char *)number_buffer;
  }
  ieee_double_exp10(ieee_number, exponent10);
  ieee_double_decimal_dot(ieee_number, exponent10, digits, p, r);
  ieee_double_round(ieee_number, exponent10, s, p);
  if (e != NULL) *e = p;
  *p = '\0';
  return s;
}

/* float to decimal */

char * float_to_string (float number, int digits)
{
  ieee_float ieee_number;
  int exponent10;
  char *s, *p; const char *_p;
  ieee_float_init(ieee_number, number);
  ieee_float_sign(ieee_number);
  if (ieee_float_is_zero(ieee_number))
  {
    ieee_copy_special_string(ieee_float_zero_string(ieee_number), p, _p);
    return (char *)number_buffer;
  }
  if (ieee_float_special_case(ieee_number))
  {
    ieee_copy_special_string(ieee_float_special_string(ieee_number), p, _p);
    return (char *)number_buffer;
  }
  s = p = number_buffer + 1;
  ieee_float_exp10(ieee_number, exponent10);
  ieee_float_decimal(ieee_number, exponent10, digits, p);
  ieee_float_round(ieee_number, exponent10, s, p);
  *p = '\0';
  return s;
}

char * float_as_string (float number, int digits, char **r, char **e)
{
  ieee_float ieee_number;
  int exponent10;
  char *s, *p; const char *_p;
  s = p = number_buffer + 1;
  ieee_float_init(ieee_number, number);
  ieee_float_sign(ieee_number);
  if (ieee_float_is_zero(ieee_number))
  {
    ieee_copy_special_string_re(ieee_float_zero_string(ieee_number), p, _p, r, e);
    return (char *)number_buffer;
  }
  if (ieee_float_special_case(ieee_number))
  {
    ieee_copy_special_string_re(ieee_float_special_string(ieee_number), p, _p, r, e);
    return (char *)number_buffer;
  }
  ieee_float_exp10(ieee_number, exponent10);
  ieee_float_decimal_dot(ieee_number, exponent10, digits, p, r);
  ieee_float_round(ieee_number, exponent10, s, p);
  if (e != NULL) *e = p;
  *p = '\0';
  return s;
}

/* decimal string to double/float */

#define string_scan_decimal(s, c, number) _scan_decimal(c, number, *++s)
#define string_scan_fraction(s, c, number, exponent10) _scan_fraction(c, number, exponent10, *++s)
#define string_scan_exponent10(s, c, exponent10) _scan_exponent10(c, exponent10, *++s)

const char * string_to_double (const char *s, double *number)
{
  int sign, exponent10, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_decimal(s, c, *number);
  if (c == '.')
  {
    c = *++s;
    string_scan_fraction(s, c, *number, exponent10);
  }
  else
    exponent10 = 0;
  if (c == 'e' || c == 'E')
  {
    c = *++s;
    string_scan_exponent10(s, c, exponent10);
  }
  double_exp10(*number, exponent10);
  if (sign) *number = -*number;
  return s;
}

const char * string_to_float (const char *s, float *number)
{
  int sign, exponent10, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_decimal(s, c, *number);
  if (c == '.')
  {
    c = *++s;
    string_scan_fraction(s, c, *number, exponent10);
  }
  else
    exponent10 = 0;
  if (c == 'e' || c == 'E')
  {
    c = *++s;
    string_scan_exponent10(s, c, exponent10);
  }
  float_exp10(*number, exponent10);
  if (sign) *number = -*number;
  return s;
}

/* conventional form */

const char * convert_to_double (const char *s, double *number)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_decimal(s, c, *number);
  if (c == '.' || c == ',')
  {
    int exponent10;
    c = *++s;
    string_scan_fraction(s, c, *number, exponent10);
    if (exponent10 < 0)
      double_negative_exp10(*number, exponent10);
  }
  if (sign) *number = -*number;
  return s;
}

const char * convert_to_float (const char *s, float *number)
{
  int sign, c = *s;
  string_scan_sign(s, c, sign);
  string_scan_decimal(s, c, *number);
  if (c == '.' || c == ',')
  {
    int exponent10;
    c = *++s;
    string_scan_fraction(s, c, *number, exponent10);
    if (exponent10 < 0)
      float_negative_exp10(*number, exponent10);
  }
  if (sign) *number = -*number;
  return s;
}

/* pretty common stuff */

size_t bytes_to_hex_lc (const void *input, size_t size, unsigned char *output)
{
  size_t i;
  const unsigned char *p;
  for (i = 0, p = (const unsigned char *)input; i < size; ++i, ++p)
  {
    *output++ = base16_lc_digit1(*p);
    *output++ = base16_lc_digit2(*p);
  }
  *output = '\0';
  return 2*size + 1;
}

size_t bytes_to_hex_uc (const void *input, size_t size, unsigned char *output)
{
  size_t i;
  const unsigned char *p;
  for (i = 0, p = (const unsigned char *)input; i < size; ++i, ++p)           
  {
    *output++ = base16_uc_digit1(*p);
    *output++ = base16_uc_digit2(*p);
  }
  *output = '\0';
  return 2*size + 1;
}

size_t hex_to_bytes (const void *input, size_t size, unsigned char *output)
{
  size_t i;
  int c1, c2;
  const unsigned char *p;
  for (i = 1, p = (const unsigned char *)input; i < size; i += 2)
  {
    c1 = base16_value(*p);
    ++p;
    c2 = base16_value(*p);
    ++p;
    if (c1 >= 0 && c2 >= 0)
      *output++ = (unsigned char)((c1<<4)|c2);
    else
      break;
  }
  return i >> 1;
}

void print_as_hex (const void *input, size_t bytes)
{
  const unsigned char *p;
  for (p = (const unsigned char *)input; bytes > 0; --bytes, ++p)
    printf("%02x", *p);
}