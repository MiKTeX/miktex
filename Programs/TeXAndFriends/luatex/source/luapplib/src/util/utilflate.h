#ifndef UTIL_FLATE_H
#define UTIL_FLATE_H

#include "utiliof.h"

typedef struct flate_state flate_state;

flate_state * flate_decoder_init (flate_state *state);
flate_state * flate_encoder_init (flate_state *state);

iof_status flate_decode_state (iof *I, iof *O, flate_state *state);
iof_status flate_encode_state (iof *I, iof *O, flate_state *state);

void flate_decoder_close (flate_state *state);
void flate_encoder_close (flate_state *state);

iof * iof_filter_flate_decoder (iof *N);
iof * iof_filter_flate_encoder (iof *N);


#endif