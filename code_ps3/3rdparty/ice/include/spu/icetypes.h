/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_TYPES_SPU
#define ICE_TYPES_SPU

// Base types for the SPU

// This file should be automatically included whenever source gets compiled.

// integer types
typedef unsigned char               U8;
typedef signed char                 I8;

typedef unsigned short              U16;
typedef signed short                I16;

typedef unsigned int                U32;
typedef signed int                  I32;

typedef unsigned long long          U64;
typedef signed long long            I64;

typedef unsigned int				U32F;
typedef int							I32F;

// floating point types
typedef signed short                F16;
typedef float                       F32;
typedef double                      F64;

// vector types
// all vector types are 16-bytes in size
typedef vector unsigned char        VU8;    // 16 elements
typedef vector signed char          VI8;    // 16 elements

typedef vector unsigned short       VU16;   // 8 elements
typedef vector signed short         VI16;   // 8 elements

typedef vector unsigned int         VU32;   // 4 elements
typedef vector signed int           VI32;   // 4 elements

typedef vector unsigned long long   VU64;   // 2 elements
typedef vector signed long long     VI64;   // 2 elements

typedef vector signed short         VF16;   // 8 elements
typedef vector float                VF32;   // 4 elements
typedef vector double               VF64;   // 2 elements

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)
#define ICE_ALIGN(alignment) __attribute__((aligned(alignment)))
#define ICE_ALIGNED_MALLOC(size, alignment) memalign(alignment, size)

#endif

