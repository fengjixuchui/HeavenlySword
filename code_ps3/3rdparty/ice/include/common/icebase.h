/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BASE_H
#define ICE_BASE_H

#ifndef ICEDEBUG
#define ICEDEBUG 0
#endif

#if defined(__GNUC__)

	#define ICE_COMPILER_GCC			1
	#define ICE_COMPILER_VISUALC		0

	#if defined (__PPU__)

		#define ICE_TARGET_PS3_PPU		1
		#define ICE_TARGET_PS3_SPU		0
		#define ICE_TARGET_WINDOWS		0
		#define ICE_TARGET_LINUX		0

		#define ICE_ARCH_X86			0
		#define ICE_ARCH_POWERPC		1
		#define ICE_ARCH_SPU			0

		#define ICE_ENDIAN_BIG			1
		#define ICE_ENDIAN_LITTLE		0

	#elif defined (__SPU__)

		#define ICE_TARGET_PS3_PPU		0
		#define ICE_TARGET_PS3_SPU		1
		#define ICE_TARGET_WINDOWS		0
		#define ICE_TARGET_LINUX		0

		#define ICE_ARCH_X86			0
		#define ICE_ARCH_POWERPC		0
		#define ICE_ARCH_SPU			1

		#define ICE_ENDIAN_BIG			1
		#define ICE_ENDIAN_LITTLE		0

	#else

		#define ICE_TARGET_PS3_PPU		0
		#define ICE_TARGET_PS3_SPU		0
		#define ICE_TARGET_WINDOWS		0
		#define ICE_TARGET_LINUX		1

		#define ICE_ARCH_X86			1
		#define ICE_ARCH_POWERPC		0
		#define ICE_ARCH_SPU			0

		#define ICE_ENDIAN_BIG			0
		#define ICE_ENDIAN_LITTLE		1

	#endif

#elif defined(_MSC_VER)

	#define ICE_COMPILER_GCC			0
	#define ICE_COMPILER_VISUALC		1

	#define ICE_TARGET_PS3_PPU			0
	#define ICE_TARGET_PS3_SPU			0
	#define ICE_TARGET_WINDOWS			1
	#define ICE_TARGET_LINUX			0

	#define ICE_ARCH_X86				1
	#define ICE_ARCH_POWERPC			0
	#define ICE_ARCH_SPU				0

	#define ICE_ENDIAN_BIG				0
	#define ICE_ENDIAN_LITTLE			1

#endif


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if ICE_TARGET_WINDOWS
    #include <xmmintrin.h>
    #include <assert.h>
    //#include <minmax.h>
#endif

#if ICE_TARGET_PS3_PPU
    #include <sdk_version.h>
	#include <sys/process.h>
	#include <sys/memory.h>
	#include <sys/sys_time.h>
	#include <sys/raw_spu.h>
    #include <assert.h>
    #include <sys/paths.h>
	#include <types.h>
#endif

#if ICE_ARCH_SPU
	#include <spu_intrinsics.h>

// define this to 1 to cause spu ICE_ASSERT to print
#define	ICE_ASSERT_SPUPRINTF_ENABLED	0

# if ICEDEBUG
	#define PRINTF_ENABLED 1
	#include "jobapi/jobprintf.h"
# endif
#endif

#define DPRINT_PTR(a) printf(#a " = %p\n", a);
#define DPRINT_F32(a) printf(#a " = %f\n", a);

// Proposed built-in C++0x type
#define nullptr 0

#define ICE_ASSERT_VAL(x) #x
#define ICE_ASSERT_STRIZE(x) ICE_ASSERT_VAL(x)

#if ICEDEBUG
#if ICE_ARCH_POWERPC

#  define ICE_HALT() do { asm __volatile__("trap"); } while(0)
#  define ICE_PRINTF ::printf
#  define ICE_ASSERT(cond) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#  define ICE_ASSERTF(cond, desc) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)
#  define ICE_ASSERT_IF(cond) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#  define ICE_ASSERTF_IF(cond, desc) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)

#elif ICE_ARCH_X86

#  define ICE_HALT() do { __asm int 3; } while(0)
#  define ICE_PRINTF ::printf
#  define ICE_ASSERT(cond) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#  define ICE_ASSERTF(cond, desc) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)
#  define ICE_ASSERT_IF(cond) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#  define ICE_ASSERTF_IF(cond, desc) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)

#elif ICE_ARCH_SPU

# define ICE_HALT() do { spu_stop(1); } while(0)
# define ICE_PRINTF JobPrintf
# if ICE_ASSERT_SPUPRINTF_ENABLED
#   define ICE_ASSERT(cond) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#   define ICE_ASSERTF(cond, desc) do { if (!(cond)) { ICE_PRINTF("Assertation failed: '(%s) == false' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)
#   define ICE_ASSERT_IF(cond) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) "\n", #cond); ICE_HALT(); }} while (0)
#   define ICE_ASSERTF_IF(cond, desc) do { if ((cond)) { ICE_PRINTF("Assertation failed: '(%s) == true' at "__FILE__ ":" ICE_ASSERT_STRIZE(__LINE__) " with ", #cond); ICE_PRINTF desc; ICE_PRINTF("\n"); ICE_HALT(); }} while (0)
# else
#   define ICE_ASSERT(cond) do { if (!(cond)) { ICE_HALT(); }} while (0)
#   define ICE_ASSERTF(cond, desc) do { if (!(cond)) { ICE_HALT(); }} while (0)
#   define ICE_ASSERT_IF(cond) do { if ((cond)) { ICE_HALT(); }} while (0)
#   define ICE_ASSERTF_IF(cond, desc) do { if ((cond)) { ICE_HALT(); }} while (0)
# endif

#else

#  define ICE_HALT() assert(0)
#  define ICE_ASSERT(cond) assert(cond)
#  define ICE_ASSERTF(cond, desc) assert(cond)
#  define ICE_ASSERT_IF(cond) assert(!(cond))
#  define ICE_ASSERTF_IF(cond, desc) assert(!(cond))

#endif
#else

# define ICE_HALT() do {} while(0)
# define ICE_ASSERT(cond) do {(void)(cond);} while(0)
# define ICE_ASSERTF(cond, desc) do {(void)(cond);} while(0)
# define ICE_ASSERT_IF(cond) do {(void)(cond);} while(0)
# define ICE_ASSERTF_IF(cond, desc) do {(void)(cond);} while(0)

#endif

#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
#define FORCE_INLINE inline __attribute__((always_inline))
#elif ICE_ARCH_X86 && ICE_COMPILER_VISUALC
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif

#if ICE_ARCH_POWERPC && ICE_COMPILER_GCC
#define DEPRECATED         __attribute__((deprecated))
#define MALLOC_FUNCTION    __attribute__((malloc))
#define WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#else
#define DEPRECATED
#define MALLOC_FUNCTION
#define WARN_UNUSED_RESULT
#endif

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*(a)))

namespace Ice
{
	template <typename type> static inline type Min(type const &a, type const &b)
	{
		return a < b ? a : b;
	}

	template <typename type> static inline type Max(type const &a, type const &b)
	{
		return a > b ? a : b;
	}
	
	static inline U32 F32ToU32(F32 a)
	{
		union {U32 u; F32 f;} u2f;
		u2f.f = a;
		return u2f.u;
	}
}

#endif // ICE_BASE_H
