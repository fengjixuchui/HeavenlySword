//--------------------------------------------------------------------------------------------------
/**
	@file		SkTask.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_TASK_H
#define SK_TASK_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/spu_stop_and_signal.h>
#include <sys/spu_event.h>

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  EXTERNAL DECLARATIONS
//--------------------------------------------------------------------------------------------------

extern SkKernelSetupData gKernelSetupData;

extern SkSharedAreaInternal gSharedAreaCopy;

extern "C" u32 GetDependencyValue( const SkDependencyQword dependencyQword );
extern "C" void DecrementDependencyValue( const SkDependencyQword dependencyQword );


extern u64			gCachingLoadEffecitveAddress;	//The EA address of the cacheable buffer that is presently being loaded
extern const void*	gCachingLocalStoreAddress;	//The LS address that the cacheable buffer is presently being loaded to

extern u64			gCachedLoadEffecitveAddress;	//The EA address the currently available cached buffer came from
extern const void*	gCachedLocalStoreAddress;	//The LS address of the currently available cached buffer

//--------------------------------------------------------------------------------------------------

inline u128 RotateQwordLeft( u128 val, u8 shiftBits )
{
	shiftBits	= shiftBits & 0x7F;

	u128 val1	= spu_rlqw( val, shiftBits );			//rotate left by (shiftBits & 7) bits
	u128 ret	= spu_rlqwbytebc( val1, shiftBits );	//rotate left by (shiftBits >> 3) bytes
	return ret;
}

//--------------------------------------------------------------------------------------------------

inline u128 RotateQwordRight( u128 val, u8 shiftBits )
{
	return RotateQwordLeft( val, 128-shiftBits );
}

//--------------------------------------------------------------------------------------------------

/*inline void PrintQword( const char* str, qword qw )
{
	union
	{
		qword	qw;
		u32		u_32[4];
	} u;

	u.qw = qw;
	FwPrintf( "%s = 0x%08X_%08X_%08X_%08X\n", str, u.u_32[0], u.u_32[1], u.u_32[2], u.u_32[3] );
}*/

//--------------------------------------------------------------------------------------------------

inline void MemsetLs( void* spuAddr, u8 val, u32 length )
{
	u128* outPtr	= (u128*) spuAddr;
	u128 val128		= (u128)spu_splats( val );
	do
	{
		length -= 16;
		*outPtr = val128;
		++outPtr;
	} while ( length > 0 );
}

//--------------------------------------------------------------------------------------------------

inline void MemcpyLs( void* spuDestAddr, const void* spuSrcAddr, u32 length )
{
	u128* outPtr		= (u128*) spuDestAddr;
	const u128* inPtr	= (const u128*) spuSrcAddr;
	do
	{
		length -= 16;
		*outPtr = *inPtr;
		++inPtr;
		++outPtr;
	} while ( length > 0 );
}

//--------------------------------------------------------------------------------------------------

inline u32 GetSpuId( void )
{
	return (gKernelSetupData.m_thisSpuId);
}

//--------------------------------------------------------------------------------------------------

inline u32 CountBitsInByte( u32 bits )
{
	return si_to_uint( si_cntb( si_from_uint( bits ) ) );
}

//--------------------------------------------------------------------------------------------------

//It'd probably be nice to make this a member function of PseudoAddrHalfs, but sadly that seems to
// generate significantly worse code :(
inline u32 AddrFromPseudoAddr( s32 bufferAllocation, PseudoAddrHalfs pseudoAddr )
{
#if 1
	u32 spuAddr;
	if ( bufferAllocation == kAllocationDirectionBottomUp )
		spuAddr = kSpuUserBaseAddr + ((pseudoAddr.m_spuBaseOffset) << 4);
	else
		spuAddr = kSpuUserTopAddr + ((pseudoAddr.m_spuTopOffset) << 4);
	return spuAddr;
#else
	//Actually, better to only do the shift once.
	//These two shifts are both on compile time constants.
	//Err.. but for some reason when it's inlined this one makes it worse :(
	u32 spuAddrShifted;
	if ( bufferAllocation == kAllocationDirectionBottomUp )
		spuAddrShifted = (kSpuUserBaseAddr>>4) + pseudoAddr.m_spuBaseOffset;
	else
		spuAddrShifted = (kSpuUserTopAddr>>4) + pseudoAddr.m_spuTopOffset;
	return (spuAddrShifted<<4);
#endif
}

//--------------------------------------------------------------------------------------------------

//It'd probably be nice to make this a member function of PseudoAddrWords, but sadly that seems to
// generate significantly worse code :(
inline u32 AddrFromPseudoAddr( s32 bufferAllocation, PseudoAddrWords pseudoAddr )
{
	u32 spuAddr;
	if ( bufferAllocation == kAllocationDirectionBottomUp )
		spuAddr = kSpuUserBaseAddr + pseudoAddr.m_spuBaseOffset;
	else
		spuAddr = kSpuUserTopAddr + pseudoAddr.m_spuTopOffset;
	return spuAddr;
}

//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

class SkTask
//struct SkTask
{
private:
	const char*		m_pTaskString;				//String that is pre-pended onto all printfs of this task
	u128*			m_pCommandStackCache;		//Base address of command stack cache for this task

	u64				m_nextLoadPointer;			//main mem addr of command stack - points at next chunk to load
	s32				m_bufferAllocation;			// -1 = unallocated, 0 = bottom up, 1 = top down

	u32				m_otherSpuId;
	u32				m_otherSpuAllocationDirection;

	u32				m_currProcessPointer __attribute__(( aligned(16) ));		//one byte tracks where we are in the ring buffer of commands

	u32				m_currentSlotBaseAddr __attribute__(( aligned(16) ));		//baseAddr of memory occupation of current stage of this task
	u32				m_currentSlotTopAddr  __attribute__(( aligned(16) ));		//topAddr of memory occupation of current stage of this task

	SkKernelCommand	m_cachedKernelCommand;

	void			CacheKernelCommand( SkKernelCommand command )		{ m_cachedKernelCommand = command; }

	static u128		ms_paramsArray[kMaxNumParams] __attribute__(( aligned(256) ));	//Only for use by the current execTask

	bool			RunCommandsUntilStop( void );
	void			LoadNextCommandBuffer( void );
	SkKernelCommand	GetNextKernelCommand( void );

	void			SetIdForSpu( u16 uniqueId );
	bool			FindOtherSpuById( u16 uniqueId );
	bool			FindOtherSpusByIdAndSetAsParams( u16 uniqueId, u16 paramsBaseId, u16 numWantedSpus );

	static u32		ms_signalAccumulator __attribute__(( aligned(16) ));	//Only valid for usage during the ExecStage

public:
	void			SetCommandStackCacheSpuPointer( u128* ptr ) { m_pCommandStackCache = ptr; }

	void			SetCommandStackBaseAndStartLoading( u64 mainMemAddr );

	void			GetMemoryUsageMap( void );

	bool			RunLoadCommands( void )			{	return RunCommandsUntilStop();		}
	bool			RunExecuteCommands( void )		{	return RunCommandsUntilStop();		}
	bool			RunStoreCommands( void )		{	return RunCommandsUntilStop();		}
	bool			RunPostStoreCommands( void )	{	return RunCommandsUntilStop();		}

	bool			FitsWith( const SkTask* pOtherTask ) const;
	bool			Allocate( const SkTask* pOtherTask );

	void			SetTaskString( const char* str )	{ m_pTaskString = str; }

	void			SetDirectionToUnassigned( void )	{ m_bufferAllocation = kAllocationDirectionUnassigned; }

} __attribute__(( aligned(16) ));

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline bool SkTask::Allocate( const SkTask* pOtherTask )
{
	//If we've already been allocated then don't run this function again
	if ( m_bufferAllocation > kAllocationDirectionUnassigned )
	{
		return true;
	}

	//If the other task doesn't exist
	if ( pOtherTask == NULL )
	{
		//then we can just allocate ourselves from the base and it doesn't matter
#if 1
		m_bufferAllocation = kAllocationDirectionBottomUp;
		COMMAND_PRINTF( ( "Allocating from the base up (against no other task)\n" ) );
#else
		m_bufferAllocation = kAllocationDirectionTopDown;
		COMMAND_PRINTF( ( "Allocating from the top down (against no other task)\n" ) );
#endif
		return true;
	}

	u32 otherBaseAddr;
	u32 otherTopAddr;

	if ( pOtherTask->m_bufferAllocation == kAllocationDirectionBottomUp )
	{
		//Working this out just in address relative to zero as the base (although actually it could be higher
		otherBaseAddr	= pOtherTask->m_currentSlotBaseAddr;
		otherTopAddr	= pOtherTask->m_currentSlotTopAddr;
	}
	else
	{
		//Working this out just in address relative to zero as the base (although actually it could be higher
		otherBaseAddr	= kSpuUserTopAddr - kSpuUserBaseAddr - pOtherTask->m_currentSlotTopAddr;
		otherTopAddr	= kSpuUserTopAddr - kSpuUserBaseAddr - pOtherTask->m_currentSlotBaseAddr;
	}

	if ( (m_currentSlotTopAddr <= otherBaseAddr) || ( m_currentSlotBaseAddr >= otherTopAddr ) )
	{
		m_bufferAllocation	= kAllocationDirectionBottomUp;
		COMMAND_PRINTF( ( "Allocating from the base up\n" ) );
		return true;
	}

	//Working this out just in address relative to zero as the base (although actually it could be higher
	u32 reversedBaseAddr	= kSpuUserTopAddr - kSpuUserBaseAddr - m_currentSlotTopAddr;
	u32 reversedTopAddr		= kSpuUserTopAddr - kSpuUserBaseAddr - m_currentSlotBaseAddr;

	if ( ( reversedTopAddr <= otherBaseAddr ) || ( reversedBaseAddr >= otherTopAddr ) )
	{
		m_bufferAllocation	= kAllocationDirectionTopDown;
		COMMAND_PRINTF( ( "Allocating from the top down\n" ) );
		return true;
	}


    m_bufferAllocation		= kAllocationDirectionUnassigned;	//can't allocate
	return false;
}

//--------------------------------------------------------------------------------------------------

inline void SkTask::SetCommandStackBaseAndStartLoading( u64 mainMemAddr )
{
	FW_ASSERT( FwIsAligned( mainMemAddr, 16 ) );


	//Yikes.  This has been a nasty bug.
	//Suppose the Finish command of a Post-Store Stage is in the last qword of a 128-byte chunk
	// then when that command is taken, another 128-byte chunk will start being loaded speculatively
	// That 128-byte chunk isn't needed.
	// That task in fact has reached the end of its lifetime
	// A fetchTask now gets promoted into the commandLoadStage and takes up the pipeline slot memory
	// made available by that previously vacated task.
	// This commandLoad task now starts 2 dams into the command cache as seen in this function.
	// This then means we now have the earlier dma to this address that we are no longer interested in
	// and also one of these new dmas too.  The hardware can potentially execute these dmas out of order
	// and we'll get the commands we didn't want.
	//Hence we need to sync here (actually, just fencing would do) to make sure that previous dma has
	// completed before we start these new ones.
	SkDma::SyncChannel( DMA_TAG_COMMAND_STACK_LOADING );

	//Hmmm... maybe I should have 4 different tags for my 4 different tasks actually so that a task
	// only has to stall for its own dmas

	//From the mainMemAddr, round down and load that dma slice into one of the commandStackCache's
	// and move forwards 0x80 and load that slice into the other commandStackCache buffer
	u64 loadPointer				= mainMemAddr & ~0x7F;
	u32 whichBuffer				= (mainMemAddr & 0x80);

	TRACE(( "Loading command buffers from 0x%08X and 0x%08X\n", (u32)loadPointer, (u32)(loadPointer+0x80) ));

	//FwPrintf( "m_pCommandStackCache = 0x%08X\n", (u32)m_pCommandStackCache );
	u32 buff0					= whichBuffer;						//0x00 or 0x80
	u32 buff1					= (whichBuffer + 0x80) & 0x80;		//0x80 or 0x00
	SkDma::BeginDmaIn( (void*) (((u32)m_pCommandStackCache) + buff0),		loadPointer,		0x80,	DMA_TAG_COMMAND_STACK_LOADING );
	SkDma::BeginDmaIn( (void*) (((u32)m_pCommandStackCache) + buff1),		loadPointer+0x80,	0x80,	DMA_TAG_COMMAND_STACK_LOADING );
	m_nextLoadPointer		= (loadPointer + 0x100);
	m_currProcessPointer	= mainMemAddr & 0xFF;
}

//--------------------------------------------------------------------------------------------------

inline void SkTask::LoadNextCommandBuffer( void )
{
	u64 loadPointer			= m_nextLoadPointer;
	u32 whichBuffer			= (loadPointer & 0x80);

	TRACE(( "Loading command buffer from 0x%08X\n", (u32)loadPointer ));

	SkDma::BeginDmaIn( (void*) (((u32)m_pCommandStackCache) + whichBuffer),	loadPointer,		0x80,	DMA_TAG_COMMAND_STACK_LOADING );
	m_nextLoadPointer		= loadPointer + 0x80;
}

//--------------------------------------------------------------------------------------------------

inline void SkTask::SetIdForSpu( u16 uniqueId )
{
	bool storeSuccess;

	do
	{
		SkDma::DmaLoadAndReserve( &gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );

		gSharedAreaCopy.m_uniqueIds[GetSpuId()]	= uniqueId;
		u8 allocDir								= gSharedAreaCopy.m_allocationDirections;
		allocDir								= allocDir & (~gKernelSetupData.m_thisSpuIdBit);
		allocDir								= allocDir | (m_bufferAllocation << gKernelSetupData.m_thisSpuId);
		gSharedAreaCopy.m_allocationDirections	= allocDir;

		storeSuccess = SkDma::DmaStoreWithReservation( &gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );

	} while ( !storeSuccess );
}

//--------------------------------------------------------------------------------------------------

inline bool SkTask::FindOtherSpuById( u16 uniqueId )
{
	SkDma::DmaLoadAndReserve( &gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );
			//If we guaranteed updating the gSharedAreaCopy
			// in SkMain.cpp could possibly skip updating here

#if 0	//Work in progress - not working yet
	vector unsigned short uniqueId128	= spu_splats( uniqueId );
	vector unsigned short compMask		= spu_cmpeq( uniqueId128, *(vector unsigned short*)gSharedAreaCopy.m_uniqueIds );
	vector unsigned int gathered		= spu_gather( compMask );
	//gathered = 0x000000??		At most one bit set
	vector unsigned int lzCount			= spu_cntlz( gathered );
	vector unsigned int eltNo128		= spu_sub( 32, lzCount );

	uint eltNo = si_to_uint( (qword) eltNo128 );
	if ( eltNo )
	{
		eltNo -= 1;
		m_otherSpuId = eltNo;
		m_otherSpuAllocationDirection	= (gSharedAreaCopy.m_allocationDirections >> eltNo) & 0x1;
		return true;
	}
	return false;
#else
//Write this in a sexy parallel manner
	for (u32 spuId = 0; spuId < 8; ++spuId)
	{
		if ( gSharedAreaCopy.m_uniqueIds[ spuId ] == uniqueId )
		{
			m_otherSpuId					= spuId;
			m_otherSpuAllocationDirection	= (gSharedAreaCopy.m_allocationDirections >> spuId) & 0x1;
COMMAND_PRINTF( ( "Found other SPU (spu_num = %d, alloc dir = %d)\n", spuId, m_otherSpuAllocationDirection ) );
			return true;
		}
	}

#endif
	//storeSuccess = SkDma::DmaStoreWithReservation( (u32)&gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );

//FwPrintf( "Failed to find other SPU by ID\n" );
	return false;
}

//--------------------------------------------------------------------------------------------------

inline bool SkTask::FindOtherSpusByIdAndSetAsParams( u16 uniqueId, u16 paramsBaseId, u16 numWantedSpus )
{
	u32 numFoundSpus = 0;

	SkDma::DmaLoadAndReserve( &gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );
			//If we guaranteed updating the gSharedAreaCopy
			// in SkMain.cpp could possibly skip updating here

//Write this in a sexy parallel manner?
	for (u32 spuId = 0; spuId < 8; ++spuId)
	{
		if ( gSharedAreaCopy.m_uniqueIds[ spuId ] == uniqueId )
		{
			ms_paramsArray[paramsBaseId + numFoundSpus] = si_from_uint( spuId );
			++numFoundSpus;
		}
	}

	//storeSuccess = SkDma::DmaStoreWithReservation( (u32)&gSharedAreaCopy, gKernelSetupData.m_ydramSharedAreaAddr );

	return ( numFoundSpus == numWantedSpus );	//true if we've found them all
}

//--------------------------------------------------------------------------------------------------

inline void StartOfExecStage( void )
{
	//During the Load Stage we *might* have set a cacheable buffer loading.
	//If we have, promote it to now being a cached buffer during our Exec Stage,
	// and reset the caching buffer to NULL so that the next task in the Load Stage
	// can optionally load another buffer
	gCachedLoadEffecitveAddress		= gCachingLoadEffecitveAddress;
	gCachedLocalStoreAddress		= gCachingLocalStoreAddress;
	gCachingLoadEffecitveAddress	= NULL;
	gCachingLocalStoreAddress		= NULL;
}

//--------------------------------------------------------------------------------------------------

inline void EndOfExecStage( void )
{
	//The cached buffer is no longer available for reading so clear it to NULL
	gCachedLoadEffecitveAddress		= NULL;
	gCachedLocalStoreAddress		= NULL;
}

//--------------------------------------------------------------------------------------------------

#endif // SK_TASK_H
