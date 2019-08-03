/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_UTIL_H
#define ICE_UTIL_H


namespace Ice
{

	template <typename type> inline type *AlignPointer(type *ptr, U32 align)
	{
		unsigned long addr = (unsigned long) ptr;
		addr = (addr + (align - 1)) & ~(align - 1);
		return ((type *) addr);
	}
	
	
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

}


#endif
