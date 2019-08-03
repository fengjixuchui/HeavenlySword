/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_MATH_H
#define ICE_MATH_H

#include "icebase.h"

namespace Ice
{
	inline float Fabs(float rx)
	{
#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
		
		register float		rd;
		
		asm("fabs %0,%1" : "=f" (rd) : "f" (rx));
		return (rd);
		
#else		
		return (fabsf(rx));
		
#endif
	}


	inline float Fnabs(float rx)
	{
#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
		
		register float		rd;
		
		asm("fnabs %0,%1" : "=f" (rd) : "f" (rx));
		return (rd);
		
#else
		
		return (-fabsf(rx));
		
#endif
	}


#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
	
	float Sqrt(float f);
	float RSqrt(float f);

#else

	inline float Sqrt(float f)
	{
		return (sqrtf(f));
	}
	
	inline float RSqrt(float f)
	{
		return (1.0F / sqrtf(f));
	}

#endif


	inline float Sin(float f)
	{
		return (sinf(f));
	}
	
	inline float Cos(float f)
	{
		return (cosf(f));
	}
	
	inline float Tan(float f)
	{
		return (tanf(f));
	}
	
	inline float Asin(float f)
	{
		return (asinf(f));
	}
	
	inline float Acos(float f)
	{
		return (acosf(f));
	}
	
	inline float Atan(float f)
	{
		return (atanf(f));
	}
	
	inline float Atan(float y, float x)
	{
		return (atan2f(y, x));
	}
	
	inline float Exp(float f)
	{
		return (expf(f));
	}
	
	inline float Log(float f)
	{
		return (logf(f));
	}

	inline float Log10(float f)
	{
		return (log10f(f));
	}
	
	inline float Pow(float base, float exponent)
	{
		return (powf(base, exponent));
	}
};

#endif
