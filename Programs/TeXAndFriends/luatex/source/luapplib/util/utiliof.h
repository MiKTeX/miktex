
#ifndef UTIL_IOF_H
#define UTIL_IOF_H

#include <stdio.h>  // for FILE *
#include <errno.h>  // for errno
#include <string.h> // for strerror()
#include <stdint.h> // for uintN_t

#include "utildecl.h"
#include "utilnumber.h"

/* handler call modes */

typedef enum {
  IOFREAD  = 0, /* read to buffer */
  IOFLOAD  = 1, /* read all to buffer */
  IOFWRITE = 2, /* write buffer to the output */
  IOFFLUSH = 3, /* flush buffer to the output */
  IOFCLOSE = 4  /* (flush and) close */
} iof_mode;

/* return statuses */

typedef enum {
  IOFEOF   = -1, /* end of input */
  IOFEMPTY = -2, /* end of input buffer*/
  IOFFULL  = -3, /* end of output buffer */
  IOFERR   = -4  /* error */
} iof_status;

const char * iof_status_kind (iof_status status);

/* iof_file */

typedef struct iof_file {
  union {
    FILE *iofh; // access via iof_file_get_fh / iof_file_set_fh (below)
    union {
    	struct { uint8_t *buf, *pos, *end; };
    	struct { const uint8_t *rbuf, *rpos, *rend; }; // to trick compiler warnings about cast discarding const
    };
  };
  size_t *offset;
  char *name;
  size_t size;
  int refcount;
  int flags;
} iof_file;

/* iof handler function */

typedef struct iof iof;
typedef size_t (*iof_handler) (iof *I, iof_mode mode);

/* iof structure */

#define IOF_MEMBERS \
  union { \
    struct { uint8_t *buf, *pos, *end; }; \
    struct { uint16_t *hbuf, *hpos, *hend; }; \
    struct { uint32_t *ibuf, *ipos, *iend; }; \
    struct { const uint8_t *rbuf, *rpos, *rend; }; \
  }; \
  size_t space; \
  iof_handler more; \
  union { iof *next; FILE *file; iof_file *iofile; void *link; }; \
  int flags; \
  int refcount

/*
  buf -- the beginning of buffer
  pos -- the current position
  end -- the end of buffer
  space -- private space size, not always eq. (end - buf)
  more -- handler function
  next/file/iofile/link -- reader source or writer target
  source -- source filter
  flags -- private filter info
  refcount -- refcount
*/

struct iof {
  IOF_MEMBERS;
};

typedef void (*iof_dump_function) (const void *value, iof *O);

/* flags */

#define IOF_ALLOC          (1<<0) // iof is allocated
#define IOF_HEAP           (1<<1) // iof taken from iof heap
#define IOF_BUFFER_ALLOC   (1<<2) // buffer allocated
#define IOF_BUFFER_HEAP    (1<<3) // buffer taken from iof heap

#define IOF_SHORT          (1<<4) // buffer uses 16bit integers
#define IOF_LONG           (1<<5) // buffer uses 32bit integers

#define IOF_TAIL           (1<<6) // preserve reader tail
#define IOF_READER         (1<<7) // is reader
#define IOF_WRITER         (1<<8) // is writer

#define IOF_DATA           (1<<9)  // binds some memory
#define IOF_FILE_HANDLE    (1<<10) // links FILE *
#define IOF_FILE           (1<<11) // links iof_file *
#define IOF_NEXT           (1<<12) // links next iof *
#define IOF_CLOSE_FILE     (1<<13) // close FILE * on free
#define IOF_REOPEN_FILE    (1<<14) // close/reopen mode for iof_file
#define IOF_RECLOSE_FILE   (1<<15) // ditto

#define IOF_STOPPED        (1<<16) // stopped

// #define IOF_CUSTOM         (1<<17) // first custom flag

#define IOF_BUFSIZ (sizeof(iof) + BUFSIZ*sizeof(uint8_t))

/*
reading buffer -- all of buf, pos, end pointers are initialized to the beginning of the private buffer,
  next call to a handler function moves the end pointer to bufer+space
writer -- buf and pos pointers initialized to the beginning of the buffer, end initialized to bufer+space

Every call to handler returns size_t number of bytes
available (to write/read) or 0 if there is no more space.

We usually align the data buffer just after the iof structure.
This is convenient, especially when a memory for the structure
and its buffer is to be allocated. In the case of growing output
buffers we used to check if the memory of the buffer is allocated
by the handler function using test (O->buf != (O+1)). We don't use
it any longer not to rely on little secrets. Now there is an explicit
IOF_BUFFER_ALLOC flag for that. IOF_ALLOC tells if the structure
itself is taken from malloc (not used so far). Assuming the buffer size
is way larger the sizeof(iof)
*/

/* initializers */

#define IOF_READER_STRUCT(handler, file, buffer, size, flags) \
  { {{ (uint8_t *)(buffer), (uint8_t *)(buffer), (uint8_t *)(buffer) }}, size, handler, { file }, flags|IOF_READER, 0 }

#define IOF_WRITER_STRUCT(handler, file, buffer, size, flags) \
  { {{ (uint8_t *)(buffer), (uint8_t *)(buffer), (uint8_t *)(buffer) + size }}, size, handler, { file }, flags|IOF_WRITER, 0 }

#define IOF_STRING_STRUCT(buffer, size) \
  { {{ (uint8_t *)(buffer), (uint8_t *)(buffer), (uint8_t *)(buffer) + size }}, size, NULL, { NULL }, 0|IOF_READER|IOF_DATA, 0 }

#define IOF_STRING() IOF_STRING_STRUCT(0, 0)

/* refcount */

#define iof_incref(I) (++(I)->refcount)
#define iof_decref(I) ((void)(--(I)->refcount <= 0 && iof_close(I)))
#define iof_unref(I) (--(I)->refcount)

/* setting up iof and buffer from mem buffer of a given size */

#define iof_setup_reader(I, buffer, size) \
  ((I) = (iof *)(buffer), iof_reader_buffer(I, (I)+1, size - sizeof(iof)))

#define iof_setup_writer(O, buffer, size) \
  ((O) = (iof *)buffer, iof_writer_buffer(O, (O)+1, size - sizeof(iof)))

/* binding buffer of a given size */

#define iof_reader_buffer(I, buffer, size) \
  ((I)->buf = (I)->pos = (I)->end = (uint8_t *)(buffer), \
   (I)->space = size, (I)->flags = 0|IOF_READER, (I)->refcount = 0)

#define iof_writer_buffer(O, buffer, size) \
  ((O)->buf = (O)->pos = (uint8_t *)(buffer), \
   (O)->end = (uint8_t *)(buffer) + size, \
   (O)->space = size, (O)->flags = 0|IOF_WRITER, (O)->refcount = 0)

/* basics */

#define iof_space(I) ((I)->end - (I)->buf)
#define iof_left(I)  ((I)->end - (I)->pos)
#define iof_size(I)  ((I)->pos - (I)->buf)

#define iof_input(I)  ((I)->more ? (I)->more((I), IOFREAD) : 0lu)
#define iof_load(I)   ((I)->more ? (I)->more((I), IOFLOAD) : 0lu)

#define iof_output(O) ((O)->more ? (O)->more((O), IOFWRITE) : 0lu)
//#define iof_flush(O)  ((O)->pos > (O)->buf && (O)->more ? (O)->more(O, IOFFLUSH) : 0lu)
// flush should be unconditional, because encoders emits EOD markers only on flush
#define iof_flush(O) ((O)->more ? (O)->more(O, IOFFLUSH) : 0lu)
#define iof_close(O)  ((O)->more ? (O)->more(O, IOFCLOSE) : 0lu)

#define iof_stop(F) ((void)(F->pos = F->end = F->buf, F->flags |= IOF_STOPPED))

/*
Rewriting reader tail to the beginning of new data portion; readers reacting on IOFREAD
mode must be aware of some not yet read data, but treat it necessary only if IOF_TAIL flag is set.
Parsers using iof input may protect not yet read data when there may be a need to put bytes
back to the stream. This is trivial when I->pos > I->buf, as we can make a move by --I->pos.
But when there is a need to put back more then one byte, we can protect the data tail, so that
realoder will rewrite it to the beginning of new data chunk. 

  iof_tail(I) - internal, used by iof handlers at IOFREAD mode
  iof_protect_tail(I) - used by parsers to ensure some bytes chunk in one piece

*/

size_t iof_save_tail (iof *I);
#define iof_tail(I) (((I)->flags & IOF_TAIL) && (I)->pos < (I)->end ? iof_save_tail(I) : 0)

size_t iof_input_save_tail (iof *I, size_t back);
#define iof_protect_tail(I, back, length) ((iof_left(I) >= (length) - (back)) ? 1 : (iof_input_save_tail(I, back) >= length - back))

//uint8_t * iof_tail_data (iof *I, size_t *ptail);
//#define iof_tail_free(data) util_free(data)

/* panic */

// #define iof_panic(mess) return 0
#ifndef iof_panic
  #define iof_panic(mess) (fputs(mess, stderr), abort())
#endif
//#define iof_memory_error() iof_panic(strerror(errno))
#define iof_fwrite_error() iof_panic(strerror(errno))

/* generic helpers */

UTILAPI uint8_t * iof_copy_file_data (const char *filename, size_t *psize);
UTILAPI uint8_t * iof_copy_file_handle_data (FILE *file, size_t *psize);

/* In the future we may need releasing file handle and restoring it from iofile->name, so access file handle via macros */

#define iof_file_get_fh(iofile) ((iofile)->iofh)
#define iof_file_set_fh(iofile, fh) ((iofile)->iofh = fh)
#define iof_file_get_file(iofile) (((iofile)->flags & IOF_DATA) ? NULL : iof_file_get_fh(iofile))
FILE * iof_get_file (iof *F);

/* basic iof_file interface */

iof_file * iof_file_new (FILE *file);
iof_file * iof_file_init (iof_file *iofile, FILE *file);

iof_file * iof_file_rdata (const void *data, size_t size);
iof_file * iof_file_wdata (void *data, size_t size);

iof_file * iof_file_rdata_init (iof_file *iofile, const void *data, size_t size);
iof_file * iof_file_wdata_init (iof_file *iofile, void *data, size_t size);

iof_file * iof_file_reader_from_file_handle (iof_file *iofile, const char *filename, FILE *file, int preload, int closefile);
iof_file * iof_file_reader_from_file (iof_file *iofile, const char *filename, int preload);
iof_file * iof_file_reader_from_data (iof_file *iofile, const void *data, size_t size, int preload, int freedata);
//iof_file * iof_file_writer_from_file (iof_file *iofile, const char *filename);

void * iof_copy_data (const void *data, size_t size);
#define iof_data_free(data) util_free(data)
#define iof_file_wdata_copy(data, size) iof_file_wdata(iof_copy_data(data, size), size)
#define iof_file_rdata_copy(data, size) iof_file_rdata(iof_copy_data(data, size), size)

void iof_file_free (iof_file *iofile);

#define iof_file_get_name(iofile) ((iofile)->name)
void iof_file_set_name (iof_file *iofile, const char *name);

#define iof_file_incref(iofile) (++(iofile)->refcount)
#define iof_file_decref(iofile) ((void)(--(iofile)->refcount <= 0 && (iof_file_free(iofile), 0)))

int iof_file_seek (iof_file *iofile, long offset, int whence);
long iof_file_tell (iof_file *iofile);
size_t iof_file_size (iof_file *iofile);
int iof_file_eof (iof_file *iofile);

size_t iof_file_read (void *ptr, size_t size, size_t items, iof_file *iofile);
size_t iof_file_write (const void *ptr, size_t size, size_t items, iof_file *iofile);
size_t iof_file_ensure (iof_file *iofile, size_t bytes);
int iof_file_flush (iof_file *iofile);

int iof_file_getc (iof_file *iofile);
int iof_file_putc (iof_file *iofile, int c);

int iof_file_close_input (iof_file *iofile);
int iof_file_reopen_input (iof_file *iofile);

#define iof_file_reopen(iofile) (((iofile)->flags & IOF_REOPEN_FILE) ? iof_file_reopen_input(iofile) : 1)
#define iof_file_reclose(iofile) (void)(((iofile)->flags & IOF_RECLOSE_FILE) ? iof_file_close_input(iofile) : 0)

/* wrappers of basic operations for iof */

int iof_reader_seek (iof *I, long offset, int whence);
int iof_reader_reseek (iof *I, long offset, int whence);
int iof_writer_seek (iof *I, long offset, int whence);
int iof_writer_reseek (iof *I, long offset, int whence);

int iof_seek (iof *I, long offset, int whence);
int iof_reseek (iof *I, long offset, int whence);

long iof_reader_tell (iof *I);
long iof_writer_tell (iof *I);
long iof_tell (iof *I);
size_t iof_fsize (iof *I);

#define iof_setup_iofile(I, f) (iof_file_incref(f), (I)->iofile = f, (I)->flags |= IOF_FILE)
#define iof_setup_file(I, fh) ((I)->file = fh, (I)->flags |= IOF_FILE_HANDLE)
#define iof_setup_next(I, N) ((I)->next = N, iof_incref(N), (I)->flags |= IOF_NEXT)

/* file handler reader and writer */

UTILAPI iof * iof_setup_file_handle_reader (iof *I, void *buffer, size_t space, FILE *f);
UTILAPI iof * iof_setup_file_handle_writer (iof *O, void *buffer, size_t space, FILE *f);

#define iof_get_file_handle_reader(buffer, space, fh) iof_setup_file_handle_reader(NULL, buffer, space, fh)
#define iof_get_file_handle_writer(buffer, space, fh) iof_setup_file_handle_writer(NULL, buffer, space, fh)

/* file reader and writer */

UTILAPI iof * iof_setup_file_reader (iof *I, void *buffer, size_t space, const char *filename);
UTILAPI iof * iof_setup_file_writer (iof *O, void *buffer, size_t space, const char *filename);

#define iof_get_file_reader(buffer, space, filename) iof_setup_file_reader(NULL, buffer, space, filename)
#define iof_get_file_writer(buffer, space, filename) iof_setup_file_writer(NULL, buffer, space, filename)

/* mem writer */

UTILAPI iof * iof_setup_buffer (iof *O, void *buffer, size_t space);
UTILAPI iof * iof_setup_buffermin (iof *O, void *buffer, size_t space, size_t min);

#define iof_buffer(buffer, space) iof_setup_buffer(NULL, buffer, space)
#define iof_buffermin(buffer, space, min) iof_setup_buffermin(NULL, buffer, space, min)

UTILAPI iof * iof_buffer_create (size_t space);
#define iof_buffer_new() iof_buffer_create(BUFSIZ)

/* custom handler */

UTILAPI iof * iof_reader (iof *I, void *link, iof_handler reader, const void *s, size_t bytes);
UTILAPI iof * iof_writer (iof *O, void *link, iof_handler writer,       void *s, size_t bytes);

/* stdout wrapper */

extern UTILAPI iof iof_stdout;
extern UTILAPI iof iof_stderr;

/* simple string reader */

UTILAPI iof * iof_string_reader (iof *I, const void *s, size_t bytes);

#define iof_string(I, s, bytes) \
  (((I)->rbuf = (I)->rpos = (const uint8_t *)s), ((I)->rend = (I)->rbuf + (bytes)), ((I)->flags |= IOF_DATA), (I))

/* dummies */

UTILAPI iof * iof_dummy (void *buffer, size_t space);
UTILAPI iof * iof_null (void *buffer, size_t space);

/* checking available space */

#define iof_loadable(I) ((I)->pos < (I)->end || iof_load(I))
#define iof_readable(I) ((I)->pos < (I)->end || iof_input(I))
#define iof_writable(O) ((O)->pos < (O)->end || iof_output(O))

#define iof_hloadable iof_loadable
#define iof_iloadable iof_loadable

#define iof_hreadable iof_readable
#define iof_ireadable iof_readable

#define iof_hwritable iof_writable
#define iof_iwritable iof_writable

/* ensure space to write several bytes (several means less then I->space) */

#define iof_ensure(O, n) ((O)->pos+(n)-1 < (O)->end || iof_output(O)) // iof_ensure(O, 1) eq iof_writable(O)
#define iof_hensure(O, n) ((O)->hpos+(n)-1 < (O)->hend || iof_output(O))
#define iof_iensure(O, n) ((O)->ipos+(n)-1 < (O)->iend || iof_output(O))

/* reading */

UTILAPI int iof_getc (iof *I);
UTILAPI int iof_hgetc (iof *I);
UTILAPI int iof_igetc (iof *I);

// UTILAPI int iof_cmp (iof *I, const char *s);
// UTILAPI int iof_cmpn (iof *I, const char *s, size_t bytes);

UTILAPI iof_status iof_pass (iof *I, iof *O);
#define iof_hpass iof_pass
#define iof_ipass iof_pass

/* readers helpers */

UTILAPI size_t iof_read (iof *I, void *s, size_t bytes);
UTILAPI size_t iof_hread (iof *I, void *s, size_t bytes);
UTILAPI size_t iof_iread (iof *I, void *s, size_t bytes);

UTILAPI size_t iof_skip (iof *I, size_t bytes);
UTILAPI size_t iof_hskip (iof *I, size_t bytes);
UTILAPI size_t iof_iskip (iof *I, size_t bytes);

/* get */

#define iof_pos(I)  (*(I)->pos++)
#define iof_hpos(I) (*(I)->hpos++)
#define iof_ipos(I) (*(I)->ipos++)

#define iof_get(I)  (iof_readable(I)  ? (int)(*(I)->pos++)  : IOFEOF)
#define iof_hget(I) (iof_hreadable(I) ? (int)(*(I)->hpos++) : IOFEOF)
#define iof_iget(I) (iof_ireadable(I) ? (int)(*(I)->ipos++) : IOFEOF)

#define iof_char(I)  (iof_readable(I)  ? (int)(*(I)->pos) : IOFEOF)
#define iof_hcurr(I) (iof_hreadable(I) ? (int)(*(I)->hpos) : IOFEOF)
#define iof_icurr(I) (iof_ireadable(I) ? (int)(*(I)->ipos) : IOFEOF)

#define iof_next(I)  (++(I)->pos, iof_char(I))
#define iof_hnext(I) (++(I)->hpos, iof_hcurr(I))
#define iof_inext(I) (++(I)->ipos, iof_icurr(I))

/* unget */

/*
If possible, we just move the position backward. If it is not possible to
move backward, we call iof_backup(I, c) that sets all pointers to the end of
a private backup space, then moves buf AND pos pointers backward and set c at
pos (==buf). We can backup characters as long as there is a private space. If
several calls to iof_backup() are followed by iof_get(), pos pointer
increases in normal way and so the use of another iof_unget() works just fine
by moving the position. Once we swallow all backup characters (when
pos==end), backup handler restores the previous pointers.

Obviously we assume that the character provided to iof_unget() is always the
character just obtained from iof_get(). We CAN'T just overwrite the character
at a given position as the space we read may not be writable.

When backup is in use, we can only get bytes until automatically restored.
*/

/* backup */

/*
#define iof_uses_backup(I) ((I)->more == iof_unget_handler)

#define iof_save(I, B) \
  ((B)->buf = (I)->buf, (B)->pos = (I)->pos, (B)->end = (I)->end, (B)->space = (I)->space, \
   (B)->link = I->link, (B)->more = (I)->more, (B)->flags = (I)->flags)
#define iof_restore(B, I) iof_save(I, B)

#define iof_unget(I, c) \
  ((void)(c == (uint8_t)c ? ((I)->pos > (I)->buf ? --(I)->pos : iof_backup(I, c)) : 0)
int iof_backup (iof *I, int c);
*/

/* writing */

UTILAPI size_t iof_write_file_handle (iof *O, FILE *file);
UTILAPI size_t iof_write_file (iof *O, const char *filename);
UTILAPI size_t iof_write_iofile (iof *O, iof_file *iofile, int savepos);

UTILAPI int iof_putc (iof *O, int u);
UTILAPI int iof_hputc (iof *O, int u);
UTILAPI int iof_iputc (iof *O, int u);

UTILAPI size_t iof_write (iof *O, const void *data, size_t size);
UTILAPI size_t iof_hwrite (iof *O, const void *data, size_t size);
UTILAPI size_t iof_iwrite (iof *O, const void *data, size_t size);

UTILAPI iof_status iof_puts (iof *O, const void *data);
UTILAPI size_t iof_put_string (iof *O, const void *data);
UTILAPI size_t iof_putfs (iof *O, const char *format, ...);
UTILAPI size_t iof_repc (iof *O, char c, size_t bytes);

#define iof_putl(O, s) iof_write(O, "" s, sizeof(s)-1)
//#define iof_putl iof_puts

#define iof_set(O, c)               (*(O)->pos++ = (uint8_t)(c))
#define iof_set2(O, c1, c2)         (iof_set(O, c1), iof_set(O, c2))
#define iof_set3(O, c1, c2, c3)     (iof_set(O, c1), iof_set(O, c2), iof_set(O, c3))
#define iof_set4(O, c1, c2, c3, c4) (iof_set(O, c1), iof_set(O, c2), iof_set(O, c3), iof_set(O, c4))
#define iof_set5(O, c1, c2, c3, c4, c5) (iof_set(O, c1), iof_set(O, c2), iof_set(O, c3), iof_set(O, c4), iof_set(O, c5))

#define iof_hset(O, c)              (*(O)->hpos++ = (uint16_t)(c))
#define iof_iset(O, c)              (*(O)->ipos++ = (uint32_t)(c))

#define iof_put(O, c)               ((void)iof_ensure(O, 1), iof_set(O, c))
#define iof_put2(O, c1, c2)         ((void)iof_ensure(O, 2), iof_set2(O, c1, c2))
#define iof_put3(O, c1, c2, c3)     ((void)iof_ensure(O, 3), iof_set3(O, c1, c2, c3))
#define iof_put4(O, c1, c2, c3, c4) ((void)iof_ensure(O, 4), iof_set4(O, c1, c2, c3, c4))
#define iof_put5(O, c1, c2, c3, c4, c5) ((void)iof_ensure(O, 5), iof_set5(O, c1, c2, c3, c4, c5))

#define iof_hput(O, c)               ((void)iof_hensure(O, 1), iof_hset(O, c))
#define iof_iput(O, c)               ((void)iof_iensure(O, 1), iof_iset(O, c))

#define iof_put_uc_hex(O, c) iof_put2(O, base16_uc_digit1(c), base16_uc_digit2(c))
#define iof_put_lc_hex(O, c) iof_put2(O, base16_lc_digit1(c), base16_lc_digit2(c))
#define iof_set_uc_hex(O, c) iof_set2(O, base16_uc_digit1(c), base16_uc_digit2(c))
#define iof_set_lc_hex(O, c) iof_set2(O, base16_lc_digit1(c), base16_lc_digit2(c))
#define iof_put_hex iof_put_uc_hex
#define iof_set_hex iof_set_uc_hex

/* number from iof; return 1 on success, 0 otherwise */

#define iof_scan_sign(I, c, sign) _scan_sign(c, sign, iof_next(I))
#define iof_scan_integer(I, c, number) _scan_integer(c, number, iof_next(I))
#define iof_scan_radix(I, c, number, radix) _scan_radix(c, number, radix, iof_next(I))
#define iof_read_integer(I, c, number) _read_integer(c, number, iof_next(I))
#define iof_read_radix(I, c, number, radix) _read_radix(c, number, radix, iof_next(I))

#define iof_scan_decimal(I, c, number) _scan_decimal(c, number, iof_next(I))
#define iof_scan_fraction(I, c, number, exponent10) _scan_fraction(c, number, exponent10, iof_next(I))
#define iof_scan_exponent10(I, c, exponent10) _scan_exponent10(c, exponent10, iof_next(I))

UTILAPI int iof_get_int32 (iof *I, int32_t *number);
UTILAPI int iof_get_intlw (iof *I, intlw_t *number);
UTILAPI int iof_get_int64 (iof *I, int64_t *number);

UTILAPI int iof_get_uint32 (iof *I, uint32_t *number);
UTILAPI int iof_get_uintlw (iof *I, uintlw_t *number);
UTILAPI int iof_get_uint64 (iof *I, uint64_t *number);

UTILAPI int iof_get_int32_radix (iof *I, int32_t *number, int radix);
UTILAPI int iof_get_intlw_radix (iof *I, intlw_t *number, int radix);
UTILAPI int iof_get_int64_radix (iof *I, int64_t *number, int radix);

UTILAPI int iof_get_uint32_radix (iof *I, uint32_t *number, int radix);
UTILAPI int iof_get_uintlw_radix (iof *I, uintlw_t *number, int radix);
UTILAPI int iof_get_uint64_radix (iof *I, uint64_t *number, int radix);

UTILAPI int iof_get_roman (iof *I, unsigned short int *number);

UTILAPI int iof_get_double (iof *I, double *number);
UTILAPI int iof_get_float (iof *I, float *number);

UTILAPI int iof_conv_double (iof *I, double *number);
UTILAPI int iof_conv_float (iof *I, float *number);

/* number to iof; return a number of written bytes */

UTILAPI size_t iof_put_int32 (iof *O, int32_t number);
UTILAPI size_t iof_put_intlw (iof *O, intlw_t number);
UTILAPI size_t iof_put_int64 (iof *O, int64_t number);

UTILAPI size_t iof_put_uint32 (iof *O, uint32_t number);
UTILAPI size_t iof_put_uintlw (iof *O, uintlw_t number);
UTILAPI size_t iof_put_uint64 (iof *O, uint64_t number);

UTILAPI size_t iof_put_int32_radix (iof *O, int32_t number, int radix);
UTILAPI size_t iof_put_intlw_radix (iof *O, intlw_t number, int radix);
UTILAPI size_t iof_put_int64_radix (iof *O, int64_t number, int radix);

UTILAPI size_t iof_put_uint32_radix (iof *O, uint32_t number, int radix);
UTILAPI size_t iof_put_uintlw_radix (iof *O, uintlw_t number, int radix);
UTILAPI size_t iof_put_uint64_radix (iof *O, uint64_t number, int radix);

UTILAPI size_t iof_put_roman_uc (iof *O, uint16_t number);
UTILAPI size_t iof_put_roman_lc (iof *O, uint16_t number);
#define iof_put_roman(I, number) iof_put_roman_uc(I, number)

UTILAPI size_t iof_put_double(iof *O, double number, int digits);
UTILAPI size_t iof_put_float(iof *O, float number, int digits);

/* common stuff for binary integers */

UTILAPI int iof_get_be_uint2 (iof *I, uint32_t *pnumber);
UTILAPI int iof_get_be_uint3 (iof *I, uint32_t *pnumber);
UTILAPI int iof_get_be_uint4 (iof *I, uint32_t *pnumber);

UTILAPI int iof_get_le_uint2 (iof *I, uint32_t *pnumber);
UTILAPI int iof_get_le_uint3 (iof *I, uint32_t *pnumber);
UTILAPI int iof_get_le_uint4 (iof *I, uint32_t *pnumber);

// iof_set() and iof_put() suite casts arguments to uint8_t, so we don't need &0xff mask

#define iof_set_be_uint1(O, u) iof_set(O, u)
#define iof_set_be_uint2(O, u) iof_set2(O, (u)>>8, u)
#define iof_set_be_uint3(O, u) iof_set3(O, (u)>>16, (u)>>8, u)
#define iof_set_be_uint4(O, u) iof_set4(O, (u)>>24, (u)>>16, (u)>>8, u)

#define iof_set_le_uint1(O, u) iof_set(O, u)
#define iof_set_le_uint2(O, u) iof_set2(O, u, (u)>>8)
#define iof_set_le_uint3(O, u) iof_set3(O, u, (u)>>8, (u)>>16)
#define iof_set_le_uint4(O, u) iof_set4(O, u, (u)>>8, (u)>>16, (u)>>24)

#define iof_put_be_uint1(O, u) iof_put(O, u)
#define iof_put_be_uint2(O, u) iof_put2(O, (u)>>8, u)
#define iof_put_be_uint3(O, u) iof_put3(O, (u)>>16, (u)>>8, u)
#define iof_put_be_uint4(O, u) iof_put4(O, (u)>>24, (u)>>16, (u)>>8, u)

#define iof_put_le_uint1(O, u) iof_put(O, u)
#define iof_put_le_uint2(O, u) iof_put2(O, u, (u)>>8)
#define iof_put_le_uint3(O, u) iof_put3(O, u, (u)>>8, (u)>>16)
#define iof_put_le_uint4(O, u) iof_put4(O, u, (u)>>8, (u)>>16, (u)>>24)

/* buffer results */

#define iof_reader_result(I, size) ((size = iof_left(I)), (I)->pos)
#define iof_writer_result(I, size) ((size = iof_size(I)), (I)->buf)
#define iof_result(I, size) (((I)->flags & IOF_READER) ? iof_reader_result(I, size) : iof_writer_result(I, size))

uint8_t * iof_file_input_data (iof_file *iofile, size_t *psize, int *isnew);
//uint8_t * iof_file_reader_data (iof_file *iofile, size_t *size);
//uint8_t * iof_file_writer_data (iof_file *iofile, size_t *size);

uint8_t * iof_reader_data (iof *I, size_t *psize);
uint8_t * iof_writer_data (iof *O, size_t *psize);
size_t iof_reader_to_file_handle (iof *I, FILE *file);
size_t iof_reader_to_file (iof *I, const char *filename);

#define iof_loaded(I) ((I)->end = (I)->pos, (I)->pos = (I)->buf, iof_left(I))

#define iof_data_to_file_handle(data, size, file) fwrite(data, sizeof(uint8_t), size, file)
UTILAPI size_t iof_data_to_file (const void *data, size_t size, const char *filename);

UTILAPI size_t iof_result_to_file_handle (iof *F, FILE *file);
UTILAPI size_t iof_result_to_file (iof *F, const char *filename);
UTILAPI void iof_debug (iof *I, const char *filename);

/* common filters allocator */

void iof_filters_init (void);
void iof_filters_free (void);

void * iof_filter_new (size_t size);
void iof_heap_back (void *data);
#define iof_filter_free(F) iof_heap_back(F)
#define iof_filter_buffer_free(data) iof_heap_back(data)

// &((void *)pstate

iof * iof_filter_reader_new (iof_handler handler, size_t statesize, void **pstate);
#define iof_filter_reader(handler, statesize, pstate) iof_filter_reader_new(handler, statesize, (void **)(pstate))
iof * iof_filter_reader_with_buffer_new (iof_handler handler, size_t statesize, void **pstate, void *buffer, size_t buffersize);
#define iof_filter_reader_with_buffer(handler, statesize, pstate, buffer, buffersize) iof_filter_reader_with_buffer_new(handler, statesize, (void **)(pstate), buffer, buffersize)
iof * iof_filter_writer_new (iof_handler handler, size_t statesize, void **pstate);
#define iof_filter_writer(handler, statesize, pstate) iof_filter_writer_new(handler, statesize, (void **)(pstate))
iof * iof_filter_writer_with_buffer_new (iof_handler handler, size_t statesize, void **pstate, void *buffer, size_t buffersize);
#define iof_filter_writer_with_buffer(handler, statesize, pstate, buffer, buffersize) iof_filter_writer_with_buffer_new(handler, statesize, (void **)(pstate), buffer, buffersize)

#define iof_filter_state(statetype, F) (statetype)((F) + 1)

void iof_free (iof *F);
void iof_discard (iof *F);

size_t iof_resize_buffer_to (iof *O, size_t space);
#define iof_resize_buffer(O) iof_resize_buffer_to(O, (O)->space << 1)

size_t iof_decoder_retval (iof *I, const char *type, iof_status status);
size_t iof_encoder_retval (iof *O, const char *type, iof_status status);

/* filters */

iof * iof_filter_file_handle_reader (FILE *file);
iof * iof_filter_file_handle_writer (FILE *file);

iof * iof_filter_iofile_reader (iof_file *iofile, size_t offset);
iof * iof_filter_iofile_writer (iof_file *iofile, size_t offset);

iof * iof_filter_file_reader (const char *filename);
iof * iof_filter_file_writer (const char *filename);

iof * iof_filter_string_reader (const void *s, size_t length);
iof * iof_filter_string_writer (const void *s, size_t length);

iof * iof_filter_buffer_writer (size_t size);

iof * iof_filter_stream_reader (FILE *file, size_t offset, size_t length);
iof * iof_filter_stream_coreader (iof_file *iofile, size_t offset, size_t length);

iof * iof_filter_stream_writer (FILE *file);
iof * iof_filter_stream_cowriter (iof_file *iofile, size_t offset);

FILE * iof_filter_file_reader_source (iof *I, size_t *poffset, size_t *plength);
iof_file * iof_filter_file_coreader_source (iof *I, size_t *poffset, size_t *plength);
iof * iof_filter_reader_replacement (iof *P, iof_handler handler, size_t statesize, void **pstate);
#define iof_filter_reader_replace(P, handler, statesize, pstate) iof_filter_reader_replacement(P, handler, statesize, (void **)(pstate))

#endif