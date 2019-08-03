//--------------------------------------------------------------------------------------------------
/**
	@file		SkCommand.h

	@brief		This file is available to both the PU and SPU and contains all structures that need
				to be shared across the two

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_COMMAND_H
#define SK_COMMAND_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  EXTERNAL DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

enum
{
	kSpuUserBaseAddr		= 0,
	kSpuUserTopAddr			= 240*1024,		//This should move up to 248K
	kSpuUserAvailableMem	= kSpuUserTopAddr - kSpuUserBaseAddr,

	kKernelSetupData		= 0x3FFE0,		//These special addresses are used for communicating
	kSpuPrintfLsAddr		= 0x3FFF0,		// data between the PU and SPU
};

enum
{
	kMaxNumParams = 16,
};

enum
{
	//The first few slots in the SkSharedAreaInternal structure are used for data other than dependencies
	kNumNonDependencySlots = 3,
};

enum
{
	kAllocationDirectionUnassigned		= -1,
	kAllocationDirectionBottomUp		= 0,
	kAllocationDirectionTopDown			= 1,
};

//The gKernelCommandStrings array below and the kValidInstructionStages
// array in SkTaskList.cpp must be kept in sync with this enum
enum
{
	kUnused = 0,				//Command ID zero is never used

	kDefineMemoryOccupancy,

	kDmaToLsFromEffectiveAddress,
	kDmaToEffectiveAddressFromLs,
	kDmaToLsFromEffectiveAddressCacheable,

	kGpuKick,
	kRunCode,
	kSyncDma,

	kParam64,
	kParamPseudoAddr,
	kSetParamsQwords,			//Presently untested

	kMemsetLs,					//Presently untested
	kMemcpyLs,					//Presently untested

	kStallForDependency,
	kDecrementDependendency,

	kSetIdForThisSpu,
	kFindOtherSpu,
	kFindOtherSpusAsParams,		//Presently untested
	kDmaToLsFromOtherSpuLs,
	kDmaFromLsToOtherSpuLs,
	kSendSignalToOtherSpu,
	kStallOnSignalSendToOtherSpu,
	kStallOnNumSignals,

	kPrintString,
	kCallInterruptCallback,

	kInstallSentinel,
	kVerifySentinel,

	kNop,
	kFinish,
};

const char* const gKernelCommandStrings[] =
{
	"kUnused",

	"kDefineMemoryOccupancy",

	"kDmaToLsFromEffectiveAddress",
	"kDmaToEffectiveAddressFromLs",
	"kDmaToLsFromEffectiveAddressCacheable",

	"kGpuKick",
	"kRunCode",
	"kSyncDma",

	"kParam64",
	"kParamPseudoAddr",
	"kSetParamsQwords",

	"kMemsetLs",
	"kMemcpyLs",

	"kStallForDependency",
	"kDecrementDependendency",

	"kSetIdForThisSpu",
	"kFindOtherSpu",
	"kFindOtherSpusAsParams",
	"kDmaToLsFromOtherSpuLs",
	"kDmaFromLsToOtherSpuLs",
	"kSendSignalToOtherSpu",
	"kStallOnSignalSendToOtherSpu",
	"kStallOnNumSignals",

	"kPrintString",
	"kCallInterruptCallback",

	"kInstallSentinel",
	"kVerifySentinel",

	"kNop",
	"kFinish",
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDependency

	@brief			Contains information to identify the storage location for a single Dependency
**/
//--------------------------------------------------------------------------------------------------

struct SkDependency
{
	u8		m_cacheLineNum;
	u8		m_dependencySlotNum;
	u8		m_lowerBitOffset;
	u8		m_numBits;

	bool IsValidDependency( void ) const
	{
		bool valid = true;
		if ( (m_cacheLineNum == 0) && (m_dependencySlotNum < kNumNonDependencySlots) )
		{
			//The first three qwords in the first cacheline are special
			valid = false;
		}
		if ( m_dependencySlotNum >= 8 )
			valid = false;
		if ( m_lowerBitOffset >= 128 )
			valid = false;
		if ( m_numBits == 0 )
			valid = false;
		if ( m_numBits > 32 )
			valid = false;
		return valid;
	}
} __attribute__((aligned(4)));

//If a SkDependency is embedded in a qword (ie. SkTaskSetupObject, SkStallForDependencyCommand, SkDecrementDependencyCommand)
// it must locate within the qword in this format
struct SkDependencyQword
{
	u64				pad1;
	SkDependency	m_depData;
	u32				pad2;
} __attribute__((aligned(16)));

//--------------------------------------------------------------------------------------------------
/**
	@struct			PseudoAddrHalfs

	@brief			Contains information for mapping from a pseudo-addressed buffer to a real address
**/
//--------------------------------------------------------------------------------------------------

struct PseudoAddrHalfs
{
	s16		m_spuBaseOffset;
	s16		m_spuTopOffset;

	void	SetPseudoAddr( u32 spuBufferAddr, u32 spuBufferSize, u32 offset );
} __attribute__((aligned(4)));

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a pseudoAddr to point to a buffer

	@param			spuBufferAddr	- The base pseudo-address of the buffer

	@param			spuBufferSize	- The size of the buffer

	@param			offset			- The offset into the buffer of this sub-buffer
									- This will usually be zero

	@note			The parameters must all be qword multiples

	@note			The internal storage does some pre-calculation in order to reduce storage.
					Hence all information from the input parameters cannot necessarily be retrieved.
**/
//--------------------------------------------------------------------------------------------------

inline void PseudoAddrHalfs::SetPseudoAddr( u32 spuBufferAddr, u32 spuBufferSize, u32 offset )
{
	FW_ASSERT( FwIsAligned( spuBufferAddr, 16 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 16 ) );
	FW_ASSERT( FwIsAligned( offset, 16 ) );

//We take three pieces of input data, but don't necessarily have space to store this in the instruction
	//Fortunately, we only need two results from that input data, so can just pre-calculate those results
	//And given the results are qword aligned on this 19-bit address, we can fit it in a s16.
	m_spuBaseOffset	= ( ( spuBufferAddr + offset ) >> 4 );
	m_spuTopOffset	= - ( ( spuBufferAddr + spuBufferSize - offset ) >> 4 );
}

//--------------------------------------------------------------------------------------------------
/**
	@struct			PseudoAddrWords

	@brief			Contains information for mapping from a pseudo-addressed buffer to a real address
**/
//--------------------------------------------------------------------------------------------------

struct PseudoAddrWords
{
	s32		m_spuBaseOffset;
	s32		m_spuTopOffset;

	void	SetPseudoAddr( u32 spuBufferAddr, u32 spuBufferSize, u32 offset );
} __attribute__((aligned(8)));

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a pseudoAddr to point to a buffer

	@param			spuBufferAddr	- The base pseudo-address of the buffer

	@param			spuBufferSize	- The size of the buffer

	@param			offset			- The offset into the buffer of this sub-buffer
									- This will usually be zero

	@note			The parameters must all be qword multiples

	@note			The internal storage does some pre-calculation in order to reduce storage.
					Hence all information from the input parameters cannot necessarily be retrieved.
**/
//--------------------------------------------------------------------------------------------------

inline void PseudoAddrWords::SetPseudoAddr( u32 spuBufferAddr, u32 spuBufferSize, u32 offset )
{
	FW_ASSERT( FwIsAligned( spuBufferAddr, 16 ) );
	FW_ASSERT( FwIsAligned( spuBufferSize, 16 ) );
	FW_ASSERT( FwIsAligned( offset, 16 ) );

	//We take three pieces of input data, but don't necessarily have space to store this in the instruction
	//Fortunately, we only need two results from that input data, so can just pre-calculate those results
	m_spuBaseOffset	= ( spuBufferAddr + offset );
	m_spuTopOffset	= - ( spuBufferAddr + spuBufferSize - offset );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
	@struct			SkMemoryOccupancyCommand

	@brief			This struct shows the 128-bit layout of the SkMemoryOccupancyCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkMemoryOccupancyCommand
{
	u32		m_currentSlotBaseAddr;
	u32		m_currentSlotTopAddr;
	u32		pad2;
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDmaInCommand

	@brief			This struct shows the 128-bit layout of the SkDmaInCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------


struct SkDmaInCommand
{
	u64		m_effectiveAddr;
	PseudoAddrHalfs	m_pseudoAddr;
	u16		m_dmaSize;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDmaOutCommand

	@brief			This struct shows the 128-bit layout of the SkDmaOutCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkDmaOutCommand
{
	u64		m_effectiveAddr;
	PseudoAddrHalfs	m_pseudoAddr;
	u16		m_dmaSize;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkRunCodeCommand

	@brief			This struct shows the 128-bit layout of the SkRunCodeCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkRunCodeCommand
{
	u32		m_spuElfBaseAddr;
	u32		m_spuElfLength;
	u32		m_spuEntryPointOffset;
	u16		m_breakpoint;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkParamCommand64

	@brief			This struct shows the 128-bit layout of the SkParamCommand64

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkParamCommand64
{
	u64		m_paramValue64;
	u32		m_paramId;
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkParamPseudoAddrCommand

	@brief			This struct shows the 128-bit layout of the SkParamPseudoAddrCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkParamPseudoAddrCommand
{
	PseudoAddrWords m_pseudoAddr;
	u32		m_paramId;
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkSetParamsQwordsCommand

	@brief			This struct shows the 128-bit layout of the SkSetParamsQwordsCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkSetParamsQwordsCommand
{
	u32		m_paramBaseIndex;
	u32		m_numParams;
	u32		pad;
	u16		pad2;
	u16		m_cmdId;
};


//--------------------------------------------------------------------------------------------------
/**
	@struct			SkMemsetLsCommand

	@brief			This struct shows the 128-bit layout of the SkMemsetLsCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkMemsetLsCommand
{
//Need to sort this one to work with sub-buffers
	u32		m_bufferPseudoLocation;
	u32		m_bufferSize;
	u8		m_val;
	u8		pad2[3];
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkMemcpyLsCommand

	@brief			This struct shows the 128-bit layout of the SkMemcpyLsCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkMemcpyLsCommand
{
//Need to sort this one to work with sub-buffers
	u32		m_destBufferPseudoLocation;
	u32		m_srcBufferPseudoLocation;
	u32		m_bufferSize;
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkStallForDependencyCommand

	@brief			This struct shows the 128-bit layout of the SkStallForDependencyCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkStallForDependencyCommand
{
	u32		pad[2];
	SkDependency	m_depData;	//Note: Must be same location in qword as in SkDependencyQword
	u16		pad1;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDecrementDependencyCommand

	@brief			This struct shows the 128-bit layout of the SkDecrementDependencyCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkDecrementDependencyCommand
{
	u32		pad[2];
	SkDependency	m_depData;	//Note: Must be same location in qword as in SkDependencyQword
	u16		pad1;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkSetIdForSpuCommand

	@brief			This struct shows the 128-bit layout of the SkSetIdForSpuCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkSetIdForSpuCommand
{
	u16		m_uniqueId;
	u16		pad3;
	u32		pad2[2];
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkFindOtherSpuCommand

	@brief			This struct shows the 128-bit layout of the SkFindOtherSpuCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkFindOtherSpuCommand
{
	u16		m_uniqueId;
	u16		pad3;
	u32		pad2[2];
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkFindOtherSpusAsParamsCommand

	@brief			This struct shows the 128-bit layout of the SkFindOtherSpusAsParamsCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkFindOtherSpusAsParamsCommand
{
	u16		m_uniqueId;
	u16		m_paramBaseIndex;
	u32		pad2[2];
	u16		m_numSpus;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDmaInFromOtherSpuCommand

	@brief			This struct shows the 128-bit layout of the SkDmaInFromOtherSpuCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkDmaInFromOtherSpuCommand
{
	PseudoAddrHalfs	m_thisSpuPseudoAddr;
	PseudoAddrHalfs	m_otherSpuPseudoAddr;
	u16		m_dmaSize;
	u16		m_pad1;
	u16		m_pad2;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkDmaOutToOtherSpuCommand

	@brief			This struct shows the 128-bit layout of the SkDmaOutToOtherSpuCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkDmaOutToOtherSpuCommand
{
	PseudoAddrHalfs	m_thisSpuPseudoAddr;
	PseudoAddrHalfs	m_otherSpuPseudoAddr;
	u16		m_dmaSize;
	u16		m_pad1;
	u16		m_pad2;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkStallOnNumSignalsCommand

	@brief			This struct shows the 128-bit layout of the SkStallOnNumSignalsCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkStallOnNumSignalsCommand
{
	u32		m_numSignals;
	u32		pad2[2];
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkPrintStringCommand

	@brief			This struct shows the 128-bit layout of the SkPrintStringCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkPrintStringCommand
{
	char	m_str[14];
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkUserCallbackCommand

	@brief			This struct shows the 128-bit layout of the SkUserCallbackCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkUserCallbackCommand
{
	u64		m_pUserCallbackFunction;
	u32		m_userVal;
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkInstallSentinelCommand

	@brief			This struct shows the 128-bit layout of the SkInstallSentinelCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkInstallSentinelCommand
{
	u32		m_pseudoAddr;
	u32		m_id;
	u16		pad[3];
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkVerifySentinelCommand

	@brief			This struct shows the 128-bit layout of the SkVerifySentinelCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkVerifySentinelCommand
{
	u32		m_pseudoAddr;
	u32		m_id;
	u16		pad[3];
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkSimpleCommand

	@brief			This struct shows the 128-bit layout of the SkSimpleCommand

	@note			m_cmdId is always in the last u16 of this u128
**/
//--------------------------------------------------------------------------------------------------

struct SkSimpleCommand
{
	u32		pad2[3];
	u16		pad;
	u16		m_cmdId;
};

//--------------------------------------------------------------------------------------------------
/**
	@union			SkKernelCommand

	@brief			This is a union of all the 129-bit Kernel Commands

	@note			m_cmdId is always in the last u16 of this u128 so can be read consistently
					before knowing what type the current command is					
**/
//--------------------------------------------------------------------------------------------------

union SkKernelCommand
{
	SkSimpleCommand					m_simple;

	SkMemoryOccupancyCommand		m_memOccupancy;
	SkDmaInCommand					m_dmaIn;
	SkDmaOutCommand					m_dmaOut;
	SkRunCodeCommand				m_runCode;
	SkParamCommand64				m_param64;
	SkParamPseudoAddrCommand		m_paramPseudoAddr;
	SkSetParamsQwordsCommand		m_setParamsQwords;
	SkMemsetLsCommand				m_memsetLs;
	SkMemcpyLsCommand				m_memcpyLs;
	SkStallForDependencyCommand		m_stallForDependency;
	SkDecrementDependencyCommand	m_decrementDependency;
	SkSetIdForSpuCommand			m_setIdForSpu;
	SkFindOtherSpuCommand			m_findOtherSpu;
	SkFindOtherSpusAsParamsCommand	m_findOtherSpusAsParams;
	SkDmaInFromOtherSpuCommand		m_dmaInFromOtherSpu;
	SkDmaOutToOtherSpuCommand		m_dmaOutToOtherSpu;
	SkStallOnNumSignalsCommand		m_stallOnNumSignals;
	SkPrintStringCommand			m_printString;
	SkUserCallbackCommand			m_userCallback;
	SkInstallSentinelCommand		m_installSentinelCommand;
	SkVerifySentinelCommand			m_verifySentinelCommand;


	SkDependencyQword				m_depQword;	//Note: This qword matches up with the location of m_depData in m_stallForDependency and m_decrementDependency
	u128							m_data;
};

//--------------------------------------------------------------------------------------------------

enum
{
	kTaskSetupDependencyNone		= 0,
	kTaskSetupDependencyStall		= 1,
	kTaskSetupDependencyDecrement	= 2,
};

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkTaskSetupObject

	@brief			The Task List contains an array of Task Setup Objects
					The Task Setup Object points to the Command List and may contain a Task Startup
					Dependency stall or decrement
**/
//--------------------------------------------------------------------------------------------------

struct SkTaskSetupObject
{
	union
	{
		struct
		{
			u64		m_pCommandListPtr;
			SkDependency	m_depData;	//Note: Must be same location in qword as in SkDependencyQword
			u32		m_processDepdency;	// 2 - decrement, 1 - stall on, 0 - nothing
		};

		SkDependencyQword	m_depQword;	//Note: This qword matches up with the location of m_depData in the struct
	};

} __attribute__(( aligned(16) ));

//--------------------------------------------------------------------------------------------------
/**
	@struct			SkSharedAreaInternal

	@brief			The SkSharedArea class contains a pointer to the "Shared Area" of memory.
					The first two qwords of that Shared area have special content as seen in this
					struct.  The remaining memory in the Shared Area is used for Dependencies
**/
//--------------------------------------------------------------------------------------------------

struct SkSharedAreaInternal
{
	//Note that changing the layout of this structure would mean
	// inline asm code in SkSharedArea.cpp needs changing too.
	u64		m_pTaskSetupObjectBasePtr;
	u8		m_allocationDirections;				//one bit per SPU to indicate allocation direction
	u8		m_exitKernelOnTaskListCompletion;	//boolean - can choose to keep waiting for new tasks
	u8		m_pad1[2];
	u16		m_currTaskIndex;
	u16		m_numTasks;

	u64		m_pProrityTaskSetupObjectBasePtr;
	u32		m_pad2;
	u16		m_currPriorityTaskIndex;
	u16		m_numPriorityTasks;

	u16		m_uniqueIds[8];

	u128	m_dependencySlots[5];
} __attribute__(( aligned(128) ));


//--------------------------------------------------------------------------------------------------
/**
	@struct			SkKernelSetupData

	@brief			To setup the kernel, the PU writes a qword of this form into the fixed Local
					Storage address kKernelSetupData
**/
//--------------------------------------------------------------------------------------------------

struct SkKernelSetupData
{
	u64		m_ydramSharedAreaAddr;
	u32		m_thisSpuId;
	u32		m_thisSpuIdBit;		//this is just (1<<m_thisSpuId)
				//Note that m_thisSpuIdBit must be at an address whose bottom nybble is 0xC
				// because it will be sent as a word to the Signal Notification Register
				// whose address ends in 0xC
} __attribute__(( aligned(16) ));

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

#endif // SK_COMMAND_H
