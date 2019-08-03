/* 
 * sceatargetmacros.h 
 * $Id$
 *
 * Sharing Initiative Runtime Library
 *
 * Copyright (C) 2004 Sony Computer Entertainment America, Inc.
 * 
 * CONFIDENTIAL
 */

#ifndef _SCEA_TARGETMACROS_H
#define _SCEA_TARGETMACROS_H 1

/*
 * These macros are intended to be used directly in preprocessor or runtime
 * logic and should not be tested for using #ifdef or #if defined() expressions.
 * In other words, these macros WILL be defined as either TRUE (1) or FALSE (0).
 *
 * Test only for the most specific feature you actually need. For example, test
 * for SCEA_TARGET_RT_LITTLE_ENDIAN instead of testing the host compiler or
 * target platform.
 *
 * All of the macros which are defined in this file are first #undef'd, so you
 * you get a birds eye view of the entire list below. Lists of macros which
 * are indented are subordinate to the macro they are listed under. For example,
 * SCEA_TARGET_CPU_MIPS_R3000 and SCEA_TARGET_CPU_MIPS_R5000 are subordinate to
 * SCEA_TARGET_CPU_MIPS. A subordinate macro will only be set to "1" if it's
 * parent macro is also set to "1". However, just because a parent macro is
 * set to "1" doesn't mean that one of it's subordinates will be set to "1",
 * as well.
 *
 * Here's a brief rundown of the macro lists:
 *
 * SCEA_HOST_COMPILER... The compiler family which is building this file.
 *
 * SCEA_TARGET_CPU... The target processor.
 *
 * SCEA_TARGET_RT_...ENDIAN... The native byte order of the target processor.
 *
 * SCEA_TARGET_RT_PTR_SIZE_... The pointer size used in the target runtime.
 *
 * SCEA_TARGET_CAPS... The target processors capabilities. If the parent macro
 *  is set then the target processor and/or compiler supports the data type and
 *  size indicated. If the _HW subordinate macro is set, then the data type is
 *  supported in hardware. If the _SW subordinate macro is set, then the data
 *  type is supported in software. If the parent macro is NOT set, then the
 *  target processor and/or compiler do NOT support the data type and size
 *  indicated, and no such type will be defined in sceabasetypes.h.
 *
 * SCEA_TARGET_RT...<format>... The runtime executable file format.
 *
 * SCEA_TARGET_OS... The target operating system family.
 */

#undef SCEA_HOST_COMPILER_MWERKS
#undef SCEA_HOST_COMPILER_MSVC
#undef SCEA_HOST_COMPILER_XLC
#undef SCEA_HOST_COMPILER_GCC
	#undef SCEA_HOST_COMPILER_GCC_SN

#undef SCEA_ALIGN_BEG
#undef SCEA_ALIGN_END

#undef SCEA_TARGET_CPU_PPC
#undef SCEA_TARGET_CPU_CELL_SPU
#undef SCEA_TARGET_CPU_X86
#undef SCEA_TARGET_CPU_MIPS
	#undef SCEA_TARGET_CPU_MIPS_R3000
	#undef SCEA_TARGET_CPU_MIPS_R5900
	#undef SCEA_TARGET_CPU_MIPS_ALLEGREX
	#undef SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU

#undef SCEA_TARGET_RT_LITTLE_ENDIAN
#undef SCEA_TARGET_RT_BIG_ENDIAN

#undef SCEA_TARGET_RT_PTR_SIZE_32
#undef SCEA_TARGET_RT_PTR_SIZE_64

#undef SCEA_TARGET_CAPS_INT64
	#undef SCEA_TARGET_CAPS_INT64_HW
	#undef SCEA_TARGET_CAPS_INT64_SW
#undef SCEA_TARGET_CAPS_INT128
	#undef SCEA_TARGET_CAPS_INT128_HW
	#undef SCEA_TARGET_CAPS_INT128_SW
#undef SCEA_TARGET_CAPS_F16
	#undef SCEA_TARGET_CAPS_F16_HW
	#undef SCEA_TARGET_CAPS_F16_SW
#undef SCEA_TARGET_CAPS_F32
	#undef SCEA_TARGET_CAPS_F32_HW
	#undef SCEA_TARGET_CAPS_F32_SW
#undef SCEA_TARGET_CAPS_F64
	#undef SCEA_TARGET_CAPS_F64_HW
	#undef SCEA_TARGET_CAPS_F64_SW
#undef SCEA_TARGET_CAPS_F80
	#undef SCEA_TARGET_CAPS_F80_HW
	#undef SCEA_TARGET_CAPS_F80_SW

#undef SCEA_TARGET_RT_ELF
	#undef SCEA_TARGET_RT_ELF_LINUX
	#undef SCEA_TARGET_RT_ELF_PS2_EE
	#undef SCEA_TARGET_RT_ELF_PS2_ERX
	#undef SCEA_TARGET_RT_ELF_PS2_IRX
	#undef SCEA_TARGET_RT_ELF_PSP_PRX
#undef SCEA_TARGET_RT_PECOFF
#undef SCEA_TARGET_RT_MACHO
#undef SCEA_TARGET_RT_XCOFF

#undef SCEA_TARGET_OS_PS1
#undef SCEA_TARGET_OS_PS2
	#undef SCEA_TARGET_OS_PS2_EE
	#undef SCEA_TARGET_OS_PS2_IOP
#undef SCEA_TARGET_OS_PSP
#undef SCEA_TARGET_OS_PS3
	#undef SCEA_TARGET_OS_PS3_PU
	#undef SCEA_TARGET_OS_PS3_SPU
#undef SCEA_TARGET_OS_WIN32
#undef SCEA_TARGET_OS_MAC
#undef SCEA_TARGET_OS_UNIX
	#undef SCEA_TARGET_OS_UNIX_MACOSX
	#undef SCEA_TARGET_OS_UNIX_AIX
	#undef SCEA_TARGET_OS_UNIX_CYGWIN
	#undef SCEA_TARGET_OS_UNIX_LINUX

/*
 * This block of tests is by necessity order specific. For example,
 * several compilers define __GNUC__ because they support all of
 * the GCC language extensions. Be careful when you edit.
 */

#if defined(__MWERKS__)
	#define	SCEA_HOST_COMPILER_MWERKS				1

	#define SCEA_ALIGN_BEG(x) 
	#define SCEA_ALIGN_END(x)						__attribute__((__aligned__(x)))

	#if defined(__POWERPC__)
		#define SCEA_TARGET_CPU_PPC					1
		#define SCEA_TARGET_RT_BIG_ENDIAN			1
	#elif defined(__INTEL__)
		#define SCEA_TARGET_CPU_X86					1
		#define SCEA_TARGET_RT_LITTLE_ENDIAN		1
		#define	SCEA_TARGET_CAPS_INT64				1
		#define	SCEA_TARGET_CAPS_INT64_SW			1
		#define	SCEA_TARGET_CAPS_F32				1
		#define	SCEA_TARGET_CAPS_F32_HW				1
		#define	SCEA_TARGET_CAPS_F64				1
		#define	SCEA_TARGET_CAPS_F64_HW				1
	#elif defined(__MIPS__)
		#define SCEA_TARGET_CPU_MIPS				1

		#if __option(little_endian)
			#define SCEA_TARGET_RT_LITTLE_ENDIAN	1
		#else
			#define SCEA_TARGET_RT_BIG_ENDIAN		1
		#endif

	#endif

	#if defined(__ELF__)
		#define SCEA_TARGET_RT_ELF					1
	#elif defined(__MACH__)
		#define SCEA_TARGET_RT_MACHO				1
		#define SCEA_TARGET_OS_MAC					1
		#define SCEA_TARGET_OS_UNIX					1
		#define SCEA_TARGET_OS_UNIX_MACOSX			1
	#elif defined(_WIN32)
		#define SCEA_TARGET_RT_PECOFF				1
		#define SCEA_TARGET_OS_WIN32				1
	#elif defined(__MIPS_PSX2__)
		#define	SCEA_TARGET_CPU_MIPS				1
		#define SCEA_TARGET_CPU_MIPS_R5900			1
		#define	SCEA_TARGET_CAPS_INT64				1
		#define	SCEA_TARGET_CAPS_INT64_HW			1
		#define SCEA_TARGET_CAPS_INT128				1
		#define	SCEA_TARGET_CAPS_INT128_HW			1
		#define	SCEA_TARGET_CAPS_F32				1
		#define	SCEA_TARGET_CAPS_F32_HW				1
		#define	SCEA_TARGET_CAPS_F64				1
		#define	SCEA_TARGET_CAPS_F64_SW				1
		#define SCEA_TARGET_OS_PS2					1
		#define SCEA_TARGET_OS_PS2_EE				1
		#define SCEA_TARGET_RT_ELF_PS2_EE			1
	#elif defined(__MIPS_PSP__)
		#define	SCEA_TARGET_CPU_MIPS				1
		#define	SCEA_TARGET_CPU_MIPS_ALLEGREX		1
		#define	SCEA_TARGET_CAPS_INT64				1
		#define	SCEA_TARGET_CAPS_INT64_SW			1
		#define	SCEA_TARGET_CAPS_F32				1
		#define	SCEA_TARGET_CAPS_F32_HW				1
		#define	SCEA_TARGET_CAPS_F64				1
		#define	SCEA_TARGET_CAPS_F64_SW				1
		#define SCEA_TARGET_OS_PSP					1
		#define	SCEA_TARGET_RT_ELF_PSP_PRX			1
	#endif

#elif defined(__GNUC__)
	#define SCEA_HOST_COMPILER_GCC					1

	#define SCEA_ALIGN_BEG(x) 
	#define SCEA_ALIGN_END(x)						__attribute__((__aligned__(x)))

	#if defined(__ppc__) || defined(__ppc64__) || defined(__PPU__)
		#define SCEA_TARGET_CPU_PPC					1
		
		// this really depends on the current MSR[LE] bit, but we're going to assume this
		#define SCEA_TARGET_RT_BIG_ENDIAN			1
		#define SCEA_TARGET_CAPS_INT64				1
		#define SCEA_TARGET_CAPS_INT64_HW			1
		#define SCEA_TARGET_CAPS_INT128				1
		#define SCEA_TARGET_CAPS_INT128_HW			1
		#define SCEA_TARGET_CAPS_F32				1
		#define SCEA_TARGET_CAPS_F32_HW				1
		#define SCEA_TARGET_CAPS_F64				1
		#define SCEA_TARGET_CAPS_F64_HW				1

	#elif defined(__SPU__)
		#define SCEA_TARGET_CPU_CELL_SPU			1
		#define SCEA_TARGET_RT_BIG_ENDIAN			1
		#define SCEA_TARGET_CAPS_INT64				1
		#define SCEA_TARGET_CAPS_INT64_HW			1
		#define SCEA_TARGET_CAPS_INT128				1
		#define SCEA_TARGET_CAPS_INT128_HW			1
		#define SCEA_TARGET_CAPS_F32				1
		#define SCEA_TARGET_CAPS_F32_HW				1
		#define SCEA_TARGET_CAPS_F64				1
		#define SCEA_TARGET_CAPS_F64_HW				1

		#define SCEA_TARGET_OS_PS3					1
		#define SCEA_TARGET_OS_PS3_SPU				1
	#elif defined(__i386__)
		#define SCEA_TARGET_CPU_X86					1
		#define SCEA_TARGET_RT_LITTLE_ENDIAN		1
		#define	SCEA_TARGET_CAPS_INT64				1
		#define	SCEA_TARGET_CAPS_INT64_SW			1
		#define	SCEA_TARGET_CAPS_F32				1
		#define	SCEA_TARGET_CAPS_F32_HW				1
		#define	SCEA_TARGET_CAPS_F64				1
		#define	SCEA_TARGET_CAPS_F64_HW				1
	#elif defined(__mips__) || defined(__MIPSEL__) || defined(__MIPSEB__)
		#define SCEA_TARGET_CPU_MIPS				1

		#if defined(__MIPSEL__)
			#define SCEA_TARGET_RT_LITTLE_ENDIAN	1
		#else
			#define SCEA_TARGET_RT_BIG_ENDIAN		1
		#endif

		#if defined(__psp__)	// includes PSPSNC, PSP-GCC (Cygwin), PSP-GCC (Linux)
			#if defined(__psp_gcc__) || defined(_R4000)	// PSPSNC and PSP-GCC (Linux) only
				#define SCEA_TARGET_CPU_MIPS_ALLEGREX	1
			#else
				#define	SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU	1	// PSP-GCC (Cygwin)
			#endif
			#define	SCEA_TARGET_CAPS_INT64			1
			#define	SCEA_TARGET_CAPS_INT64_SW		1
			#define	SCEA_TARGET_CAPS_F32			1
			#define	SCEA_TARGET_CAPS_F32_HW			1
			#define	SCEA_TARGET_CAPS_F64			1
			#define	SCEA_TARGET_CAPS_F64_SW			1
			#define SCEA_TARGET_RT_ELF				1
			#define SCEA_TARGET_RT_ELF_PSP_PRX		1
			#define SCEA_TARGET_OS_PSP				1
			#if defined(SN_TARGET_PSP) || defined(__SNC__)
				#define SCEA_HOST_COMPILER_GCC_SN	1
			#endif
		#elif defined(__R3000__)
			#define SCEA_TARGET_CPU_MIPS_R3000		1
			#define	SCEA_TARGET_CAPS_INT64			1
			#define	SCEA_TARGET_CAPS_INT64_SW		1
			#define	SCEA_TARGET_CAPS_F32			1
			#define	SCEA_TARGET_CAPS_F32_SW			1
			#define	SCEA_TARGET_CAPS_F64			1
			#define	SCEA_TARGET_CAPS_F64_SW			1
			#define SCEA_TARGET_RT_ELF				1
			#define SCEA_TARGET_RT_ELF_PS2_IRX		1
			#define SCEA_TARGET_OS_PS2				1
			#define SCEA_TARGET_OS_PS2_IOP			1
			#if defined(__SN_TARGET_IOP_)
				#define SCEA_HOST_COMPILER_GCC_SN	1
			#endif
		#elif defined(__R5900__)
			#define SCEA_TARGET_CPU_MIPS_R5900		1
			#define	SCEA_TARGET_CAPS_INT128			1
			#define	SCEA_TARGET_CAPS_INT128_HW		1
			#define	SCEA_TARGET_CAPS_INT64			1
			#define	SCEA_TARGET_CAPS_INT64_HW		1
			#define	SCEA_TARGET_CAPS_F32			1
			#define	SCEA_TARGET_CAPS_F32_HW			1
			#define	SCEA_TARGET_CAPS_F64			1
			#define	SCEA_TARGET_CAPS_F64_SW			1
			#define SCEA_TARGET_RT_ELF				1
			#define SCEA_TARGET_RT_ELF_PS2_EE		1
			#define SCEA_TARGET_OS_PS2				1
			#define SCEA_TARGET_OS_PS2_EE			1
			#if defined(SN_TARGET_PS2)
				#define SCEA_HOST_COMPILER_GCC_SN	1
			#endif
		#endif

	#endif

	#if defined(__ELF__)
		#define SCEA_TARGET_RT_ELF					1
		#if defined(linux)
			#define SCEA_TARGET_RT_ELF_LINUX		1
			#define	SCEA_TARGET_OS_UNIX				1
			#define	SCEA_TARGET_OS_UNIX_LINUX		1
		#elif defined(__CELLOS_LV2__)
			#define SCEA_TARGET_OS_PS3				1
			//!!! define ELF variants here
			#if defined(__PU__) || defined(__PPU__)
				#define SCEA_TARGET_OS_PS3_PU		1
			#else
				#define SCEA_TARGET_OS_PS3_SPU		1
			#endif
		#endif

	#elif defined(__MACH__)
		#define SCEA_TARGET_RT_MACHO				1
		#define SCEA_TARGET_OS_MAC					1
		#define SCEA_TARGET_OS_UNIX					1
		#define SCEA_TARGET_OS_UNIX_MACOSX			1
	#elif defined(_WIN32)
		#define SCEA_TARGET_RT_PECOFF				1
		#define SCEA_TARGET_OS_WIN32				1
	#elif defined(__CYGWIN__)
		#define	SCEA_TARGET_RT_PECOFF				1
		#define	SCEA_TARGET_OS_UNIX					1
		#define	SCEA_TARGET_OS_UNIX_CYGWIN			1
	#endif

#elif defined(_MSC_VER)
	#define	SCEA_HOST_COMPILER_MSVC					1

	#define SCEA_ALIGN_BEG(x)						__declspec( align(x) )
	#define SCEA_ALIGN_END(x) 

	#if defined(_M_PPC)
		#define SCEA_TARGET_CPU_PPC					1
		#define SCEA_TARGET_RT_LITTLE_ENDIAN		1		//!!! likely to change for XBox360
	#elif defined(_M_MPPC)
		#define SCEA_TARGET_CPU_PPC					1
		#define SCEA_TARGET_RT_BIG_ENDIAN			1
	#elif defined(_M_IX86)
		#define SCEA_TARGET_CPU_X86					1
		#define SCEA_TARGET_RT_LITTLE_ENDIAN		1
		#define	SCEA_TARGET_CAPS_INT64				1
		#define	SCEA_TARGET_CAPS_INT64_SW			1
		#define	SCEA_TARGET_CAPS_F32				1
		#define	SCEA_TARGET_CAPS_F32_HW				1
		#define	SCEA_TARGET_CAPS_F64				1
		#define	SCEA_TARGET_CAPS_F64_HW				1
	#elif defined(_M_MRX000)
		#define SCEA_TARGET_CPU_MIPS				1
		#define SCEA_TARGET_RT_LITTLE_ENDIAN		1		//!!! is there a way to detect this?
	#endif

	#if defined(_WIN32)
		#define SCEA_TARGET_RT_PECOFF				1
		#define SCEA_TARGET_OS_WIN32				1
	#else
		#error "Microsoft Visual C++ with unknown target runtime"
	#endif

#elif defined(__xlc) || defined(__xlC) || defined(__xlC__)
	#define	SCEA_HOST_COMPILER_XLC					1	// IBM Visual Age C++

	#define SCEA_ALIGN_BEG(x) 
	#define SCEA_ALIGN_END(x)						__attribute__((__aligned__(x)))

	#if defined(_powerc)
		#define SCEA_TARGET_CPU_PPC					1
		#define SCEA_TARGET_RT_BIG_ENDIAN			1
	#else
		#error "IBM VisualAge xlC with unknown target cpu"
	#endif

	#if defined(__MACH__)
		#define SCEA_TARGET_RT_MACHO				1
		#define SCEA_TARGET_OS_MAC					1
		#define SCEA_TARGET_OS_UNIX					1
		#define SCEA_TARGET_OS_UNIX_MACOSX			1
	#elif defined(_AIX)
		#define SCEA_TARGET_RT_XCOFF				1
		#define SCEA_TARGET_OS_UNIX					1
		#define SCEA_TARGET_OS_UNIX_AIX				1
	#else
		#error "IBM VisualAge xlC with unknown target runtime"
	#endif

#endif

/* defined always */

// another possible test would be to comapre <stdint.h>'s PTRDIFF_MAX or INTPTR_MAX to INT32_MAX
#if defined(__LP64__) || defined(MS_WIN64) || defined(_LP64)
	#define SCEA_TARGET_RT_PTR_SIZE_64			1
#else
	#define SCEA_TARGET_RT_PTR_SIZE_64			0
#endif
#define SCEA_TARGET_RT_PTR_SIZE_32				(!SCEA_TARGET_RT_PTR_SIZE_64)

/* not defined if the host compiler is unrecognized because we want to fail compilation */

//#define SCEA_ALIGN_BEG 
//#define SCEA_ALIGN_END

/* define the remaining settings to 0 so they may be used in runtime logic */

#if !SCEA_HOST_COMPILER_MWERKS
	#define SCEA_HOST_COMPILER_MWERKS			0
#endif

#if !SCEA_HOST_COMPILER_MSVC
	#define SCEA_HOST_COMPILER_MSVC				0
#endif
#if !SCEA_HOST_COMPILER_XLC
	#define SCEA_HOST_COMPILER_XLC				0
#endif
#if !SCEA_HOST_COMPILER_GCC
	#define SCEA_HOST_COMPILER_GCC				0
#endif
    #if !SCEA_HOST_COMPILER_GCC_SN
        #define SCEA_HOST_COMPILER_GCC_SN	    0
    #endif

#if !SCEA_TARGET_CPU_PPC
	#define SCEA_TARGET_CPU_PPC					0
#endif
#if !SCEA_TARGET_CPU_CELL_SPU
	#define SCEA_TARGET_CPU_CELL_SPU			0
#endif
#if !SCEA_TARGET_CPU_X86
	#define SCEA_TARGET_CPU_X86					0
#endif
#if !SCEA_TARGET_CPU_MIPS
	#define SCEA_TARGET_CPU_MIPS				0
#endif
    #if !SCEA_TARGET_CPU_MIPS_R3000
        #define SCEA_TARGET_CPU_MIPS_R3000		0
    #endif
    #if !SCEA_TARGET_CPU_MIPS_R5900
        #define SCEA_TARGET_CPU_MIPS_R5900		0
    #endif
    #if !SCEA_TARGET_CPU_MIPS_ALLEGREX
        #define SCEA_TARGET_CPU_MIPS_ALLEGREX	0
    #endif
	#if !SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU
		#define	SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU 0
	#endif

#if !SCEA_TARGET_RT_LITTLE_ENDIAN
	#define SCEA_TARGET_RT_LITTLE_ENDIAN		0
#endif
#if !SCEA_TARGET_RT_BIG_ENDIAN
	#define SCEA_TARGET_RT_BIG_ENDIAN			0
#endif

// defined always with logic above
//#if !SCEA_TARGET_RT_PTR_SIZE_32
//	#define SCEA_TARGET_RT_PTR_SIZE_32			0
//#endif
//#if !SCEA_TARGET_RT_PTR_SIZE_64
//	#define SCEA_TARGET_RT_PTR_SIZE_64			0
//#endif

#if !SCEA_TARGET_CAPS_INT64
	#define	SCEA_TARGET_CAPS_INT64				0
#endif
	#if !SCEA_TARGET_CAPS_INT64_HW
		#define	SCEA_TARGET_CAPS_INT64_HW		0
	#endif
	#if !SCEA_TARGET_CAPS_INT64_SW
		#define	SCEA_TARGET_CAPS_INT64_SW		0
	#endif
#if !SCEA_TARGET_CAPS_INT128
	#define	SCEA_TARGET_CAPS_INT128				0
#endif
	#if !SCEA_TARGET_CAPS_INT128_HW
		#define	SCEA_TARGET_CAPS_INT128_HW		0
	#endif
	#if !SCEA_TARGET_CAPS_INT128_SW
		#define	SCEA_TARGET_CAPS_INT128_SW		0
	#endif
#if !SCEA_TARGET_CAPS_F16
	#define	SCEA_TARGET_CAPS_F16				0
#endif
	#if !SCEA_TARGET_CAPS_F16_HW
		#define	SCEA_TARGET_CAPS_F16_HW			0
	#endif
	#if !SCEA_TARGET_CAPS_F16_SW
		#define	SCEA_TARGET_CAPS_F16_SW			0
	#endif
#if !SCEA_TARGET_CAPS_F32
	#define	SCEA_TARGET_CAPS_F32				0
#endif
	#if !SCEA_TARGET_CAPS_F32_HW
		#define	SCEA_TARGET_CAPS_F32_HW			0
	#endif
	#if !SCEA_TARGET_CAPS_F32_SW
		#define	SCEA_TARGET_CAPS_F32_SW			0
	#endif
#if !SCEA_TARGET_CAPS_F64
	#define	SCEA_TARGET_CAPS_F64				0
#endif
	#if !SCEA_TARGET_CAPS_F64_HW
		#define	SCEA_TARGET_CAPS_F64_HW			0
	#endif
	#if !SCEA_TARGET_CAPS_F64_SW
		#define	SCEA_TARGET_CAPS_F64_SW			0
	#endif
#if !SCEA_TARGET_CAPS_F80
	#define	SCEA_TARGET_CAPS_F80				0
#endif
	#if !SCEA_TARGET_CAPS_F80_HW
		#define	SCEA_TARGET_CAPS_F80_HW			0
	#endif
	#if !SCEA_TARGET_CAPS_F80_SW
		#define	SCEA_TARGET_CAPS_F80_SW			0
	#endif

#if !SCEA_TARGET_RT_ELF
	#define SCEA_TARGET_RT_ELF					0
#endif
#if !SCEA_TARGET_RT_ELF_LINUX
	#define SCEA_TARGET_RT_ELF_LINUX			0
#endif
#if !SCEA_TARGET_RT_ELF_PS2_EE
	#define SCEA_TARGET_RT_ELF_PS2_EE			0
#endif
#if !SCEA_TARGET_RT_ELF_PS2_ERX
	#define SCEA_TARGET_RT_ELF_PS2_ERX			0
#endif
#if !SCEA_TARGET_RT_ELF_PS2_IRX
	#define SCEA_TARGET_RT_ELF_PS2_IRX			0
#endif
#if !SCEA_TARGET_RT_ELF_PSP_PRX
	#define SCEA_TARGET_RT_ELF_PSP_PRX			0
#endif
#if !SCEA_TARGET_RT_PECOFF
	#define SCEA_TARGET_RT_PECOFF				0
#endif
#if !SCEA_TARGET_RT_MACHO
	#define SCEA_TARGET_RT_MACHO				0
#endif
#if !SCEA_TARGET_RT_XCOFF
	#define SCEA_TARGET_RT_XCOFF				0
#endif

#if !SCEA_TARGET_OS_PS1
	#define SCEA_TARGET_OS_PS1					0
#endif
#if !SCEA_TARGET_OS_PS2
	#define SCEA_TARGET_OS_PS2					0
#endif
    #if !SCEA_TARGET_OS_PS2_EE
        #define SCEA_TARGET_OS_PS2_EE			0
    #endif
    #if !SCEA_TARGET_OS_PS2_IOP
        #define SCEA_TARGET_OS_PS2_IOP			0
    #endif
#if !SCEA_TARGET_OS_PSP
	#define SCEA_TARGET_OS_PSP					0
#endif
#if !SCEA_TARGET_OS_PS3
	#define SCEA_TARGET_OS_PS3					0
#endif
	#if !SCEA_TARGET_OS_PS3_PU
		#define SCEA_TARGET_OS_PS3_PU			0
	#endif
	#if !SCEA_TARGET_OS_PS3_SPU
		#define SCEA_TARGET_OS_PS3_SPU			0
	#endif
#if !SCEA_TARGET_OS_WIN32
	#define SCEA_TARGET_OS_WIN32				0
#endif
#if !SCEA_TARGET_OS_MAC
	#define SCEA_TARGET_OS_MAC					0
#endif
#if !SCEA_TARGET_OS_UNIX
	#define SCEA_TARGET_OS_UNIX					0
#endif
    #if !SCEA_TARGET_OS_UNIX_MACOSX
        #define SCEA_TARGET_OS_UNIX_MACOSX		0
    #endif
    #if !SCEA_TARGET_OS_UNIX_AIX
        #define SCEA_TARGET_OS_UNIX_AIX			0
    #endif
	#if !SCEA_TARGET_OS_UNIX_CYGWIN
		#define	SCEA_TARGET_OS_UNIX_CYGWIN		0
	#endif
	#if !SCEA_TARGET_OS_UNIX_LINUX
		#define	SCEA_TARGET_OS_UNIX_LINUX		0
	#endif

#endif	//_SCEA_TARGETMACROS_H

/*****************************************************************
 *
 * $Log: SCEATargetMacros.h,v $
 * Revision 1.2.3 2004/12/03 XX:XX:XX jsproul
 * Added SCEA_TARGET_CPU_MIPS_ALLEGREX_EMU for PSP emulator. Added
 * missing CPU and TARGET_OS macros for Metrowerks PS2 and PSP.
 *
 * Revision 1.2.2 2004/12/02 XX:XX:XX jsproul
 * Added SCEA_TARGET_CAPS... macros, and extended usage comments.
 * Fixed bug where SCEA_HOST_COMPILER_GCC_SN was not being set using
 * PSPSNC when only the preprocessor was being run.
 *
 * Revision 1.2.1 2004/11/24 XX:XX:XX jsproul
 * Added ELF macros for PS2 and PSP executable formats. Fixed some
 * problems with detecting target CPU using MSVC compilers.
 *
 * Revision 1.2 2004/11/1 XX:XX:XX  jsproul
 * added macros for Cygwin and Linux OS
 *
 * Revision 1.1	2004/10/1 XX:XX:XX	dhilton
 * added distinction for target under metrowerks section
 * added defines for SN compilers
 *
 * Revision 0	2004/08/19 XX:XX:XX	alexr
 * first created
 *
 *****************************************************************/
