/* lzw implementation for postscript/pdf filters
# Notes on LZW

# Encoder

Initially the table contains 256 entires for single bytes. Encoder consumes
input bytes trying to find the longest sequence stored so far in the table.
Once it finds a sequence that is not present in the table, it outputs the table
index of the longest sequence found (accumulated bytes except the last
consumed) and pushes the new sequence (accumulated bytes including the last
one) on the top of the table. The last taken byte is not yet written to the
output, it becomes the beginning of the new sequence to accumulate. Initially,
encoder outputs 9-bit codes. While the table grows, the number of bits for each
code increases up to 12. In example, after adding a table entry of index 511 it
is high time to switch to 10-bit bytes. /EarlyChange=true parameter in stream
dictionary (both postscript and pdf) informs to increase the number of bits one
code earlier then necessary. Looks pretty much like an early days bug that
became a specification :) I have never found a PDF having /EarlyChange key
specified anyway.

Once the table becomes full (or when encoder decides it is worthy),
a clear-table marker (code 256) purges the table and restores codes length to
9. End-of-data marker (code 257) ends the stream. Conventionally, the beginning
of the stream starts with clear-table marker.

Postscript allows to provide a /UnitLength which determines the bit length of
codes. The above description assumes UnitLength=8 (default). Allowed values are
from 3 to 8. Different UnitLength also affects markers; clear-table is then
2^UnitLength and end-of-data marker is 2^UnitLenth+1.

Encoder outputs 9-12bit codes that are packed into bytes using high-bits-first
scheme (default) or low-bits-scheme.

PDF spec p. 73 (PS spec p. 135 gives an mistaken output sequence and so
mistaken output bytes)

Input character sequence (decimal)
45 45 45 45 45 65 45 45 45 66

Output 9bit codes (decimal)
256 45 258 258 65 259 66 257

Output 9bit codes (binary)
100000000 000101101 100000010 100000010 001000001 100000011 001000010 100000001

Output bytes (LowBitsFirst=false); eight high-order bits of code becomes
the first byte, remaining low-order bit of code becomes the high-order bit of the
next byte;
10000000 00001011 01100000 01010000 00100010 00001100 00001100 10000101 00000001
-> 80 0B 60 50 22 0C 0C 85 01

Output bytes (binary, LowBitsFirst=true); eight low-order bits of code becomes
the first byte, remaining high-order bit of code becomes low-order bit of the
next byte;
00000000 01011011 00001000 00010100 00011000 01100100 10100000 10000000 10010000
-> 00 5B 08 14 18 64 A0 80 90

# Decoder

Decoder consumes input bytes transforming them to 9 to 12 bit codes. Initially
it starts with 9bit codes and the table of 258 fixed codes (same as encoder).
Basically, it interprets incoming codes as table indices (except 256 and 257
markers) and it outputs byte sequences stored at given indices. It also
upbuilds the table and changes the number of bits of codes when necessary. The
key point on lzw is that both encoder and decoder builds the table
synchronously.

However, decoder needs some "knowledge" about how encoder works to be able to
interpret a table index that it doesn't have so far. Look that the output from
encoder in the example above. The first output code is conventional clear-table
(256). Then comes a code 45. So far so good, decoder interprets code 45 as
a (fixed) entry of the table, emitting byte 45. The next code is 258, which is
should be interpreted as an index in the table. Oops, encoder doesn't have one
yet. If that occurs, it means that encoder was able to output the new entry
code just after adding it to a table. It means that

  sequence_before + next_byte == next_byte + sequence_after

This may happen not only for sequences like 45 45 45, but also symmetric series
such as abcbabcba; abcb + a == a + bcba. Decoder must be aware of that and if
it gets a code one larger than the top table index, it should create one on-fly
by appending last entry sequence by the first by of the last entry.

# UnitLength

Postscript specification mentions about UnitLength parameter that can be used
in LZW decoder (not allowed in encoder), with possible values from 3 to 8. This
parameter determines the number of bits per code; form UnitLength + 1 to 12. It
also determines which codes are used for clear-table marker (2^UnitLength) and
end-of-data marker ((2^UnitLength)+1). Postscript specification says (page 134):

"Initially, the code length is (UnitLength + 1) bits and the table contains only
entries for the (2^UnitLength + 2) fixed codes. As encoding proceeds, entries are
appended to the table, associating new codes with longer and longer input character
sequences. The encoding and decoding filters maintain identical copies of
this table."

Later on page 136 Postscript specification says:

"Data that has been LZW-encoded with a UnitLength less than 8 consists only of
codes in the range 0 to 2^UnitLength - 1; consequently, the LZWDecode filter produces
only codes in that range when read. UnitLength also affects the encoded
representation, as described above."

UnitLength (Postscript only) and LowBitsFirst are used only by decoder.
EarlyChange should obviously be respected by both encoder and decoder. When
table index reaches current bit length boundary (511, 1023, ...) it must react
by increasing the number of bits of input code. But if the index reaches it
maximum value (when the table is full), decoder is NOT supposed to clear the
table. When the table is full, encoder must emit clear-table marker and it
emits this code using 12 bits and reinitialize code bits after that. It means
that, when the table is full, decoder should get one more 12-bit code (which
should be clear-table marker) and actually clear the table and reinitialize
code bits after that.

# Clear-table vs last entry track (after tries and checks)

It is also not quite clear what should actually happen when encoder gets a full
table and it is supposed to emit clear-table marker. When it gets full, it
means that it has just appended another entry to the table. And that happens
only the input sequence collected so far plus the last byte is not present in
the table. Encoder is supposed to output the table index of the present
sequence and set the recent byte as a starting index of the new sequence to be
collected. Even if it is time to clear the table, encoder is still supposed to
keep the track of the last table entry. Decoder, however, must drop the track of the
last code on clear-table.

# Decoder table vs encoder table

While decoding we need query lzw table by (subsequent) numeric codes and output
character sequences stored in the table. While encoding we need to query the
table on every input byte and fetch indices pointing to character sequences.
Note that we never need to query the entire table for the longest sequence
found so far. The encoder table do not need to access the longest character
sequence at one piece. It is enough to keep the track of the current table
index and the very next byte. We organize an encoder table into a search tree,
where every node contains its table index (value) and last byte (key). Except
initial tree content, every node is created on the base of the previous node
and it conceptually point the sequence represented by that nodo consists of the
previous node sequence plus the next byte.

Every new node is a descendant of the node it has been derived from. Every node
has a map (a search subtree) indexed by suffix byte value, pointing to
descendants nodes. Every node also has binary tentackles (left/right fields)
necessary to search the map (except initials, every node lives in a map of some
ancestor node). The key point is that on every input byte we don't search the
entire tree, but only the map of the current node children. The map tree is
a simple binary tree with no balancing mechanism (not worthy to optimize an
ephemeric structure that may be upbuilt more often then queried).

In our implementation, decoder table requires  4069 entries (topmost index 4095).
Encoder table, however, needs 4097 entries to handle the case when EarlyIndex
parameter is 0 (I have never a chance to test that in practise). The node of index
4096 might be added to a search tree, but its code is never emitted; the lookup
is purged just after adding that node.

todo:
- support for LowBitsFirst encoding
*/

#include "utilmem.h"
#include "utillzw.h"

/* filter state struct */

typedef struct lzw_entry {
  union {
    const char *rdata; // to be able to init with string literal
    char *data;
  };
  int size;
} lzw_entry;

#define lzw_index short

typedef struct lzw_node lzw_node;

struct lzw_node {
  lzw_index index;
  unsigned char suffix;
  lzw_node *left;
  lzw_node *right;
  lzw_node *map;
};

struct lzw_state {
  union {
    lzw_node *lookup;                 /* encoder table */
    lzw_entry *table;                 /* decoder table */
  };
  lzw_index index;                    /* table index */
  union {
    lzw_node *lastnode;               /* previous encoder table node */
    struct {
      lzw_entry *lastentry;           /* previous decoder table entry */
      int tailbytes;                  /* num of bytes of lastentry not yet written out */
    };
  };
  int basebits;                       /* /UnitLength parameter (8) */
  int codebits;                       /* current code bits */
  int lastbyte;                       /* previosly read byte */
  int tailbits;                       /* lastbyte bits not yet consumed */
  int flush;                          /* encoder */
  int flags;                          /* options */
};

/* macros */

#define LZW_MIN_BITS 3
#define LZW_MAX_BITS 12
#define LZW_TABLE_SIZE (1 << LZW_MAX_BITS)
#define LZW_LOOKUP_SIZE (LZW_TABLE_SIZE + 1)

#define lzw_bit_range(bits) (bits >= LZW_MIN_BITS && bits <= LZW_BASE_BITS)
#define lzw_base_bits(flags) (flags & ((1 << 4) - 1)) // 4 low bits of flags is basebits (UnitLength)

#define lzw_initial_codes(state) (1 << state->basebits)
#define lzw_clear_code(state)    lzw_initial_codes(state)
#define lzw_eod_code(state)      (lzw_initial_codes(state) + 1)
#define lzw_initial_index(state) (lzw_initial_codes(state) + 2)

#define lzw_max_index(state) ((1 << state->codebits) - ((state->flags & LZW_EARLY_INDEX) ? 1 : 0))
#define lzw_check_bits(state) ((void)(state->index == lzw_max_index(state) && state->codebits < LZW_MAX_BITS && ++state->codebits))

#define lzw_malloc util_malloc
#define lzw_free util_free

/* decoder */

static struct lzw_entry lzw_initial_table[] = {
  {{"\x00"}, 1}, {{"\x01"}, 1}, {{"\x02"}, 1}, {{"\x03"}, 1}, {{"\x04"}, 1}, {{"\x05"}, 1}, {{"\x06"}, 1}, {{"\x07"}, 1}, {{"\x08"}, 1}, {{"\x09"}, 1}, {{"\x0A"}, 1}, {{"\x0B"}, 1}, {{"\x0C"}, 1}, {{"\x0D"}, 1}, {{"\x0E"}, 1}, {{"\x0F"}, 1},
  {{"\x10"}, 1}, {{"\x11"}, 1}, {{"\x12"}, 1}, {{"\x13"}, 1}, {{"\x14"}, 1}, {{"\x15"}, 1}, {{"\x16"}, 1}, {{"\x17"}, 1}, {{"\x18"}, 1}, {{"\x19"}, 1}, {{"\x1A"}, 1}, {{"\x1B"}, 1}, {{"\x1C"}, 1}, {{"\x1D"}, 1}, {{"\x1E"}, 1}, {{"\x1F"}, 1},
  {{"\x20"}, 1}, {{"\x21"}, 1}, {{"\x22"}, 1}, {{"\x23"}, 1}, {{"\x24"}, 1}, {{"\x25"}, 1}, {{"\x26"}, 1}, {{"\x27"}, 1}, {{"\x28"}, 1}, {{"\x29"}, 1}, {{"\x2A"}, 1}, {{"\x2B"}, 1}, {{"\x2C"}, 1}, {{"\x2D"}, 1}, {{"\x2E"}, 1}, {{"\x2F"}, 1},
  {{"\x30"}, 1}, {{"\x31"}, 1}, {{"\x32"}, 1}, {{"\x33"}, 1}, {{"\x34"}, 1}, {{"\x35"}, 1}, {{"\x36"}, 1}, {{"\x37"}, 1}, {{"\x38"}, 1}, {{"\x39"}, 1}, {{"\x3A"}, 1}, {{"\x3B"}, 1}, {{"\x3C"}, 1}, {{"\x3D"}, 1}, {{"\x3E"}, 1}, {{"\x3F"}, 1},
  {{"\x40"}, 1}, {{"\x41"}, 1}, {{"\x42"}, 1}, {{"\x43"}, 1}, {{"\x44"}, 1}, {{"\x45"}, 1}, {{"\x46"}, 1}, {{"\x47"}, 1}, {{"\x48"}, 1}, {{"\x49"}, 1}, {{"\x4A"}, 1}, {{"\x4B"}, 1}, {{"\x4C"}, 1}, {{"\x4D"}, 1}, {{"\x4E"}, 1}, {{"\x4F"}, 1},
  {{"\x50"}, 1}, {{"\x51"}, 1}, {{"\x52"}, 1}, {{"\x53"}, 1}, {{"\x54"}, 1}, {{"\x55"}, 1}, {{"\x56"}, 1}, {{"\x57"}, 1}, {{"\x58"}, 1}, {{"\x59"}, 1}, {{"\x5A"}, 1}, {{"\x5B"}, 1}, {{"\x5C"}, 1}, {{"\x5D"}, 1}, {{"\x5E"}, 1}, {{"\x5F"}, 1},
  {{"\x60"}, 1}, {{"\x61"}, 1}, {{"\x62"}, 1}, {{"\x63"}, 1}, {{"\x64"}, 1}, {{"\x65"}, 1}, {{"\x66"}, 1}, {{"\x67"}, 1}, {{"\x68"}, 1}, {{"\x69"}, 1}, {{"\x6A"}, 1}, {{"\x6B"}, 1}, {{"\x6C"}, 1}, {{"\x6D"}, 1}, {{"\x6E"}, 1}, {{"\x6F"}, 1},
  {{"\x70"}, 1}, {{"\x71"}, 1}, {{"\x72"}, 1}, {{"\x73"}, 1}, {{"\x74"}, 1}, {{"\x75"}, 1}, {{"\x76"}, 1}, {{"\x77"}, 1}, {{"\x78"}, 1}, {{"\x79"}, 1}, {{"\x7A"}, 1}, {{"\x7B"}, 1}, {{"\x7C"}, 1}, {{"\x7D"}, 1}, {{"\x7E"}, 1}, {{"\x7F"}, 1},
  {{"\x80"}, 1}, {{"\x81"}, 1}, {{"\x82"}, 1}, {{"\x83"}, 1}, {{"\x84"}, 1}, {{"\x85"}, 1}, {{"\x86"}, 1}, {{"\x87"}, 1}, {{"\x88"}, 1}, {{"\x89"}, 1}, {{"\x8A"}, 1}, {{"\x8B"}, 1}, {{"\x8C"}, 1}, {{"\x8D"}, 1}, {{"\x8E"}, 1}, {{"\x8F"}, 1},
  {{"\x90"}, 1}, {{"\x91"}, 1}, {{"\x92"}, 1}, {{"\x93"}, 1}, {{"\x94"}, 1}, {{"\x95"}, 1}, {{"\x96"}, 1}, {{"\x97"}, 1}, {{"\x98"}, 1}, {{"\x99"}, 1}, {{"\x9A"}, 1}, {{"\x9B"}, 1}, {{"\x9C"}, 1}, {{"\x9D"}, 1}, {{"\x9E"}, 1}, {{"\x9F"}, 1},
  {{"\xA0"}, 1}, {{"\xA1"}, 1}, {{"\xA2"}, 1}, {{"\xA3"}, 1}, {{"\xA4"}, 1}, {{"\xA5"}, 1}, {{"\xA6"}, 1}, {{"\xA7"}, 1}, {{"\xA8"}, 1}, {{"\xA9"}, 1}, {{"\xAA"}, 1}, {{"\xAB"}, 1}, {{"\xAC"}, 1}, {{"\xAD"}, 1}, {{"\xAE"}, 1}, {{"\xAF"}, 1},
  {{"\xB0"}, 1}, {{"\xB1"}, 1}, {{"\xB2"}, 1}, {{"\xB3"}, 1}, {{"\xB4"}, 1}, {{"\xB5"}, 1}, {{"\xB6"}, 1}, {{"\xB7"}, 1}, {{"\xB8"}, 1}, {{"\xB9"}, 1}, {{"\xBA"}, 1}, {{"\xBB"}, 1}, {{"\xBC"}, 1}, {{"\xBD"}, 1}, {{"\xBE"}, 1}, {{"\xBF"}, 1},
  {{"\xC0"}, 1}, {{"\xC1"}, 1}, {{"\xC2"}, 1}, {{"\xC3"}, 1}, {{"\xC4"}, 1}, {{"\xC5"}, 1}, {{"\xC6"}, 1}, {{"\xC7"}, 1}, {{"\xC8"}, 1}, {{"\xC9"}, 1}, {{"\xCA"}, 1}, {{"\xCB"}, 1}, {{"\xCC"}, 1}, {{"\xCD"}, 1}, {{"\xCE"}, 1}, {{"\xCF"}, 1},
  {{"\xD0"}, 1}, {{"\xD1"}, 1}, {{"\xD2"}, 1}, {{"\xD3"}, 1}, {{"\xD4"}, 1}, {{"\xD5"}, 1}, {{"\xD6"}, 1}, {{"\xD7"}, 1}, {{"\xD8"}, 1}, {{"\xD9"}, 1}, {{"\xDA"}, 1}, {{"\xDB"}, 1}, {{"\xDC"}, 1}, {{"\xDD"}, 1}, {{"\xDE"}, 1}, {{"\xDF"}, 1},
  {{"\xE0"}, 1}, {{"\xE1"}, 1}, {{"\xE2"}, 1}, {{"\xE3"}, 1}, {{"\xE4"}, 1}, {{"\xE5"}, 1}, {{"\xE6"}, 1}, {{"\xE7"}, 1}, {{"\xE8"}, 1}, {{"\xE9"}, 1}, {{"\xEA"}, 1}, {{"\xEB"}, 1}, {{"\xEC"}, 1}, {{"\xED"}, 1}, {{"\xEE"}, 1}, {{"\xEF"}, 1},
  {{"\xF0"}, 1}, {{"\xF1"}, 1}, {{"\xF2"}, 1}, {{"\xF3"}, 1}, {{"\xF4"}, 1}, {{"\xF5"}, 1}, {{"\xF6"}, 1}, {{"\xF7"}, 1}, {{"\xF8"}, 1}, {{"\xF9"}, 1}, {{"\xFA"}, 1}, {{"\xFB"}, 1}, {{"\xFC"}, 1}, {{"\xFD"}, 1}, {{"\xFE"}, 1}, {{"\xFF"}, 1}
};

#define lzw_entry_at(state, index) (&state->table[index])

static lzw_state * lzw_decoder_init_table (lzw_state *state, lzw_entry *table, int flags)
{
  state->basebits = lzw_base_bits(flags); // first four bits or flags
  if (!lzw_bit_range(state->basebits))
    return NULL;
  state->flags = flags;
  if ((state->table = table) == NULL)
  {
    state->table = (lzw_entry *)lzw_malloc(LZW_TABLE_SIZE * sizeof(lzw_entry));
    state->flags |= LZW_TABLE_ALLOC;
  }
  memcpy(state->table, lzw_initial_table, (size_t)lzw_initial_codes(state)*sizeof(lzw_entry));
  // memset(&state->table[lzw_initial_codes(state)], 0, 2*sizeof(lzw_entry)); // eod and clear entries never accessed
  state->codebits = state->basebits + 1;
  state->index = lzw_initial_index(state);
  state->lastentry = NULL;
  state->tailbytes = 0;
  state->lastbyte = 0;
  state->tailbits = 0;
  return state;
}

lzw_state * lzw_decoder_init (lzw_state *state, int flags)
{
  return lzw_decoder_init_table(state, NULL, flags);
}

static void lzw_decoder_clear (lzw_state *state)
{
  lzw_entry *entry;
  lzw_index initindex = lzw_initial_index(state);
  while (state->index > initindex)
  {
    entry = lzw_entry_at(state, --state->index);
    lzw_free(entry->data);
    // entry->data = NULL;
    // entry->size = 0;
  }
  state->lastentry = NULL;
  state->tailbytes = 0;
  state->codebits = state->basebits + 1;
}

void lzw_decoder_close (lzw_state *state)
{
  lzw_decoder_clear(state);
  if (state->flags & LZW_TABLE_ALLOC)
    lzw_free(state->table);
}

static int lzw_next_entry (lzw_state *state, lzw_entry *nextentry)
{
  lzw_entry *lastentry, *newentry;
  if ((lastentry = state->lastentry) == NULL)
    return 1; /* its ok */
  if (state->index == LZW_TABLE_SIZE)
    return 0; /* invalid input; eod marker expected earlier */
  /* put the new entry on the top of the table */
  newentry = lzw_entry_at(state, state->index++);
  /* its size is the last entrtyy size plus 1 */
  newentry->size = lastentry->size + 1;
  /* its content is the content of the last entry, */
  newentry->data = (char *)lzw_malloc((size_t)newentry->size);
  memcpy(newentry->data, lastentry->data, lastentry->size);
  /* plus the first byte of the new entry (usually fixed code entry) */
  newentry->data[newentry->size - 1] = nextentry->data[0];
  return 1;
}

#define lzw_write_bytes(O, state) ((state->tailbytes -= (int)iof_write(O, state->lastentry->data, (size_t)state->tailbytes)) == 0)

iof_status lzw_decode_state (iof *I, iof *O, lzw_state *state)
{
  const lzw_index clear = lzw_clear_code(state), eod = lzw_eod_code(state);
  lzw_index code;
  lzw_entry *entry;
  if (state->lastentry != NULL)
  { /* write out the tail from the last call */
    if (state->tailbytes > 0 && !lzw_write_bytes(O, state))
      return IOFFULL;
    /* do what we normally do at the end of the loop body below */
    lzw_check_bits(state);
  }
  // if (state->flags & LZW_LOW_BITS_FIRST)
  //   return IOFERR;
  while (1)
  {
    /* get input code of length state->codebits */
    code = (state->lastbyte & ((1 << state->tailbits) - 1)) << (state->codebits - state->tailbits);
    for (state->tailbits -= state->codebits; state->tailbits < 0; )
    {
      get_code:
      if ((state->lastbyte = iof_get(I)) < 0)
        return state->flush ? IOFEOF : state->lastbyte;
      state->tailbits += 8;
      if (state->tailbits < 0)
      {
        code |= (state->lastbyte << (-state->tailbits));
        goto get_code;
      }
      else
      {
        code |= (state->lastbyte >> state->tailbits);
        break;
      }
    }
    /* interpret the code */
    if (code < state->index)
    { /* single byte code or special marker */
      if (code == clear)
      {
        lzw_decoder_clear(state);
        continue;
      }
      if (code == eod)
        return IOFEOF;
      entry = lzw_entry_at(state, code);
      if (!lzw_next_entry(state, entry))
        return IOFERR;
    }
    else if (code == state->index)
    { /* apparently encoder has emitted the code of the key just created (see notes) */
      if (!lzw_next_entry(state, state->lastentry))
        return IOFERR;
      entry = lzw_entry_at(state, state->index - 1);
    }
    else
    { /* invalid input code */
      return IOFERR;
    }
    /* record the entry found */
    state->lastentry = entry;
    /* emit the sequence pointed by that entry */
    state->tailbytes = entry->size;
    if (!lzw_write_bytes(O, state))
      return IOFFULL;
    /* check and update code bits */
    lzw_check_bits(state);
  }
  return state->lastbyte; // never reached
}

/* encoder */

#define lzw_node_at(state, index) (&state->lookup[index])

#define lzw_node_init(node, i, c) (node->index = i, node->suffix = c, node->left = NULL, node->right = NULL, node->map = NULL)

static lzw_state * lzw_encoder_init_table (lzw_state *state, lzw_node *lookup, int flags)
{
  lzw_index index;
  lzw_node *node;
  state->basebits = lzw_base_bits(flags); // first four bits of flags is base bits of code (default 8)
  if (!lzw_bit_range(state->basebits))
    return NULL;
  state->flags = flags;
  if ((state->lookup = lookup) == NULL)
  {
    state->lookup = lzw_malloc(LZW_LOOKUP_SIZE*sizeof(lzw_node));
    state->flags |= LZW_TABLE_ALLOC;
  }
  state->index = lzw_initial_index(state);
  for (index = 0; index < lzw_initial_codes(state); ++index)
  {
    node = lzw_node_at(state, index);
    lzw_node_init(node, index, (unsigned char)index);
  }
  state->codebits = state->basebits + 1;
  state->lastnode = NULL;
  state->lastbyte = 0;
  state->tailbits = 0;
  return state;
}

lzw_state * lzw_encoder_init (lzw_state *state, int flags)
{
  return lzw_encoder_init_table(state, NULL, flags);
}

void lzw_encoder_close (lzw_state *state)
{
  if (state->flags & LZW_TABLE_ALLOC)
    lzw_free(state->lookup);
}

static void lzw_encoder_clear (lzw_state *state)
{
  lzw_node *node;
  lzw_index index;
  /* clear fixed nodes */
  for (index = 0; index < lzw_initial_codes(state); ++index)
  {
    node = lzw_node_at(state, index);
    lzw_node_init(node, index, (unsigned char)index);
  }
  /* reset table index */
  state->index = lzw_initial_index(state);
  /* reset code bits */
  state->codebits = state->basebits + 1;
}

static void lzw_put_code (iof *O, lzw_state *state, lzw_index code, int todobits)
{
  int leftbits, rightbits;
  do
  {
    leftbits = 8 - state->tailbits;
    rightbits = todobits - leftbits;
    if (rightbits >= 0)
    {
      state->lastbyte |= (code >> rightbits);
      iof_put(O, state->lastbyte);
      code = code & ((1 << rightbits) - 1);
      todobits -= leftbits;
      state->lastbyte = 0;
      state->tailbits = 0;
    }
    else
    {
      state->lastbyte |= (code << (-rightbits));
      state->tailbits += todobits;
      return;
    }
  } while (1);
}

static iof_status lzw_encode_last (iof *O, lzw_state *state)
{
  if (state->flush)
  {
    /* put the last code if any */
    if (state->lastnode != NULL)
      lzw_put_code(O, state, state->lastnode->index, state->codebits);
    /* put eod marker, */
    lzw_put_code(O, state, lzw_eod_code(state), state->codebits);
    /* with tail bits set to 0 */
    if (state->tailbits > 0)
      lzw_put_code(O, state, 0, 8 - state->tailbits);
    return IOFEOF;
  }
  return IOFEMPTY;
}

static lzw_node * lzw_node_push (lzw_state *state, unsigned char suffix)
{
  lzw_node *node;
  node = lzw_node_at(state, state->index);
  lzw_node_init(node, state->index, suffix);
  ++state->index;
  return node;
}

static int lzw_next_node (lzw_state *state, unsigned char suffix)
{
  lzw_node *node;
  if ((node = state->lastnode->map) == NULL)
  {
    state->lastnode->map = lzw_node_push(state, suffix);
    return 0;
  }
  while (1)
  {
    if (suffix < node->suffix)
    {
      if (node->left == NULL)
      {
        node->left = lzw_node_push(state, suffix);
        return 0;
      }
      node = node->left;
    }
    else if (suffix > node->suffix)
    {
      if (node->right == NULL)
      {
        node->right = lzw_node_push(state, suffix);
        return 0;
      }
      node = node->right;
    }
    else
    {
      state->lastnode = node;
      return 1;
    }
  }
  return 0; // never reached
}

iof_status lzw_encode_state (iof *I, iof *O, lzw_state *state)
{
  int byte;
  if (state->lastnode == NULL)
  { /* first call only; following convention, put clear-table marker */
    if (!iof_ensure(O, 2))
      return IOFFULL;
    lzw_put_code(O, state, lzw_clear_code(state), state->codebits);
    /* get the first input byte and initialize the current table entry */
    if ((byte = iof_get(I)) < 0)
      return lzw_encode_last(O, state);
    state->lastnode = lzw_node_at(state, byte);
  }
  while (iof_ensure(O, 2))
  { /* we need to write at most 2 bytes on each iteration */
    if ((byte = iof_get(I)) < 0)
      return lzw_encode_last(O, state);
    if (lzw_next_node(state, (unsigned char)byte) == 0)
    { /* means that the key hasn't been found and the new entry has just been created */
      /* output the code pointing the longest sequence so far */
      lzw_put_code(O, state, state->lastnode->index, state->codebits);
      /* update code bits */
      if (state->index == lzw_max_index(state) + 1)
      {
        if (state->codebits < LZW_MAX_BITS)
          ++state->codebits;
        else
        {
          /* put clear-table marker */
          lzw_put_code(O, state, lzw_clear_code(state), state->codebits);
          /* reset the table */
          lzw_encoder_clear(state);
        }
      }
      /* in any case, recent byte becomes the current table code */
      state->lastnode = lzw_node_at(state, byte);
    }
    /* otherwise no new entry is appended and state->lastnode points the longer sequence just found */
  }
  return IOFFULL;
}

/* single call codecs */

iof_status lzw_decode (iof *I, iof *O, int flags)
{
  lzw_state state = { { 0 } }; // shut overactive warnings
  lzw_entry table[LZW_TABLE_SIZE];
  int ret;
  lzw_decoder_init_table(&state, table, flags);
  state.flush = 1;
  ret = lzw_decode_state(I, O, &state);
  // iof_flush(O); // ?
  lzw_decoder_close(&state);
  return ret;
}

iof_status lzw_encode (iof *I, iof *O, int flags)
{
  lzw_state state;
  lzw_node lookup[LZW_LOOKUP_SIZE];
  int ret;
  lzw_encoder_init_table(&state, lookup, flags);
  state.flush = 1;
  ret = lzw_encode_state(I, O, &state);
  // iof_flush(O); // ?
  lzw_encoder_close(&state);
  return ret;
}

/* filters */

// lzw decoder function

static size_t lzw_decoder (iof *F, iof_mode mode)
{
  lzw_state *state;
  iof_status status;
  size_t tail;

  state = iof_filter_state(lzw_state *, F);
  switch(mode)
  {
    case IOFLOAD:
    case IOFREAD:
      if (F->flags & IOF_STOPPED)
        return 0;
      tail = iof_tail(F);
      F->pos = F->buf + tail;
      F->end = F->buf + F->space;
      do {
        status = lzw_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "lzw", status);
    case IOFCLOSE:
      lzw_decoder_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// lzw encoder function

static size_t lzw_encoder (iof *F, iof_mode mode)
{
  lzw_state *state;
  iof_status status;

  state = iof_filter_state(lzw_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = lzw_encode_state(F, F->next, state);
      return iof_encoder_retval(F, "lzw", status);
    case IOFCLOSE:
      if (!state->flush)
        lzw_encoder(F, IOFFLUSH);
      lzw_encoder_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

iof * iof_filter_lzw_decoder (iof *N, int flags)
{
  iof *I;
  lzw_state *state;
  I = iof_filter_reader(lzw_decoder, sizeof(lzw_state), &state);
  iof_setup_next(I, N);
  if (lzw_decoder_init(state, flags) == NULL)
  {
    iof_discard(I);
    return NULL;
  }
  state->flush = 1;
  return I;
}

iof * iof_filter_lzw_encoder (iof *N, int flags)
{
  iof *O;
  lzw_state *state;
  O = iof_filter_writer(lzw_encoder, sizeof(lzw_state), &state);
  iof_setup_next(O, N);
  if (lzw_encoder_init(state, flags) == NULL)
  {
    iof_discard(O);
    return NULL;
  }
  return O;
}
