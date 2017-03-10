/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#define LZW_NORMAL 0
#define LZW_EOD 1

/* user visible state */

struct lzw_stream_s {
  const char *next_in; /* pointer to next input character */
  int avail_in;        /* number of input chars available */
  char *next_out;      /* pointer to next free byte in output buffer */
  int avail_out;       /* remaining size of output buffer */

  void *internal;      /* internal state, not user accessible */
};
typedef struct lzw_stream_s lzw_stream_t;

/* user visible functions */

/* The interface for compression and decompression is the same.  The
   application must first call lzw_init to create and initialize a
   compression object.  Then it calls lzw_compress on this object
   repeatedly, as follows: next_in and next_out must point to valid,
   non-overlapping regions of memory of size at least avail_in and
   avail_out, respectively.  The lzw_compress function will read and
   process as many input bytes as possible as long as there is room in
   the output buffer. It will update next_in, avail_in, next_out, and
   avail_out accordingly. Some input may be consumed without producing
   any output, or some output may be produced without consuming any
   input. However, the lzw_compress function makes progress in the
   sense that, after calling this function, at least one of avail_in
   or avail_out is guaranteed to be 0. The mode flag is normally set
   to LZW_NORMAL. It can be set to LZW_EOD (end of data) to indicate
   that the current input buffer represents the entire remaining input
   data stream.  When called with mode=LZW_EOD, and avail_out is
   non-zero after the call, then the application may conclude that the
   end of output has been reached. (However, if avail_out==0 after the
   call, then lzw_compress should be called again with the remaining
   input, if any). Finally, lzw_free should be called to deallocate
   the lzw_stream. Lzw_init returns NULL on error, with errno
   set. Lzw_compress returns 0 on success, and 1 on error with errno
   set. EINVAL is used to indicate an internal error, which should not
   happen. */

lzw_stream_t *lzw_init(void);
int lzw_compress(lzw_stream_t *s, int mode);
void lzw_free(lzw_stream_t *s);
