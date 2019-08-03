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

#ifndef WWS_JOB_COMMAND_LIST_BUILDER_H
#define WWS_JOB_COMMAND_LIST_BUILDER_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/jobdefinition.h>

//--------------------------------------------------------------------------------------------------

struct DependencyCounter;
class SpuModuleHandle;

//--------------------------------------------------------------------------------------------------

class CommandListBuilder
{
public:
				CommandListBuilder( void* pBuffer, U32 bufferSize );
	void		ResetCommandListBuilder( void );

	void		InitializeJob( void );
	JobHeader	FinalizeJob( void );
	JobHeader	FinalizeJobWithBreakpoint( void );

	void		InitializeCommandList( void );
	const WwsJob_Command*	FinalizeCommandList( void );

	void		ReserveBufferSet( U32 bufferSetNum, U32 numBuffers, U32 firstPageNum, U32 numPagesPerBuffer );
	void		UnreserveBufferSets( U32 bufferSetMask );

	void		UseUninitializedBuffer( U32 bufferSetNum, U32 logicalBufferNum, void* mainMemAddress = 0, U32 mainMemSize = 0, WwsJob_Command::BufferCacheType cacheType = WwsJob_Command::kNonCached);
	void		UseInputBuffer( U32 bufferSetNum, U32 logicalBufferNum, void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType );

	void		UseUninitializedBuffer( U32 bufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType);
	void		UseInputBuffer( U32 bufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType );
	void		UseInputDmaListBuffer( U32 bufferSetNum, U32 logicalBufferNum, const void* mainMemAddress, U32 mainMemSize, WwsJob_Command::BufferCacheType cacheType, U32 numPadQwordsBelowListElements );

#ifndef __SPU__
	void		UseInputBuffer( U32 bufferSetNum, U32 logicalBufferNum, const SpuModuleHandle& spuModHandle, WwsJob_Command::BufferCacheType cacheType );
#endif

	void		RequestDependencyDecrement( DependencyCounter* eaDependencyCounter );

	void		AddNopCommand( void );

	void		RunJob( U32 logicalBufferSetNum, U32 logicalBufferNum );
	void		AddParams( const void* pParams, U32 paramSize );
	void		AddU32Param( U16 paramIndex, U32 paramVal );

	void		AddEndCommand( void );	//Called automatically by CloseCommandList

	WwsJob_Command*	GetCurrentCommandPtr( void )				{ return (WwsJob_Command*) m_pCurrent; }

	void		AlignBuffer128( void );

	//Returns the number of bytes left in the buffer.  Negative means we've overflowed
	U32			QueryRemainingSpaceInBuffer( void )				{ return ((U32)m_pBufferBase + m_bufferSize - (U32)m_pCurrent); }

	void*		AllocParamSpace( U32 paramSize );

private:
	WwsJob_Command* AllocCommandSpace( void );
	void		AlignBuffer16( void );

	U32			m_bufferSize;
	void*		m_pBufferBase;
	WwsJob_Command*	m_pCurrent;
	const void*	m_pLastJobStart;
	const void* m_pLastCommandListStart;
	U32*		m_pParamsBuffer;	//NULL while adding commands, a valid pointer if adding single params, kAddSingleParamInvalidated if adding buffer params

	enum
	{
		kAddSingleParamInvalidated	= 0xFFFFFFFF,
		kMaxSingleParmas			= 8,
	};
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_COMMAND_LIST_BUILDER_H */
