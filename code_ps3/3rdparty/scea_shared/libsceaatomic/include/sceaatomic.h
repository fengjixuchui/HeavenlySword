/*
 *  sceaatomic.h
 *
 *  Created by Alex Rosenberg on 5/13/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 *	GCC voodoo. Even the GCC, STI and SCEI engineers get this stuff slightly wrong.
 *
 *	TODO
 *		rework SPU portion when 32-bit pointers arrive on PPU
 *		port to SNC and MWERKS when available
 *		x86 VC++ fastcall?
 */

#ifndef __SCEAATOMIC_H__
#define __SCEAATOMIC_H__ 1

#ifndef SCEAATOMIC_CONFIG_DSYNC_REQUIRED
	#define SCEAATOMIC_CONFIG_DSYNC_REQUIRED	1
#endif

#ifndef SCEAATOMIC_CONFIG_SSE_BARRIERS
	#define SCEAATOMIC_CONFIG_SSE_BARRIERS		0
#endif

#include "sceacommon/include/sceabasetypes.h"

#if !(SCEA_TARGET_CPU_PPC || SCEA_TARGET_CPU_CELL_SPU || SCEA_TARGET_CPU_X86 || SCEA_TARGET_CPU_MIPS)
	#error sceaatomic.h included on wrong platform/compiler
#endif

#if SCEA_TARGET_CPU_PPC
	#if SCEA_TARGET_OS_PS3
		#include <sdk_version.h>
	#endif
	#if CELL_SDK_VERSION > 0x084000
		#include <ppu_intrinsics.h>
	#else
		#include "scea_ppu_intrinsics.h"
	#endif
#elif SCEA_TARGET_CPU_CELL_SPU
	#include <cell/atomic.h>
#endif

//#define SCEAATOMIC_INLINE __attribute__((always_inline)) inline
#define SCEAATOMIC_INLINE inline

#if SCEAATOMIC_CONFIG_DSYNC_REQUIRED
	//!!! The BPA/SPU Architecture allows for LS accesses to be cached. In order for MFC
	//!!! operations to be consistent with the contents of that cache, a dsync is required.
	//!!! However, as of Cell DD2.0, dsync is just a nop. Inoue-san says that the STI folks
	//!!! are considering redefining the architecture to eliminate this need.
	//!!! Until such a decision is made, we _must_ keep the dsync instructions in because
	//!!! otherwise we may sacrifice compatibility with future Cell revisions (possibly ones
	//!!! that may happen after PS3 ships such as die shrinks, cost reductions, PS4, etc.).
	#define SPU_DSYNC() spu_dsync();
#else
	#define SPU_DSYNC() 
#endif

#if SCEA_TARGET_CPU_X86 && SCEA_HOST_COMPILER_MSVC
	extern "C" void _WriteBarrier();
	extern "C" void _ReadWriteBarrier();
	#pragma intrinsic(_WriteBarrier)
	#pragma intrinsic(_ReadWriteBarrier)
#endif

namespace SCEA { namespace Atomic {

/*
 * xx CompareAndSwap(volatile xx* p, xx old_value, xx new_value)
 *
 * This function atomically reads from the memory location pointed to by p and
 * if that value is equal to old_value, it stores new_value into it. It always
 * returns the value that was in *p.
 *
 * xx TestAndSet(volatile xx* p, xx new_value)
 *
 * This function atomically reads from the memory location pointed to by p and
 * if that value is zero, it stores new_value into it. It always returns the
 * value that was in *p.
 *
 * SCEAATOMIC_READ_BARRIER
 * SCEAATOMIC_WRITE_BARRIER
 * SCEAATOMIC_READWRITE_BARRIER
 * 
 * These macros are used prior to any atomic or DMA put operation. Select according
 * to the type of operations preceeding the barrier that must be completed prior
 * to peforming the atomic or DMA put operation.
 */

#if SCEA_TARGET_CPU_PPC	
/*================================================================================
 *=
 *= PowerPC
 *=
 *================================================================================*/	
	
#define SCEAATOMIC_READ_BARRIER()		__lwsync()
#define SCEAATOMIC_WRITE_BARRIER()		__eieio()
#define SCEAATOMIC_READWRITE_BARRIER()	__lwsync()
	
static SCEAATOMIC_INLINE U32 CompareAndSwap(volatile U32* p, U32 old_value, U32 new_value)
{
	U32 old;
	
	asm ("# SCEA::CompareAndSwap(U32 p=%[p],old=%[old],old_value=%[old_value],new_value=%[new_value])\n"
		 ".Lloop%=:\n"
		 "	lwarx %[old],0,%[p]\n"
		 "	cmplw%I[old_value] %[old],%b[old_value]\n"
		 "	bne- .Ldone%=\n"
		 "	stwcx. %[new_value],0,%[p]\n"
		 "	bne- .Lloop%=\n"
		 ".Ldone%=:"
		 : [old] "=&r,&r" (old), "+m,m" (*p)
		 : [p] "r,r" (p), [old_value] "r,K" (old_value), [new_value] "r,r" (new_value), "m,m" (*p)
		 : "cr0", "memory");
	
	//!!! when gcc is upgraded to 4.1, change to "+Z" output constraint for p and "%y1" for print operand
	return old;
}

static SCEAATOMIC_INLINE U64 CompareAndSwap(volatile U64* p, U64 old_value, U64 new_value)
{
	U64 old;
	
#if defined(__PPU__) || defined(__ppc64__) || defined(__powerpc64__)
	asm ("# SCEA::CompareAndSwap(U64 p=%[p],old=%[old],old_value=%[old_value],new_value=%[new_value])\n"
		 ".Lloop%=:\n"
		 "	ldarx %[old],0,%[p]\n"
		 "	cmpld%I[old_value] %[old],%b[old_value]\n"
		 "	bne .Ldone%=\n"
		 "	stdcx. %[new_value],0,%[p]\n"
		 "	bne .Lloop%=\n"
		 ".Ldone%=:"
		 : [old] "=&r,&r" (old), "+m,m" (*p)
		 : [p] "r,r" (p), [old_value] "r,K" (old_value), [new_value] "r,r" (new_value), "m,m" (*p)
		 : "cr0", "memory");
	//!!! when gcc is upgraded to 4.1, change to "+Z" output constraint for p and "%y1" for print operand
#else
	#warning "Nonproduction nonatomic code! ppc64 instruction required (ldarx) but targeting ppc32."
	// This case is only for prototyping on ppc32 machines like a PowerBook
	old = *p;
    if( old == old_value )
        *p = new_value;
    return old;
#endif
	
	return old;
}

static SCEAATOMIC_INLINE I32 CompareAndSwap(volatile I32* p, I32 old_value, I32 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U32*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE I64 CompareAndSwap(volatile I64* p, I64 old_value, I64 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U64*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE U32 TestAndSet(volatile U32* p, U32 new_value)
{
	U32 old;
	
	asm ("# SCEA::TestAndSet(U32 p=%[p],old=%[old],new_value=%[new_value])\n"
		 ".Lloop%=:\n"
		 "	lwarx %[old],0,%[p]\n"
		 "	cmplwi %[old],0\n"
		 "	bne .Ldone%=\n"
		 "	stwcx. %[new_value],0,%[p]\n"
		 "	bne .Lloop%=\n"
		 ".Ldone%=:"
		 : [old] "=&r" (old), "+m" (*p)
		 : [p] "r" (p), [new_value] "r" (new_value), "m" (*p)
		 : "cr0", "memory");
	//!!! when gcc is upgraded to 4.1, change to "+Z" output constraint for p and "%y1" for print operand
    return old;
}

static SCEAATOMIC_INLINE U64 TestAndSet(volatile U64* p, U64 new_value)
{
	U64 old;
#if defined(__PPU__) || defined(__ppc64__) || defined(__powerpc64__)
	asm (
		"# SCEA::TestAndSet(U64 p=%[p],old=%[old],new_value=%[new_value])\n"
		".Lloop%=:\n"
		"	ldarx %[old],0,%[p]\n"
		"	cmpldi %[old],0\n"
		"	bne .Ldone%=\n"
		"	stdcx. %[new_value],0,%[p]\n"
		"	bne .Lloop%=\n"
		".Ldone%=:"
		: [old] "=&r" (old), "+m" (*p)
		: [p] "r" (p), [new_value] "r" (new_value), "m" (*p)
		: "cr0", "memory"
	);
	//!!! when gcc is upgraded to 4.1, change to "+Z" output constraint for p and "%y1" for print operand
#else
	#warning "Nonproduction nonatomic code! ppc64 instruction required (ldarx) but targeting ppc32."
	old = *p;
	if( 0x0000000000000000LL == *p ) {
		*p = new_value;
	}
#endif
    return old;
}

static SCEAATOMIC_INLINE I32 TestAndSet(volatile I32* p, I32 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U32*>(p), new_value);
}

static SCEAATOMIC_INLINE I64 TestAndSet(volatile I64* p, I64 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U64*>(p), new_value);
}

#elif SCEA_TARGET_CPU_CELL_SPU
/*================================================================================
*=
*= Cell SPU
*=
*================================================================================*/	

#define SCEAATOMIC_READ_BARRIER()		SPU_DSYNC()
#define SCEAATOMIC_WRITE_BARRIER()		SPU_DSYNC()
#define SCEAATOMIC_READWRITE_BARRIER()	SPU_DSYNC()

extern U32 CompareAndSwap(U64 p, U32 old_value, U32 new_value);	// libsceaatomic.a
extern U64 CompareAndSwap(U64 p, U64 old_value, U64 new_value);	// libsceaatomic.a

static SCEAATOMIC_INLINE I32 CompareAndSwap(U64 p, I32 old_value, I32 new_value)
{
	return CompareAndSwap(p, static_cast<U32>(old_value), static_cast<U32>(new_value));
}

static SCEAATOMIC_INLINE I64 CompareAndSwap(U64 p, I64 old_value, I64 new_value)
{
	return CompareAndSwap(p, static_cast<U64>(old_value), static_cast<U64>(new_value));
}

extern U32 TestAndSet(U64 p, U32 new_value);	// libsceaatomic.a
extern U64 TestAndSet(U64 p, U64 new_value);	// libsceaatomic.a

static SCEAATOMIC_INLINE I32 TestAndSet(U64 p, I32 new_value)
{
	return TestAndSet(p, static_cast<U32>(new_value));
}

static SCEAATOMIC_INLINE I64 TestAndSet(U64 p, I64 new_value)
{
	return TestAndSet(p, static_cast<U64>(new_value));
}

#elif SCEA_TARGET_CPU_X86
/*================================================================================
*=
*= x86
*=
*================================================================================*/	

#if SCEA_HOST_COMPILER_MSVC
	#define SCEAATOMIC_READ_BARRIER()		_ReadWriteBarrier()
	#define SCEAATOMIC_WRITE_BARRIER()		_WriteBarrier()
	#define SCEAATOMIC_READWRITE_BARRIER()	_ReadWriteBarrier()
#elif SCEA_HOST_COMPILER_GCC
	// lfence, sfence, or mfence may be necessary (respectively) for some SSE memory operations
	#if SCEAATOMIC_CONFIG_SSE_BARRIERS
		#define SCEAATOMIC_READ_BARRIER()		__asm__ volatile ("lfence" : : : "memory")
		#define SCEAATOMIC_WRITE_BARRIER()		__asm__ volatile ("sfence" : : : "memory")
		#define SCEAATOMIC_READWRITE_BARRIER()	__asm__ volatile ("mfence" : : : "memory")
	#else
		#define SCEAATOMIC_READ_BARRIER()		__asm__ volatile ("" : : : "memory")
		#define SCEAATOMIC_WRITE_BARRIER()		__asm__ volatile ("" : : : "memory")
		#define SCEAATOMIC_READWRITE_BARRIER()	__asm__ volatile ("" : : : "memory")
	#endif
#endif

static SCEAATOMIC_INLINE U32 CompareAndSwap(volatile U32* p, U32 old_value, U32 new_value)
{
#if SCEA_HOST_COMPILER_MSVC
	__asm {
		mov eax, old_value;
		mov ecx, p;
		mov edx, new_value;
		lock cmpxchg [ecx], edx;
		mov old_value, eax;
	}
#else
	asm volatile (
		"# SCEA::CompareAndSwap(U32 p=%[p],new_value=%[new_value])\n"
		"lock;cmpxchgl %[new_value],%[p]"
		: [accumulator] "+a" (old_value)
		: [new_value]   "r"  (new_value),
		  [p]           "m"  (*(p))
		: "memory"
	);
#endif
	return old_value;
}


static SCEAATOMIC_INLINE U64 CompareAndSwap(volatile U64* p, U64 old_value, U64 new_value)
{
#if SCEA_HOST_COMPILER_MSVC
	__asm {
		lea edi,new_value;
		mov ebx,[edi];
		mov ecx,4[edi];

		lea edi,old_value;
		mov eax,[edi];
		mov edx,4[edi];

		mov esi, p;
		lock cmpxchg8b [esi];

		mov [edi],eax;
		mov 4[edi],edx;
	}
#else
	asm volatile (
		"# SCEA::CompareAndSwap(U64 p=%[p])\n"
		"xchgl %%edi,%%ebx\n"
		"movl %[p], %%esi\n"
		"lock;cmpxchg8b (%%esi)\n"
		"movl %%edi,%%ebx\n"
		: "+A" (old_value)
		: "c" ((U32)(new_value >> 32)),
		  "D" ((U32)(new_value & 0xFFFFFFFF)),
		  [p] "m" (p)
		: "memory","%esi"
	);
#endif
	return old_value;
}

static SCEAATOMIC_INLINE I32 CompareAndSwap(volatile I32* p, I32 old_value, I32 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U32*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE I64 CompareAndSwap(volatile I64* p, I64 old_value, I64 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U64*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE U32 TestAndSet(volatile U32* p, U32 new_value)
{
	U32 old_value = 0;
#if SCEA_HOST_COMPILER_MSVC
	__asm {
		mov eax, old_value;
		mov ecx, p;
		mov edx, new_value;
		lock cmpxchg [ecx], edx;
		mov old_value, eax;
	}
#else
	asm volatile (
		"# SCEA::TestAndSet(U32 p=%[p],new_value=%[new_value])\n"
		"lock;cmpxchgl %[new_value],%[p]"
		: [accumulator] "+a" (old_value)
		: [new_value]   "r"  (new_value),
		  [p]           "m"  (*(p))
		: "memory"
	);
#endif
	return old_value;
}

static SCEAATOMIC_INLINE U64 TestAndSet(volatile U64* p, U64 new_value)
{
	U64 old_value = 0x0000000000000000LL;
#if SCEA_HOST_COMPILER_MSVC
	__asm {
		lea edi,new_value;
		mov ebx,[edi]
		mov ecx,4[edi]

		lea edi,old_value;
		mov eax,[edi]
		mov edx,4[edi]

		mov esi,p;
		lock cmpxchg8b [esi];

		mov [edi],eax;
		mov 4[edi],edx;
	}
#else
	asm volatile (
		"# SCEA::TestAndSet(U64 p=%[p])\n"
		"xchgl %%edi,%%ebx\n"
		"movl %[p], %%esi\n"
		"lock;cmpxchg8b (%%esi)\n"
		"movl %%edi,%%ebx\n"
		: "+A" (old_value)
		: "c" ((U32)(new_value >> 32)),
		  "D" ((U32)(new_value & 0xFFFFFFFF)),
		  [p] "m" (p)
		: "memory","%esi"
	);
#endif
	return old_value;
}

static SCEAATOMIC_INLINE I32 TestAndSet(volatile I32* p, I32 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U32*>(p), new_value);
}

static SCEAATOMIC_INLINE I64 TestAndSet(volatile I64* p, I64 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U64*>(p), new_value);
}

#elif SCEA_TARGET_CPU_MIPS
/*================================================================================
*=
*= MIPS (PSP)
*=
*================================================================================*/	

/*
 * MIPS implementation note:
 * PS1 uses R3000, PS2 uses R5900, PSP uses Allegrex.
 * All of these lack 64-bit lld/scd atomic instructions. The R3000 and R5900
 * lack 32-bit ll/sc atomic instructions.
 * When atomic instructions aren't available, interrupts are disabled instead.
 */

#if SCEA_TARGET_CPU_MIPS_R3000 || SCEA_TARGET_CPU_MIPS_R5900
#error "libsceaatomic is currently unqualified for PS1 and PS2"
#endif

#include <intrman.h> // for sceKernelCpuSuspendIntr() and sceKernelCpuResumeIntr().

#define SCEAATOMIC_READ_BARRIER()		asm volatile ("" : : : "memory")
#define SCEAATOMIC_WRITE_BARRIER()		asm volatile ("" : : : "memory")
#define SCEAATOMIC_READWRITE_BARRIER()	asm volatile ("" : : : "memory")

static SCEAATOMIC_INLINE U32 CompareAndSwap(volatile U32* p, U32 old_value, U32 new_value)
{
	U32 old;
	
	asm volatile (
		"retry1:					\n"	//	
		"	ll		%0, %1			\n"	//	old = *p
		"	bne		%0, %2, fail1	\n"	//	if old != old_value goto fail
		"	move	$t9, %3			\n"	//	tmp ($t9) = new_value
		"	sc		$t9, %1			\n"	//	*p = tmp
		"	beqz	$t9, retry1		\n"	//	if tmp == 0 goto retry
		"	nop						\n"	//	
		"fail1:						\n"	//	
		: "=r" (old)		//	%0
		: "m" (*p)			//	%1
		  "r" (old_value),	//	%2
		  "r" (new_value)	//	%3
		: "memory", "$t9"
	);
	
	return old;
}

static SCEAATOMIC_INLINE U64 CompareAndSwap(volatile U64* p, U64 old_value, U64 new_value)
{
	// All Playstation MIPS processors lack lld/scd. Disable interrupts as a workaround.
	int oldStat = sceKernelCpuSuspendIntr();
		U64 old = *p;
		if( old == old_value )
			*p = new_value;
	sceKernelCpuResumeIntr( oldStat );
	return old;
}

static SCEAATOMIC_INLINE I32 CompareAndSwap(volatile I32* p, I32 old_value, I32 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U32*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE I64 CompareAndSwap(volatile I64* p, I64 old_value, I64 new_value)
{
	return CompareAndSwap(reinterpret_cast<volatile U64*>(p), old_value, new_value);
}

static SCEAATOMIC_INLINE U32 TestAndSet(volatile U32* p, U32 new_value)
{
	U32 old;

	asm volatile (
		"retry2:					\n"	//	
		"	ll		%0, %1			\n"	//	old = *p
		"	bne		%0, $0, fail2	\n"	//	if old != 0x00000000 goto fail
		"	nop						\n"	//	
		"	move	$t9, %2			\n"	//	tmp ($t9) = new_value
		"	sc		$t9, %1			\n"	//	*p = tmp
		"	beqz	$t9, retry2		\n"	//	if tmp == 0 goto retry
		"	nop						\n"	//	
		"fail2:						\n"	//	
		: "=r" (old)		//	%0
		: "m" (*p)			//	%1
		  "r" (new_value)	//	%2
		: "memory", "$t9"
	);
	
	return old;
}

static SCEAATOMIC_INLINE U64 TestAndSet(volatile U64* p, U64 new_value)
{
	// All Playstation MIPS processors lack lld/scd. Disable interrupts as a workaround.
	int oldStat = sceKernelCpuSuspendIntr();
		U64 old = *p;
		if( 0x0000000000000000LL == old )
			*p = new_value;
	sceKernelCpuResumeIntr( oldStat );
	return old;
}

static SCEAATOMIC_INLINE I32 TestAndSet(volatile I32* p, I32 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U32*>(p), new_value);
}

static SCEAATOMIC_INLINE I64 TestAndSet(volatile I64* p, I64 new_value)
{
	return TestAndSet(reinterpret_cast<volatile U64*>(p), new_value);
}

#endif // processor type

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_H__
