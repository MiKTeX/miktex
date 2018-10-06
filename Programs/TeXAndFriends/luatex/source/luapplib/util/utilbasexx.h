
/* base encodings */

#ifndef UTIL_BASEXX_H
#define UTIL_BASEXX_H

#include "utiliof.h"

/* base codecs state */

typedef struct basexx_state basexx_state;

#define BASEXX_MAXLINE 80
#define BASEXX_PDF

void basexx_state_init_ln (basexx_state *state, size_t line, size_t maxline);
#define basexx_state_init(state) basexx_state_init_ln(state, 0, BASEXX_MAXLINE)

/* base16 */

int base16_getc (iof *I);
int base16_uc_putc (iof *I, int c);
int base16_lc_putc (iof *I, int c);
#define base16_putc base16_uc_putc

iof_status base16_encoded_uc (const void *data, size_t size, iof *O);
iof_status base16_encoded_lc (const void *data, size_t size, iof *O);
iof_status base16_encoded_uc_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline);
iof_status base16_encoded_lc_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline);

iof_status base16_encode_uc (iof *I, iof *O);
iof_status base16_encode_lc (iof *I, iof *O);
iof_status base16_encode_uc_ln (iof *I, iof *O, size_t line, size_t maxline);
iof_status base16_encode_lc_ln (iof *I, iof *O, size_t line, size_t maxline);
iof_status base16_decode (iof *I, iof *O);

#define base16_encoded base16_encoded_uc
#define base16_encoded_ln base16_encoded_uc_ln
#define base16_encode base16_encode_uc
#define base16_encode_ln base16_encode_uc_ln

iof_status base16_encode_state_uc (iof *I, iof *O, basexx_state *state);
iof_status base16_encode_state_lc (iof *I, iof *O, basexx_state *state);
iof_status base16_encode_state_uc_ln (iof *I, iof *O, basexx_state *state);
iof_status base16_encode_state_lc_ln (iof *I, iof *O, basexx_state *state);
iof_status base16_decode_state (iof *I, iof *O, basexx_state *state);

#define base16_encode_state base16_encode_state_uc
#define base16_encode_state_ln base16_encode_state_uc_ln

/* base64 */

extern const char base64_alphabet[];
extern const int base64_lookup[];

iof_status base64_encoded (const void *data, size_t size, iof *O);
iof_status base64_encoded_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline);

iof_status base64_encode (iof *I, iof *O);
iof_status base64_encode_ln (iof *I, iof *O, size_t line, size_t maxline);
iof_status base64_decode (iof *I, iof *O);

iof_status base64_encode_state (iof *I, iof *O, basexx_state *state);
iof_status base64_encode_state_ln (iof *I, iof *O, basexx_state *state);
iof_status base64_decode_state (iof *I, iof *O, basexx_state *state);

/* base85 */

extern const char base85_alphabet[];
extern const int base85_lookup[];

iof_status base85_encoded (const void *data, size_t size, iof *O);
iof_status base85_encoded_ln (const void *data, size_t size, iof *O, size_t line, size_t maxline);

iof_status base85_encode (iof *I, iof *O);
iof_status base85_encode_ln (iof *I, iof *O, size_t line, size_t maxline);
iof_status base85_decode (iof *I, iof *O);

iof_status base85_encode_state (iof *I, iof *O, basexx_state *state);
iof_status base85_encode_state_ln (iof *I, iof *O, basexx_state *state);
iof_status base85_decode_state (iof *I, iof *O, basexx_state *state);

/* run length */

typedef struct runlength_state runlength_state;

void runlength_state_init (runlength_state *state);

iof_status runlength_encode (iof *I, iof *O);
iof_status runlength_encode_state (iof *I, iof *O, runlength_state *state);

iof_status runlength_decode (iof *I, iof *O);
iof_status runlength_decode_state (iof *I, iof *O, runlength_state *state);

/* eexec */

typedef struct eexec_state eexec_state;

#define EEXEC_MAXLINE 80

void eexec_state_init_ln (eexec_state *state, size_t line, size_t maxline, const char *initbytes);
#define eexec_state_init(state) eexec_state_init_ln(state, 0, EEXEC_MAXLINE, NULL)

iof_status eexec_decode (iof *I, iof *O);
iof_status eexec_decode_state (iof *I, iof *O, eexec_state *state);

iof_status eexec_encode (iof *I, iof *O, size_t line, size_t maxline);
iof_status eexec_encode_state (iof *I, iof *O, eexec_state *state);

iof_status type1_charstring_decode (void *data, size_t size, void *outdata, uint8_t leniv);
iof_status type1_charstring_encode (void *data, size_t size, void *outdata, uint8_t leniv);

/* filters */

int iof_filter_basexx_encoder_ln (iof *N, size_t line, size_t maxline);

iof * iof_filter_base16_decoder (iof *N);
iof * iof_filter_base16_encoder (iof *N);

iof * iof_filter_base64_decoder (iof *N);
iof * iof_filter_base64_encoder (iof *N);

iof * iof_filter_base85_decoder (iof *N);
iof * iof_filter_base85_encoder (iof *N);

iof * iof_filter_runlength_decoder (iof *N);
iof * iof_filter_runlength_encoder (iof *N);

iof * iof_filter_eexec_decoder (iof *N);
iof * iof_filter_eexec_encoder (iof *N);


#endif
