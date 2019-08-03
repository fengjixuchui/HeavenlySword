/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 *
 * This file contains declarations which are internal to the mesh
 * processor implementation.  It should NOT be included in any files
 * outside the mesh processor!  If you find that you need to reference
 * something in this file from outside code, it should probably be
 * moved to the public mesh processor include file (icemesh.h).
 */

#ifndef ICE_MESH_INTERNAL_H
#define ICE_MESH_INTERNAL_H

#include "icebase.h"
#include "icemesh.h"
#include "icemeshstructs.h"
#include "icequadword.h"
#include <string.h>

#define ALIGNED_128 ICE_ALIGN(128)

#if MESH_PRINTF_ENABLED
# ifdef __SPU__
#  include "jobapi/jobprintf.h"
#  define PRINTF(...) do { JobPrintf(__VA_ARGS__); } while(0)
# else
#  define PRINTF(...) do { printf(__VA_ARGS__); } while(0)
# endif
#else
# define PRINTF(...) do {} while(0)
#endif

#ifdef __SPU__
#include "icemeshdma.h"
#if COLLECT_MESH_STATS
#define INSERT_AUDIT(id, data0, data1) Job::GetTimeAndStoreAudit(kMeshAuditSystem, id, data0, data1)
#else
#define INSERT_AUDIT(id, data0, data1) {(void)(id), (void)(data0), (void)(data1);}
#endif
#else
#define INSERT_AUDIT(id, data0, data1) {(void)(id), (void)(data0), (void)(data1);}
#endif

#define PRINT_MATRIX(mat) PrintMatrix(#mat, mat)
static inline void PrintMatrix(const char *matName, const F32 *m)
{
	PRINTF("%s:\n", matName);
	PRINTF("\t[%3.2f %3.2f %3.2f %3.2f]\n", m[0],  m[1],  m[2],  m[3] );
	PRINTF("\t[%3.2f %3.2f %3.2f %3.2f]\n", m[4],  m[5],  m[6],  m[7] );
	PRINTF("\t[%3.2f %3.2f %3.2f %3.2f]\n", m[8],  m[9],  m[10], m[11]);
	PRINTF("\t[%3.2f %3.2f %3.2f %3.2f]\n", m[12], m[13], m[14], m[15]);

	// To quiet the warnings when PRINTF is turned off.
	(void)matName;
	(void)m;
}

#define PRINT_VEC3(a) PRINTF(#a " = %f %f %f\n", (a)[0], (a)[1], (a)[2])
#define PRINT_F32(a) PRINTF(#a " = %f\n", a)
#define PRINT_I32(a) PRINTF(#a " = %i\n", a)
#define PRINT_U32(a) PRINTF(#a " = %d\n", a)

// Use this union to safely convert between ints and floats; the
// idiomatic "i = *(I32*)&f;" construct appears to tickle a subtle
// compiler bug, and results are unpredictable.
typedef union
{
	U32 m_u32;
	F32 m_f32;
} IntFloat;

//                      11111111111 X 000007FF
//           1111111111100000000000 Y 003FF800
// 11111111110000000000000000000000 Z FFC00000
// ZZZZZZZZZZXXXXXXXXXXXYYYYYYYYYYY
// #defines for X11Y11Z10 format
#define X11Y11Z10_X_MASK 0x000007FF
#define X11Y11Z10_X_SIGN 0x00000400
#define X11Y11Z10_X_BITS 11
#define X11Y11Z10_X_SHIFT 0

#define X11Y11Z10_Y_MASK 0x003FF800
#define X11Y11Z10_Y_SIGN 0x00200000
#define X11Y11Z10_Y_BITS 11
#define X11Y11Z10_Y_SHIFT 11

#define X11Y11Z10_Z_MASK 0xFFC00000
#define X11Y11Z10_Z_SIGN 0x80000000
#define X11Y11Z10_Z_BITS 10
#define X11Y11Z10_Z_SHIFT 22

#define MAKE_X11Y11Z10N(x,y,z) \
		  ((I32(Max(Min((x)*1024.f, 1023.f), -1024.f)) & (X11Y11Z10_X_MASK >> X11Y11Z10_X_SHIFT)) << X11Y11Z10_X_SHIFT) \
		| ((I32(Max(Min((y)*1024.f, 1023.f), -1024.f)) & (X11Y11Z10_Y_MASK >> X11Y11Z10_Y_SHIFT)) << X11Y11Z10_Y_SHIFT) \
		| ((I32(Max(Min((z)*512.f,  511.f ), -512.f )) & (X11Y11Z10_Z_MASK >> X11Y11Z10_Z_SHIFT)) << X11Y11Z10_Z_SHIFT)

#define FROM_X11Y11Z10N(v, x,y,z) \
        { \
		(x) = F32((I32(v) << 21) >> 21) * (2.f / 2047.f) + (1.f / 2047.f); \
		(y) = F32((I32(v) << 10) >> 21) * (2.f / 2047.f) + (1.f / 2047.f); \
		(z) = F32(I32(v) >> 22) * (2.f / 1023.f) + (1.f / 1023.f); \
	}

#define DEBUG_LOG_COMMANDS 0
#if DEBUG_LOG_COMMANDS
#define CMD_LOG(cmdQuad1, cmdQuad2) Ice::MeshProc::LogCommand(__FUNCTION__, (cmdQuad1), (cmdQuad2))
#else // DEBUG_LOG_COMMANDS
#define CMD_LOG(cmdQuad, cmdQuad2) do { cmdQuad1.m_data.u32.A += 0; cmdQuad2.m_data.u32.B += 0;} while(0) // avoid unused variable warnings
#endif //DEBUG_LOG_COMMANDS

extern "C" {
	// The following functions have been downcoded into SPU assembly; the sources
	// with full documentation can be found in the ice/mesh/spu/ directory.  Only the
	// barest of summaries is provided below.

	//! Basically, memcpy().
	void CopyBytes( void* pDst, const void* pSrc, const U32 numBytes );
	//! CopyBytes() with the restriction that pDst and pSrc must be 16-byte aligned, and numBytes must be a multiple of 16.
	void CopyQWords( void* pDst, const void* pSrc, const U32 numBytes );
	//! Implements discrete progressive mesh.
	void PmDiscreteTower(U16 *pParent, U32 numPtrs, U32 *pIn, U32 *pOut, F32 blendFactor, U32 startVert, U32 vertCount);
	//! Implements continuous progressive mesh.
	void PmContinuousTower(const U16 *pParent, const U32 numPtrs, const F32 *pPos, const U32 *pIn, U32 *pOut, const F32 *blendParms, const F32 *cameraLoc, U32 startVert, U32 vertCount);
	//! Decompress an index table from packed 2-10-10-10 format.
	void DecompressIndexes4Byte(U32 *in, U16 *out, U32 numTris);
	//! Decompresses an index table from 16-16-16 format.
	void DecompressIndexes6Byte(U16 *in, U16 *out, U32 numTris);
	//! Decompresses an F32 vertex attribute into a uniform table.
	void DecompressF32(F32 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const numComponents,
		U32 const numVerts);
	//! Decompresses an F16 vertex attribute into a uniform table.
	void DecompressF16(U16 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const numComponents,
		U32 const numVerts, const Ice::MeshProc::IceQuadWord scale, const Ice::MeshProc::IceQuadWord bias);
	//! Decompresses an I16 vertex attribute into a uniform table.
	void DecompressI16(I16 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const numComponents,
		U32 const numVerts, const Ice::MeshProc::IceQuadWord scale, const Ice::MeshProc::IceQuadWord bias);
	//! Decompresses a U8 vertex attribute into a uniform table.
	void DecompressU8(U8 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const numComponents,
		U32 const numVerts, const Ice::MeshProc::IceQuadWord scale, const Ice::MeshProc::IceQuadWord bias);
	//! Decompresses an x11y11z10n vertex attribute into a uniform table.
	void DecompressX11Y11Z10(U32 const * const pSrc, U32 const srcStride, F32 * __restrict pDst, U32 const numComponents,
		U32 const numVerts, const Ice::MeshProc::IceQuadWord scale, const Ice::MeshProc::IceQuadWord bias);
	//! Decompresses a matrix table.
	void DecompressMatrices(F32 *out, F32 *in, U32 numMats);
	//! Decompresses a Software Format (variable bit width)
	void DecompressSwFormat(
		U8  *in,
		F32 *out,
		U32 numVerts,
		U32 bitsX,
		U32 bitsY,
		U32 bitsZ,
		U32 bitsW,
		Ice::MeshProc::IceQuadWord intoff,
		Ice::MeshProc::IceQuadWord scale,
		Ice::MeshProc::IceQuadWord bias);

#if ICE_MESH_STENCIL_SHADOWS_ON
	//! Determines whether each triangle in the index buffer faces a light.
	U32 GenerateTriangleFacingTable(U16* indexes, F32 *pos, U32 *facing, F32 lightPos[3], U32 isPointLight, U32 numTris);
	//! Determines the profile edge list for the current vertex set.
	U32 GenerateProfileEdgeList(U32 *edges, U16 *profileEdges, U32 numEdges, U32 *facing);
	//! Extrudes a list of profile edges into stencil shadow volumes.
	void ExtrudeProfileEdges(U16 *indexes, F32 *pos, U16 *profileEdges, F32 *out, U32 numProfileEdges, U32 isPointLight);
	//! Generates stencil shadow caps.
	U32 GenerateCapIndexes(U16 *indexes, U32 *facing, U16* out, U32 numTris);
#endif // ICE_MESH_STENCIL_SHADOWS_ON

	//! Compresses an F32 vertex attribute into an output vertex stream.
	void InsertF32(F32 const * const pSrc, F32 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
		U16 const *pReindex, U32 const numVerts);
	//! Compresses an F16 vertex attribute into an output vertex stream.
	void InsertF16(F32 const * const pSrc, U16 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
		U16 const *pReindex, U32 const numVerts, Ice::MeshProc::IceQuadWord const scale, Ice::MeshProc::IceQuadWord const bias);
	//! Compresses an I16 vertex attribute into an output vertex stream.
	void InsertI16(F32 const * const pSrc, I16 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
		U16 const *pReindex, U32 const numVerts, Ice::MeshProc::IceQuadWord const scale, Ice::MeshProc::IceQuadWord const bias);
	//! Compresses a U8 vertex attribute into an output vertex stream.
	void InsertU8(F32 const * const pSrc, U8 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
		U16 const *pReindex, U32 const numVerts, Ice::MeshProc::IceQuadWord const scale, Ice::MeshProc::IceQuadWord const bias);
	//! Compresses an x11y11z10n vertex attribute into an output vertex stream.
	void InsertX11Y11Z10(F32 const * const pSrc, U32 * __restrict const pDst, U32 const dstStride, U32 const numComponents,
		U16 const *pReindex, U32 const numVerts, Ice::MeshProc::IceQuadWord const scale, Ice::MeshProc::IceQuadWord const bias);
	//! Compresses an index table into a 16-16-16 format.
	void CompressIndexes6Byte(U16 *pSrc, U16 *pDst, U32 numTris);
	//! Trims a triangle list.  If pReindex is not NULL, vertex trimming is performed. This version uses one sample per pixel.
	void TrimTriangles(U16 *pTris, U32 *trimCounts, const U8 *pViewportInfo, const F32 *pRigidXformInfo,
		const U8 *pObjectInfo, U32 *pVertTemp, U16 *pReindex, const F32 *pVertPositions);
	//! Blend two uniform tables.
	void BlendUniform(F32 *in1, F32 *in2, F32 *out, F32 blendFactor, F32 blendFactor2, U32 numElems);
	//! Apply skinning to the position uniform table.
	void SkinPos(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl, F32 *pPos);
	//! Apply skinning to the position and normal uniform tables.
	void SkinPosNorm(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl,
		F32 *pPos, F32 *pNormal);
	//! Apply skinning to the position, normal and tangent uniform tables.
	void SkinPosNormTan(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl,
		F32 *pPos, F32 *pNormal, F32 *pTangent);
	//! Apply skinning to the position, normal, tangent and displacement normal uniform tables.
	void SkinPosNormTanDisp(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl,
		F32 *pPos, F32 *pNormal, F32 *pTangent, F32 *pDisp);
	//! Copy vertexes of any size from pIn to pOut, using a reindex table to determine the final order.
	void SelectVertsGeneric(const U8 *pIn, U8 *pOut, const U16 *pReindex, const U32 reindexCount, const U32 vertSize);
	//! Copy 4-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts4Byte(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);
	//! Copy 8-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts8Byte(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);
	//! Copy 16*N-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts16N(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);
	//! Copy 12-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts12Byte(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);
	//! Copy 24-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts24Byte(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);
	//! Copy 40-byte vertexes from pIn to pOut, using a reindex table to determine the final order.
	void SelectVerts40Byte(U8 *pSrc, U8 *pDst, U16 *pIndexes, U32 numVerts);

	//
	// From icemesh.cpp:
	//

	/**
	 * Allocate a block of memory outside of the SPU.
	 * This is done through a mutex in main memory which is accessed through
	 * atomic operations.  The address of the mutex is stored in the NvControlInfo structure.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param size The number of bytes to allocate.
	 * @return The address of the allocate block (note that it's not a pointer!)
	 */
	U32 MeshMutexAllocate(U32 *pStaticMem, U32 size);

	//! Variant of MeshKickOutputBuffer().
	void MeshKickOutputBufferToAddr(U32 *pStaticMem, U32 outputStartPtr, Ice::MeshProc::IceQuadWord noUpdateKickPtr = 0);

	/**
	 * Transfers the contents of the current output buffer to VRAM.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param pOutputStartPtr Address of the destination to kick to.  If NULL, a new destiation
	 *                        will be allocated using MeshMutexAllocate().
	 */
	void MeshKickOutputBuffer(U32 *pStaticMem, U32 *pOutputStartPtr = NULL);

	/**
	 * Allocates a block of memory in the current output buffer.  If
	 * insufficient space is available, the current outut buffer is kicked and
	 * the function blocks until the second output buffer has finished kicking.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param size The number of bytes to allocate.
	 * @param alignment The minimum alignment of the new memory block, in bytes.
	 * @param patchPtr The next time the current output buffer is kicked, the final (destination)
	 *                 address of this memory block in VRAM will be stored at the location pointed to by patchPtr.
	 * @param pOutputStartPtr The address of the destination of the output buffer.  Usually omitted.
	 */
	U8 *MeshOutputAllocate(U32 *pStaticMem, U32 size, U32 alignment=16, U32 *patchPtr = NULL, U32 *pOutputStartPtr = NULL);

	// from icemeshdecompress.cpp:

	/**
	 * Decompresses (and optionally copies) a vertex stream from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupNvStream(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Decompresses a series of skinning tables from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupSkinning(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Decompresses a table of skinning matrices from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupMatrices(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	// from icemeshskin.cpp

	/**
	 * Performs skinning on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSkinObject(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);
};

namespace Ice
{
	namespace MeshProc
	{
	/**
	 * Assorted hard-coded limits and sizes.
	 */
	enum
		{
#if ICE_MESH_STENCIL_SHADOWS_ON
			kMaxLightCount      = 4,
#else
			kMaxLightCount      = 0,
#endif // ICE_MESH_STENCIL_SHADOWS_ON
			kMaxLodCount        = 8,

			// all sizes are in bytes
			kSizeOfNvControl            = (48 + 24 * kMaxLightCount),
			kSizeOfObjectInfo           = 4,
			kSizeOfLodInfo              = (8 * kMaxLodCount),
			kSizeOfViewportInfo         = 104,
			kSizeOfRigidXformInfo       = 64,
			kSizeOfShadowInfo           = (16 * kMaxLightCount),
			kSizeOfContinuousPmInfo     = (16 + 8 * kMaxLodCount),
			kSizeOfDiscretePmInfo       = 8,
			kSizeOfDmInfo               = 8,
		};

	/**
	 * Mesh processor static data locations.  These are all word
	 * offsets into the work buffer.
	 */
	enum
	{
		// Area for atomic operations (128 bytes)
		kStaticAtomic = 0,

		// Area for loading mesh processing job completion bits (112 bytes)
		kStaticJobCompletion = kStaticAtomic + 32,

		// Output ring buffer allocation (16 bytes)
		kStaticRingBufferStartPtr = kStaticJobCompletion + 28,
		kStaticRingBufferEndPtr,
		kStaticRingBufferFreeStartPtr,
		kStaticRingBufferSemaphorePtr,

		// Memory allocation (96 bytes)
		kStaticInputBufferPtr = kStaticRingBufferStartPtr + 4,
		kStaticInputBufferTagId,
		kStaticWorkBufferTagId,

		kStaticWorkFreeStart = kStaticInputBufferPtr + 4,

		kStaticWorkFreeEnd = kStaticWorkFreeStart + 4,

		kStaticOutputBufferNum = kStaticWorkFreeEnd + 4,
		kStaticOutputBufferPtr,                             // There are 2 entries here

		kStaticOutputFree = kStaticOutputBufferNum + 4,

		kStaticOutputKickPtr = kStaticOutputFree + 4,

		// Command list parsing information (16 bytes)
		kStaticCmdParsePtr = kStaticOutputKickPtr + 4,
		kStaticCmdParseCallDepth,
		kStaticCmdParseCallStack,                           // There are 2 entries here

		// Quad aligned large tables allocated during decompression (192 bytes)
		kStaticVertexCount = kStaticCmdParsePtr + 4,
		kStaticHaloVertexCount,
		kStaticOutputVertexCount,

		kStaticIndexCount = kStaticVertexCount + 4,
		kStaticHaloIndexCount,

		kStaticIndexPtr = kStaticIndexCount + 4,
		kStaticIndexOutputPtr,

		kStaticEdgeCount = kStaticIndexPtr + 4,
		kStaticGeoEdgeCount,
		kStaticDmEdgeFlag,

		kStaticEdgePtr = kStaticEdgeCount + 4,

		kStaticSkinControlPtr = kStaticEdgePtr + 4,
		kStaticSkinSamePtr,
		kStaticSkinDiffPtr,
		kStaticSkinWeightPtr,

		kStaticMatrixPtr = kStaticSkinControlPtr + 4,

		kStaticPmParentPtr = kStaticMatrixPtr + 4,    // There are kMaxLodCount entries here

		kStaticDmDisplacementPtr = kStaticPmParentPtr + kMaxLodCount,

		kStaticPixelShaderPtr = kStaticDmDisplacementPtr + 4,
		kStaticPixelShaderSize,
		kStaticPixelShaderPatchPtr,
		kStaticPixelShaderConstPtr,
		kStaticPixelShaderOutputPtr,

		// Small tables allocated during decompression (144 bytes)
		kStaticCmdPtr = kStaticPixelShaderPtr + 8,

		kStaticNvControlPtr = kStaticCmdPtr + 4,

		kStaticViewportInfoPtr = kStaticNvControlPtr + 4,

		kStaticRigidObjectXformPtr = kStaticViewportInfoPtr + 4,

		kStaticShadowInfoPtr = kStaticRigidObjectXformPtr + 4,

		kStaticDiscretePmInfoPtr = kStaticShadowInfoPtr + 4,

		kStaticContinuousPmInfoPtr = kStaticDiscretePmInfoPtr + 4,

		kStaticDmInfoPtr = kStaticContinuousPmInfoPtr + 4,

		kStaticObjInfoPtr = kStaticDmInfoPtr + 4,

		// Uniform tables (112 bytes)
		kStaticUniformCount = kStaticObjInfoPtr + 4,

		kStaticUniformPtr = kStaticUniformCount + 4,  // There are 16 entries here

		kStaticAttributeId = kStaticUniformPtr + 16,        // This is a table of 16 bytes, one for each uniform table, containing the attribute ID stored in that table (0 means the table is unused).

		kStaticUniformPosPtr = kStaticAttributeId + 4,
		kStaticUniformNormPtr,
		kStaticUniformTanPtr,
		kStaticUniformDnormPtr,

		// Stream tracking (112 bytes)
		kStaticFixedFormatPtr = kStaticUniformPosPtr + 4,
		kStaticVariableFormatPtr,
		kStaticDeltaFormatPtr,

		kStaticStreamWorkPtr = kStaticFixedFormatPtr + 4,               // There are 8 entries here

		kStaticStreamOutputPtr = kStaticStreamWorkPtr + 8,              // There are 8 entries here

		kStaticCustomCompressionFuncPtr = kStaticStreamOutputPtr + 8,   // There are 8 entries here

		// Tables generated during mesh processing (32 bytes)
		kStaticReindexPtr = kStaticCustomCompressionFuncPtr + 8,
		kStaticReindexCount,
		kStaticReindexIsDummy, // non-zero if the reindex table is currently populated with the dummy identity mapping
		kStaticRenamePtr,
		kStaticTriFacingPtr,
		kStaticProfileEdgePtr,

		// Progressive mesh info (80 bytes)
		kStaticPmHighestLod = kStaticReindexPtr + 8,

		kStaticPmVertexLodInfo = kStaticPmHighestLod + 4,      // There are 8 double word sized entries here

		// Displacement mapping info (16 bytes)
		kStaticDmFlag = kStaticPmVertexLodInfo + 16,

		// Run list index table info (16 bytes)
		kStaticIndexTablePtr = kStaticDmFlag + 4,
		kStaticNumIndexesInTable,

		// Shadow geometry info (80 bytes)
		kStaticShadowProfileEdgeOutputPtr       = kStaticIndexTablePtr + 4,                 // There are 4 word sized entries here
		kStaticShadowProfileEdgeVertexCount     = kStaticShadowProfileEdgeOutputPtr + 4,    // There are 4 word sized entries here
		kStaticShadowCapIndexOutputPtr          = kStaticShadowProfileEdgeVertexCount + 4,  // There are 4 word sized entries here
		kStaticShadowCapIndexCount              = kStaticShadowCapIndexOutputPtr + 4,       // There are 4 word sized entries here
		kStaticShadowCapVertexOutputPtr         = kStaticShadowCapIndexCount + 4,
		kStaticShadowCapVertexFormat,

		// Output patch information (112 bytes)
		kStaticOutputGenerateDrawCalls = kStaticShadowCapVertexOutputPtr + 4, // Only true if mesh processing changes the index buffer (i.e. if index trimming or vertex trimming is called)

		kStaticOutputBufferAllocationFailure = kStaticOutputGenerateDrawCalls + 4,

		kStaticOutputNumPatchPtrs = kStaticOutputBufferAllocationFailure + 4,

		kStaticOutputPatchPtr = kStaticOutputNumPatchPtrs + 4, // There are 16 word sizeed entries here

		// Total number (byte size = number * 4) (1264 bytes)
		kStaticNum = kStaticOutputPatchPtr + 16
	};

	/**
	 * Change
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 */

	//
	// From icemesh.cpp:
	//

	//! Signal that this job is complete and ready for Gpu consumption.
	/*! The atomicity of this operation is protected by a mutex in the
	    NvControlInfo. This may spin-wait if it is too far (32 jobs) ahead
	    of the processing queue. That is a limitation of the gpu sync
	    algorithm.
	    \param pStaticMem  A pointer to the static memory area.
	*/
	void MeshGpuSyncMutex(U32 *pStaticMem);

	/**
	 * Allocate a block of memory from the work buffer.  The buffer is
	 * managed like a stack, with each new allocation coming off the
	 * top of the stack.  Memory allocated with this function must be freed
	 * with MeshMemFree().
	 *
	 * @see MeshMemAllocateEnd()
	 * @see MeshMemFree()
	 * @param pStaticMem A pointer to the static memory area.
	 * @param size The number of bytes to allocate.
	 * @return A pointer to the newly-allocated block.
	 */
	U8 *MeshMemAllocate(U32 *pStaticMem, U32 size);

	/**
	 * Allocate a block of memory from the work buffer.  Behaves just
	 * like MeshMemAllocate(), except that allocates are made from the
	 * other end of the buffer (growing downward instead of upward),
	 * and the corresponding de-allocation function is
	 * MeshMemFreeEnd().
	 *
	 * @see MeshMemAllocate()
	 * @see MeshMemFreeEnd()
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param size The number of bytes to allocate.
	 * @return A pointer to the newly-allocated block.
	 */
	U8 *MeshMemAllocateEnd(U32 *pStaticMem, U32 size);

	/**
	 * Releases memory previously allocated in the work buffer, though
	 * not in the traditional free() fashion; since the work buffer is
	 * managed as a stack, MeshMemFree() actually frees everything
	 * above the address passed.  It us up to the caller to keep track
	 * of the system's allocations to make sure calling this function
	 * doesn't release memory that's still in use!
	 *
	 * @see MeshMemAllocate()
	 * @see MeshFreeEnd()
	 * @param pStaticMem A pointer to the static memory area.
	 * @param newStart An address in the work buffer that will become to the new top of the allocation
	 *                 stack.
	 */
	static inline void MeshMemFree(U32 *pStaticMem, U32 newStart)
	{
		pStaticMem[kStaticWorkFreeStart] = newStart;
#if ICEDEBUG
		memset((void*)pStaticMem[kStaticWorkFreeStart], 0xCD,
			pStaticMem[kStaticWorkFreeEnd] - pStaticMem[kStaticWorkFreeStart]);
#endif
	}

	/**
	 * Frees memory allocated by MeshMemAllocateEnd(), in a manner analogous to MeshMemFree().
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param newEnd Address of the end of the work buffer.
	 */
	static inline void MeshMemFreeEnd(U32 *pStaticMem, U32 newEnd)
	{
		pStaticMem[kStaticWorkFreeEnd] = newEnd;
#if ICEDEBUG
		memset((void*)pStaticMem[kStaticWorkFreeStart], 0xCD,
			pStaticMem[kStaticWorkFreeEnd] - pStaticMem[kStaticWorkFreeStart]);
#endif
	}

	//
	// From icemeshblend.cpp:
	//

	/**
	 * Generates a table of indexes from the compressed input run list.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupRunTable(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Blends an input delta stream with an attribute that has already been decompressed.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdBlendDeltaStream(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	//
	// From icemeshcommand.cpp:
	//

	/**
	* Interpret an SPU command list.  Copies the command quadwords and calls the functions associated with the commands.
	*
	* @param pCommandList Word-aligned pointer to command list. The first
	*                     entry is the length of the list in bytes; the
	*                     rest of the commands and their arguments follow.
	*/
	void InterpretCommandList(U32 *pStaticMem);

	/**
	 * Sets the pointer of the command list parser to the offset within the input buffer specified in the jump command.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdJump(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Sets the pointer of the command list parser to the offset within the input buffer specified in the call command.
	 * Saves the next address in the command list in the cell stack.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdCall(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Sets the pointer of the command list parser to the last address stored on the call stack.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdReturn(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Prints out information about the command.  This includes the command name and the values of the arguments
	 *
	 * @param funcName Name of the function associated with the command
	 * @param cmdQuads The two quadwords which hold information necessary to carry out the command
	 */
	void LogCommand(const char *funcName, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	//
	// from icemeshcompress.cpp:
	//

	/**
	 * Compresses attributes from uniform tables (in the case of F32 it is basically just a copy) and inserts them
	 * into a vertex stream.  Operates on four attributes at a time, and intended to operate on every fourth attribute.
	 *
	 * @param pSrcUniform A pointer to the uniform table holding the attribute in the form of F32s.
	 * @param pDst A pointer to a vertex in the Nv vertex stream; starting at this location attributes will be inserted.
	 * @param vertexStride Stride of the vertexes in the Nv vertex stream.
	 * @param attribComponentCount How many components this particular attribute contains per vertex.
	 * @param attribType Identifier specifying the type of the attribute.
	 * @param scale The scale for each component of the attribute being compressed.
	 * @param bias The bias for each component of the attribute being compressed.
	 * @param pSaveSrcAndSab Optional pointer to memory to save pSrcUniform, the final scale, and the final bias.
	 */
	void InsertAttributeIntoNvStream(F32 const * const pSrcUniform, U8 * __restrict pDst, U32 const vertexStride,
		U32 const attribComponentCount, U8 const attribType, Ice::MeshProc::IceQuadWord scale, Ice::MeshProc::IceQuadWord bias,
		U16 const * const pReindex, U32 const numVerts, Ice::MeshProc::IceQuadWord * __restrict const pSaveSrcAndSab = NULL);

	/**
	 * Compresses the index buffer and copies it into the output buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdOutputIndexes(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Compresses a single vertex attribute and inserts it into an output vertex stream.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdInsertAttributeIntoNvStream(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a complete, compressed output vertex stream from the work buffer to the output buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdOutputCopiedNvStream(U32* pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Creates an entire output vertex stream by compressing the relevant attributes from the uniform tables.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdOutputConvertedUniformTable(U32* pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	// from icemeshdecompress.cpp:

	/**
	 * Copies the NvControlInfo struct from the input buffer into the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupNvControlInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies the ObjectInfo struct from the input buffer into the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupObjectInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies the fixed stream format info and variable stream format info structs from the input buffer into the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupFormatInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a delta stream format info struct from the input buffer into the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupDeltaFormatInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Specifies the vertex counts for this vertex set (non-PM version).
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupVertexInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Specifies the vertex counts for each LOD in this vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupPmVertexInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Decompresses (and optionally copies) a vertex stream from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupSwStream(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies the custom compression code to the work buffer for the specified stream.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupCustomCompress(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2);

	/**
	 * Decompresses a triangle list from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupIndexes(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a set of pixel shader instructions from the input buffer to the work buffer (TO BE IMPLEMENTED)
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupPixelShader(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a ViewportInfo struct from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupViewportInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies the RootTransformInfo struct from the input buffer to the work buffer.z
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupRootTransformInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

#if ICE_MESH_STENCIL_SHADOWS_ON
	/**
	 * Copies the ShadowInfo struct from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupShadowInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Decompresses an edge list from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupEdges(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);
#endif // ICE_MESH_STENCIL_SHADOWS_ON

	/**
	 * Copies a ContinuousPmInfo struct from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupContinuousPmInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a DiscretePmInfo struct from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupDiscretePmInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Decompresses a PM parent table from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupPmParent(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a DmInfo struct from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupDmInfo(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Copies a DM displacement table from the input buffer to the work buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdSetupDmDisplacements(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	//
	// from icemeshmisc.cpp:
	//

	/**
	 * Checks whether the reindex buffer exists.  If it does, this
	 * function does nothing.  If not, the table is allocated and
	 * optionally filled with the dummy (identity) reindex mapping.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param initializeToDummy Fill with identity mapping?
	 */
	void CreateReindexTable(U32 *pStaticMem, bool initializeToDummy);

	/**
	 * Adds a value to each halfword in stream.
	 *
	 * @param in Input stream.
	 * @param out Output stream.
	 * @param toAdd The value to add to each halfword in input stream.
	 * @param numElems Number of halfwords to operate on.
	 */
	void AddIntToHalfTable(U16 *in, U16 *out, U16 toAdd, U32 numElems);

	/**
	 * Patches constants into a pixel shader.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 */
	void PatchPixelShader(U32 *pStaticMem);

	/**
	 * Mimic the task manager load of the data stored in the SPUD.
	 *
	 * @param pSpud A pointer to the SPUD data.
	 * @param sizeOfEntries = 8 * numEntries.
	 * @param pDest Destination pointer.
	 */
	void MeshLoadInputBuffer(U32 *pSpud, U32 sizeOfEntries, U8 *pDest);

	/**
	 * Terminates mesh processing, patches holes in the push buffer, and kicks the output buffer one last time.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdCleanupAndExit(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Starts a list DMA operation to the input buffer.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdStartInputListDma(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Waits for all DMAs to the input buffer to complete.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdStallOnInputDma(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Terminates the setup phase of mesh processing, and releases the input buffer (so the task manager can start
	 * transfering the next vertex set).
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdEndSetup(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	// from icemeshpm.cpp:

	/**
	 * Performs the progressive mesh operation on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdPerformPm(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	// from icemeshdm.cpp:

	/**
	 * Performs the displacement mapping operation on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdPerformDm(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	// from icemeshshadow.cpp:

#if ICE_MESH_STENCIL_SHADOWS_ON
	/**
	 * Performs stencil shadow extrusion on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdExtrudeShadows(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);
#endif // ICE_MESH_STENCIL_SHADOWS_ON

	// from icemeshtrim.cpp:

	/**
	 * Performs index trimming on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdTrimIndexes(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

	/**
	 * Performs vertex trimming on the current vertex set.
	 * See the Command List Construction.doc file for full details on this command.
	 *
	 * @param pStaticMem A pointer to the static memory area.
	 * @param cmdQuadWords The contents of next 32 bytes in the command list, starting with this command in the first byte.
	 */
	void CmdTrimVertexes(U32 *pStaticMem, Ice::MeshProc::IceQuadWord cmdQuad1, Ice::MeshProc::IceQuadWord cmdQuad2);

#ifndef __SPU__
	/// The SPU number that this job is running on.
	extern U32 g_spuNum;

	/// The number of the current job.
	extern U32 g_jobNum;

	/// Structure for storing some basic stats across multiple calls to Mesh Processing.
	/// These are the numbers and sizes of various things being output into the big buffer.
	struct MeshProcessingStats {
		U32 m_indexesNum;
		U32 m_indexesSize;
		U32 m_vertexesNum;
		U32 m_vertexesSize;
		U32 m_capIndexesNum;
		U32 m_capIndexesSize;
		U32 m_profileEdgeNum;
		U32 m_profileEdgeSize;
		U32 m_capsNum;
		U32 m_capsSize;
		U32 m_holeSize;
	};

	/// The one global structure to collect Mesh Processing stats.
	extern MeshProcessingStats g_meshProcessingStats;
#endif

	}
}

/// Structure definition for global constant data.
/// This is currently used custom compress/decompress routines and X11Y11Z10 routines.
struct MeshGlobalConstData {
	U32 m_x11y11z10_decomp_mask[4];
	U32 m_x11y11z10_decomp_shifts[4];
	F32 m_x11y11z10_decomp_scale[4];
	F32 m_x11y11z10_decomp_bias[4];
	U32 m_x11y11z10_comp_shifts[4];
	F32 m_x11y11z10_comp_scale[4];
	F32 m_x11y11z10_comp_bias[4];
};

/// The actual structure of global constant data.
extern MeshGlobalConstData g_meshGlobalConstData;

#endif

