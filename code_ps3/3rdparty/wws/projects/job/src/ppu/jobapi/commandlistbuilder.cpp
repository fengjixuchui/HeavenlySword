/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Utility class for building command lists
**/
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include <sys/process.h>
#include <jobapi/commandlistbuilder.h>

#ifndef __SPU__
#include <jobapi/spumodule.h>
#endif

//--------------------------------------------------------------------------------------------------

CommandListBuilder::CommandListBuilder( void* pBuffer, U32 bufferSize )
:
	m_bufferSize( bufferSize ),
	m_pBufferBase( pBuffer ),
	m_pCurrent( (WwsJob_Command*)pBuffer ),
	m_pLastJobStart( NULL ),
	m_pLastCommandListStart( NULL )
{
	WWSJOB_ASSERT( !sys_process_is_stack( pBuffer ) );//Buffer will be read by SPU so mustn't be on stack
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::ResetCommandListBuilder( void )
{
	WWSJOB_ASSERT( m_pLastJobStart == NULL );
	m_pCurrent = (WwsJob_Command*) m_pBufferBase;
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AlignBuffer16( void )
{
	m_pCurrent = (WwsJob_Command*) WwsJob_AlignPtr( m_pCurrent, 0x10 );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AlignBuffer128( void )
{
	m_pCurrent = (WwsJob_Command*) WwsJob_AlignPtr( m_pCurrent, 0x80 );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::InitializeJob( void )
{
	//Can either start a job, or a command list
	WWSJOB_ASSERT( NULL == m_pLastJobStart );
	WWSJOB_ASSERT( NULL == m_pLastCommandListStart );
	m_pLastJobStart = m_pCurrent;
	m_pParamsBuffer = NULL;
}

//--------------------------------------------------------------------------------------------------

JobHeader CommandListBuilder::FinalizeJob( void )
{
	//Can only call CloseJob if it was a job we were building
	WWSJOB_ASSERT( m_pLastJobStart );
	WWSJOB_ASSERT( NULL == m_pLastCommandListStart );

	AlignBuffer16();

	JobHeader newJob;
	newJob.m_mmaLoadCommands		= (U32)m_pLastJobStart;
	newJob.m_loadCommandsSize		= ((U32)m_pCurrent) - ((U32)m_pLastJobStart);;
	newJob.m_enableBreakpoint		= 0;
	newJob.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;
	m_pLastJobStart	= NULL;
	return newJob;
}

//--------------------------------------------------------------------------------------------------

JobHeader CommandListBuilder::FinalizeJobWithBreakpoint( void )
{
	//Can only call CloseJob if it was a job we were building
	WWSJOB_ASSERT( m_pLastJobStart );
	WWSJOB_ASSERT( NULL == m_pLastCommandListStart );

	AlignBuffer16();

	JobHeader newJob;
	newJob.m_mmaLoadCommands		= (U32)m_pLastJobStart;
	newJob.m_loadCommandsSize		= ((U32)m_pCurrent) - ((U32)m_pLastJobStart);;
	newJob.m_enableBreakpoint		= 1;
	newJob.m_jobHeaderCommand		= JobHeaderCommand::kJobExists;
	m_pLastJobStart	= NULL;
	return newJob;
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::InitializeCommandList( void )
{
	//Can either start a job, or a command list
	WWSJOB_ASSERT( NULL == m_pLastJobStart );
	WWSJOB_ASSERT( NULL == m_pLastCommandListStart );
	m_pLastCommandListStart = m_pCurrent;
	m_pParamsBuffer = NULL;
}

//--------------------------------------------------------------------------------------------------

const WwsJob_Command* CommandListBuilder::FinalizeCommandList( void )
{
	//Can only call CloseCommandList if it was a command list we were building
	WWSJOB_ASSERT( NULL == m_pLastJobStart );
	WWSJOB_ASSERT( m_pLastCommandListStart );

	AddEndCommand();
	AlignBuffer16();

	const WwsJob_Command* pLastCommandListStart = (const WwsJob_Command*) m_pLastCommandListStart;

	m_pLastCommandListStart	= NULL;

	return pLastCommandListStart;	//return base of command list that we're closing
}

//--------------------------------------------------------------------------------------------------

WwsJob_Command* CommandListBuilder::AllocCommandSpace( void )
{
	WWSJOB_ASSERT( QueryRemainingSpaceInBuffer() >= sizeof( WwsJob_Command ) );
	WWSJOB_ASSERT( m_pParamsBuffer == NULL );
	WwsJob_Command* pCmd = m_pCurrent;
	++m_pCurrent;
	return pCmd;
}

//--------------------------------------------------------------------------------------------------

void* CommandListBuilder::AllocParamSpace( U32 paramSize )
{
	WWSJOB_ASSERT( (paramSize & 0xF) == 0 );
	WWSJOB_ASSERT( QueryRemainingSpaceInBuffer() >= paramSize );

	void* pParams = m_pCurrent;
	m_pCurrent = (WwsJob_Command*)(((U32)m_pCurrent) + paramSize);
	return pParams;
}

//--------------------------------------------------------------------------------------------------

void	CommandListBuilder::ReserveBufferSet( U32 bufferSetNum, U32 numBuffers, U32 firstPageNum, U32 numPagesPerBuffer )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->ReserveBufferSet( bufferSetNum, numBuffers, firstPageNum, numPagesPerBuffer );
}

//--------------------------------------------------------------------------------------------------

void	CommandListBuilder::UnreserveBufferSets( U32 bufferSetMask )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UnreserveBufferSets( bufferSetMask );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::UseUninitializedBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseUninitializedBuffer( logicalBufferSetNum, logicalBufferNum, (U32)mainMemAddress, mainMemSize, cacheType );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::UseUninitializedBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType )
{
	//If it's a const pointer, we can't be over-writing it
	WWSJOB_ASSERT( cacheType != WwsJob_Command::kReadWriteCached );

	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseUninitializedBuffer( logicalBufferSetNum, logicalBufferNum, (U32)mainMemAddress, mainMemSize, cacheType );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::UseInputBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseInputBuffer( logicalBufferSetNum, logicalBufferNum, (U32)mainMemAddress, mainMemSize, cacheType );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::UseInputBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType )
{
	//If it's a const pointer, we can't be over-writing it
	WWSJOB_ASSERT( cacheType != WwsJob_Command::kReadWriteCached );

	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseInputBuffer( logicalBufferSetNum, logicalBufferNum, (U32)mainMemAddress, mainMemSize, cacheType );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::UseInputDmaListBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType, U32 numPadQwordsBelowListElements )
{
	//We'll never be sending out from the dmalist again because it's over-written, so the cacheType must be appropriate
	WWSJOB_ASSERT( cacheType != WwsJob_Command::kReadWriteCached );

	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseInputDmaListBuffer( logicalBufferSetNum, logicalBufferNum, (U32)mainMemAddress, mainMemSize, cacheType, numPadQwordsBelowListElements );
}

//--------------------------------------------------------------------------------------------------

#ifndef __SPU__
void CommandListBuilder::UseInputBuffer( U32 logicalBufferSetNum, U32 logicalBufferNum, const SpuModuleHandle& spuModHandle, WwsJob_Command::BufferCacheType cacheType )
{
	//If it's a const pointer, we can't be over-writing it
	WWSJOB_ASSERT( cacheType != WwsJob_Command::kReadWriteCached );

	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->UseInputBuffer( logicalBufferSetNum, logicalBufferNum, (U32)spuModHandle.GetAddress(), spuModHandle.GetFileSize(), cacheType );
}
#endif

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::RequestDependencyDecrement( DependencyCounter* eaDependencyCounter )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->RequestDependencyDecrement( (U32)eaDependencyCounter );
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::RunJob( U32 logicalBufferSetNum, U32 logicalBufferNum )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->RunJob( logicalBufferSetNum, logicalBufferNum );
	AlignBuffer16();
	//Parameters added after this must be aligned
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AddParams( const void* pParams, U32 paramSize )
{
	WWSJOB_ASSERT( (paramSize & 0xF) == 0 );
	void* pParamsBuffer = AllocParamSpace( paramSize );
	WwsJob_MemcpyQword( pParamsBuffer, pParams, paramSize );
	m_pParamsBuffer = (U32*)kAddSingleParamInvalidated;
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AddU32Param( U16 paramIndex, U32 paramVal )
{
	WWSJOB_ASSERT( m_pParamsBuffer != (U32*)kAddSingleParamInvalidated );
	//If we've already called AddParams, then don't call AddSingleU32Param
	WWSJOB_ASSERT( paramIndex < kMaxSingleParmas );

	if ( m_pParamsBuffer == NULL )
		m_pParamsBuffer = (U32*) AllocParamSpace( 0x20 );

	m_pParamsBuffer[paramIndex] = paramVal;
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AddEndCommand( void )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->AddEndCommand();
}

//--------------------------------------------------------------------------------------------------

void CommandListBuilder::AddNopCommand( void )
{
	WwsJob_Command* pCmd = AllocCommandSpace();
	pCmd->AddNopCommand();
}

//--------------------------------------------------------------------------------------------------
