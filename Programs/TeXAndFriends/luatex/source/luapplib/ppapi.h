
#ifndef PP_API_H
#define PP_API_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "ppconf.h"

#define pplib_version "v1.03"
#define pplib_author "p.jackowski@gust.org.pl"

/* types */

typedef int64_t ppint;
typedef size_t ppuint; // machine word

typedef double ppnum;
typedef char * ppname;
typedef char * ppstring;

typedef struct {
  size_t size;
  int flags;
} _ppname;

typedef struct {
  size_t size;
  int flags;
} _ppstring;

typedef struct ppobj ppobj;
typedef struct ppref ppref;


#if ARM_COMPLIANT
typedef struct {
  ppobj *data;
  size_t size;
  ppnum PPARRAY_ALIGNMENT;
} pparray;
#else
typedef struct {
  ppobj *data;
  size_t size;
} pparray;
#endif


#if ARM_COMPLIANT
typedef struct {
  ppobj *data;
	ppname *keys;
  size_t size;
  ppnum PPDICT_ALIGNMENT;
} ppdict;

#else
typedef struct {
  ppobj *data;
	ppname *keys;
  size_t size;
} ppdict;
#endif

typedef enum {
  PPSTREAM_BASE16 = 0,
  PPSTREAM_BASE85,
  PPSTREAM_RUNLENGTH,
  PPSTREAM_FLATE,
  PPSTREAM_LZW,
  PPSTREAM_CCITT,
  PPSTREAM_DCT,
  PPSTREAM_JBIG2,
  PPSTREAM_JPX,
  PPSTREAM_CRYPT
} ppstreamtp;

typedef struct {
  ppstreamtp *filters;
  ppdict **params;
  size_t count;
} ppstream_filter;

typedef struct {
  ppdict *dict;
  void *input, *I;
  size_t offset;
  size_t length;
  ppstream_filter filter;
  ppobj *filespec;
  ppstring cryptkey;
  int flags;
} ppstream;

PPDEF extern const char * ppstream_filter_name[];
PPAPI int ppstream_filter_type (ppname filtername, ppstreamtp *filtertype);
PPAPI void ppstream_filter_info (ppstream *stream, ppstream_filter *info, int decode);

#define PPSTREAM_FILTER (1<<0)
#define PPSTREAM_IMAGE (1<<1)
#define PPSTREAM_ENCRYPTED_AES (1<<2)
#define PPSTREAM_ENCRYPTED_RC4 (1<<3)
#define PPSTREAM_ENCRYPTED (PPSTREAM_ENCRYPTED_AES|PPSTREAM_ENCRYPTED_RC4)
#define PPSTREAM_ENCRYPTED_OWN (1<<4)
#define PPSTREAM_NOT_SUPPORTED (1<<6)

#define ppstream_compressed(stream) ((stream)->flags & (PPSTREAM_FILTER|PPSTREAM_IMAGE))
#define ppstream_filtered(stream) ((stream)->flags & PPSTREAM_FILTER)
#define ppstream_image(stream) ((stream)->flags & PPSTREAM_IMAGE)
#define ppstream_encrypted(stream) ((stream)->flags & PPSTREAM_ENCRYPTED)

typedef enum {
  PPNONE = 0,
  PPNULL,
  PPBOOL,
  PPINT,
  PPNUM,
  PPNAME,
  PPSTRING,
  PPARRAY,
  PPDICT,
  PPSTREAM,
  PPREF
} ppobjtp;

PPDEF extern const char * ppobj_kind[];

struct ppobj {
  ppobjtp type;
  union {
    ppint integer;
    ppnum number;
    ppname name;
    ppstring string;
    pparray *array;
    ppdict *dict;
    ppstream *stream;
    ppref *ref;
    void *any;
  };
};

typedef struct ppxref ppxref;

struct ppref {
  ppobj object;
  ppuint number, version;
  size_t offset;
  size_t length;
  ppxref *xref;
};

typedef struct ppdoc ppdoc;

/* object */

#define ppobj_get_null(o) ((o)->type == PPNULL ? 1 : 0)
#define ppobj_get_bool(o, v) ((o)->type == PPBOOL ? ((v = ((o)->integer != 0)), 1) : 0)
#define ppobj_get_int(o, v) ((o)->type == PPINT ? ((v = (o)->integer), 1) : 0)
#define ppobj_get_uint(o, v) ((o)->type == PPINT && (o)->integer >= 0 ? ((v = (ppuint)((o)->integer)), 1) : 0)
#define ppobj_get_num(o, v) ((o)->type == PPNUM ? ((v = (o)->number), 1) : (((o)->type == PPINT ? ((v = (ppnum)((o)->integer)), 1) : 0)))
#define ppobj_get_name(o) ((o)->type == PPNAME ? (o)->name : NULL)
#define ppobj_get_string(o) ((o)->type == PPSTRING ? (o)->string : NULL)
#define ppobj_get_array(o) ((o)->type == PPARRAY ? (o)->array : NULL)
#define ppobj_get_dict(o) ((o)->type == PPDICT ? (o)->dict : NULL)
#define ppobj_get_stream(o) ((o)->type == PPSTREAM ? (o)->stream : NULL)
#define ppobj_get_ref(o) ((o)->type == PPREF ? (o)->ref : NULL)

#define ppobj_rget_obj(o) ((o)->type == PPREF ? ppref_obj((o)->ref) : o)
#define ppobj_rget_null(o) ((o)->type == PPNULL ? 1 : ((o)->type == PPREF ? ppobj_get_null(ppref_obj((o)->ref)) : 0))
#define ppobj_rget_bool(o, v) ((o)->type == PPBOOL ? ((v = ((o)->integer != 0)), 1) : ((o)->type == PPREF ? ppobj_get_bool(ppref_obj((o)->ref), v) : 0))
#define ppobj_rget_int(o, v) ((o)->type == PPINT ? ((v = (o)->integer), 1) : ((o)->type == PPREF ? ppobj_get_int(ppref_obj((o)->ref), v) : 0))
#define ppobj_rget_uint(o, v) ((o)->type == PPINT && (o)->integer >= 0 ? ((v = (ppuint)((o)->integer)), 1) : ((o)->type == PPREF ? ppobj_get_uint(ppref_obj((o)->ref), v) : 0))
#define ppobj_rget_num(o, v) ((o)->type == PPNUM ? ((v = (o)->number), 1) : (((o)->type == PPINT ? ((v = (ppnum)((o)->integer)), 1) : ((o)->type == PPREF ? ppobj_get_num(ppref_obj((o)->ref), v) : 0))))
#define ppobj_rget_name(o) ((o)->type == PPNAME ? (o)->name : ((o)->type == PPREF ? ppobj_get_name(ppref_obj((o)->ref)) : NULL))
#define ppobj_rget_string(o) ((o)->type == PPSTRING ? (o)->string : ((o)->type == PPREF ? ppobj_get_string(ppref_obj((o)->ref)) : NULL))
#define ppobj_rget_array(o) ((o)->type == PPARRAY ? (o)->array : ((o)->type == PPREF ? ppobj_get_array(ppref_obj((o)->ref)) : NULL))
#define ppobj_rget_dict(o) ((o)->type == PPDICT ? (o)->dict : ((o)->type == PPREF ? ppobj_get_dict(ppref_obj((o)->ref)) : NULL))
#define ppobj_rget_stream(o) ((o)->type == PPSTREAM ? (o)->stream : ((o)->type == PPREF ? ppobj_get_stream(ppref_obj((o)->ref)) : NULL))
#define ppobj_rget_ref(o) ((o)->type == PPREF ? (o)->ref : ((o)->type == PPREF ? ppobj_get_ref(ppref_obj((o)->ref)) : NULL))

#define ppobj_get_bool_value(o) ((o)->type == PPBOOL ? ((o)->integer != 0) : 0)
#define ppobj_get_int_value(o) ((o)->type == PPINT ? (o)->integer : 0)
#define ppobj_get_num_value(o) ((o)->type == PPNUM  ? (o)->number : ((o)->type == PPINT  ? (ppnum)((o)->integer) : 0.0))

/* name */

#define ppname_is(name, s) (memcmp(name, s, sizeof("" s) - 1) == 0)
#define ppname_eq(name, n) (memcmp(name, s, ppname_size(name)) == 0)

#if ARM_COMPLIANT
#define _ppname_ghost(name) (((const _ppname *)((void *)name)) - 1)
#else
#define _ppname_ghost(name) (((const _ppname *)(name)) - 1)
#endif

#define ppname_size(name) (_ppname_ghost(name)->size)
#define ppname_exec(name) (_ppname_ghost(name)->flags & PPNAME_EXEC)

#define PPNAME_ENCODED (1 << 0)
#define PPNAME_DECODED (1 << 1)
#define PPNAME_EXEC (1 << 1)

PPAPI ppname ppname_decoded (ppname name);
PPAPI ppname ppname_encoded (ppname name);

/* string */

#if ARM_COMPLIANT
#define _ppstring_ghost(string) (((const _ppstring *)((void *)string)) - 1)
#else
#define _ppstring_ghost(string) (((const _ppstring *)(string)) - 1)
#endif

#define ppstring_size(string) (_ppstring_ghost(string)->size)

#define PPSTRING_ENCODED (1 << 0)
#define PPSTRING_DECODED (1 << 1)
//#define PPSTRING_EXEC (1 << 2) // postscript only
#define PPSTRING_PLAIN 0
#define PPSTRING_BASE16 (1 << 3)
#define PPSTRING_BASE85 (1 << 4)
#define PPSTRING_UTF16BE (1 << 5)
#define PPSTRING_UTF16LE (1 << 6)

#define ppstring_type(string) (_ppstring_ghost(string)->flags & (PPSTRING_BASE16|PPSTRING_BASE85))
#define ppstring_hex(string) (_ppstring_ghost(string)->flags & PPSTRING_BASE16)
#define ppstring_utf(string) (_ppstring_ghost(string)->flags & (PPSTRING_UTF16BE|PPSTRING_UTF16LE))

PPAPI ppstring ppstring_decoded (ppstring string);
PPAPI ppstring ppstring_encoded (ppstring string);

/* array */

#define pparray_size(array) ((array)->size)
#define pparray_at(array, index) ((array)->data + index)

#define pparray_first(array, index, obj) ((index) = 0, (obj) = pparray_at(array,  0))
#define pparray_next(index, obj) (++(index), ++(obj))

#define pparray_get(array, index) (index < (array)->size ? pparray_at(array, index) : NULL)

PPAPI ppobj * pparray_get_obj (pparray *array, size_t index);
PPAPI int pparray_get_bool (pparray *array, size_t index, int *v);
PPAPI int pparray_get_int (pparray *array, size_t index, ppint *v);
PPAPI int pparray_get_uint (pparray *array, size_t index, ppuint *v);
PPAPI int pparray_get_num (pparray *array, size_t index, ppnum *v);
PPAPI ppname pparray_get_name (pparray *array, size_t index);
PPAPI ppstring pparray_get_string (pparray *array, size_t index);
PPAPI pparray * pparray_get_array (pparray *array, size_t index);
PPAPI ppdict * pparray_get_dict (pparray *array, size_t index);
//PPAPI ppstream * pparray_get_stream (pparray *array, size_t index);
PPAPI ppref * pparray_get_ref (pparray *array, size_t index);

PPAPI ppobj * pparray_rget_obj (pparray *array, size_t index);
PPAPI int pparray_rget_bool (pparray *array, size_t index, int *v);
PPAPI int pparray_rget_int (pparray *array, size_t index, ppint *v);
PPAPI int pparray_rget_uint (pparray *array, size_t index, ppuint *v);
PPAPI int pparray_rget_num (pparray *array, size_t index, ppnum *v);
PPAPI ppname pparray_rget_name (pparray *array, size_t index);
PPAPI ppstring pparray_rget_string (pparray *array, size_t index);
PPAPI pparray * pparray_rget_array (pparray *array, size_t index);
PPAPI ppdict * pparray_rget_dict (pparray *array, size_t index);
PPAPI ppstream * pparray_rget_stream (pparray *array, size_t index);
PPAPI ppref * pparray_rget_ref (pparray *array, size_t index);

/* dict */

#define ppdict_size(dict) ((dict)->size)
#define ppdict_at(dict, index) ((dict)->data + index)
#define ppdict_key(dict, index) ((dict)->keys[index])

PPAPI ppobj * ppdict_get_obj (ppdict *dict, const char *name);
PPAPI int ppdict_get_bool (ppdict *dict, const char *name, int *v);
PPAPI int ppdict_get_int (ppdict *dict, const char *name, ppint *v);
PPAPI int ppdict_get_uint (ppdict *dict, const char *name, ppuint *v);
PPAPI int ppdict_get_num (ppdict *dict, const char *name, ppnum *v);
PPAPI ppname ppdict_get_name (ppdict *dict, const char *name);
PPAPI ppstring ppdict_get_string (ppdict *dict, const char *name);
PPAPI pparray * ppdict_get_array (ppdict *dict, const char *name);
PPAPI ppdict * ppdict_get_dict (ppdict *dict, const char *name);
//PPAPI ppstream * ppdict_get_stream (ppdict *dict, const char *name);
PPAPI ppref * ppdict_get_ref (ppdict *dict, const char *name);

PPAPI ppobj * ppdict_rget_obj (ppdict *dict, const char *name);
PPAPI int ppdict_rget_bool (ppdict *dict, const char *name, int *v);
PPAPI int ppdict_rget_int (ppdict *dict, const char *name, ppint *v);
PPAPI int ppdict_rget_uint (ppdict *dict, const char *name, ppuint *v);
PPAPI int ppdict_rget_num (ppdict *dict, const char *name, ppnum *v);
PPAPI ppname ppdict_rget_name (ppdict *dict, const char *name);
PPAPI ppstring ppdict_rget_string (ppdict *dict, const char *name);
PPAPI pparray * ppdict_rget_array (ppdict *dict, const char *name);
PPAPI ppdict * ppdict_rget_dict (ppdict *dict, const char *name);
PPAPI ppstream * ppdict_rget_stream (ppdict *dict, const char *name);
PPAPI ppref * ppdict_rget_ref (ppdict *dict, const char *name);

#define ppdict_first(dict, pkey, obj) (pkey = (dict)->keys, obj = (dict)->data)
#define ppdict_next(pkey, obj) (++(pkey), ++(obj))

/* stream */

#define ppstream_dict(stream) ((stream)->dict)

PPAPI uint8_t * ppstream_first (ppstream *stream, size_t *size, int decode);
PPAPI uint8_t * ppstream_next (ppstream *stream, size_t *size);
PPAPI uint8_t * ppstream_all (ppstream *stream, size_t *size, int decode);
PPAPI void ppstream_done (ppstream *stream);

PPAPI void ppstream_init_buffers (void);
PPAPI void ppstream_free_buffers (void);

/* ref */

#define ppref_obj(ref) (&(ref)->object)

/* xref */

PPAPI ppxref * ppdoc_xref (ppdoc *pdf);
PPAPI ppxref * ppxref_prev (ppxref *xref);
PPAPI ppdict * ppxref_trailer (ppxref *xref);
PPAPI ppdict * ppxref_catalog (ppxref *xref);
PPAPI ppdict * ppxref_info (ppxref *xref);
PPAPI ppref * ppxref_pages (ppxref *xref);
PPAPI ppref * ppxref_find (ppxref *xref, ppuint refnumber);

/* doc */

PPAPI ppdoc * ppdoc_load (const char *filename);
PPAPI ppdoc * ppdoc_mem (const void *data, size_t size);
PPAPI void ppdoc_free (ppdoc *pdf);

#define ppdoc_trailer(pdf) ppxref_trailer(ppdoc_xref(pdf))
#define ppdoc_catalog(pdf) ppxref_catalog(ppdoc_xref(pdf))
#define ppdoc_info(pdf) ppxref_info(ppdoc_xref(pdf))
#define ppdoc_pages(pdf) ppxref_pages(ppdoc_xref(pdf))

PPAPI ppuint ppdoc_page_count (ppdoc *pdf);
PPAPI ppref * ppdoc_page (ppdoc *pdf, ppuint index);
PPAPI ppref *  ppdoc_first_page (ppdoc *pdf);
PPAPI ppref * ppdoc_next_page (ppdoc *pdf);

PPAPI ppstream * ppcontents_first (ppdict *dict);
PPAPI ppstream * ppcontents_next (ppdict *dict, ppstream *stream);

/* crypt */

typedef enum {
  PPCRYPT_NONE = 0,
  PPCRYPT_DONE = 1,
  PPCRYPT_FAIL = -1,
  PPCRYPT_PASS = -2
} ppcrypt_status;

PPAPI ppcrypt_status ppdoc_crypt_status (ppdoc *pdf);
PPAPI ppcrypt_status ppdoc_crypt_pass (ppdoc *pdf, const void *userpass, size_t userpasslength, const void *ownerpass, size_t ownerpasslength);

/* permission flags, effect in Acrobat File -> Properties -> Security tab */

PPAPI ppint ppdoc_permissions (ppdoc *pdf);

#define PPDOC_ALLOW_PRINT (1<<2)        // printing
#define PPDOC_ALLOW_MODIFY (1<<3)       // filling form fields, signing, creating template pages
#define PPDOC_ALLOW_COPY (1<<4)         // copying, copying for accessibility
#define PPDOC_ALLOW_ANNOTS (1<<5)       // filling form fields, copying, signing
#define PPDOC_ALLOW_EXTRACT (1<<9)      // contents copying for accessibility
#define PPDOC_ALLOW_ASSEMBLY (1<<10)    // (no effect)
#define PPDOC_ALLOW_PRINT_HIRES (1<<11) // (no effect)

/* context */

typedef struct ppcontext ppcontext;

PPAPI ppcontext * ppcontext_new (void);
PPAPI void ppcontext_done (ppcontext *context);
PPAPI void ppcontext_free (ppcontext *context);

/* contents parser */

PPAPI ppobj * ppcontents_first_op (ppcontext *context, ppstream *stream, size_t *psize, ppname *pname);
PPAPI ppobj * ppcontents_next_op (ppcontext *context, ppstream *stream, size_t *psize, ppname *pname);
PPAPI ppobj * ppcontents_parse (ppcontext *context, ppstream *stream, size_t *psize);

/* boxes and transforms */

typedef struct {
  ppnum lx, ly, rx, ry;
} pprect;

PPAPI pprect * pparray_to_rect (pparray *array, pprect *rect);
PPAPI pprect * ppdict_get_rect (ppdict *dict, const char *name, pprect *rect);
PPAPI pprect * ppdict_get_box (ppdict *dict, const char *name, pprect *rect);

typedef struct {
  ppnum xx, xy, yx, yy, x, y;
} ppmatrix;

PPAPI ppmatrix * pparray_to_matrix (pparray *array, ppmatrix *matrix);
PPAPI ppmatrix * ppdict_get_matrix (ppdict *dict, const char *name, ppmatrix *matrix);

/* logger */

typedef void (*pplogger_callback) (const char *message, void *alien);
PPAPI void pplog_callback (pplogger_callback logger, void *alien);
PPAPI int pplog_prefix (const char *prefix);

/* version */

PPAPI const char * ppdoc_version_string (ppdoc *pdf);
PPAPI int ppdoc_version_number (ppdoc *pdf, int *minor);

/* doc info */

PPAPI size_t ppdoc_file_size (ppdoc *pdf);
PPAPI ppuint ppdoc_objects (ppdoc *pdf);
PPAPI size_t ppdoc_memory (ppdoc *pdf, size_t *waste);

#endif
