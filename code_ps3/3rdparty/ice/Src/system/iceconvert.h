/*
* Copyright (c) 2003-2005 Naughty Dog, Inc. 
* A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
* Use and distribution without consent strictly prohibited
*/

#ifndef ICE_CONVERT_WIN32
#define ICE_CONVERT_WIN32

namespace Ice
{
	inline void Reverse(I16 *addr)
	{
		unsigned long x = *reinterpret_cast<U16 *>(addr);
		*reinterpret_cast<I16 *>(addr) = (I16) ((x << 8) | (x >> 8));
	}

	inline void Reverse(U16 *addr)
	{
		unsigned long x = *addr;
		*addr = (U16) ((x << 8) | (x >> 8));
	}

	inline void Reverse(I32 *addr)
	{
		unsigned long x = *reinterpret_cast<U32 *>(addr);
		*addr = (I32) ((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24));
	}

	inline void Reverse(U32 *addr)
	{
		U32 x = *addr;
		*addr = (x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24);
	}

	inline void Reverse(float *addr)
	{
		U32 x = *reinterpret_cast<U32 *>(addr);
		*reinterpret_cast<U32 *>(addr) = (x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24);
	}

	inline void Reverse(I64 *addr)
	{
		U32 * a32 = reinterpret_cast<U32 *>(addr);
		// reverse two 32 bit values
		Reverse(a32);
		Reverse(a32+1);
		// swap
		U32 a0 = a32[0];
		U32 a1 = a32[1];
		a32[0] = a1;
		a32[1] = a0;
	}

	inline void Reverse(U64 *addr)
	{
		U32 * a32 = reinterpret_cast<U32 *>(addr);
		// reverse two 32 bit values
		Reverse(a32);
		Reverse(a32+1);
		// swap
		U32 a0 = a32[0];
		U32 a1 = a32[1];
		a32[0] = a1;
		a32[1] = a0;
	}
}

#endif

