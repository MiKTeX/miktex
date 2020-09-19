
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include "pplib.h"

const char * ppobj_kind[] = { "none", "null", "bool", "integer", "number", "name", "string", "array", "dict", "stream", "ref" };

#define ignored_char(c) (c == 0x20 || c == 0x0A || c == 0x0D || c == 0x09 || c == 0x00)
#define newline_char(c) (c == 0x0A || c == 0x0D)
#define IGNORED_CHAR_CASE 0x20: case 0x0A: case 0x0D: case 0x09: case 0x00
#define NEWLINE_CHAR_CASE 0x0A: case 0x0D
#define DIGIT_CHAR_CASE '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9'
#define OCTAL_CHAR_CASE '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7'

#define MAX_INT_DIGITS 32

#define PP_LENGTH_UNKNOWN ((size_t)-1)

static const char * ppref_str (ppuint refnumber, ppuint refversion)
{
  static char buffer[MAX_INT_DIGITS + 1 + MAX_INT_DIGITS + 1 + 1 + 1];
  sprintf(buffer, "%lu %lu R", (unsigned long)(refnumber), (unsigned long)(refversion));
  return buffer;
}

/* name */

/*
pdf spec page 57:
"The name may include any regular characters, but not delimiter or white-space characters (see Section 3.1, �Lexical Conventions�)."
"The token / (a slash followed by no regular characters) is a valid name"
"Beginning with PDF 1.2, any character except null (character code 0) may be included in a name by writing its 2-digit hexadecimal code,
preceded by the number sign character (#); see implementation notes 3 and 4 in Appendix H. This syntax is required to represent any of the
delimiter or white-space characters or the number sign character itself; it is recommended but not required for characters whose codes
are outside the range 33 (!) to 126 (~)."

This suggests we should accept bytes 128..255 as a part of the name.
*/

// pdf name delimiters: 0..32, ()<>[]{}/%
// # treated specially
// .+- are valid part of name; keep in mind names such as -| | |- .notdef ABCDEF+Font etc.
static const int8_t ppname_byte_lookup[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, '#', 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/*
20190827: The end of the name is any byte with 0 lookup value. When reading a ref object or objstm stream containing
a single name, we may get input byte IOFEOF (-1), which must not be treated as 255. So a check for (c >= 0) is needed,
otherwise we keep writing byte 255 to the output buffer until not enough memory.
*/

#define ppnamebyte(c) (c >= 0 && ppname_byte_lookup[(uint8_t)(c)])

static const int8_t pphex_byte_lookup[] = {
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

/* no need for (c >= 0) check here */

#define pphex(c) pphex_byte_lookup[(uint8_t)(c)]

#define PPNAME_INIT (7 + 1)

static ppname * ppscan_name (iof *I, ppheap *heap)
{
  ppname *encoded, *decoded;
  iof *O;
  int decode, c;
  uint8_t *p, *e;
  int8_t h1, h2;

  O = ppbytes_buffer(heap, PPNAME_INIT);
  for (decode = 0, c = iof_char(I); ppnamebyte(c); c = iof_next(I))
  {
    if (c == '#') decode = 1;
    iof_put(O, c);
  }
  encoded = (ppname *)ppstruct_take(heap, sizeof(ppname));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);
  if (decode)
  {
    O = ppbytes_buffer(heap, encoded->size); // decoded always a bit smaller
    for (p = (uint8_t *)encoded->data, e = p + encoded->size; p < e; ++p)
    {
      if (*p == '#' && p + 2 < e && (h1 = pphex(p[1])) >= 0 && (h2 = pphex(p[2])) >= 0)
      {
        iof_set(O, ((h1 << 4)|h2));
        p += 2;
      }
      else
        iof_set(O, *p);
    }
    decoded = (ppname *)ppstruct_take(heap, sizeof(ppname));
    decoded->data = ppbytes_flush(heap, O, &decoded->size);
    encoded->flags = PPNAME_ENCODED;
    decoded->flags = PPNAME_DECODED;
    encoded->alterego = decoded, decoded->alterego = encoded;
  }
  else
  {
    encoded->flags = 0;
    encoded->alterego = encoded;
  }
  return encoded;
}

static ppname * ppscan_exec (iof *I, ppheap *heap, uint8_t firstbyte)
{
  ppname *encoded, *decoded;
  iof *O;
  int decode, c;
  uint8_t *p, *e;
  int8_t h1, h2;

  O = ppbytes_buffer(heap, PPNAME_INIT);
  iof_put(O, firstbyte);
  for (decode = 0, c = iof_char(I); ppnamebyte(c); c = iof_next(I))
  {
    if (c == '#') decode = 1;
    iof_put(O, c);
  }
  encoded = (ppname *)ppstruct_take(heap, sizeof(ppname));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);
  if (decode)
  {
    O = ppbytes_buffer(heap, encoded->size);
    for (p = (uint8_t *)encoded->data, e = p + encoded->size; p < e; ++p)
    {
      if (*p == '#' && p + 2 < e && (h1 = pphex(p[1])) >= 0 && (h2 = pphex(p[2])) >= 0)
      {
        iof_set(O, ((h1 << 4)|h2));
        p += 2;
      }
      else
        iof_set(O, *p);
    }
    decoded = (ppname *)ppstruct_take(heap, sizeof(ppname));
    decoded->data = ppbytes_flush(heap, O, &decoded->size);
    encoded->flags = PPNAME_EXEC|PPNAME_ENCODED;
    decoded->flags = PPNAME_EXEC|PPNAME_DECODED;
    encoded->alterego = decoded, decoded->alterego = encoded;
  }
  else
  {
    encoded->flags = PPNAME_EXEC;
    encoded->alterego = encoded;
  }
  return encoded;
}

static ppname * ppname_internal (const void *data, size_t size, int flags, ppheap *heap)
{ // so far needed only for 'EI' operator
  ppname *encoded;
  encoded = (ppname *)ppstruct_take(heap, sizeof(ppname));
  encoded->data = (ppbyte *)ppbytes_take(heap, size + 1);
  memcpy(encoded->data, data, size);
  encoded->data[size] = '\0';
  encoded->size = size;
  encoded->alterego = encoded;
  encoded->flags = flags;
  return encoded;
}

#define ppexec_internal(data, size, heap) ppname_internal(data, size, PPNAME_EXEC, heap)

ppname * ppname_decoded (ppname *name)
{
  return (name->flags & PPNAME_ENCODED) ? name->alterego : name;
}

ppname * ppname_encoded (ppname *name)
{
  return (name->flags & PPNAME_DECODED) ? name->alterego : name;
}

ppbyte * ppname_decoded_data (ppname *name)
{
  return (name->flags & PPNAME_ENCODED) ? name->alterego->data : name->data;
}

ppbyte * ppname_encoded_data (ppname *name)
{
  return (name->flags & PPNAME_DECODED) ? name->alterego->data : name->data;
}

/* string */

static const int8_t ppstring_byte_escape[] = { /* -1 escaped with octal, >0 escaped with \\, 0 left intact*/
 -1,-1,-1,-1,-1,-1,-1,-1,'b','t','n',-1,'f','r',-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  0, 0, 0, 0, 0, 0, 0, 0,'(',')', 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'\\', 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

//// pp string

#define PPSTRING_INIT (7 + 1)

#define ppstring_check_bom(decoded) ((void)\
  (decoded->size >= 2 ? (ppstring_utf16be_bom(decoded->data) ? (decoded->flags |= PPSTRING_UTF16BE) : \
                        (ppstring_utf16le_bom(decoded->data) ? (decoded->flags |= PPSTRING_UTF16LE) : 0)) : 0))

#define ppstring_check_bom2(decoded, encoded) ((void)\
  (decoded->size >= 2 ? (ppstring_utf16be_bom(decoded->data) ? ((decoded->flags |= PPSTRING_UTF16BE), (encoded->flags |= PPSTRING_UTF16BE)) : \
                        (ppstring_utf16le_bom(decoded->data) ? ((decoded->flags |= PPSTRING_UTF16LE), (encoded->flags |= PPSTRING_UTF16LE)) : 0)) : 0))

#define ppstring_utf16be_bom(data) (data[0] == '\xFE' && data[1] == '\xFF')
#define ppstring_utf16le_bom(data) (data[0] == '\xFF' && data[1] == '\xFE')

#define ppstringesc(c) ppstring_byte_escape[(uint8_t)(c)]

static ppstring * ppscan_string (iof *I, ppheap *heap)
{
  ppstring *encoded, *decoded;
  iof *O;
  int c, decode, balance;
  uint8_t *p, *e;

  O = ppbytes_buffer(heap, PPSTRING_INIT);
  for (decode = 0, balance = 0, c = iof_char(I); c >= 0; )
  {
    switch (c)
    {
      case '\\':
        decode = 1;
        iof_put(O, '\\');
        if ((c = iof_next(I)) >= 0)
        {
          iof_put(O, c);
          c = iof_next(I);
        }
        break;
      case '(': // may be unescaped if balanced
        ++balance;
        iof_put(O, '(');
        c = iof_next(I);
        break;
      case ')':
        if (balance == 0)
        {
          c = IOFEOF;
          ++I->pos;
          break;
        }
        --balance;
        iof_put(O, ')');
        c = iof_next(I);
        break;
      default:
        iof_put(O, c);
        c = iof_next(I);
    }
  }
  encoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);
  if (decode)
  {
    O = ppbytes_buffer(heap, encoded->size); // decoded can only be smaller
    for (p = (uint8_t *)encoded->data, e = p + encoded->size; p < e; ++p)
    {
      if (*p == '\\')
      {
        if (++p >= e)
          break;
        switch (*p)
        {
          case OCTAL_CHAR_CASE:
            c = *p - '0';
            if (++p < e && *p >= '0' && *p <= '7')
            {
              c = (c << 3) + *p - '0';
              if (++p < e && *p >= '0' && *p <= '7')
                c = (c << 3) + *p - '0';
            }
            iof_set(O, c);
            break;
          case 'n':
            iof_set(O, '\n');
            break;
          case 'r':
            iof_set(O, '\r');
            break;
          case 't':
            iof_set(O, '\t');
            break;
          case 'b':
            iof_set(O, '\b');
            break;
          case 'f':
            iof_set(O, '\f');
            break;
          case NEWLINE_CHAR_CASE: // not a part of the string, ignore (pdf spec page 55)
            break;
          case '(': case ')': case '\\':
          default: // for anything else backslash is ignored (pdf spec page 54)
            iof_set(O, *p);
            break;
        }
      }
      else
        iof_set(O, *p);
    }
    decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
    decoded->data = ppbytes_flush(heap, O, &decoded->size);
    encoded->flags = PPSTRING_ENCODED;
    decoded->flags = PPSTRING_DECODED;
    encoded->alterego = decoded, decoded->alterego = encoded;
    ppstring_check_bom2(decoded, encoded);
  }
  else
  {
    encoded->flags = 0;
    encoded->alterego = encoded;
    ppstring_check_bom(encoded);
  }
  return encoded;
}

static ppstring * ppscan_base16 (iof *I, ppheap *heap)
{
  ppstring *encoded, *decoded;
  iof *O;
  int c;
  uint8_t *p, *e;
  int8_t h1, h2;

  O = ppbytes_buffer(heap, PPSTRING_INIT);
  for (c = iof_char(I); (pphex(c) >= 0 || ignored_char(c)); c = iof_next(I))
    iof_put(O, c);
  if (c == '>')
    ++I->pos;
  encoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);

  O = ppbytes_buffer(heap, ((encoded->size + 1) >> 1) + 1); // decoded can only be smaller
  for (p = (uint8_t *)encoded->data, e = p + encoded->size; p < e; ++p)
  {
    if ((h1 = pphex(*p)) < 0) // ignored
      continue;
    for (h2 = 0, ++p; p < e && (h2 = pphex(*p)) < 0; ++p);
    iof_set(O, (h1 << 4)|h2);
  }
  decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  decoded->data = ppbytes_flush(heap, O, &decoded->size);

  encoded->flags = PPSTRING_BASE16|PPSTRING_ENCODED;
  decoded->flags = PPSTRING_BASE16|PPSTRING_DECODED;
  encoded->alterego = decoded, decoded->alterego = encoded;

  ppstring_check_bom2(decoded, encoded);
  return encoded;
}

static ppstring * ppstring_buffer (iof *O, ppheap *heap)
{
  ppstring *encoded, *decoded;
  uint8_t *p, *e;

  decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  decoded->data = ppbytes_flush(heap, O, &decoded->size);

  O = ppbytes_buffer(heap, (decoded->size << 1) + 1); // the exact size known
  for (p = (uint8_t *)decoded->data, e = p + decoded->size; p < e; ++p)
    iof_set2(O, base16_uc_alphabet[(*p) >> 4], base16_uc_alphabet[(*p) & 0xF]);
  encoded = ppstruct_take(heap, sizeof(ppstring));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);
  encoded->flags = PPSTRING_BASE16|PPSTRING_ENCODED;
  decoded->flags = PPSTRING_BASE16|PPSTRING_DECODED;
  encoded->alterego = decoded, decoded->alterego = encoded;
  // ppstring_check_bom2(decoded, encoded); // ?
  return encoded;
}

ppstring * ppstring_internal (const void *data, size_t size, ppheap *heap)
{ // so far used only for crypt key
  iof *O;
  O = ppbytes_buffer(heap, size);
  memcpy(O->buf, data, size);
  O->pos = O->buf + size;
  return ppstring_buffer(O, heap);
}

/* base85; local function for that to make that part independent from utilbasexx */

static const int8_t ppstring_base85_lookup[] = {
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

#define base85_value(c) ppstring_base85_lookup[(uint8_t)(c)]

#define base85_code(c1, c2, c3, c4, c5) ((((c1 * 85 + c2) * 85 + c3) * 85 + c4) * 85 + c5)
#define base85_eof(c) (c == '~' || c < 0)

static iof_status ppscan_base85_decode (iof *I, iof *O)
{
  int c1, c2, c3, c4, c5;
  uint32_t code;
  while (iof_ensure(O, 4))
  {
    do { c1 = iof_get(I); } while (ignored_char(c1));
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
    do { c2 = iof_get(I); } while (ignored_char(c2));
    if (base85_eof(c2))
      return IOFERR;
    do { c3 = iof_get(I); } while (ignored_char(c3));
    if (base85_eof(c3))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, 84, 84, 84); /* padding with 'u' (117); 117-33 = 84 */
      iof_set(O, (code >> 24));
      return IOFEOF;
    }
    do { c4 = iof_get(I); } while (ignored_char(c4));
    if (base85_eof(c4))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 || (c3 = base85_value(c3)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, c3, 84, 84);
      iof_set2(O, code>>24, ((code>>16) & 0xff));
      return IOFEOF;
    }
    do { c5 = iof_get(I); } while (ignored_char(c5));
    if (base85_eof(c5))
    {
      if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 ||
          (c3 = base85_value(c3)) < 0 || (c4 = base85_value(c4)) < 0)
        return IOFERR;
      code = base85_code(c1, c2, c3, c4, 84);
      iof_set3(O, (code >> 24), ((code >> 16) & 0xff), ((code >> 8) & 0xff));
      return IOFEOF;
    }
    if ((c1 = base85_value(c1)) < 0 || (c2 = base85_value(c2)) < 0 || (c3 = base85_value(c3)) < 0 ||
        (c4 = base85_value(c4)) < 0 || (c5 = base85_value(c5)) < 0)
      return IOFERR;
    code = base85_code(c1, c2, c3, c4, c5);
    iof_set4(O, (code >> 24), ((code >> 16) & 0xff), ((code >> 8) & 0xff), (code & 0xff));
  }
  return IOFFULL;
}

static ppstring * ppscan_base85 (iof *I, ppheap *heap)
{ // base85 alphabet is 33..117, adobe also hires 'z' and 'y' for compression
  ppstring *encoded, *decoded;
  iof *O, B;
  int c;

  O = ppbytes_buffer(heap, PPSTRING_INIT);
  for (c = iof_char(I); (c >= '!' && c <= 'u') || c == 'z' || c == 'y'; c = iof_next(I))
    iof_put(O, c);
  if (c == '~')
    if ((c = iof_next(I)) == '>')
      ++I->pos;
  encoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);

  iof_string_reader(&B, encoded->data, encoded->size);
  O = ppbytes_buffer(heap, (encoded->size * 5 / 4) + 1); // may be larger that that because of 'z' and 'y'
  ppscan_base85_decode(&B, O);
  decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  decoded->data = ppbytes_flush(heap, O, &decoded->size);

  encoded->flags = PPSTRING_BASE85|PPSTRING_ENCODED;
  decoded->flags = PPSTRING_BASE85|PPSTRING_DECODED;
  encoded->alterego = decoded, decoded->alterego = encoded;

  ppstring_check_bom2(decoded, encoded);
  return encoded;
}

ppstring * ppstring_decoded (ppstring *string)
{
  return (string->flags & PPSTRING_ENCODED) ? string->alterego : string;
}

ppstring * ppstring_encoded (ppstring *string)
{
  return (string->flags & PPSTRING_DECODED) ? string->alterego : string;
}

ppbyte * ppstring_decoded_data (ppstring *string)
{
  return (string->flags & PPSTRING_ENCODED) ? string->alterego->data : string->data;
}

ppbyte * ppstring_encoded_data (ppstring *string)
{
  return (string->flags & PPSTRING_DECODED) ? string->alterego->data : string->data;
}


/* encrypted string */

static ppstring * ppscan_crypt_string (iof *I, ppcrypt *crypt, ppheap *heap)
{
  ppstring *encoded, *decoded;
  iof *O;
  int c, b, balance, encode;
  uint8_t *p, *e;
  size_t size;

  O = ppbytes_buffer(heap, PPSTRING_INIT);
  for (balance = 0, encode = 0, c = iof_char(I); c >= 0; )
  {
    switch (c)
    {
      case '\\':
        if ((c = iof_next(I)) < 0)
          break;
        encode = 1;
        switch (c)
        {
          case OCTAL_CHAR_CASE:
            b = c - '0';
            if ((c = iof_next(I)) >= 0 && c >= '0' && c <= '7')
            {
              b = (b << 3) + c - '0';
              if ((c = iof_next(I)) >= 0 && c >= '0' && c <= '7')
              {
                b = (b << 3) + c - '0';
                c = iof_next(I);
              }
            }
            iof_put(O, b);
            // c is set to the next char
            break;
          case 'n':
            iof_put(O, '\n');
            c = iof_next(I);
            break;
          case 'r':
            iof_put(O, '\r');
            c = iof_next(I);
            break;
          case 't':
            iof_put(O, '\t');
            c = iof_next(I);
            break;
          case 'b':
            iof_put(O, '\b');
            c = iof_next(I);
            break;
          case 'f':
            iof_put(O, '\f');
            c = iof_next(I);
            break;
          case NEWLINE_CHAR_CASE: // not a part of the string, ignore (pdf spec page 55)
            c = iof_next(I);
            break;
          case '(': case ')': case '\\':
          default: // for enything else backslash is ignored (pdf spec page 54)
            iof_put(O, c);
            c = iof_next(I);
            break;
        }
        break;
      case '(':
        ++balance;
        encode = 1;
        iof_put(O, '(');
        c = iof_next(I);
        break;
      case ')':
        if (balance == 0)
        {
          c = IOFEOF;
          ++I->pos;
        }
        else
        {
          --balance;
          //encode = 1;
          iof_put(O, ')');
          c = iof_next(I);
        }
        break;
      default:
        if (ppstringesc(c) != 0)
          encode = 1;
        iof_put(O, c);
        c = iof_next(I);
    }
  }
  /* decrypt the buffer in place, update size */
  if (ppstring_decrypt(crypt, O->buf, iof_size(O), O->buf, &size))
    O->pos = O->buf + size;
  decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  decoded->data = ppbytes_flush(heap, O, &decoded->size);
  /* make encoded counterpart */
  if (encode)
  {
    O = ppbytes_buffer(heap, decoded->size + 1); // we don't know
    for (p = (uint8_t *)decoded->data, e = p + decoded->size; p < e; ++p)
    {
      b = ppstringesc(*p);
      switch (b)
      {
        case 0:
          iof_put(O, *p);
          break;
        case -1:
          iof_put4(O, '\\', ((*p) >> 6) + '0', (((*p) >> 3) & 7) + '0', ((*p) & 7) + '0');
          break;
        default:
          iof_put2(O, '\\', b);
          break;
      }
    }
    encoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
    encoded->data = ppbytes_flush(heap, O, &encoded->size);
    encoded->flags = PPSTRING_ENCODED;
    decoded->flags = PPSTRING_DECODED;
    encoded->alterego = decoded, decoded->alterego = encoded;
    ppstring_check_bom2(decoded, encoded);
  }
  else
  {
    decoded->flags = 0;
    decoded->alterego = decoded;
    ppstring_check_bom(decoded);
    encoded = decoded;
  }
  return encoded;
}

static ppstring * ppscan_crypt_base16 (iof *I, ppcrypt *crypt, ppheap *heap)
{
  ppstring *encoded, *decoded;
  iof *O;
  int c;
  uint8_t *p, *e;
  int8_t h1, h2;
  size_t size;

  O = ppbytes_buffer(heap, PPSTRING_INIT);
  for (c = iof_char(I); c != '>'; )
  {
    if ((h1 = pphex(c)) < 0)
    {
      if (ignored_char(c))
      {
        c = iof_next(I);
        continue;
      }
      break;
    }
    do {
      c = iof_next(I);
      if ((h2 = pphex(c)) >= 0)
      {
        c = iof_next(I);
        break;
      }
      if (!ignored_char(c)) // c == '>' || c < 0 or some crap
      {
        h2 = 0;
        break;
      }
    } while (1);
    iof_put(O, (h1 << 4)|h2);
  }
  if (c == '>')
    ++I->pos;
  /* decrypt the buffer in place, update size */
  if (ppstring_decrypt(crypt, O->buf, iof_size(O), O->buf, &size))
    O->pos = O->buf + size;
  decoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  decoded->data = ppbytes_flush(heap, O, &decoded->size);

  O = ppbytes_buffer(heap, (decoded->size << 1) + 1);
  for (p = (uint8_t *)decoded->data, e = p + decoded->size; p < e; ++p)
    iof_set2(O, base16_uc_alphabet[(*p) >> 4], base16_uc_alphabet[(*p) & 0xF]);
  encoded = (ppstring *)ppstruct_take(heap, sizeof(ppstring));
  encoded->data = ppbytes_flush(heap, O, &encoded->size);

  encoded->flags = PPSTRING_BASE16|PPSTRING_ENCODED;
  decoded->flags = PPSTRING_BASE16|PPSTRING_DECODED;
  encoded->alterego = decoded, decoded->alterego = encoded;

  ppstring_check_bom2(decoded, encoded);
  return encoded;
}

/* scanner stack */

#define PPSTACK_BUFFER 512

static void ppstack_init (ppstack *stack, ppheap *heap)
{
  stack->buf = stack->pos = (ppobj *)pp_malloc(PPSTACK_BUFFER * sizeof(ppobj));
  stack->size = 0;
  stack->space = PPSTACK_BUFFER;
  stack->heap = heap;
}

#define ppstack_free_buffer(stack) (pp_free((stack)->buf))

static void ppstack_resize (ppstack *stack)
{
  ppobj *newbuffer;
  stack->space <<= 1;
  newbuffer = (ppobj *)pp_malloc(stack->space * sizeof(ppobj));
  memcpy(newbuffer, stack->buf, stack->size * sizeof(ppobj));
  ppstack_free_buffer(stack);
  stack->buf = newbuffer;
  stack->pos = newbuffer + stack->size;
}

#define ppstack_push(stack) ((void)((stack)->size < (stack)->space || (ppstack_resize(stack), 0)), ++(stack)->size, (stack)->pos++)
#define ppstack_pop(stack, n) ((stack)->size -= (n), (stack)->pos -= (n))
#define ppstack_at(stack, i) ((stack)->buf + i)
#define ppstack_clear(stack) ((stack)->pos = (stack)->buf, (stack)->size = 0)

/* scanner commons */

#define ppscan_uint(I, u) iof_get_usize(I, u)
#define ppread_uint(s, u) string_to_usize((const char *)(s), u)

static ppobj * ppscan_numobj (iof *I, ppobj *obj, int negative)
{
  ppint integer;
  ppnum number;
  int exponent;
  int c;
  c = iof_char(I);
  iof_scan_integer(I, c, integer);
  switch(c)
  {
    case '.':
    {
      number = (ppnum)integer;
      c = iof_next(I);
      iof_scan_fraction(I, c, number, exponent);
      double_negative_exp10(number, exponent);
      obj->type = PPNUM, obj->number = negative ? -number : number;
      break;
    }
    default:
      obj->type = PPINT, obj->integer = negative ? -integer : integer;
      break;
  }
  return obj;
}

static ppobj * ppscan_numobj_frac (iof *I, ppobj *obj, int negative)
{
  ppnum number;
  int c, exponent;

  number = 0.0;
  c = iof_next(I);
  iof_scan_fraction(I, c, number, exponent);
  double_negative_exp10(number, exponent);
  obj->type = PPNUM, obj->number = negative ? -number : number;
  return obj;
}

static int ppscan_find (iof *I)
{ // skips whitechars and comments
  int c;
  for (c = iof_char(I); ; c = iof_next(I))
  {
    switch (c)
    {
      case IGNORED_CHAR_CASE:
        break;
      case '%': {
        do {
          if ((c = iof_next(I)) < 0)
            return c;
        } while (!newline_char(c));
        break;
      }
      default:
        return c;
    }
  }
  return c; // never reached
}

static int ppscan_keyword (iof *I, const char *keyword, size_t size)
{
  size_t i;
  int c;
  if ((size_t)iof_left(I) >= size)
  {
    if (memcmp(I->pos, keyword, size) != 0)
      return 0;
    I->pos += size;
    return 1;
  }
  // sticky case, we can't go back
  for (i = 0, c = iof_char(I); i < size; ++i, ++keyword, c = iof_next(I))
    if (c < 0 || *keyword != c) /* PJ20190503 bugfix: there was (i!=c), we actually never get here anyway */
      return 0;
  return 1;
}

#define ppscan_key(I, literal) ppscan_keyword(I, "" literal, sizeof(literal) - 1)

/* objects parser */

static ppref * ppref_unresolved (ppheap *heap, ppuint refnumber, ppuint refversion)
{
  ppref *ref = (ppref *)ppstruct_take(heap, sizeof(ppref));
  memset(ref, 0, sizeof(ppref));
  ref->object.type = PPNONE;
  ref->number = refnumber;
  ref->version = refversion;
  return ref;
}

#define PPMARK PPNONE

static ppobj * ppscan_obj (iof *I, ppdoc *pdf, ppxref *xref)
{
  int c;
  ppobj *obj;
  size_t mark, size;
  ppuint refnumber, refversion;
  ppref *ref;
  ppstack *stack;
  ppcrypt *crypt;

  stack = &pdf->stack;
  c = iof_char(I);
  switch (c)
  {
    case DIGIT_CHAR_CASE:
      return ppscan_numobj(I, ppstack_push(stack), 0);
    case '.':
      return ppscan_numobj_frac(I, ppstack_push(stack), 0);
    case '+':
      ++I->pos;
      return ppscan_numobj(I, ppstack_push(stack), 0);
    case '-':
      ++I->pos;
      return ppscan_numobj(I, ppstack_push(stack), 1);
    case '/':
      ++I->pos;
      obj = ppstack_push(stack);
      obj->type = PPNAME;
      obj->name = ppscan_name(I, &pdf->heap);
      return obj;
    case '(':
      ++I->pos;
      obj = ppstack_push(stack);
      obj->type = PPSTRING;
      if (ppcrypt_ref(pdf, crypt))
        obj->string = ppscan_crypt_string(I, crypt, &pdf->heap);
      else
        obj->string = ppscan_string(I, &pdf->heap);
      return obj;
    case '[':
      mark = stack->size;
      obj = ppstack_push(stack);
      obj->type = PPMARK; // ppscan_obj() checks types backward for 'R', so set the type immediatelly (reserved for PPARRAY)
      obj->any = NULL;
      ++I->pos;
      for (c = ppscan_find(I); c != ']'; c = ppscan_find(I))
      {
        if (ppscan_obj(I, pdf, xref) == NULL)
        { // callers assume that NULL returns means nothing pushed
          size = stack->size - mark; // pop items AND the obj reserved for array
          ppstack_pop(stack, size);
          return NULL;
        }
      }
      ++I->pos;
      size = stack->size - mark - 1;
      obj = ppstack_at(stack, mark); // stack might have been realocated
      obj->type = PPARRAY;
      obj->array = pparray_create(ppstack_at(stack, mark + 1), size, &pdf->heap);
      ppstack_pop(stack, size); // pop array items, leave the array on top
      return obj;
    case '<':
      if ((c = iof_next(I)) == '<')
      {
        mark = stack->size;
        obj = ppstack_push(stack);
        obj->type = PPMARK;
        obj->any = NULL;
        ++I->pos;
        for (c = ppscan_find(I); c != '>'; c = ppscan_find(I))
        {
          if (ppscan_obj(I, pdf, xref) == NULL)
          {
            size = stack->size - mark;
            ppstack_pop(stack, size);
            return NULL;
          }
        }
        if (iof_next(I) == '>')
          ++I->pos;
        size = stack->size - mark - 1;
        obj = ppstack_at(stack, mark);
        obj->type = PPDICT;
        obj->dict = ppdict_create(ppstack_at(stack, mark + 1), size, &pdf->heap);
        ppstack_pop(stack, size);
        return obj;
      }
      obj = ppstack_push(stack);
      obj->type = PPSTRING;
      if (ppcrypt_ref(pdf, crypt))
        obj->string = ppscan_crypt_base16(I, crypt, &pdf->heap);
      else
        obj->string = ppscan_base16(I, &pdf->heap);
      return obj;
    case 'R':
      if (stack->size >= 2 && stack->pos[-1].type == PPINT && stack->pos[-2].type == PPINT)
      {
        ++I->pos;
        obj = &stack->pos[-2];
        refnumber = (ppuint)obj->integer;
        ppstack_pop(stack, 1); // pop version number, retype obj to a reference
        if (xref == NULL || (ref = ppxref_find(xref, refnumber)) == NULL)
        { /* pdf spec page 64: unresolvable reference is not an error, should just be treated as a reference to null.
             we also need this to read trailer, where refs can't be resolved yet */
          refversion = (obj + 1)->integer;
          //if (xref != NULL)
          //  loggerf("unresolved reference %s", ppref_str(refnumber, refversion));
          ref = ppref_unresolved(stack->heap, refnumber, refversion);
        }
        obj->type = PPREF;
        obj->ref = ref;
        return obj;
      }
      break;
    case 't':
      if (iof_next(I) == 'r' && iof_next(I) == 'u' && iof_next(I) == 'e')
      {
        ++I->pos;
        obj = ppstack_push(stack);
        obj->type = PPBOOL;
        obj->integer = 1;
        return obj;
      }
      break;
    case 'f':
      if (iof_next(I) == 'a' && iof_next(I) == 'l' && iof_next(I) == 's' && iof_next(I) == 'e')
      {
        ++I->pos;
        obj = ppstack_push(stack);
        obj->type = PPBOOL;
        obj->integer = 0;
        return obj;
      }
      break;
    case 'n':
      if (iof_next(I) == 'u' && iof_next(I) == 'l' && iof_next(I) == 'l')
      {
        ++I->pos;
        obj = ppstack_push(stack);
        obj->type = PPNULL;
        obj->any = NULL;
        return obj;
      }
      break;
  }
  return NULL;
}

/*
A variant for contents streams (aka postscript); wise of operators, blind to references.
We are still PDF, so we don't care about postscript specific stuff such as radix numbers
and scientific numbers notation. It takes ppstack * as context (no ppdoc *) to be able
to run contents parser beyond the scope of ppdoc heap.
*/

static ppstring * ppstring_inline (iof *I, ppdict *imagedict, ppheap *heap);

static ppobj * ppscan_psobj (iof *I, ppstack *stack)
{
  int c;
  ppobj *obj, *op;
  size_t size, mark;
  ppname *exec;
  ppbyte *data;

  c = iof_char(I);
  switch (c)
  {
    case DIGIT_CHAR_CASE:
      return ppscan_numobj(I, ppstack_push(stack), 0);
    case '.':
      return ppscan_numobj_frac(I, ppstack_push(stack), 0);
    case '+':
      c = iof_next(I);
      if (base10_digit(c)) // '+.abc' is probably an executable name, but we are not in postscript
        return ppscan_numobj(I, ppstack_push(stack), 0);
      else if (c == '.')
        return ppscan_numobj_frac(I, ppstack_push(stack), 0);
      obj = ppstack_push(stack);
      obj->type = PPNAME;
      obj->name = ppscan_exec(I, stack->heap, '+');
      return obj;
    case '-':
      c = iof_next(I);
      if (base10_digit(c)) // ditto, we would handle type1 '-|' '|-' operators though
        return ppscan_numobj(I, ppstack_push(stack), 1);
      else if (c == '.')
        return ppscan_numobj_frac(I, ppstack_push(stack), 1);
      obj = ppstack_push(stack);
      obj->type = PPNAME;
      obj->name = ppscan_exec(I, stack->heap, '-');
      return obj;
    case '/':
      ++I->pos;
      obj = ppstack_push(stack);
      obj->type = PPNAME;
      obj->name = ppscan_name(I, stack->heap);
      return obj;
    case '(':
      ++I->pos;
      obj = ppstack_push(stack);
      obj->type = PPSTRING;
      obj->string = ppscan_string(I, stack->heap);
      return obj;
    case '[':
      mark = stack->size;
      obj = ppstack_push(stack);
      obj->type = PPMARK;
      obj->any = NULL;
      ++I->pos;
      for (c = ppscan_find(I); c != ']'; c = ppscan_find(I))
      {
        if (ppscan_psobj(I, stack) == NULL)
        {
          size = stack->size - mark;
          ppstack_pop(stack, size);
          return NULL;
        }
      }
      ++I->pos;
      size = stack->size - mark - 1;
      obj = ppstack_at(stack, mark);
      obj->type = PPARRAY;
      obj->array = pparray_create(ppstack_at(stack, mark + 1), size, stack->heap);
      ppstack_pop(stack, size);
      return obj;
    case '<':
      if ((c = iof_next(I)) == '<')
      {
        mark = stack->size;
        obj = ppstack_push(stack);
        obj->type = PPMARK;
        obj->any = NULL;
        ++I->pos;
        for (c = ppscan_find(I); c != '>'; c = ppscan_find(I))
        {
          if (ppscan_psobj(I, stack) == NULL)
          {
            size = stack->size - mark;
            ppstack_pop(stack, size);
            return NULL;
          }
        }
        if (iof_next(I) == '>')
          ++I->pos;
        size = stack->size - mark - 1;
        obj = ppstack_at(stack, mark);
        obj->type = PPDICT;
        obj->dict = ppdict_create(ppstack_at(stack, mark + 1), size, stack->heap);
        ppstack_pop(stack, size);
        return obj;
      }
      obj = ppstack_push(stack);
      obj->type = PPSTRING;
      if (c == '~')
        ++I->pos, obj->string = ppscan_base85(I, stack->heap);
      else
        obj->string = ppscan_base16(I, stack->heap);
      return obj;
    default:
      if (!ppnamebyte(c))
        break; // forbid empty names; dead loop otherwise
      ++I->pos;
      /* true false null practically don't occur in streams so it makes sense to assume that we get an operator name here.
         If it happen to be a keyword we could give back those several bytes to the heap but.. heap buffer is tricky enough. */
      exec = ppscan_exec(I, stack->heap, (uint8_t)c);
      data = exec->data;
      obj = ppstack_push(stack);
      switch (data[0])
      {
        case 't':
          if (data[1] == 'r' && data[2] == 'u' && data[3] == 'e' && data[4] == '\0')
          {
            obj->type = PPBOOL;
            obj->integer = 1;
            return obj;
          }
          break;
        case 'f':
          if (data[1] == 'a' && data[2] == 'l' && data[3] == 's' && data[4] == 'e' && data[5] == '\0')
          {
            obj->type = PPBOOL;
            obj->integer = 0;
            return obj;
          }
          break;
        case 'n':
          if (data[1] == 'u' && data[2] == 'l' && data[3] == 'l' && data[4] == '\0')
          {
            obj->type = PPNULL;
            obj->any = NULL;
            return obj;
          }
          break;
        case 'B':
           /*
           Inline images break rules of operand/operator syntax, so 'BI/ID' operators need to be treated as special syntactic keywords.

             BI <keyval pairs> ID<whitechar?><imagedata><whitechar?>EI

           We treat the image as a single syntactic token; BI starts collecting a dict, ID is the beginning of the data. Effectively EI
           operator obtains two operands - dict and string. It is ok to put three items onto the stack, callers dont't assume there is just one.
           */
          if (data[1] == 'I' && data[2] == '\0')
          {
            ppdict *imagedict;
            ppname *name;
            /* key val pairs -> dict */
            mark = stack->size - 1;
            obj->type = PPMARK;
            obj->any = NULL;
            for (c = ppscan_find(I); ; c = ppscan_find(I))
            {
              if ((op = ppscan_psobj(I, stack)) == NULL)
              {
                size = stack->size - mark;
                ppstack_pop(stack, size);
                return NULL;
              }
              if (op->type == PPNAME)
              {
                name = op->name;
                if (name->flags & PPNAME_EXEC)
                {
                  if (name->size != 2 || name->data[0] != 'I' || name->data[1] != 'D')
                  { // weird
                    size = stack->size - mark;
                    ppstack_pop(stack, size);
                    return NULL;
                  }
                  break;
                }
              }
            }
            size = stack->size - mark - 1;
            obj = ppstack_at(stack, mark);
            obj->type = PPDICT;
            obj->dict = imagedict = ppdict_create(ppstack_at(stack, mark + 1), size, stack->heap);
            ppstack_pop(stack, size);
            /* put image data string */
            obj = ppstack_push(stack);
            obj->type = PPSTRING;
            obj->string = ppstring_inline(I, imagedict, stack->heap);;
            /* put EI operator name */
            obj = ppstack_push(stack);
            obj->type = PPNAME;
            obj->name = ppexec_internal("EI", 2, stack->heap);
            return obj;
          }
          break;
      }
      obj->type = PPNAME;
      obj->name = exec;
      return obj;
  }
  return NULL;
}

/*
We try to get the exact inline image length from its dict params. If cannot predict the length, we have to scan the input until 'EI'.
I've checked on may examples that it gives the same results but one can never be sure, as 'EI' might happen to be a part of the data.
Stripping white char is also very heuristic; \0 is a white char in PDF and very likely to be a data byte.. weak method (pdf spec page 352).

Revision 20190327: inline images may be compressed, in which case we can't predict the length.
*/

static size_t inline_image_length (ppdict *dict)
{
  ppuint w, h, bpc, colors;
  ppname *cs;

  if (ppdict_get_name(dict, "F") == NULL)
  {
    if (ppdict_get_uint(dict, "W", &w) && ppdict_get_uint(dict, "H", &h) && ppdict_get_uint(dict, "BPC", &bpc) && (cs = ppdict_get_name(dict, "CS")) != NULL)
    {
      if (ppname_is(cs, "DeviceGray"))
        colors = 1;
      else if (ppname_is(cs, "DeviceRGB"))
        colors = 3;
      else if (ppname_is(cs, "DeviceCMYK"))
        colors = 4;
      else
        return PP_LENGTH_UNKNOWN;
      return (w * h * bpc * colors + 7) >> 3;
    }
  }
  return PP_LENGTH_UNKNOWN;
}

static ppstring * ppstring_inline (iof *I, ppdict *imagedict, ppheap *heap)
{
  iof *O;
  int c, d, e;
  size_t length, leftin, leftout, bytes;

  c = iof_char(I);
  if (ignored_char(c))
    c = iof_next(I);

  length = inline_image_length(imagedict);
  if (length != PP_LENGTH_UNKNOWN)
  {
    O = ppbytes_buffer(heap, length);
    while (length > 0 && iof_readable(I) && iof_writable(O))
    {
      leftin = iof_left(I);
      leftout = iof_left(O);
      bytes = length;
      if (bytes > leftin) bytes = leftin;
      if (bytes > leftout) bytes = leftout;
      memcpy(O->pos, I->pos, bytes);
      I->pos += bytes;
      O->pos += bytes;
      length -= bytes;
    }
    // gobble EI
    if (ppscan_find(I) == 'E')
      if (iof_next(I) == 'I')
        ++I->pos;
  }
  else
  {
    O = ppbytes_buffer(heap, PPSTRING_INIT); // ?
    while (c >= 0)
    {
      if (c == 'E')
      {
        d = iof_next(I);
        if (d == 'I')
        {
          e = iof_next(I);
          if (!ppnamebyte(e))
          { /* strip one newline from the end and stop */
            if (O->pos - 2 >= O->buf) // sanity
            {
              c = *(O->pos - 1);
              if (ignored_char(c))
              {
                if (c == 0x0A && *(O->pos - 2) == 0x0D)
                  O->pos -= 2;
                else
                  O->pos -= 1;
              }
            }
            break;
          }
          iof_put2(O, c, d);
          c = e;
        }
        else
        {
          iof_put(O, c);
          c = d;
        }
      }
      else
      {
        iof_put(O, c);
        c = iof_next(I);
      }
    }
  }
  return ppstring_buffer(O, heap);
}

/* input reader */

/*
PDF input is a pseudo file that either keeps FILE * or data. Reader iof * is a proxy to input
that provides byte-by-byte interface. Our iof structure is capable to link iof_file *input,
but t avoid redundant checks on IOF_DATA flag, here we link iof *I directly to FILE * or mem buffer.
When reading from file we need an internal buffer, which should be kept rather small, as it is
only used to parse xrefs and objects (no streams). We allocate the buffer from a private heap
(not static) to avoid conflicts when processing >1 pdfs at once. Besides, the input buffer may be
needed after loading the document, eg. to access references raw data.
*/

#define PPDOC_BUFFER 0xFFF // keep that small, it is only used to parse body objects

static void ppdoc_reader_init (ppdoc *pdf, iof_file *input)
{
  iof *I;
  pdf->input = *input;
  input = &pdf->input;
  input->refcount = 1;
  I = &pdf->reader;
  if (input->flags & IOF_DATA)
  {
    pdf->buffer = NULL;            // input iof_file is the buffer
    iof_string_reader(I, NULL, 0);
  }
  else
  {
    pdf->buffer = (uint8_t *)ppbytes_take(&pdf->heap, PPDOC_BUFFER);
    iof_setup_file_handle_reader(I, NULL, 0, iof_file_get_fh(input)); // gets IOF_FILE_HANDLE flag and FILE *
    I->space = PPDOC_BUFFER; // used on refill
  }
}

/*
Whenever we need to read the input file, we fseek the to the given offset and fread to the private buffer.
The length we need is not always predictable, in which case PPDOC_BUFFER bytes are read (keep it small).
I->buf = I->pos is set to the beginning, I->end set to the end (end is the first byte one shouldn't read).
*/

static iof * ppdoc_reader (ppdoc *pdf, size_t offset, size_t length)
{
  iof_file *input;
  iof *I;
  input = &pdf->input;
  I = &pdf->reader;
  if (iof_file_seek(input, (long)offset, SEEK_SET) != 0)
    return NULL;
  I->flags &= ~IOF_STOPPED;
  if (input->flags & IOF_DATA)
  {
    I->buf = I->pos = input->pos;
    I->end = (length == PP_LENGTH_UNKNOWN || I->pos + length >= input->end) ? input->end : (I->pos + length);
  }
  else
  {
    I->buf = I->pos = pdf->buffer; // ->buf is actually permanently equal pdf->buffer but we might need some tricks
    if (length == PP_LENGTH_UNKNOWN || length > PPDOC_BUFFER)
      length = PPDOC_BUFFER;
    length = fread(I->buf, 1, length, I->file);
    I->end = I->buf + length;
  }
  return I;
}

/* The position from the beginning of input
- for data buffer: (pdf->input.pos - pdf->input.buf) + (I->pos - I->buf)
  I->buf == pdf->input.pos, so this resolves to (I->pos - pdf->input.buf), independent from I->buf
- for file buffer: ftell(pdf->input.file) - (I->end - I->pos)
*/

#define ppdoc_reader_tell(pdf, I) ((size_t)(((pdf)->input.flags & IOF_DATA) ? ((I)->pos - (pdf)->input.buf) : (ftell(iof_file_get_fh(&(pdf)->input)) - ((I)->end - (I)->pos))))

/* pdf */

#define PPDOC_HEADER 10 // "%PDF-?.??\n"

static int ppdoc_header (ppdoc *pdf, uint8_t header[PPDOC_HEADER])
{
  size_t i;
  if (memcmp(header, "%PDF-", 5) != 0)
    return 0;
  for (i = 5; i < PPDOC_HEADER - 1 && !ignored_char(header[i]); ++i)
    pdf->version[i - 5] = header[i];
  pdf->version[i - 5] = '\0';
  return 1;
}

static int ppdoc_tail (ppdoc *pdf, iof_file *input, size_t *pxrefoffset)
{
  int c;
  uint8_t tail[4*10], *p, back, tailbytes;

  if (iof_file_seek(input, 0, SEEK_END) != 0)
    return 0;
  pdf->filesize = (size_t)iof_file_tell(input);
  // simple heuristic to avoid fgetc() / fseek(-2) hiccup: keep seeking back by len(startxref) + 1 == 10
  // until a letter found (assuming liberal white characters and tail length)
  for (back = 1, tailbytes = 0; ; ++back)
  {
    if (iof_file_seek(input, -10, SEEK_CUR) != 0)
      return 0;
    tailbytes += 10;
    c = iof_file_getc(input);
    tailbytes -= 1;
    switch (c)
    {
      case IGNORED_CHAR_CASE:
      case DIGIT_CHAR_CASE:
      case '%': case 'E': case 'O': case 'F':
        if (back > 4) // 2 should be enough
          return 0;
        continue;
      case 's': case 't': case 'a': case 'r': case 'x': case 'e': case 'f':
        if (iof_file_read(tail, 1, tailbytes, input) != tailbytes)
          return 0;
        tail[tailbytes] = '\0';
        for (p = &tail[0]; ; ++p)
        {
          if (*p == '\0')
            return 0;
          if ((c = base10_value(*p)) >= 0)
            break;
        }
        ppread_uint(p, pxrefoffset);
        return 1;
      default:
        return 0;
    }
  }
  return 0; // never reached
}

/* xref/body */

static int ppscan_start_entry (iof *I, ppref *ref)
{
  ppuint u;
  ppscan_find(I); if (!ppscan_uint(I, &u) || u != ref->number) return 0;
  ppscan_find(I); if (!ppscan_uint(I, &u) || u != ref->version) return 0;
  ppscan_find(I); if (!ppscan_key(I, "obj")) return 0;
  ppscan_find(I);
  return 1;
}

static int ppscan_skip_entry (iof *I)
{
  ppuint u;
  ppscan_find(I); if (!ppscan_uint(I, &u)) return 0;
  ppscan_find(I); if (!ppscan_uint(I, &u)) return 0;
  ppscan_find(I); if (!ppscan_key(I, "obj")) return 0;
  ppscan_find(I);
  return 1;
}

static int ppscan_start_stream (iof *I, ppdoc *pdf, size_t *streamoffset)
{
  int c;
  ppscan_find(I);
  if (ppscan_key(I, "stream"))
  { // PJ20180912 bugfix: we were gobbling white characters (also null byte), while "stream" may be followed by EOL
    // pdf spec page 60: "CARRIAGE RETURN and a LINE FEED or just a LINE FEED, and not by a CARRIAGE RETURN alone"
    c = iof_char(I);
    if (c == 0x0D)
    {
      if (iof_next(I) == 0x0A) // should be
        ++I->pos;
    }
    else if (c == 0x0A)
    {
      ++I->pos;
    }
    *streamoffset = ppdoc_reader_tell(pdf, I);
    return 1;
  }
  return 0;
}

static ppxref * ppxref_load (ppdoc *pdf, size_t xrefoffset);
static ppxref * ppxref_load_chain (ppdoc *pdf, ppxref *xref);

/* Parsing xref table

  1 10               // first ref number and refs count
  0000000000 00000 n // 10-digits offset, 5 digits version, type identifier
  0000000000 00000 n // n states for normal I guess
  0000000000 00000 f // f states for free (not used)
  ...

Free entries seem to be a relic of ancient times, completelly useless for us. To avoid parsing xref table twice,
we waste some space on free entries by allocating one plane of refs for each section. Later on we slice sections,
so that effectively free entries are not involved in map.

Subsequent refs gets number, version and offset. Other fields initialized when parsing PDF body.

Having xref table loaded, we sort sections for future binary search (xref with objects count == 0 is considered invalid).

Then we have to deal with the trailer dict. In general, to load objects and resolve references we need a complete chain
of xrefs (not only the top). To load the previous xref, we need its offset, which is given in trailer. So we have to
parse the trailer ignoring references, which might be unresolvable at this point (objects parser makes a dummy check
for xref != NULL on refs resolving ppscan_obj(), which irritates me but I don't want a separate parser for trailer..).
The same applies to xref streams, in which we have parse the trailer not having xref map at all. So the procedure is:

  - load xref map, initialize references, make it ready to search
  - parse trailer ignoring references
  - get /Prev xref offset and load older xref (linked list via ->prev)
  - sort all refs in all xrefs by offset
  - parse refs in order resolving references in contained objects
  - fix trailer references

First created xref becomes a pdf->xref (top xref). We link that early to control offsets already read (insane loops?).
*/

// Every xref table item "0000000000 00000 n" is said to be terminated with 2-byte EOL but we don't like relying on whites.
#define xref_item_length (10 + 1 + 5 + 1 + 1)

static ppxref * ppxref_load_table (iof *I, ppdoc *pdf, size_t xrefoffset)
{
  ppxref *xref;
  ppxsec *xrefsection;
  ppref *ref;
  ppuint first, count, refindex;
  uint8_t buffer[xref_item_length + 1];
  const char *p;
  const ppobj *obj;

  buffer[xref_item_length] = '\0';
  xref = ppxref_create(pdf, 0, xrefoffset);
  if (pdf->xref == NULL) pdf->xref = xref;

  for (ppscan_find(I); ppscan_uint(I, &first); ppscan_find(I))
  {
    ppscan_find(I);
    if (!ppscan_uint(I, &count))
      return NULL;
    if (count == 0) // weird
      continue;
    xref->count += count;
    xrefsection = NULL;
    ref = (ppref *)ppstruct_take(&pdf->heap, count * sizeof(ppref));
    for (refindex = 0; refindex < count; ++refindex, ++ref)
    {
      ref->xref = xref;
      ref->number = first + refindex;
      ppscan_find(I);
      iof_read(I, buffer, xref_item_length);
      switch (buffer[xref_item_length - 1])
      {
        case 'n':
          if (xrefsection == NULL)
          {
            xrefsection = ppxref_push_section(xref, &pdf->heap);
            xrefsection->first = ref->number;
            xrefsection->refs = ref;
          }
          xrefsection->last = ref->number;
          for (p = (const char *)buffer; *p == '0'; ++p);
          p = ppread_uint(p, &ref->offset);
          for ( ; *p == ' ' || *p == '0'; ++p);
          p = ppread_uint(p, &ref->version);
          ref->object.type = PPNONE; // init for sanity
          ref->object.any = NULL;
          ref->length = 0;
          break;
        case 'f':
        default:
          --ref;
          xrefsection = NULL;
          --xref->count;
      }
    }
  }
  /* sort section */
  ppxref_sort(xref); // case of xref->size == 0 handled by ppxref_load_chain()
  /* get trailer ignoring refs */
  if (!ppscan_key(I, "trailer"))
    return NULL;
  ppscan_find(I);
  if ((obj = ppscan_obj(I, pdf, NULL)) == NULL)
    return NULL;
  ppstack_pop(&pdf->stack, 1);
  if (obj->type != PPDICT)
    return NULL;
  xref->trailer = *obj;
  return ppxref_load_chain(pdf, xref);
}

/* Parsing xref stream
First we load the trailer, ignoring references. Dict defines sections and fields lengths:

  /Size                                  % max ref number plus 1
  /Index [ first count first count ... ] % a pair of numbers for every section, defaults to [0 Size]
  /W [w1 w2 w3]                          % fields lengths, 0 states for omitted field

xref stream data is a continuous stream of binary number triplets. First number is a type:

  0 - free entry (as 'f' in xref table)
  1 - normal entry, followed by offset an version (as 'n' in xref table)
  2 - compressed entry, followed by parent object stream number and entry index

0 and 1 are handled as 'n' and 'f' entries in xref table. For type 2 we normally initialize
ref->number and ref->version (the later is implicitly 0). ref->offset is set to 0 (invalid offset),
which is recognized by objects loader.
*/

#define XREF_STREAM_MAX_FIELD 4

static ppxref * ppxref_load_stream (iof *I, ppdoc *pdf, size_t xrefoffset)
{
  ppxref *xref;
  ppxsec *xrefsection;
  ppref *ref;
  ppobj *obj;
  ppstream *xrefstream;
  size_t streamoffset;
  ppuint w1, w2, w3, w, bufferbytes;
  uint8_t buffer[3 * XREF_STREAM_MAX_FIELD], *b;
  ppuint first, count, f1, f2, f3;
  pparray *fieldwidths, *sectionindices;
  ppobj sectionmock[2], *sectionfirst, *sectioncount;
  size_t sections, sectionindex, refindex;

  if (!ppscan_skip_entry(I))
    return NULL;
  if ((obj = ppscan_obj(I, pdf, NULL)) == NULL)
    return NULL;
  ppstack_pop(&pdf->stack, 1);
  if (obj->type != PPDICT || !ppscan_start_stream(I, pdf, &streamoffset))
    return NULL;
  xrefstream = ppstream_create(pdf, obj->dict, streamoffset);
  ppstream_info(xrefstream, pdf);
  if ((fieldwidths = ppdict_get_array(xrefstream->dict, "W")) != NULL)
  {
    if (!pparray_get_uint(fieldwidths, 0, &w1)) w1 = 0;
    if (!pparray_get_uint(fieldwidths, 1, &w2)) w2 = 0;
    if (!pparray_get_uint(fieldwidths, 2, &w3)) w3 = 0;
  }
  else
    w1 = w2 = w3 = 0;
  if (w1 > XREF_STREAM_MAX_FIELD || w2 > XREF_STREAM_MAX_FIELD || w3 > XREF_STREAM_MAX_FIELD)
    return NULL;
  bufferbytes = w1 + w2 + w3;
  if ((sectionindices = ppdict_get_array(xrefstream->dict, "Index")) != NULL)
  {
    sections = sectionindices->size >> 1;
    sectionfirst = sectionindices->data;
  }
  else
  {
    sections = 1;
    sectionmock[0].type = PPINT;
    sectionmock[0].integer = 0;
    sectionmock[1].type = PPINT;
    if (!ppdict_get_int(xrefstream->dict, "Size", &sectionmock[1].integer))
      sectionmock[1].integer = 0;
    sectionfirst = &sectionmock[0];
  }
  if ((I = ppstream_read(xrefstream, 1, 0)) == NULL)
    return NULL; // we fseek() so original I is useless anyway
  xref = ppxref_create(pdf, sections, xrefoffset);
  if (pdf->xref == NULL) pdf->xref = xref;
  xref->trailer.type = PPSTREAM;
  xref->trailer.stream = xrefstream;
  for (sectionindex = 0; sectionindex < sections; ++sectionindex, sectionfirst += 2)
  {
    sectioncount = sectionfirst + 1;
    first = 0, count = 0; // warnings
    if (!ppobj_get_uint(sectionfirst, first) || !ppobj_get_uint(sectioncount, count))
      goto xref_stream_error;
    if (count == 0)
      continue;
    xref->count += count;
    xrefsection = NULL;
    ref = (ppref *)ppstruct_take(&pdf->heap, count * sizeof(ppref));
    for (refindex = 0; refindex < count; ++refindex, ++ref)
    {
      ref->xref = xref;
      ref->number = first + refindex;
      if (iof_read(I, buffer, bufferbytes) != bufferbytes)
        goto xref_stream_error;
      b = buffer;
      if (w1 == 0)
        f1 = 1; // default type is 1
      else
        for (f1 = 0, w = 0; w < w1; f1 = (f1 << 8)|(*b), ++w, ++b);
      for (f2 = 0, w = 0; w < w2; f2 = (f2 << 8)|(*b), ++w, ++b);
      for (f3 = 0, w = 0; w < w3; f3 = (f3 << 8)|(*b), ++w, ++b);
      switch (f1)
      {
        case 0:
          //--ref;
          xrefsection = NULL;
          --xref->count;
          break;
        case 1:
          if (xrefsection == NULL)
          {
            xrefsection = ppxref_push_section(xref, &pdf->heap);
            xrefsection->first = ref->number;
            xrefsection->refs = ref;
          }
          xrefsection->last = ref->number;
          ref->offset = f2;
          ref->version = f3;
          ref->object.type = PPNONE;
          ref->object.any = NULL;
          ref->length = 0;
          break;
        case 2:
          if (xrefsection == NULL)
          {
            xrefsection = ppxref_push_section(xref, &pdf->heap);
            xrefsection->first = ref->number;
            xrefsection->refs = ref;
          }
          xrefsection->last = ref->number;
          ref->offset = 0; // f2 is parent objstm, f3 is index in parent, both useless
          ref->version = 0; // compressed objects has implicit version == 0
          ref->object.type = PPNONE;
          ref->object.any = NULL;
          ref->length = 0;
          break;
        default:
          goto xref_stream_error;
      }
    }
  }
  /* sort sections */
  ppxref_sort(xref); // case of xref->size == 0 handled by ppxref_load_chain()
  /* close the stream _before_ loading prev xref */
  ppstream_done(xrefstream);
  /* load prev and return */
  return ppxref_load_chain(pdf, xref);
xref_stream_error:
  ppstream_done(xrefstream);
  return NULL;
}

/*
The following procedure loads xref /Prev, links xref->prev and typically returns xref.
Some docs contain empty xref (one section with zero objects) that is actually a proxy
to xref stream referred as /XRefStm (genuine concept of xrefs old/new style xrefs in
the same doc). In case of 0-length xref we ignore the proxy and return the target xref
(otherwise we would need annoying sanity check for xref->size > 0 on every ref search).
*/

static ppxref * ppxref_load_chain (ppdoc *pdf, ppxref *xref)
{
  ppdict *trailer;
  ppuint xrefoffset;
  ppxref *prevxref, *nextxref;

  trailer = ppxref_trailer(xref);
  if (!ppdict_get_uint(trailer, "Prev", &xrefoffset)) // XRefStm is useless
    return xref;
  for (nextxref = pdf->xref; nextxref != NULL; nextxref = nextxref->prev)
    if (nextxref->offset == xrefoffset) // insane
      return NULL;
  if ((prevxref = ppxref_load(pdf, (size_t)xrefoffset)) == NULL)
    return NULL;
  if (xref->size > 0)
  {
    xref->prev = prevxref;
    return xref;
  }
  if (pdf->xref == xref)
    pdf->xref = prevxref;
  return prevxref;
}

static ppxref * ppxref_load (ppdoc *pdf, size_t xrefoffset)
{
  iof *I;

  if ((I = ppdoc_reader(pdf, xrefoffset, PP_LENGTH_UNKNOWN)) == NULL)
    return NULL;
  ppscan_find(I);
  if (ppscan_key(I, "xref"))
    return ppxref_load_table(I, pdf, xrefoffset);
  return ppxref_load_stream(I, pdf, xrefoffset);
  // iof_close(I) does nothing here
}

static void ppoffmap_sort (ppref **left, ppref **right)
{
  ppref **l, **r, *t;
  ppuint pivot;
  l = left, r = right;
  pivot = (*(l + ((r - l) / 2)))->offset;
  do
  { // don't read from pointer!
    while ((*l)->offset < pivot) ++l;
    while ((*r)->offset > pivot) --r;
    if (l <= r)
    {
      t = *l;
      *l = *r;
      *r = t;
      ++l, --r;
    }
  } while (l <= r);
  if (left < r)
    ppoffmap_sort(left, r);
  if (l < right)
    ppoffmap_sort(l, right);
}


static void fix_trailer_references (ppdoc *pdf)
{
  ppxref *xref;
  ppdict *trailer;
  ppname **pkey;
  ppobj *obj;
  ppref *ref;
  for (xref = pdf->xref; xref != NULL; xref = xref->prev)
  {
    if ((trailer = ppxref_trailer(xref)) == NULL)
      continue;
    for (ppdict_first(trailer, pkey, obj); *pkey != NULL; ppdict_next(pkey, obj))
    { // no need to go deeper in structs, all items in trailer except info and root must be direct refs
      if (obj->type != PPREF)
        continue;
      ref = obj->ref;
      if (ref->offset == 0) // unresolved?
        if ((ref = ppxref_find(xref, ref->number)) != NULL)
          obj->ref = ref; // at this moment the reference still points nothing, but should be the one with the proper offset
    }
  }
}

/*
Here comes a procedure that loads all entries from all document bodies. We resolve references while
parsing objects and to make resolving correct, we need a complete chain of xref maps, and a knowledge
about possible linearized dict (first offset). So loading refs sorted by offsets makes sense (not sure
if it matters nowadays but we also avoid fseek() by large offsets).

Here is the proc:

  - create a list of all refs in all bodies
  - sort the list by offsets
  - for every ref from the sorted list:
    - estimate object length to avoid fread-ing more than necessary (not perfect but enough)
    - fseek() to the proper offset, fread() entry data or its part
    - parse the object with ppscan_obj(I, pdf, xref), where xref is not necessarily top pdf->xref
      (since v0.98 xref actually no longer matters, see xref_find() notes)
    - save the actual ref->length (not used so far, but we keep that so..)
    - make a stream if a dict is followed by "stream" keyword, also save the stream offset
  - free the list

PJ2080916: Luigi and Hans fixeed a bug (rev 6491); a document having a stream with /Length being
a reference, that was stored in /ObjStm, and therefore not yet resolved when caching /Length key
value as stream->offset (ppstream_info()). At the end, references were resolved propertly, but
the stream was no readable; stream->offset == 0. In rev6491 ObjStm streams are loaded before
others streams.
*/

static int ppdoc_load_objstm (ppstream *stream, ppdoc *pdf, ppxref *xref);

#define ppref_is_objstm(ref, stream, type) \
  ((ref)->xref->trailer.type == PPSTREAM && (type = ppdict_get_name((stream)->dict, "Type")) != NULL && ppname_is(type, "ObjStm"))


static void ppdoc_load_entries (ppdoc *pdf)
{
  size_t objects, sectionindex, refnumber, offindex;
  size_t streams = 0, object_streams = 0, redundant_indirections = 0;
  ppnum linearized;
  ppref **offmap, **pref, *ref;
  ppxref *xref;
  ppxsec *xsec;
  ppobj *obj;
  ppname *type;
  ppcrypt *crypt;
  ppstream *stream;

  if ((objects = (size_t)ppdoc_objects(pdf)) == 0) // can't happen
    return;
  pref = offmap = (ppref **)pp_malloc(objects * sizeof(ppref *));
  objects = 0; // recount refs with offset > 0
  for (xref = pdf->xref; xref != NULL; xref = xref->prev)
    for (sectionindex = 0, xsec = xref->sects; sectionindex < xref->size; ++sectionindex, ++xsec)
      for (refnumber = xsec->first, ref = xsec->refs; refnumber <= xsec->last; ++refnumber, ++ref)
        if (ref->offset > 0) // 0 means compressed or insane
          *pref++ = ref, ++objects;
  ppoffmap_sort(offmap, offmap + objects - 1);

  crypt = pdf->crypt;
  for (offindex = 0, pref = offmap; offindex < objects; )
  {
    ref = *pref;
    ++pref;
    ++offindex;
    if (ref->object.type != PPNONE) // might be preloaded already (/Encrypt dict, stream filter dicts, stream /Length..)
      continue;
    if (offindex < objects)
      ref->length = (*pref)->offset - ref->offset;
    else
      ref->length = pdf->filesize > ref->offset ? pdf->filesize - ref->offset : 0;
    if (crypt != NULL)
    {
      ppcrypt_start_ref(crypt, ref);
      obj = ppdoc_load_entry(pdf, ref);
      ppcrypt_end_ref(crypt);
    }
    else
    {
      obj = ppdoc_load_entry(pdf, ref);
    }
    switch (obj->type)
    {
      case PPDICT: /* Check if the object at first offset is linearized dict. We need that to resolve all references properly. */
        if (offindex == 1 && ppdict_get_num(obj->dict, "Linearized", &linearized)) // /Linearized value is a version number, default 1.0
          pdf->flags |= PPDOC_LINEARIZED;
        break;
      case PPSTREAM:
        ++streams;
        if (ppref_is_objstm(ref, obj->stream, type))
          ++object_streams;
        break;
      case PPREF:
        ++redundant_indirections;
        break;
      default:
        break;
    }
  }

  /* cut references pointing to references (rare). doing for all effectively cuts all insane chains */
  for (pref = offmap; redundant_indirections > 0; )
  {
    ref = *pref++;
    if (ref->object.type == PPREF)
    {
      --redundant_indirections;
      ref->object = ref->object.ref->object;
    }
  }

  /* load pdf 1.5 object streams _before_ other streams */
  for (pref = offmap; object_streams > 0; )
  {
    ref = *pref++;
    obj = &ref->object;
    if (obj->type != PPSTREAM)
      continue;
    stream = obj->stream;
    if (ppref_is_objstm(ref, stream, type))
    {
      --object_streams;
      if (crypt != NULL)
       {
         ppcrypt_start_ref(crypt, ref);
         ppstream_info(stream, pdf);
         ppcrypt_end_ref(crypt);
       }
       else
       {
         ppstream_info(stream, pdf);
       }
       if (!ppdoc_load_objstm(stream, pdf, ref->xref))
         loggerf("invalid objects stream %s at offset " PPSIZEF, ppref_str(ref->number, ref->version), ref->offset);

    }
  }

  /* now handle other streams */
  for (pref = offmap; streams > 0; )
  {
    ref = *pref++;
    obj = &ref->object;
    if (obj->type != PPSTREAM)
      continue;
    --streams;
    stream = obj->stream;
    if (crypt != NULL)
    {
      ppcrypt_start_ref(crypt, ref);
      ppstream_info(stream, pdf);
      ppcrypt_end_ref(crypt);
    }
    else
    {
      ppstream_info(stream, pdf);
    }
  }
  pp_free(offmap);
}

ppobj * ppdoc_load_entry (ppdoc *pdf, ppref *ref)
{
  iof *I;
  size_t length;
  ppxref *xref;
  ppobj *obj;
  ppstack *stack;
  size_t streamoffset;
  ppref *refref;
  ppuint refnumber, refversion;

  length = ref->length > 0 ? ref->length : PP_LENGTH_UNKNOWN; // estimated or unknown
  if ((I = ppdoc_reader(pdf, ref->offset, length)) == NULL || !ppscan_start_entry(I, ref))
  {
    loggerf("invalid %s offset " PPSIZEF, ppref_str(ref->number, ref->version), ref->offset);
    return &ref->object; // PPNONE
  }
  stack = &pdf->stack;
  xref = ref->xref;
  if ((obj = ppscan_obj(I, pdf, xref)) == NULL)
  {
    loggerf("invalid %s object at offset " PPSIZEF, ppref_str(ref->number, ref->version), ref->offset);
    return &ref->object; // PPNONE
  }
  ref->object = *obj;
  ppstack_pop(stack, 1);
  obj = &ref->object;
  ref->length = ppdoc_reader_tell(pdf, I) - ref->offset;
  if (obj->type == PPDICT)
  {
    if (ppscan_start_stream(I, pdf, &streamoffset))
    {
      obj->type = PPSTREAM;
      obj->stream = ppstream_create(pdf, obj->dict, streamoffset);
    }
  }
  else if (obj->type == PPINT)
  {
    ppscan_find(I);
    if (ppscan_uint(I, &refversion) && ppscan_find(I) == 'R')
    {
      refnumber = (ppuint)obj->integer;
      if ((refref = ppxref_find(xref, refnumber)) != NULL)
      {
        obj->type = PPREF;
        obj->ref = refref;
      }
      else
      {
        obj->type = PPNONE; // as ppref_unresolved()
        obj->any = NULL;
      }
    }
  }
  return obj;
}

/* Loading entries from object stream

  /N is the number of contained entries
  /First is the offset of the first item

The stream consists of N pairs of numbers <objnum> <offset> <objnum> <offset> ...
Offsets are ascending (relative to the first), but ref numbers order is arbitrary.
PDF spec says there might be some additional data between objects, so we should obey offsets.
Which means we should basically load the stream at once (may be needed anyway to grab the stream [...]).
*/

static int ppdoc_load_objstm (ppstream *stream, ppdoc *pdf, ppxref *xref)
{
  ppdict *dict; // stream dict, actually still on stack
  ppref *ref;
  ppobj *obj;
  ppuint items, firstoffset, offset, objnum, i, invalid = 0;
  iof *I;
  uint8_t *firstdata, *indexdata;
  ppstack *stack;

  dict = stream->dict;
  if (!ppdict_rget_uint(dict, "N", &items) || !ppdict_rget_uint(dict, "First", &firstoffset))
    return 0;
  if ((I = ppstream_read(stream, 1, 1)) == NULL)
    return 0;
  firstdata = I->pos + firstoffset;
  if (firstdata >= I->end)
    goto invalid_objstm;
  stack = &pdf->stack;
  //if (pdf->crypt != NULL)
  //  ppcrypt_end_ref(pdf->crypt); // objects are not encrypted, pdf->crypt->ref ensured NULL
  for (i = 0; i < items; ++i)
  {
    ppscan_find(I);
    if (!ppscan_uint(I, &objnum))
      goto invalid_objstm;
    ppscan_find(I);
    if (!ppscan_uint(I, &offset))
      goto invalid_objstm;
    if ((ref = ppxref_find_local(xref, objnum)) == NULL || ref->object.type != PPNONE)
    {
      loggerf("invalid compressed object number " PPUINTF " at position " PPUINTF, objnum, i);
      ++invalid;
      continue;
    }
    if (firstdata + offset >= I->end)
    {
      loggerf("invalid compressed object offset " PPUINTF " at position " PPUINTF, offset, i);
      ++invalid;
      continue;
    }
    indexdata = I->pos; // save position
    I->pos = firstdata + offset; // go to the object
    ppscan_find(I);
    if ((obj = ppscan_obj(I, pdf, xref)) != NULL)
    {
      ref->object = *obj;
      ppstack_pop(stack, 1);
      // nothing more needed, as obj can never be indirect ref or stream
    }
    else
    {
      ++invalid;
      loggerf("invalid compressed object %s at stream offset " PPUINTF, ppref_str(objnum, 0), offset);
    }
    I->pos = indexdata; // restore position and read next from index
  }
  ppstream_done(stream);
  return invalid == 0;
invalid_objstm:
  ppstream_done(stream);
  return 0;
}

/* main PDF loader proc */

ppcrypt_status ppdoc_crypt_pass (ppdoc *pdf, const void *userpass, size_t userpasslength, const void *ownerpass, size_t ownerpasslength)
{
  switch (pdf->cryptstatus)
  {
    case PPCRYPT_NONE:
    case PPCRYPT_DONE:
    case PPCRYPT_FAIL:
      break;
    case PPCRYPT_PASS: // initial status or really needs password
      pdf->cryptstatus = ppdoc_crypt_init(pdf, userpass, userpasslength, ownerpass, ownerpasslength);
      switch (pdf->cryptstatus)
      {
        case PPCRYPT_NONE:
        case PPCRYPT_DONE:
          ppdoc_load_entries(pdf);
          break;
        case PPCRYPT_PASS: // user needs to check ppdoc_crypt_status() and recall ppdoc_crypt_pass() with the proper password
        case PPCRYPT_FAIL: // hopeless..
          break;
      }
      break;
  }
  return pdf->cryptstatus;
}

static ppdoc * ppdoc_read (ppdoc *pdf, iof_file *input)
{
  uint8_t header[PPDOC_HEADER];
  size_t xrefoffset;

  input = &pdf->input;
  if (iof_file_read(header, 1, PPDOC_HEADER, input) != PPDOC_HEADER || !ppdoc_header(pdf, header))
    return NULL;
  if (!ppdoc_tail(pdf, input, &xrefoffset))
    return NULL;
  if (ppxref_load(pdf, xrefoffset) == NULL)
    return NULL;
  fix_trailer_references(pdf); // after loading xrefs but before accessing trailer refs (/Encrypt might be a reference)
  // check encryption, if any, try empty password
  switch (ppdoc_crypt_pass(pdf, "", 0, NULL, 0))
  {
    case PPCRYPT_NONE: // no encryption
    case PPCRYPT_DONE: // encryption with an empty password
    case PPCRYPT_PASS: // the user needs to check ppdoc_crypt_status() and call ppdoc_crypt_pass()
      break;
    case PPCRYPT_FAIL: // hopeless
      //loggerf("decryption failed");
      //return NULL;
      break;
  }
  return pdf;
}

static void ppdoc_pages_init (ppdoc *pdf);

/*
20191214: We used to allocate ppdoc, as all other structs, from the internal heap:

  ppheap heap;
  ppheap_init(&heap);
  pdf = (ppdoc *)ppstruct_take(&heap, sizeof(ppdoc));
  pdf->heap = heap;
  ppbytes_buffer_init(&pdf->heap);
  ...

So ppdoc pdf was allocated from the heap owned by the pdf itself. Somewhat tricky, but should work fine,
as from that point nothing refered to a local heap variable addres. For some reason that causes a crash
on openbsd.
*/

static ppdoc * ppdoc_create (iof_file *input)
{
  ppdoc *pdf;

  pdf = (ppdoc *)pp_malloc(sizeof(ppdoc));
  ppheap_init(&pdf->heap);
  ppbytes_buffer_init(&pdf->heap);
  ppstack_init(&pdf->stack, &pdf->heap);
  ppdoc_reader_init(pdf, input);
  ppdoc_pages_init(pdf);
  pdf->xref = NULL;
  pdf->crypt = NULL;
  pdf->cryptstatus = PPCRYPT_PASS; // check on ppdoc_read() -> ppdoc_crypt_pass()
  pdf->flags = 0;
  pdf->version[0] = '\0';
  if (ppdoc_read(pdf, &pdf->input) != NULL)
    return pdf;
  ppdoc_free(pdf);
  return NULL;
}

ppdoc * ppdoc_load (const char *filename)
{
  FILE *file;
  iof_file input;
  if ((file = fopen(filename, "rb")) == NULL)
    return NULL;
  iof_file_init(&input, file);
  input.flags |= IOF_CLOSE_FILE;
  return ppdoc_create(&input);
}

ppdoc * ppdoc_filehandle (FILE *file, int closefile)
{
  iof_file input;
  if (file == NULL)
    return NULL;
  iof_file_init(&input, file);
  if (closefile)
    input.flags |= IOF_CLOSE_FILE;
  return ppdoc_create(&input);
}

ppdoc * ppdoc_mem (const void *data, size_t size)
{
  iof_file input;
  iof_file_rdata_init(&input, data, size);
  input.flags |= IOF_BUFFER_ALLOC;
  return ppdoc_create(&input);
}

void ppdoc_free (ppdoc *pdf)
{
  iof_file_decref(&pdf->input);
  ppstack_free_buffer(&pdf->stack);
  ppheap_free(&pdf->heap);
  pp_free(pdf);
}

ppcrypt_status ppdoc_crypt_status (ppdoc *pdf)
{
  return pdf->cryptstatus;
}

ppint ppdoc_permissions (ppdoc *pdf)
{
  return pdf->crypt != NULL ? pdf->crypt->permissions : (ppint)0xFFFFFFFFFFFFFFFF;
}

/* pages access */

static pparray * pppage_node (ppdict *dict, ppuint *count, ppname **type)
{
  ppname **pkey, *key;
  ppobj *obj;
  pparray *kids = NULL;
  *count = 0;
  *type = NULL;
  for (ppdict_first(dict, pkey, obj); (key = *pkey) != NULL; ppdict_next(pkey, obj))
  {
    switch (key->data[0])
    {
      case 'T':
        if (ppname_is(key, "Type"))
          *type = ppobj_get_name(obj);
        break;
      case 'C':
        if (ppname_is(key, "Count"))
          ppobj_rget_uint(obj, *count);
        break;
      case 'K':
        if (ppname_is(key, "Kids"))
          kids = ppobj_rget_array(obj);
        break;
    }
  }
  return kids;
}

#define ppname_is_page(type) (type != NULL && ppname_is(type, "Page"))

ppuint ppdoc_page_count (ppdoc *pdf)
{
  ppref *ref;
  ppname *type;
  ppuint count;
  if ((ref = ppxref_pages(pdf->xref)) == NULL)
    return 0;
  if (pppage_node(ref->object.dict, &count, &type) == NULL)
    return ppname_is_page(type) ? 1 : 0; // acrobat and ghostscript accept documents with root /Pages entry being a reference to a sole /Page object
  return count;
}

ppref * ppdoc_page (ppdoc *pdf, ppuint index)
{
  ppdict *dict;
  ppuint count;
  pparray *kids;
  size_t size, i;
  ppobj *r, *o;
  ppref *ref;
  ppname *type;

  if ((ref = ppxref_pages(pdf->xref)) == NULL)
    return NULL;
  dict = ref->object.dict;
  if ((kids = pppage_node(dict, &count, &type)) != NULL)
  {
    if (index < 1 || index > count)
      return NULL;
  }
  else
  {
    return index == 1 && ppname_is_page(type) ? ref : NULL;
  }
scan_array:
  if (index <= count / 2)
  { // probably shorter way from the beginning
    for (i = 0, size = kids->size, r = pparray_at(kids, 0); i < size; ++i, ++r)
    {
      if (r->type != PPREF)
        return NULL;
      o = &r->ref->object;
      if (o->type != PPDICT)
        return NULL;
      dict = o->dict;
      if ((kids = pppage_node(dict, &count, &type)) != NULL)
      {
        if (index <= count)
          goto scan_array;
        index -= count;
        continue;
      }
      if (index == 1 && ppname_is_page(type))
        return r->ref;
      --index;
    }
  }
  else if ((size = kids->size) > 0) // for safe (size-1)
  { // probably shorter way from the end
    index = count - index + 1;
    for (i = 0, r = pparray_at(kids, size - 1); i < size; ++i, --r)
    {
      if (r->type != PPREF)
        return NULL;
      o = &r->ref->object;
      if (o->type != PPDICT)
        return NULL;
      dict = o->dict;
      if ((kids = pppage_node(dict, &count, &type)) != NULL)
      {
        if (index <= count) {
          index = count - index + 1;
          goto scan_array;
        }
        index -= count;
        continue;
      }
      if (index == 1 && ppname_is_page(type))
        return r->ref;
      --index;
    }
  }
  return NULL;
}

/*
Through pages iterator. Iterating over pages tree just on the base of /Kids and /Parent keys
is ineffective, as to get next pageref we need to take parent, find the pageref in /Kids,
take next (or go upper).. Annoying. We use a dedicated stack for pages iterator. This could
actually be done with pdf->stack, but some operations may clear it, so safer to keep it independent
Besides, its depth is constant (set on first use), so no need for allocs.
*/

static void ppdoc_pages_init (ppdoc *pdf)
{
  pppages *pages;
  pages = &pdf->pages;
  pages->root = pages->parent = &pages->buffer[0];
  pages->depth = 0;
  pages->space = PPPAGES_STACK_DEPTH;
}

static ppkids * pppages_push (ppdoc *pdf, pparray *kids)
{
  ppkids *newroot, *bounds;
  pppages *pages;
  pages = &pdf->pages;
  if (pages->depth == pages->space)
  {
    pages->space <<= 1;
    newroot = (ppkids *)ppstruct_take(&pdf->heap, pages->space * sizeof(ppkids));
    memcpy(newroot, pages->root, pages->depth * sizeof(ppkids));
    pages->root = newroot;
  }
  bounds = pages->parent = &pages->root[pages->depth++];
  bounds->current = pparray_at(kids, 0);
  bounds->sentinel = pparray_at(kids, kids->size);
  return bounds;
}

#define pppages_pop(pages) (--((pages)->parent), --((pages)->depth))

static ppref * ppdoc_pages_group_first (ppdoc *pdf, ppref *ref)
{
  ppdict *dict;
  pparray *kids;
  ppuint count;
  ppname *type;
  ppobj *o;

  dict = ref->object.dict; // typecheck made by callers
  while ((kids = pppage_node(dict, &count, &type)) != NULL)
  {
    if ((o = pparray_get_obj(kids, 0)) == NULL) // empty /Kids
      return ppdoc_next_page(pdf);
    if ((ref = ppobj_get_ref(o)) == NULL || ref->object.type != PPDICT)
      return NULL;
    pppages_push(pdf, kids);
    dict = ref->object.dict;
  }
  return ppname_is_page(type) ? ref : NULL;
}

ppref * ppdoc_first_page (ppdoc *pdf)
{
  ppref *ref;
  pppages *pages;
  if ((ref = ppdoc_pages(pdf)) == NULL)
    return NULL;
  pages = &pdf->pages;
  pages->parent = pages->root;
  pages->depth = 0;
  return ppdoc_pages_group_first(pdf, ref);
}

ppref * ppdoc_next_page (ppdoc *pdf)
{
  pppages *pages;
  ppkids *bounds;
  ppref *ref;
  ppobj *obj;
  pages = &pdf->pages;
  while (pages->depth > 0)
  {
    bounds = pages->parent;
    obj = ++bounds->current;
    if (obj < bounds->sentinel)
    {
      if (obj->type != PPREF)
        return NULL;
      ref = obj->ref;
      if (ref->object.type != PPDICT)
        return NULL;
      return ppdoc_pages_group_first(pdf, ref);
    }
    else
    { // no next node, go upper
      pppages_pop(pages);
    }
  }
  return NULL;
}

/* context */

ppcontext * ppcontext_new (void)
{
  ppcontext *context;
  context = (ppcontext *)pp_malloc(sizeof(ppcontext));
  ppheap_init(&context->heap);
  ppbytes_buffer_init(&context->heap);
  ppstack_init(&context->stack, &context->heap);
  return context;
}

void ppcontext_done (ppcontext *context)
{
  ppheap_renew(&context->heap);
  ppstack_clear(&context->stack);
}

void ppcontext_free (ppcontext *context)
{
  ppstack_free_buffer(&context->stack);
  ppheap_free(&context->heap);
  pp_free(context);
}

/* page contents streams */

//#define ppcontents_first_stream(array) pparray_rget_stream(array, 0)

static ppstream * ppcontents_first_stream (pparray *array)
{
  size_t i;
  ppobj *obj;
  ppref *ref;
  for (pparray_first(array, i, obj); i < array->size; pparray_next(i, obj))
    if ((ref = ppobj_get_ref(obj)) != NULL && ref->object.type == PPSTREAM)
      return ref->object.stream;
  return NULL;
}

static ppstream * ppcontents_next_stream (pparray *array, ppstream *stream)
{
  size_t i;
  ppobj *obj;
  ppref *ref;
  for (pparray_first(array, i, obj); i < array->size; pparray_next(i, obj))
    if ((ref = ppobj_get_ref(obj)) != NULL && ref->object.type == PPSTREAM && ref->object.stream == stream)
      if (++i < array->size && (ref = ppobj_get_ref(obj + 1)) != NULL && ref->object.type == PPSTREAM)
        return ref->object.stream;
  return NULL;
}

ppstream * ppcontents_first (ppdict *dict)
{
  ppobj *contentsobj;
  if ((contentsobj = ppdict_rget_obj(dict, "Contents")) == NULL)
    return NULL;
  switch (contentsobj->type)
  {
    case PPARRAY:
      return ppcontents_first_stream(contentsobj->array);
    case PPSTREAM:
      return contentsobj->stream;
    default:
      break;
  }
  return NULL;
}

ppstream * ppcontents_next (ppdict *dict, ppstream *stream)
{
  ppobj *contentsobj;
  if ((contentsobj = ppdict_rget_obj(dict, "Contents")) == NULL)
    return NULL;
  switch (contentsobj->type)
  {
    case PPARRAY:
      return ppcontents_next_stream(contentsobj->array, stream);
    case PPSTREAM:
      break;
    default:
      break;
  }
  return NULL;
}

static ppobj * ppcontents_op (iof *I, ppstack *stack, size_t *psize, ppname **pname)
{
  ppobj *obj;
  ppstack_clear(stack);
  do {
    if (ppscan_find(I) < 0)
      return NULL;
    if ((obj = ppscan_psobj(I, stack)) == NULL)
      return NULL;
  } while (obj->type != PPNAME || !ppname_exec(obj->name));
  *pname = obj->name;
  *psize = stack->size - 1;
  return stack->buf;
}

ppobj * ppcontents_first_op (ppcontext *context, ppstream *stream, size_t *psize, ppname **pname)
{
  iof *I;
  if ((I = ppstream_read(stream, 1, 0)) == NULL)
    return NULL;
  return ppcontents_op(I, &context->stack, psize, pname);
}

ppobj * ppcontents_next_op (ppcontext *context, ppstream *stream, size_t *psize, ppname **pname)
{
  return ppcontents_op(ppstream_iof(stream), &context->stack, psize, pname);
}

ppobj * ppcontents_parse (ppcontext *context, ppstream *stream, size_t *psize)
{
  iof *I;
  ppstack *stack;
  ppobj *obj;
  stack = &context->stack;
  ppstack_clear(stack);
  if ((I = ppstream_read(stream, 1, 0)) == NULL)
    return NULL;
  while (ppscan_find(I) >= 0)
    if ((obj = ppscan_psobj(I, stack)) == NULL)
      goto error;
  *psize = stack->size;
  ppstream_done(stream);
  return stack->buf;
error:
  ppstream_done(stream);
  return NULL;
}

/* boxes */

pprect * pparray_to_rect (pparray *array, pprect *rect)
{
  ppobj *obj;
  if (array->size != 4)
    return NULL;
  obj = pparray_at(array, 0);
  if (!ppobj_get_num(obj, rect->lx)) return NULL;
  obj = pparray_at(array, 1);
  if (!ppobj_get_num(obj, rect->ly)) return NULL;
  obj = pparray_at(array, 2);
  if (!ppobj_get_num(obj, rect->rx)) return NULL;
  obj = pparray_at(array, 3);
  if (!ppobj_get_num(obj, rect->ry)) return NULL;
  return rect;
}

pprect * ppdict_get_rect (ppdict *dict, const char *name, pprect *rect)
{
  pparray *array;
  return (array = ppdict_rget_array(dict, name)) != NULL ? pparray_to_rect(array, rect) : NULL;
}

pprect * ppdict_get_box (ppdict *dict, const char *name, pprect *rect)
{
  do {
    if (ppdict_get_rect(dict, name, rect) != NULL)
      return rect;
    dict = ppdict_rget_dict(dict, "Parent");
  } while (dict != NULL);
  return NULL;
}

ppmatrix * pparray_to_matrix (pparray *array, ppmatrix *matrix)
{
  ppobj *obj;
  if (array->size != 6)
    return NULL;
  obj = pparray_at(array, 0);
  if (!ppobj_get_num(obj, matrix->xx)) return NULL;
  obj = pparray_at(array, 1);
  if (!ppobj_get_num(obj, matrix->xy)) return NULL;
  obj = pparray_at(array, 2);
  if (!ppobj_get_num(obj, matrix->yx)) return NULL;
  obj = pparray_at(array, 3);
  if (!ppobj_get_num(obj, matrix->yy)) return NULL;
  obj = pparray_at(array, 4);
  if (!ppobj_get_num(obj, matrix->x)) return NULL;
  obj = pparray_at(array, 5);
  if (!ppobj_get_num(obj, matrix->y)) return NULL;
  return matrix;
}

ppmatrix * ppdict_get_matrix (ppdict *dict, const char *name, ppmatrix *matrix)
{
  pparray *array;
  return (array = ppdict_rget_array(dict, name)) != NULL ? pparray_to_matrix(array, matrix) : NULL;
}

/* logger */

void pplog_callback (pplogger_callback logger, void *alien)
{
  logger_callback((logger_function)logger, alien);
}

int pplog_prefix (const char *prefix)
{
  return logger_prefix(prefix);
}

/* version */

const char * ppdoc_version_string (ppdoc *pdf)
{
  return pdf->version;
}

int ppdoc_version_number (ppdoc *pdf, int *minor)
{
  *minor = pdf->version[2] - '0';
  return pdf->version[0] - '0';
}

/* doc info */

size_t ppdoc_file_size (ppdoc *pdf)
{
  return pdf->filesize;
}

ppuint ppdoc_objects (ppdoc *pdf)
{
  ppuint count;
  ppxref *xref;
  for (count = 0, xref = pdf->xref; xref != NULL; xref = xref->prev)
    count += xref->count;
  return count;
}

size_t ppdoc_memory (ppdoc *pdf, size_t *waste)
{
	mem_info info;
  size_t used;
  ppbytes_heap_info(&pdf->heap, &info, 0);
  ppstruct_heap_info(&pdf->heap, &info, 1);

  *waste = info.ghosts + info.blockghosts + info.left; // info.ghosts == 0
  used = info.used + *waste;
  used += pdf->stack.space * sizeof(ppobj);
  return used;
}
