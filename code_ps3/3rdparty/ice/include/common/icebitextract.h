/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icebase.h"

#ifndef ICE_BITEXTRACT_H
#define ICE_BITEXTRACT_H

namespace Ice 
{
#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC

	/*!
	 * Rlwinm is the Rotate Left Word Immediate Then AND with Mask instruction.
	 * It logically ANDs a generated mask with the result of rotating left by a specified 
	 * number of bits in the contents of a general-purpose register.
	 *
	 * Notice that shift, maskBegin and maskEnd must be compile-time constants for this to work.
	 */
	#define Rlwinm(rd, rx, shift, maskBegin, maskEnd) \
		({asm("rlwinm %0,%1,%2,%3,%4" : "=r" (rd) : "r" (rx), "n" (shift), "n" (maskBegin), "n" (maskEnd)); rd;})


	/*!
	 * Rlwimi is the Rotate Left Word Immediate Then Mask Insert instruction.
	 * It rotates the contents of a general purpose register to the left by a specified number of bits
	 * and stores the result in another general-purpose register under the control of a generated mask.
	 *
	 * Notice that shift, maskBegin and maskEnd must be compile-time constants for this to work.
	 *
	 * Example: If a = 0x0000_0003 = 0b00000000_00000000_00000000_00000011
	 *             b = 0x9000_3000 = 0b10010000_00000000_00110000_00000000
	 *          after Rlwimi(a, b, 2, 0, 29), 
	 *             a = 0x4000_C003 = 0b01000000_00000000_11000000_00000011
	 */
	#define Rlwimi(rd, rx, shift, maskBegin, maskEnd) \
		({asm("rlwimi %0,%2,%3,%4,%5" : "=r" (rd) : "0" (rd), "r" (rx), "n" (shift), "n" (maskBegin), "n" (maskEnd)); rd;})

#else

	template <typename type> inline type Rlwinm(type& rd, U32 rx, U32 shift, U32 maskBegin, U32 maskEnd)
	{
		U32 mask = (0xFFFFFFFFU >> maskBegin) - (0x7FFFFFFFU >> maskEnd);
		U32 rotate = (rx << shift) | (rx >> (32 - shift));
		return (rd = rotate & mask);
	}
	
	template <typename type> inline type Rlwimi(type& rd, U32 rx, U32 shift, U32 maskBegin, U32 maskEnd)
	{
		U32 mask = (0xFFFFFFFFU >> maskBegin) - (0x7FFFFFFFU >> maskEnd);
		U32 rotate = (rx << shift) | (rx >> (32 - shift));
		return (rd = ((U32) rd & ~mask) | (rotate & mask));
	}

#endif

	inline U32 Cntlzw(U32 rx)
	{
#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
		
		register U32	rd;
		
		asm("cntlzw %0,%1" : "=r" (rd) : "r" (rx));
		return (rd);
		
#elif ICE_ARCH_X86 && ICE_COMPILER_VISUALC
	
		if (rx == 0) return (32);
		
		_asm
		{
			BSR		EBX,rx
			MOV		EAX,31
			SUB		EAX,EBX
		}
	
#else
	
		U32 rd = 32;
		while (rx != 0)
		{
			rd--;
			rx >>= 1;
		}
		
		return (rd);
	
#endif
	}


}

#endif // ICE_BITEXTRACT_H
