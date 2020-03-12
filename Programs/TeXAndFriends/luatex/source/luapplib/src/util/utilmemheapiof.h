
#ifndef UTIL_MEM_HEAP_IOF_H
#define UTIL_MEM_HEAP_IOF_H

#include "utilmemheap.h"
#include "utiliof.h"

UTILAPI size_t heap8_writer (iof *O, iof_mode mode);
UTILAPI size_t heap16_writer (iof *O, iof_mode mode);
UTILAPI size_t heap32_writer (iof *O, iof_mode mode);
UTILAPI size_t heap64_writer (iof *O, iof_mode mode);

#define HEAP8_BUFFER_INIT(heap) IOF_WRITER_INIT(heap8_writer, (void *)(heap), NULL, 0, 0)
#define HEAP16_BUFFER_INIT(heap) IOF_WRITER_INIT(heap16_writer, (void *)(heap), NULL, 0, 0)
#define HEAP32_BUFFER_INIT(heap) IOF_WRITER_INIT(heap32_writer, (void *)(heap), NULL, 0, 0)
#define HEAP64_BUFFER_INIT(heap) IOF_WRITER_INIT(heap64_writer, (void *)(heap), NULL, 0, 0)

#define heap8_buffer_init(heap, O) iof_writer(O, (void *)(heap), heap8_writer, NULL, 0)
#define heap16_buffer_init(heap, O) iof_writer(O, (void *)(heap), heap16_writer, NULL, 0)
#define heap32_buffer_init(heap, O) iof_writer(O, (void *)(heap), heap32_writer, NULL, 0)
#define heap64_buffer_init(heap, O) iof_writer(O, (void *)(heap), heap64_writer, NULL, 0)

UTILAPI iof * _heap8_buffer_some (heap8 *heap, iof *O, size_t atleast);
UTILAPI iof * _heap16_buffer_some (heap16 *heap, iof *O, size_t atleast);
UTILAPI iof * _heap32_buffer_some (heap32 *heap, iof *O, size_t atleast);
UTILAPI iof * _heap64_buffer_some (heap64 *heap, iof *O, size_t atleast);

#define heap8_buffer_some(heap, O, atleast) (heap8_ensure_head(heap), _heap8_buffer_some(heap, O, atleast))
#define heap16_buffer_some(heap, O, atleast) (heap16_ensure_head(heap), _heap16_buffer_some(heap, O, atleast))
#define heap32_buffer_some(heap, O, atleast) (heap32_ensure_head(heap), _heap32_buffer_some(heap, O, atleast))
#define heap64_buffer_some(heap, O, atleast) (heap64_ensure_head(heap), _heap64_buffer_some(heap, O, atleast))

#define heap8_buffer_done(heap, O) heap8_done(heap, (O)->buf, (size_t)iof_size(O))
#define heap16_buffer_done(heap, O) heap16_done(heap, (O)->buf, (size_t)iof_size(O))
#define heap32_buffer_done(heap, O) heap32_done(heap, (O)->buf, (size_t)iof_size(O))
#define heap64_buffer_done(heap, O) heap64_done(heap, (O)->buf, (size_t)iof_size(O))

#define heap8_buffer_giveup(heap, O) heap8_giveup(heap, (O)->buf)
#define heap16_buffer_giveup(heap, O) heap16_giveup(heap, (O)->buf)
#define heap32_buffer_giveup(heap, O) heap32_giveup(heap, (O)->buf)
#define heap64_buffer_giveup(heap, O) heap64_giveup(heap, (O)->buf)

#endif