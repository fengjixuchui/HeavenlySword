//--------------------------------------------------------------------------------------------------
/**
	@file		SpuKernelInterface.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef SPU_KERNEL_INTERFACE_H
#define SPU_KERNEL_INTERFACE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/SpuKernel/Spu/SkCommand.h>

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  EXTERNAL DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------
class SkTaskList;
class SkElf;
class SkCommandList;
class SkMemoryOccupancy;
class SkLsSubBuffer;

typedef void (*SkUserCallbackFunc)( int spuId, u32 allocationDirection, u32 userVal );

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkLsBuffer

	@brief			Helper class to correspond to a buffer in SPU Local Storage, used during task
					list building
**/
//--------------------------------------------------------------------------------------------------

class SkLsBuffer
{
public:
	friend class		SkCommandList;
	friend class		SkMemoryOccupancy;
	friend class		SkLsSubBuffer;

	SkLsBuffer( u32 baseAddr, u32 size )		{ m_baseAddr = baseAddr; m_size = size; }

	u32					GetBaseAddress( void )	{ return m_baseAddr; }
	u32					GetBufferSize( void )	{ return m_size; }

private:
	u32					m_baseAddr;
	u32					m_size;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkLsSubBuffer

	@brief			Helper class to correspond to a sub-buffer of an SkLsBuffer in SPU Local Storage,
					used during task list building.

	@note			Because of how the kernel allocate memory, SkLsBuffer isn't guaranteed to be
					next to another SkLsBuffer in any predictable manner, however two SkLsSubBuffer's
					do maintain the relative ordering within a SkLsBuffer
**/
//--------------------------------------------------------------------------------------------------

class SkLsSubBuffer
{
public:
	friend class		SkCommandList;

	SkLsSubBuffer( const SkLsBuffer& containingBuffer, u32 offset, u32 subBufferSize )
	{
		m_baseAddr	= containingBuffer.m_baseAddr;
		m_size		= containingBuffer.m_size;
		m_offset	= offset;
		FW_UNUSED( subBufferSize );
	}

private:
	u32					m_baseAddr;
	u32					m_size;
	u32					m_offset;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkSentinel

	@brief			Helper class for putting a marker into LS and then later asserting that it
					hasn't been corrupted by a buffer over-run
**/
//--------------------------------------------------------------------------------------------------

class SkSentinel
{
public:
	friend class		SkCommandList;
	friend class		SkMemoryOccupancy;

	SkSentinel( u32 addr, u32 id = 0x12345678 )		{ m_addr = addr; m_id = id; }

private:
	u32					m_addr;
	u32					m_id;
};
//--------------------------------------------------------------------------------------------------
/**
	@struct			SkMemoryOccupancy

	@brief			Contains the bounds of all SkBuffers needed within a given Stage of a task
**/
//--------------------------------------------------------------------------------------------------

class SkMemoryOccupancy
{
public:
	friend class		SkCommandList;

	SkMemoryOccupancy()											{ m_baseAddr = 0xFFFFFFFF; m_topAddr = 0; }
	void				Contains( const SkLsBuffer& buff );
	void				Contains( const SkSentinel& sentinel );

private:
	u32					m_baseAddr;
	u32					m_topAddr;
};

inline void SkMemoryOccupancy::Contains( const SkLsBuffer& buff )
{
	m_baseAddr	= min( m_baseAddr, buff.m_baseAddr );
	m_topAddr	= max( m_topAddr, buff.m_baseAddr + buff.m_size );
}

inline void SkMemoryOccupancy::Contains( const SkSentinel& sentinel )
{
	m_baseAddr	= min( m_baseAddr, sentinel.m_addr );
	m_topAddr	= max( m_topAddr, sentinel.m_addr + 0x10 );
}

//--------------------------------------------------------------------------------------------------
/**
	@class			SkCommandList

	@brief			Helper class for building Command Lists
**/
//--------------------------------------------------------------------------------------------------

class SkCommandList
{
	friend				class SkTaskList;

public:
						SkCommandList( void* pCommandListBuffer, u32 numCommnads );

	SkKernelCommand*	GetCurrentElement( void );
	u32					GetCurrentElementIndex( void );
	u32					CloseCommandList( void );
	
	void				DefineMemoryOccupancy( u32 baseAddr, u32 topAddr );
	void				DmaToLsFromEa( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize );
	void				DmaFromLsToEa( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, void* pBuffer, u32 dmaSize );
	void				DmaToLsFromEaCacheable( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize );
	void				DmaToLsFromElf( const SkElf& rElf, u32 spuAddr, u32 spuBufferSize, bool cacheable );
	void				Finish( void );
	void				Nop( void );
	void				PrintString( const char* str );
	void				CallbackUserFunction( SkUserCallbackFunc func, u32 userVal );
	void				Param32( u32 paramId, u32 paramValue32 );
	void				Param64( u32 paramId, u64 paramValue64 );
	void				ParamFloat( u32 paramId, float floatVal );
	void				ParamEffAddr( u32 paramId, void* ptr );
	void				ParamEffAddr( u32 paramId, const void* ptr );
	void				ParamPseudoAddr( u32 paramId, u32 spuBufferAddr, u32 spuBufferSize, u32 offset );
	void				SetParam1Qword( u32 paramIndex, u128 param );
	void				SetParamQwords( u32 paramBaseIndex, u32 numParams, const u128* params );
	void				RunCode( u32 spuAddr, u32 spuBufferSize, u32 entryPointOffset );
	void				RunCodeWithBreakpoint( u32 spuAddr, u32 spuBufferSize, u32 entryPointOffset );

	void				SetIdForThisSpu( u16 uniqueId );
	void				StallOnNumSignals( u32 numSignals );
	void				SyncDma( void );
	void				FindOtherSpu( u16 uniqueId );
	void				DmaInFromOtherSpu( u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize );
	void				DmaOutToOtherSpu( u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize );
	void				SendSignalToOtherSpu( void );
	void				StallOnSendSignalToOtherSpu( void );
	void				StallForDependency( const SkDependency& dependency );
	void				DecrementDependency( const SkDependency& dependency );

	void				InstallSentinel( u32 spuPseudoAddr, u32 sentinelId );
	void				VerifySentinel( u32 spuPseudoAddr, u32 sentinelId );

	void				ZeroCacheLine( void );
	void				ZeroNextCacheLine( void );

	void				DefineMemoryOccupancy( const SkMemoryOccupancy& memOcc  );

	void				DmaToLsFromEa( const SkLsBuffer& input, const void* pBuffer, u32 dmaSize );
	void				DmaFromLsToEa( const SkLsBuffer& output, void* pBuffer, u32 dmaSize );
	void				DmaToLsFromEaCacheable( const SkLsBuffer& input, const void* pBuffer, u32 dmaSize );
	void				DmaToLsFromElf( const SkElf& rElf, const SkLsBuffer& input, bool cacheable );
	void				ParamPseudoAddr( u32 paramId, const SkLsBuffer& buffer );
	void				RunCode( const SkLsBuffer& codeBuffer, u32 entryPointOffset );
	void				RunCodeWithBreakpoint( const SkLsBuffer& codeBuffer, u32 entryPointOffset );
	void				DmaInFromOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaOutToOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize );

	void				DmaToLsFromEa( const SkLsSubBuffer& input, const void* pBuffer, u32 dmaSize );
	void				DmaFromLsToEa( const SkLsSubBuffer& output, void* pBuffer, u32 dmaSize );
	void				DmaToLsFromEaCacheable( const SkLsSubBuffer& input, const void* pBuffer, u32 dmaSize );
	void				ParamPseudoAddr( u32 paramId, const SkLsSubBuffer& buffer );
	void				DmaInFromOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaInFromOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaInFromOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaOutToOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaOutToOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize );
	void				DmaOutToOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize );

	void				InstallSentinel( const SkSentinel& sentinel );
	void				VerifySentinel( const SkSentinel& sentinel );

private:
	u32					m_numCommands;
	u32					m_currentElement;
	SkKernelCommand*	m_pCommandListBase;

	SkKernelCommand*	GetCommandListBase( void );
	const SkKernelCommand*	GetCommandListBase( void ) const;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			Construct an SkCommandList
					This is a helper object for building command lists

	@param			pCommandListBuffer	- A pointer to the memory where the commands will be built

	@param			numCommnads			- The maximum number of commands that can be held in the
											buffer
**/
//--------------------------------------------------------------------------------------------------

inline SkCommandList::SkCommandList( void* pCommandListBuffer, u32 numCommnads )
{
	m_numCommands		= numCommnads;
	m_currentElement	= 0;
	m_pCommandListBase	= (SkKernelCommand*) pCommandListBuffer;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the pointer to the current element to be operated on

	@return			The index of the current element to be operated on
**/
//--------------------------------------------------------------------------------------------------

inline SkKernelCommand* SkCommandList::GetCurrentElement( void )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];

	return pKernelCommand;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the index of the current element to be operated on

	@return			The index of the current element to be operated on
**/
//--------------------------------------------------------------------------------------------------

inline u32 SkCommandList::GetCurrentElementIndex( void )
{
	return m_currentElement;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Closes the Command List

	@return			The number of elements added

	@note			Although this command presently does nothing, that may be subject to change
**/
//--------------------------------------------------------------------------------------------------

inline u32 SkCommandList::CloseCommandList( void )
{
	//returns num commands added
	return m_currentElement;
}


//--------------------------------------------------------------------------------------------------
/**
**/
//--------------------------------------------------------------------------------------------------

inline SkKernelCommand*	SkCommandList::GetCommandListBase( void )
{
	return m_pCommandListBase;
}

inline const SkKernelCommand*	SkCommandList::GetCommandListBase( void ) const
{
	return m_pCommandListBase;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

inline void SkDefineMemoryOccupancy( SkKernelCommand* pKernelCommand, u32 baseAddr, u32 topAddr )
{
	FW_ASSERT( baseAddr <= topAddr );
	FW_ASSERT( topAddr <= kSpuUserAvailableMem );
	FW_ASSERT( FwIsAligned( baseAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( topAddr, 0x10 ) );

	pKernelCommand->m_memOccupancy.m_cmdId						= kDefineMemoryOccupancy;
	pKernelCommand->m_memOccupancy.m_currentSlotBaseAddr		= baseAddr;
	pKernelCommand->m_memOccupancy.m_currentSlotTopAddr			= topAddr;
}

//--------------------------------------------------------------------------------------------------

inline void SkDmaToLsFromEa( SkKernelCommand* pKernelCommand, u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( pBuffer );
	FW_ASSERT( dmaSize > 0 );
	FW_ASSERT( FwIsAligned( spuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( offset, 0x10 ) );
	FW_ASSERT( FwIsAligned( pBuffer, 0x10 ) );
	FW_ASSERT( FwIsAligned( dmaSize, 0x10 ) );
	FW_ASSERT( offset+dmaSize <= spuBufferSize );

	pKernelCommand->m_dmaIn.m_cmdId								= kDmaToLsFromEffectiveAddress;
	pKernelCommand->m_dmaIn.m_pseudoAddr.SetPseudoAddr( spuBufferAddr, spuBufferSize, offset );
	pKernelCommand->m_dmaIn.m_effectiveAddr						= (u32) pBuffer;
	pKernelCommand->m_dmaIn.m_dmaSize							= dmaSize >> 4;

/*	pKernelCommand->m_dmaIn.m_cmdId								= kDmaToLsFromEffectiveAddress;
	pKernelCommand->m_dmaIn.m_length							= length;
	pKernelCommand->m_dmaIn.m_spuAddr							= spuAddr;
	pKernelCommand->m_dmaIn.m_effectiveAddr						= (u64) pBuffer;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkDmaToLsFromEaCacheable( SkKernelCommand* pKernelCommand, u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( pBuffer );
	FW_ASSERT( dmaSize > 0 );
	FW_ASSERT( FwIsAligned( spuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( offset, 0x10 ) );
	FW_ASSERT( FwIsAligned( pBuffer, 0x10 ) );
	FW_ASSERT( FwIsAligned( dmaSize, 0x10 ) );
	FW_ASSERT( offset+dmaSize <= spuBufferSize );

	pKernelCommand->m_dmaIn.m_cmdId								= kDmaToLsFromEffectiveAddressCacheable;
	pKernelCommand->m_dmaIn.m_pseudoAddr.SetPseudoAddr( spuBufferAddr, spuBufferSize, offset );
	pKernelCommand->m_dmaIn.m_effectiveAddr						= (u32) pBuffer;
	pKernelCommand->m_dmaIn.m_dmaSize							= dmaSize >> 4;

/*	pKernelCommand->m_dmaIn.m_cmdId								= kDmaToLsFromEffectiveAddress;
	pKernelCommand->m_dmaIn.m_length							= length;
	pKernelCommand->m_dmaIn.m_spuAddr							= spuAddr;
	pKernelCommand->m_dmaIn.m_effectiveAddr						= (u64) pBuffer;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkDmaFromLsToEa( SkKernelCommand* pKernelCommand, u32 spuBufferAddr, u32 spuBufferSize, u32 offset, void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( pBuffer );
	FW_ASSERT( dmaSize > 0 );
	FW_ASSERT( FwIsAligned( spuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( offset, 0x10 ) );
	FW_ASSERT( FwIsAligned( pBuffer, 0x10 ) );
	FW_ASSERT( FwIsAligned( dmaSize, 0x10 ) );
	FW_ASSERT( offset+dmaSize <= spuBufferSize );

	pKernelCommand->m_dmaOut.m_cmdId							= kDmaToEffectiveAddressFromLs;
	pKernelCommand->m_dmaOut.m_pseudoAddr.SetPseudoAddr( spuBufferAddr, spuBufferSize, offset );
	pKernelCommand->m_dmaOut.m_effectiveAddr					= (u32) pBuffer;
	pKernelCommand->m_dmaOut.m_dmaSize							= dmaSize >> 4;

/*	pKernelCommand->m_dmaOut.m_cmdId							= kDmaToEffectiveAddressFromLs;
	pKernelCommand->m_dmaOut.m_length							= length;
	pKernelCommand->m_dmaOut.m_spuAddr							= spuAddr;
	pKernelCommand->m_dmaOut.m_effectiveAddr					= (u64) pBuffer;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkFinish( SkKernelCommand* pKernelCommand )
{
	pKernelCommand->m_simple.m_cmdId							= kFinish;
}

//--------------------------------------------------------------------------------------------------

inline void SkNop( SkKernelCommand* pKernelCommand )
{
	pKernelCommand->m_simple.m_cmdId							= kNop;
}

//--------------------------------------------------------------------------------------------------

inline void SkPrintString( SkKernelCommand* pKernelCommand, const char* str )
{
	FW_ASSERT( str );
	FW_ASSERT( strlen( str )+1 <= 14 );
	pKernelCommand->m_printString.m_cmdId						= kPrintString;
	FwStrCpy( &pKernelCommand->m_printString.m_str[0], str, 14 );
}

//--------------------------------------------------------------------------------------------------

inline void SkCallbackUserFunction( SkKernelCommand* pKernelCommand, SkUserCallbackFunc func, u32 userVal )
{
	FW_ASSERT( func );
	pKernelCommand->m_userCallback.m_cmdId						= kCallInterruptCallback;
	pKernelCommand->m_userCallback.m_userVal					= userVal;
	pKernelCommand->m_userCallback.m_pUserCallbackFunction		= (u32) func;
}

//--------------------------------------------------------------------------------------------------

inline void SkParamPseudoAddr( SkKernelCommand* pKernelCommand, u32 paramId, u32 spuBufferAddr, u32 spuBufferSize, u32 offset )
{
	FW_ASSERT( FwIsAligned( spuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( offset, 0x10 ) );
	FW_ASSERT( paramId < kMaxNumParams );
	FW_ASSERT( spuBufferSize > 0 );

	pKernelCommand->m_paramPseudoAddr.m_cmdId					= kParamPseudoAddr;
	pKernelCommand->m_paramPseudoAddr.m_paramId					= paramId;
	pKernelCommand->m_paramPseudoAddr.m_pseudoAddr.SetPseudoAddr( spuBufferAddr, spuBufferSize, offset );
/*	pKernelCommand->m_paramPseudoAddr.m_cmdId					= kParamPseudoAddr;
	pKernelCommand->m_paramPseudoAddr.m_paramId					= paramId;
	pKernelCommand->m_paramPseudoAddr.m_bufferPseudoLocation	= bufferPseudoLocation;
	pKernelCommand->m_paramPseudoAddr.m_bufferSize				= bufferSize;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkParam64( SkKernelCommand* pKernelCommand, u32 paramId, u64 paramValue64 )
{
	FW_ASSERT( paramId < kMaxNumParams );

	pKernelCommand->m_param64.m_cmdId								= kParam64;
	pKernelCommand->m_param64.m_paramId								= paramId;
	pKernelCommand->m_param64.m_paramValue64						= paramValue64;
}

//--------------------------------------------------------------------------------------------------

inline void SkSetParamsQwords( SkKernelCommand* pKernelCommand, u32 paramBaseIndex, u32 numParams )
{
	FW_ASSERT( (paramBaseIndex+numParams) < (kMaxNumParams+1) );

	pKernelCommand->m_setParamsQwords.m_cmdId						= kSetParamsQwords;
	pKernelCommand->m_setParamsQwords.m_paramBaseIndex				= paramBaseIndex;
	pKernelCommand->m_setParamsQwords.m_numParams					= numParams;
}

//--------------------------------------------------------------------------------------------------

inline void SkRunCode( SkKernelCommand* pKernelCommand, u32 spuAddr, u32 spuBufferSize, u32 entryPointOffset, bool breakpointEnable )
{
	FW_ASSERT( FwIsAligned( spuAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( entryPointOffset, 0x4 ) );

	pKernelCommand->m_runCode.m_cmdId								= kRunCode;
	pKernelCommand->m_runCode.m_breakpoint							= breakpointEnable;
	pKernelCommand->m_runCode.m_spuElfBaseAddr						= spuAddr;
	pKernelCommand->m_runCode.m_spuElfLength						= spuBufferSize;
	pKernelCommand->m_runCode.m_spuEntryPointOffset					= entryPointOffset;
}

//--------------------------------------------------------------------------------------------------

inline void SkSetIdForThisSpu( SkKernelCommand* pKernelCommand, u16 uniqueId )
{
	pKernelCommand->m_setIdForSpu.m_cmdId							= kSetIdForThisSpu;
	pKernelCommand->m_setIdForSpu.m_uniqueId						= uniqueId;
}

//--------------------------------------------------------------------------------------------------

inline void SkStallOnNumSignals( SkKernelCommand* pKernelCommand, u32 numSignals )
{
	FW_ASSERT( numSignals < 8 );	//At most 1 signal from each other SPU

	pKernelCommand->m_stallOnNumSignals.m_cmdId						= kStallOnNumSignals;
	pKernelCommand->m_stallOnNumSignals.m_numSignals				= numSignals;
}

//--------------------------------------------------------------------------------------------------

inline void SkSyncDma( SkKernelCommand* pKernelCommand )
{
    pKernelCommand->m_simple.m_cmdId								= kSyncDma;
}

//--------------------------------------------------------------------------------------------------

inline void SkFindOtherSpu( SkKernelCommand* pKernelCommand, u16 uniqueId )
{
	pKernelCommand->m_findOtherSpu.m_cmdId							= kFindOtherSpu;
	pKernelCommand->m_findOtherSpu.m_uniqueId						= uniqueId;
}

//--------------------------------------------------------------------------------------------------

inline void SkDmaInFromOtherSpu( SkKernelCommand* pKernelCommand, u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize )
{
	FW_ASSERT( FwIsAligned( thisSpuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( thisSpuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( thisSpuOffset, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuOffset, 0x10 ) );
	FW_ASSERT( FwIsAligned( dmaSize, 0x10 ) );
	FW_ASSERT( dmaSize > 0 );


	pKernelCommand->m_dmaInFromOtherSpu.m_cmdId						= kDmaToLsFromOtherSpuLs;
	pKernelCommand->m_dmaInFromOtherSpu.m_thisSpuPseudoAddr.SetPseudoAddr( thisSpuBufferAddr, thisSpuBufferSize, thisSpuOffset );
	pKernelCommand->m_dmaInFromOtherSpu.m_otherSpuPseudoAddr.SetPseudoAddr( otherSpuBufferAddr, otherSpuBufferSize, otherSpuOffset );
	pKernelCommand->m_dmaInFromOtherSpu.m_dmaSize					= ( (dmaSize) >> 4 );
/*	pKernelCommand->m_dmaInFromOtherSpu.m_cmdId						= kDmaToLsFromOtherSpuLs;
	pKernelCommand->m_dmaInFromOtherSpu.m_bufferPseudoLocation		= thisSpuAddr + thisSpuOffset;
	pKernelCommand->m_dmaInFromOtherSpu.m_otherSpuPseudoLocation	= otherSpuBufferAddr + otherSpuOffset;
	pKernelCommand->m_dmaInFromOtherSpu.m_bufferSize				= bufferSize;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkDmaOutToOtherSpu( SkKernelCommand* pKernelCommand, u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize )
{
	FW_ASSERT( FwIsAligned( thisSpuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( thisSpuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( thisSpuOffset, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuBufferAddr, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuBufferSize, 0x10 ) );
	FW_ASSERT( FwIsAligned( otherSpuOffset, 0x10 ) );
	FW_ASSERT( FwIsAligned( dmaSize, 0x10 ) );
	FW_ASSERT( dmaSize > 0 );

	pKernelCommand->m_dmaOutToOtherSpu.m_cmdId						= kDmaFromLsToOtherSpuLs;
	pKernelCommand->m_dmaOutToOtherSpu.m_thisSpuPseudoAddr.SetPseudoAddr( thisSpuBufferAddr, thisSpuBufferSize, thisSpuOffset );
	pKernelCommand->m_dmaOutToOtherSpu.m_otherSpuPseudoAddr.SetPseudoAddr( otherSpuBufferAddr, otherSpuBufferSize, otherSpuOffset );
	pKernelCommand->m_dmaOutToOtherSpu.m_dmaSize					= ( (dmaSize) >> 4 );
/*	pKernelCommand->m_dmaOutToOtherSpu.m_cmdId						= kDmaFromLsToOtherSpuLs;
	pKernelCommand->m_dmaOutToOtherSpu.m_bufferPseudoLocation		= thisSpuAddr + thisSpuOffset;
	pKernelCommand->m_dmaOutToOtherSpu.m_otherSpuPseudoLocation		= otherSpuBufferAddr + otherSpuOffset;
	pKernelCommand->m_dmaOutToOtherSpu.m_bufferSize					= bufferSize;*/
}

//--------------------------------------------------------------------------------------------------

inline void SkSendSignalToOtherSpu( SkKernelCommand* pKernelCommand )
{
	pKernelCommand->m_simple.m_cmdId								= kSendSignalToOtherSpu;
}

//--------------------------------------------------------------------------------------------------

inline void SkStallOnSendSignalToOtherSpu( SkKernelCommand* pKernelCommand )
{
	pKernelCommand->m_simple.m_cmdId								= kStallOnSignalSendToOtherSpu;
}

//--------------------------------------------------------------------------------------------------

inline void SkStallForDependency( SkKernelCommand* pKernelCommand, const SkDependency& dependency )
{
	FW_ASSERT( dependency.IsValidDependency() );

	pKernelCommand->m_stallForDependency.m_cmdId					= kStallForDependency;
	pKernelCommand->m_stallForDependency.m_depData					= dependency;
}

//--------------------------------------------------------------------------------------------------

inline void SkDecrementDependency( SkKernelCommand* pKernelCommand, const SkDependency& dependency )
{
	FW_ASSERT( dependency.IsValidDependency() );

	pKernelCommand->m_decrementDependency.m_cmdId					= kDecrementDependendency;
	pKernelCommand->m_decrementDependency.m_depData					= dependency;
}

//--------------------------------------------------------------------------------------------------

inline void SkInstallSentinel( SkKernelCommand* pKernelCommand, u32 spuPseudoAddr, u32 sentinelId )
{
	FW_ASSERT( FwIsAligned( spuPseudoAddr, 0x10 ) );

	pKernelCommand->m_installSentinelCommand.m_cmdId				= kInstallSentinel;
	pKernelCommand->m_installSentinelCommand.m_pseudoAddr			= spuPseudoAddr;
	pKernelCommand->m_installSentinelCommand.m_id					= sentinelId;
}

//--------------------------------------------------------------------------------------------------

inline void SkVerifySentinel( SkKernelCommand* pKernelCommand, u32 spuPseudoAddr, u32 sentinelId )
{
	FW_ASSERT( FwIsAligned( spuPseudoAddr, 0x10 ) );

	pKernelCommand->m_verifySentinelCommand.m_cmdId					= kVerifySentinel;
	pKernelCommand->m_verifySentinelCommand.m_pseudoAddr			= spuPseudoAddr;
	pKernelCommand->m_verifySentinelCommand.m_id					= sentinelId;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a DefineMemoryOccupancy command to the command list.
					This command is always at the start of the Load Stage, Exec Stage and Store Stage

	@param			baseAddr - The lower bound on the range of SPU pseudo-addresses used by this
					stage of this task

	@param			topAddr - The upper bound on the range of SPU pseudo-addresses used by this
					stage of this task
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DefineMemoryOccupancy( u32 baseAddr, u32 topAddr )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDefineMemoryOccupancy( pKernelCommand, baseAddr, topAddr );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a DmaToLsFromEa command to the command list.
					This command pulls a buffer from an Effective Address into Local Storage

	@param			spuBufferAddr	- The pseudo-address the buffer will be going to

	@param			spuBufferSize	- The size of the buffer that will be being read in

	@param			offset			- Optionally this dma may not want to go to the base of the
										SPU's buffer but could be offset into it

	@param			pBuffer			- Effective Address of the buffer that will be being pulled in

	@param			dmaSize			- The size of the dma being pulled in
									- This read in data must fit inside the spuBufferSize, including
										the potential offset.
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaToLsFromEa( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDmaToLsFromEa( pKernelCommand, spuBufferAddr, spuBufferSize, offset, pBuffer, dmaSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a DmaFromLsToEa command to the command list.
					This command sends a buffer to an Effective Address from Local Storage

	@param			spuBufferAddr	- The pseudo-address the buffer will be being sent out from

	@param			spuBufferSize	- The size of the buffer

	@param			offset			- Optionally this dma may not want to come from the base of the
										SPU's buffer but could be offset into it

	@param			pBuffer			- Effective Address of the buffer that will be being sent to

	@param			dmaSize			- The size of the dma being sent out
									- This sent out data must, including the potential offset, must
										have come from within spuBufferSize
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaFromLsToEa( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDmaFromLsToEa( pKernelCommand, spuBufferAddr, spuBufferSize, offset, pBuffer, dmaSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a DmaToLsFromEa command to the command list.
					This command pulls a buffer from an Effective Address into Local Storage

	@param			spuBufferAddr	- The pseudo-address the buffer will be going to

	@param			spuBufferSize	- The size of the buffer that will be being read in

	@param			offset			- Optionally this dma may not want to go to the base of the
										SPU's buffer but could be offset into it

	@param			pBuffer			- Effective Address of the buffer that will be being pulled in

	@param			dmaSize			- The size of the dma being pulled in
									- This read in data must fit inside the spuBufferSize, including
										the potential offset.

	@note			This command is much like DmaToLsFromEa with the difference that if two
					consecutive tasks use the same input buffer the kernel can choose to use the
					cached version in Local Storage, rather than necessarily having to read it from
					its Effective Address again
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaToLsFromEaCacheable( u32 spuBufferAddr, u32 spuBufferSize, u32 offset, const void* pBuffer, u32 dmaSize )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDmaToLsFromEaCacheable( pKernelCommand, spuBufferAddr, spuBufferSize, offset, pBuffer, dmaSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a Finish command to the command list.
					This command marks the end of the Load, Exec, Store or Post Store Stage

**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::Finish( void )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkFinish( pKernelCommand );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a Nop command to the command list.
					This command does nothing
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::Nop( void  )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkNop( pKernelCommand );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a PrintString command to the command list.

	@param			str			- The string to printed

	@note			This is only recommended for debugging and should not be left in in final code
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::PrintString( const char* str )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkPrintString( pKernelCommand, str );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a CallbackUserFunction command to the command list.

	@param			func			- The function to be called

	@param			userVal			- A 32-bit user value to be passed back to the function

	@note			This is only recommended for debugging and should not be left in in final code
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::CallbackUserFunction( SkUserCallbackFunc func, u32 userVal )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkCallbackUserFunction( pKernelCommand, func, userVal );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a kParamPseudoAddr command to the command list.
					This command converts a pseudo-address into the actual SPU address of a buffer
					in Local Storage and sets it into the params array

	@param			paramId			- The index of the parameter to be set

	@param			spuBufferAddr	- The pseudo-address of the buffer

	@param			spuBufferSize	- The size of the buffer

	@param			offset			- Optionally the pointer may not want to point at the base of
										the SPU's buffer but could be offset into it

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ParamPseudoAddr( u32 paramId, u32 spuBufferAddr, u32 spuBufferSize, u32 offset )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkParamPseudoAddr( pKernelCommand, paramId, spuBufferAddr, spuBufferSize, offset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a 32-bit parameter in the params array
					This command is used when the value is going to be read out of the 32-bit
					preferred slot of the SPU register

	@param			paramId			- The index of the parameter to be set

	@param			paramValue32	- The value to be passed in

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::Param32( u32 paramId, u32 paramValue32 )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	//Annoyingly, the prefered slot for a u32 is actually the upper 32 bits of a u64
	//It's the same command, but we pass it as a u64 and on the SPU the user can use it as u32
	SkParam64( pKernelCommand, paramId, ((u64)paramValue32)<<32L );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a float in the params array
					This command is used when the value is going to be read out of the 32-bit
					preferred slot of the SPU register

	@param			paramId			- The index of the parameter to be set

	@param			floatVal		- The value to be passed in

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ParamFloat( u32 paramId, float floatVal )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	u32 paramValue32 = FwFloatBits( floatVal );

	//Annoyingly, the prefered slot for a u32 is actually the upper 32 bits of a u64
	//It's the same command, but we pass it as a u64 and on the SPU the user can use it as u32
	SkParam64( pKernelCommand, paramId, ((u64)paramValue32)<<32L );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a 64-bit parameter in the params array
					This command is used when the value is going to be read out of the 64-bit
					preferred slot of the SPU register

	@param			paramId			- The index of the parameter to be set

	@param			paramValue64	- The value to be passed in

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::Param64( u32 paramId, u64 paramValue64 )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkParam64( pKernelCommand, paramId, paramValue64 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a 64-bit parameter in the params array
					This command is used when the value is going to be read out of the 64-bit
					preferred slot of the SPU register

	@param			paramId			- The index of the parameter to be set

	@param			ptr				- The 64-bit pointer to be passed in

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ParamEffAddr( u32 paramId, void* ptr )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	u64 paramValue64 = (u32)ptr;
	SkParam64( pKernelCommand, paramId, paramValue64 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a 64-bit parameter in the params array
					This command is used when the value is going to be read out of the 64-bit
					preferred slot of the SPU register

	@param			paramId			- The index of the parameter to be set

	@param			ptr				- The 64-bit pointer to be passed in

	@note			Type safety cannot be guaranteed between these commands and the SPU code that
					reads them
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ParamEffAddr( u32 paramId, const void* ptr )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	u64 paramValue64 = (u32)ptr;
	SkParam64( pKernelCommand, paramId, paramValue64 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass a 128-bit parameter in the params array
					This command first adds a kSetParamsQwords, then adds the 1 qword of data

	@param			paramId			- The index of the parameter to be set

	@param			param			- The value to be passed in
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::SetParam1Qword( u32 paramId, u128 param )
{
	const u32 numParams = 1;
	FW_ASSERT( (paramId + numParams) <= kMaxNumParams );
	FW_ASSERT( (m_currentElement+numParams) < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	m_currentElement += (1+numParams);

	SkSetParamsQwords( pKernelCommand, paramId, numParams );
	pKernelCommand[1].m_data = param;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Pass multiple 128-bit parameters in the params array
					This command first adds a kSetParamsQwords, then adds the requisite number of
					qwords of data

	@param			paramBaseIndex	- The index of the first parameter to be set

	@param			numParams		- The number of parameters to be set

	@param			pParams			- A pointer to the array of qwords to be passes as params
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::SetParamQwords( u32 paramBaseIndex, u32 numParams, const u128* pParams )
{
	FW_ASSERT( (paramBaseIndex + numParams) <= kMaxNumParams );
	FW_ASSERT( (m_currentElement+numParams) < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	m_currentElement += (1+numParams);

	SkSetParamsQwords( pKernelCommand, paramBaseIndex, numParams );

	//And copy the params into the Command List
	do
	{
		++pKernelCommand;
		pKernelCommand->m_data = *pParams;
		--numParams;
		++pParams;
	} while( numParams );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a RunCode command to the Command List

	@param			spuBufferAddr	- The pseudo-address of the buffer the elf was loaded into

	@param			spuBufferSize	- The size of the buffer the elf was loaded into

	@param			entryPointOffset- The entry point of the elf relative to the base of the buffer
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::RunCode( u32 spuBufferAddr, u32 spuBufferSize, u32 entryPointOffset )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkRunCode( pKernelCommand, spuBufferAddr, spuBufferSize, entryPointOffset, false );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a RunCode command to the Command List

	@param			spuBufferAddr	- The pseudo-address of the buffer the elf was loaded into

	@param			spuBufferSize	- The size of the buffer the elf was loaded into

	@param			entryPointOffset- The entry point of the elf relative to the base of the buffer

	@note			This function is much the same as RunCode, with the difference that it sets a
					breakpoint ("stop 1") just at the point the code is going to be run
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::RunCodeWithBreakpoint( u32 spuAddr, u32 spuBufferSize, u32 entryPointOffset )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkRunCode( pKernelCommand, spuAddr, spuBufferSize, entryPointOffset, true );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set the uniqueId provided in the slot in the Shared Area that belongs to the
					SPU that executes this command

	@param			uniqueId		- A 16-bit unique identifier
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::SetIdForThisSpu( u16 uniqueId )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkSetIdForThisSpu( pKernelCommand, uniqueId );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Stall at this command until the requested number of signals have been sent to
					the Signal Notification Register

	@param			numSignals		- Number of signals (ie. SPUs) to wait for
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::StallOnNumSignals( u32 numSignals )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkStallOnNumSignals( pKernelCommand, numSignals );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Stall at this command until all dmas have completed
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::SyncDma( void )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkSyncDma( pKernelCommand );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Look in the Shared Area to find which SPU has posted the specified uniqueId and
					remember it as the "Other SPU".
					If that uniqueId is not yet visible, stall at this command

	@param			uniqueId		- A 16-bit unique identifier
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::FindOtherSpu( u16 uniqueId )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkFindOtherSpu( pKernelCommand, uniqueId );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a kDmaToLsFromOtherSpuLs command to the command list.
					This command pulls a buffer in to Local Storage from the Local Storage of the
					"Other SPU".

	@param			thisSpuBufferAddr	- The pseudo-address of the buffer on the SPU that executed
											this command that will be being written in to

	@param			thisSpuBufferSize	- The size of the buffer on this SPU

	@param			thisSpuOffset		- Optionally this dma may not want to go to the base of the
											SPU's buffer but could be offset into it

	@param			otherSpuBufferAddr	- The pseudo-address of the buffer on the "Other SPU" that
											this dma will be read from

	@param			otherSpuBufferSize	- The size of the buffer on the "Other SPU"

	@param			otherSpuOffset		- Optionally this dma may not want to come from the base of
											the "Other SPU"'s buffer but could be offset into it

	@param			dmaSize				- The size of the dma being transferred
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaInFromOtherSpu( u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDmaInFromOtherSpu( pKernelCommand, thisSpuBufferAddr, thisSpuBufferSize, thisSpuOffset, otherSpuBufferAddr, otherSpuBufferSize, otherSpuOffset, dmaSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Add a kDmaFromLsToOtherSpuLs command to the command list.
					This command sends a buffer from the Local Storage of the SPU that executed
					this command, in to Local Storage of the "Other SPU".

	@param			thisSpuBufferAddr	- The pseudo-address of the buffer on the SPU that executed
											this command that will be being read from to

	@param			thisSpuBufferSize	- The size of the buffer on this SPU

	@param			thisSpuOffset		- Optionally this dma may not want to come from the base of
											the SPU's buffer but could be offset into it

	@param			otherSpuBufferAddr	- The pseudo-address of the buffer on the "Other SPU" that
											this dma will be written to

	@param			otherSpuBufferSize	- The size of the buffer on the "Other SPU"

	@param			otherSpuOffset		- Optionally this dma may not want to go to the base of the
											"Other SPU"'s buffer but could be offset into it

	@param			dmaSize				- The size of the dma being transferred
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaOutToOtherSpu( u32 thisSpuBufferAddr, u32 thisSpuBufferSize, u32 thisSpuOffset, u32 otherSpuBufferAddr, u32 otherSpuBufferSize, u32 otherSpuOffset, u32 dmaSize )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDmaOutToOtherSpu( pKernelCommand, thisSpuBufferAddr, thisSpuBufferSize, thisSpuOffset, otherSpuBufferAddr, otherSpuBufferSize, otherSpuOffset, dmaSize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Send a Signal to the Signal Notification Register of the "Other SPU"
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::SendSignalToOtherSpu( void  )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkSendSignalToOtherSpu( pKernelCommand );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Stall at this command until the write to the "Other SPU"'s Signal Notification
					Register has completed
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::StallOnSendSignalToOtherSpu( void  )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkStallOnSendSignalToOtherSpu( pKernelCommand );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Stall at this command until the require dependency has been fulfilled

	@param			dependency		- The identifier for the Dependency being waited on			
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::StallForDependency( const SkDependency& dependency )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkStallForDependency( pKernelCommand, dependency );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Decrement the value of the current dependency

	@param			dependency		- The identifier for the Dependency being waited on			
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DecrementDependency( const SkDependency& dependency )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkDecrementDependency( pKernelCommand, dependency );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Place a sentinel marker at the specified address

	@param			spuPseudoAddr	- The pseudo-addr in LS to store the sentinel at

	@param			sentinelId		- The value to store to mark the sentinel
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::InstallSentinel( u32 spuPseudoAddr, u32 sentinelId )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkInstallSentinel( pKernelCommand, spuPseudoAddr, sentinelId );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Check the existence of a sentinel marker at the specified address

	@param			spuPseudoAddr	- The pseudo-addr in LS to check for the sentinel at

	@param			sentinelId		- The value to check for sentinel marking
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::VerifySentinel( u32 spuPseudoAddr, u32 sentinelId )
{
	FW_ASSERT( m_currentElement < m_numCommands );
	SkKernelCommand* pKernelCommand = &m_pCommandListBase[ m_currentElement ];
	++m_currentElement;

	SkVerifySentinel( pKernelCommand, spuPseudoAddr, sentinelId );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Zeros out the current cache-line in the cache
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ZeroCacheLine( void )
{
	const u32 offset = 0;
	void* addr = &m_pCommandListBase[ m_currentElement ];

	FW_ASSERT( FwIsAligned( addr, 128 ) );

	//Set a whole cache-line to zero in the cache
	asm ( "dcbz			%[base],	%[offset]		\n"
		:
		:	[base]		"r"			(addr),
			[offset]	"r"			(offset)
		:				"memory"
		);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Zeros out the next cache-line in the cache
**/
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::ZeroNextCacheLine( void )
{
	const u32 offset = 128;
	void* addr = &m_pCommandListBase[ m_currentElement ];

	FW_ASSERT( FwIsAligned( addr, 128 ) );

	//Set a whole cache-line to zero in the cache
	asm ( "dcbz			%[base],	%[offset]		\n"
		:
		:	[base]		"r"			(addr),
			[offset]	"r"			(offset)
		:				"memory"
		);
}

//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DefineMemoryOccupancy( const SkMemoryOccupancy& memOcc  )
{
	FW_ASSERT( memOcc.m_baseAddr != 0xFFFFFFFF );
	DefineMemoryOccupancy( memOcc.m_baseAddr, memOcc.m_topAddr );
}
inline void SkCommandList::DmaToLsFromEa( const SkLsBuffer& input, const void* pBuffer, u32 dmaSize )
{
	DmaToLsFromEa( input.m_baseAddr, input.m_size, 0, pBuffer, dmaSize );
}
inline void SkCommandList::DmaFromLsToEa( const SkLsBuffer& output, void* pBuffer, u32 dmaSize )
{
	DmaFromLsToEa( output.m_baseAddr, output.m_size, 0, pBuffer, dmaSize );
}
inline void SkCommandList::DmaToLsFromEaCacheable( const SkLsBuffer& input, const void* pBuffer, u32 dmaSize )
{
	DmaToLsFromEaCacheable( input.m_baseAddr, input.m_size, 0, pBuffer, dmaSize );
}
inline void SkCommandList::DmaToLsFromElf( const SkElf& rElf, const SkLsBuffer& input, bool cacheable )
{
	DmaToLsFromElf( rElf, input.m_baseAddr, input.m_size, cacheable );
}
inline void SkCommandList::ParamPseudoAddr( u32 paramId, const SkLsBuffer& buffer )
{
	ParamPseudoAddr( paramId, buffer.m_baseAddr, buffer.m_size, 0 );
}
inline void SkCommandList::RunCode( const SkLsBuffer& codeBuffer, u32 entryPointOffset )
{
	RunCode( codeBuffer.m_baseAddr, codeBuffer.m_size, entryPointOffset );
}
inline void SkCommandList::RunCodeWithBreakpoint( const SkLsBuffer& codeBuffer, u32 entryPointOffset )
{
	RunCodeWithBreakpoint( codeBuffer.m_baseAddr, codeBuffer.m_size, entryPointOffset );
}
inline void SkCommandList::DmaInFromOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaInFromOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, 0, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, 0, dmaSize );
}
inline void SkCommandList::DmaOutToOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaOutToOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, 0, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, 0, dmaSize );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

inline void SkCommandList::DmaToLsFromEa( const SkLsSubBuffer& input, const void* pBuffer, u32 dmaSize )
{
	DmaToLsFromEa( input.m_baseAddr, input.m_size, input.m_offset, pBuffer, dmaSize );
}
inline void SkCommandList::DmaFromLsToEa( const SkLsSubBuffer& output, void* pBuffer, u32 dmaSize )
{
	DmaFromLsToEa( output.m_baseAddr, output.m_size, output.m_offset, pBuffer, dmaSize );
}
inline void SkCommandList::DmaToLsFromEaCacheable( const SkLsSubBuffer& input, const void* pBuffer, u32 dmaSize )
{
	DmaToLsFromEaCacheable( input.m_baseAddr, input.m_size, input.m_offset, pBuffer, dmaSize );
}
inline void SkCommandList::ParamPseudoAddr( u32 paramId, const SkLsSubBuffer& buffer )
{
	ParamPseudoAddr( paramId, buffer.m_baseAddr, buffer.m_size, buffer.m_offset );
}
inline void SkCommandList::DmaInFromOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaInFromOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, 0, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, otherSpuBuffer.m_offset, dmaSize );
}
inline void SkCommandList::DmaInFromOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaInFromOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, thisSpuBuffer.m_offset, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, 0, dmaSize );
}
inline void SkCommandList::DmaInFromOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaInFromOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, thisSpuBuffer.m_offset, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, otherSpuBuffer.m_offset, dmaSize );
}
inline void SkCommandList::DmaOutToOtherSpu( const SkLsBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaOutToOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, 0, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, otherSpuBuffer.m_offset, dmaSize );
}
inline void SkCommandList::DmaOutToOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaOutToOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, thisSpuBuffer.m_offset, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, 0, dmaSize );
}
inline void SkCommandList::DmaOutToOtherSpu( const SkLsSubBuffer& thisSpuBuffer, const SkLsSubBuffer& otherSpuBuffer, u32 dmaSize )
{
	DmaOutToOtherSpu( thisSpuBuffer.m_baseAddr, thisSpuBuffer.m_size, thisSpuBuffer.m_offset, otherSpuBuffer.m_baseAddr, otherSpuBuffer.m_size, otherSpuBuffer.m_offset, dmaSize );
}

//--------------------------------------------------------------------------------------------------

inline void SkCommandList::InstallSentinel( const SkSentinel& sentinel )
{
	InstallSentinel( sentinel.m_addr, sentinel.m_id );
}
inline void SkCommandList::VerifySentinel( const SkSentinel& sentinel )
{
	VerifySentinel( sentinel.m_addr, sentinel.m_id );
}

//--------------------------------------------------------------------------------------------------

#endif // SPU_KERNEL_INTERFACE_H

