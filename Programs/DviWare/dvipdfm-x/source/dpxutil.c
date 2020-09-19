/* This is dvipdfmx, an eXtended version of dvipdfm by Mark A. Wicks.

    Copyright (C) 2002-2019 by Jin-Hwan Cho and Shunsaku Hirata,
    the dvipdfmx project team.

    Copyright (C) 1998, 1999 by Mark A. Wicks <mwicks@kettering.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "system.h"
#include "mem.h"
#include "error.h"

#include "dpxutil.h"

int
xtoi (char c)
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'f')
    return (c - 'W');
  else if (c >= 'A' && c <= 'F')
    return (c - '7');
  else
    return -1;
}

double
min4 (double x1, double x2, double x3, double x4)
{
  double v = x1;
  if (x2 < v) v = x2;
  if (x3 < v) v = x3;
  if (x4 < v) v = x4;
  return v;
}

double
max4 (double x1, double x2, double x3, double x4)
{
  double v = x1;
  if (x2 > v) v = x2;
  if (x3 > v) v = x3;
  if (x4 > v) v = x4;
  return v;
}


#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

/* If an environment variable SOURCE_DATE_EPOCH is correctly defined like
 * SOURCE_DATE_EPOCH=1456304492, then returns this value, to be used as the
 * 'current time', otherwise returns INVALID_EPOCH_VALUE (= (time_t)-1).
 * In the case of Microsoft Visual Studio 2010, the value should be less
 * than 32535291600.
 */

time_t
dpx_util_get_unique_time_if_given(void)
{
  const char *source_date_epoch;
  int64_t epoch;
  char *endptr;
  time_t ret = INVALID_EPOCH_VALUE;

  source_date_epoch = getenv("SOURCE_DATE_EPOCH");
  if (source_date_epoch) {
    errno = 0;
    epoch = strtoll(source_date_epoch, &endptr, 10);
    if (!(epoch < 0 || *endptr != '\0' || errno != 0)) {
      ret = (time_t) epoch;
#if defined(_MSC_VER)
      if (ret > 32535291599ULL)
        ret = 32535291599ULL;
#endif
    }
  }
  return ret;
}


#ifndef HAVE_TM_GMTOFF
#ifndef HAVE_TIMEZONE

/* auxiliary function to compute timezone offset on
   systems that do not support the tm_gmtoff in struct tm,
   or have a timezone variable.  Such as i386-solaris.  */

static int32_t
compute_timezone_offset()
{
  time_t now;
  struct tm tm;
  struct tm local;
  time_t gmtoff;

  now = get_unique_time_if_given();
  if (now == INVALID_EPOCH_VALUE) {
    now = time(NULL);
    localtime_r(&now, &local);
    gmtime_r(&now, &tm);
    return (mktime(&local) - mktime(&tm));
  } else {
    return(0);
  }
}

#endif /* HAVE_TIMEZONE */
#endif /* HAVE_TM_GMTOFF */

/*
 * Docinfo
 */
int
dpx_util_format_asn_date (char *date_string, int need_timezone)
{
  int32_t     tz_offset;
  time_t      current_time;
  struct tm  *bd_time;

  current_time = dpx_util_get_unique_time_if_given();
  if (current_time == INVALID_EPOCH_VALUE) {
    time(&current_time);
    bd_time = localtime(&current_time);

#ifdef HAVE_TM_GMTOFF
    tz_offset = bd_time->tm_gmtoff;
#else
#  ifdef HAVE_TIMEZONE
    tz_offset = -timezone;
#  else
    tz_offset = compute_timezone_offset();
#  endif /* HAVE_TIMEZONE */
#endif /* HAVE_TM_GMTOFF */
  } else {
    bd_time = gmtime(&current_time);
    tz_offset = 0;
  }
  if (need_timezone) {
    if (bd_time->tm_isdst > 0) {
      tz_offset += 3600;
    }
    sprintf(date_string, "D:%04d%02d%02d%02d%02d%02d%c%02d'%02d'",
            bd_time->tm_year + 1900, bd_time->tm_mon + 1, bd_time->tm_mday,
            bd_time->tm_hour, bd_time->tm_min, bd_time->tm_sec,
            (tz_offset > 0) ? '+' : '-', abs(tz_offset) / 3600,
                                        (abs(tz_offset) / 60) % 60);
  } else {
    sprintf(date_string, "D:%04d%02d%02d%02d%02d%02d",
            bd_time->tm_year + 1900, bd_time->tm_mon + 1, bd_time->tm_mday,
            bd_time->tm_hour, bd_time->tm_min, bd_time->tm_sec);
  }

  return strlen(date_string);
}

void
skip_white_spaces (unsigned char **s, unsigned char *endptr)
{
  while (*s < endptr)
    if (!is_space(**s))
      break;
    else
      (*s)++;
}

void
ht_init_table (struct ht_table *ht, hval_free_func hval_free_fn)
{
  int  i;

  ASSERT(ht);

  for (i = 0; i < HASH_TABLE_SIZE; i++) {
    ht->table[i] = NULL;
  }
  ht->count = 0;
  ht->hval_free_fn = hval_free_fn;
}

void
ht_clear_table (struct ht_table *ht)
{
  int   i;

  ASSERT(ht);

  for (i = 0; i < HASH_TABLE_SIZE; i++) {
    struct ht_entry *hent, *next;

    hent = ht->table[i];
    while (hent) {
      if (hent->value && ht->hval_free_fn) {
	ht->hval_free_fn(hent->value);
      }
      hent->value  = NULL;
      if (hent->key) {
	RELEASE(hent->key);
      }
      hent->key = NULL;
      next = hent->next;
      RELEASE(hent);
      hent = next;
    }
    ht->table[i] = NULL;
  }
  ht->count = 0;
  ht->hval_free_fn = NULL;
}

int ht_table_size (struct ht_table *ht)
{
  ASSERT(ht);

  return ht->count;
}

static unsigned int
get_hash (const void *key, int keylen)
{
  unsigned int hkey = 0;
  int      i;

  for (i = 0; i < keylen; i++) {
    hkey = (hkey << 5) + hkey + ((const char *)key)[i];
  }

  return (hkey % HASH_TABLE_SIZE);
}

void *
ht_lookup_table (struct ht_table *ht, const void *key, int keylen)
{
  struct ht_entry *hent;
  unsigned int     hkey;

  ASSERT(ht && key);

  hkey = get_hash(key, keylen);
  hent = ht->table[hkey];
  while (hent) {
    if (hent->keylen == keylen &&
	!memcmp(hent->key, key, keylen)) {
      return hent->value;
    }
    hent = hent->next;
  }

  return NULL;
}

int
ht_remove_table (struct ht_table *ht,
		 const void *key, int keylen)
/* returns 1 if the element was found and removed and 0 otherwise */
{
  struct ht_entry *hent, *prev;
  unsigned int     hkey;

  ASSERT(ht && key);

  hkey = get_hash(key, keylen);
  hent = ht->table[hkey];
  prev = NULL;
  while (hent) {
    if (hent->keylen == keylen &&
	!memcmp(hent->key, key, keylen)) {
      break;
    }
    prev = hent;
    hent = hent->next;
  }
  if (hent) {
    if (hent->key)
      RELEASE(hent->key);
    hent->key    = NULL;
    hent->keylen = 0;
    if (hent->value && ht->hval_free_fn) {
      ht->hval_free_fn(hent->value);
    }
    hent->value  = NULL;
    if (prev) {
      prev->next = hent->next;
    } else {
      ht->table[hkey] = hent->next;
    }
    RELEASE(hent);
    ht->count--;
    return 1;
  } else
    return 0;
}

/* replace... */
void
ht_insert_table (struct ht_table *ht,
		 const void *key, int keylen, void *value)
{
  struct ht_entry *hent, *prev;
  unsigned int     hkey;

  ASSERT(ht && key);

  hkey = get_hash(key, keylen);
  hent = ht->table[hkey];
  prev = NULL;
  while (hent) {
    if (hent->keylen == keylen &&
	!memcmp(hent->key, key, keylen)) {
      break;
    }
    prev = hent;
    hent = hent->next;
  }
  if (hent) {
      if (hent->value && ht->hval_free_fn)
	ht->hval_free_fn(hent->value);
      hent->value  = value;
  } else {
    hent = NEW(1, struct ht_entry);
    hent->key = NEW(keylen, char);
    memcpy(hent->key, key, keylen);
    hent->keylen = keylen;
    hent->value  = value;
    hent->next   = NULL;
    if (prev) {
      prev->next      = hent;
    } else {
      ht->table[hkey] = hent;
    }
    ht->count++;
  }
}

void
ht_append_table (struct ht_table *ht,
		 const void *key, int keylen, void *value) 
{
  struct ht_entry *hent, *last;
  unsigned int hkey;

  hkey = get_hash(key, keylen);
  hent = ht->table[hkey];
  if (!hent) {
    hent = NEW(1, struct ht_entry);
    ht->table[hkey] = hent;
  } else {
    while (hent) {
      last = hent;
      hent = hent->next;
    }
    hent = NEW(1, struct ht_entry);
    last->next = hent;
  }
  hent->key = NEW(keylen, char);
  memcpy(hent->key, key, keylen);
  hent->keylen = keylen;
  hent->value  = value;
  hent->next   = NULL;

  ht->count++;
}

int
ht_set_iter (struct ht_table *ht, struct ht_iter *iter)
{
  int    i;

  ASSERT(ht && iter);

  for (i = 0; i < HASH_TABLE_SIZE; i++) {
    if (ht->table[i]) {
      iter->index = i;
      iter->curr  = ht->table[i];
      iter->hash  = ht;
      return 0;
    }
  }

  return -1;
}

void
ht_clear_iter (struct ht_iter *iter)
{
  if (iter) {
    iter->index = HASH_TABLE_SIZE;
    iter->curr  = NULL;
    iter->hash  = NULL;
  }
}

char *
ht_iter_getkey (struct ht_iter *iter, int *keylen)
{
  struct ht_entry *hent;

  hent = (struct ht_entry *) iter->curr;
  if (iter && hent) {
    *keylen = hent->keylen;
    return hent->key;
  } else {
    *keylen = 0;
    return NULL;
  }
}

void *
ht_iter_getval (struct ht_iter *iter)
{
  struct ht_entry *hent;

  hent = (struct ht_entry *) iter->curr;
  if (iter && hent) {
    return hent->value;
  } else {
    return NULL;
  }
}

int
ht_iter_next (struct ht_iter *iter)
{
  struct ht_entry *hent;
  struct ht_table *ht;

  ASSERT(iter);

  ht   = iter->hash;
  hent = (struct ht_entry *) iter->curr;
  hent = hent->next;
  while (!hent &&
         ++iter->index < HASH_TABLE_SIZE) {
    hent = ht->table[iter->index];
  }
  iter->curr = hent;

  return (hent ? 0 : -1);
}


static int
read_c_escchar (char *r, const char **pp, const char *endptr)
{
  int   c = 0, l = 1;
  const char *p = *pp;

  switch (p[0]) {
  case 'a' : c = '\a'; p++; break;
  case 'b' : c = '\b'; p++; break;
  case 'f' : c = '\f'; p++; break;
  case 'n' : c = '\n'; p++; break;
  case 'r' : c = '\r'; p++; break;
  case 't' : c = '\t'; p++; break;
  case 'v' : c = '\v'; p++; break;
  case '\\': case '?': case '\'': case '\"':
    c = p[0]; p++;
    break;
  case '\n': l = 0; p++; break;
  case '\r':
    {
      p++;
      if (p < endptr && p[0] == '\n')
        p++;
      l = 0;
    }
    break;
  case '0': case '1': case '2': case '3':
  case '4': case '5': case '6': case '7':
    {
      int  i;
      for (c = 0, i = 0;
           i < 3 && p < endptr &&
           p[0] >= '0' && p[0] <= '7'; i++, p++)
        c = (c << 3) + (p[0] - '0');
    }
    break;
  case 'x':
    {
      int  i;
      for (c = 0, i = 0, p++;
           i < 2 && p < endptr && isxdigit((unsigned char)p[0]);
           i++, p++)
        c = (c << 4) +
            (isdigit((unsigned char)p[0]) ?
             p[0] - '0' :
             (islower((unsigned char)p[0]) ? p[0] - 'a' + 10: p[0] - 'A' + 10));
    }
    break;
  default:
    WARN("Unknown escape char sequence: \\%c", p[0]);
    l = 0; p++;
    break;
  }

  if (r)
    *r = (char) c;
  *pp  = p;
  return  l;
}

#define C_QUOTE  '"'
#define C_ESCAPE '\\'
static int
read_c_litstrc (char *q, int len, const char **pp, const char *endptr)
{
  const char *p;
  int    l = 0;
#define Q_TERM          0
#define Q_CONT         -1
#define Q_ERROR_UNTERM -1
#define Q_ERROR_INVAL  -2
#define Q_ERROR_BUFF   -3
  int    s = Q_CONT;

  for (l = 0, p = *pp;
       s == Q_CONT && p < endptr; ) {
    switch (p[0]) {
    case C_QUOTE:
      s = Q_TERM; p++;
      break;
    case C_ESCAPE:
      if (q && l == len)
        s = Q_ERROR_BUFF;
      else {
        p++;
        l += read_c_escchar(q ? &q[l] : NULL, &p, endptr);
      }
      break;
    case '\n': case '\r':
      s = Q_ERROR_INVAL;
      break;
    default:
      if (q && l == len)
        s = Q_ERROR_BUFF;
      else {
        if (!q)
          l++;
        else
          q[l++] = p[0];
        p++;
      }
      break;
    }
  }
  if (s == Q_TERM) {
    if (q && l == len)
      s = Q_ERROR_BUFF;
    else if (q)
      q[l++] = '\0';
  }

  *pp = p;
  return ((s == Q_TERM) ? l : s);
}

char *
parse_c_string (const char **pp, const char *endptr)
{
  char  *q = NULL;
  const char *p = *pp;
  int    l = 0;

  if (p >= endptr || p[0] != C_QUOTE)
    return NULL;

  p++;
  l = read_c_litstrc(NULL, 0, &p, endptr);
  if (l >= 0) {
    q = NEW(l + 1, char);
    p = *pp + 1;
    l = read_c_litstrc(q, l + 1, &p, endptr);
  }

  *pp = p;
  return  q;
}

#define ISCNONDIGITS(c) ( \
  (c) == '_' || \
  ((c) >= 'a' && (c) <= 'z') || \
  ((c) >= 'A' && (c) <= 'Z') \
)
#define ISCIDENTCHAR(c) ( \
  ISCNONDIGITS((c)) || \
  ((c) >= '0' && (c) <= '9') \
)

char *
parse_c_ident (const char **pp, const char *endptr)
{
  char  *q = NULL;
  const char *p = *pp;
  int    n;

  if (p >= endptr || !ISCNONDIGITS(*p))
    return NULL;

  for (n = 0; p < endptr && ISCIDENTCHAR(*p); p++, n++);
  q = NEW(n + 1, char);
  memcpy(q, *pp, n); q[n] = '\0';

  *pp = p;
  return  q;
}

char *
parse_float_decimal (const char **pp, const char *endptr)
{
  char  *q = NULL;
  const char *p = *pp;
  int    s = 0, n = 0;

  if (p >= endptr)
    return NULL;

  if (p[0] == '+' || p[0] == '-')
    p++;

  /* 1. .01 001 001E-001 */
  for (s = 0, n = 0; p < endptr && s >= 0; ) {
    switch (p[0]) {
    case '+': case '-':
      if (s != 2)
        s = -1;
      else {
        s = 3; p++;
      }
      break;
    case '.':
      if (s > 0)
        s = -1;
      else {
        s =  1; p++;
      }
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      n++; p++;
      break;
    case 'E': case 'e':
      if (n == 0 || s == 2)
        s = -1;
      else {
        s = 2; p++;
      }
      break;
    default:
      s = -1;
      break;
    }
  }

  if (n != 0) {
    n = (int) (p - *pp);
    q = NEW(n + 1, char);
    memcpy(q, *pp, n); q[n] = '\0';
  }

  *pp = p;
  return  q;
}
