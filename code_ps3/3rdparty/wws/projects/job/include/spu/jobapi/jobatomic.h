/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Atomic functions in an interrupt safe manner

	@note		Unlike the cellAtomic functions, you don't need to pass in a 128-byte temp buffer
				because I can just use the same one as used by the job manager and spurs kernel
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_ATOMIC_H
#define WWS_JOB_JOB_ATOMIC_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>
#include <spu_mfcio.h>
#include <jobapi/spuinterrupts.h>

//--------------------------------------------------------------------------------------------------

U32 JobAtomicAdd32( U32 ea, U32 value );
U64 JobAtomicAdd64( U32 ea, U64 value );
U32 JobAtomicAnd32( U32 ea, U32 value );
U64 JobAtomicAnd64( U32 ea, U64 value );
U32 JobAtomicCompareAndSwap32( U32 ea, U32 compare, U32 value );
U64 JobAtomicCompareAndSwap64( U32 ea, U64 compare, U64 value );
U32 JobAtomicDecr32( U32 ea );
U64 JobAtomicDecr64( U32 ea );
U32 JobAtomicIncr32( U32 ea );
U64 JobAtomicIncr64( U32 ea );
U32 JobAtomicNop32( U32 ea );
U64 JobAtomicNop64( U32 ea );
U32 JobAtomicOr32( U32 ea, U32 value );
U64 JobAtomicOr64( U32 ea, U64 value );
U32 JobAtomicStore32( U32 ea, U32 value );
U64 JobAtomicStore64( U32 ea, U64 value );
U32 JobAtomicSub32( U32 ea, U32 value );
U64 JobAtomicSub64( U32 ea, U64 value );
U32 JobAtomicTestAndDecr32( U32 ea );
U64 JobAtomicTestAndDecr64( U32 ea );

//--------------------------------------------------------------------------------------------------

inline U32 JobAtomicLockLine32( U32* ls, U32 ea )
{
	unsigned int i = (ea & 0x7F) >> 2;

	WWSJOB_ASSERT( ((uintptr_t)ls & 0x7F) == 0 );
	ea &= ~0x7F;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
	mfc_read_atomic_status();
	EnableInterrupts();

	return ls[i];
}

//--------------------------------------------------------------------------------------------------

inline U64 JobAtomicLockLine64( U64* ls, U32 ea )
{
	unsigned int i = (ea & 0x7F) >> 3;

	WWSJOB_ASSERT( ((uintptr_t)ls & 0x7F) == 0 );
	ea &= ~0x7F;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
	mfc_read_atomic_status();
	EnableInterrupts();

	return ls[i];
}

//--------------------------------------------------------------------------------------------------

inline int JobAtomicStoreConditional32( U32* ls, U32 ea, U32 value )
{
	unsigned int i = (ea & 0x7F) >> 2;

	WWSJOB_ASSERT( ((uintptr_t)ls & 0x7F) == 0 );
	ls[i] = value;
	ea &= ~0x7F;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	int ret = mfc_read_atomic_status();
	EnableInterrupts();

	return ret;
}

//--------------------------------------------------------------------------------------------------

inline int JobAtomicStoreConditional64( U64* ls, U32 ea, U64 value )
{
	unsigned int i = (ea & 0x7F) >> 3;

	WWSJOB_ASSERT( ((uintptr_t)ls & 0x7F) == 0 );
	ls[i] = value;
	ea &= ~0x7F;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	int ret = mfc_read_atomic_status();
	EnableInterrupts();

	return ret;
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_ATOMIC_H */
