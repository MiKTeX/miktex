
#include "utilmem.h"
#include "utillog.h"
#include "utilflate.h"
#include <zlib.h>

/* flate codec */

/*
Flate codec example provided at http://www.zlib.net/zpipe.c (http://www.zlib.net/zlib_how.html) uses the following scheme:
- provide input data buffer
- keep providing output until codec function uses it

For encoder:

  z->zalloc = z->zfree = z->zopaque = NULL;
  deflateInit(z, compression_level);
  do {
    z->next_in = <input buffer>
    z->avail_in = <input buffer bytes>
    do {
      z->next_out = <output buffer>
      z->avail_out = <output buffer bytes>
      deflate(z, flush);
      // write obtained output from deflate
    } while (z->avail_out == 0);
    assert(z->avail_in == 0);
  } while (flush != Z_FINISH);
  deflateEnd(z);

'z' is an internal codec state of type z_stream, 'flush' is either Z_NO_FLUSH or Z_FINISH at the end of data.
deflate() ensures to consume the entire input if there are no obstackles to write an output. The inner loop
provides an output space as long as it is used by deflate(). When deflate() wrote everything it could,
it leaves z->avail_out > 0, which breaks the inner loop. At this point z->avail_in should also be zero.
The example documentation claims that the return codes from deflate() doesn't really need to be checked,
as checking z->avail_out for zero is enough.

The scheme for decoder is pretty similar, but with substantial differences:
- the end of stream is automatically found by decoder, so using Z_FINISH flag to indicate an end of stream
  is not necessary, but if provided, it MUST be given only if the EOF marker actually occurs in the input chunk,
  and subsequent calls to inflate() must consequently use Z_FINISH
- calling inflate() as long as it uses the output buffer provided still works for decoder, but inflate()
  does not ensure to consume the entire input, as it will read until end of stream marker
- the return code from inflate() must be checked to ensure the proper reaction on invalid data stream and
  end of stream signals
- initialization must set an input buffer to NULL or to some existing chunk (the later helps zlib to perform
  better on inflate(), but inflate() does the research on the first call anyway)

  z->zalloc = z->zfree = z->zopaque = NULL;
  z->next_in = NULL, z->avail_in = 0;
  inflateInit(z);
  do {
    z->next_in = <input buffer>
    z->avail_in = <input buffer bytes>
    do {
      z->next_out = <output buffer>
      z->avail_out = <output buffer bytes>
      status = inflate(z, flush);
      // check return status
      // write obtained output from inflate
    } while (z->avail_out == 0);
  } while (status != Z_STREAM_END);
  inflateEnd(z);

Our wrapper generally follows "prepare input, keep pomping output" scheme, but we need to support handler function
breaks on IOFEMPTY and IOFFULL. For a consistent come back from those on subsequent calls to the handler function,
we use 3 states:
- FLATE_IN - get input, when got something then goto FALTE_OUT
- FLATE_OUT - set z_stream buffers and keep writing output until enything to write, then goto FLATE_IN or FLATE_DONE
- FLATE_DONE - we are done, no return from that state
Distinction of FLATE_IN and FLATE_OUT states guarantees that we will not get more input until zlib consumes the stuff
from the previous feed, possibly interrupted by IOFFULL return on filling the output buffer. This distinction is not
critical, but makes the filter running according to the scheme described above. Note that we set zlib input buffer
(z->next_in, z->avail_in) at the beginning of FLATE_OUT state. Also note that we always update our buffers according
to updated avail_in / avail_out values, just after a call to inflate() / deflate(). So no matter what have happens
between handler calls, zlib input buffer is in sync with ours.
*/

struct flate_state {
  z_stream z;
  int flush;
  int status;
  int level; /* encoder compression level -1..9 */
};

enum {
  FLATE_IN,
  FLATE_OUT,
  FLATE_DONE
};

flate_state * flate_decoder_init (flate_state *state)
{ /* initialize zlib */
  z_stream *z = &state->z;
  z->zalloc = Z_NULL;
  z->zfree = Z_NULL;
  z->opaque = Z_NULL;
  z->avail_in = 0;     /* must be initialized before inflateInit() */
  z->next_in = Z_NULL; /* ditto */
  if (inflateInit(z) != Z_OK)
    return NULL;
  state->status = FLATE_IN;
  return state;
}

flate_state * flate_encoder_init (flate_state *state)
{
  z_stream *z = &state->z;
  z->zalloc = Z_NULL;
  z->zfree = Z_NULL;
  z->opaque = Z_NULL;
  z->avail_in = 0;
  z->next_in = Z_NULL;
  state->level = Z_DEFAULT_COMPRESSION; // will probably be moved upward
  if (deflateInit(z, state->level) != Z_OK)
    return NULL;
  state->status = FLATE_IN;
  return state;
}

static const char * zmess (int zstatus)
{
  switch (zstatus)
  {
    case Z_OK:            return "ok";
    case Z_STREAM_END:    return "end of stream";
    case Z_BUF_ERROR:     return "buffer error";
    case Z_STREAM_ERROR:  return "stream error";
    case Z_NEED_DICT:     return "need dict";
    case Z_DATA_ERROR:    return "data error";
    case Z_MEM_ERROR:     return "memory error";
    case Z_VERSION_ERROR: return "version error";
    case Z_ERRNO:         return "io error";
    default:
      break;
  }
  return "unknown error";
}

iof_status flate_decode_state (iof *I, iof *O, flate_state *state)
{
  z_stream *z;
  int zstatus = Z_OK;
  z = &state->z;
  while (state->status != FLATE_DONE)
  {
    if (state->status == FLATE_IN)
    {
      if (!iof_readable(I))
        return state->flush ? IOFERR : IOFEMPTY;
      state->status = FLATE_OUT;
    }
    z->next_in = (Bytef *)I->pos;
    z->avail_in = (uInt)iof_left(I);
    do {
      if (!iof_writable(O))
        return IOFFULL;
      z->next_out = (Bytef *)O->pos;
      z->avail_out = (uInt)iof_left(O);
      zstatus = inflate(z, Z_NO_FLUSH);
      I->pos += iof_left(I) - z->avail_in;
      O->pos += iof_left(O) - z->avail_out;
      switch (zstatus)
      {
        case Z_OK:
        case Z_STREAM_END:
          break;
        default:
          loggerf("flate decoder %s (%d)", zmess(zstatus), zstatus);
          return IOFERR;
      }
    } while (z->avail_out == 0);
    state->status = zstatus == Z_STREAM_END ? FLATE_DONE : FLATE_IN;
  }
  return IOFEOF;
}

iof_status flate_encode_state (iof *I, iof *O, flate_state *state)
{
  z_stream *z;
  int zstatus;
  z = &state->z;
  while (state->status != FLATE_DONE)
  {
    if (state->status == FLATE_IN)
    {
      if (!iof_readable(I))
        if (!state->flush)
          return IOFEMPTY;
      state->status = FLATE_OUT;
    }
    z->next_in = (Bytef *)I->pos;
    z->avail_in = (uInt)iof_left(I);
    do {
      if (!iof_writable(O))
        return IOFFULL;
      z->next_out = (Bytef *)O->pos;
      z->avail_out = (uInt)iof_left(O);
      zstatus = deflate(z, state->flush ? Z_FINISH : Z_NO_FLUSH);
      I->pos += iof_left(I) - z->avail_in;
      O->pos += iof_left(O) - z->avail_out;
      switch (zstatus)
      {
        case Z_OK:
        case Z_STREAM_END:
          break;
        default:
          loggerf("flate encoder %s (%d)", zmess(zstatus), zstatus);
          return IOFERR;
      }
    } while (z->avail_out == 0);
    state->status = state->flush ? FLATE_DONE : FLATE_IN;
  }
  return IOFEOF;
}


void flate_decoder_close (flate_state *state)
{
  inflateEnd(&state->z);
}

void flate_encoder_close (flate_state *state)
{
  deflateEnd(&state->z);
}

/* filter */

// flate decoder function

static size_t flate_decoder (iof *F, iof_mode mode)
{
  flate_state *state;
  iof_status status;
  size_t tail;

  state = iof_filter_state(flate_state *, F);
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
        status = flate_decode_state(F->next, F, state);
      } while (mode == IOFLOAD && status == IOFFULL && iof_resize_buffer(F));
      return iof_decoder_retval(F, "flate", status);
    case IOFCLOSE:
      flate_decoder_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

// flate encoder function

static size_t flate_encoder (iof *F, iof_mode mode)
{
  flate_state *state;
  iof_status status;

  state = iof_filter_state(flate_state *, F);
  switch (mode)
  {
    case IOFFLUSH:
      state->flush = 1;
      // fall through
    case IOFWRITE:
      F->end = F->pos;
      F->pos = F->buf;
      status = flate_encode_state(F, F->next, state);
      return iof_encoder_retval(F, "flate", status);
    case IOFCLOSE:
      if (!state->flush)
        flate_encoder(F, IOFFLUSH);
      flate_encoder_close(state);
      iof_free(F);
      return 0;
    default:
      break;
  }
  return 0;
}

iof * iof_filter_flate_decoder (iof *N)
{
  iof *I;
  flate_state *state;
  I = iof_filter_reader(flate_decoder, sizeof(flate_state), &state);
  iof_setup_next(I, N);
  if (flate_decoder_init(state) == NULL)
  {
    iof_discard(I);
    return NULL;
  }
  state->flush = 1;
  return I;
}

iof * iof_filter_flate_encoder (iof *N)
{
  iof *O;
  flate_state *state;
  O = iof_filter_writer(flate_encoder, sizeof(flate_state), &state);
  iof_setup_next(O, N);
  if (flate_encoder_init(state) == NULL)
  {
    iof_discard(O);
    return NULL;
  }
  return O;
}
