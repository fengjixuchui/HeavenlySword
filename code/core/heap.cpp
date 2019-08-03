/** \file core/dlmalloc.c
  This is a version (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea. and released to the public domain.  Use, modify, and
  redistribute this code without permission or acknowledgement in any
  way you wish.  Send questions, comments, complaints, performance
  data, etc to dl@cs.oswego.edu

* VERSION 2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)

   Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
         Check before installing!

 Massively modified to only allocate from a fixed size pool
*/

#include "core/heap.h"

/* #define WIN32 */

#if defined(WIN32) || defined(_XBOX)


/* Win32 doesn't supply or need the following headers */
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H

#endif

#include <stddef.h>   /* for size_t */

/* define LACKS_UNISTD_H if your system does not have a <unistd.h>. */

/* #define  LACKS_UNISTD_H */

#ifndef LACKS_UNISTD_H
#include <unistd.h>
#endif

/* define LACKS_SYS_PARAM_H if your system does not have a <sys/param.h>. */

/* #define  LACKS_SYS_PARAM_H */


/*
  Debugging:

  Because freed chunks may be overwritten with bookkeeping fields, this
  malloc will often die when freed memory is overwritten by user
  programs.  This can be very effective (albeit in an annoying way)
  in helping track down dangling pointers.

  If you compile with -DDEBUG, a number of ntAssertion checks are
  enabled that will catch more memory errors. You probably won't be
  able to make much sense of the actual ntAssertion errors, but they
  should help you locate incorrectly overwritten memory.  The
  checking is fairly extensive, and will slow down execution
  noticeably. Calling malloc_stats or mallinfo with DEBUG set will
  attempt to check every non-mmapped allocated and free chunk in the
  course of computing the summmaries. (By nature, mmapped regions
  cannot be checked very much automatically.)

  Setting DEBUG may also be helpful if you are trying to modify
  this code. The ntAssertions in the check routines spell out in more
  detail the assumptions and invariants underlying the algorithms.

  Setting DEBUG does NOT provide an automated mechanism for checking
  that all accesses to malloced memory stay within their
  bounds. However, there are several add-ons and adaptations of this
  or other mallocs available that do this.
*/

/* Deano mod
#if DEBUG
#include <ntAssert.h>
#else
#define ntAssert(x) ((void)0)
#endif

*/
/*
  The unsigned integer type used for comparing any two chunk sizes.
  This should be at least as wide as size_t, but should not be signed.
*/

#ifndef CHUNK_SIZE_T
#define CHUNK_SIZE_T unsigned long
#endif

/* 
  The unsigned integer type used to hold addresses when they are are
  manipulated as integers. Except that it is not defined on all
  systems, intptr_t would suffice.
*/
#ifndef PTR_UINT
#define PTR_UINT unsigned long
#endif


/*
  INTERNAL_SIZE_T is the word-size used for internal bookkeeping
  of chunk sizes.

  The default version is the same as size_t.

  While not strictly necessary, it is best to define this as an
  unsigned type, even if size_t is a signed type. This may avoid some
  artificial size limitations on some systems.

  On a 64-bit machine, you may be able to reduce malloc overhead by
  defining INTERNAL_SIZE_T to be a 32 bit `unsigned int' at the
  expense of not being able to handle more than 2^32 of malloced
  space. If this limitation is acceptable, you are encouraged to set
  this unless you are on a platform requiring 16byte alignments. In
  this case the alignment requirements turn out to negate any
  potential advantages of decreasing size_t word size.

  Implementors: Beware of the possible combinations of:
     - INTERNAL_SIZE_T might be signed or unsigned, might be 32 or 64 bits,
       and might be the same width as int or as long
     - size_t might have different width and signedness as INTERNAL_SIZE_T
     - int and long might be 32 or 64 bits, and might be the same width
  To deal with this, most comparisons and difference computations
  among INTERNAL_SIZE_Ts should cast them to CHUNK_SIZE_T, being
  aware of the fact that casting an unsigned int to a wider long does
  not sign-extend. (This also makes checking for negative numbers
  awkward.) Some of these casts result in harmless compiler warnings
  on some systems.
*/

// change when we move to 080
#define INTERNAL_SIZE_T uint32_t

/* The corresponding word size */
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))



/*
  MALLOC_ALIGNMENT is the minimum alignment for malloc'ed chunks.
  It must be a power of two at least 2 * SIZE_SZ, even on machines
  for which smaller alignments would suffice. It may be defined as
  larger than this though. Note however that code and data structures
  are optimized for the case of 8-byte alignment.
*/
#define MALLOC_ALIGNMENT       16

/* The corresponding bit mask value */
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)

#ifndef DEFAULT_TRIM_THRESHOLD
#define DEFAULT_TRIM_THRESHOLD (256 * 1024)
#endif

#ifndef DEFAULT_MXFAST
#define DEFAULT_MXFAST     64
#endif


/*
  TRIM_FASTBINS controls whether free() of a very small chunk can
  immediately lead to trimming. Setting to true (1) can reduce memory
  footprint, but will almost always slow down programs that use a lot
  of small chunks.

  Define this only if you are willing to give up some speed to more
  aggressively reduce system-level memory footprint when releasing
  memory in programs that use many small chunks.  You can get
  essentially the same effect by setting MXFAST to 0, but this can
  lead to even greater slowdowns in programs using many small chunks.
  TRIM_FASTBINS is an in-between compile-time option, that disables
  only those chunks bordering topmost memory from being placed in
  fastbins.
*/

#ifndef TRIM_FASTBINS
#define TRIM_FASTBINS  0
#endif

/*
  -----------------------  Chunk representations -----------------------
*/


/*
  This struct declaration is misleading (but accurate and necessary).
  It declares a "view" into memory allowing access to necessary
  fields at known offsets from a given base. See explanation below.
*/

struct malloc_chunk {

  INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};


typedef struct malloc_chunk* mchunkptr;

/*
   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:


    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk, if allocated            | |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             User data starts here...                          .
            .                                                               .
            .             (malloc_usable_space() bytes)                     .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk                                     |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus at least double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory. If
    prev_inuse is set for any given chunk, then you CANNOT determine
    the size of the previous chunk, and might even get a memory
    addressing fault when trying to do so.

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. This makes it easier to
    deal with alignments etc but can be very confusing when trying
    to extend or adapt this code.

    The two exceptions to all this are

     1. The special chunk `top' doesn't bother using the
        trailing size field since there is no next contiguous chunk
        that would have to index off it. After initialization, `top'
        is forced to always exist.  If it would become less than
        MINSIZE bytes long, it is replenished.

     2. Chunks allocated via mmap, which have the second-lowest-order
        bit (IS_MMAPPED) set in their size fields.  Because they are
        allocated one-by-one, each must contain its own trailing size field.

*/

/*
  ---------- Size and alignment checks and conversions ----------
*/

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (sizeof(struct malloc_chunk))

/* The smallest size we can malloc is an aligned minimal chunk */

#define MINSIZE  \
  (CHUNK_SIZE_T)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

/* Check if m has acceptable alignment */

#define aligned_OK(m)  (((PTR_UINT)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/* 
   Check if a request is so large that it would wrap around zero when
   padded and aligned. To simplify some other code, the bound is made
   low enough so that adding MINSIZE will also not wrap around sero.
*/

#define REQUEST_OUT_OF_RANGE(req)                                 \
  ((CHUNK_SIZE_T)(req) >=                                        \
   (CHUNK_SIZE_T)(INTERNAL_SIZE_T)(-2 * MINSIZE))    

/* pad request bytes into a usable size -- internal version */

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/*  Same, except also perform argument check */
/* Doens't compile on Win32 Warn3X the check does evil things
#define checked_request2size(req, sz)                             \
  if (REQUEST_OUT_OF_RANGE(req)) {                                \
    MALLOC_FAILURE_ACTION;                                        \
    return 0;                                                     \
  }                                                               \
  (sz) = request2size(req);                                              
*/
  #define checked_request2size(req, sz)                             \
  (sz) = request2size(req);                                              

/*
  --------------- Physical chunk operations ---------------
*/


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
#define PREV_INUSE 0x1

/* extract inuse bit of previous chunk */
#define prev_inuse(p)       ((p)->size & PREV_INUSE)


/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
#define IS_MMAPPED 0x2

/* check for mmap()'ed chunk */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* 
  Bits to mask off when extracting size 

  Note: IS_MMAPPED is intentionally not masked off from size field in
  macros for which mmapped chunks should never be seen. This should
  cause helpful core dumps to occur if it is tried by accident by
  people extending or adapting this malloc.
*/
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))


/* Ptr to next physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */
#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* extract p's inuse bit */
#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* set/clear chunk as being inuse without otherwise disturbing */
#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)


/* check/set/clear inuse bits in known places */
#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))


/* Set size at head, without disturbing its use bit */
#define set_head_size(p, s)  ((p)->size = (((p)->size & PREV_INUSE) | (s)))

/* Set size/use field */
#define set_head(p, s)       ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */
#define set_foot(p, s)       (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))

#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))


/*
  -------------------- Internal data structures --------------------

   All internal state is held in an instance of malloc_state defined
   below. There are no other static variables, except in two optional
   cases: 
   * If USE_MALLOC_LOCK is defined, the mALLOC_MUTEx declared above. 
   * If HAVE_MMAP is true, but mmap doesn't support
     MAP_ANONYMOUS, a dummy file descriptor for mmap.

   Beware of lots of tricks that minimize the total bookkeeping space
   requirements. The result is a little over 1K bytes (for 4byte
   pointers and size_t.)
*/

/*
  Bins

    An array of bin headers for free chunks. Each bin is doubly
    linked.  The bins are approximately proportionally (log) spaced.
    There are a lot of these bins (128). This may look excessive, but
    works very well in practice.  Most bins hold sizes that are
    unusual as malloc request sizes, but are more usual for fragments
    and consolidated sets of chunks, which is what these bins hold, so
    they can be found quickly.  All procedures maintain the invariant
    that no consolidated chunk physically borders another one, so each
    chunk in a list is known to be preceeded and followed by either
    inuse chunks or the ends of memory.

    Chunks in bins are kept in size order, with ties going to the
    approximately least recently used chunk. Ordering isn't needed
    for the small bins, which all contain the same-sized chunks, but
    facilitates best-fit allocation for larger chunks. These lists
    are just sequential. Keeping them in order almost never requires
    enough traversal to warrant using fancier ordered data
    structures.  

    Chunks of the same size are linked with the most
    recently freed at the front, and allocations are taken from the
    back.  This results in LRU (FIFO) allocation order, which tends
    to give each chunk an equal opportunity to be consolidated with
    adjacent freed chunks, resulting in larger free chunks and less
    fragmentation.

    To simplify use in double-linked lists, each bin header acts
    as a malloc_chunk. This avoids special-casing for headers.
    But to conserve space and improve locality, we allocate
    only the fd/bk pointers of bins, and then use repositioning tricks
    to treat these as the fields of a malloc_chunk*.  
*/

typedef struct malloc_chunk* mbinptr;

/* addressing -- note that bin_at(0) does not exist */
#define bin_at(m, i) ((mbinptr)((char*)&((m)->bins[(i)<<1]) - (SIZE_SZ<<1)))

/* analog of ++bin */
#define next_bin(b)  ((mbinptr)((char*)(b) + (sizeof(mchunkptr)<<1)))

/* Reminders about list directionality within bins */
#define first(b)     ((b)->fd)
#define last(b)      ((b)->bk)

/* Take a chunk off a bin list */
#define unlink(P, BK, FD) {                                            \
  FD = P->fd;                                                          \
  BK = P->bk;                                                          \
  FD->bk = BK;                                                         \
  BK->fd = FD;                                                         \
}

/*
  Indexing

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically spaced:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    The bins top out around 1MB because we expect to service large
    requests via mmap.
*/

#define NBINS              96
#define NSMALLBINS         32
#define SMALLBIN_WIDTH      8
#define MIN_LARGE_SIZE    256

#define in_smallbin_range(sz)  \
  ((CHUNK_SIZE_T)(sz) < (CHUNK_SIZE_T)MIN_LARGE_SIZE)

#define smallbin_index(sz)     (((unsigned)(sz)) >> 3)

/*
  Compute index for size. We expect this to be inlined when
  compiled with optimization, else not, which works out well.
*/
static int largebin_index(unsigned int sz) {
  unsigned int  x = sz >> SMALLBIN_WIDTH; 
  unsigned int m;            /* bit position of highest set bit of m */

  if (x >= 0x10000) return NBINS-1;

  /* On intel, use BSRL instruction to find highest bit */
#if defined(__GNUC__) && defined(i386)

  __asm__("bsrl %1,%0\n\t"
          : "=r" (m) 
          : "g"  (x));

#else
  {
    /*
      Based on branch-free nlz algorithm in chapter 5 of Henry
      S. Warren Jr's book "Hacker's Delight".
    */

    unsigned int n = ((x - 0x100) >> 16) & 8;
    x <<= n; 
    m = ((x - 0x1000) >> 16) & 4;
    n += m; 
    x <<= m; 
    m = ((x - 0x4000) >> 16) & 2;
    n += m; 
    x = (x << m) >> 14;
    m = 13 - n + (x & ~(x>>1));
  }
#endif

  /* Use next 2 bits to create finer-granularity bins */
  return NSMALLBINS + (m << 2) + ((sz >> (m + 6)) & 3);
}

#define bin_index(sz) \
 ((in_smallbin_range(sz)) ? smallbin_index(sz) : largebin_index(sz))

/*
  FIRST_SORTED_BIN_SIZE is the chunk size corresponding to the
  first bin that is maintained in sorted order. This must
  be the smallest size corresponding to a given bin.

  Normally, this should be MIN_LARGE_SIZE. But you can weaken
  best fit guarantees to sometimes speed up malloc by increasing value.
  Doing this means that malloc may choose a chunk that is 
  non-best-fitting by up to the width of the bin.

  Some useful cutoff values:
      512 - all bins sorted
     2560 - leaves bins <=     64 bytes wide unsorted  
    12288 - leaves bins <=    512 bytes wide unsorted
    65536 - leaves bins <=   4096 bytes wide unsorted
   262144 - leaves bins <=  32768 bytes wide unsorted
       -1 - no bins sorted (not recommended!)
*/

#define FIRST_SORTED_BIN_SIZE MIN_LARGE_SIZE 
/* #define FIRST_SORTED_BIN_SIZE 65536 */

/*
  Unsorted chunks

    All remainders from chunk splits, as well as all returned chunks,
    are first placed in the "unsorted" bin. They are then placed
    in regular bins after malloc gives them ONE chance to be used before
    binning. So, basically, the unsorted_chunks list acts as a queue,
    with chunks being placed on it in free (and malloc_consolidate),
    and taken off (to be either used or placed in bins) in malloc.
*/

/* The otherwise unindexable 1-bin is used to hold unsorted chunks. */
#define unsorted_chunks(M)          (bin_at(M, 1))

/*
  Top

    The top-most available chunk (i.e., the one bordering the end of
    available memory) is treated specially. It is never included in
    any bin, is used only if no other chunk is available, and is
    released back to the system if it is very large (see
    M_TRIM_THRESHOLD).  Because top initially
    points to its own bin with initial zero size, thus forcing
    extension on the first malloc request, we avoid having any special
    code in malloc to check whether it even exists yet. But we still
    need to do so when getting memory from system, so we make
    initial_top treat the bin as a legal but unusable chunk during the
    interval between initialization and the first call to
    sYSMALLOc. (This is somewhat delicate, since it relies on
    the 2 preceding words to be zero during this interval as well.)
*/

/* Conveniently, the unsorted bin can be used as dummy top on first call */
#define initial_top(M)              (unsorted_chunks(M))

/*
  Binmap

    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binmap' is a
    bitvector recording whether bins are definitely empty so they can
    be skipped over during during traversals.  The bits are NOT always
    cleared as soon as bins are empty, but instead only
    when they are noticed to be empty during traversal in malloc.
*/

/* Conservatively use 32 bits per map word, even if on 64bit system */
#define BINMAPSHIFT      5
#define BITSPERMAP       (1U << BINMAPSHIFT)
#define BINMAPSIZE       (NBINS / BITSPERMAP)

#define idx2block(i)     ((i) >> BINMAPSHIFT)
#define idx2bit(i)       ((1U << ((i) & ((1U << BINMAPSHIFT)-1))))

#define mark_bin(m,i)    ((m)->binmap[idx2block(i)] |=  idx2bit(i))
#define unmark_bin(m,i)  ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
#define get_binmap(m,i)  ((m)->binmap[idx2block(i)] &   idx2bit(i))

/*
  Fastbins

    An array of lists holding recently freed small chunks.  Fastbins
    are not doubly linked.  It is faster to single-link them, and
    since chunks are never removed from the middles of these lists,
    double linking is not necessary. Also, unlike regular bins, they
    are not even processed in FIFO order (they use faster LIFO) since
    ordering doesn't much matter in the transient contexts in which
    fastbins are normally used.

    Chunks in fastbins keep their inuse bit set, so they cannot
    be consolidated with other free chunks. malloc_consolidate
    releases all chunks in fastbins and consolidates them with
    other free chunks. 
*/

typedef struct malloc_chunk* mfastbinptr;

/* offset 2 to use otherwise unindexable first 2 bins */
#define fastbin_index(sz)        ((((unsigned int)(sz)) >> 3) - 2)

/* The maximum fastbin request size we support */
#define MAX_FAST_SIZE     80

#define NFASTBINS  (fastbin_index(request2size(MAX_FAST_SIZE))+1)

/*
  FASTBIN_CONSOLIDATION_THRESHOLD is the size of a chunk in free()
  that triggers automatic consolidation of possibly-surrounding
  fastbin chunks. This is a heuristic, so the exact value should not
  matter too much. It is defined at half the default trim threshold as a
  compromise heuristic to only attempt consolidation if it is likely
  to lead to trimming. However, it is not dynamically tunable, since
  consolidation reduces fragmentation surrounding loarge chunks even 
  if trimming is not used.
*/

#define FASTBIN_CONSOLIDATION_THRESHOLD  \
  ((unsigned long)(DEFAULT_TRIM_THRESHOLD) >> 1)

/*
  Since the lowest 2 bits in max_fast don't matter in size comparisons, 
  they are used as flags.
*/

/*
  ANYCHUNKS_BIT held in max_fast indicates that there may be any
  freed chunks at all. It is set true when entering a chunk into any
  bin.
*/

#define ANYCHUNKS_BIT        (1U)

#define have_anychunks(M)     (((M)->max_fast &  ANYCHUNKS_BIT))
#define set_anychunks(M)      ((M)->max_fast |=  ANYCHUNKS_BIT)
#define clear_anychunks(M)    ((M)->max_fast &= ~ANYCHUNKS_BIT)

/*
  FASTCHUNKS_BIT held in max_fast indicates that there are probably
  some fastbin chunks. It is set true on entering a chunk into any
  fastbin, and cleared only in malloc_consolidate.
*/

#define FASTCHUNKS_BIT        (2U)

#define have_fastchunks(M)   (((M)->max_fast &  FASTCHUNKS_BIT))
#define set_fastchunks(M)    ((M)->max_fast |=  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))
#define clear_fastchunks(M)  ((M)->max_fast &= ~(FASTCHUNKS_BIT))

/* 
   Set value of max_fast. 
   Use impossibly small value if 0.
*/

#define set_max_fast(M, s) \
  (M)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  ((M)->max_fast &  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))

#define get_max_fast(M) \
  ((M)->max_fast & ~(FASTCHUNKS_BIT | ANYCHUNKS_BIT))

/*
  morecore_properties is a status word holding dynamically discovered
  or controlled properties of the morecore function
*/

#define MORECORE_CONTIGUOUS_BIT  (1U)

#define contiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT))
#define noncontiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT) == 0)
#define set_contiguous(M) \
        ((M)->morecore_properties |=  MORECORE_CONTIGUOUS_BIT)
#define set_noncontiguous(M) \
        ((M)->morecore_properties &= ~MORECORE_CONTIGUOUS_BIT)


/*
   ----------- Internal state representation and initialization -----------
*/

struct malloc_state 
{

  /* The maximum chunk size to be eligible for fastbin */
  INTERNAL_SIZE_T  max_fast;   /* low 2 bits used as flags */

  /* Fastbins */
  mfastbinptr      fastbins[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr        top;

  /* The remainder from the most recent split of a small request */
  mchunkptr        last_remainder;

  /* Normal bins packed as described above */
  mchunkptr        bins[NBINS * 2];

  /* Bitmap of bins. Trailing zero map handles cases of largest binned size */
  unsigned int     binmap[BINMAPSIZE+1];

  /* Tunable parameters */
  CHUNK_SIZE_T     trim_threshold;

  /* Cache malloc_getpagesize */
  unsigned int     pagesize;    

  /* Track properties of MORECORE */
  unsigned int     morecore_properties;

  /* Statistics */
  INTERNAL_SIZE_T  mmapped_mem;
  INTERNAL_SIZE_T  sbrked_mem;
  INTERNAL_SIZE_T  max_sbrked_mem;
  INTERNAL_SIZE_T  max_mmapped_mem;
  INTERNAL_SIZE_T  max_total_mem;
};

typedef struct malloc_state *mstate;

/*
   All uses of av_ are via get_malloc_state().
   At most one "call" to get_malloc_state is made per invocation of
   the public versions of malloc and free, but other routines
   that in turn invoke malloc and/or free may call more then once. 
   Also, it is called in check* routines if DEBUG is set.
*/

#define get_malloc_state() (m_pMallocState)

// debug prototypes
#if !defined( _DEBUG )

#define check_chunk(av, P)
#define check_free_chunk(av, P)
#define check_inuse_chunk(av, P)
#define check_remalloced_chunk(av, P,N)
#define check_malloced_chunk(av, P,N)
#define check_malloc_state(av)
#define do_check_malloc_state( av )

#else

void do_check_chunk(mstate av, mchunkptr p);
void do_check_free_chunk(mstate av, mchunkptr p);
void do_check_inuse_chunk(mstate av, mchunkptr p);
void do_check_remalloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s);
void do_check_malloc_state( mstate av );
void do_check_malloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s);

#define check_chunk(av, P)              do_check_chunk(av, P)
#define check_free_chunk(av, P)         do_check_free_chunk(av, P)
#define check_inuse_chunk(av, P)        do_check_inuse_chunk(av, P)
#define check_remalloced_chunk(av, P,N) do_check_remalloced_chunk(av, P,N)
#define check_malloced_chunk(av, P,N)   do_check_malloced_chunk(av, P,N)
#define check_malloc_state(av )		   do_check_malloc_state(av)

#endif


void Heap::Initialise( uintptr_t pAddr, uint32_t iSize )
{
	m_HeapCritical.Initialise();

	// we make our state the beginning of of our managed heap
	m_pMallocState = (malloc_state*)pAddr;
	memset( m_pMallocState, 0, sizeof(malloc_state) );

	mstate av = m_pMallocState;

	/* Establish circular links for normal bins */
	for (int i = 1; i < NBINS; ++i) 
	{ 
		mbinptr bin = bin_at(av,i);
		bin->fd = bin->bk = bin;
	}

	av->trim_threshold = DEFAULT_TRIM_THRESHOLD;

	set_max_fast(av, DEFAULT_MXFAST);
	av->top = initial_top(av);

	m_pCurSysHigh = (char*)(((pAddr + sizeof(malloc_state))+MALLOC_ALIGNMENT) & ~MALLOC_ALIGN_MASK);
	m_pMemTop = (char*)(pAddr + iSize);
	
	set_contiguous(av);

	av->pagesize       = 4096;

	do_check_malloc_state(av);

	// to reduce calls to sysmalloc (which is totally unnesseacy if I 
	// could figure out this damn system!) we allocate a big chunk
	// and then free it...
	void* pTmp = Malloc( (m_pMemTop - m_pCurSysHigh) - av->pagesize );
	Free( pTmp );
}

void Heap::Kill()
{
	m_HeapCritical.Kill();
}

//!
//! malloc(size_t n)
//! Returns a pointer to a newly allocated chunk of at least n bytes, or 0
//! if no space is available. 
//!
//! If n is zero, malloc returns a minumum-sized chunk. (The minimum
//! size is 16 bytes on most 32bit systems, and 24 or 32 bytes on 64bit
//! systems.)  On most systems, size_t is an unsigned type, so calls
//! with negative arguments are interpreted as requests for huge amounts
//! of space, which will often fail. The maximum supported value of n
//! differs across systems, but is in all cases less than the maximum
//! representable value of a size_t.
//!
void* Heap::Malloc(size_t bytes)
{
	ScopedCritical crit( m_HeapCritical );
 
	mstate av = get_malloc_state();

  INTERNAL_SIZE_T nb;               /* normalized request size */
  unsigned int    idx;              /* associated bin index */
  mbinptr         bin;              /* associated bin */
  mfastbinptr*    fb;               /* associated fastbin */

  mchunkptr       victim;           /* inspected/selected chunk */
  INTERNAL_SIZE_T size;             /* its size */
  int             victim_index;     /* its bin index */

  mchunkptr       remainder;        /* remainder from a split */
  CHUNK_SIZE_T    remainder_size;   /* its size */

  unsigned int    block;            /* bit map traverser */
  unsigned int    bit;              /* bit map traverser */
  unsigned int    map;              /* current word of binmap */

  mchunkptr       fwd;              /* misc temp for linking */
  mchunkptr       bck;              /* misc temp for linking */

  nb = 0;

  /*
    Convert request size to internal form by adding SIZE_SZ bytes
    overhead plus possibly more to obtain necessary alignment and/or
    to obtain a size of at least MINSIZE, the smallest allocatable
    size. Also, checked_request2size traps (returning 0) request sizes
    that are so large that they wrap around zero when padded and
    aligned.
  */

  checked_request2size(bytes, nb);

  /*
    Bypass search if no frees yet
   */
  if (!have_anychunks(av)) {
    if (av->max_fast == 0) /* initialization check */
      malloc_consolidate(av);
    goto use_top;
  }

  /*
    If the size qualifies as a fastbin, first check corresponding bin.
  */

  if ((CHUNK_SIZE_T)(nb) <= (CHUNK_SIZE_T)(av->max_fast)) { 
    fb = &(av->fastbins[(fastbin_index(nb))]);
    if ( (victim = *fb) != 0) {
      *fb = victim->fd;
      check_remalloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
  }

  /*
    If a small request, check regular bin.  Since these "smallbins"
    hold one size each, no searching within bins is necessary.
    (For a large request, we need to wait until unsorted chunks are
    processed to find best fit. But for small ones, fits are exact
    anyway, so we can check now, which is faster.)
  */

  if (in_smallbin_range(nb)) {
    idx = smallbin_index(nb);
    bin = bin_at(av,idx);

    if ( (victim = last(bin)) != bin) {
      bck = victim->bk;
      set_inuse_bit_at_offset(victim, nb);
      bin->bk = bck;
      bck->fd = bin;
      
      check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
  }

  /* 
     If this is a large request, consolidate fastbins before continuing.
     While it might look excessive to kill all fastbins before
     even seeing if there is space available, this avoids
     fragmentation problems normally associated with fastbins.
     Also, in practice, programs tend to have runs of either small or
     large requests, but less often mixtures, so consolidation is not 
     invoked all that often in most programs. And the programs that
     it is called frequently in otherwise tend to fragment.
  */

  else {
    idx = largebin_index(nb);
    if (have_fastchunks(av)) 
      malloc_consolidate(av);
  }

  /*
    Process recently freed or remaindered chunks, taking one only if
    it is exact fit, or, if this a small request, the chunk is remainder from
    the most recent non-exact fit.  Place other traversed chunks in
    bins.  Note that this step is the only place in any routine where
    chunks are placed in bins.
  */
    
  while ( (victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) {
    bck = victim->bk;
    size = chunksize(victim);
    
    /* 
       If a small request, try to use last remainder if it is the
       only chunk in unsorted bin.  This helps promote locality for
       runs of consecutive small requests. This is the only
       exception to best-fit, and applies only when there is
       no exact fit for a small chunk.
    */
    
    if (in_smallbin_range(nb) && 
        bck == unsorted_chunks(av) &&
        victim == av->last_remainder &&
        (CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      
      /* split and reattach remainder */
      remainder_size = size - nb;
      remainder = chunk_at_offset(victim, nb);
      unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
      av->last_remainder = remainder; 
      remainder->bk = remainder->fd = unsorted_chunks(av);
      
      set_head(victim, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      
      check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
    
    /* remove from unsorted list */
    unsorted_chunks(av)->bk = bck;
    bck->fd = unsorted_chunks(av);
    
    /* Take now instead of binning if exact fit */
    
    if (size == nb) {
      set_inuse_bit_at_offset(victim, size);
      check_malloced_chunk(av, victim, nb);
      return chunk2mem(victim);
    }
    
    /* place chunk in bin */
    
    if (in_smallbin_range(size)) {
      victim_index = smallbin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
    }
    else {
      victim_index = largebin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
      
      if (fwd != bck) {
        /* if smaller than smallest, place first */
        if ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(bck->bk->size)) {
          fwd = bck;
          bck = bck->bk;
        }
        else if ((CHUNK_SIZE_T)(size) >= 
                 (CHUNK_SIZE_T)(FIRST_SORTED_BIN_SIZE)) {
          
          /* maintain large bins in sorted order */
          size |= PREV_INUSE; /* Or with inuse bit to speed comparisons */
          while ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(fwd->size)) 
            fwd = fwd->fd;
          bck = fwd->bk;
        }
      }
    }
      
    mark_bin(av, victim_index);
    victim->bk = bck;
    victim->fd = fwd;
    fwd->bk = victim;
    bck->fd = victim;
  }
  
  /*
    If a large request, scan through the chunks of current bin to
    find one that fits.  (This will be the smallest that fits unless
    FIRST_SORTED_BIN_SIZE has been changed from default.)  This is
    the only step where an unbounded number of chunks might be
    scanned without doing anything useful with them. However the
    lists tend to be short.
  */
  
  if (!in_smallbin_range(nb)) {
    bin = bin_at(av, idx);
    
    for (victim = last(bin); victim != bin; victim = victim->bk) {
      size = chunksize(victim);
      
      if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb)) {
        remainder_size = size - nb;
        unlink(victim, bck, fwd);
        
        /* Exhaust */
        if (remainder_size < MINSIZE)  {
          set_inuse_bit_at_offset(victim, size);
          check_malloced_chunk(av, victim, nb);
          return chunk2mem(victim);
        }
        /* Split */
        else {
          remainder = chunk_at_offset(victim, nb);
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
          set_head(victim, nb | PREV_INUSE);
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
          check_malloced_chunk(av, victim, nb);
          return chunk2mem(victim);
        } 
      }
    }    
  }

  /*
    Search for a chunk by scanning bins, starting with next largest
    bin. This search is strictly by best-fit; i.e., the smallest
    (with ties going to approximately the least recently used) chunk
    that fits is selected.
    
    The bitmap avoids needing to check that most blocks are nonempty.
  */
    
  ++idx;
  bin = bin_at(av,idx);
  block = idx2block(idx);
  map = av->binmap[block];
  bit = idx2bit(idx);
  
  for (;;) {
    
    /* Skip rest of block if there are no more set bits in this block.  */
    if (bit > map || bit == 0) {
      do {
        if (++block >= BINMAPSIZE)  /* out of bins */
          goto use_top;
      } while ( (map = av->binmap[block]) == 0);
      
      bin = bin_at(av, (block << BINMAPSHIFT));
      bit = 1;
    }
    
    /* Advance to bin with set bit. There must be one. */
    while ((bit & map) == 0) {
      bin = next_bin(bin);
      bit <<= 1;
      ntAssert(bit != 0);
    }
    
    /* Inspect the bin. It is likely to be non-empty */
    victim = last(bin);
    
    /*  If a false alarm (empty bin), clear the bit. */
    if (victim == bin) {
      av->binmap[block] = map &= ~bit; /* Write through */
      bin = next_bin(bin);
      bit <<= 1;
    }
    
    else {
      size = chunksize(victim);
      
      /*  We know the first chunk in this bin is big enough to use. */
      ntAssert((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb));
      
      remainder_size = size - nb;
      
      /* unlink */
      bck = victim->bk;
      bin->bk = bck;
      bck->fd = bin;
      
      /* Exhaust */
      if (remainder_size < MINSIZE) {
        set_inuse_bit_at_offset(victim, size);
        check_malloced_chunk(av, victim, nb);
        return chunk2mem(victim);
      }
      
      /* Split */
      else {
        remainder = chunk_at_offset(victim, nb);
        
        unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
        remainder->bk = remainder->fd = unsorted_chunks(av);
        /* advertise as last remainder */
        if (in_smallbin_range(nb)) 
          av->last_remainder = remainder; 
        
        set_head(victim, nb | PREV_INUSE);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_foot(remainder, remainder_size);
        check_malloced_chunk(av, victim, nb);
        return chunk2mem(victim);
      }
    }
  }

  use_top:    
  /*
    If large enough, split off the chunk bordering the end of memory
    (held in av->top). Note that this is in accord with the best-fit
    search rule.  In effect, av->top is treated as larger (and thus
    less well fitting) than any other available chunk since it can
    be extended to be as large as necessary (up to system
    limitations).
    
    We require that av->top always exists (i.e., has size >=
    MINSIZE) after initialization, so if it would otherwise be
    exhuasted by current request, it is replenished. (The main
    reason for ensuring it exists is that we may need MINSIZE space
    to put in fenceposts in sysmalloc.)
  */
  
  victim = av->top;
  size = chunksize(victim);
  
  if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
    remainder_size = size - nb;
    remainder = chunk_at_offset(victim, nb);
    av->top = remainder;
    set_head(victim, nb | PREV_INUSE);
    set_head(remainder, remainder_size | PREV_INUSE);
    
    check_malloced_chunk(av, victim, nb);
    return chunk2mem(victim);
  }

  /* 
     If no space in top, use our faked sysmalloc
  */
  return sys_malloc(nb,av);    
}

void*	Heap::Shrink( void* mem, uint32_t iSize )
{
	ScopedCritical crit( m_HeapCritical );

	mchunkptr		oldp = mem2chunk( mem );
	INTERNAL_SIZE_T	oldsize = chunksize( oldp );
	INTERNAL_SIZE_T	nb;

	checked_request2size(iSize, nb);


	if ( nb >= oldsize )
	{
		return mem;
	}

	INTERNAL_SIZE_T	rsize = oldsize - nb;

	if ( rsize < MIN_CHUNK_SIZE )
	{
		return mem;
	}

	mchunkptr	newp	= oldp;
	mchunkptr	remainder = chunk_plus_offset(newp, nb);
	set_head_size(newp, nb);
	set_inuse_bit_at_offset(newp, nb);
	set_head_size(remainder, rsize);
	set_inuse_bit_at_offset(remainder, rsize);
	void*	extra = chunk2mem(remainder);
	Free(extra);

	return chunk2mem(newp);
}

//---------------------------------------
//!
//!  free(void* p)
//!  Releases the chunk of memory pointed to by p, that had been previously
//!  allocated using malloc or a related routine such as realloc.
//!  It has no effect if p is null. It can have arbitrary (i.e., bad!)
//!  effects if p has already been freed.
//!
//---------------------------------------
void Heap::Free(void* mem)
{
	ScopedCritical crit( m_HeapCritical );

	ntAssert( IsOurs( (uintptr_t) mem) );

	mstate av = get_malloc_state();

  mchunkptr       p;           /* chunk corresponding to mem */
  INTERNAL_SIZE_T size;        /* its size */
  mfastbinptr*    fb;          /* associated fastbin */
  mchunkptr       nextchunk;   /* next contiguous chunk */
  INTERNAL_SIZE_T nextsize;    /* its size */
  int             nextinuse;   /* true if nextchunk is used */
  INTERNAL_SIZE_T prevsize;    /* size of previous contiguous chunk */
  mchunkptr       bck;         /* misc temp for linking */
  mchunkptr       fwd;         /* misc temp for linking */

  /* free(0) has no effect */
  if (mem != 0) {
    p = mem2chunk(mem);
    size = chunksize(p);

    check_inuse_chunk(av, p);

    /*
      If eligible, place chunk on a fastbin so it can be found
      and used quickly in malloc.
    */

    if ((CHUNK_SIZE_T)(size) <= (CHUNK_SIZE_T)(av->max_fast)

#if TRIM_FASTBINS
        /* 
           If TRIM_FASTBINS set, don't place chunks
           bordering top into fastbins
        */
        && (chunk_at_offset(p, size) != av->top)
#endif
        ) {

      set_fastchunks(av);
      fb = &(av->fastbins[fastbin_index(size)]);
      p->fd = *fb;
      *fb = p;
    }

    /*
       Consolidate other non-mmapped chunks as they arrive.
    */

    else if (!chunk_is_mmapped(p)) {
      set_anychunks(av);

      nextchunk = chunk_at_offset(p, size);
      nextsize = chunksize(nextchunk);

      /* consolidate backward */
      if (!prev_inuse(p)) {
        prevsize = p->prev_size;
        size += prevsize;
        p = chunk_at_offset(p, -((long) prevsize));
        unlink(p, bck, fwd);
      }

      if (nextchunk != av->top) {
        /* get and clear inuse bit */
        nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
        set_head(nextchunk, nextsize);

        /* consolidate forward */
        if (!nextinuse) {
          unlink(nextchunk, bck, fwd);
          size += nextsize;
        }

        /*
          Place the chunk in unsorted chunk list. Chunks are
          not placed into regular bins until after they have
          been given one chance to be used in malloc.
        */

        bck = unsorted_chunks(av);
        fwd = bck->fd;
        p->bk = bck;
        p->fd = fwd;
        bck->fd = p;
        fwd->bk = p;

        set_head(p, size | PREV_INUSE);
        set_foot(p, size);
        
        check_free_chunk(av, p);
      }

      /*
         If the chunk borders the current high end of memory,
         consolidate into top
      */

      else {
        size += nextsize;
        set_head(p, size | PREV_INUSE);
        av->top = p;
        check_chunk(av, p);
      }

      /*
        If freeing a large space, consolidate possibly-surrounding
        chunks. Then, if the total unused topmost memory exceeds trim
        threshold, ask malloc_trim to reduce top.

        Unless max_fast is 0, we don't know if there are fastbins
        bordering top, so we cannot tell for sure whether threshold
        has been reached unless fastbins are consolidated.  But we
        don't want to consolidate on each free.  As a compromise,
        consolidation is performed if FASTBIN_CONSOLIDATION_THRESHOLD
        is reached.
      */

      if ((CHUNK_SIZE_T)(size) >= FASTBIN_CONSOLIDATION_THRESHOLD) { 
        if (have_fastchunks(av)) 
          malloc_consolidate(av);
/*
#ifndef MORECORE_CANNOT_TRIM        
        if ((CHUNK_SIZE_T)(chunksize(av->top)) >= 
            (CHUNK_SIZE_T)(av->trim_threshold))
          sYSTRIm(av->top_pad, av);
#endif*/
      }

    }
    /*
      If the chunk was allocated via mmap, release via munmap()
      Note that if HAVE_MMAP is false but chunk_is_mmapped is
      true, then user must have overwritten memory. There's nothing
      we can do to catch this error unless DEBUG is set, in which case
      check_inuse_chunk (above) will have triggered error.
    */

    else {
    }
  }
}

/*
  ------------------------- malloc_consolidate -------------------------

  malloc_consolidate is a specialized version of free() that tears
  down chunks held in fastbins.  Free itself cannot be used for this
  purpose since, among other things, it might place chunks back onto
  fastbins.  So, instead, we need to use a minor variant of the same
  code.
  
  Also, because this routine needs to be called the first time through
  malloc anyway, it turns out to be the perfect place to trigger
  initialization code.
*/

void Heap::malloc_consolidate(mstate av)
{
	mfastbinptr*    fb;                 /* current fastbin being consolidated */
	mfastbinptr*    maxfb;              /* last fastbin (for loop control) */
	mchunkptr       p;                  /* current chunk being consolidated */
	mchunkptr       nextp;              /* next chunk to consolidate */
	mchunkptr       unsorted_bin;       /* bin header */
	mchunkptr       first_unsorted;     /* chunk to link to */

	/* These have same use as in free() */
	mchunkptr       nextchunk;
	INTERNAL_SIZE_T size;
	INTERNAL_SIZE_T nextsize;
	INTERNAL_SIZE_T prevsize;
	int             nextinuse;
	mchunkptr       bck;
	mchunkptr       fwd;

	/* I assume we are always initilised first
	*/

	clear_fastchunks(av);

	unsorted_bin = unsorted_chunks(av);

	/*
		Remove each chunk from fast bin and consolidate it, placing it
		then in unsorted bin. Among other reasons for doing this,
		placing in unsorted bin avoids needing to calculate actual bins
		until malloc is sure that chunks aren't immediately going to be
		reused anyway.
	*/

	maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
	fb = &(av->fastbins[0]);
	do {
		if ( (p = *fb) != 0) {
		*fb = 0;
	    
		do {
			check_inuse_chunk(av, p);
			nextp = p->fd;
	        
			/* Slightly streamlined version of consolidation code in free() */
			size = p->size & ~PREV_INUSE;
			nextchunk = chunk_at_offset(p, size);
			nextsize = chunksize(nextchunk);
	        
			if (!prev_inuse(p)) {
			prevsize = p->prev_size;
			size += prevsize;
			p = chunk_at_offset(p, -((long) prevsize));
			unlink(p, bck, fwd);
			}
	        
			if (nextchunk != av->top) {
			nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
			set_head(nextchunk, nextsize);
	        
			if (!nextinuse) {
				size += nextsize;
				unlink(nextchunk, bck, fwd);
			}
	        
			first_unsorted = unsorted_bin->fd;
			unsorted_bin->fd = p;
			first_unsorted->bk = p;
	        
			set_head(p, size | PREV_INUSE);
			p->bk = unsorted_bin;
			p->fd = first_unsorted;
			set_foot(p, size);
			}
	        
			else {
			size += nextsize;
			set_head(p, size | PREV_INUSE);
			av->top = p;
			}
	        
		} while ( (p = nextp) != 0);
	    
		}
	} while (fb++ != maxfb);
}

//---------------------------------
//!
//!  memalign(size_t alignment, size_t n);
//!  Returns a pointer to a newly allocated chunk of n bytes, aligned
//!  in accord with the alignment argument.
//!
//!  The alignment argument should be a power of two. If the argument is
//!  not a power of two, the nearest greater power is used.
//!  16-byte alignment is guaranteed by normal malloc calls, so don't
//!  bother calling memalign with an argument of 16 or less.
//!
//!  Overreliance on memalign is a sure way to fragment space.
//!
//---------------------------------
void* Heap::Memalign(size_t alignment, size_t bytes)
{
	ScopedCritical crit( m_HeapCritical );

  INTERNAL_SIZE_T nb;             /* padded  request size */
  char*           m;              /* memory returned by malloc call */
  mchunkptr       p;              /* corresponding chunk */
  char*           brk;            /* alignment point within p */
  mchunkptr       newp;           /* chunk to return */
  INTERNAL_SIZE_T newsize;        /* its size */
  INTERNAL_SIZE_T leadsize;       /* leading space before alignment point */
  mchunkptr       remainder;      /* spare room at end to split off */
  CHUNK_SIZE_T    remainder_size; /* its size */
  INTERNAL_SIZE_T size;

  nb = 0;
  /* If need less alignment than we give anyway, just relay to malloc */

  if (alignment <= MALLOC_ALIGNMENT) return Malloc(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  if (alignment <  MINSIZE) alignment = MINSIZE;

  /* Make sure alignment is power of 2 (in case MINSIZE is not).  */
  if ((alignment & (alignment - 1)) != 0) {
    size_t a = MALLOC_ALIGNMENT * 2;
    while ((CHUNK_SIZE_T)a < (CHUNK_SIZE_T)alignment) a <<= 1;
    alignment = a;
  }

  checked_request2size(bytes, nb);

  /*
    Strategy: find a spot within that chunk that meets the alignment
    request, and then possibly free the leading and trailing space.
  */


  /* Call malloc with worst case padding to hit alignment. */

  m  = (char*)(Malloc(nb + alignment + MINSIZE));

  if (m == 0) return 0; /* propagate failure */

  p = mem2chunk(m);

  if ((((PTR_UINT)(m)) % alignment) != 0) { /* misaligned */

    /*
      Find an aligned spot inside chunk.  Since we need to give back
      leading space in a chunk of at least MINSIZE, if the first
      calculation places us at a spot with less than MINSIZE leader,
      we can move to the next aligned spot -- we've allocated enough
      total room so that this is always possible.
    */

    brk = (char*)mem2chunk((PTR_UINT)(((PTR_UINT)(m + alignment - 1)) &
                           -((signed long) alignment)));
    if ((CHUNK_SIZE_T)(brk - (char*)(p)) < MINSIZE)
      brk += alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (char*)(p);
    newsize = chunksize(p) - leadsize;

    /* For mmapped chunks, just adjust offset */
    if (chunk_is_mmapped(p)) {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      return chunk2mem(newp);
    }

    /* Otherwise, give back leader, use the rest */
    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    Free(chunk2mem(p));
    p = newp;

    ntAssert (newsize >= nb &&
            (((PTR_UINT)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */
  if (!chunk_is_mmapped(p)) {
    size = chunksize(p);
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_head_size(p, nb);
      Free(chunk2mem(remainder));
    }
  }

  check_inuse_chunk(m_pMallocState, p);
  return chunk2mem(p);
}


/*
  Debugging support

  These routines make a number of ntAssertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/
#if defined( _DEBUG )

/*
  Properties of all chunks
*/
void do_check_chunk(mstate av, mchunkptr p)
{
  CHUNK_SIZE_T  sz = chunksize(p);
  /* min and max possible addresses assuming contiguous allocation */
  char* max_address = (char*)(av->top) + chunksize(av->top);
  char* min_address = max_address - av->sbrked_mem;

  if (!chunk_is_mmapped(p)) {
    
    /* Has legal address ... */
    if (p != av->top) 
	{
        ntAssert(((char*)p) >= min_address);
        ntAssert(((char*)p + sz) <= ((char*)(av->top)));
    }
    else {
      /* top size is always at least MINSIZE */
      ntAssert((CHUNK_SIZE_T)(sz) >= MINSIZE);
      /* top predecessor always marked inuse */
      ntAssert(prev_inuse(p));
    }
      
  }
  else {
    /* force an appropriate ntAssert violation if debug set */
    ntAssert(!chunk_is_mmapped(p));
  }
}

/*
  Properties of free chunks
*/

void do_check_free_chunk(mstate av, mchunkptr p)
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(av, p);

  /* Chunk must claim to be free ... */
  ntAssert(!inuse(p));
  ntAssert (!chunk_is_mmapped(p));

  /* Unless a special marker, must have OK fields */
  if ((CHUNK_SIZE_T)(sz) >= MINSIZE)
  {
    ntAssert((sz & MALLOC_ALIGN_MASK) == 0);
    ntAssert(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ntAssert(next->prev_size == sz);
    /* ... and is fully consolidated */
    ntAssert(prev_inuse(p));
    ntAssert (next == av->top || inuse(next));

    /* ... and has minimally sane links */
    ntAssert(p->fd->bk == p);
    ntAssert(p->bk->fd == p);
  }
  else /* markers are always of size SIZE_SZ */
    ntAssert(sz == SIZE_SZ);
}

/*
  Properties of inuse chunks
*/
void do_check_inuse_chunk(mstate av, mchunkptr p)
{
  mchunkptr next;
  do_check_chunk(av, p);

  if (chunk_is_mmapped(p))
    return; /* mmapped chunks have no next/prev */

  /* Check whether it claims to be in use ... */
  ntAssert(inuse(p));

  next = next_chunk(p);

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  if (!prev_inuse(p))  {
    /* Note that we cannot even look at prev unless it is not inuse */
    mchunkptr prv = prev_chunk(p);
    ntAssert(next_chunk(prv) == p);
    do_check_free_chunk(av, prv);
  }

  if (next == av->top) {
    ntAssert(prev_inuse(next));
    ntAssert(chunksize(next) >= MINSIZE);
  }
  else if (!inuse(next))
    do_check_free_chunk(av, next);
}

/*
  Properties of chunks recycled from fastbins
*/

void do_check_remalloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s)
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;

  do_check_inuse_chunk(av, p);

  /* Legal size ... */
  ntAssert((sz & MALLOC_ALIGN_MASK) == 0);
  ntAssert((CHUNK_SIZE_T)(sz) >= MINSIZE);
  /* ... and alignment */
  ntAssert(aligned_OK(chunk2mem(p)));
  /* chunk is less than MINSIZE more than request */
  ntAssert((long)(sz) - (long)(s) >= 0);
  ntAssert((long)(sz) - (long)(s + MINSIZE) < 0);
}

/*
  Properties of nonrecycled chunks at the point they are malloced
*/
void do_check_malloced_chunk(mstate av, mchunkptr p, INTERNAL_SIZE_T s)
{
  /* same as recycled case ... */
  do_check_remalloced_chunk(av, p, s);

  /*
    ... plus,  must obey implementation invariant that prev_inuse is
    always true of any allocated chunk; i.e., that each allocated
    chunk borders either a previously allocated and still in-use
    chunk, or the base of its memory arena. This is ensured
    by making all allocations from the the `lowest' part of any found
    chunk.  This does not necessarily hold however for chunks
    recycled via fastbins.
  */

  ntAssert(prev_inuse(p));
}


/*
  Properties of malloc_state.

  This may be useful for debugging malloc, as well as detecting user
  programmer errors that somehow write into malloc_state.

  If you are extending or experimenting with this malloc, you can
  probably figure out how to hack this routine to print out or
  display chunk addresses, sizes, bins, and other instrumentation.
*/

void do_check_malloc_state( mstate av )
{
  mchunkptr p;
  mchunkptr q;
  mbinptr b;
  unsigned int binbit;
  int empty;
  unsigned int idx;
  INTERNAL_SIZE_T size;
  CHUNK_SIZE_T  total = 0;
  int max_fast_bin;

  /* internal size_t must be no wider than pointer type */
  ntAssert(sizeof(INTERNAL_SIZE_T) <= sizeof(char*));

  /* alignment is a power of 2 */
  ntAssert((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-1)) == 0);

  /* cannot run remaining checks until fully initialized */
  if (av->top == 0 || av->top == initial_top(av))
    return;


  /* properties of fastbins */

  /* max_fast is in allowed range */
  ntAssert(get_max_fast(av) <= request2size(MAX_FAST_SIZE));

  max_fast_bin = fastbin_index(av->max_fast);

  for (int i = 0; i < (int)NFASTBINS; ++i) {
    p = av->fastbins[i];

    /* all bins past max_fast are empty */
    if (i > max_fast_bin)
      ntAssert(p == 0);

    while (p != 0) {
      /* each chunk claims to be inuse */
      do_check_inuse_chunk(av, p);
      total += chunksize(p);
      /* chunk belongs in this bin */
      ntAssert( (int)fastbin_index(chunksize(p)) == i);
      p = p->fd;
    }
  }

  if (total != 0)
  {
    ntAssert(have_fastchunks(av));
  }
  else if (!have_fastchunks(av))
  {
    ntAssert(total == 0);
  }

  /* check normal bins */
  for (int i = 1; i < NBINS; ++i) {
    b = bin_at(av,i);

    /* binmap is accurate (except for bin 1 == unsorted_chunks) */
    if (i >= 2) {
      binbit = get_binmap(av,i);
      empty = last(b) == b;
      if (!binbit)
	  {
        ntAssert(empty);
	  }
      else if (!empty)
	  {
        ntAssert(binbit);
	  }
    }

    for (p = last(b); p != b; p = p->bk) {
      /* each chunk claims to be free */
      do_check_free_chunk(av, p);
      size = chunksize(p);
      total += size;
      if (i >= 2) {
        /* chunk belongs in bin */
        idx = bin_index(size);
        ntAssert( (int)idx == i);
        /* lists are sorted */
        if ((CHUNK_SIZE_T) size >= (CHUNK_SIZE_T)(FIRST_SORTED_BIN_SIZE)) {
          ntAssert(p->bk == b || 
                 (CHUNK_SIZE_T)chunksize(p->bk) >= 
                 (CHUNK_SIZE_T)chunksize(p));
        }
      }
      /* chunk is followed by a legal chain of inuse chunks */
      for (q = next_chunk(p);
           (q != av->top && inuse(q) && 
             (CHUNK_SIZE_T)(chunksize(q)) >= MINSIZE);
           q = next_chunk(q))
        do_check_inuse_chunk(av, q);
    }
  }

  /* top chunk is OK */
  check_chunk(av, av->top);

  /* sanity checks for statistics */

  ntAssert(total <= (CHUNK_SIZE_T)(av->max_total_mem));

  ntAssert((CHUNK_SIZE_T)(av->sbrked_mem) <=
         (CHUNK_SIZE_T)(av->max_sbrked_mem));

  ntAssert((CHUNK_SIZE_T)(av->mmapped_mem) <=
         (CHUNK_SIZE_T)(av->max_mmapped_mem));

  ntAssert((CHUNK_SIZE_T)(av->max_total_mem) >=
         (CHUNK_SIZE_T)(av->mmapped_mem) + (CHUNK_SIZE_T)(av->sbrked_mem));
}
#endif

/* ----------- Routines dealing with system allocation -------------- */

/*
  sysmalloc handles malloc cases requiring more memory from the system.
  On entry, it is assumed that av->top does not have enough
  space to service request for nb bytes, thus requiring that av->top
  be extended or replaced.
*/

void* Heap::sys_malloc(uint32_t nb, mstate av)
{
	const char* MORECORE_FAILURE = (const char*)-1;
  mchunkptr       old_top;        /* incoming value of av->top */
  INTERNAL_SIZE_T old_size;       /* its size */
  char*           old_end;        /* its end address */

  long            size;           /* arg to first MORECORE or mmap call */
  char*           brk;            /* return value from MORECORE */

  long            correction;     /* arg to 2nd MORECORE call */
  char*           snd_brk;        /* 2nd return val */

  INTERNAL_SIZE_T front_misalign; /* unusable bytes at front of new space */
  INTERNAL_SIZE_T end_misalign;   /* partial page left at end of new space */
  char*           aligned_brk;    /* aligned offset into brk */

  mchunkptr       p;              /* the allocated/returned chunk */
  mchunkptr       remainder;      /* remainder from allocation */
  CHUNK_SIZE_T    remainder_size; /* its size */

  CHUNK_SIZE_T    sum;            /* for updating stats */

  size_t          pagemask  = av->pagesize - 1;

  /*
    If there is space available in fastbins, consolidate and retry
    malloc from scratch rather than getting memory from system.  This
    can occur only if nb is in smallbin range so we didn't consolidate
    upon entry to malloc. It is much easier to handle this case here
    than in malloc proper.
  */

  if (have_fastchunks(av)) {
    assert(in_smallbin_range(nb));
    malloc_consolidate(av);
	return Heap::Malloc(nb - MALLOC_ALIGN_MASK);
  }

  /* Record incoming configuration of top */

  old_top  = av->top;
  old_size = chunksize(old_top);
  old_end  = (char*)(chunk_at_offset(old_top, old_size));

  brk = snd_brk = (char*)(MORECORE_FAILURE); 

  /* 
     If not the first time through, we require old_size to be
     at least MINSIZE and to have prev_inuse set.
  */

  assert((old_top == initial_top(av) && old_size == 0) || 
         ((CHUNK_SIZE_T) (old_size) >= MINSIZE &&
          prev_inuse(old_top)));

  /* Precondition: not enough current space to satisfy nb request */
  assert((CHUNK_SIZE_T)(old_size) < (CHUNK_SIZE_T)(nb + MINSIZE));

  /* Precondition: all fastbins are consolidated */
  assert(!have_fastchunks(av));


  /* Request enough space for nb + pad + overhead */

  size = nb + MINSIZE;

  /*
    If contiguous, we can subtract out existing space that we hope to
    combine with new space. We add it back later only if
    we don't actually get contiguous space.
  */

  if (contiguous(av))
    size -= old_size;

  /*
    Round to a multiple of page size.
    If MORECORE is not contiguous, this ensures that we only call it
    with whole-page arguments.  And if MORECORE is contiguous and
    this is not first time through, this preserves page-alignment of
    previous calls. Otherwise, we correct to page-align below.
  */

  size = (size + pagemask) & ~pagemask;

  /*
    Don't try to call MORECORE if argument is so big as to appear
    negative. Note that since mmap takes size_t arg, it may succeed
    below even if we cannot call MORECORE.
  */

  if (size > 0) 
  {
	  if( (m_pCurSysHigh+size) < m_pMemTop )
	  {
		  brk = m_pCurSysHigh;
		  m_pCurSysHigh += size;
	  } else
	  {
		  brk = (char*)MORECORE_FAILURE;
	  }
  }

  if (brk != (char*)(MORECORE_FAILURE)) 
  {
    av->sbrked_mem += size;

    /*
      If MORECORE extends previous space, we can likewise extend top size.
    */
    
    if (brk == old_end && snd_brk == (char*)(MORECORE_FAILURE)) {
      set_head(old_top, (size + old_size) | PREV_INUSE);
    }

    /*
      Otherwise, make adjustments:
      
      * If the first time through or noncontiguous, we need to call sbrk
        just to find out where the end of memory lies.

      * We need to ensure that all returned chunks from malloc will meet
        MALLOC_ALIGNMENT

      * If there was an intervening foreign sbrk, we need to adjust sbrk
        request size to account for fact that we will not be able to
        combine new space with existing space in old_top.

      * Almost all systems internally allocate whole pages at a time, in
        which case we might as well use the whole last page of request.
        So we allocate enough more memory to hit a page boundary now,
        which in turn causes future contiguous calls to page-align.
    */
    
    else {
      front_misalign = 0;
      end_misalign = 0;
      correction = 0;
      aligned_brk = brk;

      /*
        If MORECORE returns an address lower than we have seen before,
        we know it isn't really contiguous.  This and some subsequent
        checks help cope with non-conforming MORECORE functions and
        the presence of "foreign" calls to MORECORE from outside of
        malloc or by other threads.  We cannot guarantee to detect
        these in all cases, but cope with the ones we do detect.
      */
      if (contiguous(av) && old_size != 0 && brk < old_end) {
        set_noncontiguous(av);
      }
      
      /* handle contiguous cases */
      if (contiguous(av)) { 

        /* 
           We can tolerate forward non-contiguities here (usually due
           to foreign calls) but treat them as part of our space for
           stats reporting.
        */
        if (old_size != 0) 
          av->sbrked_mem += brk - old_end;
        
        /* Guarantee alignment of first new chunk made from this space */

        front_misalign = (((INTERNAL_SIZE_T)chunk2mem(brk)) & MALLOC_ALIGN_MASK);
        if (front_misalign > 0) {

          /*
            Skip over some bytes to arrive at an aligned position.
            We don't need to specially mark these wasted front bytes.
            They will never be accessed anyway because
            prev_inuse of av->top (and any chunk created from its start)
            is always true after initialization.
          */

          correction = MALLOC_ALIGNMENT - front_misalign;
          aligned_brk += correction;
        }
        
        /*
          If this isn't adjacent to existing space, then we will not
          be able to merge with old_top space, so must add to 2nd request.
        */
        
        correction += old_size;
        
        /* Extend the end address to hit a page boundary */
        end_misalign = (INTERNAL_SIZE_T)(brk + size + correction);
        correction += ((end_misalign + pagemask) & ~pagemask) - end_misalign;
        
        assert(correction >= 0);
		if( (m_pCurSysHigh+correction) < m_pMemTop )
		{
			snd_brk = m_pCurSysHigh;
			m_pCurSysHigh += correction;
		} else
		{
			snd_brk = (char*)MORECORE_FAILURE;
		}

        
        if (snd_brk == (char*)(MORECORE_FAILURE)) {
          /*
            If can't allocate correction, try to at least find out current
            brk.  It might be enough to proceed without failing.
          */
          correction = 0;
          snd_brk = (char*)(m_pCurSysHigh);
        }
        else if (snd_brk < brk) {
          /*
            If the second call gives noncontiguous space even though
            it says it won't, the only course of action is to ignore
            results of second call, and conservatively estimate where
            the first call left us. Also set noncontiguous, so this
            won't happen again, leaving at most one hole.
            
            Note that this check is intrinsically incomplete.  Because
            MORECORE is allowed to give more space than we ask for,
            there is no reliable way to detect a noncontiguity
            producing a forward gap for the second call.
          */
          snd_brk = brk + size;
          correction = 0;
          set_noncontiguous(av);
        }

      }
      
      /* handle non-contiguous cases */
      else { 
        /* MORECORE/mmap must correctly align */
        assert(aligned_OK(chunk2mem(brk)));
        
        /* Find out current end of memory */
        if (snd_brk == (char*)(MORECORE_FAILURE)) 
		{
          snd_brk = m_pCurSysHigh;
          av->sbrked_mem += snd_brk - brk - size;
        }
      }
      
      /* Adjust top based on results of second sbrk */
      if (snd_brk != (char*)(MORECORE_FAILURE)) {
        av->top = (mchunkptr)aligned_brk;
        set_head(av->top, (snd_brk - aligned_brk + correction) | PREV_INUSE);
        av->sbrked_mem += correction;
     
        /*
          If not the first time through, we either have a
          gap due to foreign sbrk or a non-contiguous region.  Insert a
          double fencepost at old_top to prevent consolidation with space
          we don't own. These fenceposts are artificial chunks that are
          marked as inuse and are in any case too small to use.  We need
          two to make sizes and alignments work out.
        */
   
        if (old_size != 0) {
          /* 
             Shrink old_top to insert fenceposts, keeping size a
             multiple of MALLOC_ALIGNMENT. We know there is at least
             enough space in old_top to do this.
          */
          old_size = (old_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
          set_head(old_top, old_size | PREV_INUSE);
          
          /*
            Note that the following assignments completely overwrite
            old_top when old_size was previously MINSIZE.  This is
            intentional. We need the fencepost, even if old_top otherwise gets
            lost.
          */
          chunk_at_offset(old_top, old_size          )->size =
            SIZE_SZ|PREV_INUSE;

          chunk_at_offset(old_top, old_size + SIZE_SZ)->size =
            SIZE_SZ|PREV_INUSE;

          /* 
             If possible, release the rest, suppressing trimming.
          */
          if (old_size >= MINSIZE) {
            INTERNAL_SIZE_T tt = av->trim_threshold;
            av->trim_threshold = (INTERNAL_SIZE_T)(-1);
            free(chunk2mem(old_top));
            av->trim_threshold = tt;
          }
        }
      }
    }
    
    /* Update statistics */
    sum = av->sbrked_mem;
    if (sum > (CHUNK_SIZE_T)(av->max_sbrked_mem))
      av->max_sbrked_mem = sum;
    
    sum += av->mmapped_mem;
    if (sum > (CHUNK_SIZE_T)(av->max_total_mem))
      av->max_total_mem = sum;

    check_malloc_state(av);
    
    /* finally, do the allocation */

    p = av->top;
    size = chunksize(p);
    
    /* check that one of the above allocation paths succeeded */
    if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      av->top = remainder;
      set_head(p, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      check_malloced_chunk(av, p, nb);
      return chunk2mem(p);
    }

  }

  /* catch all failure paths */
  return 0;
}

