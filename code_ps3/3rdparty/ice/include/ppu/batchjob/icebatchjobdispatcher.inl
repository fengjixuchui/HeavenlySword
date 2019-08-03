/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

/*!
 * \file icebatchjobdispatcher.inl
 * \brief Contains implementation of a batch job dispatcher main loop.
 *
 *	icebatchjobdispatcher.inl expects to be included into a batch job dispatcher
 *	cpp file.  This structure allows the including file to control the
 *	preprocessor defines which control the compilation of this file, including:
 *	BATCHJOB_NAMESPACE - sets the namespace for buffer size constants for this job
 *	BATCHJOB_DISPATCHER_DPRINTS - if not 0, enable debug prints in the batch job
 *		through a SPU LS stack or PPU printf.
 *	BATCHJOB_DISPATCHER_ASSERTS - if not 0, enable debug assertions in the batch job
 *	BATCHJOB_DISPATCHER_AUDITS - if not 0, enable audits in the SPU batch job through
 *		the job manager audit system.  BATCHJOB_NAMESPACE::kAuditSystem will be used
 *		as the system tag for all audits.
 *
 *	icebatchjobdispatcher.inl also expects to find the following constants and
 *	data when it compiles:
 *	namespace BATCHJOB_NAMESPACE {
 *		enum LocalStoreBufferSize {
 *			kInputBufferSize  = ... * 1024,
 *			kWorkBufferSize   = ... * 1024,
 *			kOutputBufferSize = ... * 1024,
 *			kTotalBufferSize  = kInputBufferSize*Ice::BatchJob::kNumInputBuffers + kWorkBufferSize*Ice::BatchJob::kNumWorkBuffers + kOutputBufferSize*Ice::BatchJob::kNumOutputBuffers
 *		};
 *		enum LocalStoreBufferOffset {
 *			kInputBufferOffset   = 0,
 *			kInputBufferOffset0  = kInputBufferOffset,
 *			kInputBufferOffset1  = kInputBufferOffset0 + kInputBufferSize,
 *			kWorkBufferOffset    = kInputBufferOffset1 + kInputBufferSize,
 *			kOutputBufferOffset  = kWorkBufferOffset + kWorkBufferSize,
 *			kOutputBufferOffset0 = kOutputBufferOffset,
 *			kOutputBufferOffset1 = kOutputBufferOffset0 + kOutputBufferSize
 *		};
 *		const U32 kDmaOutputListMaxSize = ... * 8;
 *		const U32 kDmaOutputMaxSize = kOutputBufferSize - kDmaOutputListMaxSize;
 *		enum BatchCommand {
 *			... = Ice::BatchJob::kNumCoreCommands,	// the first additional built-in command
 *			...,
 *			kNumCommands,							// sets the total number of built-in commands
 *		};
 *	}
 *
 *#if BATCHJOB_DISPATCHER_DPRINTS
 * extern "C" DISPATCHER_DPRINT_Command(U16 *pCmdAndArgs, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap, void*const* pCommandFunctions, U8 const* pCommandNumParams);
 * extern "C" DISPATCHER_DPRINT_CommandInputData(U16 *pCmdAndArgs, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap);
 * extern "C" DISPATCHER_DPRINT_CommandOutputData(U16 *pCmdAndArgs, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap);
 *#endif
 * Ice::BatchJob::DispatcherFunction g_CommandFunction[] = {
 *		NULL,					// Ice::BatchJob::kCmdEnd
 *		NULL,					// Ice::BatchJob::kCmdSwap
 *		ReserveOutputBuffer,	// Ice::BatchJob::kCmdReserveOutputBuffer
 *		ExecutePlugIn,			// Ice::BatchJob::kCmdExecutePlugIn
 *		CopyQuadwords,			// Ice::BatchJob::kCmdCopyQuadwords
 *		...						// additional built-in functions for this implementation
 * }
 * U8 g_CommandNumParams[] = {
 *		Ice::BatchJob::kCmdSizeEnd-1,
 *		Ice::BatchJob::kCmdSizeSwap-1,
 *		Ice::BatchJob::kCmdSizeReserveOutputBuffer-1,
 *		Ice::BatchJob::kCmdSizeExecutePlugIn-1,
 *		Ice::BatchJob::kCmdSizeCopyQuadwords-1,
 *		...						// number of arguments for additional built-in functions for this implementation
 * };
 *#if ICE_TARGET_PS3_SPU && BATCHJOB_DISPATCHER_AUDITS
 * For built-in command pCommand[0] and arguments pCommand[1..NumParams], return the number of items processed
 * for auditing purposes.
 *extern "C" U32 DISPATCHER_AUDIT_GetNumItems(U16 const* pCommand);
 *#endif
 */

#if ICE_TARGET_PS3_SPU
# include "spu/icedma.h"
# include "jobapi/jobdefinition.h"
# include "jobapi/jobapi.h"
# include "jobapi/jobspudma.h"
#else
# include "icelocalmemory.h"
#endif
#include "icebatchjobdebug.h"

#if ICE_TARGET_PS3_SPU
typedef U32 BufferTag;
# define kLogicalBuffer0		0
# define kLogicalBuffer1		1
# define BUFFER_TAG_FROM_WWS(tag)	(U32)((tag.m_dmaTagId << 24) | (tag.m_lsAddressInWords << 2))
# define BUFFER_TAG(ptr, dmaTagId)	(U32)(ptr)
# define GET_BUFFER_PTR(tag) 		(U8*)((tag)&0x3FFF0)
# define GET_BUFFER_TAGID(tag)		((tag)>>24)
#else
typedef U32 BufferTag;
U8 const *g_memoryLS = NULL;
# define BUFFER_TAG(ptr, dmaTagId)	(U32)((U8 const*)ptr - g_memoryLS)
# define GET_BUFFER_PTR(tag) 		(U8*)(g_memoryLS + CONV_FROM_PTR((tag)&0x3FFF0))
# define GET_BUFFER_TAGID(tag)		(0)
#endif

//------------------------------------------------------------------------------------------------
// Global data containing the current state of the task/job's input, output, and buffers:

#if ICE_TARGET_PS3_SPU
// g_memoryMap contains a map of the starts of important locations in memory (currentInputBuffer, workBuffer, currentOutputBufferPos, 0)
VU32 	 g_memoryMap;
# if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
// g_reverseMemoryMap and g_reverseMemoryMapEnd contain the start and end of the input, work, and output buffers, to allow reverse lookups in DEBUG_PtrToLoc()
VU32	 g_reverseMemoryMap;
VU32	 g_reverseMemoryMapEnd;
# endif
# if BATCHJOB_DISPATCHER_ASSERTS
VU32	 g_outputDmaListCopy[2];	// copy of first qword of output dma list for output buffer 0, 1
# endif
# if BATCHJOB_DISPATCHER_AUDITS
VU32	 m_audit_data_qw;
# endif

static U32		 g_currentInputIndex;	// the current input/pre buffer (0 or 1).
static U32		 g_currentOutputIndex;	// the current output/post buffer (0 or 1).
#else
// g_memoryMap contains a map of the starts of important locations in memory (currentInputBuffer, workBuffer, currentOutputBufferPos, volatileBuffer)
U8 const *g_memoryMap[4] = { NULL, NULL, NULL, NULL };
U8 const *g_outputBuf = NULL;

static U64		 g_currentInputIndex;	// the current input/pre buffer (0 or 1).
static U64		 g_currentOutputIndex;	// the current output/post buffer (0 or 1).
#endif
static BufferTag g_tagPreBuffer[2];		// tags pointing to our two pre buffers (or 0 if not currently in use).
static BufferTag g_tagWorkBuffer;		// tag pointing to our work buffer.
static BufferTag g_tagPostBuffer[2];	// tags pointing to our two post buffers (or 0 if not currently in use).

static U32		 g_currentOutputSize;	// number of bytes written so far to our current output buffer.
static U32		 g_outputDmaListSize;	// current size of the dma list at the start of the current output buffer.

#if ICE_TARGET_PS3_SPU
static inline void* LocToPtr(U32 loc) { return (void*)( spu_extract( spu_rlqwbyte( g_memoryMap, loc ), 0) + (loc &~ 0xF) ); }
#else
static inline void* LocToPtr(U32 loc) { return (void*)(&g_memoryMap[(loc>>2) & 0x3][loc & ~0xF]); }
#endif

//------------------------------------------------------------------------------------------------
// Helper functions for adjusting the global input/output/buffer state:
#if ICE_TARGET_PS3_SPU
enum {
	kMaskInput = 	0xF000 >> Ice::BatchJob::kLocInput,
	kMaskWork = 	0xF000 >> Ice::BatchJob::kLocWork,
	kMaskOutput = 	0xF000 >> Ice::BatchJob::kLocOutput,
	kMaskVolatile =	0xF000 >> Ice::BatchJob::kLocVolatile,
	kMaskMemory =	kMaskVolatile,	//in reverse lookup, volatiles are assumed to be in input buffer and 0xc is used for a catch all LS address
};

static inline void SetLocalMemory()
{
# if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
	VU32 mask = (VU32)spu_maskb(kMaskVolatile);
	g_reverseMemoryMap = spu_sel(g_reverseMemoryMap, spu_splats((U32)0), mask);
	g_reverseMemoryMapEnd = spu_sel(g_reverseMemoryMapEnd, spu_splats((U32)0x40000), mask);
# endif
}
static inline void SetInputBuffer(void const *pBuffer)
{
	VU32 mask = (VU32)spu_maskb(kMaskInput);
	g_memoryMap = spu_sel(g_memoryMap, spu_splats((U32)pBuffer), mask);
# if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
	g_reverseMemoryMap = spu_sel(g_reverseMemoryMap, spu_splats((U32)pBuffer), mask);
	g_reverseMemoryMapEnd = spu_sel(g_reverseMemoryMapEnd, spu_splats((U32)pBuffer + BATCHJOB_NAMESPACE::kInputBufferSize), mask);
# endif
}
static inline void SetWorkBuffer(void const *pBuffer)
{
	VU32 mask = (VU32)spu_maskb(kMaskWork);
	g_memoryMap = spu_sel(g_memoryMap, spu_splats((U32)pBuffer), mask);
# if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
	g_reverseMemoryMap = spu_sel(g_reverseMemoryMap, spu_splats((U32)pBuffer), mask);
	g_reverseMemoryMapEnd = spu_sel(g_reverseMemoryMapEnd, spu_splats((U32)pBuffer + BATCHJOB_NAMESPACE::kWorkBufferSize), mask);
# endif
}
static inline void SetOutputBuffer(void const *pBuffer)
{
	VU32 mask = (VU32)spu_maskb(kMaskOutput);
	g_memoryMap = spu_sel(g_memoryMap, spu_splats((U32)pBuffer), mask);
# if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
	g_reverseMemoryMap = spu_sel(g_reverseMemoryMap, spu_splats((U32)pBuffer), mask);
	g_reverseMemoryMapEnd = spu_sel(g_reverseMemoryMapEnd, spu_splats((U32)pBuffer + BATCHJOB_NAMESPACE::kOutputBufferSize), mask);
# endif
}
static inline void SetOutputBufferPos(void const *pOutputPos)
{
	VU32 mask = (VU32)spu_maskb(kMaskOutput);
	g_memoryMap = spu_sel(g_memoryMap, spu_splats((U32)pOutputPos), mask);
}
static inline void SetVolatileBuffer(void const *pBuffer)
{
	VU32 mask = (VU32)spu_maskb(kMaskVolatile);
	g_memoryMap = spu_sel(g_memoryMap, spu_splats((U32)pBuffer), mask);
}

#else	//if ICE_TARGET_PS3_SPU ...

enum {
	kBufInput = 	Ice::BatchJob::kLocInput  >> 2,
	kBufWork = 		Ice::BatchJob::kLocWork   >> 2,
	kBufOutput = 	Ice::BatchJob::kLocOutput >> 2,
	kBufVolatile = 	Ice::BatchJob::kLocVolatile >> 2,
};
static const U32 kBufferOffset = 0x1e000;	// include offset so no buffers start at 0 and to make buffer tags similar to SPU
static inline void SetLocalMemory(void const *pMemory) 			{ g_memoryLS = (U8 const*)pMemory - kBufferOffset; }	
static inline void SetInputBuffer(void const *pBuffer) 			{ g_memoryMap[kBufInput] = (U8 const*)pBuffer; }
static inline void SetWorkBuffer(void const *pBuffer)  			{ g_memoryMap[kBufWork] = (U8 const*)pBuffer; }
static inline void SetOutputBuffer(void const *pBuffer)			{ g_memoryMap[kBufOutput] = g_outputBuf = (U8 const*)pBuffer; }
static inline void SetOutputBufferPos(void const *pOutputPos)	{ g_memoryMap[kBufOutput] = (U8 const*)pOutputPos; }
static inline void SetVolatileBuffer(void const *pBuffer)		{ g_memoryMap[kBufVolatile] = (U8 const*)pBuffer; }

#endif	//if ICE_TARGET_PS3_SPU ... else

//------------------------------------------------------------------------------------------------
// Helper functions used by dispatcher for DMA

extern "C" {	// prevent C++ name mangling on SPU

#if ICE_TARGET_PS3_SPU
	inline void WaitOnDma(U32 tagMask)
	{
		U32 status;
		do {
			status = JobDmaWaitTagStatusAny(tagMask);
		} while (status != tagMask);
	}
#else
# define CONV_EA_TO_PTR(ea) (const void *)(U32(ea))

	inline void WaitOnDma(U32 tagMask) { (void)tagMask; }

	void DmaGetMemory(void *dest, const void *source, U32 size)
	{
		ICE_ASSERT( !(size & 0xF) || !(size & (size-1)) );	// size must be a multiple of 16 or a power of 2
		ICE_ASSERT( size <= 0x4000 );						// size must be no greater than max DMA size
		ICE_ASSERT( !(CONV_FROM_PTR(dest) & 0xF) );					// dest must be quadword aligned

		U8 *dst = (U8 *)dest;
		U8 const *src = (U8 *)source;
		U32F i = 0;
		// if (size < 4), zero fill to place source in preferred halfword or byte
		U32F align = (0x3 &~ (size-1));
		for (i = 0; i < align; i ++) {
			*dst++ = 0;
		}
		// copy source
		for ( ; i < align + size; i ++) {
			*dst++ = *src++;
		}
		// zero fill out to 16 bytes
		for ( ; i < 16; i++) {
			*dst++ = 0;
		}
	}

	void DmaPutMemory(void *dest, const void *source, U32 size)
	{
		ICE_ASSERT( !(size & 0xF) || !(size & (size-1)) );	// size must be a multiple of 16 or a power of 2
		ICE_ASSERT( size <= 0x4000 );						// size must be no greater than max DMA size
		ICE_ASSERT( !(CONV_FROM_PTR(source) & 0xF) );					// source must be quadword aligned

		U8 *dst = (U8 *)dest;
		U8 const *src = (U8 *)source;
		U32F i = 0;
		// if size < 4, advance src to point at preferred halfword or byte
		src += (0x3 &~ (size-1));
		// copy src
		for (; i < size; i++) {
			*dst++ = *src++;
		}
	}
#endif

/*
	void DmaGet(U8 *dest, U32 sourceHi, U32 sourceLo, U32 size, U32 dmaTagId)
	{
#if ICE_TARGET_PS3_SPU
		DISPATCHER_DPRINTF("BATCHJOB: spu_mfcdma64(%05x, %08x, %08x, %04x, %d, kDmaGet(%02x))\n", (U32)dest, sourceHi, sourceLo, size, dmaTagId, Ice::kDmaGett);
		JobDmaGet(dest, sourceLo, size, dmaTagId);
#else
		(void)dmaTagId;
		DISPATCHER_DPRINTF("BATCHJOB: DmaGetMemory(%08x_%08x, %08x_%08x, %04x)\n", (U32)(CONV_FROM_PTR(dest)>>32), (U32)CONV_FROM_PTR(dest), sourceHi, sourceLo, size);
		DmaGetMemory(dest, CONV_EA_TO_PTR(sourceLo), size);
#endif
	}
*/

	inline void DmaGetList(U8 *dest, U32 /*dmaListEAhi*/, U32 const *dmaList, U32 dmaListSize, U32 dmaTagId)
	{
#if ICE_TARGET_PS3_SPU
		DISPATCHER_DPRINTF("BATCHJOB: spu_mfcdma64(%05x, %08x, %05x, %04x, %d, kDmaGetl(%02x))\n", (U32)dest, 0 /*dmaListEAhi*/, (U32)dmaList, dmaListSize, dmaTagId, Ice::kDmaGetl);
		JobDmaListGet(dest, (JobDmaListElement const*)dmaList, dmaListSize, dmaTagId);
#else
		(void)dmaTagId;
		U8 *p = dest;
		dmaListSize >>= 3;
		for (U32F iDmaElem = 0; iDmaElem < dmaListSize; iDmaElem ++) {
			U32 sizeLS = ((dmaList[iDmaElem*2 + 0] + 0xF) & ~0xF);
			DISPATCHER_DPRINTF("         GET LIST(%05x <- %08x_%08x %04x)\n", (U32)DEBUG_Ptr(p), 0 /*dmaListEAhi*/, dmaList[iDmaElem*2 + 1], dmaList[iDmaElem*2 + 0]);
			DISPATCHER_DPRINTF("BATCHJOB: DmaGetMemory(%08x_%08x, %08x_%08x, %04x)\n", (U32)0 /*(CONV_FROM_PTR(p)>>32)*/, (U32)CONV_FROM_PTR(p), 0 /*dmaListEAhi*/, dmaList[iDmaElem*2 + 1], dmaList[iDmaElem*2 + 0]);
			DmaGetMemory(p, CONV_EA_TO_PTR(dmaList[iDmaElem*2 + 1]), dmaList[iDmaElem*2 + 0]);
			p += sizeLS;
		}
#endif
	}
/*
	void DmaPut(U8 const *source, U32 destHi, U32 destLo, U32 size, U32 dmaTagId)
	{
#if ICE_TARGET_PS3_SPU
		DISPATCHER_DPRINTF("BATCHJOB: spu_mfcdma64(%05x, %08x, %08x, %04x, %d, kDmaPut(%02x))\n", (U32)source, destHi, destLo, size, dmaTagId, Ice::kDmaPut);
		JobDmaPut(source, destLo, size, dmaTagId);
#else
		(void)dmaTagId;
		DISPATCHER_DPRINTF("BATCHJOB: DmaPutMemory(%08x_%08x, %08x_%08x, %04x)\n", destHi, destLo, (U32)0 / *(CONV_FROM_PTR(source)>>32)* /, (U32)CONV_FROM_PTR(source), size);
		DmaPutMemory(CONV_EA_TO_PTR(destLo), source, size);
#endif
	}
*/

	inline void DmaPutList(U8 const *source, U32 /*dmaListEAhi*/, U32 const *dmaList, U32 dmaListSize, U32 dmaTagId)
	{
#if ICE_TARGET_PS3_SPU
		DISPATCHER_DPRINTF("BATCHJOB: spu_mfcdma64(%05x, %08x, %05x, %04x, %d, kDmaPutl(%02x))\n", (U32)source, 0 /*dmaListEAhi*/, (U32)dmaList, dmaListSize, dmaTagId, Ice::kDmaPutl);
		JobDmaListPut(source, (JobDmaListElement const*)dmaList, dmaListSize, dmaTagId);
#else
		(void)dmaTagId;
		U8 const *p = source;
		dmaListSize >>= 3;
		for (U32F iDmaElem = 0; iDmaElem < dmaListSize; iDmaElem ++) {
			U32 sizeLS = ((dmaList[iDmaElem*2 + 0] + 0xF) & ~0xF);
			DISPATCHER_DPRINTF("         PUT LIST(%05x -> %08x_%08x %04x)\n", (U32)DEBUG_Ptr(p), 0 /*dmaListEAhi*/, dmaList[iDmaElem*2 + 1], dmaList[iDmaElem*2 + 0]);
			DISPATCHER_DPRINTF("BATCHJOB: DmaPutMemory(%08x_%08x, %08x_%08x, %04x)\n", 0 /*dmaListEAhi*/, dmaList[iDmaElem*2 + 1], (U32)0 /*(CONV_FROM_PTR(p)>>32)*/, (U32)CONV_FROM_PTR(p), dmaList[iDmaElem*2 + 0]);
			DmaPutMemory((void *)CONV_EA_TO_PTR(dmaList[iDmaElem*2 + 1]), p, dmaList[iDmaElem*2 + 0]);
			p += sizeLS;
		}
#endif
	}
}	// extern "C"

//------------------------------------------------------------------------------------------------
// Helper functions for command list processing
extern "C" {	// prevent C++ name mangling on SPU
#if ICE_TARGET_PS3_SPU
	inline VU16 UnpackSpuDispatcherFnArgs(U16 const* command)
	{
		VI8 pCommand = (VI8)spu_splats((U32)command);
		VU16 arg_qw0 = (VU16)si_rotqby( si_lqd(pCommand, 0x00), pCommand );
		VU16 arg_qw1 = (VU16)si_rotqby( si_lqd(pCommand, 0x10), pCommand );
		VU16 arg_mask = (VU16)spu_rlmaskqwbyte( (VU16)spu_splats((U32)-1), ((I32)command & 0xF) - 16 );
		VU16 args = spu_sel( arg_qw0, arg_qw1, arg_mask );
		return args;
	}
# define UNPACK_DISPATCHER_ARGS(pArgs) UnpackSpuDispatcherFnArgs(pArgs)
#else
# define UNPACK_DISPATCHER_ARGS(pArgs) (pArgs)

	inline U32 ArgU32(U16 hi, U16 lo)
	{
		return ((U32)hi<<16) | ((U32)lo);
	}
#endif
}	// extern "C"

//------------------------------------------------------------------------------------------------
// Locally implemented command functions
extern "C" {	// prevent C++ name mangling on SPU

	void ReserveOutputBuffer(Ice::BatchJob::DispatcherFunctionArgs params, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap)
	{
#if ICE_TARGET_PS3_SPU
		(void)memoryMap;
//		DISPATCHER_DPRINTF("      ReserveOutputBuffer(%04x, %04x_%04x)\n", spu_extract(params, 0), spu_extract(params, 1), spu_extract(params, 2));
		U32 size = (U32)spu_extract(params, 0);
		U32 destLo = spu_extract((VU32)spu_rlqwbyte(params, 2), 0);
#else
		(void)memoryMap;
		DISPATCHER_DPRINTF("      ReserveOutputBuffer(%04x, %04x_%04x)\n", params[0], params[1], params[2]);
		U32 size = (U32)params[0];
		U32 destLo = ArgU32(params[1], params[2]);
#endif

		bool bChangedOutputBuffer = false;
		if (g_tagPostBuffer[g_currentOutputIndex] != 0) {
			if ((g_currentOutputSize + size > BATCHJOB_NAMESPACE::kDmaOutputMaxSize) || (g_outputDmaListSize > BATCHJOB_NAMESPACE::kDmaOutputListMaxSize-8)) {
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputDmaStart);			// before DMAing output buffer
				// If we can't put this output into the current output buffer, flush the current output buffer
				U8 const *pOutputDmaList = GET_BUFFER_PTR(g_tagPostBuffer[g_currentOutputIndex]);
				DISPATCHER_DPRINTF("BATCHJOB: DmaPutList(@%04x %04x -> @%04x %04x)\n", DEBUG_PtrToLoc(pOutputDmaList + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize), (U32)g_currentOutputSize, DEBUG_PtrToLoc(pOutputDmaList), g_outputDmaListSize);
#if ICE_TARGET_PS3_SPU && BATCHJOB_DISPATCHER_ASSERTS
				DISPATCHER_ASSERT(*((VU32*)pOutputDmaList) == g_outputDmaListCopy[g_currentOutputIndex]);
#endif
				U32 dmaTagId = GET_BUFFER_TAGID(g_tagPostBuffer[g_currentOutputIndex]);
				DmaPutList(pOutputDmaList + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize, 0, (U32 const*)pOutputDmaList, g_outputDmaListSize, dmaTagId);

				// If the output buffer we want to write to hasn't finished its DMA download, we have to wait on it before we can allow
				// the following commands to write into it.
				dmaTagId = GET_BUFFER_TAGID(g_tagPostBuffer[1-g_currentOutputIndex]);
				U32 dmaTagGroup = 1<<dmaTagId;
				DISPATCHER_DPRINTF("BATCHJOB: DMA WAIT(OUTPUT_%d %d (%08x))\n", (U32)(1 - g_currentOutputIndex), dmaTagId, dmaTagGroup);
				WaitOnDma( dmaTagGroup );

				// Swap output buffers
				g_currentOutputIndex = 1 - g_currentOutputIndex;
				bChangedOutputBuffer = true;
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputDmaEnd, (U16)g_currentOutputSize);	// after DMAing output buffer
			}
		}
		if (g_tagPostBuffer[g_currentOutputIndex] == 0) {
#if ICE_TARGET_PS3_SPU
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputUseBufferStart, (U16)g_currentOutputIndex);
			WwsJob_Command commandList[4] WWSJOB_ALIGNED(16);
			commandList[0].UseUninitializedBuffer( Ice::BatchJob::kOutputBufferSet, g_currentOutputIndex, 0, 0, WwsJob_Command::kNonCached );
			commandList[1].UnreserveBufferSets( 1<<Ice::BatchJob::kInputBufferSet );	// let the job manager know that we are done using output buffers...
			commandList[g_currentOutputIndex + 1].AddEndCommand();	// ...if this is the second output buffer (otherwise overwrite the UnreserveBufferSets with EndCommands)
			WwsJob_JobApiExecuteCommands(commandList);
			WwsJob_BufferTag tag = WwsJob_JobApiGetBufferTag(Ice::BatchJob::kOutputBufferSet, g_currentOutputIndex, WwsJob_kAllocDmaTag);
			g_tagPostBuffer[g_currentOutputIndex] = BUFFER_TAG_FROM_WWS(tag);
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputUseBufferEnd, (U16)g_currentOutputIndex);
#else
			g_tagPostBuffer[g_currentOutputIndex] = BUFFER_TAG(g_memoryLS + kBufferOffset + BATCHJOB_NAMESPACE::kOutputBufferOffset0 + g_currentOutputIndex*BATCHJOB_NAMESPACE::kOutputBufferSize, g_currentOutputIndex );
#endif
			DISPATCHER_DPRINTF("BATCHJOB: UseBuffer(OUTPUT_%d %d %05x)\n", (U32)g_currentOutputIndex, GET_BUFFER_TAGID(g_tagPostBuffer[g_currentOutputIndex]), (U32)GET_BUFFER_PTR(g_tagPostBuffer[g_currentOutputIndex]));
			bChangedOutputBuffer = true;
		}
		U8 *pOutputDmaList = GET_BUFFER_PTR(g_tagPostBuffer[g_currentOutputIndex]);
		if (bChangedOutputBuffer) {
			g_currentOutputSize = 0;
			g_outputDmaListSize = 0;
			SetOutputBuffer(pOutputDmaList);
			DISPATCHER_DPRINTF("BATCHJOB: SetOutputBuffer(OUTPUT_%d %d %05x)\n", (U32)g_currentOutputIndex, GET_BUFFER_TAGID(g_tagPostBuffer[g_currentOutputIndex]), (U32)pOutputDmaList);
		}
		DISPATCHER_ASSERT(size <= BATCHJOB_NAMESPACE::kDmaOutputMaxSize);
		// Store our reserved output location
		SetOutputBufferPos(pOutputDmaList + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize + g_currentOutputSize);
		// increment our current output buffer size
		g_currentOutputSize += size;
		// add our output command to the dma output list, breaking it into multiple entries if size > 0x4000 (the max size for a single DMA entry).
		U32 *pOutputDmaListPos = (U32*)(pOutputDmaList + g_outputDmaListSize);
		while (1) {
			if (size <= 0x4000) {
				*pOutputDmaListPos++ = size;
				*pOutputDmaListPos++ = destLo;
				break;
			}
			*pOutputDmaListPos++ = 0x4000;
			*pOutputDmaListPos++ = destLo;
			size -= 0x4000;
			destLo += 0x4000;
		}
		g_outputDmaListSize = ((U8*)pOutputDmaListPos - pOutputDmaList);
#if ICE_TARGET_PS3_SPU && BATCHJOB_DISPATCHER_ASSERTS
		g_outputDmaListCopy[g_currentOutputIndex] = *(VU32*)pOutputDmaList;
#endif
	}

	void EndCommands()
	{
		DISPATCHER_DPRINTF("      End()\n");
/*		DISPATCHER_DPRINTF("BATCHJOB: FreeBuffer(INPUT_%d %d %05x)\n", (U32)(1-g_currentInputIndex), (g_tagPreBuffer[1-g_currentInputIndex]>>24), (U32)(g_tagPreBuffer[1-g_currentInputIndex] & 0x3FFF0));
		DISPATCHER_DPRINTF("BATCHJOB: FreeBuffer(WORK %d %05x)\n", (g_tagWorkBuffer>>24), (U32)(g_tagWorkBuffer & 0x3FFF0));
#if ICE_TARGET_PS3_SPU
		// Don't need any buffers except our last output PostBuffer while we DMA out the last list
		// Note that since we've already swapped g_currentInputIndex, we're actually currently processing g_tagPreBuffer[1-g_currentInputIndex]
		// and we should have already freed the other buffer, g_tagPreBuffer[g_currentInputIndex]...
		const U8 g_bufferList[4] = { Ice::BatchJob::kInputBufferSet, (1-g_currentInputIndex), Ice::BatchJob::kWorkBufferSet, 0 };
		WwsJob_JobApiFreeLogicalBuffers( g_bufferList, 2 );
#endif
		g_tagPreBuffer[1-g_currentInputIndex] = 0;
		g_tagWorkBuffer = 0;
*/

		// Start our last DmaOutput command if we have any data buffered.
		if (g_currentOutputSize > 0) {
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputDmaStart);			// before DMAing last output buffer
/*			if (g_tagPostBuffer[1-g_currentOutputIndex] != 0) {
				DISPATCHER_DPRINTF("BATCHJOB: FreeBuffer(OUTPUT_%d %d %05x)\n", (U32)(1-g_currentOutputIndex), GET_BUFFER_TAGID(g_tagPostBuffer[1-g_currentOutputIndex]), (U32)GET_BUFFER_PTR(g_tagPostBuffer[1-g_currentOutputIndex]));
#if ICE_TARGET_PS3_SPU
				WwsJob_JobApiFreeLogicalBuffer( Ice::BatchJob::kOutputBufferSet, (1-g_currentInputIndex) );
#endif
				g_tagPostBuffer[1-g_currentOutputIndex] = 0;
			}
*/
			U8 const* pOutputDmaList = GET_BUFFER_PTR(g_tagPostBuffer[g_currentOutputIndex]);
			U32 dmaTagId = GET_BUFFER_TAGID(g_tagPostBuffer[g_currentOutputIndex]);
			DISPATCHER_DPRINTF("BATCHJOB: DmaPutList(@%04x %04x -> %08x_@%04x %04x)\n", DEBUG_PtrToLoc(pOutputDmaList + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize), (U32)g_currentOutputSize, 0, DEBUG_PtrToLoc(pOutputDmaList), (U32)g_outputDmaListSize);
#if ICE_TARGET_PS3_SPU && BATCHJOB_DISPATCHER_ASSERTS
			DISPATCHER_ASSERT(*((VU32*)pOutputDmaList) == g_outputDmaListCopy[g_currentOutputIndex]);
#endif
			DmaPutList(pOutputDmaList + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize, 0, (U32 const*)pOutputDmaList, g_outputDmaListSize, dmaTagId);

/*			DISPATCHER_DPRINTF("BATCHJOB: FreeBuffer(OUTPUT_%d %d %05x)\n", (U32)g_currentOutputIndex, GET_BUFFER_TAGID(g_tagPostBuffer[g_currentOutputIndex]), (U32)GET_BUFFER_PTR(g_tagPostBuffer[g_currentOutputIndex]));
#if ICE_TARGET_PS3_SPU
			WwsJob_JobApiFreeLogicalBuffer( Ice::BatchJob::kOutputBufferSet, g_currentOutputIndex );
#endif
*/
			// task/job manager will handle waiting on our final DMA outgoing traffic
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditOutputDmaEnd, (U16)g_currentOutputSize);	// before DMAing last output buffer
		}
		DISPATCHER_AUDIT(Ice::BatchJob::kAuditEnd);
	}
}	// extern "C"

//================================================================================================

#define DOUBLE_BUFFERED 1

#if ICE_TARGET_PS3_SPU
extern "C" void exit() { si_stop(0); }

extern "C" {
	void JobMain();
	void _end();
}
void JobMain()
#else
extern void BATCHJOB_NAMESPACE::PpuJobMain(U32 const* pInitialDmaList, U32 initialDmaListSize)
#endif
{
	DEBUG_SetPtrToLocFn(DEBUG_PtrToLoc_Batched);
	// write 8 nulls to time null intervals over a full 128-byte block
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditNull);
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditStart);

#if ICE_TARGET_PS3_SPU
	{
//		DISPATCHER_PRINTF("BATCHJOB: jobid %08x begin\n", WwsJob_JobApiGetJobId());

		// The previous job, on being converted from a run job to a store job, unreserves all
		// buffer sets, and frees all buffers that do not have outgoing DMA - in this case, all
		// input and work buffers.
		// Therefore, we can safely reserve and use our work buffer and second input buffer here,
		// and unreserve both as we've finished using them.  We can also safely reserve our
		// output buffer set.  Any output buffers may still be in use by the store job,
		// so we must delay using the output buffers until we need them.
		// Although we have then reserved all of our buffer sets, and could call LoadNextJob,
		// we know the next job (the load job) must wait for this job to free an input buffer
		// before it can finish executing its load commands.  Therefore, we can avoid the
		// overhead of retrying the execution of the load commands by delaying our call to
		// LoadNextJob until we free an input buffer.
		const U32 kAnimBufferLSA = (LsMemoryLimits::kJobAreaEndPageNum << 10) - BATCHJOB_NAMESPACE::kTotalBufferSize;
		WwsJob_Command commandList[6] WWSJOB_ALIGNED(16);
		commandList[0].ReserveBufferSet( Ice::BatchJob::kWorkBufferSet, 1, (kAnimBufferLSA + BATCHJOB_NAMESPACE::kWorkBufferOffset)>>10, BATCHJOB_NAMESPACE::kWorkBufferSize>>10 );	// could move this to load commands
		commandList[1].ReserveBufferSet( Ice::BatchJob::kOutputBufferSet, 2, (kAnimBufferLSA + BATCHJOB_NAMESPACE::kOutputBufferOffset0)>>10, BATCHJOB_NAMESPACE::kOutputBufferSize>>10 );
		commandList[2].UseUninitializedBuffer( Ice::BatchJob::kWorkBufferSet, kLogicalBuffer0, 0, 0, WwsJob_Command::kNonCached );
		commandList[3].UseUninitializedBuffer( Ice::BatchJob::kInputBufferSet, kLogicalBuffer1, 0, 0, WwsJob_Command::kNonCached );
		commandList[4].UnreserveBufferSets( (1 << Ice::BatchJob::kWorkBufferSet) | (1 << Ice::BatchJob::kInputBufferSet) );	// let the job manager know that we are done using input and work buffers
		commandList[5].AddEndCommand();
		WwsJob_JobApiExecuteCommands(commandList);
		DISPATCHER_AUDIT(Ice::BatchJob::kAuditInitStart);
		DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferStart, (U16)0);
		WwsJob_BufferTag tag = WwsJob_JobApiGetBufferTag(Ice::BatchJob::kInputBufferSet, kLogicalBuffer0, WwsJob_kAllocDmaTag);
		DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferEnd, (U16)0);
		// Store the important parts of the WwsJob_BufferTag in our old BufferTag format:
		g_tagPreBuffer[0] = BUFFER_TAG_FROM_WWS(tag);
		g_tagPreBuffer[1] = 0;
		tag = WwsJob_JobApiGetBufferTag(Ice::BatchJob::kWorkBufferSet, kLogicalBuffer0, WwsJob_kDontAllocDmaTag);
		g_tagWorkBuffer = BUFFER_TAG_FROM_WWS(tag);
	}
	SetLocalMemory();
#else
	if (initialDmaListSize == 0) {
		DISPATCHER_DPRINTF("BATCHJOB: ignoring task with no data\n");
		return;
	}

	Ice::BatchJob::LocalMemory localMemory;
	U8 *pBuffers = localMemory.GetLocalMemory();
	SetLocalMemory(pBuffers);

	// Emulate the BufferTags used by the job manager.
	g_tagPreBuffer[0] = BUFFER_TAG(g_memoryLS + kBufferOffset + BATCHJOB_NAMESPACE::kInputBufferOffset0, 0);
	g_tagPreBuffer[1] = 0;
	g_tagWorkBuffer = BUFFER_TAG(g_memoryLS + kBufferOffset + BATCHJOB_NAMESPACE::kWorkBufferOffset, 0);
	{
		// Pre SPU setup (mimicking taskman):
		//    We receive the address of the first DMA list, and its size at the start of localMemory.
		DISPATCHER_DPRINTF("BATCHJOB: DmaGetList(@%04x <- %08x_@(%08x_%08x), %04x)\n", Ice::BatchJob::kLocInput, (U32)0, (U32)0 /*(CONV_FROM_PTR(pInitialDmaList)>>32)*/, (U32)CONV_FROM_PTR(pInitialDmaList), initialDmaListSize);
		DmaGetList(GET_BUFFER_PTR(g_tagPreBuffer[0]), (U32)0, pInitialDmaList, initialDmaListSize, 0);
	}
#endif
	DISPATCHER_DPRINTF("BATCHJOB: UseBuffer(INPUT_%d %d %05x)\n", 0, GET_BUFFER_TAGID(g_tagPreBuffer[0]), (U32)GET_BUFFER_PTR(g_tagPreBuffer[0]));
	DISPATCHER_DPRINTF("BATCHJOB: UseBuffer(WORK %d %05x)\n", GET_BUFFER_TAGID(g_tagWorkBuffer), (U32)GET_BUFFER_PTR(g_tagWorkBuffer));
	SetInputBuffer(GET_BUFFER_PTR(g_tagPreBuffer[0]));
	SetWorkBuffer(GET_BUFFER_PTR(g_tagWorkBuffer));
	SetOutputBuffer(NULL);
	SetOutputBufferPos(NULL);

	// When the tast starts, the first input buffer contains:
	//						Location				Size		{ contents }
	//		Header(1)		@0						16 bytes	{ commandLoc(1), volatileLoc(1), 0, dmaHeaderLoc(1), 0, 0 }
	//		Data(1)			@16						(commandLoc(1) - 16) bytes
	//		CommandList(1)	@commandLoc(1)			(dmaHeaderLoc(1) - commandLoc(1)) bytes
	//		Header(2)		@dmaHeaderLoc(1)		16 bytes	{ commandLoc(2), volatileLoc(2), 0, dmaHeaderLoc(2), 0, dmaListSize(2) }
	//		DmaList(2)		@dmaHeaderLoc(1)+16	dmaListSize(2) bytes
	//
	// All subsequent input buffers created by getting DmaList(i) contain:
	//						Location				Size		{ contents }
	//		Data(i)			@16						(commandLoc(i) - 16) bytes
	//		CommandList(i)	@commandLoc(i)			(dmaHeaderLoc(i) - commandLoc(i)) bytes
	//		Header(i+1)		@dmaHeaderLoc(i)		16 bytes	{ commandLoc(i+1), volatileLoc(i+1), 0, dmaHeaderLoc(i+1), 0, dmaListSize(i+1) }
	//		DmaList(i+1)	@dmaHeaderLoc(i)+16	dmaListSize(i+1) bytes

	DISPATCHER_DPRINTF("BATCHJOB: InputBuffer[0]   %05x @%04x size %04x\n", (U32)GET_BUFFER_PTR(g_tagPreBuffer[0]), Ice::BatchJob::kLocInput, BATCHJOB_NAMESPACE::kInputBufferSize);
	DISPATCHER_DPRINTF("          WorkBuffer at    %05x @%04x size %04x\n", (U32)GET_BUFFER_PTR(g_tagWorkBuffer), Ice::BatchJob::kLocWork, BATCHJOB_NAMESPACE::kWorkBufferSize);
	DISPATCHER_DPRINTF("          OutputDmaList[0] %05x       size %04x\n", (U32)GET_BUFFER_PTR(g_tagPreBuffer[0]) + BATCHJOB_NAMESPACE::kOutputBufferOffset0, BATCHJOB_NAMESPACE::kDmaOutputListMaxSize);
	DISPATCHER_DPRINTF("          OutputBuffer[0]  %05x @%04x size %04x\n", (U32)GET_BUFFER_PTR(g_tagPreBuffer[0]) + BATCHJOB_NAMESPACE::kOutputBufferOffset0 + BATCHJOB_NAMESPACE::kDmaOutputListMaxSize, Ice::BatchJob::kLocOutput, BATCHJOB_NAMESPACE::kDmaOutputMaxSize);
#if ICE_TARGET_PS3_SPU
	DISPATCHER_DPRINTF("          JobMain at        %05x\n", (U32)&JobMain);
	DISPATCHER_DPRINTF("          _end at          %05x\n", (U32)&_end);
#endif
//	DISPATCHER_DPRINTF("BATCHJOB: initial data :\n");
//	DISPATCHER_DPRINT_Mem("Initial input buffer", pPreBuffer, 512);
//	DISPATCHER_DPRINTF("         ...\n");

	g_tagPostBuffer[0] = g_tagPostBuffer[1] = 0;
	g_currentInputIndex = g_currentOutputIndex = 0;
	g_currentOutputSize = 0;
	g_outputDmaListSize = 0;

	U16 dmaHeaderLoc = 0;	// First dma header is at location 0 in the current input buffer
	DISPATCHER_AUDIT(Ice::BatchJob::kAuditInitEnd);

	while (1)	// loop until we hit a kCmdEnd
	{
		// Get the dma header from the previous input buffer (the initial dma header from the current buffer, on the first iteration)
		// This dma header is the one that contains information about the locations of the command list and dma header
		// in the current buffer.
		U32 const *dmaHeader= 	(U32 const *)LocToPtr(dmaHeaderLoc);
		DISPATCHER_DPRINTF("BATCHJOB: DMA header @%04x in PreBuffer[%d]\n", dmaHeaderLoc, (U32)(dmaHeaderLoc ? 1-g_currentInputIndex : 0));

#if !DOUBLE_BUFFERED
		// if SINGLE_BUFFERED: on every iteration after the first, get the input buffer we want to process immediately before using it
		if (dmaHeaderLoc != 0) {
			U32	dmaListSize		= dmaHeader[3]; // size of dma list
			U32 const *dmaListPtr = (dmaHeader + 4);	// dma list immediately follows header

			// Start DMA transfer of the next list into the other buffer - the resulting buffer is associated with header in this buffer
			if (!g_tagPreBuffer[g_currentInputIndex]) {
				// get our second input buffer when we need it
# if ICE_TARGET_PS3_SPU
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferStart, (U16)1);
				WwsJob_BufferTag tag = WwsJob_JobApiGetBufferTag(Ice::BatchJob::kInputBufferSet, kLogicalBuffer1, WwsJob_kAllocDmaTag);
				g_tagPreBuffer[g_currentInputIndex] = BUFFER_TAG_FROM_WWS(tag);
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferEnd, (U16)1);
# else
				g_tagPreBuffer[g_currentInputIndex] = BUFFER_TAG( g_memoryLS + kBufferOffset + BATCHJOB_NAMESPACE::kInputBufferOffset1, 1 );
# endif
				DISPATCHER_DPRINTF("BATCHJOB: UseBuffer(INPUT_%d %d %05x)\n", (U32)g_currentInputIndex, GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]), (U32)GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]));
			}
			U32 dmaTagId = GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]);
			U8 *inputBuf = GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]);
			DISPATCHER_DPRINTF("BATCHJOB: DMAH @%04x:\n", dmaHeaderLoc);
			DISPATCHER_DPRINTF("BATCHJOB: GET(%05x <- LIST@%04x %04x) : VOLA @%04x CMDL @%04x DMAH @%04x\n",
					 DEBUG_Ptr(inputBuf),
					 DEBUG_PtrToLoc(dmaListPtr),
					 dmaListSize,
					 dmaHeader[0] >> 16,
					 dmaHeader[0] & 0xFFFF,
					 dmaHeader[1] & 0xFFFF);
			DISPATCHER_DPRINT_DmaList(dmaListPtr, dmaListSize, 0, inputBuf);

			DISPATCHER_DPRINTF("BATCHJOB: DmaGetList(%05x <- @%04x, %04x, tag %d)\n", (U32)inputBuf, DEBUG_PtrToLoc(dmaListPtr), dmaListSize, dmaTagId);
			DmaGetList(inputBuf, 0, dmaListPtr, dmaListSize, dmaTagId);
		}
#endif

		// Wait for the input buffer we want to process this frame to arrive
		{
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputDmaStart);										// before waiting for pre-buffer
			//NOTE: on the first iteration, this does nothing, since the first input buffer has always arrived by this point.
			U32 dmaTagId = GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]);
			U8 *inputBuf = GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]);
			// Wait for the current buffer to finish uploading (NOTE: this will do nothing the first time through if DOUBLE_BUFFERED)
			// We need the header and dma list in it to launch the next DmaGetList, as well as the command list we're about to process.
			U32 dmaTagGroup = (1 << dmaTagId);
			DISPATCHER_DPRINTF("BATCHJOB: DMA WAIT(INPUT_%d %d (%08x))\n", (U32)g_currentInputIndex, dmaTagId, dmaTagGroup);
			WaitOnDma( dmaTagGroup );

			// switch all commands to read relative to the input buffer we just received
			SetInputBuffer( inputBuf );
			DISPATCHER_DPRINTF("BATCHJOB: SetInputBuffer(INPUT_%d %05x)\n", (U32)g_currentInputIndex, (U32)inputBuf);
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputDmaEnd, (U16)((dmaHeader[1] & 0xFFF0) + 0x10 + dmaHeader[3]));  // after waiting for pre-buffer
		}

		// Read the location of the command list we want to process this iteration and the rest of the DMA header from
		// the previous input buffer before we discard it:
		U16 volatileLoc =		(U16)(dmaHeader[0]>>16);		// Location (LS) for volatile data
		U16 commandLoc =		(U16)(dmaHeader[0] & 0xFFFF);	// Location (LS) of command data
		dmaHeaderLoc =			(U16)(dmaHeader[1] & 0xFFFF);	// Location (LS) for next DMA header and list.
		DISPATCHER_DPRINTF("         VOLA @%04x CMDL @%04x DMAH @%04x in PreBuffer[%d]\n", volatileLoc, commandLoc, dmaHeaderLoc, (U32)g_currentInputIndex);
		DISPATCHER_ASSERTF(((volatileLoc & 0xF) == Ice::BatchJob::kLocInput) && ((volatileLoc&~0xF) < BATCHJOB_NAMESPACE::kInputBufferSize), "BATCHJOB: Invalid volatile location %04x: input buffer[%d] is corrupt!\n", volatileLoc, (U32)g_currentInputIndex);
		DISPATCHER_ASSERTF(((commandLoc & 0xF) == Ice::BatchJob::kLocInput) && ((commandLoc&~0xF) < BATCHJOB_NAMESPACE::kInputBufferSize), "BATCHJOB: Invalid command location %04x: input buffer[%d] is corrupt!\n", commandLoc, (U32)g_currentInputIndex);
		DISPATCHER_ASSERTF((dmaHeaderLoc == 0) || (((dmaHeaderLoc & 0xF) == Ice::BatchJob::kLocInput) && ((dmaHeaderLoc&~0xF) < BATCHJOB_NAMESPACE::kInputBufferSize)), "BATCHJOB: Invalid dma header location %04x: input buffer[%d] is corrupt!\n", dmaHeaderLoc, (U32)g_currentInputIndex);
		SetVolatileBuffer( LocToPtr(volatileLoc) );

		// Swap input buffer indices for next frame
		g_currentInputIndex = 1 - g_currentInputIndex;

#if DOUBLE_BUFFERED
		// if DOUBLE_BUFFERED: immediately start the DMA for the input buffer we'll use next iteration so it will upload
		// while we process this buffer.
		if (dmaHeaderLoc != 0) {
			// Read in the next DMA header from the current buffer and use it to start the DMA of next iteration's data
			dmaHeader = (U32 const *)LocToPtr(dmaHeaderLoc);
			U32	dmaListSize		= dmaHeader[3]; // size of dma list
			U32 const *dmaListPtr = (dmaHeader + 4);	// dma list immediately follows header

			DISPATCHER_DPRINTF("BATCHJOB: dmaListSize=%08x\n", dmaListSize);

			// Start DMA transfer of the next list into the other buffer - the resulting buffer is associated with header in this buffer
			if (!g_tagPreBuffer[g_currentInputIndex]) {
				// get our second input buffer when we need it
# if ICE_TARGET_PS3_SPU
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferStart, (U16)1);
				WwsJob_BufferTag tag = WwsJob_JobApiGetBufferTag(Ice::BatchJob::kInputBufferSet, kLogicalBuffer1, WwsJob_kAllocDmaTag);
				g_tagPreBuffer[g_currentInputIndex] = BUFFER_TAG_FROM_WWS(tag);
				DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputUseBufferEnd, (U16)1);
# else
				g_tagPreBuffer[g_currentInputIndex] = BUFFER_TAG( g_memoryLS + kBufferOffset + BATCHJOB_NAMESPACE::kInputBufferOffset1, 1 );
# endif
				DISPATCHER_DPRINTF("BATCHJOB: UseBuffer(INPUT_%d %d %05x)\n", (U32)g_currentInputIndex, GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]), (U32)GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]));
			}
			U32 dmaTagId = GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]);
			U8 *inputBuf = GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]);
			DISPATCHER_DPRINTF("BATCHJOB: DMAH @%04x:\n", dmaHeaderLoc);
			DISPATCHER_DPRINTF("BATCHJOB: GET(%05x <- LIST@%04x %04x) : CMDL @%04x DMAH @%04x\n",
					 DEBUG_Ptr(inputBuf),
					 DEBUG_PtrToLoc(dmaListPtr),
					 dmaListSize,
					 dmaHeader[0],
					 dmaHeader[1]);
			DISPATCHER_DPRINT_DmaList(dmaListPtr, dmaListSize, 0, inputBuf);

			DISPATCHER_DPRINTF("BATCHJOB: DmaGetList(%05x <- @%04x, %04x, tag %d)\n", (U32)inputBuf, DEBUG_PtrToLoc(dmaListPtr), dmaListSize, dmaTagId);
			DmaGetList(inputBuf, 0, dmaListPtr, dmaListSize, dmaTagId);
		} else
#else
		if (dmaHeaderLoc == 0)
#endif
		{
#if ICE_TARGET_PS3_SPU
			//NOTE: we delay calling LoadNextJob until we release our first input buffer;
			// otherwise, interrupts and job api calls will generate failed calls to TryExecuteLoadCmds,
			// which we know is just waiting on us to release an input buffer...
			g_tagPreBuffer[g_currentInputIndex] = 0;
			DISPATCHER_DPRINTF("ICEANIM: FreeBuffer(INPUT_%d %d %05x)\n", (U32)g_currentInputIndex, GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]), (U32)GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]));
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputFreeBufferStart);
			WwsJob_JobApiFreeLogicalBuffer( Ice::BatchJob::kInputBufferSet, g_currentInputIndex );
			WwsJob_JobApiLoadNextJob();
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditInputFreeBufferEnd);
#else
			if (g_tagPreBuffer[g_currentInputIndex] != 0) {
				DISPATCHER_DPRINTF("ICEANIM: FreeBuffer(INPUT_%d %d %05x)\n", (U32)g_currentInputIndex, GET_BUFFER_TAGID(g_tagPreBuffer[g_currentInputIndex]), (U32)GET_BUFFER_PTR(g_tagPreBuffer[g_currentInputIndex]));
				g_tagPreBuffer[g_currentInputIndex] = 0;
			}
#endif
		}

		// Set up to process the current command list
		U16 const *command = (U16 const *)LocToPtr(commandLoc);
		DISPATCHER_DPRINTF("BATCHJOB: CMDL @%04x (%05x):\n", commandLoc, (U32)DEBUG_Ptr(command));

		// Process the current command list
		DISPATCHER_AUDIT(Ice::BatchJob::kAuditCmdStart);
		while (1)
		{
			U16 cmd = command[0];
			DISPATCHER_DPRINT_Command(command, g_memoryMap, (void const*const*)g_CommandFunction, g_CommandNumParams);
			if (cmd == Ice::BatchJob::kCmdSwap) {
				break;
			}
			if (cmd == Ice::BatchJob::kCmdEnd) {
				EndCommands();
#if ICE_TARGET_PS3_SPU
//				DISPATCHER_PRINTF("BATCHJOB: jobid %08x end\n", WwsJob_JobApiGetJobId());
#endif
				return;
			}

			DISPATCHER_ASSERTF(cmd < BATCHJOB_NAMESPACE::kNumCommands && g_CommandFunction[cmd] != NULL, "Invalid command %04x @ %04x\n", command[0], DEBUG_CommandPtr(command, g_memoryMap));
			DISPATCHER_DPRINT_CommandInputData(command, g_memoryMap);

			(*g_CommandFunction[cmd])(UNPACK_DISPATCHER_ARGS(command+1), g_memoryMap);

			DISPATCHER_DPRINT_CommandOutputData(command, g_memoryMap);
			DISPATCHER_AUDIT(Ice::BatchJob::kAuditCmd, cmd, (U64)DISPATCHER_AUDIT_GetNumItems(command));
			command += 1 + g_CommandNumParams[cmd];
		}
	}
}
