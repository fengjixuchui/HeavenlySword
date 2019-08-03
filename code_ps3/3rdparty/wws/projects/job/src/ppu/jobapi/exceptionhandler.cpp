/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This exception handler is called back if the SPU triggers an exception
**/
//--------------------------------------------------------------------------------------------------

#include <sys/spu_thread.h>
#include <sys/spu_thread_group.h>

#include <cell/spurs/control.h>
#include <cell/spurs/exception.h>

#include <jobapi/joblist.h>
#include <jobapi/exceptionhandler.h>

WwsJobExceptionEventHandler gpWwsJobExceptionEventHandlerCallback = WwsJobExceptionHandlerCallback;

//--------------------------------------------------------------------------------------------------
//
//	This function is called when an SPU exception is triggered for our workload
//	The standard handler doesn't print the $PC, so for now that's all this function does
//	but in the future we might add more here.
//
//--------------------------------------------------------------------------------------------------

static bool GetLsWord( sys_spu_thread_t spuThreadId, U32 addr, U32* pData32 )
{
	U64 data64;
	int ret = sys_spu_thread_read_ls( spuThreadId, addr & 0xFFFFFFF8, &data64, 8 );

	if ( CELL_OK != ret )
	{
		return false;
	}

	if ( addr & 0x7 )
	{
		*pData32 = (U32)(data64);
	}
	else
	{
		*pData32 = (U32)(data64>>32L);
	}
	return true;
}

//--------------------------------------------------------------------------------------------------

static bool GetLsQWord( sys_spu_thread_t spuThreadId, U32 addr, void* pQword )
{
	int ret = sys_spu_thread_read_ls( spuThreadId, addr, (U64*)pQword, 8 );
	if ( CELL_OK != ret )
	{
		return false;
	}

	ret = sys_spu_thread_read_ls( spuThreadId, addr+8, &((U64*)pQword)[1], 8 );
	if ( CELL_OK != ret )
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------

/*void SpuStackTrace( sys_spu_thread_t spuThreadId, U32 lsStackAddr, U32 linkReg, U32 maxDepth )
{
	JobBasePrintf( "Spu Stack Trace ($1 = 0x%08X)\n", lsStackAddr );
	U32 currentStackFrame = lsStackAddr;
	U32 prevStackFrame;
	bool firstRetAddr = true;

	while ( maxDepth > 0 )
	{
		if ( !GetLsWord( spuThreadId, currentStackFrame, &prevStackFrame ) )
			break;	//Reading failed, so don't try and print
		if ( prevStackFrame == 0x00000000 )
			break;

		U32 retAddr;
		if ( !GetLsWord( spuThreadId, prevStackFrame + 0x10, &retAddr ) )
			break;	//Reading failed, so don't try and print

		if ( firstRetAddr )
			JobBasePrintf( "\tretAddr ?= 0x%05X\t($0 = 0x%08X)\n", retAddr, linkReg );	//May not have been filled in by current function yet?
		else
			JobBasePrintf( "\tretAddr = 0x%05X\n", retAddr );

		firstRetAddr = false;
		currentStackFrame = prevStackFrame;

		--maxDepth;
	}
}*/

//--------------------------------------------------------------------------------------------------

union SpuStatusRegister
{
	struct
	{
		U32	m_sc								: 16;
		U32	m_reserved2							: 5;
		U32	m_isolateExitStatus					: 1;
		U32	m_isolateLoadStatus					: 1;
		U32	m_reserved1							: 1;
		U32	m_isolationStatus					: 1;
		U32	m_illegalChannelInstructionDetected	: 1;
		U32	m_invalidInstructionDetected		: 1;
		U32	m_singleStepStatus					: 1;
		U32	m_waitStatus						: 1;
		U32	m_haltStatus						: 1;
		U32	m_programStopAndSignalStatus		: 1;
		U32	m_runStatus							: 1;
	};
	U32	m_val;
};

/*static void PrintSpuStatusRegister( U32 spuStatusRegister )
{
	SpuStatusRegister status;

	status.m_val = spuStatusRegister;

	JobBasePrintf("SPU_Status = 0x%X\n", status.m_val );
	JobBasePrintf("SPU_Status.m_runStatus                         = %d\n", status.m_runStatus );
	JobBasePrintf("SPU_Status.m_programStopAndSignalStatus        = %d\n", status.m_programStopAndSignalStatus );
	JobBasePrintf("SPU_Status.m_haltStatus                        = %d\n", status.m_haltStatus );
	JobBasePrintf("SPU_Status.m_waitStatus                        = %d\n", status.m_waitStatus );
	JobBasePrintf("SPU_Status.m_singleStepStatus                  = %d\n", status.m_singleStepStatus );
 	JobBasePrintf("SPU_Status.m_invalidInstructionDetected        = %d\n", status.m_invalidInstructionDetected );
 	JobBasePrintf("SPU_Status.m_illegalChannelInstructionDetected = %d\n", status.m_illegalChannelInstructionDetected );
	JobBasePrintf("SPU_Status.m_isolationStatus                   = %d\n", status.m_isolationStatus );
	JobBasePrintf("SPU_Status.m_reserved1                         = %d\n", status.m_reserved1 );
	JobBasePrintf("SPU_Status.m_isolateLoadStatus                 = %d\n", status.m_isolateLoadStatus );
	JobBasePrintf("SPU_Status.m_isolateExitStatus                 = %d\n", status.m_isolateExitStatus );
	JobBasePrintf("SPU_Status.m_reserved2                         = %d\n", status.m_reserved2 );
	JobBasePrintf("SPU_Status.m_sc                                = 0x%04X\n", status.m_sc );
}*/

//--------------------------------------------------------------------------------------------------

void WwsJobExceptionHandlerCallback( CellSpurs* pSpurs, const CellSpursExceptionInfo* pExceptionInfo, void* pMyData )
{
	JobBasePrintf( "--------------------------------------------------------\n" );

	const JobListPrivate* pJobList	= (const JobListPrivate*) pMyData;

	sys_spu_thread_t spuThreadId	= pExceptionInfo->spu_thread;

	U32 npc							= pExceptionInfo->spu_npc & 0xFFFFFFFE;
	U32 interruptsEnabled			= pExceptionInfo->spu_npc & 0x1;

	if ( interruptsEnabled )
		JobBasePrintf( "$NPC = 0x%X [Interrupts Enabled]\n", npc );
	else
		JobBasePrintf( "$NPC = 0x%X\n", npc );

	//Print some words from around $NPC
	for ( U32 pInstr = npc - 16; pInstr < npc + 16; pInstr += 4 )
	{
		U32 instruction;
		if ( !GetLsWord( spuThreadId, pInstr, &instruction ) )
			break;	//Reading failed, so don't try and print

		if ( pInstr == npc )
			JobBasePrintf( "\t0x%05X\t0x%08x <-- $NPC\n", pInstr, instruction );
		else
			JobBasePrintf( "\t0x%05X\t0x%08x\n", pInstr, instruction );
	}

	JobBasePrintf( "--------------------------------------------------------\n" );

	//SpuStackTrace( spuThreadId, lsStackAddr, linkReg, 32 );
	//JobBasePrintf( "--------------------------------------------------------\n" );

	U32 qword[4] WWSJOB_ALIGNED(16);
	if ( GetLsQWord( spuThreadId, 0, qword ) )
		JobBasePrintf( "LS[0x00000] = 0x%08X_%08X_%08X_%08X\n", qword[0], qword[1], qword[2], qword[3] );
	if ( GetLsQWord( spuThreadId, 0x280, qword ) )
		JobBasePrintf( "LS[0x00280] = 0x%08X_%08X_%08X_%08X\n", qword[0], qword[1], qword[2], qword[3] );
	if ( GetLsQWord( spuThreadId, 0xA00, qword ) )
		JobBasePrintf( "LS[0x00A00] = 0x%08X_%08X_%08X_%08X\n", qword[0], qword[1], qword[2], qword[3] );
	if ( GetLsQWord( spuThreadId, 0x4000, qword ) )
		JobBasePrintf( "LS[0x04000] = 0x%08X_%08X_%08X_%08X\n", qword[0], qword[1], qword[2], qword[3] );
	if ( GetLsQWord( spuThreadId, 0x3E000, qword ) )
		JobBasePrintf( "LS[0x3E000] = 0x%08X_%08X_%08X_%08X\n", qword[0], qword[1], qword[2], qword[3] );

	JobBasePrintf( "--------------------------------------------------------\n" );

	U32 cause						= pExceptionInfo->cause;
	U64 option						= pExceptionInfo->option;

	switch ( cause )
	{
	case SYS_SPU_EXCEPTION_DMA_ALIGNMENT:
		JobBasePrintf( "SYS_SPU_EXCEPTION_DMA_ALIGNMENT:\n" );
		break;
	case SYS_SPU_EXCEPTION_DMA_COMMAND:
		JobBasePrintf( "SYS_SPU_EXCEPTION_DMA_COMMAND:\n" );
		break;
	case SYS_SPU_EXCEPTION_SPU_ERROR:
		JobBasePrintf( "SYS_SPU_EXCEPTION_SPU_ERROR:\n" );
		break;
	case SYS_SPU_EXCEPTION_MFC_FIR:
		JobBasePrintf( "SYS_SPU_EXCEPTION_MFC_FIR:\n" );
		break;
	case SYS_SPU_EXCEPTION_MFC_SEGMENT:
		JobBasePrintf( "SYS_SPU_EXCEPTION_MFC_SEGMENT:\n" );
		JobBasePrintf( "MFC_DAR ?= 0x%016llX\n", option );
		//This currently seems to be coming back as all zero :(
		break;
	case SYS_SPU_EXCEPTION_MFC_STORAGE:
		JobBasePrintf( "SYS_SPU_EXCEPTION_MFC_STORAGE:\n" );
		JobBasePrintf( "MFC_DAR ?= 0x%016llX\n", option );
		//This currently seems to be coming back as all zero :(
		break;
	case SYS_SPU_EXCEPTION_STOP_CALL:
		JobBasePrintf( "SYS_SPU_EXCEPTION_STOP_CALL:\n" );
		break;
	case SYS_SPU_EXCEPTION_STOP_BREAK:
		JobBasePrintf( "SYS_SPU_EXCEPTION_STOP_BREAK:\n" );
		break;
	case SYS_SPU_EXCEPTION_HALT:
		JobBasePrintf( "SYS_SPU_EXCEPTION_HALT:\n" );
		break;
	case SYS_SPU_EXCEPTION_UNKNOWN_SIGNAL:
		JobBasePrintf( "SYS_SPU_EXCEPTION_UNKNOWN_SIGNAL:\n" );
		JobBasePrintf( "SPU_Status ?= 0x%08X_%08X\n", (U32)(option >> 32L), (U32)option );
		//This currently seems to be coming back as all zero :(
		//PrintSpuStatusRegister( (U32)(option >> 32L) );
		break;
	case SYS_SPU_EXCEPTION_NO_VALUE:
		JobBasePrintf( "SYS_SPU_EXCEPTION_NO_VALUE:\n" );
		break;
	default:
		JobBasePrintf( "unknown cause(%d), option=0%16llX\n", cause, option );
		break;
	}

	JobBasePrintf( "--------------------------------------------------------\n" );

	CellSpursInfo spursInfo;
	int ret = cellSpursGetInfo( pSpurs, &spursInfo );
	if ( CELL_OK != ret )
	{
		JobBasePrintf( "cellSpursGetInfoFailed\n" );
	}
	else
	{
		JobBasePrintf( "spursInfo.nspus                                = %d\n",		spursInfo.nSpus );
		JobBasePrintf( "spursInfo.spuThreadGroupPriority               = %d\n",		spursInfo.spuThreadGroupPriority );
		JobBasePrintf( "spursInfo.ppuThreadPriority                    = %d\n",		spursInfo.ppuThreadPriority );
		JobBasePrintf( "spursInfo.exitIfNoWork                         = %d\n",		spursInfo.exitIfNoWork );
		JobBasePrintf( "spursInfo.traceBuffer                          = 0x%08X\n",	(U32)spursInfo.traceBuffer );	//I wonder if this is compiled to assume 64bit pointers?
		JobBasePrintf( "spursInfo.traceBufferSize                      = 0x%llX\n",	spursInfo.traceBufferSize );
		JobBasePrintf( "spursInfo.traceMode                            = %d\n",		spursInfo.traceMode );
		JobBasePrintf( "spursInfo.spuThreadGroup                       = 0x%08X\n",	spursInfo.spuThreadGroup );
		for ( int i = 0; i < spursInfo.nSpus; ++i )
		{
			JobBasePrintf( "spursInfo.spuThreads[%d]                        = 0x%08X\n", i, spursInfo.spuThreads[i] );
		}
		JobBasePrintf( "spursInfo.spursHandelerThread0                 = 0x%08X\n", (U32)spursInfo.spursHandelerThread0 );
		JobBasePrintf( "spursInfo.spursHandelerThread1                 = 0x%08X\n", (U32)spursInfo.spursHandelerThread1 );
	}

	JobBasePrintf( "--------------------------------------------------------\n" );

	const JobListHeader* pJobListHeader = (const JobListHeader*) pJobList->GetWQAddr();

	JobBasePrintf( "JobList.m_name                                 = \"%s\"\n",						pJobList->GetName() );
	JobBasePrintf( "JobList.m_numJobsTaken                         = %d\n",				pJobListHeader->m_numJobsTaken );
	JobBasePrintf( "JobList.m_numJobsInListHint                    = %d\n",		pJobListHeader->m_numJobsInListHint );
	for ( int spuNum = 0; spuNum < spursInfo.nSpus; ++spuNum )
	{
		JobBasePrintf( "JobList.m_minActive[%d]                         = %d\n",		spuNum, pJobListHeader->m_minActive[spuNum] );
	}
	//work out how many jobs there are on the joblist - maybe print them?

	U32 jobNum = 0;
	U32 kMaxJobsToPrint = 4096;
	while ( jobNum < kMaxJobsToPrint )
	{
		U64 jobHeader = pJobListHeader->m_jobHeader[jobNum].m_u64;
		if ( jobHeader == 0LL )
			break;
		JobBasePrintf( "JobList.m_jobHeader[%d]                         = 0x%08X_%08X\n", jobNum, (U32)(jobHeader>>32L), (U32)jobHeader );
		++jobNum;
	}

	JobBasePrintf( "--------------------------------------------------------\n" );

}

//--------------------------------------------------------------------------------------------------
