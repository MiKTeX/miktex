/*
Allocators
==========

Using allocators defined here makes sense if there is a need to take a space for rather large amount of rather small objects.
The basic idea is to take memory in reasonably large blocks and to distribute small chunks from those blocks. Once chunks are
no longer needed, one can free them all at once, or free taken chunks individually.

We define 3 types of allocators:

1. HEAP - the simplest one, provides variable length memory chunks from larger blocks and frees them all on explicit
   request. There is no way to free individual objects, only the entire heap. The heap only grows, until freed.

2. STOCK - provides variable length memory chunks from larger blocks, but allows to free individual objects as well as the
   entire stock. The stock grows and shrinks, by leaps and bounds, depending on parameters given during initialization.

3. POOL - provides fixed size memory chunks from larger blocks. It allows to free individual chunks as well as the entire pool.
   In opposite to a stock, a pool also reuses a space reclaimed by freeing individual objects; before allocating a new block it
   firsts recycles freed chunks, if any.

In general, memory chunks provided by allocators are never reallocated. Allocators do nothing with provided chunks until freed.

Allocators are represented as small structures (several pointers and integers). We assume that such structures are either static
variables, or members of larger structures. We don't bother to allocate a memory for them. Usage scheme is pretty similar for
all:

  init() - just inititalize, don't allocate anything yet
  take() - take chunks
  take()
  take()
  ...
  free() - free the all at once

For stocks and pools there is a possibility to give back individual chunks:

  init() - like above
  take() - take chunks
  take()
  take()
  back() - give chunks back when no longer needed
  take()
  back()
  ...
  free() - needed only if not all chunks have been given back

All calls take a shell structure pointer as an argument. take() returns a void pointer, aligned according to used variant
(8, 16, 32, 64). back() takes a void pointer as the second argument. It must be the pointer previously returned by take().

back() can be called in any order and can obviously be plotted with take(). By default, after back()-ing all taken chunks, the
stock returns to its initial state (zero memory used). A special KEEP flag can be used during initialization to prevent
freeing the last (sole) allocated block. If KEEP option is used, the allocator always keeps a single segment for further
allocations. This is necessary only when there is a risk that just several take() calls will be immediatelly followed by the
same number of back() calls. KEEP flag prevents allocating and freeing rather large blocks just to serve several chunks. And
this is actually important only if there are no other blocks taken, that is, if there is only one, nearly empty block in use.
In other cases KEEP flag doesn't matter, but allocators takes care to always have a block for fast allocs.

There is also clear() operation that frees all but the recent block. One can use it to free all chunks taken so far, but to
make the allocator ready for further allocs. If either KEEP flag is used or clear() is called, soner or later the user have to
call free() explicitly, to remove all the remaining memory kept by the allocator. There is no KEEP flag for heaps, as heaps
don't allow to free individual chunks. And so, the heap never needs to make a decision if the last sole block should be removed
or not. The user makes the decision by calling clear() vs free() respectively.

Pop
===

A very last chunk taken can be quickly given back with

  pop(heap, taken, size) // for heap or stock
  pop(pool, taken)       // for pool

taken must be the chunk returned by the very last take(), size must be the size requested. If the chunk has been taken from
the head block (more about blocks below), the block pointer returns to its previous position, as it was before the last take().
If the chunk has been taken from the sole block beneatch the head, the entire sole block (containing just that single chunk)
is freed. The pop() operation is different than back(); the popped chunk doesn't cause freeing  the head block when its refcount
gets zero. So pop() operation breaks the concept of stock that frees all the memory once all taken chunks are given back.
on the other hand, if for some reason the very last taken chunk is to be ignored, pop() is better, as it doesn't cause blocks
scattering. The popped chunk pointer will probably be returned by the very next call to take(). In case of heap, pop() is
the only way to discard the chunk, as there is no back() operation.

Buffer interface
================

When heap or stock is used by parsers, the caller oftenly doesn't know how many space will be needed for a data (this doesn't
apply to pools, which returns constant size memory chunks). Here is an interface for the allocator-as-bufer case (same for
heap and stock):

  some(heap, atleast, &space);
  ...
  atleast <<= 1;
  more(heap, taken, written, atleast, &space);
  ...
  done(heap, taken, written);

some() operation provides a data pointer to at least a given bytes. The actual space provided for writing is set to the third
argument. The caller may write space-bytes. If more space is needed, more() operation takes care to provide a chunk for a given
amount of bytes and rewrites already written amount of bytes from a previous chunk to a new location. Same as with() some, the
requests for atleast bytes, and the actual provided chunk size is given as space (space >= atleast).

The function takes the pointer to the chunk previously taken; the one returned by some() or more(). This argument must not be NULL. 
If you don't want to copy a data, set written argument to zero. No matter if more() operation was used zero, one or multiple times, 
all the cycle must end with done(). Calls triple - some(), more() and done() - must not be interrupted by any other api calls. 
In particular, using take() or back() smells like a segfault. However, if there is a need discard the buffer being written 
(eg. input data error), instead of done() one may use

  giveup(heap, taken)

If done() has already been called, pop() is the only option to discard the chunk

  pop(heap, taken, written)

some() operation usually doesn't change the state of the heap, unless the heap head block is NULL, or atleast parameter is too
large to fit the remaining block. more() usually changes the state, either by allocating a new head block, or by allocating
a sole block just beneath the head (blocks and blocks tiling mechanism are described below). If a sole block has been taken for
some large chunk subsequent calls to more() reallocate this sole block in place. It is assumed, that the size you request in subsequent
calls generally grows. It is ok to request space-bytes, then call done() with written value less then requested. But the drawback
is that if the chunk has already been allocated from a sole chunk, the space requested but not used is a waste.

iof interface
=============

iof is an independent interface for buffers written/read byte-by-byte. When used together with allocators, it provides 
a convenient way to write byte data to the heap or stock, without a need for intermediate buffers. The buffer is setup with

  iof output, *O
  O = buffer_init(heap, &output); // doesn't allocate anything

or 

  output = BUFFER_INIT(heap); // doesn't allocate anything
  O = &output;

iof keeps pointers to the beginning of the buffer, end of buffer, and current position. Once the position reaches the end, 
the iof internal handler updates the buffer providing more space to write. When used in conjunction with heap or stock,
the space to write is the space provided by the heap or stock. To start the buffer session:

  O = buffer_some(heap, O, atleast) // ensure iof *O to have atleast bytes to be written

Once you are done with writing some chunk

  buffer_done(heap, O)

instead of buffer_done(), one may also use

  iof_flush(O) // calls buffer_done() and buffer_some() again
  
which updates the underlying heap or stock, and makes the iof ready for a new chunk. iof itself does not allocate a memory, 
so it doesn't need finalizer. iof_close(output) does nothing. To drop the buffer use:

  buffer_giveup(heap, O) // restore the from before buffer_some()

More often then not, we need to specify a minimal space for buffer each time, eg. for memcpy() or so. The actual space left
can be checked with iof_left(O). The entire space of recent chunk is O->space (eq. O->end - O->buf).

Identical interface for heap and stock.

Blocks
======

Each alloctor structure keeps a pointer to a head block, initially NULL. Most of new chunks are taken from the head. Once the
space left in the head block is to small to provide a chunk of requested size, a new head is created and the previous one is
linked to the head (blocks form a linked list). A stock block is named a ream, a heap block is named a pyre, a pool block is
named pile (we need to distinguish structure names in code but in the description below they are all called blocks). Every
block knows a number of chunks taken from that block (refcont). A stock also keeps a number of freed chunks [actually only
for statistics; in most cases it doesn't need an extra space in struct ream, as thies structure member lays in the place
f padding bytes.]

We change the head block only if the new block is allocated, but we never change the head backward. Once some block became
->prev, it will never became a head again. This ensures that the allocator have the head block that usually has a lot of space
for new allocs. This needs a special care when removing a block that is not a head block. We check if the next block to the one
being removed is the head. If it is, and if its refcount is zero (and no KEEP flag is used) the head is removed as well.

The basis of pools is similar to stocks and heaps, but there are some significant differences. A pool servers memory chunks of
equal size, specified during initialization. This also means that the pool knows the boundaries of individual chunks (stock and
heap doesn't). A pool provides iterators over chunks in use (taken but not given back yet). A pool shell structure keeps
a pointer to a head block and a tail block (both may point a different block, the same block or NULL). This is necessary only
for iterators to let the user follow the chunks from the first or from the last taken. The extra cost of maintaining both
->head and ->tail is neglectable.

Refcounting
===========

Heap refcounting: whenever a new chunk is taken, the block refcount is incremented. It is never decremented, but plays an
important role in block tiling algorithm (below). No KEEP flag is used here. All the blocks are removed on free(), all but
recent are removed on clear().

Stock refcounting: whenever a new chunk in taken from the block, the block refcount is incremented. Whenever the chunk is given
back, the refcount is decremented. When the refcount gets zero, the block is removed and freed. To remove the block from the
list (any block, not necessarily a head block), a stock needs 2-directional list; every block has ->next and ->prev links. The
head block of the stock is freed only if this is the last (sole) block and no KEEP flag was used during initialization.
Otherwise the block is just reset, becoming ready for further allocations - refcount gets zero, data space reset to an initial
state.

Pool refcounting: pretty much like with stocks, except that any chunk given back can be recycled on further take().

Ghosts
======

Every allocated block starts with a private structure for next/prev links, data pointer, refcount. We call it a block ghost.
Except from heap, individual chunks also need a ghost (chunk ghost) so that we are able to know from which block the chunk
comes from once the chunk is given back by the user (heaps don't have back() operation so data chunks have no ghosts). We keep
ghosts possibly small. Chunk ghosts are of size natural for alignment variant (1, 2, 4 or 8 bytes). Block ghosts are somewhat
larger. Statistics show clearly that it is worthy to keep them as small as possible:
- chunk ghosts keep offset to the block ghost, not a pointer to it (we use the pointer only if it makes no difference
  to the chunk size; 64-bit aligned variant on 64-bit machine, 32 and 64 variants on 32-bit machine)
- block ghosts uses a data pointer (not an offset) so that we are able to record any requested chunk size (size_t) and to avoid
  long array indexing on every chunk request

At some point we considered storing a sheel structure pointer in the block ghost, then back() operation wouldn't need an extra
argument. But stats showed that the size of the block ghost is the most significant factor in memory usage efficiency, so eliminating
this extra pointer pays off. Besides, this would make impossible to relocate the shell structure. We don't allocate a memory
for the shell, so we shouldn't make assumptions of shell structure address.

Tiling algorithm
================

Tiling the block size refers to stocks and heaps that serves memory chunks of variable size. Both stock and heap performs best
when the average size of requested chunks is a way smaller that the configured block size. But both also put no limitations on
chunk sizes, so they need to cope with situation, where the requested size is quite large, eg. half of the block size or even
more than the block size. Here is the algorithm used for blocks tiling:

1. When the requested chunk size fills in the recent block, just serve it from that block. This is the best and hopefully the
   most common case.

2. When the requested chunk size is larger that the space left in the recent block, the new block must be allocated. But there
are two ways:

   a) either replace the head block with the new block so that the list of blocks is

      ... <- prev <- head so far <- new head

   b) or insert the block just "below the head", keeping the head intact,

      ... <- prev <- new single head <- head

The first is the regular case. It is used when the space left in the head so far is small (can be neglected), and the requested
size is relatively small (will fit the new block). If the space left in the head block is worthy to bother, or the requested
chunk size is rather large, the new chunk is served from a single block, allocated just for that chunk. The block is of the
size needed for that chunk. The block never becomes the head, no other chunks will be served from it (its refcount is
permanently 1, until freed).

Much depends on what is considered 'small, neglectable block space' and 'rather large chunk size'. The later is easier to
imagine. When the requested size is larger than the block size used for a given allocator, then the size is definitelly
considered large. When it is smaller than the block size, but still large enough to occupy most of the block size (grabbing
quite some space for tiny chunks), it is also considered large. As the block size, what is considered 'large' can be spcified
during initialization. A setup that works fine for me is (large = block_size / 2).

Making a decision what is the left block space we can neglect is quite not obvious. At first approach we used a constant value,
requested from the user during allocator initialization. But it is hard to select a good default. Now we compute this value
from block params, by dividing a complete space occupied so far in the block by the number of chunks served from that block
(the average size of chunks allocated from this block). We assume that the average chunk size (or smaller) is the space we can
neglect. The logic behind is the following: if the space left in the block is larger than the average, it makes sense not to
waste this space and keep it for further allocs. If the space left in the block is less than the average, there is only a little
chance we will get a request for suitable size, so we sacrifice that space and we start allocating from a new block.

Statistics showed a caveat in average == treshold approach. Suppose we have a block that has the average chunk size 16, there
is 18 bytes left in the block (not neglectable), and the user request is 20 bytes. Allocating a single block for 20 bytes is
bad, because the block ghost is 24 bytes (more internal than allocated memory). Allocating many of such blocks gives bad results;
much more allocs than necessary, large waste. To avoid that, we help to neglect the remaining block space by checking if the
space left is smaller than the block ghost size, which is an inevitable cost anyway.

Stats below shows clearly that we should rather focus on "how to avoid producing sole-chunk blocks" instead of "how to feel the
remaining space".

Recycling
=========

Recycling applies only to pools. When a chunk is given back, it is inserted into a list of items for recycling. Every pool
block keeps a head of that list. Once a chunk is given back, it is inserted as recycling head and the previous head is attached
to a new head. Since every chunk is associated with a ghost, we use ghosts to store a link (pointer or offset) to another item
for recycling. Note that the ghost always keeps either a link to the block it belongs to, or a link to another recyclable ghost
of the same block. This is used by iteratos to distinguish the chunk currently in use from the chunk that has already been
given back; if the link points the block, the chunk is in use.

A pool block that has at least one recyclable chunk is called a squot. A pool shell structure keeps 2-directional list of
squots. Once a pool block becomes a squot, it is inserted to that list. Once its all recyclable items has been used, it is
removed from the squots list. In every moment, the pool has an access to a list of all squots, and therefore, to a list of all
recyclable items.

Whenever there is a request for a new chunk, at first it is served from the head block, as this is the easiest and the cheapest way.
Once the recent block has no more place for new items, recycling list is used, starting from the head recyclable chunk of the head squot.
In practise this is always the most recently reclaimed chunk ghost. During further allocs, a pool will first utilize all recyclables
from all squots before allocating a new block.

Stats
=====

Some numbers. The test made on a stock8, block size 255 bytes, 10000 allocations, random chunk sizes from 1 to 32 bytes
(average 16). These are rather tight constraints because of 255 buffer limit. First approach:

  blocks: 903 - this is the actual number of malloc() calls
  singles: 214, 23.70% of all blocks
  waste: 20.16% - total memory that was allocated but not requested by the user
    block ghosts 10.04%, plus single block ghosts 3.12%
    chunk ghosts 4.55%
    neglected block tails 2.45%

After adding a test for left space that helps in 'neglect remainig space or make sole chunk block' decision:

  blocks: 723 - a way better
  singles 0
  waste: 19.04% - slightly better
  block ghosts 10.67%
  chunk ghosts 4.61%
  neglected block tails 3.76%

The actual numbers vary depending on the buffer size, the average elements size and, of course, taken alignment variant. After
some parameters tuning, on various tests we get 5-19% total waste for stocks, 3-14% total waste for heaps. But the basic scheme
of statistics remains similar: we take relatively lots of space for blocks ghost (5-10% of total memory taken), some inevitable
space for chunk ghosts (varies, 4-13% on various tests), and a little waste of neglected block tails (2-4%). Quite
surprisingly, block ghosts are, in sum, oftenly more significant than individual chunk ghosts (for the test above over half of
all the waste!). The number of block ghosts (equals the number of blocks) mostly depends on block size vs chunk size relation.
But seemingly it is worthy to bother about the size of the block ghost and the number of blocks taken - the less the better.
The waste of ghosts of individual objects (stock and pool) is inevitable, and depends only on the number/size of objects taken.
We can't use smaller ghosts, we can't do better. Anyways, the least significant is the waste of neglected block tails.

Pools stats are pretty similar, but more predictable because of known chunks size. A pool block ghost is somewhat larger
structure because it keeps ->nextsquot / ->prevsquot pointers among ->next / ->prev. On the other hand, it doesn't need
->unused counter, as for fixed-length chunks it can always be computed from the refcount and used data. Also somewhat larger
block ghost structure is compensated by the fact that the are no tail block waste and there is no 'neglect or not' problem.

Alignment
=========

Each allocator has 4 variants for 1, 2, 4, 8 bytes alignment respectively. Eg. stock32_take() always returns a pointer aligned
to 4 bytes, heap64_take() returns a pointer aligned to 8 bytes. You can ask for any data length, but in practise you'll always
obtain 1N, 2N, 4N or 8N. Alignment implies data padding unless the user requests for "aligned" sizes. In statistics the padding
is not considered a waste.

Zeroing
=======

All heap, stock and pool may return zeroed memory chunks, depending on initial flags:

  HEAP_ZERO
  STOCK_ZERO
  POOL_ZERO

There are also take0() variants that simply return memset(take(), 0, size), regardless the flag.
*/

#ifndef UTIL_MEM_ALLC_C
#define UTIL_MEM_ALLC_C

/*
Common internals for allocators suite. A selection or all of the following defines (from api headers) should already be there:

	UTIL_MEM_HEAP_H  // utilmemheap.h
	UTIL_MEM_STOCK_H // utilmemstock.h
	UTIL_MEM_POOL_H  // utilmempool.h

*/

#include <string.h> // memset()
#include <stdio.h> // printf()

#include "utilmem.h"

//#if defined(DEBUG) && debug != 0
#if 1
#  define ASSERT8(cond) ((void)((cond) || (printf("8bit allocator assertion, %s:%d: %s\n", __FILE__, __LINE__, #cond), 0)))
#  define ASSERT16(cond) ((void)((cond) || (printf("16bit allocator assertion, %s:%d: %s\n", __FILE__, __LINE__, #cond), 0)))
#  define ASSERT32(cond) ((void)((cond) || (printf("32bit allocator assertion, %s:%d: %s\n", __FILE__, __LINE__, #cond), 0)))
#  define ASSERT64(cond) ((void)((cond) || (printf("64bit allocator assertion, %s:%d: %s\n", __FILE__, __LINE__, #cond), 0)))
#else
#  define ASSERT8(cond) (void)0
#  define ASSERT16(cond) (void)0
#  define ASSERT32(cond) (void)0
#  define ASSERT64(cond) (void)0
#endif

#if defined(UTIL_MEM_STOCK_H) || defined(UTIL_MEM_POOL_H)
struct ghost8{
  uint8_t offset;
};

struct ghost16 {
  uint16_t offset;
};

#ifdef BIT32
struct ghost32 {
  union {
#ifdef UTIL_MEM_STOCK_H
    ream32 *ream;
#endif
#ifdef UTIL_MEM_POOL_H
    pile32 *pile;
    ghost32 *nextfree;
#endif
    void *block;
  };
};
#else
struct ghost32 {
  uint32_t offset;
};
#endif

struct ghost64 {
  union {
#ifdef UTIL_MEM_STOCK_H
    ream64 *ream;
#endif
#ifdef UTIL_MEM_POOL_H
    pile64 *pile;
    ghost64 *nextfree;
#endif
    void *block;
  };
#ifdef BIT32
  uint8_t dummy[4]; // force 8
#endif
};
#endif

/*
All offsets related macro horror is here. Block is 4/8-bytes aligned (32/64 pointer size), ream->data is adjusted to 1/2/4/8-bytes accordingly.
Therefore all offsets we store and pointers we cast, should be properly aligned. In all cases, sizes and offsets refers to bytes.
We need data ghosts only to access the block. For 8 and 16 we use 8/16 bit offsets to keep the ghost smaller. For 32 and 64 we either use offset,
or a pointer to the ream.

malloc() is obviously expected to return a pointer properly allowed for all standard c-types. For 64-bit we can safely expect at least 8-bytes aligned.
(at least, because long double may need 16 bytes on gcc64, or 8 bytes on msvc64, or weird on some exotics). On 32 bit machines pointers are 4 bytes
aligned, even long long is 4-bytes aligned. But double on 32bit machine is 8-bytes aligned on windows, 4 bytes aligned in linux (compiler option
-malign-double makes it 8-bytes aligned). Anyways, we cannot expect that on 32bit machine the result of malloc is always 8-bytes aligned.
This requires a very special treatment of 64-variant on 32bit machine: the first data ghost may need to be 4-bytes off. Should we ensure 4 bytes
more from malloc just in case? Hmm padding will be there anyway, as we adjust ream->data size to bytes boundaries.

In both 32/64bit environments, the ghost keeps a pointer to the block. On 32bit machine, the first chunk ghost address may need to be +4,
as this is not ensured by malloc(). See struct ream64 {}. We have an extra test; the final ghost pointer will be properly aligned iff

  ((block & 7 == 0) && (sizeof(block64) & 7 == 0)) || ((block & 7 == 4) && (sizeof(block64) & 7 == 4)

or in short

  ((block + 1) & 7) == 0

otherwise it needs 4 bytes offset.
*/

#define pointer_tointeger(p) ((size_t)(p)) // & not allowed on pointer

#define pointer_aligned32(p) ((pointer_tointeger(p) & 3) == 0)
#define pointer_aligned64(p) ((pointer_tointeger(p) & 7) == 0)

#define void_data(data) ((void *)(data))
#define byte_data(data) ((uint8_t *)(data))

/* top of the block ghost */

#define block_top(block) (byte_data(block + 1))

/* where the data begins */

#define block_edge8(block) block_top(block)
#define block_edge16(block) block_top(block)
#define block_edge32(block) block_top(block)

#ifdef BIT32
#  define ALIGN64ON32(block) (pointer_aligned64(block + 1) ? 0 : 4)
#  define block_edge64(block) (block_top(block) + ALIGN64ON32(block))
#else
#  define block_edge64(block) block_top(block)
#endif

#define block_left8(block, size) (size)
#define block_left16(block, size) (size)
#define block_left32(block, size) (size)
#ifdef BIT32
#  define block_left64(block, size) (size - ALIGN64ON32(block))
#else
#  define block_left64(block, size) (size)
#endif

/* consumed block space; it is important to use edge() macros that involves ALIGN64ON32() */

#define block_used8(block)  (block->data - block_edge8(block))
#define block_used16(block) (block->data - block_edge16(block))
#define block_used32(block) (block->data - block_edge32(block))
#define block_used64(block) (block->data - block_edge64(block))

/* align requested size to keep ream->data / pyre->data always aligned. size is always size_t, no insane overflow checks */

#define align_size8(size) ((void)size)
#define align_size16(size) (size = aligned_size16(size))
#define align_size32(size) (size = aligned_size32(size))
#define align_size64(size) (size = aligned_size64(size))

/*
done() and pop() operations decrements block->left space by an aligned size; block->left -= alignedwritten. Lets have 8-bytes aligned
variant block. If we tell the user there is 15 bytes left (block->left == 15) and the user taked 12. Aligned is 16, we cannot substract.
We could eventually set block->left to 0, but then pop() operation would no be allowed. Hance, block->left must be aligned. The procedure
is different than for size (size_t), we cannot cross 0xff/0xffff,... bondaries.
*/

#define align_space8(space) ((void)space)
#define align_space16(space) (space = aligned_space16(space))
#define align_space32(space) (space = aligned_space32(space))
#define align_space64(space) (space = aligned_space64(space))

/* handling ghost structure (stock and pool) */

#if defined(UTIL_MEM_STOCK_H) || defined(UTIL_MEM_POOL_H)

/* ghost offset from block top; not from bottom because we must not exceed offset limit */

#define ghost_offset(block, ghost) (byte_data(ghost) - block_top(block))

/* ghost <-> data */

#define ghost_data(ghost) ((void *)(ghost + 1))

/* cast from data to ghost structure goes via (void *) to shut up warnigns, alignment ok */

#define data_ghost8(data)  (((ghost8 *)void_data(data)) - 1)
#define data_ghost16(data) (((ghost16 *)void_data(data)) - 1)
#define data_ghost32(data) (((ghost32 *)void_data(data)) - 1)
#define data_ghost64(data) (((ghost64 *)void_data(data)) - 1)

/* ghost <-> block */

#define ghost_block8(ghost, block8) ((block8 *)void_data(byte_data(ghost) - ghost->offset - sizeof(block8)))
#define ghost_block16(ghost, block16) ((block16 *)void_data(byte_data(ghost) - ghost->offset - sizeof(block16)))
#ifdef BIT32
#  define ghost_block32(ghost, block32) (ghost->block)
#else
#  define ghost_block32(ghost, block32) ((block32 *)void_data(byte_data(ghost) - ghost->offset - sizeof(block32)))
#endif
#define ghost_block64(ghost, block64) (ghost->block)

/* ghost init */

#define ghost_next8(block, ghost) ((ghost = block->dataghost), (ghost->offset = (uint8_t)ghost_offset(block, ghost)))
#define ghost_next16(block, ghost) ((ghost = block->dataghost), (ghost->offset = (uint16_t)ghost_offset(block, ghost)))
#ifdef BIT32
#  define ghost_next32(bl0ck, ghost) ((ghost = bl0ck->dataghost), (ghost->block = bl0ck))
#else
#  define ghost_next32(block, ghost) ((ghost = block->dataghost), (ghost->offset = (uint32_t)ghost_offset(block, ghost)))
#endif
#define ghost_next64(bl0ck, ghost) ((ghost = bl0ck->dataghost), (ghost->block = bl0ck))

#endif

/* average block chunk size */

#define average_block_chunk8(ream)  (block_used8(ream) / ream->chunks)
#define average_block_chunk16(ream) (block_used16(ream) / ream->chunks)
#define average_block_chunk32(ream) (block_used32(ream) / ream->chunks)
#define average_block_chunk64(ream) (block_used64(ream) / ream->chunks)

/*
neglect remaining block tail and start a new block or create a single block; a test for (block->chunks > 0) is a sanity;
if block->chunks is zero (block has a full space left), we shouldn't get there, except when alloc->large is larger then alloc->space
*/

#define take_new_block8(alloc, ghoststruct, block, size) \
  ((size < alloc->large) && (block->left <= sizeof(ghoststruct) || (block->chunks > 0 && block->left <= average_block_chunk8(block))))
#define take_new_block16(alloc, ghoststruct, block, size) \
  ((size < alloc->large) && (block->left <= sizeof(ghoststruct) || (block->chunks > 0 && block->left <= average_block_chunk16(block))))
#define take_new_block32(alloc, ghoststruct, block, size) \
  ((size < alloc->large) && (block->left <= sizeof(ghoststruct) || (block->chunks > 0 && block->left <= average_block_chunk32(block))))
#define take_new_block64(alloc, ghoststruct, block, size) \
  ((size < alloc->large) && (block->left <= sizeof(ghoststruct) || (block->chunks > 0 && block->left <= average_block_chunk64(block))))

/* empty */

#define head_block_empty(alloc, block) (((block = alloc->head) == NULL) || (block->chunks == 0 && block->prev == NULL))

#endif