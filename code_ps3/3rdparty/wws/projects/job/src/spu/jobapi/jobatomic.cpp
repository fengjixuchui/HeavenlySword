/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Atomic functions in an interrupt safe manner
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobatomic.h>
#include <cell/spurs/types.h>

//--------------------------------------------------------------------------------------------------

U32 JobAtomicAdd32( U32 ea, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] += value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicAdd64( U32 ea, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] += value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicAnd32( U32 ea, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] &= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicAnd64( U32 ea, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] &= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicCompareAndSwap32( U32 ea, U32 compare, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		if ( old != compare )
			return old;
		ls[i]  = value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicCompareAndSwap64( U32 ea, U64 compare, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		if ( old != compare )
			return old;
		ls[i]  = value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicDecr32( U32 ea )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		--ls[i];

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicDecr64( U32 ea )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		--ls[i];

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicIncr32( U32 ea )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		++ls[i];

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicIncr64( U32 ea )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		++ls[i];

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicNop32( U32 ea )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
	mfc_read_atomic_status();
	EnableInterrupts();

	old = ls[i];

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicNop64( U32 ea )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	si_wrch( MFC_LSA, si_from_ptr(ls) );
	si_wrch( MFC_EAL, si_from_uint(ea) );
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
	mfc_read_atomic_status();
	EnableInterrupts();

	old = ls[i];

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicOr32( U32 ea, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] |= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicOr64( U32 ea, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] |= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicStore32( U32 ea, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] = value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicStore64( U32 ea, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] = value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicSub32( U32 ea, U32 value )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] -= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicSub64( U32 ea, U64 value )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		ls[i] -= value;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U32 JobAtomicTestAndDecr32( U32 ea )
{
	U32* ls = (U32*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U32);
	U32 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x3) == 0 );	//word aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		if ( old == 0 )
			return old;
		ls[i]  = old - 1;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------

U64 JobAtomicTestAndDecr64( U32 ea )
{
	U64* ls = (U64*)CELL_SPURS_LOCK_LINE;

	U32 i = (ea & 0x7F) / sizeof(U64);
	U64 old;

	WWSJOB_ASSERT( ((uintptr_t)ea & 0x7) == 0 );	//dword aligned

	ea &= ~0x7f;

	DisableInterrupts();
	do
	{
		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD)) );
		mfc_read_atomic_status();

		old = ls[i];
		if ( old == 0 )
			return old;
		ls[i]  = old - 1;

		si_wrch( MFC_LSA, si_from_ptr(ls) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD)) );
	} while ( WWSJOB_PREDICT( mfc_read_atomic_status(), 0 ) );
	EnableInterrupts();

	return old;
}

//--------------------------------------------------------------------------------------------------
