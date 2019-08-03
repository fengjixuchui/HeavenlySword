/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_TYPES_LINUX
#define ICE_TYPES_LINUX

typedef unsigned char			U8;
typedef signed char				I8;

typedef unsigned short			U16;
typedef short					I16;

typedef unsigned int			U32;
typedef int						I32;

typedef unsigned long long		U64;
typedef long long				I64;

typedef short					F16;
typedef float					F32;
typedef double					F64;

typedef unsigned int            U32F;
typedef signed int              I32F;
 

#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define __restrict
#define __restrict__
#define ICE_ALIGN(alignment) __declspec(align(alignment))
#define ICE_ALIGNED_MALLOC(size, alignment) memalign(alignment, size)

#endif // ICE_TYPES_LINUX
