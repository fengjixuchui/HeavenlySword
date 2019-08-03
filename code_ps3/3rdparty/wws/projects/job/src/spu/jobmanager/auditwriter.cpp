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

#include <cell/dma.h>
#include <sys/return_code.h>
#include <cell/spurs/types.h>
#include <cell/spurs/policy_module.h>
#include <cell/spurs/ready_count.h>
#include <spu_mfcio.h>
#include <stddef.h>
#include <sys/spu_event.h>

#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobmanagerauditids.h>
#include <jobapi/jobapi.h>
#include <jobapi/spumoduleheader.h>
#include <jobapi/jobdefinition.h>
#include <jobmanager/interrupthandler.h>
#include <jobmanager/allocatejob.h>
#include <jobmanager/auditwriter.h>
#include <jobmanager/jobmanagerdmatags.h>
#include <jobmanager/spustack.h>
#include <jobmanager/jobheadercache.h>
#include <jobmanager/jobmanager.h>
#include <jobmanager/data.h>

//--------------------------------------------------------------------------------------------------

static U32 GetTime( void );
#if WWS_JOB_USE_C_VERSION!=0
static void StoreAuditDataU64 ( U64 audit );
#ifndef IMMEDIATE_AUDIT_OUTPUT
static void OutputAuditBlock( void );
#endif
#endif

//--------------------------------------------------------------------------------------------------

union AuditEtc
{
	Audit	m_audit;
	U64		m_u64;
	U32		m_u32[2];
	U16		m_u16[4];
	U8		m_u8[8];
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

#ifdef ENABLE_VERBOSE_AUDITS
// these 2 are to avoid infinite loop of enableInterrupt-disableInterrupt-intermediateAudit
static AuditEtc 		*s_pEnableAudit0;
static AuditEtc 		*s_pEnableAudit1;
#endif

//====================================================================================================================
//
//	audits
//
//====================================================================================================================

inline U32 GetTime( void )
{
  #if 0 // 1 = hack to force time to just increment by 1.  0 is normal
	static U32 s_timeJustIncrements;
	return s_timeJustIncrements++;
  #else
	// NOTE: timer is inverted, so that it increments
	return ~spu_readch(SPU_RdDec);	//NOTE: >= 20 cycle delay ...
  #endif
}

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
void InitAudits( U32 spuNum, U32 eaAuditBufferArrayBase )
{
	U32 mmaAuditBufferHeader = eaAuditBufferArrayBase;

	//JobPrintf( "InitAudits: mmaAuditBufferHeader = 0x%08X, eachSpuBufferSize = 0x%08X\n", mmaAuditBufferHeader, eachSpuBufferSize );

	//The buffers for the audits to write out to all immediately follow contiguously on after the AuditBufferHeader
	// get start of ls audits
	g_lsaAudits = (U32)&g_auditBlocks[0][0];

	if ( mmaAuditBufferHeader )
	{
		//We used to use a separate buffer for reading the AuditBufferHeader into,
		//but since we only need it temporarily during initialization, and it's
		//smaller than the audit blocks buffer, we can read it into there instead
		WWSJOB_VERBOSE_ASSERT( sizeof(AuditBufferHeader) <= sizeof(g_auditBlocks) );
		AuditBufferHeader* pAuditBufferHeader = (AuditBufferHeader*)(void*)&g_auditBlocks;
		//AuditBufferHeader* pAuditBufferHeader = &g_auditBufferHeader;

		//Fetch audit buffer header info
		DMA_READ( pAuditBufferHeader, mmaAuditBufferHeader, sizeof(AuditBufferHeader), DmaTagId::kBlockingLoad );
		cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );
		g_bJobManagerAuditsEnabled	= pAuditBufferHeader->m_inputHeader.m_bJobManagerAuditsEnabled;
		g_bJobAuditsEnabled			= pAuditBufferHeader->m_inputHeader.m_bJobAuditsEnabled;
		U32 bufferNumOffset			= pAuditBufferHeader->m_inputHeader.m_bufferNumOffset;
		U32 auditBufferIndex		= bufferNumOffset + spuNum;
		U32 numDwordsWritten		= pAuditBufferHeader->m_outputHeader[auditBufferIndex].m_numDwordsWritten;
		U32 eachSpuBufferSize		= pAuditBufferHeader->m_inputHeader.m_eachSpuBufferSize;
		WWSJOB_VERBOSE_ASSERT( WwsJob_IsAligned( numDwordsWritten, NUM_AUDITS_PER_BLOCK ) );	//Make sure it's a whole number of audit blocks that have been written to main memory
		//JobPrintf( "InitAudits: numDwordsWritten = %d\n", numDwordsWritten );

		//The output buffer for this spu
		U32 mmaAuditsBegin	= mmaAuditBufferHeader + sizeof(AuditBufferHeader) + (eachSpuBufferSize * (auditBufferIndex));
		U32 mmaAuditsEnd	= mmaAuditsBegin + eachSpuBufferSize;

		// mm for audits must exist	
		//tmp	WWSJOB_ASSERT( mmaAuditsBegin < mmaAuditsEnd );

		// mma's of audits must be multiple of 128
		WWSJOB_VERBOSE_ASSERT( (mmaAuditsBegin & 0x7f) == 0  &&  (mmaAuditsEnd & 0x7f) == 0 );
		
		// size of mm audit memory MUST be multiple of AUDIT_BLOCK_SIZE
		WWSJOB_VERBOSE_ASSERT( ( (mmaAuditsEnd - mmaAuditsBegin) % AUDIT_BLOCK_SIZE == 0 ) );

		g_mmaAuditsBase	= mmaAuditsBegin;
		g_mmaAudits		= mmaAuditsBegin + (sizeof(Audit) * numDwordsWritten );
		g_mmaAuditsEndLessAuditBlockSizePlus1 = mmaAuditsEnd - AUDIT_BLOCK_SIZE + 1;

		if ( g_bJobManagerAuditsEnabled || g_bJobAuditsEnabled )
		{
			U32 mmaAuditBufferOutputHeader = mmaAuditBufferHeader
											+ sizeof(AuditBufferInputHeader)
											+ auditBufferIndex * sizeof(AuditBufferOutputHeader);
			g_mmaAuditBufferOutputHeader = mmaAuditBufferOutputHeader;
			vector unsigned int markAuditsAsActive;
			markAuditsAsActive = spu_splats( 0xFFFFFFFF );
			cellDmaPut( &markAuditsAsActive,
						mmaAuditBufferOutputHeader,
						sizeof(AuditBufferOutputHeader),
						DmaTagId::kBlockingLoad,
						0,
						0 );
			//cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );
			//Note that we don't actually need to bother to sync this dma here since
			//GetJobHeaderFromCache will do it due to the cache having been flushed
		}
	}
	else
	{
		g_bJobManagerAuditsEnabled = false;
		g_bJobAuditsEnabled = false;
		g_mmaAudits = 0;
		g_mmaAuditsEndLessAuditBlockSizePlus1 = 0;
	}

	//JobPrintf( "InitAudits: mmaAuditsBegin = 0x%08X, mmaAuditsEnd = 0x%08X\n", mmaAuditsBegin, mmaAuditsEnd );

	// size of each ls audit block must be >= 128
	WWSJOB_VERBOSE_ASSERT( AUDIT_BLOCK_SIZE >= 128 );		//Actually, this could be made a compile time audit


#ifdef ENABLE_VERBOSE_AUDITS
	// done to avoid tight loop of enable-disable audits (init with illegal value)
	s_pEnableAudit0 = (AuditEtc*)1;
	s_pEnableAudit1 = (AuditEtc*)1;
#endif
}
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
void StoreAuditInternal( U16 id, U16 hword )
{
	// create audit
	AuditEtc audit;
	audit.m_audit.m_time 		= GetTime();
	audit.m_audit.m_numDwords 	= 0;
	audit.m_audit.m_id   		= id;
	audit.m_audit.m_data 		= hword;

	// store audit to ls mem (guaranteed to exist in block), and possibly to main memory
	StoreAuditDataU64( audit.m_u64 );
}

//--------------------------------------------------------------------------------------------------

void StoreAuditInternal( U16 id, U16 hword, U16 numDwords, 
		U64 dword0, U64 dword1, U64 dword2, U64 dword3, U64 dword4, U64 dword5 )
{
	WWSJOB_VERBOSE_ASSERT( numDwords >= 1  &&  numDwords <= 6 );
	
	// create audit
	AuditEtc audit;
	audit.m_audit.m_time 		= GetTime();
	audit.m_audit.m_numDwords 	= numDwords;
	audit.m_audit.m_id   		= id;
	audit.m_audit.m_data 		= hword;

	// store audit to ls mem (guaranteed to exist in block), and possibly to main memory
	StoreAuditDataU64( audit.m_u64 );

	StoreAuditDataU64( dword0 );
	if( numDwords > 1 )
	{
		StoreAuditDataU64( dword1 );
		if( numDwords > 2 )
		{
			StoreAuditDataU64( dword2 );
			if( numDwords > 3 )
			{
				StoreAuditDataU64( dword3 );
				if( numDwords > 4 )
				{
					StoreAuditDataU64( dword4 );
					if( numDwords > 5 )
					{
						StoreAuditDataU64( dword5 );
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void StoreAuditInternal( U16 id, U16 numDwords, U64 *pData )
{
	// create audit
	AuditEtc audit;
	audit.m_audit.m_time 		= GetTime();
	audit.m_audit.m_numDwords 	= 7;
	audit.m_audit.m_id   		= id;
	audit.m_audit.m_data 		= numDwords;

	// store audit to ls mem (guaranteed to exist in block), and possibly to main memory
	StoreAuditDataU64( audit.m_u64 );

	// store audit data
	U64 *pDataEnd/*excl*/ = (U64*) ( (U32)pData + (numDwords << 3) );
	while( pData < pDataEnd )
	{
		StoreAuditDataU64( *pData++ );
	}
}


//--------------------------------------------------------------------------------------------------

void StoreAuditDataU64( U64 audit )
{
#ifdef IMMEDIATE_AUDIT_OUTPUT
	//If we want immediate mode audits then rather than storing the
	//U64 to an array in LS and dmaing out, this function instead
	//sends the U64 out as two U32s via events.
	//The PPU picks up these U32s and reconcatentate appropriately
	#define	SPU_PORT_IMMEDIATE_AUDIT_OUTPUT 14
	//const U32 kJobManagerEventPort = SPU_PORT_IMMEDIATE_AUDIT_OUTPUT;

	U32 dataHi = (audit >> 32LL);
	U32 dataLo = (audit & 0xFFFFFFFFULL);
	//U32 ret;
	//Note, these functions only actually send 24-bits+32bits in the two data elements,
	//hence we have to split our 64-bit into two 32-bits.
	//do
	//{
	//	ret = sys_spu_thread_send_event( kJobManagerEventPort, 0, dataHi );
	//} while ( EBUSY == (I32)ret );
	//WWSJOB_ASSERT( CELL_OK == ret );

	//do
	//{
	//	ret = sys_spu_thread_send_event( kJobManagerEventPort, 0, dataLo );
	//} while ( EBUSY == (I32)ret );
	//WWSJOB_ASSERT( CELL_OK == ret );

	U32 tmp1 = SPU_PORT_IMMEDIATE_AUDIT_OUTPUT << EVENT_PORT_SHIFT;	//0x0E000000;
	U32 tmp2;

	do
	{
		do
		{
			__asm__ volatile ("rchcnt %0, $ch29" : "=r"(tmp2));
		} while(tmp2);
		__asm__ volatile ("wrch $ch28, %0" :: "r"(dataHi));
		__asm__ volatile ("wrch $ch30, %0" :: "r"(tmp1));
		__asm__ volatile ("rdch %0, $ch29" : "=r"(tmp2));
	} while ( tmp2 == 0x8001000A );	//EBUSY
	WWSJOB_ASSERT( CELL_OK == tmp2 );

	__asm__ volatile ("rdch %0, $ch29" : "=r"(tmp2));	//Stall the SPU until the event is actually processed

	do
	{
		do
		{
			__asm__ volatile ("rchcnt %0, $ch29" : "=r"(tmp2));
		} while(tmp2);
		__asm__ volatile ("wrch $ch28, %0" :: "r"(dataLo));
		__asm__ volatile ("wrch $ch30, %0" :: "r"(tmp1));
		__asm__ volatile ("rdch %0, $ch29" : "=r"(tmp2));
	} while ( tmp2 == 0x8001000A );	//EBUSY
	WWSJOB_ASSERT( CELL_OK == tmp2 );

	__asm__ volatile ("rdch %0, $ch29" : "=r"(tmp2));	//Stall the SPU until the event is actually processed

#else

	// use this to simplify stores
	U64 *pU64 = (U64*)g_lsaAudits;
	
	*pU64++ = audit;

	g_lsaAudits = (U32)pU64;
			 
	// if auditBlock is full
	if( (g_lsaAudits & (AUDIT_BLOCK_SIZE-1) ) == 0 )
	{	// auditBlock is full

		// output audit block, swap audit blocks
		OutputAuditBlock();			
	}

#endif
}


//--------------------------------------------------------------------------------------------------

#ifndef IMMEDIATE_AUDIT_OUTPUT
// output audit block, swap audit blocks
void OutputAuditBlock( void )
{
	// if there is room to store audits in main memory
	// 		it's ok to write if: if g_mmaAudits + AUDIT_BLOCK_SIZE <= g_mmaAuditsEnd
	// 		 or:	g_mmaAudits <= g_mmaAuditsEnd - AUDIT_BLOCK_SIZE
	// 		 or:	g_mmaAudits < g_mmaAuditsEnd - AUDIT_BLOCK_SIZE + 1
	if( g_mmaAudits < g_mmaAuditsEndLessAuditBlockSizePlus1 )
	{	// there is room to store audits in main memory

		// dma block out to buffer
		DMA_WRITE( g_lsaAudits - AUDIT_BLOCK_SIZE /*lsa*/, g_mmaAudits /*mma*/, AUDIT_BLOCK_SIZE /*size*/, \
				DmaTagId::kAudits )

		// update audit buffer adrs
		g_mmaAudits += AUDIT_BLOCK_SIZE;
	}

	// if audits went beyond 2nd block
	if( (g_lsaAudits & ((AUDIT_BLOCK_SIZE << 1) - 1)) == 0 )
	{	// audits went beyond 2nd block
	
		// get adrs of start of 1st block									
		g_lsaAudits -= (AUDIT_BLOCK_SIZE << 1);
	}
}
#endif
#endif

//--------------------------------------------------------------------------------------------------

#ifndef IMMEDIATE_AUDIT_OUTPUT
#if WWS_JOB_USE_C_VERSION!=0
void FlushRemainingAudits( void )
{
	U64* pAudits = (U64*) g_lsaAudits;

	if ( WwsJob_IsAligned( pAudits, AUDIT_BLOCK_SIZE ) )
	{
		//All have been flushed so do nothing
		return;
	}

	AuditEtc audit;
	audit.m_u64 = 0;
	audit.m_audit.m_id   		= AuditId::kNopAudit;

	do
	{
		*pAudits = audit.m_u64;	//Pad this audit block with nop audits
		++pAudits;
	}
	while ( !WwsJob_IsAligned( pAudits, AUDIT_BLOCK_SIZE ) );

	g_lsaAudits = (U32)pAudits;

	OutputAuditBlock();
}
#endif
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
void FinalizeAudits( void )
{
	//JobPrintf( "FinalizeAudits:\n" );

	if ( ( !g_bJobManagerAuditsEnabled ) && ( !g_bJobAuditsEnabled ) )
	{
		//No audits so nothing to do
		//Note that mmaAuditBufferHeader could potentially be NULL so we need this test to be safe
		return;
	}

#ifndef IMMEDIATE_AUDIT_OUTPUT
	FlushRemainingAudits();
#endif
	cellDmaWaitTagStatusAll( 1 << DmaTagId::kAudits );
	//Note: In case we move to a more ring-buffer style of audit buffering where the PPU can read the
	//audits while the SPU is still running, then we must be careful to ensure the audits are flushed
	//*before* we update the num written value, so that the PPU doesn't start reading audits before
	//they are written

	AuditBufferOutputHeader auditBufferOutputHeader;
	auditBufferOutputHeader.m_numDwordsWritten = ( g_mmaAudits - g_mmaAuditsBase ) / sizeof(Audit);
	//JobPrintf( "FinalizeAudits: setting m_numDwordsWritten to %d\n", auditBufferOutputHeader.m_numDwordsWritten );
	U32 mmaAuditBufferOutputHeader = g_mmaAuditBufferOutputHeader;
	cellDmaPut( &auditBufferOutputHeader,
				mmaAuditBufferOutputHeader,
				sizeof(AuditBufferOutputHeader),
				DmaTagId::kBlockingLoad,
				0,
				0 );
	cellDmaWaitTagStatusAll( 1 << DmaTagId::kBlockingLoad );
}
#endif

//--------------------------------------------------------------------------------------------------

#ifdef ENABLE_VERBOSE_AUDITS
// for the enableInterrupt audit:
// 		replace normal audit with one that will avoid continuous audit loops of
// 		 	enableInterrupts-disableInterrupts-intermediateAudit
// 		 	where intermediateAudit is any single audit of any size or doesn't exist at all
// Note: if you must reduce wwsjob memory size, this routine can be entirely removed, and just
//		use StoreAudit( AuditId::kWwsJob_EnableInterrupts ) instead
void WwsJob_StoreEnableInterruptsAuditCheckingTightLoop( void )
{
	AuditEtc *pAudit, audit0, audit1;

	// if g_pEnableAudit0 has valid value, and is in same 128 bytes as next audit to write
	if( ( ( (U32)s_pEnableAudit0 ^ g_lsaAudits ) & 0x87 ) == 0 )
	{	// s_pEnableAudit0 has valid value, and is in same 128 bytes as next audit to write

		// if s_pEnableAudit0 is an enable audit
		pAudit = s_pEnableAudit0;
		audit0 = *pAudit++;
		if( audit0.m_audit.m_id == AuditId::kWwsJob_EnableInterrupts )
		{	// s_pEnableAudit0 is an enable audit
		
			// if next audit is a disable audit
			audit0 = *pAudit++;
			if( audit0.m_audit.m_id == AuditId::kWwsJob_DisableInterrupts )
			{	// next audit is a disable audit

				// get next audit
				audit1 = *pAudit++;

				// if next audit is an enable audit
				if( audit1.m_audit.m_id == AuditId::kWwsJob_EnableInterrupts )
				{	// next audit is an enable audit
					// note there is no intermediate audit
																			  
					// backup ptr to start of enable audit							 
					pAudit--;
				}
				else
				{	// next audit is not an enable audit

					// skip past this audit
					U32 numDwords = audit1.m_audit.m_numDwords;
					if( numDwords == 7 )
						numDwords = audit1.m_audit.m_data;
					pAudit += numDwords;
				}

				// if next audit is s_pEnableAudit1
				if( pAudit == s_pEnableAudit1 )
				{	// next audit is s_pEnableAudit1

					// if s_pEnableAudit1 is an enable audit
					audit0 = *pAudit++;
					if( audit0.m_audit.m_id == AuditId::kWwsJob_EnableInterrupts )
					{	// s_pEnableAudit1 is an enable audit
					
						// if next audit is a disable audit
						audit0 = *pAudit++;
						if( audit0.m_audit.m_id == AuditId::kWwsJob_DisableInterrupts )
						{	// next audit is a disable audit

							// if intermediate audit does not exist
							if( audit1.m_audit.m_id == AuditId::kWwsJob_EnableInterrupts )
							{	// intermediate audit does not exist

								goto doFinalCheck;
							}
							else
							{	// intermediate audit exists

								// if next audit matches intermediate audit
								audit0 = *pAudit++;
								if( audit0.m_audit.m_id == audit1.m_audit.m_id )
								{	// next audit matches intermediate audit

									// skip past this audit
									U32 numDwords = audit0.m_audit.m_numDwords;
									if( numDwords == 7 )
										numDwords = audit0.m_audit.m_data;
									pAudit += numDwords;

								  doFinalCheck:
								  																  
									// if this is where the next audit is to be stored
									if( (U32)pAudit == g_lsaAudits )
									{	// this is where the next audit is to be stored

										// we can delete the last enable-disable-intermediate audits!!!
										g_lsaAudits = (U32)s_pEnableAudit1;
										goto overwriteAudit;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	// we can't backup the audit, so it is a normal audit store

	// remember where the enable audits are stored
	s_pEnableAudit0 = s_pEnableAudit1;
	s_pEnableAudit1 = (AuditEtc*)g_lsaAudits;

  overwriteAudit:

	StoreAudit( AuditId::kWwsJob_EnableInterrupts );
}
#endif

//--------------------------------------------------------------------------------------------------
