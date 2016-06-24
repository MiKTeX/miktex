/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


/* code for adaptive LZW compression, as used in PostScript. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lists.h"
#include "bitops.h"
#include "lzw.h"

/* ---------------------------------------------------------------------- */
/* compression state specification */

/* The compression algorithm follows the following specification,
   expressed as a state machine. A state is a triple {s,d,n}, where s
   is a string of input symbols, d is a dictionary, which is a
   function from strings to output symbols, and n is the dictionary
   size, or equivalently, the next unused output symbol. There are
   also special states init and stop. emit[b, code] is a function
   which emits the code 'code' as a b-bit value into the output
   bitstream. hibit(n) returns the least number of binary digits
   required to represent n.

   init ---> {[], newdict, 258}

     where [] is the empty string, and newdict maps each of the 256
     singleton strings to itself. (Note that there are two special
     output symbols 256 and 257, so that the next available one is
     258). Note: hibit(258)=9.

   {[], d, n} (input c) ---> (emit[hibit(n), 256]) {c, d, n}

   {s,d,n} (input c) ---> {s*c,d,n}

     if s!=[], s*c is in the domain of d. Here s*c is the strings s
     extended by the character c.

   {s,d,n} (input c) ---> (emit[hibit(n), d(s)]) {c,d',n+1}

     if s!=[], s*c is not in the domain of d, and hibit(n+2) <= 12.
     Here d'=d+{s*c->n}.

   {s,d,n} (input c) ---> 
           (emit[hibit(n), d(s)]) (emit[hibit(n+1), 256]) {c, newdict, 258}

     if s!=[], s*c is not in the domain of d, and hibit(n+2) > 12.

   {s,d,n} (input EOD) ---> (emit[hibit(n), d(s)]) (emit[hibit(n+1), 257]) stop

     where s != []. Here, EOD stands for end-of-data.

   {[],d,n} (input EOD) ---> (emit[hibit(n), 256]) (emit[hibit(n), 257]) stop

   Notes: 

   * Each reachable state {s,d,n} satisfies hibit(n+1) <= 12.
   * Only codes of 12 or fewer bits are emitted.
   * Each reachable state {s,d,n} satisfies s=[] or s is in the domain of d.
   * The domain of d is always prefix closed (except for the empty prefix)
   * The state machine is deterministic and non-blocking.

*/
   

/* ---------------------------------------------------------------------- */
/* private state */

#define BITBUF_TYPE unsigned int

/* the dictionary is implemented as a tree of strings under the prefix
   order. The tree is in turns represented as a linked list of
   lzw_dict_t structures, with "children" pointing to a node's first
   child, and "next" pointing to a node's next sibling. As an
   optimization, the top-level nodes (singleton strings) are
   implemented lazily, i.e., the corresponding entry is not actually
   created until it is accessed. */

struct lzw_dict_s {
  char c;      /* last character of string represented by this entry */
  int code;    /* code for the string represented by this entry */
  int freq;    /* how often searched? For optimization only */
  struct lzw_dict_s *children;  /* list of sub-entries */
  struct lzw_dict_s *next;      /* for making a linked list */
};
typedef struct lzw_dict_s lzw_dict_t;

/* A state {s,d,n} is represented by the "dictionary state" part of
   the lzw_state_t structure. Here, s is a pointer directly to the node
   of the dictionary structure corresponding to the string s, or NULL
   if s=[]. Further, the lzw_state_t structure contains a buffer of
   pending output bits, and a flag indicating whether the EOD (end of
   data) has been reached in the input. */

struct lzw_state_s {
  /* dictionary state */
  int n;           /* current size of the dictionary */
  lzw_dict_t *d;     /* pointer to dictionary */
  lzw_dict_t *s;     /* pointer to current string, or NULL at beginning */

  /* buffers for pending output */
  BITBUF_TYPE buf; /* bits scheduled for output - left aligned, 0 padded */
  int bufsize;     /* number of bits scheduled for output. */
  int eod;         /* flush buffer? */
};
typedef struct lzw_state_s lzw_state_t;

/* ---------------------------------------------------------------------- */
/* auxiliary functions which operate on dictionary states */

/* recursively free an lzw_dict_t object */
static void lzw_free_dict(lzw_dict_t *s) {
  lzw_dict_t *e;

  list_forall_unlink(e, s) {
    lzw_free_dict(e->children);
    free(e);
  }
}

/* re-initialize the lzw state's dictionary state to "newdict",
   freeing any old dictionary. */
static void lzw_clear_table(lzw_state_t *st) {

  lzw_free_dict(st->d);
  st->d = NULL;
  st->n = 258;
  st->s = NULL;
}

/* ---------------------------------------------------------------------- */
/* auxiliary functions for reading/writing the bit buffer */

/* write the code to the bit buffer. Precondition st->bufsize <= 7.
   Note: this does not change the dictionary state; in particular,
   n must be updated between consecutive calls. */
static inline void lzw_emit(int code, lzw_state_t *st) {
  BITBUF_TYPE mask;
  int bits = hibit(st->n);

  /* fill bit buffer */
  mask = (1 << bits) - 1;
  code &= mask;

  st->buf |= code << (8*sizeof(BITBUF_TYPE) - st->bufsize - bits);
  st->bufsize += bits;
}

/* transfer one byte from bit buffer to output. Precondition:
   s->avail_out > 0. */
static inline void lzw_read_bitbuf(lzw_stream_t *s) {
  int ch;
  lzw_state_t *st = (lzw_state_t *)s->internal;

  ch = st->buf >> (8*sizeof(BITBUF_TYPE)-8);
  st->buf <<= 8;
  st->bufsize -= 8;

  s->next_out[0] = ch;
  s->next_out++;
  s->avail_out--;
}

/* ---------------------------------------------------------------------- */
/* The following functions implement the state machine. */

/* perform state transition of the state st on input character
   ch. This updates the dictionary state and/or writes to the bit
   buffer. Precondition: st->bufsize <= 7. Return 0 on success, or 1
   on error with errno set. */
static int lzw_encode_char(lzw_state_t *st, char c) {
  lzw_dict_t *e;

  /* st = {s,d,n}. hibit(n+1)<=12. */

  /* {[], d, n} (input c) ---> (emit[hibit(n), 256]) {c, d, n} */
  if (st->s == NULL) {
    lzw_emit(256, st);
    goto singleton;  /* enter singleton state c */
  } 

  /* {s,d,n} (input c) ---> {s*c,d,n} */
  list_find(e, st->s->children, e->c == c);
  if (e) {
    e->freq++;
    st->s = e;
    return 0;
  }

  /* {s,d,n} (input c) ---> (emit[hibit(n), d(s)]) {c,d',n+1} */
  /* {s,d,n} (input c) --->
	    (emit[hibit(n), d(s)]) (emit[hibit(n+1), 256]) {c, newdict, 258} */

  lzw_emit(st->s->code, st); /* 9-12 bits */
  if (st->n >= 4094) {   /* hibit(n+2) > 12 */
    st->n++;
    lzw_emit(256, st);
    goto dictfull;    /* reset dictionary and enter singleton state c */
  }

  /* insert new code in dictionary, if possible */
  e = (lzw_dict_t *)malloc(sizeof(lzw_dict_t));
  if (!e) {
    return 1;
  }
  e->c = c;
  e->code = st->n;
  e->freq = 1;
  e->children = NULL;
  list_prepend(st->s->children, e);
  st->n++;
  goto singleton;  /* enter singleton state c */

 dictfull:    /* reset dictionary and enter singleton state c */
  lzw_clear_table(st);
  /* fall through */
  
 singleton:   /* enter singleton state c */
  list_find(e, st->d, e->c == c);
  if (!e) {  /* not found: lazily add it */
    e = (lzw_dict_t *)malloc(sizeof(lzw_dict_t));
    if (!e) {
      return 1;
    }
    e->c = c;
    e->code = (int)(unsigned char)c;
    e->freq = 0;
    e->children = NULL;
    list_prepend(st->d, e);
  }
  e->freq++;
  st->s = e;
  return 0;
}

/* perform state transition of the state st on input EOD. The leaves
   the dictionary state undefined and writes to the bit buffer.
   Precondition: st->bufsize <= 7. This function must be called
   exactly once, at the end of the stream. */
static void lzw_encode_eod(lzw_state_t *st) {

  /* {[],d,n} (input EOD) ---> 
              (emit[hibit(n), 256]) (emit[hibit(n), 257]) stop */
  if (st->s == NULL) {
    lzw_emit(256, st);  /* 9 bits */
    st->n=258;
    lzw_emit(257, st);  /* 9 bits */
    return;
  } 

  /* {s,d,n} (input EOD) ---> 
             (emit[hibit(n), code]) (emit[hibit(n+1), 257]) stop */

  lzw_emit(st->s->code, st); /* 9-12 bits */
  st->n++;
  lzw_emit(257, st);  /* 9-12 bits */
  return;
}

/* ---------------------------------------------------------------------- */
/* User visible functions. These implement a buffer interface. See
   lzw.h for the API description. */

lzw_stream_t *lzw_init(void) {
  lzw_stream_t *s = NULL;
  lzw_state_t *st = NULL;

  s = (lzw_stream_t *)malloc(sizeof(lzw_stream_t));
  if (s==NULL) {
    goto fail;
  }
  st = (lzw_state_t *)malloc(sizeof(lzw_state_t));
  if (st==NULL) {
    goto fail;
  }
  st->buf = 0;
  st->bufsize = 0;
  st->eod = 0;
  st->d = NULL;
  lzw_clear_table(st);
  s->internal = (void *) st;
  return s;

 fail:
  free(s);
  free(st);
  return NULL;
}

int lzw_compress(lzw_stream_t *s, int mode) {
  int r;
  lzw_state_t *st = (lzw_state_t *)s->internal;

  while (st->eod == 0) {
    /* empty bit buffer */
    while (st->bufsize > 7) {
      if (s->avail_out == 0) {
	return 0;
      } else {
	lzw_read_bitbuf(s);
      }
    }
    /* fill bit buffer */
    if (s->avail_in == 0) {
      break;
    } else {
      r = lzw_encode_char(st, s->next_in[0]);
      if (r) {
	if (r==2) {
	  errno = EINVAL;
	}
	return 1;
      }
      s->next_in++;
      s->avail_in--;
    }
  }

  if (mode==LZW_EOD && st->eod == 0) {
    st->eod = 1;
    lzw_encode_eod(st);
  }

  /* flush bit buffer */
  if (st->eod) {
    while (st->bufsize > 0) {
      if (s->avail_out == 0) {
	return 0;
      } else {
	lzw_read_bitbuf(s);
      }
    }
  }

  return 0;
}

void lzw_free(lzw_stream_t *s) {
  lzw_state_t *st = (lzw_state_t *)s->internal;

  lzw_free_dict(st->d);
  free(st);
  free(s);
}

/* ---------------------------------------------------------------------- */
/* main function for testing and illustration purposes */

#ifdef LZW_MAIN

int main() {
  lzw_stream_t *s;
  int ch;
  char inbuf[100];
  char outbuf[100];
  int i, r;
  int mode;

  s = lzw_init();
  if (!s) {
    goto error;
  }
  mode = LZW_NORMAL;

  while (1) {
    /* fill inbuf */
    for (i=0; i<100; i++) {
      ch = fgetc(stdin);
      if (ch==EOF) {
	break;
      }
      inbuf[i] = ch;
    }
    if (i<100) {   /* end of input */
      mode = LZW_EOD;
    }

    /* compress */
    s->next_in = inbuf;
    s->avail_in = i;
    do {
      s->next_out = outbuf;
      s->avail_out = 100;
      r = lzw_compress(s, mode);
      if (r) {
	goto error;
      }
      fwrite(outbuf, 1, 100-s->avail_out, stdout);
    } while (s->avail_out==0);
    if (mode == LZW_EOD) {
      break;
    }
  }
  fflush(stdout);
  lzw_free(s);

  return 0;

 error:
  fprintf(stderr, "lzw: %s\n", strerror(errno));
  lzw_free(s);
  return 1;

}
#endif

