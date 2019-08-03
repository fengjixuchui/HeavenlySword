/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

#ifdef __SPU__
#include "icemeshdma.h"
#endif

using namespace Ice;
using namespace Ice::MeshProc;

// Constructs a 4-byte NV command, given a register and a data size.
// The data size is given in bytes, must be a multiple of 4 in the
// range 0x0004-0x07FC, and does not include the size of the command
// word itself.
#define NV_COMMAND(registerAddr, dataSize) (((dataSize) << 16) | (registerAddr))

/**
 * Inserts No-op commands at pHole until pHoleEnd is reached.
 *
 * @return A pointer to the address just after the last No-op written.
 *         This should be pHoleEnd!
 */
U32 *FillHoleWithNoOps(U32 *pHole, U32 *pHoleEnd)
{
	while(pHole < pHoleEnd)
	{
		*pHole++ = 0;
	}

	// Ensure we didn't write past the end of the hole.
	if (pHole != pHoleEnd)
		PRINTF("Hole not filled: Wrote to 0x%p, but hole ends at 0x%p\n",
			pHole, pHoleEnd);
	ICE_ASSERT(pHole == pHoleEnd);
	return pHole;
}

/**
 * Fills a hole with appropriate setup commands and calls to
 * DrawElements.
 *
 * @param pHole Pointer to pre-allocated hole buffer.  The draw calls will be written here.
 * @param indexBufferAddr Address of index buffer (in final command list).
 * @param indexCount Number of entries in the index buffer.  If this count is
 *                   zero, no draw calls will be written.
 * @param drawMode The primitive type to draw.  The most common values are
 *                 0x5 (triangles) and 0x8 (quads).
 * @return A pointer to the address in the hole buffer just after the last draw call.
 */
static U32 *GenerateDrawCalls(U32 *pHole, U32 indexBufferAddr, I32 indexCount, U8 drawMode)
{
	// Write draw calls
	if(!indexCount)
		return pHole;

    if ((indexBufferAddr & 0x7F) != 0)
        PRINTF("unaligned index buffer: 0x%08X\n", indexBufferAddr);
	ICE_ASSERT((indexBufferAddr & 0x7F) == 0);

	// Calculate the number of indexes that need to be rendered
	// in order to align the index pointer for subsequent draw
	// commands to a 512-byte boundary

	I32 firstCount = (512 - (indexBufferAddr & 0x01FF)) >> 1;

	// Clamp firstCount to be no greater than count
	if(firstCount > indexCount)
		firstCount = indexCount;
	ICE_ASSERT(firstCount <= 256);

	// Subtract firstCount from count and clamp to zero
	indexCount -= firstCount;
	//if(indexCount < 0) indexCount = 0;
	ICE_ASSERT(indexCount >= 0);

	U32 blockCount = indexCount / 256;
	U32 remainCount = indexCount - blockCount * 256;

	ICE_ASSERT(blockCount * 256 + remainCount >= (U32)indexCount);
	U32 numDrawCommands = blockCount + (remainCount ? 1 : 0) + (firstCount ? 1 : 0);

	*pHole++ = NV_COMMAND(0x181C, 8);              // IndexBufferAddress
	*pHole++ = indexBufferAddr & 0x7FFFFFFF;       // address (context bit removed from high bit)
	*pHole++ = 0x10 | (indexBufferAddr >> 31);     // index format (16 bits per index) and context bit

	*pHole++ = NV_COMMAND(0x1808, 4); // DrawMode
	*pHole++ = drawMode;

	*pHole++ = NV_COMMAND(0x1824, numDrawCommands*4) | 0x40000000;         // DrawElements

	if (firstCount != 0)
	{
		*pHole++ = ((firstCount-1) << 24); // draw up to 512 byte boundary
	}

	U32 indexOffset = firstCount; // offset in indexes (NOT bytes), relative to IndexBufferAddress passed earlier.
	for (U32 ii = 0; ii < blockCount; ++ii)
	{
		*pHole++ = 0xFF000000 | indexOffset; // Draw 256 indexes
		indexOffset += 256;
	}

	if (remainCount != 0)
	{
		*pHole++ = ((remainCount-1) << 24) | indexOffset; // Draw remaining indexes
	}

	*pHole++ = NV_COMMAND(0x1808, 4); // DrawMode
	*pHole++ = 0;                     // end drawing

	return pHole;
}

/**
 * Initialize the reindex table to dummy values (reindex[i] -> 16*i)
 */
void InitializeReindexToDummy(U32 *pStaticMem)
{
	U16 *pReindex = (U16 *)pStaticMem[kStaticReindexPtr];

	// We need to write out an entire quadword of data at the end of the reindex table.
	// If not, we may leave garbage in the other values which will can make the other indexes load the
	// wrong data.
	U32 reindexCount = pStaticMem[kStaticOutputVertexCount];
	U32 writeCount = (reindexCount + 0x7) & ~0x7;
	for (U32 ii = 0; ii < writeCount; ++ii)
		pReindex[ii] = ii * 16;
	pStaticMem[kStaticReindexCount] = reindexCount;
	pStaticMem[kStaticReindexIsDummy] = ~0;
}


/**
 * Checks whether the reindex buffer exists.  If it does, this
 * function does nothing.  If not, the table is allocated and
 * optionally filled with the dummy (identity) reindex mapping.
 */
void Ice::MeshProc::CreateReindexTable(U32 *pStaticMem, bool initializeToDummy)
{
	if (pStaticMem[kStaticReindexPtr] != 0)
		return; // already exists, nothing to do!

	// (reindex table is a multiple of 8 indexes with an extra
	// qword). Remember, it's scaled up by 16!
	U32 reindexCount = pStaticMem[kStaticOutputVertexCount];
	pStaticMem[kStaticReindexPtr] = (U32)MeshMemAllocate(pStaticMem,
			((reindexCount + 0x1F) & ~0xF) * sizeof(U16)); // 2 bytes per reindex
	pStaticMem[kStaticReindexCount] = 0;
	pStaticMem[kStaticReindexIsDummy] = 0;
	if (!initializeToDummy)
		return;
	InitializeReindexToDummy(pStaticMem);
}

void Ice::MeshProc::AddIntToHalfTable(U16 *in, U16 *out, U16 toAdd, U32 numElems)
{
	// 32 at a time, 8 halfs in a quad word, 4 quads at a time
	for (U32 i = 0; i < numElems; i += 32)
	{
		for (U32 ii = 0; ii < 32; ++ii)
		{
			*(out++) = *(in++) + toAdd;
		}
	}
}

void Ice::MeshProc::PatchPixelShader(U32 *pStaticMem)
{
	U16 *pPatchInfo = (U16 *)pStaticMem[kStaticPixelShaderPatchPtr];
	U64 *inst       = (U64 *)pStaticMem[kStaticPixelShaderPtr];
	U64 *constants  = (U64 *)pStaticMem[kStaticPixelShaderConstPtr];
	while(pPatchInfo[1] != 0)
	{
		inst[pPatchInfo[1]*2+0] = constants[pPatchInfo[0]*2+0];
		inst[pPatchInfo[1]*2+1] = constants[pPatchInfo[0]*2+1];
		pPatchInfo += 2;
	}
}

#ifndef __SPU__
// Mimic the task manager load of the data stored in the SPUD.
// sizeOfEntries = 8 * numEntries
void Ice::MeshProc::MeshLoadInputBuffer(U32 *pSpud, U32 sizeOfEntries, U8 *pDest)
{
	U8 *pInitDest = pDest;
	U32 numEntries = sizeOfEntries >> 3;
	for (U64 ii = 0; ii < numEntries; ii++) {

		U64 *pInputData = (U64 *)pSpud[1];
		U32 size = pSpud[0];
		if ((((U32)pInputData & 0xF) != 0) || ((size & 0xF) != 0)) {
			PRINTF("ERROR: Spud entry %lld not aligned!  Addr = 0x%08X, Size = 0x%X\n", ii, (U32)pInputData, size);
			ICE_ASSERT(((U32)pInputData & 0xF) == 0);
			ICE_ASSERT((size & 0xF) == 0);
		}
		if (size > 0x4000)
		{
			PRINTF("ERROR: Spud size %d is too large!\n", size);
			ICE_ASSERT(size <= 0x4000);
		}

		// Make sure the load does not overrun the input buffer.
		ICE_ASSERT((U32)pDest - (U32)pInitDest + size <= kMeshInputBufferSize);

		U64 *pBigDest = (U64 *)pDest;
		size >>= 3;
		for (U64 jj = 0; jj < size; jj++) {
			*pBigDest = *pInputData;
			pBigDest++;
			pInputData++;
		}
		pDest = (U8 *)pBigDest;

		pSpud += 2;
	}
	static int maxInputBufferUsage = 0;
	if (maxInputBufferUsage < pDest - pInitDest)
	{
		maxInputBufferUsage = pDest - pInitDest;
		PRINTF("max input buffer usage: %lf %d used\n", (float)maxInputBufferUsage/(float)kMeshInputBufferSize, maxInputBufferUsage);
	}
}
#endif

static VU8 GetCompletedJobs(U32 *pStaticMem)
{
	// Get pointers to the job completion data in the static memory area.
	U32 *pJobCompletion = &pStaticMem[kStaticJobCompletion];
	VU8 *pJobCompletionBits = (VU8 *)pJobCompletion;

#ifdef __SPU__
	// Wait for the job completion bits to finish being uploaded.
	WaitOnDma(1 << pStaticMem[kStaticWorkBufferTagId]);
#else
	// Copy the job completion data into the work buffer.
	U32 gpuSyncMutexPtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x18);
	VU8 *pJobCompletionLs = (VU8 *)pJobCompletion;
	VU8 *pJobCompletionEa = (VU8 *)(gpuSyncMutexPtr + 128);
	for (U32 ii = 0; ii < 8; ii++)
	{
		pJobCompletionLs[ii] = pJobCompletionEa[ii];
	}
#endif

	// Find the largest base of the six SPUs.
	U32 largestBase = pJobCompletion[0];
	for (U32 iSpu = 1; iSpu < 6; iSpu++)
	{
		largestBase = (pJobCompletion[iSpu * 4] > largestBase) ? pJobCompletion[iSpu * 4] : largestBase;
	}

	VU8 jobsComplete = VU8(0);
#ifdef __SPU__
	// Shift each SPU's completed bits to be in line with the largest base and then or them all together.
	for (U32 iSpu = 0; iSpu < 6; iSpu++)
	{
		U32 shift = largestBase - pJobCompletion[iSpu * 4];
		shift = (shift > 128) ? 128 : shift;
		jobsComplete = spu_or(jobsComplete, spu_slqwbytebc(pJobCompletionBits[iSpu], shift));
	}

	// Shift out completed jobs in groups of 8 and increase the base to reflect this.
	// This can probably be done without a loop.
	while (spu_extract(jobsComplete, 4) == 0xFF)
	{
		largestBase += 8;
		jobsComplete = spu_slqwbyte(jobsComplete, 1);
	}
#else
	// Shift each SPU's completed bits to be in line with the largest base and then or them all together.
	for (U32 iSpu = 0; iSpu < 6; iSpu++)
	{
		U32 shift = largestBase - pJobCompletion[iSpu * 4];
		shift = (shift > 128) ? 128 : shift;
		U32 ICE_ALIGN(16) shiftVec[4];
		shiftVec[3] = shift;
		VU8 *pShiftVec = (VU8 *)shiftVec;
		jobsComplete = vec_vor(jobsComplete, vec_vslo(pJobCompletionBits[iSpu], *pShiftVec));
	}

	// Shift out completed jobs in groups of 8 and increase the base to reflect this.
	U8 *pJobsComplete = (U8 *)&jobsComplete;
	while (pJobsComplete[4] == 0xFF)
	{
		largestBase += 8;
		jobsComplete = vec_vslo(jobsComplete, VU8(8));
	}
#endif

	// Set the preferred word to the base.
	U32 *pBase = (U32 *)&jobsComplete;
	pBase[0] = largestBase;

	return jobsComplete;
}

static void SyncGpuPutPtr(U32 *pStaticMem)
{
#ifdef __SPU__
	U32 spuNum = WwsJob_JobApiGetSpuNum();
	U32 jobNum = WwsJob_JobApiGetJobId() & 0xFFFF;
#else
	// Get the SPU and job numbers using replacement values for the job manager.
	U32 spuNum = g_spuNum;
	U32 jobNum = g_jobNum;
#endif

	ICE_ASSERT(spuNum < 6);

	U32 semaphorePbHolePtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x28);

	// If using ring buffers for mesh processing output, then finish them up.
	if (semaphorePbHolePtr != 0) {

		// Transfer the ring buffer allocator back to main memory.
		VU32 *pRingBufferAllocatorOutput = (VU32 *)MeshOutputAllocate(pStaticMem, 16);
		VU32 *pRingBufferAllocator = (VU32 *)&pStaticMem[kStaticRingBufferStartPtr];
		*pRingBufferAllocatorOutput = *pRingBufferAllocator;
		U32 ringBufferAllocatorPtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x10) + spuNum * 0x10;
		MeshKickOutputBufferToAddr(pStaticMem, ringBufferAllocatorPtr);

		// Generate the push buffer commands that will write to a semaphore once the data created for this
		// vertex set is completely consumed by the GPU.
		U32 *pSemaphorePbHole = (U32 *)MeshOutputAllocate(pStaticMem, 16);
		pSemaphorePbHole[0] = 0x00041D6C;
		pSemaphorePbHole[1] = pStaticMem[kStaticRingBufferSemaphorePtr] & 0x00000FF0;
		pSemaphorePbHole[2] = 0x00041D74;
		pSemaphorePbHole[3] = pStaticMem[kStaticRingBufferFreeStartPtr];
		MeshKickOutputBufferToAddr(pStaticMem, semaphorePbHolePtr);
	}

	// Get the address of the GPU sync mutex.
	U32 gpuSyncMutexPtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x18);
	ICE_ASSERT((gpuSyncMutexPtr & 0x7F) == 0);

	// Allocate memory in the output buffer for the GPU sync mutex, the GPU put pointer value, and the
	// completed bits for this SPU.
	U32 *pSyncOutput = (U32 *)MeshOutputAllocate(pStaticMem, 160, 128);
	VU8 *pNewJobCompletionBits = (VU8 *)((U32)pSyncOutput + 144);

	VU8 jobsComplete;
	U8 *pJobsComplete = (U8 *)&jobsComplete;
	U32 shift;
#ifdef __SPU__
	volatile U32 *pLsMutex = &pStaticMem[kStaticAtomic];
#else
	U32 *pLsMutex = (U32 *)gpuSyncMutexPtr;
#endif
	ICE_ASSERT(((U32)pLsMutex & 0x7F) == 0);

	bool updatePutPtr = false;
	bool reloadJobCompletion = false;
	bool lastJob;

	do
	{
		U32 base;

		do
		{
#ifdef __SPU__
			if (reloadJobCompletion)
			{
				// Should probably wait here for some time...

				// Read in the job completion bits.
				StartDma((void *)&pStaticMem[kStaticJobCompletion], gpuSyncMutexPtr + 128, 96,
						pStaticMem[kStaticWorkBufferTagId], kDmaGet);
			}
#else
			// The job completion bits can not make forward progress on the PPU, so reloading them is an error.
			ICE_ASSERT(!reloadJobCompletion);
#endif

			// Get the job completion bits and calculate the necessary shift value for this job.
			jobsComplete = GetCompletedJobs(pStaticMem);
			base = ((U32 *)&jobsComplete)[0];
			shift = jobNum - base;

			// If current job is 96 beyond what has been completed, thus we have no place to write the
			// completion bit so we need to stall until this is no longer the case.
			reloadJobCompletion = shift >= 96;

		} while (reloadJobCompletion);

		// Determine if all previous jobs have finished.
		U8 mask = 0xFF << (8 - shift);
		bool prevJobsComplete = (shift < 8) && ((pJobsComplete[4] & mask) == mask);

#ifdef __SPU__
		bool tooLongSinceUpdate;
		bool locked;

		do
		{
			// Get the GPU sync mutex into LS.
			Getllar((U32)pLsMutex, gpuSyncMutexPtr);

			// Read values set in the mutex.
			volatile U32 lock = pLsMutex[0];
			volatile U32 lastJobToPut = pLsMutex[4];
			volatile U32 totalJobCount = pLsMutex[12];
			ICE_ASSERT(jobNum < totalJobCount);

			// Determine various conditions.
			locked = lock != 0;
			tooLongSinceUpdate = jobNum >= lastJobToPut + 64;   // This number may need to be tweaked.
			lastJob = jobNum == totalJobCount - 1;

			// Try to lock the mutex so the GPU put pointer can be updated.
			if ((lock == 0) && prevJobsComplete)
			{
				pLsMutex[0] = ~0;
				updatePutPtr = !Putllc((U32)pLsMutex, gpuSyncMutexPtr);
			}

			// Reload the GPU sync mutex if the following conditions are true.
		} while (prevJobsComplete && ((!updatePutPtr && !locked) || (lastJob && locked)));

		// This job didn't lock the mutex for some reason, but we really want this job to update the GPU put pointer,
		// so we will try again.
		reloadJobCompletion = (locked && lastJob) || (!locked && !updatePutPtr && (tooLongSinceUpdate || lastJob));
#else
		// Read values set in the mutex.
		U32 lock = pLsMutex[0];
		U32 totalJobCount = pLsMutex[12];
		ICE_ASSERT(jobNum < totalJobCount);

		// Determine is this is the last job.
		lastJob = jobNum == totalJobCount - 1;

		// The mutex is not allowed to be locked on the PPU.
		ICE_ASSERT(lock == 0);

		// If this is the last job, then all previous jobs must be finished.
		ICE_ASSERT(!lastJob || prevJobsComplete);

		// If all previous jobs are complete, then update the GPU put pointer.
		updatePutPtr = prevJobsComplete;
#endif
	} while (reloadJobCompletion);

	// Set the correct completion bit and save the result in the output buffer.
	pJobsComplete[(shift >> 3) + 4] |= 0x80U >> (shift & 0x7);
	*pNewJobCompletionBits = jobsComplete;

#ifdef __SPU__
	// Make sure all DMAs have been completed before updating the completion bits.
	spu_writech(MFC_Cmd, Ice::kDmaBarrier);

	// Get the correct DMA tag ID to use.
	U32 bufNum = pStaticMem[kStaticOutputBufferNum];
	U32 dmaTagId = pStaticMem[kStaticOutputBufferPtr + (bufNum >> 2)] >> 24;

	// Put the completion bits for this SPU back into main memory for all to see.
	StartDma(pNewJobCompletionBits, gpuSyncMutexPtr + spuNum * 16 + 128, 16, dmaTagId, kDmaPut);
#else
	*(VU8 *)(gpuSyncMutexPtr + spuNum * 16 + 128) = *pNewJobCompletionBits;
#endif

	if (updatePutPtr) {
		// Determine if the put pointer value should be the one in the vertex set or the last one.
		U32 newPutPtrValue = lastJob ? pLsMutex[13] : *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x1C);

		// Copy the data stored in the mutex to the output buffer.
		for (U32 ii = 0; ii < 32; ii++)
		{
			pSyncOutput[ii] = pLsMutex[ii];
		}

		// Clear the lock in the mutex.
		pSyncOutput[0] = 0;

		// Set the last job in the mutex to the job number;
		pSyncOutput[4] = jobNum;

		// Place the new put pointer value in the first field of the quadword to be output to the GPU register.
		pSyncOutput[32] = newPutPtrValue;

		// Get the pointer to the GPU put pointer.
		U32 putPtrPtr = pLsMutex[8];
		ICE_ASSERT((putPtrPtr & 0xF) == 0);

#ifdef __SPU__
		// Update the GPU put pointer.
		StartDma((void *)(U32(pSyncOutput) + 128), putPtrPtr, 4, dmaTagId, kDmaPut);

		// Write the GPU sync mutex back to main memory.
		Putqlluc((U32)pSyncOutput, gpuSyncMutexPtr, dmaTagId);
#else
		// Update the GPU put pointer.
		U32 *pPutPtr = (U32 *)putPtrPtr;
		*pPutPtr = newPutPtrValue;

		// Update the last job number in the mutex.
		pLsMutex[4] = jobNum;
#endif
	}

#ifdef __SPU__
	// Make sure all subsequent jobs see the correct data.
	spu_writech(MFC_Cmd, Ice::kDmaBarrier);
#endif
}

void Ice::MeshProc::CmdCleanupAndExit(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// We need a reindex table initialized to dummy values, if we
	// don't have one already.
	CreateReindexTable(pStaticMem, true);

	// Patch pixel shader into output buffer.
	if (pStaticMem[kStaticPixelShaderPtr] != 0)
	{
		PatchPixelShader(pStaticMem);
		U32 *pOutputShader = (U32 *)MeshOutputAllocate(pStaticMem,
			pStaticMem[kStaticPixelShaderSize], 16, &pStaticMem[kStaticPixelShaderOutputPtr]);
		CopyQWords(pOutputShader, (U32 *)pStaticMem[kStaticPixelShaderPtr], pStaticMem[kStaticPixelShaderSize]);
	}

#if ICE_MESH_STENCIL_SHADOWS_ON
	const U8 dataTypeSizes[kNumTypes] =
		{
			4, // kF32,
			2, // kF16,
			2, // kI16n,
			2, // kI16,
			4, // kX11Y11Z10n,
			1, // kU8n,
			1, // kU8,
		};

	const U8 dataAttribType[kNumTypes] =
		{
			0x02, // kF32,
			0x03, // kF16,
			0x01, // kI16n,
			0x05, // kI16,
			0x06, // kX11Y11Z10n,
			0x04, // kU8n,
			0x07, // kU8,
		};

	// If we generated any shadow cap indexes, we need to output a
	// position-only NV stream of the entire object.
	if (pStaticMem[kStaticShadowCapVertexOutputPtr] != 0)
	{
		U8 outputVertexComponentCount = 3;
		U8 outputVertexFormat = pStaticMem[kStaticShadowCapVertexFormat];
		U8 outputVertexStride = outputVertexComponentCount*dataTypeSizes[outputVertexFormat];
		U32 numVerts = pStaticMem[kStaticOutputVertexCount];
		U8 *pOutputCapGeometry = (U8*)MeshOutputAllocate(
			pStaticMem,
			((numVerts + 4) * outputVertexStride + 0xF) & ~0xF,
			16,
			&pStaticMem[kStaticShadowCapVertexOutputPtr]);

#ifndef __SPU__
		// Collect some stats on what is being output.
		g_meshProcessingStats.m_capsNum += numVerts;
		g_meshProcessingStats.m_capsSize += ((numVerts + 4) * outputVertexStride + 0xF) & ~0xF;
#endif

		F32 *pPositions = (F32 *)pStaticMem[kStaticUniformPosPtr];

		// We need a full-sized dummy reindex table for this insertion.
		ICE_ASSERT(pStaticMem[kStaticReindexPtr] != 0);
		if (pStaticMem[kStaticReindexIsDummy] == 0)
			InitializeReindexToDummy(pStaticMem);
		pStaticMem[kStaticReindexCount] = numVerts;

		// Scale and biases for output shadow positions.
		// These may not be correct, but the data is not readily available to do this properly.
		IceQuadWord scale;
		IceQuadWord bias;
		scale.m_data.f32.A = 1.0f;
		scale.m_data.f32.B = 1.0f;
		scale.m_data.f32.C = 1.0f;
		scale.m_data.f32.D = 1.0f;
		bias.m_data.f32.A = 0.0f;
		bias.m_data.f32.B = 0.0f;
		bias.m_data.f32.C = 0.0f;
		bias.m_data.f32.D = 0.0f;

		InsertAttributeIntoNvStream(pPositions, pOutputCapGeometry, outputVertexStride,
			outputVertexComponentCount, outputVertexFormat, scale, bias,
			(U16 *)pStaticMem[kStaticReindexPtr], (numVerts + 3) & ~0x3);
	}
#endif // ICE_MESH_STENCIL_SHADOWS_ON

 	// Kick whatever is in the output buffer before we patch any holes.
 	MeshKickOutputBuffer(pStaticMem);

	// Get the address of the GPU sync mutex.
	U32 gpuSyncMutexPtr = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x18);

#ifdef __SPU__
	// If the GPU sync mutex pointer is not 0, then we need to load the completion bits needed by the sync technique.
	if (gpuSyncMutexPtr != 0)
	{
		// Read in the job completion bits.
		StartDma((void *)&pStaticMem[kStaticJobCompletion], gpuSyncMutexPtr + 128, 96,
				pStaticMem[kStaticWorkBufferTagId], kDmaGet);
	}
#endif



	U8 *pRegToAttribMap = (U8 *)pStaticMem[kStaticNvControlPtr];
	U32 indexCount = pStaticMem[kStaticIndexCount] - pStaticMem[kStaticHaloIndexCount];

	// Fill holes in push buffer!
	{
		U32 holeSize = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x24);
		U32 *pHoleStart = (U32*)MeshOutputAllocate(pStaticMem, holeSize);

#ifndef __SPU__
		// Collect some stats on what is being output.
		g_meshProcessingStats.m_holeSize += holeSize;
#endif

		U32 *pHole = pHoleStart;
		U32 *pHoleEnd = pHoleStart + (holeSize/4);

		// Write fragment program address, if it exists
		if (pStaticMem[kStaticPixelShaderPtr] != 0)
		{
			*pHole++ = NV_COMMAND(0x08E4, 4); // Fragment Program Address
			*pHole++ = pStaticMem[kStaticPixelShaderOutputPtr];
		}

		// Write vertex attribute addresses
		for (U32 i = 0; i < 8; ++i)
		{
			U16 *pFixedFormatPtr = (U16 *)pStaticMem[kStaticFixedFormatPtr];
			U16 *pFormat = (U16 *)((U32)pFixedFormatPtr + pFixedFormatPtr[i + 1]);
			if (pFormat != NULL)
			{
				U32 const streamBaseAddr = pStaticMem[kStaticStreamOutputPtr + i];
				if(streamBaseAddr != 0)
				{
					U32 const attribCount = pFormat[0];
					U8 *pAttribInfo = (U8*)(pFormat+2);
					for (U32 j = 0; j < attribCount; ++j)
					{
						U8 attribId = pAttribInfo[j*4+0];
						ICE_ASSERT(attribId > 0);
						U8 registerNum = 0;
						for(; registerNum<16; ++registerNum)
							if (pRegToAttribMap[registerNum] == attribId)
								break;

						// Not all attributes in output streams map to
						// hardware registers.  There's no need to
						// fill holes for those that don't.
						if (registerNum == 16)
							continue;

						U8 const attribOffset = pAttribInfo[j*4+3];

						*pHole++ = NV_COMMAND(0x1680 + 4*registerNum, 4); // VertexAttributeArrayAddress
						*pHole++ = streamBaseAddr + attribOffset;
					}
				}
			}
		}

		// Write draw calls, padding to the end of the hole with no-ops
		if (pStaticMem[kStaticOutputGenerateDrawCalls] != 0)
		{
			U32 indexBufferAddr = pStaticMem[kStaticIndexOutputPtr];
			pHole = GenerateDrawCalls(pHole, indexBufferAddr, indexCount, 0x5);
		}

		// Fill hole with no-ops
		pHole = FillHoleWithNoOps(pHole, pHoleEnd);

		// Kick hole
		U32 *pHoleDest = (U32 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x20));
		MeshKickOutputBufferToAddr(pStaticMem, (U32)pHoleDest);
	}

#if ICE_MESH_STENCIL_SHADOWS_ON
	// patch shadow holes
	U32 numLights = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x2C);
	U32 vertexType = dataAttribType[pStaticMem[kStaticShadowCapVertexFormat]];
	ICE_ASSERT(numLights <= kMaxLightCount);
	for (U32 i = 0; i < numLights; ++i)
	{
		// Fill in shadow sides hole.  This involves calls to
		// DrawArrays instead of DrawIndexes, so we can't use
		// GenerateDrawCalls(), even though the code is very similar.
		{
			U32 holeSize = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0x4);
			U32 *pHoleStart = (U32*)MeshOutputAllocate(pStaticMem, holeSize);

#ifndef __SPU__
			// Collect some stats on what is being output.
			g_meshProcessingStats.m_holeSize += holeSize;
#endif

			U32 *pHoleEnd = pHoleStart + (holeSize / 4);
			U32 *pHole = pHoleStart;

			U8 isPointLight = pStaticMem[kStaticShadowProfileEdgeVertexCount + i] >> 16;
			U32 vertexCount = pStaticMem[kStaticShadowProfileEdgeVertexCount + i] & 0xFFFF;
			if (vertexCount > 0)
			{
				U32 streamBaseAddr = pStaticMem[kStaticShadowProfileEdgeOutputPtr + i];

				*pHole++ = NV_COMMAND(0x1680 + 4*0, 4); // VertexAttributeArrayAddress
				*pHole++ = streamBaseAddr + 0;

				U32 components = 4;
				U32 stride = components*dataTypeSizes[pStaticMem[kStaticShadowCapVertexFormat]];
				*pHole++ = NV_COMMAND(0x1740 + 4*0, 4); // VertexAttribFormat
				*pHole++ = (stride << 8) | (components << 4) | vertexType;

				*pHole++ = NV_COMMAND(0x1808, 4);    // DrawMode
				*pHole++ = isPointLight ? 0x8 : 0x5; // = GL_QUADS or GL_TRIANGLES

				U32 batchCount   = vertexCount / 256;
				U8 remainder     = vertexCount % 256;
				U32 numDrawCommands = batchCount + (remainder != 0);
				if(numDrawCommands)
				{
					*pHole++ = NV_COMMAND(0x1814, numDrawCommands*4) | 0x40000000;          // DrawArrays
					U32 vertexOffset = 0;
					for (U32 j = 0; j < batchCount; ++j)
					{
						*pHole++ = ((256-1) << 24) | vertexOffset; // 256 at a time
						vertexOffset += 256;
					}
					if (remainder != 0)
					{
						*pHole++ = ((remainder-1) << 24) | vertexOffset; // remainder
					}
				}

				*pHole++ = NV_COMMAND(0x1808, 4); // DrawMode
				*pHole++ = 0x0;                   // Stop drawing
			}

			FillHoleWithNoOps(pHole, pHoleEnd);

			// kick hole (twice)
			U32 *pHoleDest = (U32 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0x0));
			MeshKickOutputBufferToAddr(pStaticMem, (U32)pHoleDest);
		}

		// Fill front/back cap holes
		U32 capHoleSize = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0xC);
		if (capHoleSize > 0)
		{
			U32 components = 3;
			U32 stride = components*dataTypeSizes[pStaticMem[kStaticShadowCapVertexFormat]];
   			U32 *pHoleStart = (U32 *)MeshOutputAllocate(pStaticMem, capHoleSize);

#ifndef __SPU__
			// Collect some stats on what is being output.
			g_meshProcessingStats.m_holeSize += capHoleSize;
#endif

			U32 *pHole = pHoleStart;
			U32 *pHoleEnd = pHoleStart + (capHoleSize/4);

			U32 capIndexCount    = pStaticMem[kStaticShadowCapIndexCount + i];
			if(capIndexCount > 0)
			{
				U32 streamBaseAddr = pStaticMem[kStaticShadowCapVertexOutputPtr];
				*pHole++ = NV_COMMAND(0x1680 + 4*0, 4); // VertexAttributeArrayAddress
				*pHole++ = streamBaseAddr + 0;

				*pHole++ = NV_COMMAND(0x1740 + 4*0, 4); // VertexAttribFormat
				*pHole++ = (stride << 8) | (components << 4) | vertexType;

				// Write draw calls
				U32 indexBufferAddr = pStaticMem[kStaticShadowCapIndexOutputPtr + i];
				pHole = GenerateDrawCalls(pHole, indexBufferAddr, capIndexCount, 0x5);
			}

			// Fill remainder of hole with no-ops
			pHole = FillHoleWithNoOps(pHole, pHoleEnd);

			U32 *pHoleDest = (U32 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0x8));
			U32 backCapHoleSize = *(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0x14);

			// kick back cap hole
			if(backCapHoleSize > 0)
			{
				// kick front cap hole
				MeshKickOutputBufferToAddr(pStaticMem, (U32)pHoleDest, 0xFFFFFFFF);

				// front and back cap holes should be the same size
				ICE_ASSERT(capHoleSize == backCapHoleSize);
				pHoleDest = (U32 *)(*(U32 *)(pStaticMem[kStaticNvControlPtr] + 0x30 + 0x18 * i + 0x10));
				MeshKickOutputBufferToAddr(pStaticMem, (U32)pHoleDest);
			}
			else {
				// kick front cap hole
				MeshKickOutputBufferToAddr(pStaticMem, (U32)pHoleDest);
			}
		}
	}
#endif // ICE_MESH_STENCIL_SHADOWS_ON

	// If the GPU sync mutex pointer is not 0, then we need to sync the GPU put pointer.
	if (gpuSyncMutexPtr != 0)
		SyncGpuPutPtr(pStaticMem);
}

void Ice::MeshProc::CmdStartInputListDma(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	U8 *pInputBuffer = (U8 *)pStaticMem[kStaticInputBufferPtr];
#ifdef __SPU__
	U32 inputBufferTag = pStaticMem[kStaticInputBufferTagId];
	StartDma(pInputBuffer + cmdQuad1.m_data.u16.D, (U32)(pInputBuffer + cmdQuad1.m_data.u16.B),
		cmdQuad1.m_data.u16.C, inputBufferTag, kDmaGetl);
#else
	MeshLoadInputBuffer((U32*)(pInputBuffer + cmdQuad1.m_data.u16.B),
		cmdQuad1.m_data.u16.C,
		pInputBuffer + cmdQuad1.m_data.u16.D);
#endif
}

void Ice::MeshProc::CmdStallOnInputDma(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
#ifdef __SPU__
	U32 inputBufferTag = pStaticMem[kStaticInputBufferTagId];
	WaitOnDma(1 << inputBufferTag);
#endif
	(void)pStaticMem; // Shut up warning.
}

void Ice::MeshProc::CmdEndSetup(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// If there were blend shapes, then a run table and a delta format info structure exist in the
	// work buffer.  These need to be freed.
	U32 indexTablePtr = pStaticMem[kStaticIndexTablePtr];
	if (indexTablePtr != 0)
		MeshMemFree(pStaticMem, indexTablePtr);

#ifdef __SPU__
	WwsJob_JobApiFreeLogicalBuffer(kMeshInputBufferSetNum, 0);
	WwsJob_JobApiLoadNextJob();
#endif
}

#ifdef __SPU__
#if 0
// Useful for dumping all of SPU memory to someplace in main memory.
// Just remove the #if 0 to make it work.
void DumpMemory(U32 pMainMem)
{
	U32 size = 16384;
	U8  *pLs = (U8 *)0;
	for (U32 ii = 0; ii < 16; ii++) {
		StartDma(pLs, pMainMem, size, 0, kDmaPut);
		pLs += size;
		pMainMem += size;
	}
	WaitOnDma(1);
}
#endif
#endif

