
#include "utilmemheapiof.h"

// this is identical to stock iof suite, keep in sync

size_t heap8_writer (iof *O, iof_mode mode)
{
  heap8 *heap;
  size_t written;
  heap = (heap8 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      heap8_buffer_done(heap, O);
      O->buf = _heap8_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->buf = heap8_more(heap, O->buf, written, written << 1, &O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return O->space - written;
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

size_t heap16_writer (iof *O, iof_mode mode)
{
  heap16 *heap;
  size_t written;
  heap = (heap16 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      heap16_buffer_done(heap, O);
      O->buf = _heap16_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->buf = heap16_more(heap, O->buf, written, written << 1, &O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return O->space - written;
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

size_t heap32_writer (iof *O, iof_mode mode)
{
  heap32 *heap;
  size_t written;
  heap = (heap32 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      heap32_buffer_done(heap, O);
      O->buf = _heap32_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->buf = heap32_more(heap, O->buf, written, written << 1, &O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return O->space - written;
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

size_t heap64_writer (iof *O, iof_mode mode)
{
  heap64 *heap;
  size_t written;
  heap = (heap64 *)O->link;
  switch (mode)
  {
    case IOFFLUSH:
      heap64_buffer_done(heap, O);
      O->buf = _heap64_some(heap, 0, &O->space);
      O->pos = O->buf;
      O->end = O->buf + O->space;
      break;
    case IOFWRITE:
      written = (size_t)iof_size(O);
      O->buf = heap64_more(heap, O->buf, written, written << 1, &O->space);
      O->pos = O->buf + written;
      O->end = O->buf + O->space;
      return O->space - written;
    case IOFCLOSE:
    default:
      break;
  }
  return 0;
}

/* buffer for some */

iof * _heap8_buffer_some (heap8 *heap, iof *O, size_t atleast)
{
  O->buf = _heap8_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * _heap16_buffer_some (heap16 *heap, iof *O, size_t atleast)
{
  O->buf = _heap16_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * _heap32_buffer_some (heap32 *heap, iof *O, size_t atleast)
{
  O->buf = _heap32_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}

iof * _heap64_buffer_some (heap64 *heap, iof *O, size_t atleast)
{
  O->buf = _heap64_some(heap, atleast, &O->space);
  O->pos = O->buf;
  O->end = O->buf + O->space;
  return O;
}
