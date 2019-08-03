/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icemeshinternal.h"

#ifndef __SPU__
void CopyBytes( void* pDst, const void* pSrc, const U32 numBytes )
{
	memcpy(pDst, pSrc, numBytes);
}
#endif

#ifndef __SPU__
void CopyQWords( void* pDst, const void* pSrc, const U32 numBytes )
{
	ICE_ASSERT(((U32)pDst & 0xF) == 0);
	ICE_ASSERT(((U32)pSrc & 0xF) == 0);
	ICE_ASSERT((numBytes & 0xF) == 0);

	VU8 * __restrict__ pBigDst = (VU8 *)pDst;
	VU8 const *pBigSrc = (VU8 *)pSrc;
	for (U64 ii = 0; ii < numBytes; ii += 16) 
	{
		*pBigDst = *pBigSrc;
		pBigDst++;
		pBigSrc++;
	}
}
#endif

