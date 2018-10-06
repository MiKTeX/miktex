#ifndef UTIL_LZW_H
#define UTIL_LZW_H

#include "utiliof.h"

typedef struct lzw_state lzw_state;

#define LZW_BASE_BITS 8
#define LZW_TABLE_ALLOC (1<<4)
#define LZW_EARLY_INDEX (1<<5)
//#define LZW_LOW_BITS_FIRST (1<<6)
#define LZW_DECODER_DEFAULTS (LZW_BASE_BITS|LZW_EARLY_INDEX|0)
#define LZW_ENCODER_DEFAULTS (LZW_BASE_BITS|LZW_EARLY_INDEX|0)

lzw_state * lzw_decoder_init (lzw_state *state, int flags);
lzw_state * lzw_encoder_init (lzw_state *state, int flags);

void lzw_decoder_close (lzw_state *state);
void lzw_encoder_close (lzw_state *state);

iof_status lzw_encode_state (iof *I, iof *O, lzw_state *state);
iof_status lzw_decode_state (iof *I, iof *O, lzw_state *state);

iof_status lzw_encode (iof *I, iof *O, int flags);
iof_status lzw_decode (iof *I, iof *O, int flags);

iof * iof_filter_lzw_decoder (iof *N, int flags);
iof * iof_filter_lzw_encoder (iof *N, int flags);

#endif