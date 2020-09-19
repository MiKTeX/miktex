
#include "utilnumber.h"
#include "utilmem.h"
#include "utilbasexx.h"

/* filters state structs */

struct basexx_state {
  size_t line, maxline;
  size_t left;
  int tail[5];
  int flush;
};

struct runlength_state {
  int run;
  int flush;
  int c1, c2;
  uint8_t *pos;
};

typedef union { basexx_state *basexxstate; runlength_state *runlengthstate; void *voidstate; } basexx_state_pointer; // to avoid 'dereferencing type-puned ...' warnings

/* config */

#if defined(BASEXX_PDF)
#  define ignored(c) (c == 0x20 || c == 0x0A || c == 0x0C || c == 0x0D || c == 0x09 || c == 0x00)
#  define base16_eof(c) (c == '>' || c < 0)
#  define base85_eof(c) (c == '~' || c < 0)
#else
#  define ignored(c) (c == 0x20 || c == 0x0A || c == 0x0D || c == 0x09)
#  define base16_eof(c) (c < 0)
#  define base85_eof(c) (c < 0)
#endif

#define base64_eof(c) (c == '=' || c < 0)

#define basexx_nl '\x0A'
//#define put_nl(O, line, maxline, n) ((void)((line += n) > maxline && ((line = n), iof_set(O, basexx_nl)))) // assignment in conditional warning
#define put_nl(O, line, maxline, n) do { line += n; if (line > maxline) { line = n; iof_set(O, basexx_nl); }} while (0)

/* tail macros */

#define set_tail1(state, c1)             (state->left = 1, state->tail[0] = c1)
#define set_tail2(state, c1, c2)         (state->left = 2, state->tail[0] = c1, state->tail[1] = c2)
#define set_tail3(state, c1, c2, c3)     (state->left = 3, state->tail[0] = c1, state->tail[1] = c2, state->tail[2] = c3)
#define set_tail4(state, c1, c2, c3, c4) (state->left = 4, state->tail[0] = c1, state->tail[1] = c2, state->tail[2] = c3, state->tail[3] = c4)
#define set_tail5(state, c1, c2, c3, c4, c5) \
  (state->left = 5, state->tail[0] = c1, state->tail[1] = c2, state->tail[2] = c3, state->tail[3] = c4, state->tail[4] = c5)

#define get_tail1(state, c1)             (state->left = 0, c1 = state->tail[0])
#define get_tail2(state, c1, c2)         (state->left = 0, c1 = state->tail[0], c2 = state->tail[1])
#define get_tail3(state, c1, c2, c3)     (state->left = 0, c1 = state->tail[0], c2 = state->tail[1], c3 = state->tail[2])
#define get_tail4(state, c1, c2, c3, c4) (state->left = 0, c1 = state->tail[0], c2 = state->tail[1], c3 = state->tail[2], c4 = state->tail[3])

/* basexx state initialization */

void basexx_state_init_ln (basexx_state *state, size_t line, size_t maxline)
{
  state->line = line;
  state->maxline = maxline;
  state->left = 0;
  state->flush = 0;
}

/* base 16; xxxx|xxxx */

iof_status base16_encoded_uc (const void *data, size_t size, iof *O)
{
  const uint8_t *s, *e;
  for (s = (const uint8_t *)data, e = s + size; s < e; ++s)
  {
    if (!iof_ensure(O, 2))
      return IOFFULL;
    iof_set_uc_hex(O, *s);
  }
  return IOFEOF;
}

iof_status base16_encoded_lc (const void *data, size_t size, iof *O)
{
  const uint8_t *s, *e;
  for (s = (const uint8_t *)data, e = s + size; s < e; ++s)
  {
    if (!iof_ensure(O, 2))
      return IOFFULL;
    iof_set_lc_hex(O, *s);
  }
  return IOFEOF;
}

iof_status base16_encoded_uc_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline)
{
  const uint8_t *s, *e;
  for (s = (const uint8_t *)data, e = s + size; s < e; ++s)
  {
    if (!iof_ensure(O, 3))
      return IOFFULL;
    put_nl(O, line, maxline, 2);
    iof_set_uc_hex(O, *s);
  }
  return IOFFULL;
}

iof_status base16_encoded_lc_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline)
{
  const uint8_t *s, *e;
  for (s = (const uint8_t *)data, e = s + size; s < e; ++s)
  {
    if (!iof_ensure(O, 3))
      return IOFFULL;
    put_nl(O, line, maxline, 2);
    iof_set_lc_hex(O, *s);
  }
  return IOFFULL;
}

iof_status base16_encode_uc (iof *I, iof *O)
{
  register int c;
  while (iof_ensure(O, 2))
  {
    if ((c = iof_get(I)) < 0)
      return IOFEOF;
    iof_set_uc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_state_uc (iof *I, iof *O, basexx_state *state)
{
  register int c;
  while (iof_ensure(O, 2))
  {
    if ((c = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    iof_set_uc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_lc (iof *I, iof *O)
{
  register int c;
  while (iof_ensure(O, 2))
  {
    if ((c = iof_get(I)) < 0)
      return IOFEOF;
    iof_set_lc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_state_lc (iof *I, iof *O, basexx_state *state)
{
  register int c;
  while (iof_ensure(O, 2))
  {
    if ((c = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    iof_set_lc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_uc_ln (iof *I, iof *O, size_t line, size_t maxline)
{
  register int c;
  while (iof_ensure(O, 3))
  {
    if ((c = iof_get(I)) < 0)
      return IOFEOF;
    put_nl(O, line, maxline, 2);
    iof_set_uc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_state_uc_ln (iof *I, iof *O, basexx_state *state)
{
  register int c;
  while (iof_ensure(O, 3))
  {
    if ((c = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    put_nl(O, state->line, state->maxline, 2);
    iof_set_uc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_lc_ln (iof *I, iof *O, size_t line, size_t maxline)
{
  register int c;
  while (iof_ensure(O, 3))
  {
    if ((c = iof_get(I)) < 0)
      return IOFEOF;
    put_nl(O, line, maxline, 2);
    iof_set_lc_hex(O, c);
  }
  return IOFFULL;
}

iof_status base16_encode_state_lc_ln (iof *I, iof *O, basexx_state *state)
{
  register int c;
  while (iof_ensure(O, 3))
  {
    if ((c = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    put_nl(O, state->line, state->maxline, 2);
    iof_set_lc_hex(O, c);
  }
  return IOFFULL;
}

int base16_getc (iof *I)
{
  register int c1, c2;
  do { c1 = iof_get(I); } while (ignored(c1));
  if (base16_eof(c1))
    return IOFEOF;
  do { c2 = iof_get(I); } while (ignored(c2));
  if (base16_eof(c2))
  {
    if ((c1 = base16_value(c1)) < 0)
      return IOFERR;
    return c1<<4;
  }
  if ((c1 = base16_value(c1)) < 0 || (c2 = base16_value(c2)) < 0)
    return IOFERR;
  return (c1<<4)|c2;
}

int base16_lc_putc (iof *O, int c)
{
  if (iof_ensure(O, 2))
    iof_set_lc_hex(O, c);
  return IOFFULL;
}

int base16_uc_putc (iof *O, int c)
{
  if (iof_ensure(O, 2))
    iof_set_uc_hex(O, c);
  return IOFFULL;
}


iof_status base16_decode (iof *I, iof *O)
{
  register int c1, c2;
  while (iof_ensure(O, 1))
  {
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base16_eof(c1))
      return IOFEOF;
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base16_eof(c2))
    {
      if ((c1 = base16_value(c1)) < 0)
        return IOFERR;
      iof_set(O, c1<<4); // c2 := '0'
      return IOFEOF;
    }
    if ((c1 = base16_value(c1)) < 0 || (c2 = base16_value(c2)) < 0)
      return IOFERR;
    iof_set(O, (c1<<4)|c2);
  }
  return IOFFULL;
}

iof_status base16_decode_state (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, d1, d2;
  if (!(iof_ensure(O, 1)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
  }
  while (iof_ensure(O, 1))
  {
    byte0:
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base16_eof(c1))
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base16_eof(c2))
    {
      set_tail1(state, c1); /* set tail to let the caller display invalid chars */
      if (state->flush)
      {
        if ((c1 = base16_value(c1)) < 0)
          return IOFERR;
        iof_set(O, c1<<4); // c2 := '0'
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    if ((d1 = base16_value(c1)) < 0 || (d2 = base16_value(c2)) < 0)
    {
      set_tail2(state, c1, c2);
      return IOFERR;
    }
    iof_set(O, (d1<<4)|d2);
  }
  return IOFFULL;
}

/* base 64; xxxxxx|xx xxxx|xxxx xx|xxxxxx */

const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int base64_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
  52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,9 ,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
  -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

#define base64_value(c) base64_lookup[(uint8_t)(c)]

#define base64_digit1(c1)     base64_alphabet[c1>>2]
#define base64_digit2(c1, c2) base64_alphabet[((c1&3)<<4)|(c2>>4)]
#define base64_digit3(c2, c3) base64_alphabet[((c2&15)<<2)|(c3>>6)]
#define base64_digit4(c3)     base64_alphabet[c3&63]

#define base64_encode_word(O, c1, c2, c3) \
  iof_set4(O, base64_digit1(c1), base64_digit2(c1, c2), base64_digit3(c2, c3), base64_digit4(c3))

#define base64_encode_tail2(O, c1, c2) \
  iof_set3(O, base64_digit1(c1), base64_digit2(c1, c2), base64_digit3(c2, 0))

#define base64_encode_tail1(O, c1) \
  iof_set2(O, base64_digit1(c1), base64_digit2(c1, 0))

iof_status base64_encoded (const void *data, size_t size, iof *O)
{
  const uint8_t *s, *e;
  uint8_t c1, c2, c3;
  for (s = (const uint8_t *)data, e = s + size; s + 2 < e; )
  {
    if (!iof_ensure(O, 4))
      return IOFFULL;
    c1 = *s++;
    c2 = *s++;
    c3 = *s++;
    base64_encode_word(O, c1, c2, c3);
  }
  switch (e - s)
  {
    case 0:
      break;
    case 1:
      if (!iof_ensure(O, 2))
        return IOFFULL;
      c1 = *s;
      base64_encode_tail1(O, c1);
      break;
    case 2:
      if (!iof_ensure(O, 3))
        return IOFFULL;
      c1 = *s++;
      c2 = *s;
      base64_encode_tail2(O, c1, c2);
      break;
  }
  return IOFEOF;
}

iof_status base64_encoded_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline)
{
  const uint8_t *s, *e;
  uint8_t c1, c2, c3;
  for (s = (const uint8_t *)data, e = s + size; s + 2 < e; )
  {
    if (!iof_ensure(O, 5))
      return IOFFULL;
    c1 = *s++;
    c2 = *s++;
    c3 = *s++;
    put_nl(O, line, maxline, 4);
    base64_encode_word(O, c1, c2, c3);
  }
  switch (e - s)
  {
    case 0:
      break;
    case 1:
      if (!iof_ensure(O, 3))
        return IOFFULL;
      c1 = *s;
      put_nl(O, line, maxline, 2);
      base64_encode_tail1(O, c1);
      break;
    case 2:
      if (!iof_ensure(O, 4))
        return IOFFULL;
      c1 = *s++;
      c2 = *s;
      put_nl(O, line, maxline, 3);
      base64_encode_tail2(O, c1, c2);
      break;
  }
  return IOFEOF;
}

iof_status base64_encode (iof *I, iof *O)
{
  register int c1, c2, c3;
  while(iof_ensure(O, 4))
  {
    if ((c1 = iof_get(I)) < 0)
      return IOFEOF;
    if ((c2 = iof_get(I)) < 0)
    {
      base64_encode_tail1(O, c1);
      return IOFEOF;
    }
    if ((c3 = iof_get(I)) < 0)
    {
      base64_encode_tail2(O, c1, c2);
      return IOFEOF;
    }
    base64_encode_word(O, c1, c2, c3);
  }
  return IOFFULL;
}

iof_status base64_encode_state (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3;
  if (!(iof_ensure(O, 4)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
  }
  while(iof_ensure(O, 4))
  {
    byte0:
    if ((c1 = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    if ((c2 = iof_get(I)) < 0)
      return (state->flush ? (base64_encode_tail1(O, c1), IOFEOF) : (set_tail1(state, c1), IOFEMPTY));
    byte2:
    if ((c3 = iof_get(I)) < 0)
      return (state->flush ? (base64_encode_tail2(O, c1, c2), IOFEOF) : (set_tail2(state, c1, c2), IOFEMPTY));
    base64_encode_word(O, c1, c2, c3);
  }
  return IOFFULL;
}

iof_status base64_encode_ln (iof *I, iof *O, size_t line, size_t maxline)
{
  register int c1, c2, c3;
  while(iof_ensure(O, 5))
  {
    if ((c1 = iof_get(I)) < 0)
      return IOFEOF;
    if ((c2 = iof_get(I)) < 0)
    {
      put_nl(O, line, maxline, 2);
      base64_encode_tail1(O, c1);
      return IOFEOF;
    }
    if ((c3 = iof_get(I)) < 0)
    {
      put_nl(O, line, maxline, 3);
      base64_encode_tail2(O, c1, c2);
      return IOFEOF;
    }
    put_nl(O, line, maxline, 4);
    base64_encode_word(O, c1, c2, c3);
  }
  return IOFFULL;
}

iof_status base64_encode_state_ln (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3;
  if (!(iof_ensure(O, 5)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
  }
  while(iof_ensure(O, 5))
  {
    byte0:
    if ((c1 = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    if ((c2 = iof_get(I)) < 0)
    {
      if (state->flush)
      {
        put_nl(O, state->line, state->maxline, 2);
        base64_encode_tail1(O, c1);
        return IOFEOF;
      }
      set_tail1(state, c1);
      return IOFEMPTY;
    }
    byte2:
    if ((c3 = iof_get(I)) < 0)
    {
      if (state->flush)
      {
        put_nl(O, state->line, state->maxline, 3);
        base64_encode_tail2(O, c1, c2);
        return IOFEOF;
      }
      set_tail2(state, c1, c2);
      return IOFEMPTY;
    }
    put_nl(O, state->line, state->maxline, 4);
    base64_encode_word(O, c1, c2, c3);
  }
  return IOFFULL;
}

// #define base64_code(c1, c2, c3, c4) ((c1<<18)|(c2<<12)|(c3<<6)|c4)

#define base64_decode_word(O, c1, c2, c3, c4) \
  iof_set3(O, (c1<<2)|(c2>>4), ((c2&15)<<4)|(c3>>2), ((c3&3)<<6)|c4)

#define base64_decode_tail3(O, c1, c2, c3) \
  iof_set2(O, (c1<<2)|(c2>>4), ((c2&15)<<4)|(c3>>2))

#define base64_decode_tail2(O, c1, c2) \
  iof_set(O, (c1<<2)|(c2>>4))

iof_status base64_decode (iof *I, iof *O)
{
  register int c1, c2, c3, c4;
  while(iof_ensure(O, 3))
  {
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base64_eof(c1))
      return IOFEOF;
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base64_eof(c2))
      return IOFERR;
    do { c3 = iof_get(I); } while (ignored(c3));
    if (base64_eof(c3))
    {
      if ((c1 = base64_value(c1)) < 0 || (c2 = base64_value(c2)) < 0)
        return IOFERR;
      base64_decode_tail2(O, c1, c2);
      return IOFEOF;
    }
    do { c4 = iof_get(I); } while (ignored(c4));
    if (base64_eof(c4))
    {
      if ((c1 = base64_value(c1)) < 0 || (c2 = base64_value(c2)) < 0 || (c3 = base64_value(c3)) < 0)
        return IOFERR;
      base64_decode_tail3(O, c1, c2, c3);
      return IOFEOF;
    }
    if ((c1 = base64_value(c1)) < 0 || (c2 = base64_value(c2)) < 0 ||
        (c3 = base64_value(c3)) < 0 || (c4 = base64_value(c4)) < 0)
          return IOFERR;
    base64_decode_word(O, c1, c2, c3, c4);
  }
  return IOFFULL;
}

iof_status base64_decode_state (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3, c4;
  register int d1, d2, d3, d4;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
    case 3: get_tail3(state, c1, c2, c3); goto byte3;
  }
  while(iof_ensure(O, 3))
  {
    byte0:
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base64_eof(c1))
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base64_eof(c2))
    {
      set_tail1(state, c1); /* set tail to let the caller make padding or display invalid char in case of error */
      return (state->flush ? IOFERR : IOFEMPTY); /* if state->flush then error; tail must have at least two bytes */
    }
    byte2:
    do { c3 = iof_get(I); } while (ignored(c3));
    if (base64_eof(c3))
    {
      set_tail2(state, c1, c2);
      if (state->flush)
      {
        if ((c1 = base64_value(c1)) < 0 || (c2 = base64_value(c2)) < 0)
          return IOFERR;
        base64_decode_tail2(O, c1, c2);
        return IOFEOF;
      }
      else
        return IOFEMPTY;
    }
    byte3:
    do { c4 = iof_get(I); } while (ignored(c4));
    if (base64_eof(c4))
    {
      set_tail3(state, c1, c2, c3);
      if (state->flush)
      {
        if ((c1 = base64_value(c1)) < 0 || (c2 = base64_value(c2)) < 0 || (c3 = base64_value(c3)) < 0)
          return IOFERR;
        base64_decode_tail3(O, c1, c2, c3);
        return IOFEOF;
      }
      else
        return IOFEMPTY;
    }
    if ((d1 = base64_value(c1)) < 0 || (d2 = base64_value(c2)) < 0 ||
        (d3 = base64_value(c3)) < 0 || (d4 = base64_value(c4)) < 0)
    {
      set_tail4(state, c1, c2, c3, c4);
      return IOFERR;
    }
    base64_decode_word(O, d1, d2, d3, d4);
  }
  return IOFFULL;
}

/* base85 */

const char base85_alphabet[] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu"; /* for completness, not used below */

const int base85_lookup[] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
  31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,
  47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,
  63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,
  79,80,81,82,83,84,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

#define base85_value(c) base85_lookup[(uint8_t)(c)]

#define base85_encode_word(O, code) \
  (*(O->pos+4) = '!' + code%85, code /= 85, *(O->pos+3) = '!' + code%85, code /= 85, \
   *(O->pos+2) = '!' + code%85, code /= 85, *(O->pos+1) = '!' + code%85, code /= 85, \
   *(O->pos)   = '!' + code, \
   O->pos += 5)

#define base85_encode_tail3(O, code) \
  (*(O->pos+3) = '!' + code%85, code /= 85, *(O->pos+2) = '!' + code%85, code /= 85, \
   *(O->pos+1) = '!' + code%85, code /= 85, *(O->pos)   = '!' + code, \
   O->pos += 4)

#define base85_encode_tail2(O, code) \
  (*(O->pos+2) = '!' + code%85, code /= 85, *(O->pos+1) = '!' + code%85, code /= 85, \
   *(O->pos)   = '!' + code, \
   O->pos += 3)

#define base85_encode_tail1(O, code) \
  (*(O->pos+1) = '!' + code%85, code /= 85, *(O->pos)   = '!' + code, \
   O->pos += 2)

iof_status base85_encoded (const void *data, size_t size, iof *O)
{
  unsigned int code;
  const uint8_t *s, *e;
  uint8_t c1, c2, c3, c4;
  for (s = (const uint8_t *)data, e = s + size; s + 3 < e; )
  {
    if (!iof_ensure(O, 5))
      return IOFFULL;
    c1 = *s++;
    c2 = *s++;
    c3 = *s++;
    c4 = *s++;
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      iof_set(O, 'z');
      continue;
    }
    base85_encode_word(O, code);
  }
  switch (e - s)
  {
    case 0:
      break;
    case 1:
      if (!iof_ensure(O, 2))
        return IOFFULL;
      c1 = *s;
      code = (c1<<24)/85/85/85;
      base85_encode_tail1(O, code);
      break;
    case 2:
      if (!iof_ensure(O, 3))
        return IOFFULL;
      c1 = *s++;
      c2 = *s;
      code = ((c1<<24)|(c2<<16))/85/85;
      base85_encode_tail2(O, code);
      break;
    case 3:
      if (!iof_ensure(O, 4))
        return IOFFULL;
      c1 = *s++;
      c2 = *s++;
      c3 = *s;
      code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
      base85_encode_tail3(O, code);
      break;
  }
  return IOFEOF;
}

iof_status base85_encoded_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline)
{
  unsigned int code;
  const uint8_t *s, *e;
  uint8_t c1, c2, c3, c4;
  for (s = (const uint8_t *)data, e = s + size; s + 3 < e; )
  {
    if (!iof_ensure(O, 6))
      return IOFFULL;
    c1 = *s++;
    c2 = *s++;
    c3 = *s++;
    c4 = *s++;
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      put_nl(O, line, maxline, 1);
      iof_set(O, 'z');
      continue;
    }
    put_nl(O, line, maxline, 5);
    base85_encode_word(O, code);
  }
  switch (e - s)
  {
    case 0:
      break;
    case 1:
      if (!iof_ensure(O, 3))
        return IOFFULL;
      c1 = *s;
      code = (c1<<24)/85/85/85;
      put_nl(O, line, maxline, 2);
      base85_encode_tail1(O, code);
      break;
    case 2:
      if (!iof_ensure(O, 4))
        return IOFFULL;
      c1 = *s++;
      c2 = *s;
      code = ((c1<<24)|(c2<<16))/85/85;
      put_nl(O, line, maxline, 3);
      base85_encode_tail2(O, code);
      break;
    case 3:
      if (!iof_ensure(O, 5))
        return IOFFULL;
      c1 = *s++;
      c2 = *s++;
      c3 = *s;
      code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
      put_nl(O, line, maxline, 4);
      base85_encode_tail3(O, code);
      break;
  }
  return IOFEOF;
}

iof_status base85_encode (iof *I, iof *O)
{
  register int c1, c2, c3, c4;
  register unsigned int code;
  while(iof_ensure(O, 5))
  {
    if ((c1 = iof_get(I)) < 0)
      return IOFEOF;
    if ((c2 = iof_get(I)) < 0)
    {
      code = (c1<<24)/85/85/85;
      base85_encode_tail1(O, code);
      return IOFEOF;
    }
    if ((c3 = iof_get(I)) < 0)
    {
      code = ((c1<<24)|(c2<<16))/85/85;
      base85_encode_tail2(O, code);
      return IOFEOF;
    }
    if ((c4 = iof_get(I)) < 0)
    {
      code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
      base85_encode_tail3(O, code);
      return IOFEOF;
    }
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      iof_set(O, 'z');
      continue;
    }
    /* in btoa 'y' character stays for 0x20202020, but pdf does not support this */
    /* if (code == 0x20202020)
    {
      iof_set(O, 'y');
      continue;
    } */
    base85_encode_word(O, code);
  }
  return IOFFULL;
}

iof_status base85_encode_state (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3, c4;
  register unsigned int code;
  if (!(iof_ensure(O, 5)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
    case 3: get_tail3(state, c1, c2, c3); goto byte3;
  }
  while(iof_ensure(O, 5))
  {
    byte0:
    if ((c1 = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    if ((c2 = iof_get(I)) < 0)
    {
      set_tail1(state, c1);
      if (state->flush)
      {
        code = (c1<<24)/85/85/85;
        base85_encode_tail1(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte2:
    if ((c3 = iof_get(I)) < 0)
    {
      set_tail2(state, c1, c2);
      if (state->flush)
      {
        code = ((c1<<24)|(c2<<16))/85/85;
        base85_encode_tail2(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte3:
    if ((c4 = iof_get(I)) < 0)
    {
      set_tail3(state, c1, c2, c3);
      if (state->flush)
      {
        code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
        base85_encode_tail3(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      iof_set(O, 'z');
      continue;
    }
    base85_encode_word(O, code);
  }
  return IOFFULL;
}

iof_status base85_encode_ln (iof *I, iof *O, size_t line, size_t maxline)
{
  register int c1, c2, c3, c4;
  register unsigned int code;
  while(iof_ensure(O, 6))
  {
    if ((c1 = iof_get(I)) < 0)
      return IOFEOF;
    if ((c2 = iof_get(I)) < 0)
    {
      code = (c1<<24)/85/85/85;
      put_nl(O, line, maxline, 2);
      base85_encode_tail1(O, code);
      return IOFEOF;
    }
    if ((c3 = iof_get(I)) < 0)
    {
      code = ((c1<<24)|(c2<<16))/85/85;
      put_nl(O, line, maxline, 3);
      base85_encode_tail2(O, code);
      return IOFEOF;
    }
    if ((c4 = iof_get(I)) < 0)
    {
      code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
      put_nl(O, line, maxline, 4);
      base85_encode_tail3(O, code);
      return IOFEOF;
    }
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      put_nl(O, line, maxline, 1);
      iof_set(O, 'z');
      continue;
    }
    put_nl(O, line, maxline, 5);
    base85_encode_word(O, code);
  }
  return IOFFULL;
}

iof_status base85_encode_state_ln (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3, c4;
  register unsigned int code;
  if (!(iof_ensure(O, 6)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
    case 3: get_tail3(state, c1, c2, c3); goto byte3;
  }
  while(iof_ensure(O, 6))
  {
    byte0:
    if ((c1 = iof_get(I)) < 0)
      return (state->flush ? IOFEOF : IOFEMPTY);
    byte1:
    if ((c2 = iof_get(I)) < 0)
    {
      set_tail1(state, c1);
      if (state->flush)
      {
        code = (c1<<24)/85/85/85;
        put_nl(O, state->line, state->maxline, 2);
        base85_encode_tail1(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte2:
    if ((c3 = iof_get(I)) < 0)
    {
      set_tail2(state, c1, c2);
      if (state->flush)
      {
        code = ((c1<<24)|(c2<<16))/85/85;
        put_nl(O, state->line, state->maxline, 3);
        base85_encode_tail2(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte3:
    if ((c4 = iof_get(I)) < 0)
    {
      set_tail3(state, c1, c2, c3);
      if (state->flush)
      {
        code = ((c1<<24)|(c2<<16)|(c3<<8))/85;
        put_nl(O, state->line, state->maxline, 4);
        base85_encode_tail3(O, code);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    code = (c1<<24)|(c2<<16)|(c3<<8)|c4;
    if (code == 0)
    {
      put_nl(O, state->line, state->maxline, 1);
      iof_set(O, 'z');
      continue;
    }
    put_nl(O, state->line, state->maxline, 5);
    base85_encode_word(O, code);
  }
  return IOFFULL;
}

#define base85_code(c1, c2, c3, c4, c5) ((((c1*85+c2)*85+c3)*85+c4)*85+c5)

iof_status base85_decode (iof *I, iof *O)
{
  register int c1, c2, c3, c4, c5;
  register unsigned int code;
  while (iof_ensure(O, 4))
  {
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base85_eof(c1))
      return IOFEOF;
    switch (c1)
    {
      case 'z':
        iof_set4(O, '\0', '\0', '\0', '\0');
        continue;
      case 'y':
        iof_set4(O, ' ', ' ', ' ', ' ');
        continue;
    }
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base85_eof(c2))
      return IOFERR;
    do { c3 = iof_get(I); } while (ignored(c3));
    if (base85_eof(c3))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, 84, 84, 84); /* padding with 'u' (117); 117-33 = 84 */
      iof_set(O, code>>24);
      return IOFEOF;
    }
    do { c4 = iof_get(I); } while (ignored(c4));
    if (base85_eof(c4))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 || (c3 = base85_value(c3)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, c3, 84, 84);
      iof_set2(O, code>>24, (code>>16)&255);
      return IOFEOF;
    }
    do { c5 = iof_get(I); } while (ignored(c5));
    if (base85_eof(c5))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 ||
          (c3 = base85_value(c3)) < 0 || (c4 = base85_value(c4)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, c3, c4, 84);
      iof_set3(O, code>>24, (code>>16)&255, (code>>8)&255);
      return IOFEOF;
    }
    if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 || (c3 = base85_value(c3)) < 0 ||
        (c4 = base85_value(c4)) < 0 || (c5 = base85_value(c5)) < 0)
      return IOFERR;
    code = base85_code(c1, c2, c3, c4, c5);
    iof_set4(O, code>>24, (code>>16)&255, (code>>8)&255, code&255);
  }
  return IOFFULL;
}

iof_status base85_decode_state (iof *I, iof *O, basexx_state *state)
{
  register int c1, c2, c3, c4, c5;
  register int d1, d2, d3, d4, d5;
  register unsigned int code;
  if (!(iof_ensure(O, 4)))
    return IOFFULL;
  switch(state->left)
  {
    case 0: goto byte0;
    case 1: get_tail1(state, c1); goto byte1;
    case 2: get_tail2(state, c1, c2); goto byte2;
    case 3: get_tail3(state, c1, c2, c3); goto byte3;
    case 4: get_tail4(state, c1, c2, c3, c4); goto byte4;
  }
  while (iof_ensure(O, 4))
  {
    byte0:
    do { c1 = iof_get(I); } while (ignored(c1));
    if (base85_eof(c1))
      return (state->flush ? IOFEOF : IOFEMPTY);
    switch (c1)
    {
      case 'z':
        iof_set4(O, '\0', '\0', '\0', '\0');
        continue;
      case 'y':
        iof_set4(O, ' ', ' ', ' ', ' ');
        continue;
    }
    byte1:
    do { c2 = iof_get(I); } while (ignored(c2));
    if (base85_eof(c2))
    {
      set_tail1(state, c1);
      return (state->flush ? IOFERR : IOFEMPTY); /* if state->flush then error; tail must have at least two bytes */
    }
    byte2:
    do { c3 = iof_get(I); } while (ignored(c3));
    if (base85_eof(c3))
    {
      set_tail2(state, c1, c2);
      if (state->flush)
      {
        if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0)
          return IOFERR;
        code = base85_code(c1, c2, 84, 84, 84);
        iof_set(O, code>>24);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte3:
    do { c4 = iof_get(I); } while (ignored(c4));
    if (base85_eof(c4))
    {
      set_tail3(state, c1, c2, c3);
      if (state->flush)
      {
        if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 || (c3 = base85_value(c3)) < 0)
          return IOFERR;
        code = base85_code(c1, c2, c3, 84, 84);
        iof_set2(O, code>>24, (code>>16)&255);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    byte4:
    do { c5 = iof_get(I); } while (ignored(c5));
    if (base85_eof(c5))
    {
      set_tail4(state, c1, c2, c3, c4);
      if (state->flush)
      {
        if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 ||
            (c3 = base85_value(c3)) < 0 || (c4 = base85_value(c4)) < 0)
          return IOFERR;
        code = base85_code(c1, c2, c3, c4, 84);
        iof_set3(O, code>>24, (code>>16)&255, (code>>8)&255);
        return IOFEOF;
      }
      return IOFEMPTY;
    }
    if ((d1 = base85_value(c1)) < 0 || (d2 = base85_value(c2)) < 0 || (d3 = base85_value(c3)) < 0 ||
        (d4 = base85_value(c4)) < 0 || (d5 = base85_value(c5)) < 0)
    {
      set_tail5(state, c1, c2, c3, c4, c5);
      return IOFERR;
    }
    code = base85_code(d1, d2, d3, d4, d5);
    iof_set4(O, code>>24, (code>>16)&255, (code>>8)&255, code&255);
  }
  return IOFFULL;
}

/* postscript run length */

void runlength_state_init (runlength_state *state)
{
  state->run = -1;
  state->flush = 0;
  state->c1 = 0;
  state->c2 = 0;
  state->pos = NULL;
}

iof_status runlength_encode (iof *I, iof *O)
{
  register int c1, c2, run = -1;
  uint8_t *pos;
  c1 = 0, c2 = 0; /* avoid warning */
  while (iof_ensure(O, 1+128+1))
  { /* ensured space for single length byte, up to 128 bytes to be copied, possible eod marker */
    pos = O->pos++;
    switch (run)
    {
      case -1: /* initial state; get first byte */
        if ((c1 = iof_get(I)) < 0)
          return (*pos = 128, IOFEOF);
        run = 0;
        FALLTHRU // fall through
      case 0: /* `repeat' state; get another byte and compare */
        if ((c2 = iof_get(I)) < 0)
          return (*pos = 0, iof_set2(O, c1, 128), IOFEOF);
        run = (c1 == c2 ? 257-2 : 0);
        break;
    }
    if (run < 128)
    { /* single length byte, up to 128 bytes to be copied, possible eod marker */
      iof_set(O, c1);
      for (c1 = c2, c2 = iof_char(I); c1 != c2 && run < 127; c1 = c2, c2 = iof_next(I))
      {
        if (c2 < 0) /* O->pos must not change until next call to calling encoder!!! */
          return (*pos = (uint8_t)run+1, iof_set2(O, c1, 128), IOFEOF);
        iof_set(O, c1);
        ++run;
      }
    }
    else // if run > 128
    {
      for (c2 = iof_get(I); c1 == c2 && run > 129; c2 = iof_get(I))
        --run;
      if (c2 < 0)
        return (*pos = (uint8_t)run, iof_set2(O, c1, 128), IOFEOF);
      iof_set(O, c1);
    }
    *pos = (uint8_t)run;
    c1 = c2;
    run = 0;
  }
  return IOFFULL;
}

iof_status runlength_encode_state (iof *I, iof *O, runlength_state *state)
{
  while (iof_ensure(O, 3)) /* single length byte, the byte to be repeated and eod */
  {
    state->pos = O->pos++;
    switch (state->run)
    {
      case -1: /* initial state; get first byte */
        if ((state->c1 = iof_get(I)) < 0)
          return (state->flush ? (*state->pos = 128, IOFEOF) : IOFEMPTY);
        state->run = 0;
        FALLTHRU // fall through
      case 0: /* `repeat' state; get another byte and compare */
        if ((state->c2 = iof_get(I)) < 0)
          return (state->flush ? (*state->pos = 0, iof_set2(O, state->c1, 128), IOFEOF) : IOFEMPTY);
        state->run = (state->c1 == state->c2 ? 257-2 : 0);
        break;
    }
    if (state->run < 128)
    { /* ensure space for single length byte, up to 128 bytes to be copied, plus possible eod marker, minus those already copied */
      if (!iof_ensure(O, 1+128+1-state->run))
        return IOFFULL;
      iof_set(O, state->c1);
      for (state->c1 = state->c2, state->c2 = iof_char(I);
           state->c1 != state->c2 && state->run < 127;
           state->c1 = state->c2, state->c2 = iof_next(I))
      {
        if (state->c2 < 0) /* O->pos must not change until next call to calling encoder!!! */
          return (state->flush ? (*state->pos = (uint8_t)state->run+1, iof_set2(O, state->c1, 128), IOFEOF) : IOFEMPTY);
        iof_set(O, state->c1);
        ++state->run;
      }
    }
    else // if run > 128
    {
      for (state->c2 = iof_get(I); state->c1 == state->c2 && state->run > 129; state->c2 = iof_get(I))
        --state->run;
      if (state->c2 < 0)
        return (state->flush ? (*state->pos = (uint8_t)state->run, iof_set2(O, state->c1, 128), IOFEOF) : IOFEMPTY);
      iof_set(O, state->c1);
    }
    *state->pos = (uint8_t)state->run;
    state->c1 = state->c2;
    state->run = 0;
  }
  return IOFFULL;
}

iof_status runlength_decode (iof *I, iof *O)
{
  register int c, run = -1;
  while (1)
  {
    if (run == -1) /* initial state */
    {
      if ((run = iof_get(I)) < 0)
      {
        run = -1; /* don't assume IOFEOF == -1 */
        return IOFEOF;
      }
    }
    if (run < 128)
    { /* copy (run + 1) following bytes */
      while (run > -1)
      {
        if (iof_ensure(O, 1))
        {
          if ((c = iof_get(I)) < 0)
            return IOFERR;
          iof_set(O, c);
          --run;
          continue;
        }
        return IOFFULL;
      }
    }
    else if (run > 128)
    { /* replicate the following byte (257 - run) times */
      if ((c = iof_get(I)) < 0) /* cf. state-wise version; don't change input position until we got this byte */
        return IOFERR;
      while (run < 257)
      {
        if (iof_ensure(O, 1))
        {
          iof_set(O, c);
          ++run;
          continue;
        }
        return IOFFULL;
      }
      run = -1;
    }
    else // c == 128
      return IOFEOF;
  }
  // return IOFFULL;
}

iof_status runlength_decode_state (iof *I, iof *O, runlength_state *state)
{
  register int c;
  while (1)
  {
    if (state->run == -1) /* initial state */
    {
      if ((state->run = iof_char(I)) < 0)
      {
        state->run = -1; /* don't assume IOFEOF == -1 */
        return (state->flush ? IOFEOF : IOFEMPTY);
      }
      ++I->pos;
    }
    if (state->run < 128)
    { /* copy (state->run + 1) following bytes */
      while (state->run > -1)
      {
        if (iof_ensure(O, 1))
        {
          if ((c = iof_char(I)) < 0)
            return (state->flush ? IOFERR : IOFEMPTY);
          ++I->pos;
          iof_set(O, c);
          --state->run;
          continue;
        }
        return IOFFULL;
      }
    }
    else if (state->run > 128)
    { /* replicate the following byte (257 - state->run) times */
      if ((c = iof_char(I)) < 0)
        return (state->flush ? IOFERR : IOFEMPTY);
      ++I->pos;
      while (state->run < 257)
      {
        if (iof_ensure(O, 1))
        {
          iof_set(O, c);
          ++state->run;
          continue;
        }
        return IOFFULL;
      }
      state->run = -1;
    }
    else // c == 128
      return IOFEOF;
  }
  // return IOFFULL;
}

/* filters */

// base16 decoder function

static size_t base16_decoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;
  size_t tail;

  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      state = iof_filter_state(basexx_state *, F);
      do {
        status = base16_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "base16", status);
    case IOFCLOSE:
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// base16 encoder function

static size_t base16_encoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;

  state = iof_filter_state(basexx_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      FALLTHRU // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = base16_encode_state_ln(F, F->next, state);
      return iof_encoder_retval(F, "base16", status);
    case IOFCLOSE:
      if (!state->flush)
        base16_encoder(F, IOFFLUSH);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// base64 decoder function

static size_t base64_decoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;
  size_t tail;

  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      state = iof_filter_state(basexx_state *, F);
      do {
        status = base64_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "base64", status);
    case IOFCLOSE:
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// base64 encoder function

static size_t base64_encoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;

  state = iof_filter_state(basexx_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      FALLTHRU // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = base64_encode_state_ln(F, F->next, state);
      return iof_encoder_retval(F, "base64", status);
    case IOFCLOSE:
      if (!state->flush)
        base64_encoder(F, IOFFLUSH);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// base85 decoder function

static size_t base85_decoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;
  size_t tail;

  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      state = iof_filter_state(basexx_state *, F);
      do {
        status = base85_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "base85", status);
    case IOFCLOSE:
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// base85 encoder function

static size_t base85_encoder (iof *F, iof_mode mode)
{
  basexx_state *state;
  iof_status status;

  state = iof_filter_state(basexx_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      FALLTHRU // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = base85_encode_state_ln(F, F->next, state);
      return iof_encoder_retval(F, "base85", status);
    case IOFCLOSE:
      if (!state->flush)
        base85_encoder(F, IOFFLUSH);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// runlength decoder function

static size_t runlength_decoder (iof *F, iof_mode mode)
{
  runlength_state *state;
  iof_status status;
  size_t tail;

  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      state = iof_filter_state(runlength_state *, F);
      do {
        status = runlength_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "runlength", status);
    case IOFCLOSE:
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// runlength encoder function

static size_t runlength_encoder (iof *F, iof_mode mode)
{
  runlength_state *state;
  iof_status status;

  state = iof_filter_state(runlength_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      FALLTHRU // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = runlength_encode_state(F, F->next, state);
      return iof_encoder_retval(F, "runlength", status);
    case IOFCLOSE:
      if (!state->flush)
        runlength_encoder(F, IOFFLUSH);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

//

int iof_filter_basexx_encoder_ln (iof *F, size_t line, size_t maxline)
{
  basexx_state *state;
  if (maxline > 8 && line < maxline)
  {
    state = iof_filter_state(basexx_state *, F);
    state->line = line;
    state->maxline = maxline;
    return 1;
  }
  return 0;
}

/* base 16 */

iof * iof_filter_base16_decoder (iof *N)
{
  iof *I;
  basexx_state_pointer P;
  I = iof_filter_reader(base16_decoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(I, N);
  basexx_state_init(P.basexxstate);
  P.basexxstate->flush = 1; // means N is supposed to be continuous input
  return I;
}

iof * iof_filter_base16_encoder (iof *N)
{
  iof *O;
  basexx_state_pointer P;
  O = iof_filter_writer(base16_encoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(O, N);
  basexx_state_init(P.basexxstate);
  return O;
}

/* base 64 */

iof * iof_filter_base64_decoder (iof *N)
{
  iof *I;
  basexx_state_pointer P;
  I = iof_filter_reader(base64_decoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(I, N);
  basexx_state_init(P.basexxstate);
  P.basexxstate->flush = 1;
  return I;
}

iof * iof_filter_base64_encoder (iof *N)
{
  iof *O;
  basexx_state_pointer P;
  O = iof_filter_writer(base64_encoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(O, N);
  basexx_state_init(P.basexxstate);
  return O;
}

/* base 85 */

iof * iof_filter_base85_decoder (iof *N)
{
  iof *I;
  basexx_state_pointer P;
  I = iof_filter_reader(base85_decoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(I, N);
  basexx_state_init(P.basexxstate);
  P.basexxstate->flush = 1;
  return I;
}

iof * iof_filter_base85_encoder (iof *N)
{
  iof *O;
  basexx_state_pointer P;
  O = iof_filter_writer(base85_encoder, sizeof(basexx_state), &P.voidstate);
  iof_setup_next(O, N);
  basexx_state_init(P.basexxstate);
  return O;
}

/* runlength stream filter */

iof * iof_filter_runlength_decoder (iof *N)
{
  iof *I;
  basexx_state_pointer P;
  I = iof_filter_reader(runlength_decoder, sizeof(runlength_state), &P.voidstate);
  iof_setup_next(I, N);
  runlength_state_init(P.runlengthstate);
  P.runlengthstate->flush = 1;
  return I;
}

iof * iof_filter_runlength_encoder (iof *N)
{
  iof *O;
  basexx_state_pointer P;
  O = iof_filter_writer(runlength_encoder, sizeof(runlength_state), &P.voidstate);
  iof_setup_next(O, N);
  runlength_state_init(P.runlengthstate);
  return O;
}
