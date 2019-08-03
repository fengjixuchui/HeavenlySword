/* SCEA CONFIDENTIAL
 * Sharing Initiative Project
 */
/* 
 * Sharing Initiative Runtime Library Release 0.1.7
 *	
 * Copyright (C) 2004 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * SCEAbasetypes.h
 */

/****************************************************************
CHANGE LOG

VER_1_0		8/9/2004	Dan Hilton		
	- The define for CodeWarrior compiler had been __MWERKS.  This was updated to __MWERKS__

VER_1_01	8/19/2004	Dan Hilton		
	- Additional support for PSP compilers added.  

VER_1_02	10/1/2004	Buzz Burrowes
	- Added BOOL type and TRUE | FALSE is needed

VER_1_03	11/16/2004	Gustavo Oliveira
	- Fixed the PSP hardware types for SNSystems compiler

VER_1_04	11/24/2004	Buzz Burrowes
	- Added a generic define that base types had been defined.
	  I need this so that code shared with 3rd parties can work
	  with or without this file.

VER_1_05	12/02/2004	Jim Sproul & Gustavo Oliveira
	- Jim Sproul : redesigned this file to use target macros
	- Gustavo Oliveira : added 128-bit types for PS2 EE target
			   
VER_1_06    12/03/2004  Jim Sproul
    - Changed 128-bit type declarations for PS2-EE from "long long" to
      "__attribute__ ((mode (TI)))", which works properly with
      EE-GCC under Linux.

VER_1_07    12/07/2004  Gustavo Oliveira
	- Added comments regarding the definition of the 128-bit types
	  for Win32/PS2


*****************************************************************/

#if !defined(_SCEA_BASE_TYPES_H) && !defined(SCEA_BASETYPES_DEFINED)
#define _SCEA_BASE_TYPES_H

#include "sceatargetmacros.h"

//######################################################################
// Intel x86 Targets
//######################################################################

#if SCEA_TARGET_CPU_X86 == 1

	//==================================================
	// Microsoft VC/C++ for Win32 executable
	//==================================================

	#if SCEA_HOST_COMPILER_MSVC == 1

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned __int64	U64;
		typedef signed __int64		I64;

		// 128-bit types are possible using VS .net. This allows usage of the MMX SIMD
		// extensions, however they have been not included because is dependent 
		// on the MMX headers. To do so your application must define it as in such:
		//
		//#include <xmmintrin.h>
		//typedef __m128				I128;
		//typedef __m128				U128;
		//
		// And this should be done outside of this header file

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_MSVC == 1

	//==================================================
	// GCC (MinGW) for Win32 executable
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_WIN32 == 1)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_WIN32 == 1)

	//==================================================
	// GCC (Cygwin) for Cygwin executable
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_CYGWIN == 1)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_CYGWIN == 1)

	//==================================================
	// GCC (Linux) for Linux executable
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_LINUX == 1)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_LINUX == 1)

	//==================================================
	// GCC (Mac OS X) for Mac OS X executable
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_MACOSX == 1)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_TARGET_OS_UNIX_MACOSX == 1)

#endif	// #if SCEA_TARGET_CPU_X86 == 1

//######################################################################
// PS2-EE Targets
//#############################################################################

#if SCEA_TARGET_CPU_MIPS_R5900 == 1

	//==================================================
	// SN Systems PS2CC for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_GCC_SN == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long		U64;
		typedef signed long			I64;

		// 128-bit types (should be used only for the VU/EE SIMD extensions)
		typedef unsigned int		U128 __attribute__ ((mode (TI)));
		typedef signed int			I128 __attribute__ ((mode (TI)));

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_GCC_SN == 1

	//==================================================
	// EE-GCC
	//  SN Systems for Win32 host
	//  SCEI for Linux host
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long		U64;
		typedef signed long			I64;

		// 128-bit types (should be used only for the VU/EE SIMD extensions)
		typedef unsigned int		U128 __attribute__ ((mode (TI)));
		typedef signed int			I128 __attribute__ ((mode (TI)));

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

	//==================================================
	// Metrowerks for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_MWERKS == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long		U64;
		typedef signed long			I64;

		// 128-bit types (should be used only for the VU/EE SIMD extensions)
		typedef unsigned int		U128 __attribute__ ((mode (TI)));
		typedef signed int			I128 __attribute__ ((mode (TI)));

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_MWERKS == 1

#endif	// #if SCEA_TARGET_CPU_MIPS_R5900 == 1

//######################################################################
// PS2-IOP Targets
//#############################################################################

#if SCEA_TARGET_CPU_MIPS_R3000 == 1

	//==================================================
	// SN Systems PS2CC for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_GCC_SN == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_GCC_SN == 1

	//==================================================
	// IOP-ELF-GCC and IOP-GCC
	//  SN Systems for Win32 host
	//  SCEI for Linux host
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

	//==================================================
	// Metrowerks for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_MWERKS == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_MWERKS == 1

#endif	// #if SCEA_TARGET_CPU_MIPS_R3000 == 1

//######################################################################
// PSP Targets
//######################################################################

#if (SCEA_TARGET_CPU_MIPS_ALLEGREX == 1) || (SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU == 1)

	//==================================================
	// SN Systems PSPSNC for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_GCC_SN == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_GCC_SN == 1

	//==================================================
	// PSP-GCC
	//  SCEI for Cygwin host (emulator)
	//  SCEI for Linux host (devkit)
	//==================================================

	#if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long long	U64;
		typedef signed long long	I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if (SCEA_HOST_COMPILER_GCC == 1) && (SCEA_HOST_COMPILER_GCC_SN == 0)

	//==================================================
	// Metrowerks for Win32 host
	//==================================================

	#if SCEA_HOST_COMPILER_MWERKS == 1

		// Test for redefined type macros
		#if defined(SCEA_BASETYPES_DEFINED)
			#define SCEA_BASETYPES_REDEFINED __LINE__
			#error Redefined base types!
		#endif

		// 8-bit types
		typedef unsigned char		U8;
		typedef signed char			I8;

		// 16-bit types
		typedef unsigned short		U16;
		typedef signed short		I16;

		// 32-bit types
		typedef unsigned int		U32;
		typedef signed int			I32;

		// 64-bit types
		typedef unsigned long		U64;
		typedef signed long			I64;

		// 128-bit types
		// U128 - not defined yet
		// I128 - not defined yet

		// floating point types
		// F16 - not defined yet
		typedef float				F32;
		typedef double				F64;
		// F80 - not defined yet

		// Set to detect redefined macros
		#define	SCEA_BASETYPES_DEFINED	__LINE__

	#endif	// #if SCEA_HOST_COMPILER_MWERKS == 1

#endif	// #if SCEA_TARGET_CPU_MIPS_ALLEGREX == 1

//######################################################################
// Cell PU & Mac OS X
//######################################################################

#if SCEA_TARGET_CPU_PPC == 1

	// Test for redefined type macros
	#if defined(SCEA_BASETYPES_DEFINED)
		#define SCEA_BASETYPES_REDEFINED __LINE__
		#error Redefined base types!
	#endif

	// 8-bit types
	typedef unsigned char		U8;
	typedef signed char			I8;

	// 16-bit types
	typedef unsigned short		U16;
	typedef signed short		I16;

	// 32-bit types
	typedef unsigned int		U32;
	typedef signed int			I32;

	// 64-bit types
	#if __LP64__
		typedef unsigned long		U64;
		typedef signed long			I64;
	#else
		typedef unsigned long long	U64;
		typedef signed long	long	I64;
	#endif

	// 128-bit types
	#if __VEC__
		#if !(SCEA_TARGET_OS_MAC || SCEA_TARGET_OS_UNIX_MACOSX)
			#include <altivec.h>
		#endif
		typedef vector unsigned int	U128;
		typedef vector signed int	I128;
	#endif

	// floating point types
	// F16 - not defined yet
	typedef float				F32;
	typedef double				F64;
	// F80 - not defined yet

	// vector types
	// all vector types are 16-bytes in size
	typedef vector unsigned char        VU8;    // 16 elements
	typedef vector signed char          VI8;    // 16 elements
	
	typedef vector unsigned short       VU16;   // 8 elements
	typedef vector signed short         VI16;   // 8 elements
	
	typedef vector unsigned int         VU32;   // 4 elements
	typedef vector signed int           VI32;   // 4 elements
	
	// VU64 - not defined
	// VI64 - not defined
	
	typedef vector signed short         VF16;   // 8 elements
	typedef vector float                VF32;   // 4 elements
	// VF64 - not defined
	

	// Set to detect redefined macros
	#define	SCEA_BASETYPES_DEFINED	__LINE__

#endif	// #if SCEA_TARGET_CPU_PPC == 1

//######################################################################
// Cell SPU
//######################################################################

#if SCEA_TARGET_CPU_CELL_SPU == 1

	// Test for redefined type macros
	#if defined(SCEA_BASETYPES_DEFINED)
		#define SCEA_BASETYPES_REDEFINED __LINE__
		#error Redefined base types!
	#endif

	// 8-bit types
	typedef unsigned char		U8;
	typedef signed char			I8;

	// 16-bit types
	typedef unsigned short		U16;
	typedef signed short		I16;

	// 32-bit types
	typedef unsigned int		U32;
	typedef signed int			I32;

	// 64-bit types
	typedef unsigned long long	U64;
	typedef signed long	long	I64;

	// 128-bit types
	#include <spu_intrinsics.h>
	typedef vector unsigned int	U128;
	typedef vector signed int	I128;

	// floating point types
	// F16 - not defined yet
	typedef float				F32;
	typedef double				F64;
	// F80 - not defined yet

	// all vector types are 16-bytes in size
	typedef vector unsigned char        VU8;    // 16 elements
	typedef vector signed char          VI8;    // 16 elements
	
	typedef vector unsigned short       VU16;   // 8 elements
	typedef vector signed short         VI16;   // 8 elements
	
	typedef vector unsigned int         VU32;   // 4 elements
	typedef vector signed int           VI32;   // 4 elements
	
	typedef vector unsigned long long	VU64;   // 2 elements
	typedef vector signed long long		VI64;   // 2 elements
	
	typedef vector signed short         VF16;   // 8 elements
	typedef vector float                VF32;   // 4 elements
	typedef vector double               VF64;   // 2 elements

	// Set to detect redefined macros
	#define	SCEA_BASETYPES_DEFINED	__LINE__

#endif	// #if SCEA_TARGET_CPU_CELL_SPU == 1

//######################################################################
// ERROR - Unsupported Platform
//######################################################################

#if !defined(SCEA_BASETYPES_DEFINED)
	#error Unsupported platform! Base types not defined!
#endif

//######################################################################
// Platform Independant (derived from types above)
//######################################################################

	/// character types
	typedef U8						C8;		// UTF-8
	typedef U16						C16;	// UTF-16
	typedef U32						C32;	// UTF-32

	// boolean type - WARNING: The following may go away in a future release...
	#if !defined(BOOL)
		#define	BOOL				I32
	#endif

	#if !defined(FALSE)
		#define FALSE				(1==0)
	#endif
	#if !defined(TRUE)
		#define TRUE				(1==1)
	#endif

/*
 (c) 2004 Sony Computer Entertainment America Inc. ("SCEAI")

 THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF 
 ANY KIND, AND SCEAI EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO
 EVENT SHALL SCEAI BE LIABLE FOR ANY INCIDENTAL, SPECIAL OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. SOME JURISDICTIONS MAY
 NOT ALLOW THE EXCLUSION OR LIMITATION OF INCIDENTAL OR CONSEQUENTIAL
 DAMAGES, SO THE ABOVE LIMITATIONS MAY NOT APPLY TO YOU.
*/

#endif	// #if !defined(_SCEA_BASE_TYPES_H) && !defined(SCEA_BASETYPES_DEFINED)
