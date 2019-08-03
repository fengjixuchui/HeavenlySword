/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icebase.h"
#include "icemeshinternal.h"
#include "icemeshstructs.h"

#ifdef __SPU__
#include "icemeshdma.h"
#endif

using namespace Ice;
using namespace Ice::MeshProc;

namespace Ice
{
	namespace MeshProc
	{
#ifndef __SPU__
		U8 ICE_ALIGN(128) g_meshInputBuf[kMeshInputBufferSize];
		U8 ICE_ALIGN(128) g_meshWorkBuf[kMeshWorkBufferSize];
		U8 ICE_ALIGN(128) g_meshOutputBuf[2][kMeshWorkBufferSize];

		// This data is present in SPU mode, but doesn't naturally exist in PPU mode, so it mist be passed to each job.
		U32 g_spuNum;
		U32 g_jobNum;

		// Space for tracking of allocations for debuggining purposes.
#define MAX_MEMORY_ALLOCS 512
		U64 s_numAllocs;
		U32 s_allocStart[MAX_MEMORY_ALLOCS];
		U32 s_allocSize[MAX_MEMORY_ALLOCS];
#endif
	}

}

// Allocate requested memory on the memory stack in the work buffer.
// If not enough memory is left, then we assert.
U8 *Ice::MeshProc::MeshMemAllocate(U32 *pStaticMem, U32 size)
{
	size = (size + 0xF) & ~0xF;
	U32 start = pStaticMem[kStaticWorkFreeStart];
	U32 newStart = start + size;
#ifndef __SPU__
	ICE_ASSERT(s_numAllocs <= MAX_MEMORY_ALLOCS);
	s_allocStart[s_numAllocs] = start;
	s_allocSize[s_numAllocs] = size;
	s_numAllocs++;
#endif
	if (newStart > pStaticMem[kStaticWorkFreeEnd])
	{
		PRINTF("ERROR: Allocation of %d was larger than %d left!\n", size, pStaticMem[kStaticWorkFreeEnd] - start);
#ifndef __SPU__
		PRINTF("Num:  Address     Size     Left\n");
		for (U64 ii = 0; ii < s_numAllocs; ii++)
		{
			PRINTF("%3lld: %08X %8X %8X\n", ii, s_allocStart[ii], s_allocSize[ii],
					pStaticMem[kStaticWorkFreeEnd] - s_allocStart[ii]);
		}
#endif
		ICE_ASSERT(newStart <= pStaticMem[kStaticWorkFreeEnd]);
	}

	pStaticMem[kStaticWorkFreeStart] = newStart;
#ifndef __SPU__
	static U32 maxUsed = 0;
	if (maxUsed < kMeshWorkBufferSize - pStaticMem[kStaticWorkFreeEnd] + newStart)
	{
		maxUsed = kMeshWorkBufferSize - pStaticMem[kStaticWorkFreeEnd] + newStart;
		PRINTF("max work buffer usage: %lf, %d used\n", (float)maxUsed/(float)kMeshWorkBufferSize, maxUsed);
	}
#endif

	return (U8 *)start;
}

// Allocate requested memory from the end of the memory stack.
// If not enough memory is left, then we assert.
U8 *Ice::MeshProc::MeshMemAllocateEnd(U32 *pStaticMem, U32 size)
{
	U32 end = pStaticMem[kStaticWorkFreeEnd];
	U32 newEnd = end - size;
	if (newEnd < pStaticMem[kStaticWorkFreeStart])
	{
		PRINTF("ERROR: Allocation of %d was larger than %d left!\n", size, end - pStaticMem[kStaticWorkFreeStart]);
		ICE_ASSERT(newEnd >= pStaticMem[kStaticWorkFreeStart]);
	}
	pStaticMem[kStaticWorkFreeEnd] = newEnd;

	return (U8 *)newEnd;
}

static U32 MeshRingBufferAllocate(U32 *pStaticMem, U32 size)
{
#ifdef __SPU__
	U32 *pSemaphoreValue = &pStaticMem[kStaticAtomic];

	// Make sure that the ring buffer allocator has been loaded from main memory.
	U32 workDmaTagId = pStaticMem[kStaticWorkBufferTagId];
	WaitOnDma(1 << workDmaTagId);
#endif

	// Read the values from the ring buffer allocator.
	U32 startPtr = pStaticMem[kStaticRingBufferStartPtr];
	ICE_ASSERT((startPtr & 0x7F) == 0);
	U32 endPtr = pStaticMem[kStaticRingBufferEndPtr];
	ICE_ASSERT((endPtr & 0x7F) == 0);
	U32 freeStartPtr = pStaticMem[kStaticRingBufferFreeStartPtr];
	ICE_ASSERT((freeStartPtr & 0x7F) == 0);
	U32 semaphorePtr = pStaticMem[kStaticRingBufferSemaphorePtr];
	ICE_ASSERT((semaphorePtr & 0xF) == 0);

	// Always allocate a size which is a mutliple of 128 bytes.
	size = (size + 0x7F) & ~0x7F;

	U32 freeStartPlusSize = freeStartPtr + size;
	U32 startPlusSize = startPtr + size;

	bool success = false;
	do {
#ifdef __SPU__
		// Get the value that has been written into the semaphore by the GPU.
		StartDma(pSemaphoreValue, semaphorePtr, 4, workDmaTagId, kDmaGet);

		// Wait until the semaphore has been read.
		// This also makes sure that the ring buffer allocator has been loaded from main memory.
		WaitOnDma(1 << workDmaTagId);

		U32 freeEndPtr = *pSemaphoreValue;
#else
		// Get the value that has been written into the semaphore by the GPU.
		U32 freeEndPtr = *(U32 *)semaphorePtr;
#endif

		// Test to see if the allocation will fit.
		if (((freeEndPtr <= freeStartPtr) || (freeStartPlusSize < freeEndPtr)) &&
				((freeStartPlusSize <= endPtr) || (startPlusSize < freeEndPtr)))
		{
			// Start at the beginning of the ring buffer.
			if (freeStartPlusSize > endPtr) {
				freeStartPtr = startPtr;
				freeStartPlusSize = startPlusSize;
			}
			success = true;
		}
		// May want to wait for a bit if this fails...

	} while (!success);

	// Save the new free start pointer.
	pStaticMem[kStaticRingBufferFreeStartPtr] = freeStartPlusSize;

	// Return the output pointer.
	return freeStartPtr;
}

U32 MeshMutexAllocate(U32 *pStaticMem, U32 size)
{
	// The mutex is only to be used when using a single combined buffer, so make sure we are not using ring buffers.
	U32 semaphorePbHolePtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x28);
	ICE_ASSERT(semaphorePbHolePtr == 0);

	U32 *pMutex = (U32 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x10));
	ICE_ASSERT(((U32)pMutex & 0x7F) == 0);
	size = (size + 0x7F) & ~0x7F;
	volatile U32 outputStartPtr;
	volatile U32 outputEndPtr;

#ifdef __SPU__
	volatile U32 *pLsMutex = &pStaticMem[kStaticAtomic];

	// Get the current output buffer address and update it in an atomic operation.
	do {
		// Get the mutex into LS.
		Getllar((U32)pLsMutex, (U32)pMutex);

		// Grab the start and end pointers from the mutex.
		outputStartPtr = pLsMutex[0];  // Start address is stored in first quadword.
		outputEndPtr = pLsMutex[4];    // End address is stores in the second quadword.

		// Check to see if memory is available for this allocation.
		if (outputStartPtr + size > outputEndPtr) {
			// If not, inform the system of an allocation failure.
			outputStartPtr = 0;
			pLsMutex[8] += size;         // Size of allocation failures is in the third quadword.
		}
		else {
			// Otherwise update the mutex with the new address from which allocate.
			pLsMutex[0] = outputStartPtr + size;
		}
	} while (Putllc((U32)pLsMutex, (U32)pMutex));
#else
	// Grab the start and end pointers from the mutex.
	outputStartPtr = pMutex[0];
	outputEndPtr = pMutex[4];

	// Check to see if memory is available for this allocation.
	if (outputStartPtr + size > outputEndPtr) {
		// If not, inform the system of an allocation failure.
		outputStartPtr = 0;
		pMutex[8] += size;         // Size of allocation failures is in the third quadword.
	}
	else {
		// Otherwise update the mutex with the new address from which allocate.
		pMutex[0] = outputStartPtr + size;
	}
#endif

	if (outputStartPtr == 0) {
		pStaticMem[kStaticOutputBufferAllocationFailure] = ~0;
	}

	return outputStartPtr;
}

void MeshKickOutputBufferToAddr(U32 *pStaticMem, U32 outputStartPtr, Ice::MeshProc::IceQuadWord noUpdateKickPtr)
{
	U32 outputKickPtr = pStaticMem[kStaticOutputKickPtr];
	U32 size = pStaticMem[kStaticOutputFree] - outputKickPtr;
	ICE_ASSERT((outputKickPtr & 0xF) == 0);
	ICE_ASSERT((size & 0xF) == 0);
	if (size == 0)
		return;

	if (!noUpdateKickPtr.m_data.u32.A) {
		pStaticMem[kStaticOutputKickPtr] = outputKickPtr + ((size + 0x7F) & ~0x7F);
		pStaticMem[kStaticOutputFree] = (pStaticMem[kStaticOutputFree] + 0x7F) & ~0x7F;
	}

	if (pStaticMem[kStaticOutputBufferAllocationFailure])
		return;

#ifdef __SPU__
	U32 bufNum = pStaticMem[kStaticOutputBufferNum];
	U32 dmaTagId = pStaticMem[kStaticOutputBufferPtr + (bufNum >> 2)] >> 24;

	// Perform the DMA
	while (size > 16384)
	{
		StartDma((void *)outputKickPtr, outputStartPtr, 16384, dmaTagId, kDmaPut);
		size -= 16384;
		outputStartPtr += 16384;
		outputKickPtr += 16384;
	}
	StartDma((void *)outputKickPtr, outputStartPtr, size, dmaTagId, kDmaPut);
#else
	U64 *pBigBuf = (U64 *)outputStartPtr;
	U64 *pOutputBuf = (U64 *)outputKickPtr;
	size = size >> 3;
	for (U64 ii = 0; ii < size; ++ii)
	{
		*pBigBuf = *pOutputBuf;
		pBigBuf++;
		pOutputBuf++;
	}
#endif
}

void MeshKickOutputBuffer(U32 *pStaticMem, U32 *pOutputStartPtr)
{
	U32 outputKickPtr = pStaticMem[kStaticOutputKickPtr];
	U32 size = pStaticMem[kStaticOutputFree] - outputKickPtr;
	ICE_ASSERT((outputKickPtr & 0xF) == 0);
	ICE_ASSERT((size & 0xF) == 0);
	if (size == 0)
		return;

	U32 outputStartPtr;
	if (pOutputStartPtr == NULL)
	{
		// Deterime if we are using a single combined buffer or multiple rin buffers for output.
		U32 semaphorePbHolePtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x28);
		if (semaphorePbHolePtr == 0)
			outputStartPtr = MeshMutexAllocate(pStaticMem, size);
		else
			outputStartPtr = MeshRingBufferAllocate(pStaticMem, size);
	}
	else
	{
		outputStartPtr = *pOutputStartPtr;
		*pOutputStartPtr = outputStartPtr + size;
	}

	// Patch main memory pointers with the real output position.
	U32 outputAddrOffset = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x14);
	U32 outputStartPtrAdjust = outputStartPtr - outputAddrOffset;
	for (U32 ii = 0; ii < 16; ii++)
	{
		U32 *patchPtr = (U32 *)pStaticMem[kStaticOutputPatchPtr + ii];
		if (patchPtr != NULL)
		{
			*patchPtr = *patchPtr + outputStartPtrAdjust;
			pStaticMem[kStaticOutputPatchPtr + ii] = 0;
		}
	}
	pStaticMem[kStaticOutputNumPatchPtrs] = 0;

	MeshKickOutputBufferToAddr(pStaticMem, outputStartPtr);
}

// Allocate space in an output buffer.  If the current output
// buffer does not have enough room, then we need to kick it, and
// get a new output buffer.  Also, if we have never gotten an
// output buffer then we need to do that first.
U8 *MeshOutputAllocate(U32 *pStaticMem, U32 size, U32 alignment, U32 *patchPtr, U32 *pOutputStartPtr)
{
	size = (size + 0xF) & ~0xF;
	U32 bufNum = pStaticMem[kStaticOutputBufferNum];
	U32 outputBufInfo = pStaticMem[kStaticOutputBufferPtr + (bufNum >> 2)];
	U32 outputBufPos = (pStaticMem[kStaticOutputFree] + (alignment - 1)) & ~(alignment - 1);
	pStaticMem[kStaticOutputFree] = outputBufPos;
#ifdef __SPU__
	U32 outputBuf = outputBufInfo & 0x3FFFF;   // mask out output buffer DMA tag id

	if (outputBuf == NULL)
	{
		WwsJob_Command jobCommands[2] WWSJOB_ALIGNED(16);
		jobCommands[0].UseUninitializedBuffer(kMeshOutputBufferSetNum, 0, 0, 0, WwsJob_Command::kNonCached);
		jobCommands[1].AddEndCommand();

		WwsJob_JobApiExecuteCommands(jobCommands);

		WwsJob_BufferTag outputBufferTag = WwsJob_JobApiGetBufferTag(kMeshOutputBufferSetNum, 0, WwsJob_kAllocDmaTag);
		outputBufInfo = (outputBufferTag.m_dmaTagId << 24) | (outputBufferTag.m_lsAddressInWords << 2);
		outputBuf = outputBufInfo & 0x3FFFF;   // mask out output buffer DMA tag id
		outputBufPos = outputBuf;
		pStaticMem[kStaticOutputKickPtr] = outputBuf;
	}
#else
	U32 outputBuf = outputBufInfo;

	if (outputBuf == NULL)
	{
		outputBufInfo = (U32)g_meshOutputBuf[bufNum >> 2];
		outputBuf = outputBufInfo;
		outputBufPos = outputBuf;
		pStaticMem[kStaticOutputKickPtr] = outputBuf;
	}
#endif

	// Check to make sure the requested size will fit and we have not
	// run out of patch pointers.  If it doesn't, then kick the current
	// output buffer and get the other one.
	U32 numPatchPtrs = pStaticMem[kStaticOutputNumPatchPtrs];
	U32 left = outputBuf + kMeshOutputBufferSize - outputBufPos;
	if ((left < size) || (numPatchPtrs >= 16))
	{
		if (pOutputStartPtr != NULL || patchPtr != NULL)
		{
			MeshKickOutputBuffer(pStaticMem, pOutputStartPtr);
		}
		// Kick output buffer
		bufNum = 4 - bufNum;
		outputBufInfo = pStaticMem[kStaticOutputBufferPtr + (bufNum >> 2)];

#ifdef __SPU__
		outputBuf = outputBufInfo & 0x3FFFF;   // mask out output buffer DMA tag id

		if (outputBuf == NULL)
		{
			WwsJob_Command jobCommands[2] WWSJOB_ALIGNED(16);
			jobCommands[0].UseUninitializedBuffer(kMeshOutputBufferSetNum, 1, 0, 0, WwsJob_Command::kNonCached);
			jobCommands[1].AddEndCommand();

			WwsJob_JobApiExecuteCommands(jobCommands);

			WwsJob_BufferTag outputBufferTag = WwsJob_JobApiGetBufferTag(kMeshOutputBufferSetNum, 1, WwsJob_kAllocDmaTag);
			outputBufInfo = (outputBufferTag.m_dmaTagId << 24) | (outputBufferTag.m_lsAddressInWords << 2);
			outputBuf = outputBufInfo & 0x3FFFF;   // mask out output buffer DMA tag id
		}
		else
		{
			// wait for output from previous buffer to complete
			WaitOnDma(1 << (outputBufInfo >> 24));
		}
#else
		outputBuf = outputBufInfo;

		if (outputBuf == NULL)
		{
			outputBufInfo = (U32)g_meshOutputBuf[bufNum >> 2];
			outputBuf = outputBufInfo;
		}
#endif

		outputBufPos = outputBuf;
		pStaticMem[kStaticOutputKickPtr] = outputBuf;
	}

	// Store output buffer state back into the static variables.
	pStaticMem[kStaticOutputBufferNum] = bufNum;
	pStaticMem[kStaticOutputBufferPtr + (bufNum >> 2)] = outputBufInfo;
	pStaticMem[kStaticOutputFree] = outputBufPos + size;

	// Store output buffer patch
	if (patchPtr != NULL)
	{
		numPatchPtrs = pStaticMem[kStaticOutputNumPatchPtrs];
		pStaticMem[kStaticOutputNumPatchPtrs] = numPatchPtrs + 1;
		pStaticMem[kStaticOutputPatchPtr + numPatchPtrs] = (U32)patchPtr;
		*patchPtr = outputBufPos - pStaticMem[kStaticOutputKickPtr];
	}

	return (U8 *)outputBufPos;
}

static void MeshProcessingStart(U32 inputBufferPtr, U32 inputBufferTagId, U32 workBufferPtr, U32 workBufferTagId)
{
	U32 *pWorkBuf = (U32 *)workBufferPtr;
	U32 *pStaticMem = (U32 *)pWorkBuf;

	// Set all static variables to 0
	// This is done by rounding up to the nearest 128 bytes
	U32 staticSize = (kStaticNum + 0x1F) & ~0x1F;
	for (U32 ii = 0; ii < staticSize; ii++)
	{
		pStaticMem[ii] = 0;
	}

	// Setup static variables
	pStaticMem[kStaticInputBufferPtr] = inputBufferPtr;
	pStaticMem[kStaticInputBufferTagId] = inputBufferTagId;
	pStaticMem[kStaticWorkBufferTagId] = workBufferTagId;
	pStaticMem[kStaticWorkFreeStart] = (U32)pWorkBuf + kStaticNum * sizeof(U32);
	pStaticMem[kStaticWorkFreeEnd] = (U32)pWorkBuf + kMeshWorkBufferSize;

	// Copy command list to work buffer
	U32 cmdSize = (((U16 *)inputBufferPtr)[0] + 0xF) & ~0xF;
	U8 *cmdDst = MeshMemAllocate(pStaticMem, cmdSize);
	CopyQWords(cmdDst, (U8 *)inputBufferPtr, cmdSize);
	pStaticMem[kStaticCmdPtr] = (U32)cmdDst;

	InterpretCommandList(pStaticMem);
}

#ifdef __SPU__

extern "C" {
	void JobMain();
}

void JobMain()
{
	ICE_ASSERT(kStaticNum * sizeof(U32) < 1536); // Maximum size of the statics.

	ICE_ASSERT((kMeshWorkBufferSize & 0x3FF) == 0);
	ICE_ASSERT((kMeshOutputBufferSize & 0x3FF) == 0);

	WwsJob_Command jobCommands[5] WWSJOB_ALIGNED(16);
	jobCommands[0].ReserveBufferSet(kMeshWorkBufferSetNum, 1, kMeshWorkBufferSetOrg >> 10, kMeshWorkBufferSize >> 10);
	jobCommands[1].ReserveBufferSet(kMeshOutputBufferSetNum, 2, kMeshOutputBufferSetOrg >> 10, kMeshOutputBufferSize >> 10);
	jobCommands[2].UseUninitializedBuffer(kMeshWorkBufferSetNum, 0, 0, 0, WwsJob_Command::kNonCached);
	jobCommands[3].UnreserveBufferSets(1 << kMeshWorkBufferSetNum);
	jobCommands[4].AddEndCommand();

	WwsJob_JobApiExecuteCommands(jobCommands);

	WwsJob_BufferTag inputBufferTag = WwsJob_JobApiGetBufferTag(kMeshInputBufferSetNum, 0, WwsJob_kAllocDmaTag);
	WwsJob_BufferTag workBufferTag = WwsJob_JobApiGetBufferTag(kMeshWorkBufferSetNum, 0, WwsJob_kAllocDmaTag);

	MeshProcessingStart(inputBufferTag.m_lsAddressInWords << 2, inputBufferTag.m_dmaTagId,
			workBufferTag.m_lsAddressInWords << 2, workBufferTag.m_dmaTagId);

	WwsJob_JobApiFreeLogicalBuffer(kMeshWorkBufferSetNum, 0);
}

#else

void Ice::MeshProc::MeshProcessing(U32 *pSpud, U32 spudSize, U32 spuNum, U32 jobNum)
{
	// Set globals that would normally be set by the job manager in SPU mode.
	g_spuNum = spuNum;
	ICE_ASSERT(spuNum < 6);
	g_jobNum = jobNum;

	// Clear out allocation tracking
	s_numAllocs = 0;

	// Get Buffers
	U8 *pInputBuf = g_meshInputBuf;
	MeshLoadInputBuffer(pSpud, spudSize, pInputBuf);
	U8 *pWorkBuf = g_meshWorkBuf;
	ICE_ASSERT(kStaticNum * sizeof(U32) < 1536); // Dave says this is the static size limit

	MeshProcessingStart((U32)pInputBuf, 0, (U32)pWorkBuf, 0);
}

#endif

