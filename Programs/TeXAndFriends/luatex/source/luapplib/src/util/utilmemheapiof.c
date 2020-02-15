
#include "utilmemheapiof.h"

// this is identical to stock iof suite, keep in sync

static size_t heap8_writer (iof *O, iof_mode mode)
{
  heap8 *heap;
  size_t written;
  heap = (heap8 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      written = (size_t)iof_size(O);
      heap8_done(heap, O->buf, written);
      O->buf = _heap8_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->space = written << 1;
      O->buf = heap8_more(heap, O->buf, written, O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return written; // eq (space - written)
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

static size_t heap16_writer (iof *O, iof_mode mode)
{
  heap16 *heap;
  size_t written;
  heap = (heap16 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      written = (size_t)iof_size(O);
      heap16_done(heap, O->buf, written);
      O->buf = _heap16_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->space = written << 1;
      O->buf = heap16_more(heap, O->buf, written, O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return written; // eq (space - written)
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

static size_t heap32_writer (iof *O, iof_mode mode)
{
  heap32 *heap;
  size_t written;
  heap = (heap32 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      written = (size_t)iof_size(O);
      heap32_done(heap, O->buf, written);
      O->buf = _heap32_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->space = written << 1;
      O->buf = heap32_more(heap, O->buf, written, O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return written; // eq (space - written)
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

static size_t heap64_writer (iof *O, iof_mode mode)
{
  heap64 *heap;
  size_t written;
  heap = (heap64 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      written = (size_t)iof_size(O);
      heap64_done(heap, O->buf, written);
      O->buf = _heap64_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->space = written << 1;
      O->buf = heap64_more(heap, O->buf, written, O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return written; // eq (space - written)
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

/* buffer init (made once) */

iof * heap8_buffer_init (heap8 *heap, iof *O)
{
  void *data;
  size_t space;
  data = heap8_some(heap, 0, &space);
  if (iof_writer(O, (void *)heap, heap8_writer, data, space) == NULL) // sanity
    return NULL;
  return O;
}

iof * heap16_buffer_init (heap16 *heap, iof *O)
{
  void *data;
  size_t space;
  data = heap16_some(heap, 0, &space);
  if (iof_writer(O, (void *)heap, heap16_writer, data, space) == NULL)
    return NULL;
  return O;
}

iof * heap32_buffer_init (heap32 *heap, iof *O)
{
  void *data;
  size_t space;
  data = heap32_some(heap, 0, &space);
  if (iof_writer(O, (void *)heap, heap32_writer, data, space) == NULL)
    return NULL;
  return O;
}

iof * heap64_buffer_init (heap64 *heap, iof *O)
{
  void *data;
  size_t space;
  data = heap64_some(heap, 0, &space);
  if (iof_writer(O, (void *)heap, heap64_writer, data, space) == NULL)
    return NULL;
  return O;
}

/* buffer for some */

iof * heap8_buffer_some (heap8 *heap, iof *O, size_t atleast)
{
  O->buf = _heap8_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * heap16_buffer_some (heap16 *heap, iof *O, size_t atleast)
{
  O->buf = _heap16_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * heap32_buffer_some (heap32 *heap, iof *O, size_t atleast)
{
  O->buf = _heap32_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * heap64_buffer_some (heap64 *heap, iof *O, size_t atleast)
{
  O->buf = _heap64_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}
