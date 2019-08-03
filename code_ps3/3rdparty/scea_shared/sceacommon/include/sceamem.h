/*
    sceamem.h

    SCEA memory management API.

    See the sceamem directory for documentation and samples.

    Note that to keep the file size and complexity down, there
    is an equivalent sceamem.h file in sceamem/doc/headers/sceamem.h
    that contains the embedded Doxygen comments.  Be sure to keep it
    in sync with this file.

    Please also note the use of spaces instead of tabs.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005.  All rights reserved.
*/

#ifndef SCEAMEM_H
#define SCEAMEM_H 1

    /* Include sharing initiative definitions. */
#include "sceatargetmacros.h"
#include "sceabasetypes.h"

#if SCEA_HOST_COMPILER_MSVC
#pragma once
#endif

namespace SCEA {
    namespace Memory {

typedef U32 MemSize;    // Memory size type.


typedef U32 MemFlags;   // Allocator flags type.  Or together 0 or 1 each from MemAlign,
                        // MemOption, PoolID, or MemPurpose.

    // Memory allocator API.
class Allocator
{
public:
        // Allocate memory.
    virtual void* Allocate(MemSize size, MemFlags flags,
        const char* pFile = 0, int line = 0) = 0;

        // Deallocate memory.
    virtual void Deallocate(void* pMemory, MemFlags flags,
        const char* pFile = 0, int line = 0) = 0;

        // Reallocate memory.
    virtual void* Reallocate(void* pMemory, MemSize newSize,
        MemFlags flags, const char* pFile = 0, int line = 0) = 0;

#if !SCEA_TARGET_OS_WIN32
    Allocator() {}              // Avoid warnings on non-win32 platforms.
    virtual ~Allocator() {}
#endif
};

#if MEMORY_TRANSPARENT_ANNOTATION
    #define SCEAMEM_ANNOTATION __FILE__, __LINE__
#else
    #define SCEAMEM_ANNOTATION 0, 0
#endif

    // Default allocator allocation flags.
const MemFlags kMemFlagsDefault = 0;

    // Alignment field type. It is the exponent base two of the
    // alignment, i.e.:  Alignment = 2 ^ value.
enum MemAlign
{
    kMemAlignDefault            = 0,
    kMemAlignShift              = 0,
    kMemAlignMask               = (0x1fU << kMemAlignShift),
    kMemAlign8                  = (0x03U << kMemAlignShift),
    kMemAlign16                 = (0x04U << kMemAlignShift),
    kMemAlign32                 = (0x05U << kMemAlignShift),
    kMemAlign64                 = (0x06U << kMemAlignShift),
    kMemAlign128                = (0x07U << kMemAlignShift),
    kMemAlign256                = (0x08U << kMemAlignShift),
    kMemAlign512                = (0x09U << kMemAlignShift),
    kMemAlign1k                 = (0x0aU << kMemAlignShift),
    kMemAlign2k                 = (0x0bU << kMemAlignShift),
    kMemAlign4k                 = (0x0cU << kMemAlignShift),
    kMemAlign8k                 = (0x0dU << kMemAlignShift),
    kMemAlign16k                = (0x0eU << kMemAlignShift),
    kMemAlign32k                = (0x0fU << kMemAlignShift),
    kMemAlign64k                = (0x10U << kMemAlignShift),
    kMemAlign128k               = (0x11U << kMemAlignShift),
    kMemAlign256k               = (0x12U << kMemAlignShift),
    kMemAlign512k               = (0x13U << kMemAlignShift),
    kMemAlign1Meg               = (0x14U << kMemAlignShift),
    kMemAlignVector             = kMemAlign16,
    kMemAlignCacheLine          = kMemAlign128
};

    // Extract alignment from flags.
#define MEM_ALIGN_FROM_FLAGS(flags) \
    (1 << (((flags) & SCEA::Memory::kMemAlignMask) >> SCEA::Memory::kMemAlignShift))

    // Memory manager option field flag constants.
enum MemOption
{
    kMemOptionDefault            = 0x00,
    kMemOptionShift              = 5,
    kMemOptionMask               = (0x07U << kMemOptionShift),

        // Select high end of arena in two-sided heap.
    kMemOptionHighMemory         = (0x01U << kMemOptionShift),

        // These are available for client usage
    kMemOptionFlag2              = (0x02U << kMemOptionShift),
    kMemOptionFlag3              = (0x04U << kMemOptionShift),
};

    // Memory pool ID field constants.  Selects which pool.
enum PoolID
{
    kPoolIDDefault            = 0U,
    kPoolIDShift              = 8U,
    kPoolIDMask               = (0xFFU << kPoolIDShift),
    kPoolIDMaximumIDs         = 0x100U,

    // The following IDs indicate allocators described by allocation lifetime
    // for the purpose of reducing fragmentation.

        // Long term application data.
    kPoolIDLongTerm           = (0x01U << kPoolIDShift),
        // I.e. level data.
    kPoolIDMediumTerm         = (0x02U << kPoolIDShift),
        // I.e. mid-level data.
    kPoolIDShortTerm          = (0x03U << kPoolIDShift),
        // I.e. during frame only.
    kPoolIDFrameTerm          = (0x04U << kPoolIDShift),

    // Special memory pools.

    kPoolIDScratchpad         = (0x05U << kPoolIDShift),
    kPoolIDAGP                = (0x06U << kPoolIDShift),
    kPoolIDPhysical           = (0x07U << kPoolIDShift),
    kPoolIDOverflow           = (0x08U << kPoolIDShift),
    kPoolIDDebug              = (0x09U << kPoolIDShift),

    // Reserved IDs.
    kPoolIDReservedFirst      = (0x0AU << kPoolIDShift),
    kPoolIDReservedLast       = (0x0FU << kPoolIDShift),

    // Available for runtime use.
    kPoolIDRuntimeFirst       = (0x10U << kPoolIDShift),
    kPoolIDRuntimeLast        = (0x7FU << kPoolIDShift),

    // Available for client use.
    kPoolIDUserFirst          = (0x80U << kPoolIDShift),
    kPoolIDUserLast           = (0xFFU << kPoolIDShift)
};

    // Extract memory pool ID from flags.
#define POOL_ID_FROM_FLAGS(flags) ((flags) & SCEA::Memory::kPoolIDMask)

    // Extract memory pool index from flags.
#define POOL_INDEX_FROM_FLAGS(flags) (((flags) & SCEA::Memory::kPoolIDMask) >> SCEA::Memory::kPoolIDShift)

    // Memory purpose field constants.  Describes usage of memory.
    // You can submit your own definitions for us to include here.
enum MemPurpose
{
    kMemPurposeDefault                          = 0U,
    kMemPurposeShift                            = 16U,
    kMemPurposeMask                             = (0xffffU << kMemPurposeShift),

    //----------------------------------------------------------------------------------
    // Block of IDs reserved for audio.
    // Audio group will manage allocation of IDs within this block

    kMemPurposeSoundFirst                       = (0x0001U << kMemPurposeShift),

    // scream - these are synchronized with legacy values in scream.h
    kMemPurposeSoundScreamFirst                 = (0x0001U << kMemPurposeShift),
    kMemPurposeSoundScreamBankHeader            = (0x0001U << kMemPurposeShift),
    kMemPurposeSoundScreamBankHdrEmbeddedMMD    = (0x0002U << kMemPurposeShift),
    kMemPurposeSoundScreamMMD                   = (0x0003U << kMemPurposeShift),
    kMemPurposeSoundScreamVAGStreamHeaders      = (0x0004U << kMemPurposeShift),
    kMemPurposeSoundScreamVAGStreamBuffers      = (0x0005U << kMemPurposeShift),
    kMemPurposeSoundScreamMovieSound            = (0x0006U << kMemPurposeShift),
    kMemPurposeSoundScreamRequestFromEEToSnd    = (0x0007U << kMemPurposeShift),
    kMemPurposeSoundScreamScratchMemory         = (0x0008U << kMemPurposeShift),
    kMemPurposeSoundScreamBankBody              = (0x000aU << kMemPurposeShift),
    kMemPurposeSoundScreamBank                  = (0x000bU << kMemPurposeShift),
    kMemPurposeSoundScreamLast                  = (0x01FFU << kMemPurposeShift),

    // snd_stream - these are synchronized with hard values in snd_stream.h
    kMemPurposeSoundSndStreamFirst              = (0x0200U << kMemPurposeShift),
        // snd_stream purposes are defined in snd_stream/include/snd_stream.h
    kMemPurposeSoundSndStreamLast               = (0x021FU << kMemPurposeShift),

    // skat
    kMemPurposeSoundSKATFirst                   = (0x0220U << kMemPurposeShift),
        // skat purposes are defined in skat/include/skat.h
    kMemPurposeSoundSKATLast                    = (0x023FU << kMemPurposeShift),

    // crossfader
    kMemPurposeSoundCrossfaderFirst             = (0x0240U << kMemPurposeShift),
        // crossfader purposes are defined in crossfader/include/crossfader.h
    kMemPurposeSoundCrossfaderLast              = (0x025FU << kMemPurposeShift),

    // slink
    kMemPurposeSoundSlinkFirst                  = (0x0260U << kMemPurposeShift),
        // slink purposes are defined in slink/include/slink.h
    kMemPurposeSoundSlinkLast                   = (0x027FU << kMemPurposeShift),

    kMemPurposeSoundLast                        = (0x0FFFU << kMemPurposeShift),

    //----------------------------------------------------------------------------------

    // Block of IDs reserved for runtime group.
    // Runtime group will manage IDs within this block
    kMemPurposeRuntimeFirst                     = (0x1000U << kMemPurposeShift),
    kMemPurposeFIOSFirst                        = (0x1000U << kMemPurposeShift),
        // FIOS purposes are defined in fios/include/fios_sceamem.h
    kMemPurposeFIOSLast                         = (0x10FFU << kMemPurposeShift),
        // MP3 purposes.
    kMemPurposeMP3First                         = (0x1100U << kMemPurposeShift),
    kMemPurposeMP3Last                          = (0x117FU << kMemPurposeShift),

    kMemPurposeRuntimeLast                      = (0x1FFFU << kMemPurposeShift),

    //----------------------------------------------------------------------------------

    // Etc...ice? scee? scert?

    //----------------------------------------------------------------------------------

    // Block of IDs reserved for games to use
    // (to be defined in their own headers somewhere)
    kMemPurposeUserFirst                        = (0xE000U << kMemPurposeShift),
    kMemPurposeUserLast                         = (0xFFFFU << kMemPurposeShift)
};

    // Extract memory purpose from flags.
#define MEM_PURPOSE_FROM_FLAGS(flags) ((flags) & SCEA::Memory::kMemPurposeMask)

    // Convenience structure for game to query component of memory requirements.
struct Allocation
{
    MemFlags m_flags;       // Specify pool ID and alignment.
    MemSize m_size;         // Arena size.
};

    } // namespace Memory
} // namespace SCEA

#endif // SCEAMEM_H
