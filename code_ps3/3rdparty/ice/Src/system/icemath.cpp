/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icebase.h"


#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC

namespace Ice
{
	union F32_I32 {
		F32 f32;
		I32 i32;

		F32_I32() {}
		F32_I32(F32 f32_value) : f32(f32_value) {}
		F32_I32(I32 i32_value) : i32(i32_value) {}
	};
	const float kfInfinity = F32_I32( (I32)0x7F800000 ).f32;

    inline float Frsqrte(float rx)
	{
			register float		rd;

			asm("frsqrte %0,%1" : "=f" (rd) : "f" (rx));
			return (rd);
	}

	float Sqrt(float f)
	{
		if (f <= 0.0F) return (0.0F);

		float r = Frsqrte(f);
		float r2 = r * r;
		float s = 0.5F * r;
		r = s * (3.0F - f * r2);
		r = s * (3.0F - f * r2);
		return (r * f);
	}

	float RSqrt(float f)
	{
		if (f == 0.0F) return (kfInfinity);

		float r = Frsqrte(f);
		float r2 = r * r;
		float s = 0.5F * r;
		r = s * (3.0F - f * r2);
		r = s * (3.0F - f * r2);
		return (r);
	}
}
#endif
