/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_MESH_DMA
#define ICE_MESH_DMA

#include <spu_intrinsics.h>
#include "icedma.h"

#include "jobapi/jobdefinition.h"
#include "jobapi/jobspudma.h"
#include "jobapi/jobapi.h"

static inline void Getllar(U32 ls, U32 eal) __attribute__((always_inline));
static inline U32 Putllc(U32 ls, U32 eal) __attribute__((always_inline));
static inline void Putlluc(U32 ls, U32 eal) __attribute__((always_inline));
static inline void Putqlluc(U32 ls, U32 eal, U32 tagId) __attribute__((always_inline));
static inline void StartDma(volatile void *ls, U32 eal, U32 size, U32 tagid, U32 cmd) __attribute__((always_inline));
static inline void WaitOnDma(U32 tagMask) __attribute__((always_inline));
static inline U32 TestDmaDone(U32 tagMask) __attribute__((always_inline));

// Get link line and reserve
static inline void Getllar(U32 ls, U32 eal)
{
	spu_idisable();
	spu_writech(MFC_LSA, ls);
	spu_writech(MFC_EAL, eal);
	spu_writech(MFC_Cmd, Ice::kDmaGetllar);
	spu_readch(MFC_RdAtomicStat);                           // don't care about the result, just that it executes
	spu_ienable();
}

// Put link line conditional
// Returns 0 if successful, 1 if not
static inline U32 Putllc(U32 ls, U32 eal)
{
	spu_idisable();
	spu_writech(MFC_LSA, ls);
	spu_writech(MFC_EAL, eal);
	spu_writech(MFC_Cmd, Ice::kDmaPutllc);
	U32 result = spu_readch(MFC_RdAtomicStat);                   // mask just the bit that corresponds to putllc success
	spu_ienable();

	return result & 1;
}

// Put link line unconditional
// Use this for unconditional output to something which is typically atomic.  This has a better chance of keeping the
// data in cache than a simple PUT.
static inline void Putlluc(U32 ls, U32 eal)
{
	spu_idisable();
	spu_writech(MFC_LSA, ls);
	spu_writech(MFC_EAL, eal);
	spu_writech(MFC_Cmd, Ice::kDmaPutlluc);
	spu_readch(MFC_RdAtomicStat);                           // don't care about the result, just that it executes
	spu_ienable();
}

// Put link line unconditional queued
// Use this for unconditional output to something which is typically atomic.  This has a better chance of keeping the
// data in cache than a simple PUT.
static inline void Putqlluc(U32 ls, U32 eal, U32 tagId)
{
	spu_idisable();
	spu_writech(MFC_LSA, ls);
	spu_writech(MFC_EAL, eal);
	spu_writech(MFC_TagID, tagId);
	spu_writech(MFC_Cmd, Ice::kDmaPutqlluc);
	spu_ienable();
}

static inline void StartDma(volatile void *ls, U32 eal, U32 size, U32 tagId, U32 cmd)
{
	spu_idisable();
	spu_writech(MFC_LSA, (U32)ls);
	spu_writech(MFC_EAL, eal);
	spu_writech(MFC_Size, size);
	spu_writech(MFC_TagID, tagId);
	spu_writech(MFC_Cmd, cmd);
	spu_ienable();
}

// Wait until all DMA using the tag groups in the tag mask have finished.
static inline void WaitOnDma(U32 tagMask)
{
	U32 status;

	do {
		spu_idisable();
		spu_writech(MFC_WrTagMask, tagMask);
		spu_writech(MFC_WrTagUpdate, 0);
		status = spu_readch(MFC_RdTagStat);
		spu_ienable();
	} while (status != tagMask);
}

// Test if all DMA tag groups in the tagMask are finished.
static inline U32 TestDmaDone(U32 tagMask)
{
	spu_idisable();
	spu_writech(MFC_WrTagMask, tagMask);
	spu_writech(MFC_WrTagUpdate, 0);
	U32 result = spu_readch(MFC_RdTagStat) == tagMask;
	spu_ienable();

	return result;
}

#endif

