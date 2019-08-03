/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		This file defines the main structures that define a job list, job, and job commands.
				They are shared between both PPU and SPU.
				They are used for communicating data between the two.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_DEFINITION_H
#define WWS_JOB_JOB_DEFINITION_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

//--------------------------------------------------------------------------------------------------

namespace JobHeaderCommand
{
	enum
	{
		kNoJobYet = 0,		///< No job written yet, but might be (this value MUST be 0)
		kJobExists,			///< This job exists
		kGeneralBarrier,	///< This is a general barrier
		kNopJob				///< This is a gap in the joblist.  More real jobs follow after it
	};
}

//--------------------------------------------------------------------------------------------------

union JobHeader
{
	struct
	{
		U16		m_enableBreakpoint:4;	///< normally 0, or non-zero if you want to single step code
		U16		m_loadCommandsSize:12;	///< size of load commands, qword aligned
		U16		m_jobHeaderCommand;		///< job header command
		U32		m_mmaLoadCommands;		///< main mem address of load commands, qword aligned
	};
	U64 m_u64;
	U32	m_u32[2];
	U16	m_u16[4];
	U8	m_u8[8];
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct JobListHeaderCore
{
	U32			m_mmaJobManager;			//This is just temporary since during development our PM will be over 16K
	U32			m_jobManagerSize;			//Once we're done and under 16K, this can go.
	U32			m_pad1;
	U32			m_pad2;

	U16			m_numJobsInListHint;		// #jobs in list is >= this value
	U16			m_numJobsTaken;				// #jobs taken by all spus
	U16			m_minActive[6];				// no jobs before this are active (in each spu)
} WWSJOB_ALIGNED(16);	//I've just put this in a separate struct for now so that I can use sizeof

struct JobListHeader : public JobListHeaderCore
{
	JobHeader	m_jobHeader[12];			// The first 14 JobHeader's (8 bytes each) pad out the 128 bytes
	// Note that more JobHeader's can follow here (#jobs <= 0xFFFF)
	// Note: we are considering (in the future) making each JobHeader 4 bytes instead of 8.
} WWSJOB_ALIGNED(128);

//--------------------------------------------------------------------------------------------------

union WwsJob_BufferSet	// 4 bytes, 4 byte aligned.  If all 0 then inactive
{
	struct
	{
		U8	m_firstPageNum;				// 0:0xFF
		U8	m_numPagesPerBuffer;		// 0:0xFF
		unsigned char m_reserved	:1;	// 1 if reservedPage bits set for entire bufferSet
		unsigned char m_numBuffers	:7;	// 1:MAX_NUM_BUFFERS_PER_JOB
		U8	m_firstBufferNum;			// includes job#
	};
	U32	m_u32;
	U16	m_u16[2];
	U8	m_u8[4];
} WWSJOB_ALIGNED( 4 );

//--------------------------------------------------------------------------------------------------

struct DependencyCounter
{
	U16	m_counter;
	U8	m_workloadId;
	U8	m_readyCount;
} WWSJOB_ALIGNED( 4 );

//--------------------------------------------------------------------------------------------------

// room reserved for 8 commands
namespace CommandNum
{
	enum
	{
		kNop = 0/*fixed*/,	// For NOP, you *must* have first u32 = 0
		kReserveBufferSet,
		kUseBuffer,
		kUnreserveBufferSets,
		kRequestDependencyDecrement,
		kRunJob,		   	// this also ends the load commands
		kEndCommand,	   	// this ends the run commands
		kInvalidCommand		// RunJob commands are cleared to this invalid value once they have executed
	};
}

//--------------------------------------------------------------------------------------------------

// note: if command is kNop then entire word is 0
struct CommandFlags	// 4 bytes, 4 byte aligned
{
	unsigned m_mmLengthInQwords			:14;// length of data in main memory in qwords
	unsigned m_pad						:1;
	unsigned m_continueAfterDiscardWrite:1;	// normally 0.  This is only set by JobManager!!!

	unsigned m_inputRead				:1;	// 1 if any read done
	unsigned m_inputGather				:1; // 1 if read is via a list dma
	unsigned m_outputShareable			:1;  
	unsigned m_shareableWriteIfDiscarded:1;	// can only be set if m_outputShareable is set!!!
	unsigned m_commandNum				:3;
	unsigned m_logicalBufferSetNum		:4;	// MUST be enough bits to encode MAX_NUM_BUFFER_SETS_PER_JOB
	unsigned m_logicalBufferNum			:5; // MUST be enough bits to encode MAX_NUM_BUFFERS_PER_JOB
};

//--------------------------------------------------------------------------------------------------

struct ReserveBufferSetCommand // ALL unspecified data must be set to 0
{	// note: if command is kNop then first word is 0
	CommandFlags/*4*/   	m_flags;		// set m_logicalBufferSetNum & m_commandNum
	WwsJob_BufferSet/*4*/	m_bufferSet;	// Note: m_reserved & m_firstBufferNum is 0
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct UseBufferCommand // ALL unspecified data must be set to 0
{
	union
	{
		CommandFlags/*4*/	m_flags;				// everything set
		U32					m_u32Flags;
	};

	unsigned			m_mmAddressInQwords				:28;
	unsigned			m_pad							:1;
	unsigned			m_numPadQwordsBelowListElements	:3;	// only used if readGather
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct UnreserveBufferSetsCommand  // ALL unspecified data must be set to 0
{
	CommandFlags/*4*/   m_flags;			// set m_commandNum
	U32					m_bufferSetMask;	// low bit (bit 31) corresponds to bufferSetNum 0
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct DepDecCommand // ALL unspecified data must be set to 0
{
	CommandFlags	m_flags;				// set m_commandNum

	U32				m_mmDependencyAddress;
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct RunJobCommand // ALL unspecified data must be set to 0
{
//	U16				m_enableBreakpoint;
	CommandFlags	m_flags;				// set m_logicalBufferSetNum, m_logicalBufferNum, m_commandNum, m_enableBreakpoint

	U32				m_pad;

	// optional 8 byte pad to qword alignment

	// parameters start here (qword aligned, as defined by job)
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct EndCommand // ALL unspecified data must be set to 0
{
	CommandFlags	m_flags;				// set m_commandNum

	U32				m_pad2;
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct WwsJob_Command
{
	union
	{
		ReserveBufferSetCommand		m_reserveBufferSetCommand;
		UseBufferCommand			m_useBufferCommand;
		UnreserveBufferSetsCommand   m_unreserveBufferSetsCommand;
		DepDecCommand				m_depDecCommand;
		RunJobCommand				m_runJobCommand;
		EndCommand					m_endCommand;
		U64	 						m_u64;
		U32							m_u32[2];
		U16							m_u16[4];
		U8 							m_u8 [8];
	};

	//These values are set so that bit 0 maps to m_outputShareable and bit 1 maps to m_shareableWriteIfDiscarded
	enum BufferCacheType
	{
		kNonCached			= 0,
		kReadOnlyCached		= 1,
		kReadWriteCached	= 3,
	};

	//The following functions are helpers for building the command list
	void		ReserveBufferSet( U32 bufferSetNum, U32 numBuffers, U32 firstPageNum, U32 numPagesPerBuffer );
	void		UnreserveBufferSets( U32 bufferSetMask );

	void		UseUninitializedBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress = 0, U32 mainMemSize = 0, BufferCacheType cacheType = WwsJob_Command::kNonCached);
	void		UseInputBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress, U32 mainMemSize, BufferCacheType cacheType );
	void		UseInputDmaListBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress, U32 mainMemSize, BufferCacheType cacheType, U32 numPadQwordsBelowListElements );

	void		RequestDependencyDecrement( U32 eaDependencyCounter );
	void		AddNopCommand( void );
	void		RunJob( U32 logicalBufferSetNum, U32 logicalBufferNum );
	void		AddEndCommand( void );

} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

// now many bytes can be in load commands.  Must be multiple of 16, and <= 16K
#define	MAX_LOAD_COMMANDS_SIZE	0x100

// must be a multiple of 4, and <= 16
#define WWSJOB_MAX_NUM_BUFFER_SETS_PER_JOB 16

// must be 16, 32, 64, or 128
#define WWSJOB_MAX_NUM_BUFFERS_PER_JOB 32

//--------------------------------------------------------------------------------------------------

namespace LsMemoryLimits
{
	enum
	{
		kJobStackSizeInPages	= 8,							// assume 8K used for stack
#if WWS_JOB_USE_C_VERSION
		kJobAreaBasePageNum		= 48,		// job area starts after spurs kernel + job manager
#else
		kJobAreaBasePageNum		= 16,		// job area starts after spurs kernel + job manager
#endif
		kJobAreaEndPageNum		= 256 - kJobStackSizeInPages,	// job area ends at stack
	};
}

//--------------------------------------------------------------------------------------------------
//
//	The following functions are helpers for building command lists.
//	They can probably be optimised.
//
//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::ReserveBufferSet( U32 bufferSetNum, U32 numBuffers, U32 firstPageNum, U32 numPagesPerBuffer )
{
	WWSJOB_ASSERT( firstPageNum >= LsMemoryLimits::kJobAreaBasePageNum );
	WWSJOB_ASSERT( (firstPageNum + numBuffers*numPagesPerBuffer) <= LsMemoryLimits::kJobAreaEndPageNum );

	m_u64 = 0;

	m_reserveBufferSetCommand.m_flags.m_commandNum				= CommandNum::kReserveBufferSet;
	m_reserveBufferSetCommand.m_flags.m_logicalBufferSetNum		= bufferSetNum;

	m_reserveBufferSetCommand.m_bufferSet.m_firstPageNum		= firstPageNum;
	m_reserveBufferSetCommand.m_bufferSet.m_numPagesPerBuffer	= numPagesPerBuffer;
	m_reserveBufferSetCommand.m_bufferSet.m_numBuffers			= numBuffers;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::UnreserveBufferSets( U32 bufferSetMask )
{
	m_u64 = 0;

	m_unreserveBufferSetsCommand.m_flags.m_commandNum			= CommandNum::kUnreserveBufferSets;
	m_unreserveBufferSetsCommand.m_bufferSetMask				= bufferSetMask;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::UseUninitializedBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress, U32 mainMemSize, BufferCacheType cacheType )
{
	WWSJOB_ASSERT( WwsJob_IsAligned( mainMemAddress, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( mainMemSize, 16 ) );
	UseBufferCommand useBufferCommand;

	WWSJOB_ASSERT( cacheType != kReadOnlyCached );		//Given this buffer is uninitialized, it can't be read-only cached

	useBufferCommand.m_mmAddressInQwords						= (mainMemAddress >> 4);
	useBufferCommand.m_flags.m_mmLengthInQwords					= (mainMemSize >> 4);
	useBufferCommand.m_flags.m_continueAfterDiscardWrite		= 0;
	useBufferCommand.m_flags.m_inputRead						= 0;
	useBufferCommand.m_flags.m_inputGather						= 0;
	useBufferCommand.m_flags.m_outputShareable					= ((U32)cacheType) & 1;
	useBufferCommand.m_flags.m_shareableWriteIfDiscarded		= (((U32)cacheType) & 2) >> 1;
	useBufferCommand.m_flags.m_commandNum						= CommandNum::kUseBuffer;
	useBufferCommand.m_flags.m_logicalBufferSetNum				= bufferSetNum;
	useBufferCommand.m_flags.m_logicalBufferNum					= logicalBufferNum;

	//useBufferCommand.m_pad									= 0;
	//useBufferCommand.m_numPadQwordsBelowListElements			= 0;

	m_useBufferCommand = useBufferCommand;

}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::UseInputBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress, U32 mainMemSize, BufferCacheType cacheType )
{
	WWSJOB_ASSERT( WwsJob_IsAligned( mainMemAddress, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( mainMemSize, 16 ) );

	UseBufferCommand useBufferCommand;

	useBufferCommand.m_mmAddressInQwords						= (mainMemAddress >> 4);
	useBufferCommand.m_flags.m_mmLengthInQwords					= (mainMemSize >> 4);
	useBufferCommand.m_flags.m_continueAfterDiscardWrite		= 0;
	useBufferCommand.m_flags.m_inputRead						= 1;
	useBufferCommand.m_flags.m_inputGather						= 0;
	useBufferCommand.m_flags.m_outputShareable					= ((U32)cacheType) & 1;
	useBufferCommand.m_flags.m_shareableWriteIfDiscarded		= (((U32)cacheType) & 2) >> 1;
	useBufferCommand.m_flags.m_commandNum						= CommandNum::kUseBuffer;
	useBufferCommand.m_flags.m_logicalBufferSetNum				= bufferSetNum;
	useBufferCommand.m_flags.m_logicalBufferNum					= logicalBufferNum;

	//useBufferCommand.m_pad									= 0;
	//useBufferCommand.m_numPadQwordsBelowListElements			= 0;

	m_useBufferCommand = useBufferCommand;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::UseInputDmaListBuffer( U32 bufferSetNum, U32 logicalBufferNum, U32 mainMemAddress, U32 listSize, BufferCacheType cacheType, U32 numPadQwordsBelowListElements )
{
	//Note that although the hardware dma list elements are 8 bytes big,
	//the data embeded into this command (eg. m_mmLengthInQwords) is compressed
	//down to a multiple of 16 bytes.
	//You can add a null dmalist element {0,0} to pad out as necessary so
	//that your list becomes a qword multiple in size
	WWSJOB_ASSERT( WwsJob_IsAligned( mainMemAddress, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( listSize, 16 ) );
	WWSJOB_ASSERT( listSize <= 16*1024 );	//dmalist shouldn't be over 16K

	WWSJOB_ASSERT( cacheType != kReadWriteCached );		//DmaList buffers can't be kReadWriteCached

	UseBufferCommand useBufferCommand;

	useBufferCommand.m_mmAddressInQwords						= (mainMemAddress >> 4);
	useBufferCommand.m_flags.m_mmLengthInQwords					= (listSize >> 4);
	useBufferCommand.m_flags.m_continueAfterDiscardWrite		= 0;
	useBufferCommand.m_flags.m_inputRead						= 1;
	useBufferCommand.m_flags.m_inputGather						= 1;
	useBufferCommand.m_flags.m_outputShareable					= ((U32)cacheType) & 1;
	useBufferCommand.m_flags.m_shareableWriteIfDiscarded		= (((U32)cacheType) & 2) >> 1;
	useBufferCommand.m_flags.m_commandNum						= CommandNum::kUseBuffer;
	useBufferCommand.m_flags.m_logicalBufferSetNum				= bufferSetNum;
	useBufferCommand.m_flags.m_logicalBufferNum					= logicalBufferNum;

	//useBufferCommand.m_pad									= 0;
	useBufferCommand.m_numPadQwordsBelowListElements			= numPadQwordsBelowListElements;

	m_useBufferCommand = useBufferCommand;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::RequestDependencyDecrement( U32 eaDependencyCounter )
{
	m_u64 = 0;

	m_depDecCommand.m_mmDependencyAddress						= (U32) eaDependencyCounter;
	m_depDecCommand.m_flags.m_commandNum						= CommandNum::kRequestDependencyDecrement;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::RunJob( U32 logicalBufferSetNum, U32 logicalBufferNum )
{
	m_u64 = 0;

	m_runJobCommand.m_flags.m_commandNum						= CommandNum::kRunJob;
	m_runJobCommand.m_flags.m_logicalBufferSetNum				= logicalBufferSetNum;
	m_runJobCommand.m_flags.m_logicalBufferNum					= logicalBufferNum;

	//Parameters are added after this command.  They must start 16 byte aligned.
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::AddEndCommand( void )
{
	m_u64 = 0;

	m_endCommand.m_flags.m_commandNum							= CommandNum::kEndCommand;
}

//--------------------------------------------------------------------------------------------------

inline void WwsJob_Command::AddNopCommand( void )
{
	m_u64 = 0;
	//m_endCommand.m_flags.m_commandNum							= CommandNum::kNop;
}

//--------------------------------------------------------------------------------------------------

inline U32 NumPagesToNumBytes( U32 numPages )
{
	return (numPages * 1024);
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_DEFINITION_H */
