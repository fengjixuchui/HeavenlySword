/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manager accumulating audits in LS and sending them back to main memory
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_AUDIT_WRITER_H
#define WWS_JOB_AUDIT_WRITER_H

//--------------------------------------------------------------------------------------------------

#if defined(ENABLE_WORKLOAD_AUDITS) || defined(ENABLE_TIMING_AUDITS)
	#define STORE_WORKLOAD_AUDIT StoreAudit
#else
	#define STORE_WORKLOAD_AUDIT( ... ) do { } while ( false )
#endif

#ifdef ENABLE_TIMING_AUDITS
	#define STORE_TIMING_AUDIT StoreAudit
#else
	#define STORE_TIMING_AUDIT( ... ) do { } while ( false )
#endif

//--------------------------------------------------------------------------------------------------

#ifdef ENABLE_IMPORTANT_AUDITS
	#define STORE_IMPORTANT_AUDIT StoreAudit
#else
	#define STORE_IMPORTANT_AUDIT( ... ) do { } while ( false )
#endif

//--------------------------------------------------------------------------------------------------

#ifdef ENABLE_VERBOSE_AUDITS
	#define STORE_VERBOSE_AUDIT StoreAudit
#else
	#define STORE_VERBOSE_AUDIT( ... ) do { } while ( false )
#endif

//--------------------------------------------------------------------------------------------------

//The are the valid functions for the Job manager to call
extern "C" void InitAudits( U32 spuNum, U32 eaAuditBufferArrayBase );
void StoreAudit( U16 id, U16 hword=0 );
void StoreAudit( U16 id, U16 hword, U64 dword0 );
void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1 );
void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2 );
void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3 );
void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4 );
void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4, U64 dword5 );
void StoreAudit( U16 id, U16 numDwords, U64 *pData );
extern "C" void FinalizeAudits( void );
Bool32 AreJobAuditsEnabled( void );

#ifdef ENABLE_VERBOSE_AUDITS
void WwsJob_StoreEnableInterruptsAuditCheckingTightLoop( void );
#endif

//--------------------------------------------------------------------------------------------------

//Internal functions used by the StoreAudit implementation
void StoreAuditInternal( U16 id, U16 hword );
void StoreAuditInternal( U16 id, U16 hword, U16 numDwords, U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4, U64 dword5 );
void StoreAuditInternal( U16 id, U16 numDwords, U64 *pData );

//--------------------------------------------------------------------------------------------------

extern "C"
{
extern Bool32 g_bJobAuditsEnabled;
extern Bool32 g_bJobManagerAuditsEnabled;
}

//--------------------------------------------------------------------------------------------------

inline Bool32 AreJobAuditsEnabled( void )
{
	return g_bJobAuditsEnabled;
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 1, dword0, 0, 0, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 2, dword0, dword1, 0, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 3, dword0, dword1, dword2, 0, 0, 0 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 4, dword0, dword1, dword2, dword3, 0, 0 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 5, dword0, dword1, dword2, dword3, dword4, 0 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit( U16 id, U16 hword, U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4, U64 dword5 )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, hword, 6, dword0, dword1, dword2, dword3, dword4, dword5 );
}

//--------------------------------------------------------------------------------------------------

inline void StoreAudit ( U16 id, U16 numDwords, U64 *pData )
{
	if ( g_bJobManagerAuditsEnabled )
		StoreAuditInternal( id, numDwords, pData );
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_AUDIT_WRITER_H */
