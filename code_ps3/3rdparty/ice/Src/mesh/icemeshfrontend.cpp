/*
* Copyright (c) 2003-2005 Naughty Dog, Inc.
* A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
* Use and distribution without consent strictly prohibited
*/

#include <alloca.h>
#include "icemeshfrontend.h"
#include "icemesh.h"
#include "icerender.h"
#include "icememcopy.h"

#include "jobapi/joblist.h"
#include "jobapi/spumodule.h"
#include "jobapi/commandlistbuilder.h"

using namespace Ice;
using namespace Ice::Mesh;
using namespace Ice::MeshProc;

// Set this to 1 to enable test of list DMA size.  If list DMA is too large, the job is not added to the job list and
// a warning is printed.
#define TEST_DMA_SIZE 1

namespace Ice
{
	namespace Mesh
	{
		// Global structure containing an object info structure for single sided and double sided objects.
		// This structure peforms double duty as it is also used as the source for the filling between
		// the unaligned and aligned data elements in the mesh processing input buffer.
		static ICE_ALIGN(128) MeshProc::ObjectInfo const s_objectInfo[8] =
		{
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		};

		enum
		{
			kSingleSidedObjectInfoNum = 0,
			kDoubleSidedObjectInfoNum = 1
		};
	}
}

static inline void TouchData(U64 addra, U64 addrb)
{
	__asm__ volatile ("dcbt %0,%1" :: "r"(addra),"r"(addrb));
}

static inline void TouchDataForStore(U64 addra, U64 addrb)
{
	__asm__ volatile ("dcbtst %0,%1" :: "r"(addra),"r"(addrb));
}

// Setup the Attribute Array Formats for the inputs of the vertex program.
void Ice::Mesh::SetupArrayFormats(
	AttributeArrayFormatInfo const * attribFormatInfo,
	U8 const attribCount,
	Render::CommandContextData * __restrict const commandContext)
{
	Render::InlineUnsafe::CommandContext * __restrict pCommandContext =
		static_cast<Render::InlineUnsafe::CommandContext *>(commandContext);

	// Initialize all attributes to be disabled.
	U32F attribArrayEnabled = 0;

	// Reserve 2 words in the push buffer for each possible attribute.
	bool pbAvailable = pCommandContext->Reserve(32);
	ICE_ASSERT(pbAvailable);

	// Enable all specifed attribute arrays.
	for (U32F iAttrib = 0; iAttrib < attribCount; ++iAttrib, ++attribFormatInfo)
	{
		U32 index = attribFormatInfo->m_hardwareIndex;
		pCommandContext->SetVertexAttribFormat(index, (Render::AttribType)attribFormatInfo->m_attribType,
			(Render::AttribCount)attribFormatInfo->m_attribCount, attribFormatInfo->m_elementStride, 0);
		attribArrayEnabled |= 1 << index;
	}

	// Disable any unused attribute arrays.
	U32F mask = 1;
	for (U32F iAttrib = 0; iAttrib < 16; ++iAttrib, mask <<= 1)
	{
		if (!(attribArrayEnabled & mask))
		{
			pCommandContext->DisableVertexAttribArray(iAttrib);
		}
	}
}

// Setup the Attribute Array Formats for the inputs of the vertex program for stencil shadows.
void Ice::Mesh::SetupShadowArrayFormats(Render::CommandContextData * __restrict const commandContext)
{
	Render::InlineUnsafe::CommandContext * __restrict pCommandContext =
		static_cast<Render::InlineUnsafe::CommandContext *>(commandContext);

	// Reserve 2 words in the push buffer for each disabled attribute.
	bool pbAvailable = pCommandContext->Reserve(30);
	ICE_ASSERT(pbAvailable);

	// Disable any unused attribute arrays.
	for (U32F iAttrib = 1; iAttrib < 16; ++iAttrib)
	{
		pCommandContext->DisableVertexAttribArray(iAttrib);
	}
}


// Reserves memory for a built push buffer given a command context.
// Also aligns the push buffer to the next 16 byte alignment and returns the output address.
static inline U32 *ReserveMemoryForAndAlignPushBuffer(
	Render::CommandContextData * const commandContext,
	U64 const pbSize,
	U64 const holeSize = 0)
{
	// Attempt to allocate the needed memory in this push buffer context.
	bool pbAvailable = static_cast<Render::CommandContext*>(commandContext)->Reserve((pbSize + holeSize) / sizeof(U32) + 3);

	// If the allocation fails, return NULL.
	if (!pbAvailable)
	{
		return NULL;
	}

	// Align the push buffer up to the next 16 byte alignment and fill the empty space with 0s (NOPs).
	while ((U32(commandContext->m_cmdptr) & 0xF) != 0)
	{
		commandContext->m_cmdptr[0] = 0;
		commandContext->m_cmdptr++;
	}

	// Return the address where the push buffer can be placed.
	return commandContext->m_cmdptr;
}

// Outputs a built push buffer given a command context.  This assumes that the memory has already been reserved.
static inline void OutputPushBuffer(
	Render::CommandContextData * const commandContext,
	void const * const pbCmdStart,
	U64 const pbSize)
{
	// Copy the push buffer from the stack and increment the current push buffer pointer for this context.
	CopyQuadwords(commandContext->m_cmdptr, pbCmdStart, pbSize);
	commandContext->m_cmdptr += pbSize / sizeof(U32);
}

// Outputs a built push buffer given a command context.  This assumes that the memory has already been reserved.
static inline void OutputPushBufferWithHole(
	Render::CommandContextData * const commandContext,
	void const * const pbCmdStart,
	U64 const pbSize,
	U64 const holeSize,
	bool const placeHoleLast)
{
	ICE_ASSERT(holeSize > 0);
	ICE_ASSERT((holeSize & 0xF) == 0);

	// Calculate where the push buffer is to go given if the push buffer hole is to be placed before or after.
	// Also, at the beginning of the hole insert a NOP command that will skip all of the data in the part of
	// the push buffer that is being created here.  Normally this NOP command will not be executed as the hole
	// should be filled in by mesh processing, but if mesh processing fails for for some reason (for instance, if
	// the output buffer is full), then mesh processing won't fill the hole and instead all draw commands will be
	// skipped.
	U32 * __restrict pPushBuffer;
	if (placeHoleLast)
	{
		pPushBuffer = commandContext->m_cmdptr;
		U32 * __restrict pNop = commandContext->m_cmdptr + pbSize / sizeof(U32);
		pNop[0] = 0x40000100 + ((holeSize - 4) << 16);
	}
	else
	{
		pPushBuffer = commandContext->m_cmdptr + holeSize / sizeof(U32);
		U32 * __restrict pNop = commandContext->m_cmdptr;
		pNop[0] = 0x40000100 + ((holeSize + pbSize - 4) << 16);
	}

	// Copy the push buffer from the stack and increment the current push buffer pointer for this context.
	switch(pbSize>>4)
	{
		default:
			CopyQuadwords(pPushBuffer, pbCmdStart, pbSize);
			break;
		case 8: *(((VU8*)pPushBuffer)+7) = *(((VU8*)pbCmdStart)+7);
		case 7: *(((VU8*)pPushBuffer)+6) = *(((VU8*)pbCmdStart)+6);
		case 6: *(((VU8*)pPushBuffer)+5) = *(((VU8*)pbCmdStart)+5);
		case 5: *(((VU8*)pPushBuffer)+4) = *(((VU8*)pbCmdStart)+4);
		case 4: *(((VU8*)pPushBuffer)+3) = *(((VU8*)pbCmdStart)+3);
		case 3: *(((VU8*)pPushBuffer)+2) = *(((VU8*)pbCmdStart)+2);
		case 2: *(((VU8*)pPushBuffer)+1) = *(((VU8*)pbCmdStart)+1);
		case 1: *(((VU8*)pPushBuffer)+0) = *(((VU8*)pbCmdStart)+0);
		case 0: ;
	}
	commandContext->m_cmdptr += (pbSize + holeSize) / sizeof(U32);
}

// Reserve memory for the job data given a job data allocator and return the output address.
static inline U8 *ReserveMemoryForJobData(
	JobDataAllocator * const jobDataAllocator,
	U64 const jobDataSize)
{
	ICE_ASSERT(jobDataAllocator != NULL);

	// Allocate the needed memory for the job data.
	bool jobDataMemAvailable = jobDataAllocator->Reserve(jobDataSize);

	// Return the address where the job data can be placed, if memory is available.  Otherwise return NULL.
	return jobDataMemAvailable ? jobDataAllocator->m_pCurrent : NULL;
}

// Outputs the job data given a job data allocator.  This assumes that the memory has already been reserved.
static inline void OutputJobData(
	JobDataAllocator * const jobDataAllocator,
	void const * const jobDataStart,
	U64 const jobDataSize)
{
	ICE_ASSERT(jobDataAllocator != NULL);

	// Copy the job data from the stack and increment the job data context current pointer.
	CopyQuadwords(jobDataAllocator->m_pCurrent, jobDataStart, jobDataSize);
	jobDataAllocator->m_pCurrent += jobDataSize;
}



static DmaTag *GenerateBlendShapeData(
	VertexSetBlendShapes const * const shapes,
	U32 const streamUsageBits,
	U32F const lodIndex,
	I16 const *blendShapeFactors,
	JobDataAllocator * __restrict const jobDataAllocator)
{
	ICE_ASSERT(jobDataAllocator != NULL);

	// Count the actual shapes used.
	U32F totalShapeCount = shapes->m_shapeCount;
	U32F shapeCount = 0;
	for (U32F iShape = 0; iShape < totalShapeCount; ++iShape)
	{
		shapeCount += (blendShapeFactors[shapes->m_shapeRemapTable[iShape]] != 0) ? 1 : 0;
	}

	// Get the stream mask and stream count.
	U32 usedStreamMask = streamUsageBits & 0xFFFF;
	U32 usedStreamCount = streamUsageBits >> 16;

	// Calculate the total number of streams to be uploaded.
	U32F streamCount = shapeCount * usedStreamCount;

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));
	U64 allocSize = (streamCount > 0) ? streamCount * 0x50 : 0x50;
	U8 * __restrict jobData = (U8 *)alloca(allocSize); // Maximum size possible is 0x1F40.

	union
	{
		U16 * __restrict cmd;
		DmaTag * __restrict tag;
	} d;
	d.cmd = (U16 * __restrict)jobData;

	// Reserve memory for the blend shape data and get its output address.
	U8 *jobDataOutput = ReserveMemoryForJobData(jobDataAllocator, allocSize);

	// If the job data could not be allocated, then return NULL.
	if (jobDataOutput == NULL)
	{
		return NULL;
	}

	DmaTag *tag0 = (DmaTag *)(jobDataOutput + 0x20);
	U32 cmdBase = U32(jobDataOutput);
	U32 tagBase = U32(jobDataOutput) + 0x20;

	// Return if no streams are to be uploaded.
	if (streamCount == 0)
	{
		d.cmd[0x0] = (kCmdReturn << 8);
		d.cmd[0x1] = 0;
		d.cmd[0x2] = 0;
		d.cmd[0x3] = 0;
		d.cmd[0x4] = 0;
		d.cmd[0x5] = 0;
		d.cmd[0x6] = 0;
		d.cmd[0x7] = 0;
		d.cmd[0x8] = 0;
		d.cmd[0x9] = 0;
		d.cmd[0xA] = 0;
		d.cmd[0xB] = 0;
		d.cmd[0xC] = 0;
		d.cmd[0xD] = 0;
		d.cmd[0xE] = 0;
		d.cmd[0xF] = 0;
		d.cmd += 0x10;

		d.tag[0x0].m_size = 0x20;
		d.tag[0x0].m_source = cmdBase;
		d.tag[0x1].m_size = 0;
		d.tag[0x1].m_source = 0;
		d.tag[0x2].m_size = 0;
		d.tag[0x2].m_source = 0;
		d.tag[0x3].m_size = 0;
		d.tag[0x3].m_source = 0;
		d.tag[0x4].m_size = 0;
		d.tag[0x4].m_source = 0;
		d.tag[0x5].m_size = 0;
		d.tag[0x5].m_source = 0;
		d.tag += 0x6;
	}

	U16 base = 0x6000;
	U16 other = 0x0000;
	U32F shape = 0;

	for (U32F iShape = 0; iShape < shapeCount; ++iShape)
	{
		// Find the next shape with a non-zero blend shape factor.
		while (blendShapeFactors[shapes->m_shapeRemapTable[shape]] == 0)
		{
			shape++;
		}

		// Get information about this shape.
		BlendShapeInfo const &info = shapes->m_blendShapeInfos[shape];
		U64 runCount = info.m_lodTables[lodIndex].m_runCount;
		U64 runLength = (runCount * 2 + 0xF) & ~0xF;
		U64 formatLength = (info.m_streamFormatInfo->m_length + 0xF) & ~0xF;
		U64 deltaCount = info.m_lodTables[lodIndex].m_deltaCount;

		U32F stream = 0;

		for (U32F iStream = 0; iStream < usedStreamCount; ++iStream)
		{
			// Find the next used stream.
			while (!(usedStreamMask & (1 << stream)))
			{
				stream++;
			}

			// Get information about this stream.
			U32 streamLength = (((deltaCount * info.m_streamBitCounts[stream] + 7) >> 3) + 0xF) & ~0xF;

			bool newShape = iStream == 0;
			bool firstStream = (iShape == 0) && (iStream == 0);
			bool lastStream = (iShape == shapeCount - 1) && (iStream == usedStreamCount - 1);

			if (firstStream)
			{
				if (lastStream)
				{
					//printf("Initial -- Last\n");
					d.cmd[0x0] = (kCmdSetupRunTable << 8);
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = runCount;
					d.cmd[0x3] = (kCmdSetupDeltaFormatInfo << 8);
					d.cmd[0x4] = base + 0x20 + runLength;
					d.cmd[0x5] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0x6] = base + 0x20 + runLength + formatLength;
					d.cmd[0x7] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0x8] = (kCmdReturn << 8);
					d.cmd[0x9] = 0;
					d.cmd[0xA] = 0;
					d.cmd[0xB] = 0;
					d.cmd[0xC] = 0;
					d.cmd[0xD] = 0;
					d.cmd[0xE] = 0;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = runLength;
					d.tag[0x1].m_source = U32(info.m_runList);
					d.tag[0x2].m_size = formatLength;
					d.tag[0x2].m_source = U32(info.m_streamFormatInfo);
					d.tag[0x3].m_size = streamLength;
					d.tag[0x3].m_source = U32(info.m_streams[stream]);
					d.tag[0x4].m_size = 0;
					d.tag[0x4].m_source = 0;
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;
				}
				else
				{
					//printf("Initial\n");
					d.cmd[0x0] = (kCmdStartInputListDma << 8);
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = 0x28;
					d.cmd[0x3] = other;
					d.cmd[0x4] = (kCmdSetupRunTable << 8);
					d.cmd[0x5] = base + 0x50;
					d.cmd[0x6] = runCount;
					d.cmd[0x7] = (kCmdSetupDeltaFormatInfo << 8);
					d.cmd[0x8] = base + 0x50 + runLength;
					d.cmd[0x9] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0xA] = base + 0x50 + runLength + formatLength;
					d.cmd[0xB] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0xC] = (kCmdStallOnInputDma << 8);
					d.cmd[0xD] = (kCmdJump << 8);
					d.cmd[0xE] = other;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					tagBase += 0x50;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = 0x30;
					d.tag[0x1].m_source = tagBase;
					d.tag[0x2].m_size = runLength;
					d.tag[0x2].m_source = U32(info.m_runList);
					d.tag[0x3].m_size = formatLength;
					d.tag[0x3].m_source = U32(info.m_streamFormatInfo);
					d.tag[0x4].m_size = streamLength;
					d.tag[0x4].m_source = U32(info.m_streams[stream]);
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;

					cmdBase += 0x50;
				}
			}
			else if (lastStream)
			{
				if (newShape)
				{
					//printf("Last New Shape: %i Stream: %i\n", shape, stream);
					d.cmd[0x0] = (kCmdSetupRunTable << 8);
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = runCount;
					d.cmd[0x3] = (kCmdSetupDeltaFormatInfo << 8);
					d.cmd[0x4] = base + 0x20 + runLength;
					d.cmd[0x5] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0x6] = base + 0x20 + runLength + formatLength;
					d.cmd[0x7] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0x8] = (kCmdReturn << 8);
					d.cmd[0x9] = 0;
					d.cmd[0xA] = 0;
					d.cmd[0xB] = 0;
					d.cmd[0xC] = 0;
					d.cmd[0xD] = 0;
					d.cmd[0xE] = 0;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = runLength;
					d.tag[0x1].m_source = U32(info.m_runList);
					d.tag[0x2].m_size = formatLength;
					d.tag[0x2].m_source = U32(info.m_streamFormatInfo);
					d.tag[0x3].m_size = streamLength;
					d.tag[0x3].m_source = U32(info.m_streams[stream]);
					d.tag[0x4].m_size = 0;
					d.tag[0x4].m_source = 0;
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;
				}
				else
				{
					//printf("Last Shape: %i Stream: %i\n", shape, stream);
					d.cmd[0x0] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0x3] = (kCmdReturn << 8);
					d.cmd[0x4] = 0;
					d.cmd[0x5] = 0;
					d.cmd[0x6] = 0;
					d.cmd[0x7] = 0;
					d.cmd[0x8] = 0;
					d.cmd[0x9] = 0;
					d.cmd[0xA] = 0;
					d.cmd[0xB] = 0;
					d.cmd[0xC] = 0;
					d.cmd[0xD] = 0;
					d.cmd[0xE] = 0;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = streamLength;
					d.tag[0x1].m_source = U32(info.m_streams[stream]);
					d.tag[0x2].m_size = 0;
					d.tag[0x2].m_source = 0;
					d.tag[0x3].m_size = 0;
					d.tag[0x3].m_source = 0;
					d.tag[0x4].m_size = 0;
					d.tag[0x4].m_source = 0;
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;
				}
			}
			else
			{
				if (newShape)
				{
					//printf("Next New Shape: %i Stream: %i\n", shape, stream);
					d.cmd[0x0] = (kCmdStartInputListDma << 8);
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = 0x28;
					d.cmd[0x3] = other;
					d.cmd[0x4] = (kCmdSetupRunTable << 8);
					d.cmd[0x5] = base + 0x50;
					d.cmd[0x6] = runCount;
					d.cmd[0x7] = (kCmdSetupDeltaFormatInfo << 8);
					d.cmd[0x8] = base + 0x50 + runLength;
					d.cmd[0x9] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0xA] = base + 0x50 + runLength + formatLength;
					d.cmd[0xB] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0xC] = (kCmdStallOnInputDma << 8);
					d.cmd[0xD] = (kCmdJump << 8);
					d.cmd[0xE] = other;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					tagBase += 0x50;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = 0x30;
					d.tag[0x1].m_source = tagBase;
					d.tag[0x2].m_size = runLength;
					d.tag[0x2].m_source = U32(info.m_runList);
					d.tag[0x3].m_size = formatLength;
					d.tag[0x3].m_source = U32(info.m_streamFormatInfo);
					d.tag[0x4].m_size = streamLength;
					d.tag[0x4].m_source = U32(info.m_streams[stream]);
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;

					cmdBase += 0x50;
				}
				else
				{
					//printf("Next Shape: %i Stream: %i\n", shape, stream);
					d.cmd[0x0] = (kCmdStartInputListDma << 8);
					d.cmd[0x1] = base + 0x20;
					d.cmd[0x2] = 0x28;
					d.cmd[0x3] = other;
					d.cmd[0x4] = (kCmdBlendDeltaStream << 8) | stream;
					d.cmd[0x5] = base + 0x50;
					d.cmd[0x6] = blendShapeFactors[shapes->m_shapeRemapTable[shape]];
					d.cmd[0x7] = (kCmdStallOnInputDma << 8);
					d.cmd[0x8] = (kCmdJump << 8);
					d.cmd[0x9] = other;
					d.cmd[0xA] = 0;
					d.cmd[0xB] = 0;
					d.cmd[0xC] = 0;
					d.cmd[0xD] = 0;
					d.cmd[0xE] = 0;
					d.cmd[0xF] = 0;
					d.cmd += 0x10;

					tagBase += 0x50;

					d.tag[0x0].m_size = 0x20;
					d.tag[0x0].m_source = cmdBase;
					d.tag[0x1].m_size = 0x30;
					d.tag[0x1].m_source = tagBase;
					d.tag[0x2].m_size = streamLength;
					d.tag[0x2].m_source = U32(info.m_streams[stream]);
					d.tag[0x3].m_size = 0;
					d.tag[0x3].m_source = 0;
					d.tag[0x4].m_size = 0;
					d.tag[0x4].m_source = 0;
					d.tag[0x5].m_size = 0;
					d.tag[0x5].m_source = 0;
					d.tag += 0x6;

					cmdBase += 0x50;
				}
			}
			base ^= 0x6000;
			other ^= 0x6000;
			stream++;
		}
		shape++;
	}

	// Calculates the size of the data that needs to be output and output it.
	U32 jobDataSize = U32(d.cmd) - U32(jobData);
	OutputJobData(jobDataAllocator, jobData, jobDataSize);

	return tag0;
}



// Places the matrices described by the given matrix runs into the command context.
static inline void AddMatrixRunsToCommandContext(
	Render::CommandContextData * const commandContext,
	U32F const matrixRunsCount,
	MatrixRun const * pMatrixRun,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	U32 const gpuSkinningConstantIndex
	)
{
	Render::InlineUnsafe::CommandContext * __restrict pCommandContext =
		static_cast<Render::InlineUnsafe::CommandContext *>(commandContext);

	// Determine the size of the matrices.
	U32 constIndex = gpuSkinningConstantIndex;
	if (matrixFormat == kFormatMatrix43)
	{
		// Transfer all of the matrix runs.
		for (U32F iRun = 0; iRun < matrixRunsCount; ++iRun, ++pMatrixRun)
		{
			F32 const * pData = pMatrices + pMatrixRun->m_firstIndex * 12;
			U32F matrixCount = pMatrixRun->m_length;

			// Reserve memory in the push buffer for the matrices.
			bool pbAvailable = pCommandContext->Reserve(14 * matrixCount);
			ICE_ASSERT(pbAvailable);

			// Put each matrix of the matrix run into the push buffer.
			for (U32F iMatrix = 0; iMatrix < matrixCount; ++iMatrix)
			{
				pCommandContext->SetVertexProgram3Constants(constIndex, pData);
				pData += 12;
				constIndex += 3;
			}
		}
	}
	else
	{
		ICE_ASSERT(matrixFormat == kFormatMatrix44);

		// Transfer all of the matrix runs.
		for (U32F iRun = 0; iRun < matrixRunsCount; ++iRun, ++pMatrixRun)
		{
			F32 const * pData = pMatrices + pMatrixRun->m_firstIndex * 16;
			U32F matrixCount = pMatrixRun->m_length;

			// Reserve memory in the push buffer for the matrices.
			bool pbAvailable = pCommandContext->Reserve(18 * matrixCount);
			ICE_ASSERT(pbAvailable);

			// Put each matrix of the matrix run into the push buffer.
			for (U32F iMatrix = 0; iMatrix < matrixCount; ++iMatrix)
			{
				pCommandContext->SetVertexProgram4Constants(constIndex, pData);
				pData += 16;
				constIndex += 4;
			}
		}
	}
}

// Adds static vertex attributes to a command context.  All attributes must be going to the GPU in this case.
static inline void AddStaticAttributePointersToCommandContext(
	Render::CommandContextData * const commandContext,
	Locator const * const pStreams,
	VertexInputInfo const * const pVertexInputInfo)
{
	// Reserve memory in the push buffer for all possible static attribute pointers and get the current push buffer pointer.
	bool pbAvailable = static_cast<Render::CommandContext*>(commandContext)->Reserve(32);
	ICE_ASSERT(pbAvailable);

	Render::InlineUnsafe::CommandContext * __restrict pCommandContext =
		static_cast<Render::InlineUnsafe::CommandContext *>(commandContext);

	for (U32F iAttrib = 0; iAttrib < 16; ++iAttrib)
	{
		U8 attribId = pVertexInputInfo->m_attribIdMap[iAttrib];
		if (attribId > 0)
		{
			ICE_ASSERT(attribId >= 0xF0);
			U32 attribOffsetWithContext = pStreams[attribId & 0x7].m_ofs + pVertexInputInfo->m_attribOffset[iAttrib];
			pCommandContext->SetVertexAttribPointer(iAttrib, attribOffsetWithContext, Render::AttribContext(0));
		}
	}
}



// Adds static vertex attributes and calculate the hole size required for the attributes that go through the SPUs.
static inline U64 AddStaticAttributePointersToPushBuffer(
	U32 * __restrict * const pPbCmd,
	Locator const * const pStreams,
	VertexInputInfo const * const pVertexInputInfo)
{
	U64 vertexPtrHoleSize = 0;

	for (U32F iAttrib = 0; iAttrib < 16; ++iAttrib)
	{
		U8 attribId = pVertexInputInfo->m_attribIdMap[iAttrib];
		if (attribId >= 0xF0)
		{
			(*pPbCmd)[0] = 0x00041680 + iAttrib * 4;
			(*pPbCmd)[1] = pStreams[attribId & 0x7].m_ofs + pVertexInputInfo->m_attribOffset[iAttrib];
			*pPbCmd += 2;
		}
		else if (attribId > 0)
			vertexPtrHoleSize += 8;
	}

	return vertexPtrHoleSize;
}

// Given a pointer to the indexes and the index count, adds draw commands to the push buffer.
static inline void AddDrawCommandsToPushBuffer(
	U32 * __restrict * const pPbCmd,
	U32 const indexesOffset,
	U32F indexCount)
{
	U32F firstCount = (0x200 - (indexesOffset & 0x01FF)) >> 1;

	// Clamp firstCount to be no greater than count
	I32F diff = indexCount - firstCount;
	firstCount += diff & (diff >> 31);

	// Subtract firstCount from count and clamp to zero
	indexCount -= firstCount;
	indexCount &= ~((I32) indexCount >> 31);
	U32F blockCount = indexCount >> 8;
	U32F remainCount = indexCount & 0x000000FF;
	U32F commandCount = blockCount + (remainCount != 0) + (firstCount != 0);
	ICE_ASSERT(commandCount < 0x200);

	(*pPbCmd)[0] = 0x0008181C;
	(*pPbCmd)[1] = indexesOffset & 0x7FFFFFFF;
	(*pPbCmd)[2] = 0x00000010 | (indexesOffset >> 31);
	(*pPbCmd)[3] = 0x00041808;
	(*pPbCmd)[4] = 0x00000005;
	(*pPbCmd)[5] = 0x40001824 | (commandCount << 18);
	*pPbCmd += 6;

	U32F start = firstCount;
	if (firstCount != 0)
	{
		(*pPbCmd)[0] = (firstCount - 1) << 24;
		*pPbCmd += 1;
	}

	if (blockCount != 0)
	{
		U32F value = 0xFF000000 | start;
		do
		{
			(*pPbCmd)[0] = value;
			*pPbCmd += 1;
			value += 0x100;
		} while (--blockCount);
		start = value & 0x00FFFFFF;
	}

	if (remainCount != 0)
	{
		(*pPbCmd)[0] = ((remainCount - 1) << 24) | start;
		*pPbCmd += 1;
	}

	(*pPbCmd)[0] = 0x00041808;
	(*pPbCmd)[1] = 0x00000000;
	*pPbCmd += 2;
}

// Aligns the push buffer to the next 16 bytes, adds space to the correct place if a ring buffer is being used and
// calculates the size of the push buffer hole.
static inline U64 FinishPushBufferAndGetHoleSize(
	U32 * __restrict * const pPbCmd,
	U64 const vertexPtrHoleSize,
	U64 const trimHoleSize,
	bool const performTrimming,
	bool const useRingBuffer)
{
	// If using a ring buffer and trimming is not performed, then 16 bytes of space must be made in the push buffer
	// in order to store the semaphore write required by the ring buffer.
	if (useRingBuffer && !performTrimming)
	{
		(*pPbCmd)[0] = 0;
		(*pPbCmd)[1] = 0;
		(*pPbCmd)[2] = 0;
		(*pPbCmd)[3] = 0;
		*pPbCmd += 4;
	}

	// Align the push buffer to the next 16 byte boundary.
	while ((U32(*pPbCmd) & 0xF) != 0)
	{
		(*pPbCmd)[0] = 0;
		*pPbCmd += 1;
	}

	// Calculate the hole size.
	U64 holeSize = (vertexPtrHoleSize + trimHoleSize + 0xF) & ~0xF;

	// If using a ring buffer and trimming is performed, then 16 bytes of space must be added to the hole
	// in order to store the semaphore write required by the ring buffer.
	if (useRingBuffer && performTrimming)
		holeSize += 16;

	return holeSize;
}



// Performace is better when filling in an entire cache line.
// This function assumes the input pointer is 16 byte aligned and then writes out zeros until it is 64 byte aligned.
static inline void *ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment(VU8 * __restrict pMem)
{
	// Make sure the pointer is 16 byte aligned.
	ICE_ASSERT((U32(pMem) & 0xF) == 0);

	// Write out zeros until it is 64 byte aligned.
	while ((U32(pMem) & 0x3F) != 0)
	{
		*pMem = VU8(0);
		pMem++;
	}

	return pMem;
}



// Add a new list DMA tag to the list DMA such that the source and destination DMA addresses are aligned within
// a 128 byte block.  This wastes some space in the input data, but makes the DMA transfer as efficent as possible.
static inline void WriteAlignedDmaTag(
	DmaTag ** const pDmaTags,
	U64 * const pDmaSize,
	void const * const pData,
	U64 const size)
{
	// If the data pointer is null, do not write a tag at all.
	if (pData == NULL)
		return;

	// Make sure everything is aligned properly.
	ICE_ASSERT((size & 0xF) == 0);
	ICE_ASSERT((U32(pData) & 0xF) == 0);
	ICE_ASSERT((*pDmaSize & 0xF) == 0);    // Was it misaligned from a previous tag?

	// Calculate the number of bytes the pointer is above a 128 byte boundary.
	U64 alignmentOffset = U32(pData) & 0x7F;

	// Calculate the number of extra bytes to be transferred in order to make the DMA source and destination
	// addresses match alignment within 128 byte blocks.
	U64 extraSize = (U32(pData) - *pDmaSize) & 0x7F;

	// Normally we just simply transfer the required number of extra bytes by adjusting the data pointer
	// back by the desired amount and increasing the transfer size by the same amount.
	// However, if the pointer adjustment takes us beyond a 128 byte boundary, then instead we add some
	// extra bytes to the previous transfer (which would take it to the end of a 128 byte boundary) and
	// only adjust the data pointer back to the 128 byte boundary.  This is done in case the 128 byte
	// boundary lies at the beginning of a memory page.  We do not wish to crash the system by reading
	// unmapped memory.
	U64 dmaOffset = extraSize;
	if (extraSize > alignmentOffset)
	{
		(*pDmaTags)[-1].m_size += extraSize - alignmentOffset;
		dmaOffset = alignmentOffset;
	}
	(*pDmaTags)[0].m_size = size + dmaOffset;
	(*pDmaTags)[0].m_source = U32(pData) - dmaOffset;

	// Increment the tag count and total DMA transfer size, thus far.
	*pDmaTags += 1;
	*pDmaSize += size + extraSize;
}

// Add a new list DMA tag to the list DMA such that the source and destination DMA addresses are aligned within
// a 128 byte block.  This wastes some space in the input data, but makes the DMA transfer as efficent as possible.
static inline void WriteAlignedDmaTags32K(
	DmaTag ** const pDmaTags,
	U64 * const pDmaSize,
	void const * const pData,
	U64 const size)
{
	// If the data pointer is null, do not write a tag at all.
	if (pData == NULL)
		return;

	// Make sure everything is aligned properly.
	ICE_ASSERT((size & 0xF) == 0);
	ICE_ASSERT((U32(pData) & 0xF) == 0);
	ICE_ASSERT((*pDmaSize & 0xF) == 0);    // Was it misaligned from a previous tag?

	// Calculate the number of bytes the pointer is above a 128 byte boundary.
	U64 alignmentOffset = U32(pData) & 0x7F;

	// Calculate the number of extra bytes to be transferred in order to make the DMA source and destination
	// addresses match alignment within 128 byte blocks.
	U64 extraSize = (U32(pData) - *pDmaSize) & 0x7F;

	// Normally we just simply transfer the required number of extra bytes by adjusting the data pointer
	// back by the desired amount and increasing the transfer size by the same amount.
	// However, if the pointer adjustment takes us beyond a 128 byte boundary, then instead we add some
	// extra bytes to the previous transfer (which would take it to the end of a 128 byte boundary) and
	// only adjust the data pointer back to the 128 byte boundary.  This is done in case the 128 byte
	// boundary lies at the beginning of a memory page.  We do not wish to crash the system by reading
	// unmapped memory.
	U64 dmaOffset = extraSize;
	if (extraSize > alignmentOffset)
	{
		(*pDmaTags)[-1].m_size += extraSize - alignmentOffset;
		dmaOffset = alignmentOffset;
	}

	// If the size of the tranfer is greater than 0x3F00 bytes (this is the maximum DMA size minus 256 bytes in
	// order to leave space for size adjustments for alignment), then use two tags for the transfer.
	// Otherwise use just one.
	if (size > 0x3F00)
	{
		(*pDmaTags)[0].m_size = 0x3F00 + dmaOffset;
		(*pDmaTags)[0].m_source = U32(pData) - dmaOffset;
		(*pDmaTags)[1].m_size = size - 0x3F00;
		(*pDmaTags)[1].m_source = U32(pData) + 0x3F00;
		(*pDmaTags) += 2;
	}
	else
	{
		(*pDmaTags)[0].m_size = size + dmaOffset;
		(*pDmaTags)[0].m_source = U32(pData) - dmaOffset;
		*pDmaTags += 1;
	}

	// Increment the total DMA transfer size, thus far.
	*pDmaSize += size + extraSize;
}

// Adds the command list and NV control structures to the list DMA.
static inline void AddCommandListAndNvControlToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize)
{
	// Command list.
	(*pTags)[0].m_size = 0x100;    // Size of the command list.
	(*pTags)[0].m_source = 0;      // This can't be filled in here because we don't know where the command list starts.

	// NvControl.
	(*pTags)[1].m_size = 0x30;     // Size of the NV control structure.
	(*pTags)[1].m_source = 0;      // This can't be filled in here because we don't know where NV control will wind up.
	*pTags += 2;

	*pDmaSize += 0x100 + 0x30;     // Size of the command list plus the size of the NV control structure.
}

// Adds the object info structure to the list DMA.
static inline void AddObjectInfoToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	ObjectInfo const * const pObjectInfo)
{
	ICE_ASSERT(pObjectInfo != NULL);
	ICE_ASSERT((U32(pObjectInfo) & 0xF) == 0);

	U64 size = (sizeof(ObjectInfo) + 0xF) & ~0xF;
	(*pTags)[0].m_size = size;
	(*pTags)[0].m_source = U32(pObjectInfo);
	*pTags += 1;

	*pDmaSize += size;
}

// Adds the viewport and the root transform to the list DMA which are required to perform trimming.
static inline void AddViewportAndRootTransformToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	ViewportInfo const * const pViewport,
	RigidObjectTransform const * const pRootTransform)
{
	ICE_ASSERT(pViewport != NULL);
	ICE_ASSERT((U32(pViewport) & 0xF) == 0);

	// Add the viewport.
	U64 viewportSize = (sizeof(ViewportInfo) + 0xF) & ~0xF;
	(*pTags)[0].m_size = viewportSize;
	(*pTags)[0].m_source = U32(pViewport);
	*pDmaSize += viewportSize;
	*pTags += 1;

	// Add the root transform, if it exists.
	if (pRootTransform != NULL)
	{
		ICE_ASSERT((U32(pRootTransform) & 0xF) == 0);

		U64 rootTransformSize = 0x40;
		(*pTags)[0].m_size = rootTransformSize;
		(*pTags)[0].m_source = U32(pRootTransform);
		*pDmaSize += rootTransformSize;
		*pTags += 1;
	}
}

// Adds the discrete PM info structure to the list DMA.
static inline void AddDiscretePmInfoToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize)
{
	// Discrete PM info.
	U64 size = (sizeof(DiscretePmInfo) + 0xF) & ~0xF;
	(*pTags)[0].m_size = size;
	(*pTags)[0].m_source = 0;   // This can't be filled in here because we don't know where the discrete PM info will wind up.
	*pTags += 1;

	*pDmaSize += size;
}

// Adds the continuous PM info structure to the list DMA.
static inline void AddContinuousPmInfoToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	ContinuousPmInfo const * const pContinuousPmInfo,
	U64 const continuousPmInfoSize)
{
	ICE_ASSERT((U32(pContinuousPmInfo) & 0xF) == 0);
	ICE_ASSERT((continuousPmInfoSize & 0xF) == 0);

	(*pTags)[0].m_size = continuousPmInfoSize;
	(*pTags)[0].m_source = U32(pContinuousPmInfo);
	*pTags += 1;

	*pDmaSize += continuousPmInfoSize;
}

// Adds the DM info structure to the list DMA.
static inline void AddDmInfoToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	DmInfo const * const pDmInfo)
{
	ICE_ASSERT((U32(pDmInfo) & 0xF) == 0);

	U64 size = (sizeof(DmInfo) + 0xF) & ~0xF;
	(*pTags)[0].m_size = size;
	(*pTags)[0].m_source = U32(pDmInfo);
	*pTags += 1;

	*pDmaSize += size;
}

// Add the first blend shape list DMA to the main list DMA.
static inline void AddBlendShapesToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	DmaTag const * const pBlendShapeDmaTags)
{
	U64 size = 0x30;
	(*pTags)[0].m_size = size;
	(*pTags)[0].m_source = U32(pBlendShapeDmaTags);
	*pTags += 1;
	*pDmaSize += size;
}

// Add filling between the unaligned DMA entries and the aligned DMA entries.
// In order to align the data, extra data must be uploaded in order to make the current location within the
// mesh processing input buffer align with the next piece of data being uploaded.  Normally this data comes from either
// just before the data being uploaded or just after the previous data, but in this case that memory may be unmapped.
// Reading from unmapped memory will casuse the system to crash.
static inline U64 AddAlignmentFillingToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	void const * const pNextData)
{
	U64 dmaSize = *pDmaSize;

	U64 size = (U32(pNextData) - dmaSize) & 0x7F;
	ICE_ASSERT((size & 0xF) == 0);

	if (size > 0)
	{
		(*pTags)[0].m_size = size;
		(*pTags)[0].m_source = U32(s_objectInfo);   // Junk data.
		*pTags += 1;
		*pDmaSize += size;
	}

	return dmaSize + size;
}
	
// Adds the LOD vertex table to the list DMA.
static inline void AddLodVertexTableToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	LodVertexTable const * const pLodVertexTable,
	U64 const pmSize)
{
	WriteAlignedDmaTag(pTags, pDmaSize, pLodVertexTable, pmSize);
}

// Adds the fixed stream format info, the variable stream format info, the NV streams, and the SW streams to the list DMA.
static inline void AddStreamsToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	FixedStreamFormatInfo const * const pFixedStreamFormatInfo,
	VariableStreamFormatInfo const * const pVarStreamFormatInfo,
	MeshProgramInfo const * const pMeshProgramInfo,
	U64 const vertexCount,
	Locator const * const pNvStreams,
	Locator const * const pSwStreams)
{
	// Add fixed stream format info to the List DMA.
	// There must be a fixed stream format info as all output streams are NV streams.
	U64 fixedStreamInfoSize = (pFixedStreamFormatInfo->m_length + 0xF) & ~0xF;
	WriteAlignedDmaTag(pTags, pDmaSize, pFixedStreamFormatInfo, fixedStreamInfoSize);

	// Add variable stream format info to the list DMA, if it exists.
	if (pVarStreamFormatInfo != NULL)
	{
		U64 varStreamInfoSize = (pVarStreamFormatInfo->m_length + 0xF) & ~0xF;
		WriteAlignedDmaTag(pTags, pDmaSize, pVarStreamFormatInfo, varStreamInfoSize);
	}

	// Get input stream masks.
	U32F nvInputMask = pMeshProgramInfo->m_nvInputMask;
	U32F nvOutputMask = pMeshProgramInfo->m_nvOutputMask;
	U32F swInputMask = pMeshProgramInfo->m_swInputMask;

	// Get the pointer to the custom codec offsets.
	// Offsets are byte offsets from the start of the MeshProgramInfo structure.
	U8 const *pCustomCodecOffsets = pMeshProgramInfo->m_customCodecOffsets;

	// Add NV input streams.
	U32F mask = 1;
	for (U32F iStream = 0; iStream < 8; ++iStream, mask <<= 1)
	{
		if (nvInputMask & mask)
		{
			// Get the size of the stream.
			U32 offset = pFixedStreamFormatInfo->m_formatOffsets[iStream];
			NvStreamFormat const *pFormat = (NvStreamFormat *)(U32(pFixedStreamFormatInfo) + offset);
			U64 size = (vertexCount * pFormat->m_elementSize + 0xF) & ~0xF;

			// Get a pointer to the stream data.
			void *pStream = pNvStreams[iStream].m_ptr;

			WriteAlignedDmaTags32K(pTags, pDmaSize, pStream, size);

			// Find out if custom decomprssion code exists for this stream and add it if necessary.
			U32 customCodecOffset = pCustomCodecOffsets[iStream];
			if (customCodecOffset != 0)
			{
				// Get the decompression code size and pointer.
				CustomCodecInfo const *pCustomCodecInfo = (CustomCodecInfo *)(U32(pMeshProgramInfo) + customCodecOffset);
				U32 customDecompSize = pCustomCodecInfo->m_decompressionCodeSize;
				void *pCustomDecompCode = (void *)pCustomCodecInfo->m_decompressionCodePtr;

				// Add the decompression code to the list DMA, if it exists.
				if (customDecompSize > 0)
				{
					WriteAlignedDmaTag(pTags, pDmaSize, pCustomDecompCode, customDecompSize);
				}
			}
		}
	}

	// Add SW streams.
	mask = 1;
	for (U32F iStream = 0; iStream < 16; ++iStream, mask <<= 1)
	{
		if (swInputMask & mask)
		{
			// Get the size of the stream.
			SwStreamFormat const *pFormat = &pVarStreamFormatInfo->m_streamFormats[iStream];
			U64 perVert = pFormat->m_componentBitcounts[0] + pFormat->m_componentBitcounts[1] +
				pFormat->m_componentBitcounts[2] + pFormat->m_componentBitcounts[3];
			U64 total = vertexCount * perVert;
			U64 size = ((total >> 3) + (total & 0x7 ? 1 : 0) + 0xF) & ~0xF;

			// Get a pointer to the stream data.
			void *pStream = pSwStreams[iStream].m_ptr;

			WriteAlignedDmaTags32K(pTags, pDmaSize, pStream, size);
		}
	}

	// Add custom compression code for NV output streams.
	mask = 1;
	for (U32F iStream = 0; iStream < 8; ++iStream, mask <<= 1)
	{
		if (nvOutputMask & mask)
		{
			// Find out if custom comprssion code exists for this stream and add it if necessary.
			U32 customCodecOffset = pCustomCodecOffsets[iStream];
			if (customCodecOffset != 0)
			{
				// Get the compression code size and pointer.
				CustomCodecInfo const *pCustomCodecInfo = (CustomCodecInfo *)(U32(pMeshProgramInfo) + customCodecOffset);
				U32 customCompSize = pCustomCodecInfo->m_compressionCodeSize;
				void *pCustomCompCode = (void *)pCustomCodecInfo->m_compressionCodePtr;

				// Add the compression code to the list DMA, if it exists.
				if (customCompSize > 0)
				{
					WriteAlignedDmaTag(pTags, pDmaSize, pCustomCompCode, customCompSize);
				}
			}
		}
	}
}

// Adds the indexes to the list DMA which are required to perform trimming.
static inline void AddIndexesToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	void const * const pIndexes,
	U64 const indexSize)
{
	ICE_ASSERT(pIndexes != NULL);

	// Add the indexes.
	WriteAlignedDmaTag(pTags, pDmaSize, pIndexes, (indexSize + 0xF) & ~0xF);
}

// Adds a parent table to the list DMA.
static inline void AddParentTableToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	void const * const pParentTable,
	U64 const parentSize)
{
	WriteAlignedDmaTag(pTags, pDmaSize, pParentTable, parentSize);
}

// Adds the DM displacements to the list DMA.
static inline void AddDmDisplacementsToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	void const * const pDispStream,
	U64 const vertexCount)
{
	WriteAlignedDmaTag(pTags, pDmaSize, pDispStream, (vertexCount * sizeof(F32) + 0xF) & ~0xF);
}

// Adds all of the skinning tables to the list DMA required to perform skinning on the vertex set.
static inline void AddSkinningTablesToListDma(
	DmaTag ** const pTags,
	U64 * const pDmaSize,
	SkinningTables const * const pSkinningTables,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	DmaTag ** __restrict const pMatrixTags,
	U16 * __restrict const pMatrixCount)
{
	ICE_ASSERT(pSkinningTables != NULL);

	U64 controlSize = (pSkinningTables->m_controlStreamLength + 0xF) & ~0xF;
	U64 sameSize = (pSkinningTables->m_sameStreamLength + 0xF) & ~0xF;
	U64 diffSize = (pSkinningTables->m_diffStreamLength + 0xF) & ~0xF;
	U64 weightsSize = (pSkinningTables->m_weightsStreamLength + 0xF) & ~0xF;

	// Add the four skinning tables.
	WriteAlignedDmaTag(pTags, pDmaSize, pSkinningTables->m_controlStream, controlSize);
	if (sameSize > 0)
	{
		WriteAlignedDmaTag(pTags, pDmaSize, pSkinningTables->m_sameStream, sameSize);
	}
	if (diffSize > 0)
	{
		WriteAlignedDmaTag(pTags, pDmaSize, pSkinningTables->m_diffStream, diffSize);
	}
	if (weightsSize > 0)
	{
		WriteAlignedDmaTag(pTags, pDmaSize, pSkinningTables->m_weightsStream, weightsSize);

		// Add the aux weight table, if it exists.
		void *pAuxWeights = pSkinningTables->m_auxWeightsStream;
		if (pAuxWeights != NULL)
		{
			WriteAlignedDmaTag(pTags, pDmaSize, pAuxWeights, weightsSize);
		}
	}

	// Determine the size of the matrices.
	U64 matrixSize;
	if (matrixFormat == kFormatMatrix43)
	{
		matrixSize = 0x30;
	}
	else
	{
		ICE_ASSERT(matrixFormat == kFormatMatrix44);
		matrixSize = 0x40;
	}

	// Add the matrices.
	*pMatrixTags = *pTags;
	MatrixRun const *pRun = pSkinningTables->m_matrixRuns;
	U32F runCount = pSkinningTables->m_matrixRunsCount;
	ICE_ASSERT(runCount > 0);

	// The first matrix run needs to become aligned.
	U16 count = pRun->m_length;
	*pMatrixCount += count;
	WriteAlignedDmaTag(pTags, pDmaSize, (void *)(U32(pMatrices) + pRun->m_firstIndex * matrixSize), count * matrixSize);
	pRun++;

	//Add the remaining matrix runs.
	for (U32F iRun = 1; iRun < runCount; ++iRun, ++pRun)
	{
		count = pRun->m_length;
		U64 size = count * matrixSize;
		*pMatrixCount += count;
		(*pTags)[0].m_size = size;
		(*pTags)[0].m_source = U32(pMatrices) + pRun->m_firstIndex * matrixSize;
		*pTags += 1;
		*pDmaSize += size;
	}
}

// Aligns DMA tags to the next 16 bytes and returns the new pointer.
static inline DmaTag *AlignDmaTags(DmaTag *tags)
{
	// DMA tags are already 8 byte aligned, so need to add at most only one more.
	if ((U32(tags) & 0xF) != 0)
	{
		tags->m_size = 0;
		tags->m_source = 0;
		tags++;
	}

	return tags;
}

// Checks to make sure the list DMA will fit in the mesh processing input buffer.
// If it can, return true, otherwise return false.
static inline bool TestListDmaSize(U64 dmaSize)
{
#if TEST_DMA_SIZE
	if (dmaSize > kMeshInputBufferSize)
	{
#if ICEDEBUG
		printf("Warning: List DMA size of %lld is larger than mesh input buffer size of %d.  Vertex set will not be drawn.\n",
				dmaSize, kMeshInputBufferSize);
#endif
		return false;
	}
#endif

	return true;
}



// Given a pointer to the data, its size, and the total dma size thus far, this will write the correct starting
// offset of the data into the command list.
static inline U64 WriteAlignedCmdListOffset(
	U16 * __restrict const pMpCmd,
	void const * const pData,
	U64 const size,
	U64 dmaSize)
{
	// If the pointer is 0, then output a zero to inform Mesh Processing that the data is not present.
	if (pData == NULL)
	{
		*pMpCmd = 0;
		return dmaSize;
	}

	dmaSize += (U32(pData) - dmaSize) & 0x7F;
	*pMpCmd = dmaSize;

	return dmaSize + size;
}

// Starts the mesh processing command list by placing the length of the list in it as well as the command to setup NvControl.
static inline void AddLengthAndNvControlToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize)
{
	(*pMpCmd)[0] = 0x100;   // Size of the command list.
	(*pMpCmd)[1] = kCmdSetupNvControlInfo << 8;
	(*pMpCmd)[2] = 0x100;   // NV control is located here, after the command list.
	*pMpCmd += 3;

	*pDmaSize += 0x100 + 0x30;   // Size of the command list and the NV control structure.
}

// Adds the SetupObjectInfo command to the command list.
static inline void AddObjectInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize)
{
	(*pMpCmd)[0] = kCmdSetupObjectInfo << 8;
	(*pMpCmd)[1] = *pDmaSize;
	*pMpCmd += 2;

	*pDmaSize += (sizeof(ObjectInfo) + 0xF) & ~0xF;
}

// Adds the SetupVertexInfo command to the command list.
static inline void AddVertexInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U32 const vertexCount,
	U32 const haloVertexCount)
{
	(*pMpCmd)[0] = kCmdSetupVertexInfo << 8;
	(*pMpCmd)[1] = vertexCount;
	(*pMpCmd)[2] = haloVertexCount;
	*pMpCmd += 3;
}

// Adds the SetupPmVertexInfo command to the command list.
static inline void AddPmVertexInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	LodVertexTable const * const pLodVertexTable,
	U64 const pmSize,
	U32 const lodIndex)
{
	(*pMpCmd)[0] = kCmdSetupPmVertexInfo << 8;
	*pDmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pLodVertexTable, pmSize, *pDmaSize);
	(*pMpCmd)[2] = lodIndex;
	*pMpCmd += 3;
}

// Adds the fixed stream format info, the variable stream format info, the NV streams, and the SW streams to the command list.
static inline void AddStreamsToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	FixedStreamFormatInfo const * const pFixedStreamFormatInfo,
	VariableStreamFormatInfo const * const pVarStreamFormatInfo,
	MeshProgramInfo const * const pMeshProgramInfo,
	U64 const vertexCount,
	Locator const * const pNvStreams,
	Locator const * const pSwStreams)
{
	U64 dmaSize = *pDmaSize;

	// Add setup format info command to the command list.
	(*pMpCmd)[0] = kCmdSetupFormatInfo << 8;

	// Add fixed stream format info to the command list.
	// There must be a fixed stream format info as all output streams are NV streams.
	U64 fixedStreamInfoSize = (pFixedStreamFormatInfo->m_length + 0xF) & ~0xF;
	dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pFixedStreamFormatInfo, fixedStreamInfoSize, dmaSize);

	// Add variable stream format info to the command list, if it exists.
	if (pVarStreamFormatInfo != NULL)
	{
		U64 varStreamInfoSize = (pVarStreamFormatInfo->m_length + 0xF) & ~0xF;
		dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 2, pVarStreamFormatInfo, varStreamInfoSize, dmaSize);
	}
	else
	{
		(*pMpCmd)[2] = 0;
	}
	*pMpCmd += 3;

	// Get input stream masks and the pointer to the NV stream parameter masks.
	U32F nvInputMask = pMeshProgramInfo->m_nvInputMask;
	U32F nvOutputMask = pMeshProgramInfo->m_nvOutputMask;
	U32F swInputMask = pMeshProgramInfo->m_swInputMask;
	U16 const *pNvStreamParameterMask = pMeshProgramInfo->m_nvStreamParameterMask;

	// Get the pointer to the custom codec offsets.
	// Offsets are byte offsets from the start of the MeshProgramInfo structure.
	U8 const *pCustomCodecOffsets = pMeshProgramInfo->m_customCodecOffsets;

	// Add setup input NV stream commands to the command list.
	U32F mask = 1;
	for (U32F iStream = 0; iStream < 8; ++iStream, mask <<= 1)
	{
		if (nvInputMask & mask)
		{
			// Get the size of the stream.
			U32 offset = pFixedStreamFormatInfo->m_formatOffsets[iStream];
			NvStreamFormat const *pFormat = (NvStreamFormat *)(U32(pFixedStreamFormatInfo) + offset);
			U64 size = (vertexCount * pFormat->m_elementSize + 0xF) & ~0xF;

			// Get a pointer to the stream data.
			void *pStream = pNvStreams[iStream].m_ptr;

			// Add the data to the command list.
			(*pMpCmd)[0] = (kCmdSetupNvStream << 8) | (iStream << 4);
			dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pStream, size, dmaSize);
			(*pMpCmd)[2] = pNvStreamParameterMask[iStream];

			// Find out if custom decomprssion code exists for this stream and add it if necessary.
			U32 customCodecOffset = pCustomCodecOffsets[iStream];
			if (customCodecOffset != 0)
			{
				// Get the decompression code size and pointer.
				CustomCodecInfo const *pCustomCodecInfo = (CustomCodecInfo *)(U32(pMeshProgramInfo) + customCodecOffset);
				U32 customDecompSize = pCustomCodecInfo->m_decompressionCodeSize;
				void *pCustomDecompCode = (void *)pCustomCodecInfo->m_decompressionCodePtr;

				// Add the decompression code pointer to the command list, if it exists.
				if (customDecompSize > 0)
				{
					dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 3, pCustomDecompCode, customDecompSize, dmaSize);
				}
				else
				{
					(*pMpCmd)[3] = 0;
				}
			}
			else
			{
				(*pMpCmd)[3] = 0;
			}

			*pMpCmd += 4;
		}
	}

	// Add setup SW stream commands to the command list.
	mask = 1;
	for (U32F iStream = 0; iStream < 16; ++iStream, mask <<= 1)
	{
		if (swInputMask & mask)
		{
			// Get the size of the stream.
			SwStreamFormat const *pFormat = &pVarStreamFormatInfo->m_streamFormats[iStream];
			U64 perVert = pFormat->m_componentBitcounts[0] + pFormat->m_componentBitcounts[1] +
				pFormat->m_componentBitcounts[2] + pFormat->m_componentBitcounts[3];
			U64 total = vertexCount * perVert;
			U64 size = ((total >> 3) + (total & 0x7 ? 1 : 0) + 0xF) & ~0xF;

			// Get a pointer to the stream data.
			void *pStream = pSwStreams[iStream].m_ptr;

			// Add the data to the command list.
			(*pMpCmd)[0] = (kCmdSetupSwStream << 8) | (iStream << 4);
			dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pStream, size, dmaSize);
			*pMpCmd += 2;
		}
	}

	// Add setup output NV stream commands to the command list.
	mask = 1;
	for (U32F iStream = 0; iStream < 8; ++iStream, mask <<= 1)
	{
		if (nvOutputMask & mask)
		{
			// Find out if custom comprssion code exists for this stream and add it if necessary.
			U32 customCodecOffset = pCustomCodecOffsets[iStream];
			if (customCodecOffset != 0)
			{
				// Get the compression code size and pointer.
				CustomCodecInfo const *pCustomCodecInfo = (CustomCodecInfo *)(U32(pMeshProgramInfo) + customCodecOffset);
				U32 customCompSize = pCustomCodecInfo->m_compressionCodeSize;
				void *pCustomCompCode = (void *)pCustomCodecInfo->m_compressionCodePtr;

				// Add the compression code setup command to the command list, if it exists.
				if (customCompSize > 0)
				{
					(*pMpCmd)[0] = (kCmdSetupCustomCompress << 8) | (iStream << 4);
					dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pCustomCompCode, customCompSize, dmaSize);
					(*pMpCmd)[2] = customCompSize;
					*pMpCmd += 3;
				}
			}
		}
	}

	*pDmaSize = dmaSize;
}

// Adds the indexes to the command list.
static inline void AddIndexesToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	void const * const pIndexes,
	U64 const indexSize,
	U16 const triangleCount,
	U16 const haloTriangleCount)
{
	// Add the indexes and triangle counts.
	(*pMpCmd)[0] = kCmdSetupIndexes << 8;
	*pDmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pIndexes, (indexSize + 0xF) & ~0xF, *pDmaSize);
	(*pMpCmd)[2] = triangleCount;
	(*pMpCmd)[3] = haloTriangleCount;
	*pMpCmd += 4;
}

// Adds the viewport and the root transform to the command list which are required to perform trimming.
static inline void AddViewportAndRootTransformToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	bool const hasRootTransform)
{
	U64 dmaSize = *pDmaSize;

	// Add the viewport.
	U64 const viewportSize = (sizeof(ViewportInfo) + 0xF) & ~0xF;
	(*pMpCmd)[0] = kCmdSetupViewportInfo << 8;
	(*pMpCmd)[1] = dmaSize;
	dmaSize += viewportSize;
	*pMpCmd += 2;

	// Add the root transform, if it exists.
	if (hasRootTransform)
	{
		U64 const rootTransformSize = 0x40;
		(*pMpCmd)[0] = kCmdSetupRootTransformInfo << 8;
		(*pMpCmd)[1] = dmaSize;
		dmaSize += rootTransformSize;
		*pMpCmd += 2;
	}

	*pDmaSize = dmaSize;
}

// Adds the SetupDiscrtePmInfo command to the command list.
static inline void AddDiscretePmInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize)
{
	(*pMpCmd)[0] = kCmdSetupDiscretePmInfo << 8;
	(*pMpCmd)[1] = *pDmaSize;
	*pMpCmd += 2;

	*pDmaSize += (sizeof(DiscretePmInfo) + 0xF) & ~0xF;
}

// Adds the SetupDiscrtePmInfo command to the command list for use with DM.
static inline void AddDiscretePmInfoForDmToCommandList(
	U16 * __restrict * const pMpCmd)
{
	(*pMpCmd)[0] = kCmdSetupDiscretePmInfo << 8;
	(*pMpCmd)[1] = 0;  // It doesn't matter where the data comes from as it will be overwritten.
	*pMpCmd += 2;
}

// Adds the SetupContinuousPmInfo command to the command list.
static inline void AddContinuousPmInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	U64 const continuousPmInfoSize)
{
	(*pMpCmd)[0] = kCmdSetupContinuousPmInfo << 8;
	(*pMpCmd)[1] = *pDmaSize;

	*pDmaSize += continuousPmInfoSize;
	*pMpCmd += 2;
}

// Adds a SetupPmParent command to the command list.
static inline void AddParentTableToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	void const * const pParentTable,
	U64 const parentSize,
	U32 const parentCount,
	U64 const lod = 0)
{
	(*pMpCmd)[0] = (kCmdSetupPmParent << 8) | (lod << 4);
	if (parentCount != 0) {
		*pDmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pParentTable, parentSize, *pDmaSize);
	}
	else
	{
		(*pMpCmd)[1] = 0;
	}
	(*pMpCmd)[2] = parentCount;
	*pMpCmd += 3;
}

// Adds the SetupDmInfo command to the command list.
static inline void AddDmInfoToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize)
{
	(*pMpCmd)[0] = kCmdSetupDmInfo << 8;
	(*pMpCmd)[1] = *pDmaSize;

	*pDmaSize += (sizeof(DmInfo) + 0xF) & ~0xF;
	*pMpCmd += 2;
}

// Adds the SetupDmDisplacements command to the command list.
static inline void AddDmDisplacementsToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	void const * const pDispStream,
	U64 const vertexCount)
{
	U64 dispSize = vertexCount * sizeof(F32);

	(*pMpCmd)[0] = kCmdSetupDmDisplacements << 8;
	*pDmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pDispStream, (dispSize + 0xF) & ~0xF, *pDmaSize);
	(*pMpCmd)[2] = dispSize;
	*pMpCmd += 3;
}

// Adds the SetupSkinning command to the command list.
static inline void AddSkinningTablesToCommandList(
	U16 * __restrict * const pMpCmd,
	U64 * const pDmaSize,
	SkinningTables const * const pSkinningTables,
	U8 const weightsFormat,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	U16 const matrixCount)
{
	ICE_ASSERT(pSkinningTables != NULL);

	U64 dmaSize = *pDmaSize;

	// Get the sizes of the various skinning tables.
	U64 controlSize = (pSkinningTables->m_controlStreamLength + 0xF) & ~0xF;
	U64 sameSize = (pSkinningTables->m_sameStreamLength + 0xF) & ~0xF;
	U64 diffSize = (pSkinningTables->m_diffStreamLength + 0xF) & ~0xF;
	U64 weightsSize = (pSkinningTables->m_weightsStreamLength + 0xF) & ~0xF;

	// Add the command to setup the skinning tables.
	(*pMpCmd)[0] = (kCmdSetupSkinning << 8) | weightsFormat;
	dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 1, pSkinningTables->m_controlStream, controlSize, dmaSize);
	(*pMpCmd)[2] = pSkinningTables->m_controlStreamLength;

	if (sameSize > 0)
	{
		dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 3, pSkinningTables->m_sameStream, sameSize, dmaSize);
	}
	else
	{
		(*pMpCmd)[3] = 0;
	}
	(*pMpCmd)[4] = pSkinningTables->m_sameStreamLength;

	if (diffSize > 0)
	{
		dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 5, pSkinningTables->m_diffStream, diffSize, dmaSize);
	}
	else
	{
		(*pMpCmd)[5] = 0;
	}
	(*pMpCmd)[6] = pSkinningTables->m_diffStreamLength;

	if (weightsSize > 0)
	{
		dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 7, pSkinningTables->m_weightsStream, weightsSize, dmaSize);

		void *auxWeightsSource = pSkinningTables->m_auxWeightsStream;
		if (auxWeightsSource != NULL)
		{
			dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 8, auxWeightsSource, weightsSize, dmaSize);
		}
		else
		{
			(*pMpCmd)[8] = 0;
		}
	}
	else
	{
		(*pMpCmd)[7] = 0;
		(*pMpCmd)[8] = 0;
	}
	(*pMpCmd)[9] = pSkinningTables->m_weightsStreamLength;

	// Determine the size of the matrices.
	U64 matrixSize;
	if (matrixFormat == kFormatMatrix43)
	{
		matrixSize = 0x30;
	}
	else
	{
		ICE_ASSERT(matrixFormat == kFormatMatrix44);
		matrixSize = 0x40;
	}

	// Add the command to setup the matrices.
	(*pMpCmd)[10] = (kCmdSetupMatrices << 8) | matrixFormat;
	dmaSize = WriteAlignedCmdListOffset(*pMpCmd + 11,
		(void *)(U32(pMatrices) + pSkinningTables->m_matrixRuns->m_firstIndex * matrixSize),
		matrixCount * matrixSize, dmaSize);
	(*pMpCmd)[12] = matrixCount;
	*pMpCmd += 13;

	*pDmaSize = dmaSize;
}

// Adds the commands to the command list necessary to process blend shapes.
static inline void AddBlendShapesToCommandList(U16 *__restrict * pMpCmd, U64 const dmaSize)
{
	(*pMpCmd)[0] = kCmdStartInputListDma << 8;
	(*pMpCmd)[1] = dmaSize;
	(*pMpCmd)[2] = 0x28;
	(*pMpCmd)[3] = 0x6000;
	(*pMpCmd)[4] = kCmdStallOnInputDma << 8;
	(*pMpCmd)[5] = kCmdCall << 8;
	(*pMpCmd)[6] = 0x6000;
	*pMpCmd += 7;
}

// Adds the EndSetup command to the command list.
static inline void AddEndSetupToCommandList(U16 *__restrict * pMpCmd)
{
	(*pMpCmd)[0] = kCmdEndSetup << 8;
	*pMpCmd += 1;
}

// Adds the PerformDm command to the command list.
static inline void AddPerformDmToCommandList(U16 *__restrict * pMpCmd)
{
	(*pMpCmd)[0] = kCmdPerformDm << 8;
	*pMpCmd += 1;
}

// Adds the SkinObject command to the command list.
static inline void AddSkinObjectToCommandList(U16 *__restrict * pMpCmd, U16 const skinningBits)
{
	(*pMpCmd)[0] = (kCmdSkinObject << 8) | skinningBits;
	*pMpCmd += 1;
}

// Add the PerformPm command to the command list.
static inline void AddPerformPmToCommandList(U16 *__restrict * pMpCmd)
{
	(*pMpCmd)[0] = kCmdPerformPm << 8;
	*pMpCmd += 1;
}

// Adds the TrimIndexes and the OutputIndexes commands to the command list.
static inline void AddTrimmingToCommandList(U16 *__restrict * pMpCmd)
{
	(*pMpCmd)[0] = kCmdTrimIndexes << 8;
	(*pMpCmd)[1] = kCmdOutputIndexes << 8;
	*pMpCmd += 2;
}

// Adds an OutputConvertedUniformTable command for each output stream to the command list.
static inline void AddOutputStreamsToCommandList(
	U16 * __restrict * const pMpCmd,
	U16 const nvOutputMask)
{
	U16 mask = 1;
	for (U32F iStream = 0; iStream < 8; ++iStream, mask <<= 1)
	{
		if (nvOutputMask & mask)
		{
			(*pMpCmd)[0] = (kCmdOutputConvertedUniformTable << 8) | (iStream << 4);
			*pMpCmd += 1;
		}
	}
}

// Adds the CleanupAndExit command to the command list and then rounds up the command list to the next 16 bytes.
static inline void FinishCommandList(
	U16 * __restrict * const pMpCmd)
{
	// This must be the last command in the command list.
	(*pMpCmd)[0] = kCmdCleanupAndExit << 8;
	*pMpCmd += 1;

	// Round mpCmd up to the next 16 bytes.
	while ((U32(*pMpCmd) & 0xF) != 0)
	{
		(*pMpCmd)[0] = 0;
		*pMpCmd += 1;
	}
}



// Aligns the list DMA, builds a job command list, and returns a valid job header if the input DMA size is not too large.
static inline void SetJobCommandList(
	SpuModuleHandle *pSpuModule,
	DmaTag const * const tagsStart,
	U64 const tagsSize,
	WwsJob_Command * __restrict const commandList,
	U8 const * const jobData,
	U8 const * const jobDataOutput)
{
	// Calculate the actual output addresses of the list DMA and job command list.
	U32 tagsPtr = U32(tagsStart) - U32(jobData) + U32(jobDataOutput);

	// Get information about the code.
	U32 codeSize = pSpuModule->GetFileSize();
	U32 codePtr = U32(pSpuModule->GetAddress());
	U32 codeSizeInPages = pSpuModule->GetRequiredBufferSizeInPages();

	ICE_ASSERT((kMeshInputBufferSize & 0x3FF) == 0);
	ICE_ASSERT((kMeshInputBufferSetOrg & 0x3FF) == 0);
	ICE_ASSERT(LsMemoryLimits::kJobAreaBasePageNum + codeSizeInPages <= kMeshInputBufferSetOrg >> 10);

	// Generate the job commands.
	commandList[0].ReserveBufferSet(kMeshCodeBufferSetNum, 1, LsMemoryLimits::kJobAreaBasePageNum, codeSizeInPages);
	commandList[1].ReserveBufferSet(kMeshInputBufferSetNum, 1, kMeshInputBufferSetOrg >> 10, kMeshInputBufferSize >> 10);
	commandList[2].UseInputBuffer(kMeshCodeBufferSetNum, 0, codePtr, codeSize, WwsJob_Command::kReadOnlyCached);
	commandList[3].UseInputDmaListBuffer(kMeshInputBufferSetNum, 0, tagsPtr, tagsSize, WwsJob_Command::kNonCached, 0);
	commandList[4].UnreserveBufferSets((1 << kMeshCodeBufferSetNum) | (1 << kMeshInputBufferSetNum));
	commandList[5].RunJob(kMeshCodeBufferSetNum, 0);
}

static inline JobHeader SetJobHeader(
	U32 const commandListPtr)
{
	// Build a job header.
	JobHeader jobHeader;
	jobHeader.m_enableBreakpoint = 0;
	jobHeader.m_loadCommandsSize = 6 * sizeof(WwsJob_Command);
	jobHeader.m_jobHeaderCommand = JobHeaderCommand::kJobExists;
	jobHeader.m_mmaLoadCommands = commandListPtr;

	// Return the job header.
	return jobHeader;
}



// Sets up the push buffer contents for a vertex set that is to be rendered directly on the GPU.
void Ice::Mesh::RenderGpuVertexSet(
	GpuVertexSet const * const vertexSet,
	VertexInputInfo const * const pVertexInputInfo,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	U32 const gpuSkinningConstantIndex,
	Render::CommandContextData * __restrict const commandContext)
{
	// If there are matrix runs, then skinning exists, so we need to place the matrices into the push buffer.
	U32F matrixRunsCount = vertexSet->m_matrixRunsCount;
	if (matrixRunsCount > 0)
	{
		AddMatrixRunsToCommandContext(commandContext, matrixRunsCount, vertexSet->m_matrixRuns,
				pMatrices, matrixFormat, gpuSkinningConstantIndex);
	}

	// Build the push buffer.
	AddStaticAttributePointersToCommandContext(commandContext, vertexSet->m_streams, pVertexInputInfo);

	Render::Inline::CommandContext * __restrict pCommandContext =
		static_cast<Render::Inline::CommandContext *>(commandContext);
	U32 indexOffset = vertexSet->m_indexes.m_ofs;
	pCommandContext->DrawElements(Render::kDrawTriangles, 0, vertexSet->m_triangleCount * 3, Render::kIndex16,
		indexOffset & 0x7FFFFFFF, Render::IndexContext(indexOffset >> 31));
}

// Sets up the push buffer contents for a basic vertex set that is to be rendered directly on the GPU.
static void RenderBasicVertexSetOnGpu(
	BasicVertexSet const * const vertexSet,
	VertexInputInfo const * const pVertexInputInfo,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	U32 const gpuSkinningConstantIndex,
	Render::CommandContextData * __restrict const commandContext)
{
	// If there are matrix runs, then skinning exists, so we need to place the matrices into the push buffer.
	SkinningTables const *pSkinningTables = vertexSet->m_skinningTables;
	if (pSkinningTables != NULL)
	{
		U32F matrixRunsCount = pSkinningTables->m_matrixRunsCount;
		if (matrixRunsCount > 0)
		{
			AddMatrixRunsToCommandContext(commandContext, matrixRunsCount, pSkinningTables->m_matrixRuns,
					pMatrices, matrixFormat, gpuSkinningConstantIndex);
		}
	}

	// Build the push buffer.
	AddStaticAttributePointersToCommandContext(commandContext, vertexSet->m_streams, pVertexInputInfo);

	Render::Inline::CommandContext * __restrict pCommandContext =
		static_cast<Render::Inline::CommandContext *>(commandContext);
	U32 haloTriangleCount = (vertexSet->m_stencilInfo == NULL) ? 0 : vertexSet->m_stencilInfo->m_haloTriangleCount;
	U32 indexOffset = vertexSet->m_indexes.m_ofs;
	U32 indexCount = (vertexSet->m_triangleCount - haloTriangleCount) * 3;
	pCommandContext->DrawElements(Render::kDrawTriangles, 0, indexCount, Render::kIndex16,
		indexOffset & 0x7FFFFFFF, Render::IndexContext(indexOffset >> 31));
}

// Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses
// no CLOD, optional skinning, optional trimming, and optional blend shapes.
// Skinning is performed if skinning bits are present in the mesh program info.
// Trimming is performed if the kDoTrimming flag is set in the processing flags.
// Blend shapes are used if the pointer to the blend shapes is valid.
// If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
void Ice::Mesh::RenderBasicVertexSet(
	BasicVertexSet const * const vertexSet,
	VertexSetBlendShapes const * const blendShapes,
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	CutAndPasteInfo * __restrict const cutAndPaste)
{
	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;
	
	// If there is no mesh program info, then render the basic vertex set directly on the GPU.
	if (inputInfo->m_pMeshProgramInfo == NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		RenderBasicVertexSetOnGpu(vertexSet, inputInfo->m_pVertexInputInfo, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, inputInfo->m_gpuSkinningConstantIndex,
				pCommandContext);
		return;
	}

	// Generate blend shape data, if they exist.
	DmaTag const *pBlendShapeDmaTags = NULL;
	if (blendShapes != NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		pBlendShapeDmaTags = GenerateBlendShapeData(blendShapes, inputInfo->m_deltaStreamUsageBits, 0,
				inputInfo->m_pBlendShapeFactors, outputLocs->m_jobDataAllocator);
	}

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));

	// Write onto the stack for optimal performance.
	// Allocate space for maximum push buffer size, job info, DMA tags, command list, nvControl, and discrete PM info.
	U32 * __restrict pbCmd = (U32 * __restrict)alloca(0x500);

	// Get the triangle and vertex counts.
	U64 triangleCount = vertexSet->m_triangleCount;
	U64 haloTriangleCount = (vertexSet->m_stencilInfo == NULL) ? 0 : vertexSet->m_stencilInfo->m_haloTriangleCount;
	U64 vertexCount = vertexSet->m_vertexCount;
	U64 haloVertexCount = (vertexSet->m_stencilInfo == NULL) ? 0 : vertexSet->m_stencilInfo->m_haloVertexCount;

	// Get the processing options.
	bool performTrimming = inputInfo->m_procFlags & kDoTrimming;
	bool performSkinning = inputInfo->m_pMeshProgramInfo->m_skinningBits;
	bool performDoubleSidedTrimming = (inputInfo->m_procFlags & kDoubleSided) && performTrimming;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;



	// Build the push buffer.
	U32 *pbCmdStart = pbCmd;    // Remember where the push buffer started.

	// Add the static attribute pointers to the push buffer and get the necessary hole size required to store
	// the attribute pointers for attributes output by mesh processing.
	U64 vertexPtrHoleSize = AddStaticAttributePointersToPushBuffer(&pbCmd, vertexSet->m_streams, inputInfo->m_pVertexInputInfo);

	// If trimming is not performed, then add the draw commands into the push buffer.
	// Otherwise, calculate the hole size necessary to store the draw commands.
	U64 trimHoleSize;
	U64 indexCount = (triangleCount - haloTriangleCount) * 3;
	if (performTrimming)
	{
		trimHoleSize = (10 + (indexCount >> 8)) * 4;
	}
	else
	{
		AddDrawCommandsToPushBuffer(&pbCmd, vertexSet->m_indexes.m_ofs, indexCount);
		trimHoleSize = 0;
	}
	U64 holeSize = FinishPushBufferAndGetHoleSize(&pbCmd, vertexPtrHoleSize, trimHoleSize, performTrimming, useRingBuffer);

	// Reserve memory for the push buffer and determine its final output address.
	U64 pbSize = U32(pbCmd) - U32(pbCmdStart);
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, holeSize);



	// Put the list DMA tags at the next 64 byte alignment after the push buffer on the stack.
	U8 *jobData = (U8 *)ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)pbCmd);

	DmaTag *tags = (DmaTag *)jobData;
	DmaTag *tagsStart = tags;         // Remember the start of the DMA tags to patch the command list in later.
	U64 dmaSize = 0;                  // Tracks size of data so far in the input buffer.

	// Build the list DMA tags.
	AddCommandListAndNvControlToListDma(&tags, &dmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToListDma(&tags, &dmaSize, &s_objectInfo[kDoubleSidedObjectInfoNum]);
	}

	DmaTag *trimmingTags = tags;      // Save pointer to trimming tags for cut and paste.
	if (performTrimming)
	{
		AddViewportAndRootTransformToListDma(&tags, &dmaSize, inputInfo->m_pViewport, inputInfo->m_pRootTransform);
	}

	DmaTag *blendShapeTag = tags;     // Save pointer to blend shape tag for cut and paste.
	if (pBlendShapeDmaTags != NULL)
	{
		AddBlendShapesToListDma(&tags, &dmaSize, pBlendShapeDmaTags);
	}

	U64 alignedDmaSize = AddAlignmentFillingToListDma(&tags, &dmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo);

	AddStreamsToListDma(&tags, &dmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo, vertexSet->m_streamFormatInfo,
		inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams, vertexSet->m_streams + vertexSet->m_nvStreamCount);

	if (performTrimming)
	{
		AddIndexesToListDma(&tags, &dmaSize, vertexSet->m_indexes.m_ptr, triangleCount * 6);
	}

	DmaTag *matrixTags = NULL;        // Save pointer to matrices tag for cut and paste.
	U16 matrixCount = 0;              // Save matrix count for cut and paste.
	if (performSkinning)
	{
		ICE_ASSERT((cutAndPaste != NULL) || (inputInfo->m_pMatrices != NULL));
		AddSkinningTablesToListDma(&tags, &dmaSize, vertexSet->m_skinningTables, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, &matrixTags, &matrixCount);
	}

	bool addJob = TestListDmaSize(dmaSize);



	// Get the start of the command list.  The command list starts in memory directly after the DMA tags.
	U16 * __restrict mpCmd = (U16 * __restrict)AlignDmaTags(tags);
	U16 * __restrict mpCmdStart = mpCmd;
	U64 unalignedDmaSize = 0;   // Tracks size of the unaligned data so far in the input buffer.

	// Build the command list
	AddLengthAndNvControlToCommandList(&mpCmd, &unalignedDmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToCommandList(&mpCmd, &unalignedDmaSize);
	}
	AddVertexInfoToCommandList(&mpCmd, vertexCount, haloVertexCount);
	AddStreamsToCommandList(&mpCmd, &alignedDmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo,
		vertexSet->m_streamFormatInfo, inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams,
		vertexSet->m_streams + vertexSet->m_nvStreamCount);
	if (performTrimming)
	{
		AddIndexesToCommandList(&mpCmd, &alignedDmaSize, vertexSet->m_indexes.m_ptr, triangleCount * 6,
			triangleCount, haloTriangleCount);
		AddViewportAndRootTransformToCommandList(&mpCmd, &unalignedDmaSize, inputInfo->m_pRootTransform != NULL);
	}
	if (performSkinning)
	{
		AddSkinningTablesToCommandList(&mpCmd, &alignedDmaSize, vertexSet->m_skinningTables,
				inputInfo->m_pFixedFormatInfo->m_skinWeightFormat, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, matrixCount);
	}
	if (pBlendShapeDmaTags != NULL)
	{
		AddBlendShapesToCommandList(&mpCmd, unalignedDmaSize);
	}
	AddEndSetupToCommandList(&mpCmd);
	if (performSkinning)
	{
		AddSkinObjectToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_skinningBits);
	}
	if (performTrimming)
	{
		AddTrimmingToCommandList(&mpCmd);
	}
	AddOutputStreamsToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_nvOutputMask);
	FinishCommandList(&mpCmd);




	// Calculate the size of the job data, including 0x30 bytes for the job command list, and round this to a 64 byte multiple.
	// NvControl is not generated if cut and paste is being used.
	U64 jobDataSize;
	if (cutAndPaste != NULL)
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x30;
	}
	else
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x60;
	}
	U64 jobDataSizeRounded = (jobDataSize + 0x3F) & ~0x3F;

	// Reserve memory for the job data and determine its final output address.
	ICE_ASSERT(pbSize + jobDataSizeRounded <= 0x500);
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSizeRounded);

	// Calculate the size of the list DMA tags, rounded up to a multiple of 16 bytes.
	U32 tagsSize = (U32(tags) - U32(tagsStart) + 0xF) & ~0xF;

	// Setup the job command list.
	SetJobCommandList(outputLocs->m_spuModule, tagsStart, tagsSize, (WwsJob_Command *)mpCmd, jobData, jobDataOutput);

	// The job will not be added if there is no room left in either the push buffer or the job data buffer.
	addJob &= (pbOutput != NULL) && (jobDataOutput != NULL);



	if (cutAndPaste != NULL)
	{
		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(mpCmd) + 0x30));

		// Set cut and paste info.
		cutAndPaste->m_pushBuffer = addJob ? pbOutput : NULL;
		cutAndPaste->m_pbSize = pbSize;
		cutAndPaste->m_pbHoleSize = holeSize;
		cutAndPaste->m_jobData = jobDataOutput;
		cutAndPaste->m_jobDataSize = jobDataSize;
		cutAndPaste->m_tagsSize = tagsSize;
		cutAndPaste->m_trimmingTagsOffset = performTrimming ? U32(trimmingTags) - U32(jobData) : 0;
		cutAndPaste->m_discretePmInfoTagOffset = 0;
		cutAndPaste->m_continuousPmInfoTagOffset = 0;
		cutAndPaste->m_dmInfoTagOffset = 0;
		cutAndPaste->m_blendShapeTagOffset = (pBlendShapeDmaTags != NULL) ? U32(blendShapeTag) - U32(jobData) : 0;
		cutAndPaste->m_matrixRunsCount = performSkinning ? vertexSet->m_skinningTables->m_matrixRunsCount : 0;
		cutAndPaste->m_matricesPtrOffset = U32(inputInfo->m_pMatrices);
		cutAndPaste->m_matrixTagsOffset = U32(matrixTags) - U32(jobData);
		cutAndPaste->m_numParentTablesForContinuousPm = 0;
		cutAndPaste->m_parentTableTagsOffset = 0;
		cutAndPaste->m_commandListOffset = U32(mpCmdStart) - U32(jobData);

		// Output the push buffer only if the job could be added.
		if (addJob)
		{
			OutputPushBuffer(pCommandContext, pbCmdStart, pbSize);
		}
	}
	else if (addJob)
	{
		// Build NV control.
		NvControl * __restrict pNvControl = (NvControl * __restrict)(U32(mpCmd) + 0x30);
		U32 *pPrimaryPbHole = performTrimming ? pbOutput + pbSize / sizeof(U32) : pbOutput;
		U32 pbEndPtr = U32(pbOutput) + pbSize + holeSize + outputLocs->m_cmdBufferOffset;
		((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
		((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
		pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
		pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
		pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
		pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
		pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
		pNvControl->m_primaryPbHoleSize = holeSize;
		pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;
		pNvControl->m_numLights = 0;



		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(pNvControl) + 0x30));

		// Patch the DMA tags now that we know where the job data will be output.
		tagsStart[0].m_source = U32(jobDataOutput) + U32(mpCmdStart) - U32(jobData);  // Pointer to the command list.
		tagsStart[1].m_source = U32(jobDataOutput) + U32(pNvControl) - U32(jobData);  // Pointer to NV control.

		// Add the job header to the job list.
		outputLocs->m_jobList->AddJob(SetJobHeader(U32(mpCmd) - U32(jobData) + U32(jobDataOutput)));

		// Output the push buffer data.
		OutputPushBufferWithHole(pCommandContext, pbCmdStart, pbSize, holeSize, performTrimming);
	}

	// Output the job data if the job is to be added.
	if (addJob)
	{
		OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSizeRounded);
	}
}

// Sets up the push buffer contents for a PM vertex set that is to be rendered directly on the GPU.
static void RenderPmVertexSetOnGpu(
	PmVertexSet const * const vertexSet,
	VertexInputInfo const * const pVertexInputInfo,
	F32 const * const pMatrices,
	U8 const matrixFormat,
	U32 const gpuSkinningConstantIndex,
	U32F const lodIndex,
	Render::CommandContextData * __restrict const commandContext)
{
	// If there are matrix runs, then skinning exists, so we need to place the matrices into the push buffer.
	if (vertexSet->m_lodSkinningTables != NULL)
	{
		SkinningTables const *pSkinningTables = &vertexSet->m_lodSkinningTables[lodIndex];
		U32F matrixRunsCount = pSkinningTables->m_matrixRunsCount;
		if (matrixRunsCount > 0)
		{
			AddMatrixRunsToCommandContext(commandContext, matrixRunsCount, pSkinningTables->m_matrixRuns,
					pMatrices, matrixFormat, gpuSkinningConstantIndex);
		}
	}

	// Build the push buffer.
	AddStaticAttributePointersToCommandContext(commandContext, vertexSet->m_streams, pVertexInputInfo);

	Render::Inline::CommandContext * __restrict pCommandContext =
		static_cast<Render::Inline::CommandContext *>(commandContext);
	IndexTable const *indexTable = &vertexSet->m_lodIndexTables[lodIndex];
	U32 indexOffset = indexTable->m_indexes.m_ofs;
	U32 indexCount = (indexTable->m_triangleCount - indexTable->m_haloTriangleCount) * 3;
	pCommandContext->DrawElements(Render::kDrawTriangles, 0, indexCount, Render::kIndex16,
		indexOffset & 0x7FFFFFFF, Render::IndexContext(indexOffset >> 31));
}

// Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses
// discrete PM, optional skinning, optional trimming, and optional blend shapes.
// Skinning is performed if skinning bits are present in the mesh program info.
// Trimming is performed if the kDoTrimming flag is set in the processing flags.
// Blend shapes are used if the pointer to the blend shapes is valid.
// If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
void Ice::Mesh::RenderDiscretePmVertexSet(
	PmVertexSet const * const vertexSet,
	VertexSetBlendShapes const * const blendShapes,
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	CutAndPasteInfo * __restrict const cutAndPaste)
{
	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;
	
	U32F lodIndex = inputInfo->m_lodLevel;

	// If there is no mesh program info, then render the PM vertex set directly on the GPU.
	if (inputInfo->m_pMeshProgramInfo == NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		RenderPmVertexSetOnGpu(vertexSet, inputInfo->m_pVertexInputInfo, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, inputInfo->m_gpuSkinningConstantIndex,
				lodIndex, pCommandContext);
		return;
	}

	// Generate blend shape data, if they exist.
	DmaTag const *pBlendShapeDmaTags = NULL;
	if (blendShapes != NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		pBlendShapeDmaTags = GenerateBlendShapeData(blendShapes, inputInfo->m_deltaStreamUsageBits, lodIndex,
				inputInfo->m_pBlendShapeFactors, outputLocs->m_jobDataAllocator);
	}

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));

	// Write onto the stack for optimal performance.
	// Allocate space for maximum push buffer size, job info, DMA tags, command list, nvControl, and discrete PM info.
	U32 * __restrict pbCmd = (U32 * __restrict)alloca(0x500);

	// Get the triangle and vertex counts.
	LodVertexTable const *vertexTable = &vertexSet->m_lodVertexTable[lodIndex];
	IndexTable const *indexTable = &vertexSet->m_lodIndexTables[lodIndex];
	U64 triangleCount = indexTable->m_triangleCount;
	U64 haloTriangleCount = indexTable->m_haloTriangleCount;
	U64 vertexCount = vertexTable->m_firstVertex + vertexTable->m_vertexCount;

	// Get PM and parent sizes.
	U64 pmSize = (sizeof(*vertexSet->m_lodVertexTable) * vertexSet->m_lodVertexTable[0].m_rowCount + 0xF) & ~0xF;
	U64 parentSize = (vertexTable->m_vertexCount * 2 + 0xF) & ~0xF;

	// Get the processing options.
	bool performTrimming = inputInfo->m_procFlags & kDoTrimming;
	bool performSkinning = inputInfo->m_pMeshProgramInfo->m_skinningBits;
	bool performDoubleSidedTrimming = (inputInfo->m_procFlags & kDoubleSided) && performTrimming;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;



	// Build the push buffer.
	U32 *pbCmdStart = pbCmd;    // Remember where the push buffer started.

	// Add the static attribute pointers to the push buffer and get the necessary hole size required to store
	// the attribute pointers for attributes output by mesh processing.
	U64 vertexPtrHoleSize = AddStaticAttributePointersToPushBuffer(&pbCmd, vertexSet->m_streams, inputInfo->m_pVertexInputInfo);

	// If trimming is not performed, then add the draw commands into the push buffer.
	// Otherwise, calculate the hole size necessary to store the draw commands.
	U64 trimHoleSize;
	U64 indexCount = (triangleCount - haloTriangleCount) * 3;
	if (performTrimming)
	{
		trimHoleSize = (10 + (indexCount >> 8)) * 4;
	}
	else
	{
		AddDrawCommandsToPushBuffer(&pbCmd, indexTable->m_indexes.m_ofs, indexCount);
		trimHoleSize = 0;
	}
	U64 holeSize = FinishPushBufferAndGetHoleSize(&pbCmd, vertexPtrHoleSize, trimHoleSize, performTrimming, useRingBuffer);

	// Reserve memory for the push buffer and determine its final output address.
	U64 pbSize = U32(pbCmd) - U32(pbCmdStart);
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, holeSize);



	// Put the list DMA tags at the next 64 byte alignment after the push buffer on the stack.
	U8 *jobData = (U8 *)ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)pbCmd);

	DmaTag *tags = (DmaTag *)jobData;
	DmaTag *tagsStart = tags;         // Remember the start of the DMA tags to patch the command list in later.
	U64 dmaSize = 0;                  // Tracks size of data so far in the input buffer.

	// Build the list DMA tags.
	AddCommandListAndNvControlToListDma(&tags, &dmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToListDma(&tags, &dmaSize, &s_objectInfo[kDoubleSidedObjectInfoNum]);
	}

	DmaTag *trimmingTags = tags;      // Save pointer to trimming tags for cut and paste.
	if (performTrimming)
	{
		AddViewportAndRootTransformToListDma(&tags, &dmaSize, inputInfo->m_pViewport, inputInfo->m_pRootTransform);
	}

	DmaTag *discretePmInfoTag = tags;
	AddDiscretePmInfoToListDma(&tags, &dmaSize);

	DmaTag *blendShapeTag = tags;     // Save pointer to blend shape tag for cut and paste.
	if (pBlendShapeDmaTags != NULL)
	{
		AddBlendShapesToListDma(&tags, &dmaSize, pBlendShapeDmaTags);
	}

	U64 alignedDmaSize = AddAlignmentFillingToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable);

	AddLodVertexTableToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable, pmSize);
	AddStreamsToListDma(&tags, &dmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo, vertexSet->m_streamFormatInfo,
		inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams, vertexSet->m_streams + vertexSet->m_nvStreamCount);

	if (performTrimming)
	{
		AddIndexesToListDma(&tags, &dmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6);
	}

	DmaTag *matrixTags = NULL;       // Save pointer to matrices tag for cut and paste.
	U16 matrixCount = 0;             // Save matrix count for cut and paste.
	if (performSkinning)
	{
		ICE_ASSERT(vertexSet->m_lodSkinningTables != NULL);
		ICE_ASSERT((cutAndPaste != NULL) || (inputInfo->m_pMatrices != NULL));
		AddSkinningTablesToListDma(&tags, &dmaSize, &vertexSet->m_lodSkinningTables[lodIndex], inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, &matrixTags, &matrixCount);
	}

	// Get a pointer to the parent table.  If we are at a LOD flat spot, then set the parent table to NULL to signify
	// that no disrete PM will take place (or upload the parent tables).
	U64 parentTableDmaStart = dmaSize;
	void const *parentTable = vertexSet->m_lodParentTables[lodIndex];
	if (inputInfo->m_lodLevelFractional == 0.0f)
	{
		parentTable = NULL;
	}

	// If a parent table exists at this LOD, then add it.
	DmaTag *parentTableTag = tags;
	if (parentTable != NULL)
	{
		AddParentTableToListDma(&tags, &dmaSize, parentTable, parentSize);
	}

	bool addJob = TestListDmaSize(dmaSize);



	// Get the start of the command list.  The command list starts in memory directly after the DMA tags.
	U16 * __restrict mpCmd = (U16 * __restrict)AlignDmaTags(tags);
	U16 * __restrict mpCmdStart = mpCmd;
	U64 unalignedDmaSize = 0;   // Tracks size of the unaligned data so far in the input buffer.

	// Build the command list
	AddLengthAndNvControlToCommandList(&mpCmd, &unalignedDmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToCommandList(&mpCmd, &unalignedDmaSize);
	}
	AddPmVertexInfoToCommandList(&mpCmd, &alignedDmaSize, vertexSet->m_lodVertexTable, pmSize, lodIndex);
	AddStreamsToCommandList(&mpCmd, &alignedDmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo,
		vertexSet->m_streamFormatInfo, inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams,
		vertexSet->m_streams + vertexSet->m_nvStreamCount);
	if (performTrimming)
	{
		AddIndexesToCommandList(&mpCmd, &alignedDmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6,
			triangleCount, haloTriangleCount);
		AddViewportAndRootTransformToCommandList(&mpCmd, &unalignedDmaSize, inputInfo->m_pRootTransform != NULL);
	}
	AddDiscretePmInfoToCommandList(&mpCmd, &unalignedDmaSize);
	if (parentTable != NULL)
	{
		AddParentTableToCommandList(&mpCmd, &parentTableDmaStart, parentTable, parentSize, vertexTable->m_vertexCount);
	}
	if (performSkinning)
	{
		ICE_ASSERT(vertexSet->m_lodSkinningTables != NULL);
		AddSkinningTablesToCommandList(&mpCmd, &alignedDmaSize, &vertexSet->m_lodSkinningTables[lodIndex],
				inputInfo->m_pFixedFormatInfo->m_skinWeightFormat, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, matrixCount);
	}
	if (pBlendShapeDmaTags != NULL)
	{
		AddBlendShapesToCommandList(&mpCmd, unalignedDmaSize);
	}
	AddEndSetupToCommandList(&mpCmd);
	if (performSkinning)
	{
		AddSkinObjectToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_skinningBits);
	}
	if (parentTable != NULL)
	{
		AddPerformPmToCommandList(&mpCmd);
	}
	if (performTrimming)
	{
		AddTrimmingToCommandList(&mpCmd);
	}
	AddOutputStreamsToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_nvOutputMask);
	FinishCommandList(&mpCmd);



	// Calculate the size of the job data, including 0x30 bytes for the job command list, and round this to a 64 byte multiple.
	// NvControl is not generated if cut and paste is being used.
	U64 jobDataSize;
	if (cutAndPaste != NULL)
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x30;
	}
	else
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x70;
	}
	U64 jobDataSizeRounded = (jobDataSize + 0x3F) & ~0x3F;

	// Reserve memory for the job data and determine its final output address.
	ICE_ASSERT(pbSize + jobDataSizeRounded <= 0x500);
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSizeRounded);

	// Calculate the size of the list DMA tags, rounded up to a multiple of 16 bytes.
	U32 tagsSize = (U32(tags) - U32(tagsStart) + 0xF) & ~0xF;

	// Setup the job command list.
	SetJobCommandList(outputLocs->m_spuModule, tagsStart, tagsSize, (WwsJob_Command *)mpCmd, jobData, jobDataOutput);

	// The job will not be added if there is no room left in either the push buffer or the job data buffer.
	addJob &= (pbOutput != NULL) && (jobDataOutput != NULL);



	if (cutAndPaste != NULL)
	{
		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(mpCmd) + 0x30));

		// Set cut and paste info.
		cutAndPaste->m_pushBuffer = addJob ? pbOutput : NULL;
		cutAndPaste->m_pbSize = pbSize;
		cutAndPaste->m_pbHoleSize = holeSize;
		cutAndPaste->m_jobData = jobDataOutput;
		cutAndPaste->m_jobDataSize = jobDataSize;
		cutAndPaste->m_tagsSize = tagsSize;
		cutAndPaste->m_trimmingTagsOffset = performTrimming ? U32(trimmingTags) - U32(jobData) : 0;
		cutAndPaste->m_discretePmInfoTagOffset = U32(discretePmInfoTag) - U32(jobData);
		cutAndPaste->m_continuousPmInfoTagOffset = 0;
		cutAndPaste->m_dmInfoTagOffset = 0;
		cutAndPaste->m_blendShapeTagOffset = (pBlendShapeDmaTags != NULL) ? U32(blendShapeTag) - U32(jobData) : 0;
		cutAndPaste->m_matrixRunsCount = performSkinning ? vertexSet->m_lodSkinningTables[lodIndex].m_matrixRunsCount : 0;
		cutAndPaste->m_matricesPtrOffset = U32(inputInfo->m_pMatrices);
		cutAndPaste->m_matrixTagsOffset = U32(matrixTags) - U32(jobData);
		cutAndPaste->m_numParentTablesForContinuousPm = 0;
		cutAndPaste->m_parentTableTagsOffset = (parentTable != NULL) ? U32(parentTableTag) - U32(jobData) : 0;
		cutAndPaste->m_commandListOffset = U32(mpCmdStart) - U32(jobData);

		// Output the push buffer only if the job could be added.
		if (addJob)
		{
			OutputPushBuffer(pCommandContext, pbCmdStart, pbSize);
		}
	}
	else if (addJob)
	{
		// Build NV control.
		NvControl * __restrict pNvControl = (NvControl * __restrict)(U32(mpCmd) + 0x30);
		U32 *pPrimaryPbHole = performTrimming ? pbOutput + pbSize / sizeof(U32) : pbOutput;
		U32 pbEndPtr = U32(pbOutput) + pbSize + holeSize + outputLocs->m_cmdBufferOffset;
		((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
		((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
		pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
		pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
		pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
		pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
		pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
		pNvControl->m_primaryPbHoleSize = holeSize;
		pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;
		pNvControl->m_numLights = 0;

		// Build discrete PM info.
		DiscretePmInfo * __restrict pDiscretePmInfo = (DiscretePmInfo * __restrict)(U32(pNvControl) + 0x30);
		pDiscretePmInfo->m_alpha = 1.0f - inputInfo->m_lodLevelFractional;
		pDiscretePmInfo->m_padding0 = 0;
		pDiscretePmInfo->m_padding1 = 0;

		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(pDiscretePmInfo) + 0x10));

		// Patch the DMA tags now that we know where the job data will be output.
		tagsStart[0].m_source = U32(jobDataOutput) + U32(mpCmdStart) - U32(jobData);  // Pointer to the command list.
		tagsStart[1].m_source = U32(jobDataOutput) + U32(pNvControl) - U32(jobData);  // Pointer to NV control.
		discretePmInfoTag[0].m_source = U32(jobDataOutput) + U32(pDiscretePmInfo) - U32(jobData);

		// Add the job header to the job list.
		outputLocs->m_jobList->AddJob(SetJobHeader(U32(mpCmd) - U32(jobData) + U32(jobDataOutput)));

		// Output the push buffer data.
		OutputPushBufferWithHole(pCommandContext, pbCmdStart, pbSize, holeSize, performTrimming);
	}

	// Output the job data if the job is to be added.
	if (addJob)
	{
		OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSizeRounded);
	}
}

// Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses
// continuous PM, and optional trimming.
// Trimming is performed if the kDoTrimming flag is set in the processing flags.
// If inputInfo->m_pMeshProgramInfo == NULL, then the vertex set is routed directly to the GPU.
void Ice::Mesh::RenderContinuousPmVertexSet(
	PmVertexSet const * const vertexSet,
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	CutAndPasteInfo * __restrict const cutAndPaste)
{
	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;
	
	U32F lodIndex = inputInfo->m_lodLevel;

	// If there is no mesh program info, then render the PM vertex set directly on the GPU.
	if (inputInfo->m_pMeshProgramInfo == NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		RenderPmVertexSetOnGpu(vertexSet, inputInfo->m_pVertexInputInfo, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, inputInfo->m_gpuSkinningConstantIndex,
				lodIndex, pCommandContext);
		return;
	}

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));

	// Write onto the stack for optimal performance.
	// Allocate space for maximum push buffer size, job info, DMA tags, command list, nvControl, and discrete PM info.
	U32 * __restrict pbCmd = (U32 * __restrict)alloca(0x500);

	// Get the triangle and vertex counts.
	LodVertexTable const *vertexTable = &vertexSet->m_lodVertexTable[lodIndex];
	IndexTable const *indexTable = &vertexSet->m_lodIndexTables[lodIndex];
	U64 triangleCount = indexTable->m_triangleCount;
	U64 haloTriangleCount = indexTable->m_haloTriangleCount;
	U64 vertexCount = vertexTable->m_firstVertex + vertexTable->m_vertexCount;

	// Get PM size.
	U64 pmSize = (sizeof(*vertexSet->m_lodVertexTable) * vertexSet->m_lodVertexTable[0].m_rowCount + 0xF) & ~0xF;

	// Get the processing options.
	bool performTrimming = inputInfo->m_procFlags & kDoTrimming;
	bool performDoubleSidedTrimming = (inputInfo->m_procFlags & kDoubleSided) && performTrimming;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;



	// Build the push buffer.
	U32 *pbCmdStart = pbCmd;    // Remember where the push buffer started.

	// Add the static attribute pointers to the push buffer and get the necessary hole size required to store
	// the attribute pointers for attributes output by mesh processing.
	U64 vertexPtrHoleSize = AddStaticAttributePointersToPushBuffer(&pbCmd, vertexSet->m_streams, inputInfo->m_pVertexInputInfo);

	// If trimming is not performed, then add the draw commands into the push buffer.
	// Otherwise, calculate the hole size necessary to store the draw commands.
	U64 trimHoleSize;
	U64 indexCount = (triangleCount - haloTriangleCount) * 3;
	if (performTrimming)
	{
		trimHoleSize = (10 + (indexCount >> 8)) * 4;
	}
	else
	{
		AddDrawCommandsToPushBuffer(&pbCmd, indexTable->m_indexes.m_ofs, indexCount);
		trimHoleSize = 0;
	}
	U64 holeSize = FinishPushBufferAndGetHoleSize(&pbCmd, vertexPtrHoleSize, trimHoleSize, performTrimming, useRingBuffer);

	// Reserve memory for the push buffer and determine its final output address.
	U64 pbSize = U32(pbCmd) - U32(pbCmdStart);
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, holeSize);



	// Put the list DMA tags at the next 64 byte alignment after the push buffer on the stack.
	U8 *jobData = (U8 *)ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)pbCmd);

	DmaTag *tags = (DmaTag *)jobData;
	DmaTag *tagsStart = tags;         // Remember the start of the DMA tags to patch the command list in later.
	U64 dmaSize = 0;                  // Tracks size of data so far in the input buffer.

	// Build the list DMA tags.
	AddCommandListAndNvControlToListDma(&tags, &dmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToListDma(&tags, &dmaSize, &s_objectInfo[kDoubleSidedObjectInfoNum]);
	}

	DmaTag *trimmingTags = tags;      // Save pointer to trimming tags for cut and paste.
	if (performTrimming)
	{
		AddViewportAndRootTransformToListDma(&tags, &dmaSize, inputInfo->m_pViewport, inputInfo->m_pRootTransform);
	}

	ICE_ASSERT((cutAndPaste != NULL) || (inputInfo->m_pContinuousPmInfo != NULL));
	DmaTag *continuousPmInfoTag = tags;  // Save pointer to continuous PM info tag for cut and paste.
	U64 continuousPmInfoSize = (sizeof(F32) * 4 + (inputInfo->m_lowestLodLevel + 1) * sizeof(F32) * 2 + 0xF) & ~0xF;
	AddContinuousPmInfoToListDma(&tags, &dmaSize, inputInfo->m_pContinuousPmInfo, continuousPmInfoSize);

	U64 alignedDmaSize = AddAlignmentFillingToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable);

	AddLodVertexTableToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable, pmSize);
	AddStreamsToListDma(&tags, &dmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo, vertexSet->m_streamFormatInfo,
		inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams, vertexSet->m_streams + vertexSet->m_nvStreamCount);

	if (performTrimming)
	{
		AddIndexesToListDma(&tags, &dmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6);
	}

	// Loop through all of the used parent tables and upload them.
	U32F lowestLod = inputInfo->m_lowestLodLevel;
	DmaTag *parentTableTags = tags;   // Save pointer to parent table tags for cut and paste.
	for (U32F iLod = lodIndex; iLod <= lowestLod; ++iLod)
	{
		ICE_ASSERT(iLod < vertexSet->m_lodVertexTable[0].m_rowCount);
		U64 parentCount = vertexSet->m_lodVertexTable[iLod].m_vertexCount;

		// Make sure there are parents to upload.
		if (parentCount != 0)
		{
			void const *pParentTable = vertexSet->m_lodParentTables[iLod];
			U64 parentSize = (parentCount * 2 + 0xF) & ~0xF;
			AddParentTableToListDma(&tags, &dmaSize, pParentTable, parentSize);
		}
		else
		{
			// Insert an empty tag as required by cut and paste.
			tags[0].m_size = 0;

			// The source must be set to a mapped memory address as subsequent parent tables may
			// be added and the DMA alignment code may modify this tag to DMA up to 112 bytes extra
			// in order to maintain proper DMA alignment.
			tags[0].m_source = U32(s_objectInfo);
			tags++;
		}
	}

	bool addJob = TestListDmaSize(dmaSize);



	// Get the start of the command list.  The command list starts in memory directly after the DMA tags.
	U16 * __restrict mpCmd = (U16 * __restrict)AlignDmaTags(tags);
	U16 * __restrict mpCmdStart = mpCmd;
	U64 unalignedDmaSize = 0;   // Tracks size of the unaligned data so far in the input buffer.

	// Build the command list
	AddLengthAndNvControlToCommandList(&mpCmd, &unalignedDmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToCommandList(&mpCmd, &unalignedDmaSize);
	}
	AddPmVertexInfoToCommandList(&mpCmd, &alignedDmaSize, vertexSet->m_lodVertexTable, pmSize, lodIndex);
	AddStreamsToCommandList(&mpCmd, &alignedDmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo,
		vertexSet->m_streamFormatInfo, inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams,
		vertexSet->m_streams + vertexSet->m_nvStreamCount);
	if (performTrimming)
	{
		AddIndexesToCommandList(&mpCmd, &alignedDmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6,
			triangleCount, haloTriangleCount);
		AddViewportAndRootTransformToCommandList(&mpCmd, &unalignedDmaSize, inputInfo->m_pRootTransform != NULL);
	}
	AddContinuousPmInfoToCommandList(&mpCmd, &unalignedDmaSize, continuousPmInfoSize);

	// Loop through all of the used parent tables and add them to the command list.
	bool parentTableExists = false;
	for (U32F iLod = lodIndex; iLod <= lowestLod; ++iLod)
	{
		ICE_ASSERT(iLod < vertexSet->m_lodVertexTable[0].m_rowCount);
		U64 parentCount = vertexSet->m_lodVertexTable[iLod].m_vertexCount;

		void *pParentTable = vertexSet->m_lodParentTables[iLod];
		U64 parentSize = (parentCount * 2 + 0xF) & ~0xF;
		AddParentTableToCommandList(&mpCmd, &alignedDmaSize, pParentTable, parentSize, parentCount, iLod);
		parentTableExists = parentTableExists || (parentCount != 0);
	}

	AddEndSetupToCommandList(&mpCmd);
	if (parentTableExists)
	{
		AddPerformPmToCommandList(&mpCmd);
	}
	if (performTrimming)
	{
		AddTrimmingToCommandList(&mpCmd);
	}
	AddOutputStreamsToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_nvOutputMask);
	FinishCommandList(&mpCmd);



	// Calculate the size of the job data, including 0x30 bytes for the job command list, and round this to a 64 byte multiple.
	// NvControl is not generated if cut and paste is being used.
	U64 jobDataSize;
	if (cutAndPaste != NULL)
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x30;
	}
	else
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x60;
	}
	U64 jobDataSizeRounded = (jobDataSize + 0x3F) & ~0x3F;

	// Reserve memory for the job data and determine its final output address.
	ICE_ASSERT(pbSize + jobDataSizeRounded <= 0x500);
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSizeRounded);

	// Calculate the size of the list DMA tags, rounded up to a multiple of 16 bytes.
	U32 tagsSize = (U32(tags) - U32(tagsStart) + 0xF) & ~0xF;

	// Setup the job command list.
	SetJobCommandList(outputLocs->m_spuModule, tagsStart, tagsSize, (WwsJob_Command *)mpCmd, jobData, jobDataOutput);

	// The job will not be added if there is no room left in either the push buffer or the job data buffer.
	addJob &= (pbOutput != NULL) && (jobDataOutput != NULL);



	if (cutAndPaste != NULL)
	{
		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(mpCmd) + 0x30));

		// Set cut and paste info.
		cutAndPaste->m_pushBuffer = addJob ? pbOutput : NULL;
		cutAndPaste->m_pbSize = pbSize;
		cutAndPaste->m_pbHoleSize = holeSize;
		cutAndPaste->m_jobData = jobDataOutput;
		cutAndPaste->m_jobDataSize = jobDataSize;
		cutAndPaste->m_tagsSize = tagsSize;
		cutAndPaste->m_trimmingTagsOffset = performTrimming ? U32(trimmingTags) - U32(jobData) : 0;
		cutAndPaste->m_discretePmInfoTagOffset = 0;
		cutAndPaste->m_continuousPmInfoTagOffset = U32(continuousPmInfoTag) - U32(jobData);
		cutAndPaste->m_dmInfoTagOffset = 0;
		cutAndPaste->m_blendShapeTagOffset = 0;
		cutAndPaste->m_matrixRunsCount = 0;
		cutAndPaste->m_matricesPtrOffset = 0;
		cutAndPaste->m_matrixTagsOffset = 0;
		cutAndPaste->m_numParentTablesForContinuousPm = lowestLod - lodIndex + 1;
		cutAndPaste->m_parentTableTagsOffset = U32(parentTableTags) - U32(jobData);
		cutAndPaste->m_commandListOffset = U32(mpCmdStart) - U32(jobData);

		// Output the push buffer only if the job could be added.
		if (addJob)
		{
			OutputPushBuffer(pCommandContext, pbCmdStart, pbSize);
		}
	}
	else if (addJob)
	{
		// Build NV control.
		NvControl * __restrict pNvControl = (NvControl * __restrict)(U32(mpCmd) + 0x30);
		U32 *pPrimaryPbHole = performTrimming ? pbOutput + pbSize / sizeof(U32) : pbOutput;
		U32 pbEndPtr = U32(pbOutput) + pbSize + holeSize + outputLocs->m_cmdBufferOffset;
		((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
		((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
		pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
		pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
		pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
		pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
		pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
		pNvControl->m_primaryPbHoleSize = holeSize;
		pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;
		pNvControl->m_numLights = 0;



		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(pNvControl) + 0x30));

		// Patch the DMA tags now that we know where the job data will be output.
		tagsStart[0].m_source = U32(jobDataOutput) + U32(mpCmdStart) - U32(jobData);  // Pointer to the command list.
		tagsStart[1].m_source = U32(jobDataOutput) + U32(pNvControl) - U32(jobData);  // Pointer to NV control.

		// Add the job header to the job list.
		outputLocs->m_jobList->AddJob(SetJobHeader(U32(mpCmd) - U32(jobData) + U32(jobDataOutput)));

		// Output the push buffer data.
		OutputPushBufferWithHole(pCommandContext, pbCmdStart, pbSize, holeSize, performTrimming);
	}

	// Output the job data if the job is to be added.
	if (addJob)
	{
		OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSizeRounded);
	}
}

// Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses
// displacement mapping, optional skinning, and optional trimming.
// Skinning is performed if skinning bits are present in the mesh program info.
// Trimming is performed if the kDoTrimming flag is set in the processing flags.
// Blend shapes are not currently supported, but maybe in the future.
void Ice::Mesh::RenderDmVertexSet(
	PmVertexSet const * const vertexSet,
	VertexSetBlendShapes const * const blendShapes,
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	CutAndPasteInfo * __restrict const cutAndPaste)
{
	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;
	
	U32F lodIndex = inputInfo->m_lodLevel;

	// If there is no mesh program info, then render the PM vertex set directly on the GPU.
	if (inputInfo->m_pMeshProgramInfo == NULL)
	{
		ICE_ASSERT(cutAndPaste == NULL);
		RenderPmVertexSetOnGpu(vertexSet, inputInfo->m_pVertexInputInfo, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, inputInfo->m_gpuSkinningConstantIndex,
				lodIndex, pCommandContext);
		return;
	}

	// Blend shapes are not currently supported with DM and we want to quiet the warning about the unused parameter.
	(void)blendShapes;

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));

	// Write onto the stack for optimal performance.
	// Allocate space for maximum push buffer size, job info, DMA tags, command list, nvControl, and discrete PM info.
	U32 * __restrict pbCmd = (U32 * __restrict)alloca(0x500);

	// Get the triangle and vertex counts.
	LodVertexTable const *vertexTable = &vertexSet->m_lodVertexTable[lodIndex];
	IndexTable const *indexTable = &vertexSet->m_lodIndexTables[lodIndex];
	U64 triangleCount = indexTable->m_triangleCount;
	U64 haloTriangleCount = indexTable->m_haloTriangleCount;
	U64 vertexCount = vertexTable->m_firstVertex + vertexTable->m_vertexCount;

	// Get PM and parent sizes.
	U64 pmSize = (sizeof(*vertexSet->m_lodVertexTable) * vertexSet->m_lodVertexTable[0].m_rowCount + 0xF) & ~0xF;
	U64 parentSize = (vertexTable->m_vertexCount * 2 + 0xF) & ~0xF;

	// Get the processing options.
	bool performTrimming = inputInfo->m_procFlags & kDoTrimming;
	bool performSkinning = inputInfo->m_pMeshProgramInfo->m_skinningBits;
	bool performDoubleSidedTrimming = (inputInfo->m_procFlags & kDoubleSided) && performTrimming;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;



	// Build the push buffer.
	U32 *pbCmdStart = pbCmd;    // Remember where the push buffer started.

	// Add the static attribute pointers to the push buffer and get the necessary hole size required to store
	// the attribute pointers for attributes output by mesh processing.
	U64 vertexPtrHoleSize = AddStaticAttributePointersToPushBuffer(&pbCmd, vertexSet->m_streams, inputInfo->m_pVertexInputInfo);

	// If trimming is not performed, then add the draw commands into the push buffer.
	// Otherwise, calculate the hole size necessary to store the draw commands.
	U64 trimHoleSize;
	U64 indexCount = (triangleCount - haloTriangleCount) * 3;
	if (performTrimming)
	{
		trimHoleSize = (10 + (indexCount >> 8)) * 4;
	}
	else
	{
		AddDrawCommandsToPushBuffer(&pbCmd, indexTable->m_indexes.m_ofs, indexCount);
		trimHoleSize = 0;
	}
	U64 holeSize = FinishPushBufferAndGetHoleSize(&pbCmd, vertexPtrHoleSize, trimHoleSize, performTrimming, useRingBuffer);

	// Reserve memory for the push buffer and determine its final output address.
	U64 pbSize = U32(pbCmd) - U32(pbCmdStart);
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, holeSize);



	// Put the list DMA tags at the next 64 byte alignment after the push buffer on the stack.
	U8 *jobData = (U8 *)ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)pbCmd);

	DmaTag *tags = (DmaTag *)jobData;
	DmaTag *tagsStart = tags;         // Remember the start of the DMA tags to patch the command list in later.
	U64 dmaSize = 0;                  // Tracks size of data so far in the input buffer.

	// Build the list DMA tags.
	AddCommandListAndNvControlToListDma(&tags, &dmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToListDma(&tags, &dmaSize, &s_objectInfo[kDoubleSidedObjectInfoNum]);
	}

	DmaTag *trimmingTags = tags;      // Save pointer to trimming tags for cut and paste.
	if (performTrimming)
	{
		AddViewportAndRootTransformToListDma(&tags, &dmaSize, inputInfo->m_pViewport, inputInfo->m_pRootTransform);
	}

	ICE_ASSERT((cutAndPaste != NULL) || (inputInfo->m_pDmInfo != NULL));
	DmaTag *dmInfoTag = tags;
	AddDmInfoToListDma(&tags, &dmaSize, inputInfo->m_pDmInfo);

	U64 alignedDmaSize = AddAlignmentFillingToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable);

	AddLodVertexTableToListDma(&tags, &dmaSize, vertexSet->m_lodVertexTable, pmSize);
	AddStreamsToListDma(&tags, &dmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo, vertexSet->m_streamFormatInfo,
		inputInfo->m_pMeshProgramInfo, vertexCount, vertexSet->m_streams, vertexSet->m_streams + vertexSet->m_nvStreamCount);

	if (performTrimming)
	{
		AddIndexesToListDma(&tags, &dmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6);
	}

	// If a parent table exists at this LOD then add it.
	void const *parentTable = vertexSet->m_lodParentTables[lodIndex];
	if (parentTable != NULL)
	{
		AddParentTableToListDma(&tags, &dmaSize, parentTable, parentSize);
	}

	void const *pDispStream = vertexSet->m_streams[vertexSet->m_streamCount].m_ptr;
	AddDmDisplacementsToListDma(&tags, &dmaSize, pDispStream, vertexCount);

	DmaTag *matrixTags = NULL;       // Save pointer to matrices tag for cut and paste.
	U16 matrixCount = 0;              // Save matrix count for cut and paste.
	if (performSkinning)
	{
		ICE_ASSERT(vertexSet->m_lodSkinningTables != NULL);
		ICE_ASSERT((cutAndPaste != NULL) || (inputInfo->m_pMatrices != NULL));
		AddSkinningTablesToListDma(&tags, &dmaSize, &vertexSet->m_lodSkinningTables[lodIndex], inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, &matrixTags, &matrixCount);
	}

	bool addJob = TestListDmaSize(dmaSize);



	// Get the start of the command list.  The command list starts in memory directly after the DMA tags.
	U16 * __restrict mpCmd = (U16 * __restrict)AlignDmaTags(tags);
	U16 * __restrict mpCmdStart = mpCmd;
	U64 unalignedDmaSize = 0;   // Tracks size of data so far in the input buffer.

	// Build the command list
	AddLengthAndNvControlToCommandList(&mpCmd, &unalignedDmaSize);
	if (performDoubleSidedTrimming)
	{
		AddObjectInfoToCommandList(&mpCmd, &unalignedDmaSize);
	}
	AddPmVertexInfoToCommandList(&mpCmd, &alignedDmaSize, vertexSet->m_lodVertexTable, pmSize, lodIndex);
	AddStreamsToCommandList(&mpCmd, &alignedDmaSize, inputInfo->m_pFixedFormatInfo->m_streamFormatInfo,
		vertexSet->m_streamFormatInfo, inputInfo->m_pMeshProgramInfo,
		vertexCount, vertexSet->m_streams,
		vertexSet->m_streams + vertexSet->m_nvStreamCount);
	if (performTrimming)
	{
		AddIndexesToCommandList(&mpCmd, &alignedDmaSize, indexTable->m_indexes.m_ptr, triangleCount * 6,
			triangleCount, haloTriangleCount);
		AddViewportAndRootTransformToCommandList(&mpCmd, &unalignedDmaSize, inputInfo->m_pRootTransform != NULL);
	}
	if (parentTable != NULL)
	{
		AddDiscretePmInfoForDmToCommandList(&mpCmd);
		AddParentTableToCommandList(&mpCmd, &alignedDmaSize, parentTable, parentSize, vertexTable->m_vertexCount);
	}
	AddDmInfoToCommandList(&mpCmd, &unalignedDmaSize);
	AddDmDisplacementsToCommandList(&mpCmd, &alignedDmaSize, pDispStream, vertexCount);
	if (performSkinning)
	{
		ICE_ASSERT(vertexSet->m_lodSkinningTables != NULL);
		AddSkinningTablesToCommandList(&mpCmd, &alignedDmaSize, &vertexSet->m_lodSkinningTables[lodIndex],
				inputInfo->m_pFixedFormatInfo->m_skinWeightFormat, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, matrixCount);
	}
	AddEndSetupToCommandList(&mpCmd);
	AddPerformDmToCommandList(&mpCmd);
	if (performSkinning)
	{
		AddSkinObjectToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_skinningBits);
	}
	if (performTrimming)
	{
		AddTrimmingToCommandList(&mpCmd);
	}
	AddOutputStreamsToCommandList(&mpCmd, inputInfo->m_pMeshProgramInfo->m_nvOutputMask);
	FinishCommandList(&mpCmd);



	// Calculate the size of the job data, including 0x30 bytes for the job command list, and round this to a 64 byte multiple.
	// NvControl is not generated if cut and paste is being used.
	U64 jobDataSize;
	if (cutAndPaste != NULL)
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x30;
	}
	else
	{
		jobDataSize = U32(mpCmd) - U32(jobData) + 0x60;
	}
	U64 jobDataSizeRounded = (jobDataSize + 0x3F) & ~0x3F;

	// Reserve memory for the job data and determine its final output address.
	ICE_ASSERT(pbSize + jobDataSizeRounded <= 0x500);
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSizeRounded);

	// Calculate the size of the list DMA tags, rounded up to a multiple of 16 bytes.
	U32 tagsSize = (U32(tags) - U32(tagsStart) + 0xF) & ~0xF;

	// Setup the job command list.
	SetJobCommandList(outputLocs->m_spuModule, tagsStart, tagsSize, (WwsJob_Command *)mpCmd, jobData, jobDataOutput);

	// The job will not be added if there is no room left in either the push buffer or the job data buffer.
	addJob &= (pbOutput != NULL) && (jobDataOutput != NULL);



	if (cutAndPaste != NULL)
	{
		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(mpCmd) + 0x30));

		// Set cut and paste info.
		cutAndPaste->m_pushBuffer = addJob ? pbOutput : NULL;
		cutAndPaste->m_pbSize = pbSize;
		cutAndPaste->m_pbHoleSize = holeSize;
		cutAndPaste->m_jobData = jobDataOutput;
		cutAndPaste->m_jobDataSize = jobDataSize;
		cutAndPaste->m_tagsSize = tagsSize;
		cutAndPaste->m_trimmingTagsOffset = performTrimming ? U32(trimmingTags) - U32(jobData) : 0;
		cutAndPaste->m_discretePmInfoTagOffset = 0;
		cutAndPaste->m_continuousPmInfoTagOffset = 0;
		cutAndPaste->m_dmInfoTagOffset = U32(dmInfoTag) - U32(jobData);
		cutAndPaste->m_blendShapeTagOffset = 0;
		cutAndPaste->m_matrixRunsCount = performSkinning ? vertexSet->m_lodSkinningTables[lodIndex].m_matrixRunsCount : 0;
		cutAndPaste->m_matricesPtrOffset = U32(inputInfo->m_pMatrices);
		cutAndPaste->m_matrixTagsOffset = U32(matrixTags) - U32(jobData);
		cutAndPaste->m_numParentTablesForContinuousPm = 0;
		cutAndPaste->m_parentTableTagsOffset = 0;
		cutAndPaste->m_commandListOffset = U32(mpCmdStart) - U32(jobData);

		// Output the push buffer only if the job could be added.
		if (addJob)
		{
			OutputPushBuffer(pCommandContext, pbCmdStart, pbSize);
		}
	}
	else if (addJob)
	{
		// Build NV control.
		NvControl * __restrict pNvControl = (NvControl * __restrict)(U32(mpCmd) + 0x30);
		U32 *pPrimaryPbHole = performTrimming ? pbOutput + pbSize / sizeof(U32) : pbOutput;
		U32 pbEndPtr = U32(pbOutput) + pbSize + holeSize + outputLocs->m_cmdBufferOffset;
		((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
		((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
		pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
		pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
		pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
		pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
		pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
		pNvControl->m_primaryPbHoleSize = holeSize;
		pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;
		pNvControl->m_numLights = 0;



		// Align job data output to 64 bytes.
		ZeroMemoryTo64ByteAlignmentFrom16ByteAlignment((VU8 *)(U32(pNvControl) + 0x30));

		// Patch the DMA tags now that we know where the job data will be output.
		tagsStart[0].m_source = U32(jobDataOutput) + U32(mpCmdStart) - U32(jobData);  // Pointer to the command list.
		tagsStart[1].m_source = U32(jobDataOutput) + U32(pNvControl) - U32(jobData);  // Pointer to NV control.

		// Add the job header to the job list.
		outputLocs->m_jobList->AddJob(SetJobHeader(U32(mpCmd) - U32(jobData) + U32(jobDataOutput)));

		// Output the push buffer data.
		OutputPushBufferWithHole(pCommandContext, pbCmdStart, pbSize, holeSize, performTrimming);
	}

	// Output the job data if the job is to be added.
	if (addJob)
	{
		OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSizeRounded);
	}
}

// Sets up all of the mesh processing job data and push buffer contents for a vertex set that uses
// a provided mesh program.
// It is necessary to set the kDoTrimming flag in the processing flags if trimming is to be done.
// It is necessary to set the kDoShadowVolumeExtrusion flag in the processing flags if stencil shadows are to be used.
// Blend shapes are used if the pointer to the blend shapes is valid.
// If inputInfo->m_pMeshProcCommandList == NULL, then the vertex set is routed directly to the GPU.
void Ice::Mesh::RenderGeneralPmVertexSet(
	PmVertexSet const * const vertexSet,
	VertexSetBlendShapes const * const blendShapes,
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	ShadowOutputLocations * __restrict const shadowOutputLocs)
{
	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;
	
	// Calculate LOD values.
	U32F lodIndex = inputInfo->m_lodLevel;

	// If there is no mesh program command list, then render the PM vertex set directly on the GPU.
	U16 const *meshProcCommandList = inputInfo->m_pMeshProcCommandList;
	if (meshProcCommandList == NULL)
	{
		RenderPmVertexSetOnGpu(vertexSet, inputInfo->m_pVertexInputInfo, inputInfo->m_pMatrices,
				inputInfo->m_pFixedFormatInfo->m_matrixFormat, inputInfo->m_gpuSkinningConstantIndex,
				lodIndex, pCommandContext);
		return;
	}

	// Perform blend shapes, if they exist.
	DmaTag const *pBlendShapeDmaTags = NULL;
	if (blendShapes != NULL)
	{
		pBlendShapeDmaTags = GenerateBlendShapeData(blendShapes, inputInfo->m_deltaStreamUsageBits, lodIndex,
				inputInfo->m_pBlendShapeFactors, outputLocs->m_jobDataAllocator);
	}

	// Align the stack to 64 bytes.
	U32 stack = U32(alloca(0));
	alloca(0x40 - (stack & 0x3F));

	// Write onto the stack for optimal performance.
	U32 * __restrict pbCmd = (U32 * __restrict)alloca(0x180);
	U8 *jobData = (U8 *)alloca(0x300);
	DmaTag *tags = (DmaTag *)jobData;
	U16 * __restrict mpCmd = (U16 * __restrict)alloca(0x140);   // 0x100 bytes with extra for padding.
	U8 *smallTableData = (U8 *)alloca(0x100);

	IndexTable const *indexTable = &vertexSet->m_lodIndexTables[lodIndex];
	U64 triangleCount = indexTable->m_triangleCount;
	U64 haloTriangleCount = indexTable->m_haloTriangleCount;
	LodVertexTable const *vertexTable = &vertexSet->m_lodVertexTable[lodIndex];
	U64 vertexCount = vertexTable->m_firstVertex + vertexTable->m_vertexCount;
	U64 haloVertexCount = vertexTable->m_haloVertexCount;
	SkinningTables const *pSkinningTables = &vertexSet->m_lodSkinningTables[lodIndex];

	// Get the processing options.
	bool performTrimming = inputInfo->m_procFlags & kDoTrimming;
	bool isDoubleSided = inputInfo->m_procFlags & kDoubleSided;
	bool performShadowVolumeExtrusion = inputInfo->m_procFlags & kDoShadowVolumeExtrusion;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;



	// Build the push buffer.
	U32 *pbCmdStart = pbCmd;    // Remember where the push buffer started.

	// Add the static attribute pointers to the push buffer and get the necessary hole size required to store
	// the attribute pointers for attributes output by mesh processing.
	U64 vertexPtrHoleSize = AddStaticAttributePointersToPushBuffer(&pbCmd, vertexSet->m_streams, inputInfo->m_pVertexInputInfo);

	// If trimming is not performed, then add the draw commands into the push buffer.
	// Otherwise, calculate the hole size necessary to store the draw commands.
	U64 trimHoleSize;
	U64 indexCount = (triangleCount - haloTriangleCount) * 3;
	if (performTrimming)
	{
		trimHoleSize = (10 + (indexCount >> 8)) * 4;
	}
	else
	{
		AddDrawCommandsToPushBuffer(&pbCmd, indexTable->m_indexes.m_ofs, indexCount);
		trimHoleSize = 0;
	}
	U64 holeSize = FinishPushBufferAndGetHoleSize(&pbCmd, vertexPtrHoleSize, trimHoleSize, performTrimming, useRingBuffer);

	// Reserve memory for the push buffer and determine its final output address.
	U64 pbSize = U32(pbCmd) - U32(pbCmdStart);
	ICE_ASSERT(pbSize <= 0x180);
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, holeSize);

	// Leave if out of memory.
	if (pbOutput == NULL)
	{
		return;
	}



	// Reserve memory for the maximum size of the small data tables.
	// This is done here so we know where they will go ahead of time and can store their addresses during construction.
	U8 *smallTableDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, 0xA0);

	// Leave if out of memory.
	if (smallTableDataOutput == NULL)
	{
		return;
	}

	U8 *smallTableDataStart = smallTableData;
	DmaTag *tagsStart = tags;
	U16 *mpCmdStart = mpCmd;

	// Add command list to DMA list.
	tags[0].m_size = 0x100;
	tags[0].m_source = 0;      // This can't be filled in here because we don't know where mpCmd starts.
	tags++;
	U64 dmaSize = 0x100;

	// Put the size of the command list at the start of the list.
	mpCmd[0] = 0x100;
	mpCmd++;

	// Get the base pointer for the custom compression/decompression info structures.
	U32 customCodecInfosPtr = U32(meshProcCommandList);

	U16 cmd;
	bool continuousPmExists = false;
	while((cmd = *meshProcCommandList++) != (kCmdCleanupAndExit << 8))
	{
		// Extract the command.
		U8 const cmdByte = cmd >> 8;
		switch(cmdByte)
		{
			// SetupNvControlInfo - This command is responsible for transferring the
			// data associated with general processing and is always in the command
			// list.
		case kCmdSetupNvControlInfo:
			{
				// Build the NV control structure.
				NvControl *pNvControl = (NvControl * __restrict)smallTableData;
				U32 *pPrimaryPbHole = (!performTrimming) ? pbOutput : pbOutput + pbSize / sizeof(U32);
				U32 pbEndPtr = U32(pbOutput) + pbSize + holeSize + outputLocs->m_cmdBufferOffset;
				((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
				((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
				pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
				pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
				pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
				pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
				pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
				pNvControl->m_primaryPbHoleSize = holeSize;
				pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;

				U64 nvControlSize;

				// Create the push buffer holes and add their addresses to the NV control structure for the shadows
				// extruded for each light, if shadow volume extrusion is turned on.
				if (performShadowVolumeExtrusion)
				{

					U32 const numPointLights = inputInfo->m_numPointLights;
					U32 const numDirectionalLights = inputInfo->m_numDirectionalLights;
					pNvControl->m_numLights = numPointLights + numDirectionalLights;

					NvControlLight * __restrict l = &pNvControl->m_perLightData[0];
					ShadowInfo const *si = inputInfo->m_pShadowInfos;
					U64 profileSize = ((10 + ((triangleCount*3*4) >> 8)) * 4 + 0xF) & ~0xF;
					U64 capSize = ((14 + ((triangleCount*3) >> 8)) * 4 + 0xF) & ~0xF;
					U64 light = 0;
					for (U32F i = 0; i < numPointLights; ++i, ++si, ++l, ++light)
					{
						// Reserve memory for the profile edges in its correspondng push buffer for this light.
						U32 *profileOutput = ReserveMemoryForAndAlignPushBuffer(
								shadowOutputLocs->m_profileCommandContext[light], 0, profileSize);

						// Leave if out of memory.
						if (profileOutput == NULL)
						{
							return;
						}

						l->m_profilePBHole = U32(profileOutput) + outputLocs->m_cmdBufferOffset;
						l->m_profilePBHoleSize = profileSize;
						OutputPushBufferWithHole(shadowOutputLocs->m_profileCommandContext[light], NULL, 0, profileSize, true);

						if (LIKELY(si->m_outputShadowCaps))
						{
							// Reserve memory for the caps in its correspondng push buffer for this light.
							U32 *frontCapOutput = ReserveMemoryForAndAlignPushBuffer(
									shadowOutputLocs->m_frontCapCommandContext[light], 0, capSize);
							U32 *backCapOutput = ReserveMemoryForAndAlignPushBuffer(
									shadowOutputLocs->m_backCapCommandContext[light], 0, capSize);

							// Leave if out of memory.
							if ((frontCapOutput == NULL) || (backCapOutput == NULL))
							{
								return;
							}

							l->m_frontCapPBHole = U32(frontCapOutput) + outputLocs->m_cmdBufferOffset;
							l->m_backCapPBHole = U32(backCapOutput) + outputLocs->m_cmdBufferOffset;
							l->m_frontCapPBHoleSize = capSize;
							l->m_backCapPBHoleSize = capSize;
							OutputPushBufferWithHole(shadowOutputLocs->m_frontCapCommandContext[light], NULL, 0, capSize, true);
							OutputPushBufferWithHole(shadowOutputLocs->m_backCapCommandContext[light], NULL, 0, capSize, true);
						}
						else
						{
							l->m_frontCapPBHoleSize = 0;
							l->m_backCapPBHoleSize = 0;
						}
					}

					for (U32F i = 0; i < numDirectionalLights; ++i, ++si, ++l, ++light)
					{
						// Reserve memory for the profile edges in its correspondng push buffer for this light.
						U32 *profileOutput = ReserveMemoryForAndAlignPushBuffer(
								shadowOutputLocs->m_profileCommandContext[light], 0, profileSize);

						// Leave if out of memory.
						if (profileOutput == NULL)
						{
							return;
						}

						l->m_profilePBHole = U32(profileOutput) + outputLocs->m_cmdBufferOffset;
						l->m_profilePBHoleSize = profileSize;
						OutputPushBufferWithHole(shadowOutputLocs->m_profileCommandContext[light], NULL, 0, profileSize, true);

						if (LIKELY(si->m_outputShadowCaps))
						{
							// Reserve memory for the caps in its correspondng push buffer for this light.
							U32 *frontCapOutput = ReserveMemoryForAndAlignPushBuffer(
									shadowOutputLocs->m_frontCapCommandContext[light], 0, capSize);

							// Leave if out of memory.
							if (frontCapOutput == NULL)
							{
								return;
							}

							l->m_frontCapPBHole = U32(frontCapOutput) + outputLocs->m_cmdBufferOffset;
							l->m_frontCapPBHoleSize = capSize;
							OutputPushBufferWithHole(shadowOutputLocs->m_frontCapCommandContext[light], NULL, 0, capSize, true);
						}
						else
						{
							l->m_frontCapPBHoleSize = 0;
						}
						l->m_backCapPBHoleSize = 0;
					}

					// Set the size of the NV control structure to its full size.
					nvControlSize = 0x90;
				}
				else
				{
					// Since shadow volumes are not being extruded, set the number of lights to zero.
					pNvControl->m_numLights = 0;

					// Set the size of the NV control structure to its size without the shadow information.
					nvControlSize = 0x30;
				}

				// Increment the small table data pointer by the size of the NV control structure.
				smallTableData += nvControlSize;

				U32 sourcePtr = U32(smallTableDataOutput) + U32(pNvControl) - U32(smallTableDataStart);
				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, (void *)sourcePtr, nvControlSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, (void *)sourcePtr, nvControlSize);
			}
			break;
			// SetupObjectInfo command.
			// This command is responsible for setting up less commonly used mesh processing data.
			// NOTE: This will automatically setup an object info with double sided info set and nothing else.
		case kCmdSetupObjectInfo:
			{
				mpCmd[0] = cmd;
				mpCmd[1] = dmaSize;
				mpCmd += 2;

				tags[0].m_size = sizeof(ObjectInfo);
				if (isDoubleSided)
				{
					tags[0].m_source = U32(&s_objectInfo[kDoubleSidedObjectInfoNum]);
				}
				else
				{
					tags[0].m_source = U32(&s_objectInfo[kSingleSidedObjectInfoNum]);
				}
				tags++;

				dmaSize += sizeof(ObjectInfo);
			}
			break;
			// SetupVertexInfo command.
			// This command is responsible for letting mesh processing know how many
			// vertices and halo vertices there are.
		case kCmdSetupVertexInfo:
			{
				// Actually Setup the command in the mesh processing command
				// stream.
				mpCmd[0] = cmd;
				mpCmd[1] = vertexCount;
				mpCmd[2] = haloVertexCount;
				mpCmd += 3;
			}
			break;
			// SetupPmVertexInfo command.
			// This command is responsible for informing mesh processing about
			// Pm related data.
		case kCmdSetupPmVertexInfo:
			{
				// Actually Setup the command in the mesh processing command
				// stream. In its most efficient this form involves directly
				// copying the first 16-bits (in the command list) in the command
				// list. Followed by the input buffer address to the lod vertex
				// table structure.
				U16 rowCount = vertexSet->m_lodVertexTable[0].m_rowCount;
				U64 pmSize = (sizeof(*vertexSet->m_lodVertexTable) * rowCount + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, vertexSet->m_lodVertexTable, pmSize, dmaSize);
				mpCmd[2] = lodIndex;
				mpCmd += 3;

				WriteAlignedDmaTag(&tags, &dmaSize, vertexSet->m_lodVertexTable, pmSize);
			}
			break;
			// SetupNvStream command.
			// This command is responsible for informing mesh processing about a
			// paticular nvidia stream it is supposed to process. The command stream
			// may contain many of these commands.
		case kCmdSetupNvStream:
			{
				// Extract the ID from the command.
				U32 id = (cmd & 0xF0) >> 4;

				// Add the Stream Data to the dma list.
				FixedStreamFormatInfo const *pFixedStreamFormatInfo = inputInfo->m_pFixedFormatInfo->m_streamFormatInfo;
				void const *pStream = vertexSet->m_streams[id].m_ptr;
				U16 formatOffset = pFixedStreamFormatInfo->m_formatOffsets[id];
				NvStreamFormat const *nvStreamFormat = (NvStreamFormat const *)((U8 *)pFixedStreamFormatInfo + formatOffset);
				U64 streamSize = (vertexCount * nvStreamFormat->m_elementSize + 0xF) & ~0xF;

				// Extract the decompression flags from the next command.
				U16 decompressFlags = *meshProcCommandList++;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, pStream, streamSize, dmaSize);
				mpCmd[2] = decompressFlags;

				WriteAlignedDmaTags32K(&tags, &dmaSize, pStream, streamSize);

				// Add custom decompression code, if it exists.
				U16 customDecompInfoOffset = *meshProcCommandList++;
				if (customDecompInfoOffset != 0)
				{
					// Get the custom compression code size and pointer.
					CustomCodecInfoGeneral *pCustomDecompInfo =
						(CustomCodecInfoGeneral *)(customCodecInfosPtr + customDecompInfoOffset);
					U32 customDecompSize = pCustomDecompInfo->m_codeSize;
					void *pCustomDecompCode = (void *)pCustomDecompInfo->m_codePtr;
					ICE_ASSERT(customDecompSize > 0);
					ICE_ASSERT((customDecompSize & 0xF) == 0);
					ICE_ASSERT(pCustomDecompCode != NULL);

					WriteAlignedCmdListOffset(mpCmd + 3, pCustomDecompCode, customDecompSize, dmaSize);
					WriteAlignedDmaTag(&tags, &dmaSize, pCustomDecompCode, customDecompSize);
				}
				else
				{
					mpCmd[3] = 0;
				}
				mpCmd += 4;
			}
			break;
			// SetupSwStream command.
			// This command is responsible for informing mesh processing about a
			// paticular software stream it is supposed to process. The command stream
			// may contain many of these commands.
		case kCmdSetupSwStream:
			{
				// Extract the ID from the command.
				U32 id = (cmd & 0xF0) >> 4;

				// Add the Stream Data to the dma list.
				VariableStreamFormatInfo const *pVarStreamInfo = vertexSet->m_streamFormatInfo;
				void const *pStream = vertexSet->m_streams[id + vertexSet->m_nvStreamCount].m_ptr; // SW stream pointers follow NV stream pointers
				ICE_ASSERT(pVarStreamInfo);
				SwStreamFormat const *swStreamFormat = &pVarStreamInfo->m_streamFormats[id];
				U32 totalBitCount = swStreamFormat->m_componentBitcounts[0] + swStreamFormat->m_componentBitcounts[1] +
					swStreamFormat->m_componentBitcounts[2] + swStreamFormat->m_componentBitcounts[3];
				U32 streamBitCount = vertexCount * totalBitCount;
				U64 streamSize = ((streamBitCount >> 3) + (streamBitCount & 0x7 ? 1 : 0) + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, pStream, streamSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTags32K(&tags, &dmaSize, pStream, streamSize);
			}
			break;
			// SetupCustomCompress - This command is responsible for setting up a custom compression
			// routine for an output NV stream.
		case kCmdSetupCustomCompress:
			{
				// Get the custom compression code size and pointer.
				U16 customCompInfoOffset = *meshProcCommandList++;
				ICE_ASSERT(customCompInfoOffset != 0);
				CustomCodecInfoGeneral *pCustomCompInfo =
					(CustomCodecInfoGeneral *)(customCodecInfosPtr + customCompInfoOffset);
				U32 customCompSize = pCustomCompInfo->m_codeSize;
				void *pCustomCompCode = (void *)pCustomCompInfo->m_codePtr;
				ICE_ASSERT(customCompSize > 0);
				ICE_ASSERT((customCompSize & 0xF) == 0);
				ICE_ASSERT(pCustomCompCode != NULL);

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, pCustomCompCode, customCompSize, dmaSize);
				mpCmd[2] = customCompSize;
				mpCmd += 3;

				WriteAlignedDmaTag(&tags, &dmaSize, pCustomCompCode, customCompSize);
			}
			break;
			// SetupFormatInfo - This command is responsible for transferring the
			// data associated with general processing and is always in the command
			// list.
		case kCmdSetupFormatInfo:
			{
				// Add the fixed stream format info structure to the dma list, if present.
				mpCmd[0] = cmd;

				FixedStreamFormatInfo const *pFixedStreamFormatInfo = inputInfo->m_pFixedFormatInfo->m_streamFormatInfo;
				if (pFixedStreamFormatInfo != NULL)
				{
					U64 fixedFormatSize = (pFixedStreamFormatInfo->m_length + 0xF) & ~0xF;

					WriteAlignedCmdListOffset(mpCmd + 1, pFixedStreamFormatInfo, fixedFormatSize, dmaSize);

					WriteAlignedDmaTag(&tags, &dmaSize, pFixedStreamFormatInfo, fixedFormatSize);
				}
				else
				{
					mpCmd[1] = 0;
				}

				// Add the variable stream format info structure to the dma list, if present.
				VariableStreamFormatInfo const *pVarStreamInfo = vertexSet->m_streamFormatInfo;
				if (pVarStreamInfo != NULL)
				{
					U64 varFormatSize = (pVarStreamInfo->m_length + 0xF) & ~0xF;

					WriteAlignedCmdListOffset(mpCmd + 2, pVarStreamInfo, varFormatSize, dmaSize);

					WriteAlignedDmaTag(&tags, &dmaSize, pVarStreamInfo, varFormatSize);
				}
				else
				{
					mpCmd[2] = 0;
				}

				mpCmd += 3;
			}
			break;
			// SetupIndexes command.
			// This command is responsible for informing mesh processing about a paticular nvidia index
			// data stream and information about it.
		case kCmdSetupIndexes:
			{
				// Setup the index buffer stream for transport into the input
				// buffer.
				void const *indexes = indexTable->m_indexes.m_ptr;
				U64 indexSize = (triangleCount * 6 + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, indexes, indexSize, dmaSize);
				mpCmd[2] = triangleCount;
				mpCmd[3] = haloTriangleCount;
				mpCmd += 4;

				WriteAlignedDmaTag(&tags, &dmaSize, indexes, indexSize);
			}
			break;
			// SetupViewportInfo command.
			// This command is responsible for informing mesh processing about viewport
			// related information.
		case kCmdSetupViewportInfo:
			{
				U64 viewportSize = (sizeof(ViewportInfo) + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, inputInfo->m_pViewport, viewportSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, inputInfo->m_pViewport, viewportSize);
			}
			break;
			// SetupRootTransformInfo command.
			// This command is responsible for informing mesh processing about the
			// object it is processing's model matrix.
		case kCmdSetupRootTransformInfo:
			{
				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, inputInfo->m_pRootTransform, 0x40, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, inputInfo->m_pRootTransform, 0x40);
			}
			break;
			// SetupShadowInfo command.
			// This command is responsible for informing mesh processing about
			// various stencil shadow information.
		case kCmdSetupShadowInfo:
			{
				U64 shadowInfosSize = (sizeof(ShadowInfo) * 4 + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, inputInfo->m_pShadowInfos, shadowInfosSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, inputInfo->m_pShadowInfos, shadowInfosSize);
			}
			break;
			// SetupEdges command.
			// This command is responsible for transferring the edge connectivity
			// information necessary for stencil shadow volume extrusion to mesh
			// processing.
		case kCmdSetupEdges:
			{
				EdgeList const *pEdgeList = &vertexSet->m_lodEdgeLists[lodIndex];
				U16 edgeCount = pEdgeList->m_edgeCount;
				U64 edgeSize = (edgeCount * sizeof(U32) + 0xF) & ~0xF;
				void *pEdgeTable = pEdgeList->m_edgeTable;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, pEdgeTable, edgeSize, dmaSize);
				mpCmd[2] = edgeCount;
				mpCmd += 3;

				WriteAlignedDmaTag(&tags, &dmaSize, pEdgeTable, edgeSize);
			}
			break;
			// SetupContinuousPmInfo command.
			// This command is responsible for transferring the information required
			// for continuous progressive mesh processing.
		case kCmdSetupContinuousPmInfo:
			{
				// Only need to transfer the table up to the lowest LOD used.
				U64 continuousPmInfoSize = (sizeof(F32) * 4 + (inputInfo->m_lowestLodLevel + 1) * sizeof(F32) * 2 + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, inputInfo->m_pContinuousPmInfo, continuousPmInfoSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, inputInfo->m_pContinuousPmInfo, continuousPmInfoSize);

				continuousPmExists = true;
			}
			break;
			// SetupDiscretePmInfo command.
			// This command is responsible for transferring the information required
			// for discrete progressive mesh processing.
		case kCmdSetupDiscretePmInfo:
			{
				DiscretePmInfo * __restrict pDiscretePmInfo = (DiscretePmInfo * __restrict)smallTableData;
				pDiscretePmInfo->m_alpha = 1.0f - inputInfo->m_lodLevelFractional;
				pDiscretePmInfo->m_padding0 = 0;
				pDiscretePmInfo->m_padding1 = 0;
				U64 pmInfoSize = (sizeof(DiscretePmInfo) + 0xF) & ~0xF;
				smallTableData += pmInfoSize;

				U32 sourcePtr = U32(smallTableDataOutput) + U32(pDiscretePmInfo) - U32(smallTableDataStart);

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, (void *)sourcePtr, pmInfoSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, (void *)sourcePtr, pmInfoSize);
			}
			break;
			// SetupPmParent command.
			// This command is responsible for transferring the information describing
			// the parent vertex relationship necessary for both continuous and
			// discrete progressive mesh processing.
		case kCmdSetupPmParent:
			{
				// If we are transferring CPM parents...
				if (continuousPmExists)
				{
					// Loop through all of the used parent lists and upload them.
					U64 lowestLod = inputInfo->m_lowestLodLevel;
					for (U32F iLod = lodIndex; iLod <= lowestLod; ++iLod)
					{
						ICE_ASSERT(iLod < vertexSet->m_lodVertexTable[0].m_rowCount);
						U16 parentCount = vertexSet->m_lodVertexTable[iLod].m_vertexCount;

						// Make sure there are parents to upload.
						if(parentCount != 0)
						{
							void const *pParentTable = vertexSet->m_lodParentTables[iLod];
							U64 parentSize = (parentCount * 2 + 0xF) & ~0xF;

							// Actually Setup the command in the mesh processing command stream.
							// Tack on extra information about which LOD this parent data belongs to so that mesh
							// processing knows where to put it.
							mpCmd[0] = cmd | (iLod << 4);
							WriteAlignedCmdListOffset(mpCmd + 1, pParentTable, parentSize, dmaSize);
							mpCmd[2] = parentCount;
							mpCmd += 3;

							WriteAlignedDmaTag(&tags, &dmaSize, pParentTable, parentSize);
						}
					}
				}
				// If we are transferring DPM parents...
				else
				{
					void const *pParentTable = vertexSet->m_lodParentTables[lodIndex];
					U16 parentCount = vertexTable->m_vertexCount;
					U64 parentSize = (parentCount * 2 + 0xF) & ~0xF;

					mpCmd[0] = cmd;
					WriteAlignedCmdListOffset(mpCmd + 1, pParentTable, parentSize, dmaSize);
					mpCmd[2] = parentCount;
					mpCmd += 3;

					WriteAlignedDmaTag(&tags, &dmaSize, pParentTable, parentSize);
				}
			}
			break;
			// SetupDmInfo command.
			// This command is responsible for transferring the information
			// required to do displacement map processing.
		case kCmdSetupDmInfo:
			{
				U64 dmInfoSize = (sizeof(DmInfo) + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, inputInfo->m_pDmInfo, dmInfoSize, dmaSize);
				mpCmd += 2;

				WriteAlignedDmaTag(&tags, &dmaSize, inputInfo->m_pDmInfo, dmInfoSize);
			}
			break;
			// SetupDmDisplacements command.
			// This command is responsible for transferring the per-vertex
			// displacement information necessary for displacement map
			// processing.
		case kCmdSetupDmDisplacements:
			{
				void const *pDispStream = vertexSet->m_streams[vertexSet->m_streamCount].m_ptr;
				U32 dispSizeOrg = vertexCount * sizeof(F32);
				U64 dispSize = (dispSizeOrg + 0xF) & ~0xF;

				mpCmd[0] = cmd;
				WriteAlignedCmdListOffset(mpCmd + 1, pDispStream, dispSize, dmaSize);
				mpCmd[2] = dispSizeOrg;
				mpCmd += 3;

				WriteAlignedDmaTag(&tags, &dmaSize, pDispStream, dispSize);
			}
			break;
			// SetupSkinning command.
			// This command is responsible for transferring information
			// required to perform skeletal mesh skinning on the data.
		case kCmdSetupSkinning:
			{
				U64 controlSize = (pSkinningTables->m_controlStreamLength + 0xF) & ~0xF;
				U64 sameSize = (pSkinningTables->m_sameStreamLength + 0xF) & ~0xF;
				U64 diffSize = (pSkinningTables->m_diffStreamLength + 0xF) & ~0xF;
				U64 weightsSize = (pSkinningTables->m_weightsStreamLength + 0xF) & ~0xF;

				mpCmd[0] = cmd;

				// The control codes tell MP what to do.
				WriteAlignedCmdListOffset(mpCmd + 1, pSkinningTables->m_controlStream, controlSize, dmaSize);
				WriteAlignedDmaTag(&tags, &dmaSize, pSkinningTables->m_controlStream, controlSize);
				mpCmd[2] = pSkinningTables->m_controlStreamLength;

				// The same tells MP what to do when a same code is encountered.
				WriteAlignedCmdListOffset(mpCmd + 3, pSkinningTables->m_sameStream, sameSize, dmaSize);
				WriteAlignedDmaTag(&tags, &dmaSize, pSkinningTables->m_sameStream, sameSize);
				mpCmd[4] = pSkinningTables->m_sameStreamLength;

				// The diff tells MP what to do when a diff code is encountered.
				WriteAlignedCmdListOffset(mpCmd + 5, pSkinningTables->m_diffStream, diffSize, dmaSize);
				WriteAlignedDmaTag(&tags, &dmaSize, pSkinningTables->m_diffStream, diffSize);
				mpCmd[6] = pSkinningTables->m_diffStreamLength;

				// The weights hold the weighting information with regards to
				// both diff and same codes.
				WriteAlignedCmdListOffset(mpCmd + 7, pSkinningTables->m_weightsStream, weightsSize, dmaSize);
				WriteAlignedDmaTag(&tags, &dmaSize, pSkinningTables->m_weightsStream, weightsSize);

				// Aux weights is for blending out the contribution of certain
				// weights for skeletal lod.
				if (pSkinningTables->m_auxWeightsStream)
				{
					WriteAlignedCmdListOffset(mpCmd + 8, pSkinningTables->m_auxWeightsStream, weightsSize, dmaSize);
					WriteAlignedDmaTag(&tags, &dmaSize, pSkinningTables->m_auxWeightsStream, weightsSize);
				}
				else
				{
					mpCmd[8] = 0;
				}

				mpCmd[9] = pSkinningTables->m_weightsStreamLength;
				mpCmd += 10;
			}
			break;
			// SetupMatrices command.
			// This command is responsible for transferring an array of 4x3 or 4x4 matrices
			// to mesh processing for use in skeletal mesh skinning.
		case kCmdSetupMatrices:
			{
				mpCmd[0] = cmd;
				mpCmd[1] = dmaSize;

				// Determine the size of the matrices.
				U8 matrixFormat = cmd & 0xF;
				U64 matrixSize;
				if (matrixFormat == kFormatMatrix43)
				{
					matrixSize = 0x30;
				}
				else
				{
					ICE_ASSERT(matrixFormat == kFormatMatrix44);
					matrixSize = 0x40;
				}

				// Transfer all of the matrix runs.
				MatrixRun const *run = pSkinningTables->m_matrixRuns;
				U64 matrixCount = 0;
				U64 matrixRunsCount = pSkinningTables->m_matrixRunsCount;
				ICE_ASSERT(inputInfo->m_pMatrices);
				for (U32F iRun = 0; iRun < matrixRunsCount; ++iRun, ++run)
				{
					U64 count = run->m_length;
					matrixCount += count;
					tags[0].m_size = count * matrixSize;
					tags[0].m_source = U32(&inputInfo->m_pMatrices[run->m_firstIndex * (matrixSize / sizeof(F32))]);
					tags++;
				}

				mpCmd[2] = matrixCount;
				mpCmd += 3;

				dmaSize += matrixCount * matrixSize;
			}
			break;
			// InsertAttributeIntoNvStream command.
			// This command is responsible for converting uniform tables back into
			// their nv format counterpart and then inserting a single uniform table
			// back into the original nv stream.
		case kCmdInsertAttributeIntoNvStream:
			{
				// Actually Setup the command in the mesh processing command
				// stream.
				mpCmd[0] = cmd;
				mpCmd[1] = *meshProcCommandList++;
				mpCmd += 2;
			}
			break;
			// EndSetup command.
			// This is the command to specify that all setup commands have been
			// issued and any intermediate processing on the data can now be done.
		case kCmdEndSetup:
			// ExtrudeShadows command.
			// This command Extrudes profile edges and generates shadow caps.
		case kCmdExtrudeShadows:
			// PerformDm command.
			// This command performs Displacement Mapping on the uniform tables.
		case kCmdPerformDm:
			// SkinObject command.
			// This command performs Skeletal Mesh Skinning on the uniform tables.
		case kCmdSkinObject:
			// PerformPm command.
			// This command performs Progressive Meshing on the uniform tables.
		case kCmdPerformPm:
			// TrimIndexes command.
			// This command trims the indexes that do not contribute to the final
			// image on the screen.
		case kCmdTrimIndexes:
			// TrimVertexes command.
			// This command trims the vertexes that do not contribute to the final
			// image on the screen.
		case kCmdTrimVertexes:
			// OutputIndexes command.
			// This command outputs the indexes at this stage in the processing.
		case kCmdOutputIndexes:
			// OutputCopiedNvStream command.
			// This command outputs a given gpu stream exactly as it is stored
			// in local storage memory, perhaps after modification of some kind.
		case kCmdOutputCopiedNvStream:
			// OutputConvertedUniformTable command.
			// This command converts the internal set of uniform tables back into
			// a gpu consumable stream and outputs the data.
		case kCmdOutputConvertedUniformTable:
			{
				// Actually Setup the command in the mesh processing command stream.
				mpCmd[0] = cmd;
				mpCmd++;
			}
			break;
		case kCmdStartInputListDma: // Used in compiled command list as 'do blend shapes' placeholder
			{
				if (pBlendShapeDmaTags)
				{
					tags[0].m_size = 0x30;
					tags[0].m_source = U32(pBlendShapeDmaTags);
					tags++;

					mpCmd[0] = kCmdStartInputListDma << 8;
					mpCmd[1] = dmaSize;
					mpCmd[2] = 0x28;
					mpCmd[3] = 0x6000;
					mpCmd[4] = kCmdStallOnInputDma << 8;
					mpCmd[5] = kCmdCall << 8;
					mpCmd[6] = 0x6000;
					mpCmd += 7;

					dmaSize += 0x30;
				}
			}
			break;
			// SetupPixelShader command.
			// This command is not implemented yet and should not be used.
		case kCmdSetupPixelShader:
			{
				// TODO
			}
			break;
		}
	}

	// Make sure the command list is terminated properly.
	mpCmd[0] = kCmdCleanupAndExit << 8;
	mpCmd++;

	// Align the list DMA tags to the next quadword.
	tags = AlignDmaTags(tags);

	// Align mpCmd up to the next quadword.
	while ((U32(mpCmd) & 0xF) != 0) *mpCmd++ = 0;
	ICE_ASSERT(U32(mpCmd) - U32(mpCmdStart));



	// Output the push buffer.
	OutputPushBufferWithHole(pCommandContext, pbCmdStart, pbSize, holeSize, performTrimming);

	// Output the small data tables.
	U64 smallTableDataSize = U32(smallTableData) - U32(smallTableDataStart);
	ICE_ASSERT(smallTableDataSize <= 0x100);
	OutputJobData(outputLocs->m_jobDataAllocator, smallTableDataStart, smallTableDataSize);

	// Pad out the command list so that the total size of the command list and the small data tables is a 64 byte multiple.
	while (((U32(mpCmd) + smallTableDataSize) & 0x3F) != 0)
	{
		*((VU8 *)mpCmd) = VU8(0);
		mpCmd += 8;
	}

	// Reserve the command list.
	U64 mpCmdSize = U32(mpCmd) - U32(mpCmdStart);
	U8 *mpCmdOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, mpCmdSize);

	// Leave if out of memory.
	if (mpCmdOutput == NULL)
	{
		return;
	}

	// Output the command list.
	OutputJobData(outputLocs->m_jobDataAllocator, mpCmdStart, mpCmdSize);

	// Set the tags pointer to the mesh processing command list.
	tagsStart[0].m_source = U32(mpCmdOutput);

	// Calculate the size of the job data, including the job command list and padding to a 64 byte alignment.
	U64 jobDataSize = (U32(tags) - U32(jobData) + 0x30 + 0x3F) & ~0x3F;
	ICE_ASSERT(jobDataSize <= 0x300);

	// Reserve the job data.
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSize);

	// Leave if out of memory.
	if (jobDataOutput == NULL)
	{
		return;
	}

	// Calculate the size of the list DMA tags, rounded up to a multiple of 16 bytes.
	U32 tagsSize = (U32(tags) - U32(tagsStart) + 0xF) & ~0xF;

	// Setup the job command list.
	SetJobCommandList(outputLocs->m_spuModule, tagsStart, tagsSize, (WwsJob_Command *)tags, jobData, jobDataOutput);

	// Output the job data.
	OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSize);

	if (TestListDmaSize(dmaSize))
	{
		// Add the job header to the job list if the list DMA isn't too large.
		outputLocs->m_jobList->AddJob(SetJobHeader(U32(tags) - U32(jobData) + U32(jobDataOutput)));
	}
}

void Ice::Mesh::CutAndPasteVertexSet(
	InputInfo const * const inputInfo,
	OutputLocations * __restrict const outputLocs,
	CutAndPasteInfo const * const cutAndPaste)
{
	ICE_ASSERT(cutAndPaste != NULL);

	// Use the default command context if one wasn't specified.
	Render::CommandContextData *pCommandContext = (outputLocs->m_commandContext != NULL) ?
		outputLocs->m_commandContext : Render::g_currentCommandContext;

	// Get the processing options.
	U32 trimmingTagsOffset = cutAndPaste->m_trimmingTagsOffset;
	bool performTrimming = trimmingTagsOffset != 0;
	bool useRingBuffer = outputLocs->m_meshOutputBufferType == kMeshOutputRingBuffers;
	U32 discretePmInfoTagOffset = cutAndPaste->m_discretePmInfoTagOffset;
	bool discretePmInfoExists = discretePmInfoTagOffset != 0;

	// Attempt to reserve space in the push buffer.
	U64 pbSize = cutAndPaste->m_pbSize;
	U64 pbHoleSize = cutAndPaste->m_pbHoleSize;
	U32 *pbOutput = ReserveMemoryForAndAlignPushBuffer(pCommandContext, pbSize, pbHoleSize);

	// Leave if there is not enough space in the push buffer or if the cut and paste data failed to be created.
	if ((pbOutput == NULL) || (cutAndPaste->m_pushBuffer == NULL))
	{
		return;
	}

	// Output the push buffer and its hole.
	OutputPushBufferWithHole(pCommandContext, cutAndPaste->m_pushBuffer, pbSize, pbHoleSize, performTrimming);

	// Calculate and attempt to reserve space for the job data.
	U8 *jobData = cutAndPaste->m_jobData;
	U64 jobDataSize = cutAndPaste->m_jobDataSize;
	ICE_ASSERT((jobDataSize & 0xF) == 0);
	U64 smallTablesSize = discretePmInfoExists ? 0x40 : 0x30;
	U8 *jobDataOutput = ReserveMemoryForJobData(outputLocs->m_jobDataAllocator, jobDataSize + smallTablesSize);

	// Leave if there is not enough space for the output data.
	if (jobDataOutput == NULL)
	{
		return;
	}

	// Output the job data.
	OutputJobData(outputLocs->m_jobDataAllocator, jobData, jobDataSize);
	outputLocs->m_jobDataAllocator->m_pCurrent += smallTablesSize;

	// Build NV control.
	NvControl * __restrict pNvControl = (NvControl * __restrict)(U32(jobDataOutput) + jobDataSize);
	U32 *pPrimaryPbHole = performTrimming ? pbOutput + pbSize / sizeof(U32) : pbOutput;
	U32 pbEndPtr = U32(pbOutput) + pbSize + pbHoleSize + outputLocs->m_cmdBufferOffset;
	((U64 * __restrict)pNvControl->m_attributeMap)[0] = ((U64 *)inputInfo->m_pVertexInputInfo)[0];
	((U64 * __restrict)pNvControl->m_attributeMap)[1] = ((U64 *)inputInfo->m_pVertexInputInfo)[1];
	pNvControl->m_outputBufferAllocator = U32(outputLocs->m_meshOutputBufferAllocator);
	pNvControl->m_outputBufferOffset = outputLocs->m_meshOutputBufferOffset;
	pNvControl->m_gpuSyncMutex = U32(outputLocs->m_gpuSyncMutex);
	pNvControl->m_putPtrValue = Render::TranslateAddressToIoOffset((void *)pbEndPtr);
	pNvControl->m_primaryPbHole = U32(pPrimaryPbHole) + outputLocs->m_cmdBufferOffset;
	pNvControl->m_primaryPbHoleSize = pbHoleSize;
	pNvControl->m_semaphorePbHole = useRingBuffer ? pbEndPtr - 16 : 0;
	pNvControl->m_numLights = 0;

	// Build discrete PM info, if needed.
	DiscretePmInfo * __restrict pDiscretePmInfo = (DiscretePmInfo * __restrict)(U32(pNvControl) + 0x30);
	if (discretePmInfoExists)
	{
		pDiscretePmInfo->m_alpha = 1.0f - inputInfo->m_lodLevelFractional;
		pDiscretePmInfo->m_padding0 = 0;
		pDiscretePmInfo->m_padding1 = 0;
	}

	// Patch the DMA tags now that we know where the job data will be output.
	DmaTag *tags = (DmaTag *)jobDataOutput;
	//U64 commandListSource;
	//__asm__ ("add %0, %1, %2" : "=r"(commandListSource) : "r"(jobDataOutput), "r"(cutAndPaste->m_commandListOffset));
	//__asm__ ("stw %0, 0x04(%1)" :: "r"(commandListSource), "b"(jobDataOutput));
	//__asm__ ("stw %0, 0x0C(%1)" :: "r"(pNvControl), "b"(jobDataOutput));
	tags[0].m_source = U32(jobDataOutput) + cutAndPaste->m_commandListOffset;  // Pointer to the command list.
	tags[1].m_source = U32(pNvControl);                                        // Pointer to NV control.

	// Patch the viewport and root transform tags if trimming is being performed.
	if (performTrimming)
	{
		DmaTag *trimTags = (DmaTag *)(U32(jobDataOutput) + trimmingTagsOffset);
		ICE_ASSERT(inputInfo->m_pViewport != NULL);
		trimTags[0].m_source = U32(inputInfo->m_pViewport);
		if (inputInfo->m_pRootTransform != NULL)
		{
			ICE_ASSERT(inputInfo->m_pRootTransform != NULL);
			trimTags[1].m_source = U32(inputInfo->m_pRootTransform);
		}
	}

	// Patch the discrete PM info tag and parent table, if it exists.
	if (discretePmInfoExists)
	{
		DmaTag *discretePmInfoTag = (DmaTag *)(U32(jobDataOutput) + discretePmInfoTagOffset);

		discretePmInfoTag->m_source = U32(pDiscretePmInfo);

		// Clear out the parent table tag if at a LOD flat spot.
		U32 parentTableTagsOffset = cutAndPaste->m_parentTableTagsOffset;
		if ((parentTableTagsOffset != 0) && (inputInfo->m_lodLevelFractional <= 0.0f))
		{
			U64 *parentTableTag = (U64 *)(U32(jobDataOutput) + parentTableTagsOffset);
			*parentTableTag = 0;
		}
	}
	else
	{
		// Patch the continuous PM info tag, if it exists.
		U32 continuousPmInfoTagOffset = cutAndPaste->m_continuousPmInfoTagOffset;
		if (continuousPmInfoTagOffset != 0)
		{
			ICE_ASSERT(inputInfo->m_pContinuousPmInfo != NULL);
			DmaTag *continuousPmInfoTag = (DmaTag *)(U32(jobDataOutput) + continuousPmInfoTagOffset);
			continuousPmInfoTag->m_source = U32(inputInfo->m_pContinuousPmInfo);

			// Patch the parent table tags, if they exist.
			U32F numParentTablesForContinuousPm = cutAndPaste->m_numParentTablesForContinuousPm;
			if (numParentTablesForContinuousPm != 0)
			{
				// Get information about the parent tables.
				U32F lodLevel = inputInfo->m_lodLevel;
				U32F lowestLod = inputInfo->m_lowestLodLevel;
				ICE_ASSERT(lowestLod < lodLevel + numParentTablesForContinuousPm);

				// Clear out parent tables which do not need to be used.
				U64 *parentTableTags = (U64 *)(U32(jobDataOutput) + cutAndPaste->m_parentTableTagsOffset);
				for (U32F iLod = lowestLod - lodLevel + 1; iLod < numParentTablesForContinuousPm; ++iLod)
				{
					parentTableTags[iLod] = 0;
				}
			}
		}
		else
		{
			// Patch the DM info tag, if it exists.
			U32 dmInfoTagOffset = cutAndPaste->m_dmInfoTagOffset;
			if (dmInfoTagOffset != 0)
			{
				ICE_ASSERT(inputInfo->m_pDmInfo != NULL);
				DmaTag *dmInfoTag = (DmaTag *)(U32(jobDataOutput) + dmInfoTagOffset);
				dmInfoTag->m_source = U32(inputInfo->m_pDmInfo);
			}
		}
	}

	// Get the skinning matrices information.
	U64 *matrixTags = (U64 *)(U32(jobDataOutput) + cutAndPaste->m_matrixTagsOffset);
	U32F matrixRunsCount = cutAndPaste->m_matrixRunsCount;
	U64 matricesPtrOffset = cutAndPaste->m_matricesPtrOffset;
	U64 newMatricesPtr = U64(inputInfo->m_pMatrices);

	// Offset within 128 bytes must be the same for the old and new matrices array, if skinning is used.
	ICE_ASSERT(!matrixRunsCount || (((matricesPtrOffset & 0x7F) == (newMatricesPtr & 0x7F)) && (newMatricesPtr != 0)));

	// Patch the matrix tags, if they exist.
	U64 dummyReg;
	U64 matrixDelta = newMatricesPtr - matricesPtrOffset;
	switch(matrixRunsCount)
	{
		default:
			for (U32F iTag = 0; iTag < matrixRunsCount; iTag++)
			{
				matrixTags[iTag] += matrixDelta;
			}
			break;
		case 16:
			__asm__ volatile ("ld %0, 0x78(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x78(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 15:
			__asm__ volatile ("ld %0, 0x70(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x70(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 14:
			__asm__ volatile ("ld %0, 0x68(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x68(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 13:
			__asm__ volatile ("ld %0, 0x60(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x60(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 12:
			__asm__ volatile ("ld %0, 0x58(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x58(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 11:
			__asm__ volatile ("ld %0, 0x50(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x50(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 10:
			__asm__ volatile ("ld %0, 0x48(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x48(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 9:
			__asm__ volatile ("ld %0, 0x40(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x40(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 8:
			__asm__ volatile ("ld %0, 0x38(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x38(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 7:
			__asm__ volatile ("ld %0, 0x30(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x30(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 6:
			__asm__ volatile ("ld %0, 0x28(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x28(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 5:
			__asm__ volatile ("ld %0, 0x20(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x20(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 4:
			__asm__ volatile ("ld %0, 0x18(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x18(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 3:
			__asm__ volatile ("ld %0, 0x10(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x10(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 2:
			__asm__ volatile ("ld %0, 0x08(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x08(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 1:
			__asm__ volatile ("ld %0, 0x00(%1)" : "=r"(dummyReg) : "b"(matrixTags));
			__asm__ volatile ("add %0, %1, %2" : "=r"(dummyReg) : "r"(dummyReg), "r"(matrixDelta));
			__asm__ volatile ("std %0, 0x00(%1)" : : "r"(dummyReg), "b"(matrixTags));
		case 0:
			;
	}

	// Blend shape tag is not currently patched.

	// Patch the command list for the job.
	WwsJob_Command *commandList = (WwsJob_Command *)(U32(jobDataOutput) + jobDataSize - 6 * sizeof(WwsJob_Command));
	commandList[3].UseInputDmaListBuffer(kMeshInputBufferSetNum, 0, U32(jobDataOutput), cutAndPaste->m_tagsSize,
			WwsJob_Command::kNonCached, 0);

	// Add the job header to the job list.
	outputLocs->m_jobList->AddJob(SetJobHeader(U32(commandList)));
}

