#ifndef UTIL_FILTER_PREDICTOR_H
#define UTIL_FILTER_PREDICTOR_H

#include "utiliof.h"

typedef struct predictor_state predictor_state;

predictor_state * predictor_decoder_init (predictor_state *state, int predictor, int rowsamples, int components, int compbits);
predictor_state * predictor_encoder_init (predictor_state *state, int predictor, int rowsamples, int components, int compbits);

void predictor_decoder_close (predictor_state *state);
void predictor_encoder_close (predictor_state *state);

iof_status predictor_decode_state (iof *I, iof *O, predictor_state *state);
iof_status predictor_encode_state (iof *I, iof *O, predictor_state *state);

iof_status predictor_decode (iof *I, iof *O, int predictor, int rowsamples, int components, int compbits);
iof_status predictor_encode (iof *I, iof *O, int predictor, int rowsamples, int components, int compbits);

iof * iof_filter_predictor_decoder (iof *N, int predictor, int rowsamples, int components, int compbits);
iof * iof_filter_predictor_encoder (iof *N, int predictor, int rowsamples, int components, int compbits);

#endif