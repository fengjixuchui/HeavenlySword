/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icebase.h"

#ifndef ICE_MEMCOPY_H
#define ICE_MEMCOPY_H

#if ICE_ARCH_POWERPC
inline void CopyQuadwords(void * dst, const void * src, U64 size)
{
	if(size == 0)
		return;

	VU8 * __restrict__ d = (VU8*)dst;
	const VU8 * __restrict__ s = (const VU8*)src;

	U64 qsize = size >> 4;
	U64 i;
	if (qsize < 0x8) 
	{
		i = 0; 
	}
	else 
	{ 
		i = (qsize-1) >> 3;
		qsize &= 0x7;
	}

	switch (qsize)
	{
	case 0x0:	*d++ = *s++;
	case 0x7:	*d++ = *s++;
	case 0x6:	*d++ = *s++;
	case 0x5:	*d++ = *s++;
	case 0x4:	*d++ = *s++;
	case 0x3:	*d++ = *s++;
	case 0x2:	*d++ = *s++;
	case 0x1:	*d++ = *s++;
	}

	for(;LIKELY(i > 0); --i) 
	{
		d[0] = s[0];
		d[1] = s[1];
		d[2] = s[2];
		d[3] = s[3];
		d[4] = s[4];
		d[5] = s[5];
		d[6] = s[6];
		d[7] = s[7];
		d += 8;
		s += 8;
	}
}

inline void CopyDoublewords(void * __restrict__ dst, const void * __restrict__ src, U64 size)
{
	if(size == 0)
		return;

	U64 srco = U32(src) & 0xF;
	U64 dsto = U32(dst) & 0xF;
	if(srco == dsto)
	{
		if(srco == 8)
		{
			*(U64*)dst = *(U64*)src;
			dst = (void *)((U8*)dst + 8);
			src = (const void *)((U8*)src + 8);
			size -= 8;
		}
		
		VU8 * __restrict__ d = (VU8*)dst;
		const VU8 * __restrict__ s = (const VU8*)src;

		U64 qsize = size >> 4;
		U64 i;
		if (qsize < 0x8) 
		{
			i = 0; 
		}
		else 
		{ 
			i = (qsize-1) >> 3;
			qsize &= 0x7;
		}
		
		switch (qsize)
		{
		case 0x0:	*d++ = *s++;
		case 0x7:	*d++ = *s++;
		case 0x6:	*d++ = *s++;
		case 0x5:	*d++ = *s++;
		case 0x4:	*d++ = *s++;
		case 0x3:	*d++ = *s++;
		case 0x2:	*d++ = *s++;
		case 0x1:	*d++ = *s++;
		}
		
		for(;LIKELY(i > 0); --i) 
		{
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
			d[4] = s[4];
			d[5] = s[5];
			d[6] = s[6];
			d[7] = s[7];
			d += 8;
			s += 8;
		}
		
		if(size & 0x8)
		{
			*(U64*)d = *(U64*)s;
		}
	}
	else
	{
		U64 * __restrict__ d = (U64*)dst;
		const U64 * __restrict__ s = (const U64*)src;

		size >>= 3;
		U64 i;
		if (size < 0x10) 
		{
			i = 0; 
		}
		else 
		{ 
			i = (size-1) >> 4;
			size &= 0xF;
		}
		
		switch (size)
		{
		case 0x0:	*d++ = *s++;
		case 0xF:	*d++ = *s++;
		case 0xE:	*d++ = *s++;
		case 0xD:	*d++ = *s++;
		case 0xC:	*d++ = *s++;
		case 0xB:	*d++ = *s++;
		case 0xA:	*d++ = *s++;
		case 0x9:	*d++ = *s++;
		case 0x8:	*d++ = *s++;
		case 0x7:	*d++ = *s++;
		case 0x6:	*d++ = *s++;
		case 0x5:	*d++ = *s++;
		case 0x4:	*d++ = *s++;
		case 0x3:	*d++ = *s++;
		case 0x2:	*d++ = *s++;
		case 0x1:	*d++ = *s++;
		}
		
		for(;LIKELY(i > 0); --i) 
		{
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
			d[4] = s[4];
			d[5] = s[5];
			d[6] = s[6];
			d[7] = s[7];
			d[8] = s[8];
			d[9] = s[9];
			d[10] = s[10];
			d[11] = s[11];
			d[12] = s[12];
			d[13] = s[13];
			d[14] = s[14];
			d[15] = s[15];
			d += 16;
			s += 16;
		}
	}
}
#elif ICE_ARCH_X86 && ICE_COMPILER_VISUALC
inline void CopyQuadwords(void * dst, const void * src, U64 size)
{
	U64 * d = (U64*)dst;
	const U64 * s = (const U64*)src;

	size = (size + 0xF) >> 4;
	if(size > 0)
	{
		do
		{
			*d++ = *s++;
			*d++ = *s++;
		} while(--size);
	}
}

inline void CopyDoublewords(void * dst, const void * src, U64 size)
{
	U64 * d = (U64*)dst;
	const U64 * s = (const U64*)src;

	size = (size + 0x7) >> 3;
	if(size > 0)
	{
		do
		{
			*d++ = *s++;
		} while(--size);
	}
}
#else
inline void CopyQuadwords(void * dst, const void * src, U64 size)
{
	U64 * __restrict__ d = (U64*)dst;
	const U64 * __restrict__ s = (const U64*)src;

	size = (size + 0xF) >> 4;
	if(size > 0)
	{
		do
		{
			*d++ = *s++;
			*d++ = *s++;
		} while(LIKELY(--size));
	}
}

inline void CopyDoublewords(void * dst, const void * src, U64 size)
{
	U64 * __restrict__ d = (U64*)dst;
	const U64 * __restrict__ s = (const U64*)src;

	size = (size + 0x7) >> 3;
	if(size > 0)
	{
		do
		{
			*d++ = *s++;
		} while(LIKELY(--size));
	}
}
#endif

#endif // ICE_MEMCOPY_H
