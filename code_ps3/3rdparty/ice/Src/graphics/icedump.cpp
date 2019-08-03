/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 *
 * Revision History:
 *  - Created 8/8/05
 */

#include "icegraphics.h"
#include "icecg.h"
#include "icefx.h"
#include "icemesh.h"
#include "icelights.h"
#include "iceobjectbatch.h"
#include "iceeffects.h"
#include "icetextures.h"
#include "icedebugdraw.h"
#include "icedump.h"


using namespace SMath;
using namespace Ice::Bucketer;
using namespace Ice::Graphics;
using namespace Ice::Mesh;
using namespace Ice::MeshProc;
using namespace Ice::Render;
using namespace Ice;

// Helper functions

static void DumpVariableStreamFormatInfo(const VariableStreamFormatInfo *formatInfo, U32 swStreamFormatCount, U32 /*verbosity*/, int indent)
{
	// TODO: validate length
	printf("%*sVariable Stream Format Info with %d SW stream formats:\n", indent, "", swStreamFormatCount);
	for (U64 iStream = 0; iStream < swStreamFormatCount; iStream++) {
		const SwStreamFormat &format = formatInfo->m_streamFormats[iStream];
		printf("%*s   stream[%lld] - ID: 0x%x, scale/bias flags: 0x%x, %d components, bitcounts: %d/%d/%d/%d\n",
			   indent, "", iStream, format.m_name, format.m_scaleBiasIntoffFlags, format.m_componentCount,
			   format.m_componentBitcounts[0], format.m_componentBitcounts[1], format.m_componentBitcounts[2],
			   format.m_componentBitcounts[3]);
	}
	// TODO: printf scale/bias/offset data, at least for SW formats...
}

static void DumpVertexSet(const PmVertexSet *vertexSet, U32 startLod, U32 lodCount, bool hasDmData, unsigned verbosity, int indent)
{
	printf("%*s%d streams, %d NV streams:\n", indent, "", vertexSet->m_streamCount, vertexSet->m_nvStreamCount);
	for (U64 iStream = 0; iStream < vertexSet->m_streamCount; iStream++) {
		printf("%*s   stream[%lld]: pointer 0x%p, offset 0x%x\n", indent, "", iStream,
		       vertexSet->m_streams[iStream].m_ptr, vertexSet->m_streams[iStream].m_ofs);
	}
	if (hasDmData) {
		printf("%*s   displacements: pointer 0x%p\n", indent, "", vertexSet->m_streams[vertexSet->m_streamCount].m_ptr);
	}

	// LOD vertex table
	printf("%*sLOD vertex table:\n", indent, "");
	U64 rowCount = vertexSet->m_lodVertexTable[0].m_rowCount;
	for (U64 iRow = 0; iRow < rowCount; iRow++) {
		const LodVertexTable *vertexTable = &vertexSet->m_lodVertexTable[iRow];
		printf("%*s   LOD[%lld] - firstVertex: %d, vertexCount: %d, haloVertexCount: %d\n", indent, "",
			   iRow + startLod, vertexTable->m_firstVertex, vertexTable->m_vertexCount,
			   vertexTable->m_haloVertexCount);
	}

	// LOD index table
	printf("%*sLOD index table:\n", indent, "");
	for (U32F iLod = 0; iLod < lodCount; iLod++) {
		const IndexTable *indexTable = &vertexSet->m_lodIndexTables[iLod];
		printf("%*s   LOD[%lld] -  triangleCount: %d, haloTriangleCount: %d, indexes - pointer 0x%p, offset 0x%x\n",
			   indent, "", iLod + startLod, indexTable->m_triangleCount, indexTable->m_haloTriangleCount,
			   indexTable->m_indexes.m_ptr, indexTable->m_indexes.m_ofs);
		if (verbosity > 1) {
			// NOTE: this code is only valid if indexes are only U16s.
			printf("%*s   indexes:\n", indent, "");

			const U16 *index = (const U16 *)indexTable->m_indexes.m_ptr;
			for (U64 a = 0; a < indexTable->m_triangleCount; a++)
			{
				printf("%*s         %d, %d, %d\n", indent, "", index[0], index[1], index[2]);
				index += 3;
			}
		}
	}

	// LOD edge lists
	if (vertexSet->m_lodEdgeLists) {
		printf("%*sLOD edge lists:\n", indent, "");
		for (U32F iLod = 0; iLod < lodCount; iLod++) {
			const EdgeList *edgeList = &vertexSet->m_lodEdgeLists[iLod];
			printf("%*s   LOD[%lld] -  edgeCount: %d\n", indent, "", iLod + startLod, edgeList->m_edgeCount);
			if (verbosity > 1) {
				// NOTE: this code is only valid if edges are one word each.
				printf("%*s   edges:\n", indent, "");

				U32 *edgePtr = (U32 *)edgeList->m_edgeTable;
				for (U64 a = 0; a < edgeList->m_edgeCount; a++)
				{
					printf("%*s      0x%x\n", indent, "", edgePtr[a]);
				}
			}
		}
	} else {
		printf("%*sNo edge data\n", indent, "");
	}

	// LOD parent lists
	if (vertexSet->m_lodParentTables) {
		printf("%*sLOD parent lists:\n", indent, "");
		for (U32F iLod = 0; iLod < lodCount; iLod++) {
			const void *parentTable = vertexSet->m_lodParentTables[iLod];
			printf("%*s   LOD[%lld] -  parent list ptr: 0x%p\n", indent, "", iLod + startLod, parentTable);
			if (verbosity > 1 && parentTable) {
				// NOTE: this code is only valid if parent indexes are only U16s.
				printf("%*s   parents:\n", indent, "");
				U16 *parentPtr = (U16 *)parentTable;
				U64 parentCount = vertexSet->m_lodVertexTable[iLod].m_firstVertex + vertexSet->m_lodVertexTable[iLod].m_vertexCount;
				for (U64 a = 0; a < parentCount; a++)
				{
					printf("%*s         %d\n", indent, "", parentPtr[a]);
				}
			}
		}
	}

	// LOD skinning table
	if (vertexSet->m_lodSkinningTables) {
		printf("%*sLOD skinning table:\n", indent, "");
		for (U32F iLod = 0; iLod < lodCount; iLod++) {
			const SkinningTables &skinTable = vertexSet->m_lodSkinningTables[iLod];
			printf("%*s   LOD[%lld] -  matrixRunCount: %d, ctrlStrLen: %d, sameStrLen: %d, diffStrLen: %d, weightStrLen: %d\n",
				   indent, "",  iLod + startLod, skinTable.m_matrixRunsCount, skinTable.m_controlStreamLength,
				   skinTable.m_sameStreamLength, skinTable.m_diffStreamLength, skinTable.m_weightsStreamLength);
			// TODO: if verbosity high enough, should print out the data as well...
		}
	}

	// Variable stream format info:
	if (vertexSet->m_streamFormatInfo) {
		printf("%*sVariable stream format info address: 0x%p\n", indent, "", vertexSet->m_streamFormatInfo);
		DumpVariableStreamFormatInfo(vertexSet->m_streamFormatInfo, vertexSet->m_streamCount - vertexSet->m_nvStreamCount, verbosity, indent + 3);
	}
}

static void DumpVertexSetBlendShapes(const VertexSetBlendShapes *blendShapes, U32 effectiveDeltaStreamCount, U32 startLod, U32 lodCount, unsigned verbosity, int indent)
{
	U64 shapeCount = blendShapes->m_shapeCount;
	if (shapeCount) 
	{
		printf("%*s%lld blend shapes:\n", indent, "", shapeCount);
		for (U64 iShape = 0; iShape < shapeCount; iShape++) 
		{
			printf("%*s   shape[%lld] -  object shape index: %d\n", indent, "", iShape, blendShapes->m_shapeRemapTable[iShape]);
			if (verbosity > 0) {
				BlendShapeInfo &shape = blendShapes->m_blendShapeInfos[iShape];
				DumpVariableStreamFormatInfo(shape.m_streamFormatInfo, effectiveDeltaStreamCount, verbosity, indent + 3);
				printf("%*s      run list (start index/count): ", indent, "");
				U64 maxRunCount = shape.m_lodTables[0].m_runCount;
				for (U64 iRun = 0; iRun < maxRunCount; iRun++) {
					printf("(%d/%d) ", shape.m_runList[iRun].m_startIndex, shape.m_runList[iRun].m_count);
				}
				printf("\n");
				for (U64 iStream = 0; iStream < effectiveDeltaStreamCount; iStream++) {
					printf("%*s      stream[%lld] -  address: 0x%p, bits per element: %d\n", indent, "", iStream,
						   shape.m_streams[iStream], shape.m_streamBitCounts[iStream]);
				}
				for (U64 iLod = 0; iLod < lodCount; iLod++) {
					printf("%*s      lod[%lld] - run count: %d, nonzero delta count: %d\n", indent, "",
						   iLod + startLod, shape.m_lodTables[iLod].m_runCount, shape.m_lodTables[iLod].m_deltaCount);
				}
			}
		}
	}
}

static void DumpMaterialInstance(const MaterialInstance *matInstance, U32 objLodCount, U32 tessCount, unsigned verbosity, int indent)
{
	char tempBuf[256];
	printf("%*smaterial descriptor pointer: 0x%p\n", indent, "", matInstance->m_materialDescriptor);
	bool shaderLod = matInstance->m_flags & kMaterialInstanceShaderLod;
	printf("%*sshader LOD: %s\n", indent, "", shaderLod ? "on" : "off");

	// Dump technique table
	printf("%*stechnique table (flat/transition):\n", indent, "");
	printf("%*s   LOD              ", indent, "");

	// Sort used variations by column (we assume a cap on the number of variations here for dumping purposes)
	const U64 maxUsedVariations = 64;
	U8 temp[maxUsedVariations];
	U64 usedVarCount = matInstance->m_materialDescriptor->m_variationCount;
	usedVarCount = usedVarCount <= maxUsedVariations ? usedVarCount : maxUsedVariations;
	U64 usedVarSoFar = 0;
	U8 *varMappingTable = matInstance->m_materialDescriptor->m_variationMappingTable;
	for (U64 i = 0; usedVarSoFar < usedVarCount; i++) {
		if (varMappingTable[i] != kVariationNotPresent) {
			temp[varMappingTable[i]] = i;
			usedVarSoFar++;
		}
	}

	// Print column headers
	for (U32 i = 0; i < usedVarCount; i++) {
		printf("%20d", temp[i]);
	}
	printf("\n");

	// Print rows
	if (shaderLod) {
		U32 tableIdx = 0;
		U32 i = 0;
		for (; i < tessCount; i++) {
			sprintf(tempBuf, "LOD %i", i - tessCount);
			printf("%*s%-20s", indent, "", tempBuf);
			for (U32 j = 0; j < usedVarCount; j++) {
				sprintf(tempBuf, "%i / N/A", matInstance->m_techniqueTable[tableIdx].m_flatLodTechniqueIndex);
				printf("%20s", tempBuf);
				tableIdx++;
			}
			printf("\n");
		}
		for (; i < objLodCount; i++) {
			sprintf(tempBuf, "LOD %i", i - tessCount);
			printf("%*s%-20s", indent, "", tempBuf);
			for (U32 j = 0; j < usedVarCount; j++) {
				sprintf(tempBuf, "%i / %i", matInstance->m_techniqueTable[tableIdx].m_flatLodTechniqueIndex,
						matInstance->m_techniqueTable[tableIdx].m_transitionLodTechniqueIndex);
				printf("%20s", tempBuf);
				tableIdx++;
			}
			printf("\n");
		}
	} else {
		if (tessCount) {
			printf("%*s   Negative LODs:   ", indent, "");
			for (U32 i = 0; i < usedVarCount; i++) {
				sprintf(tempBuf, "%i / N/A", matInstance->m_techniqueTable[usedVarCount + i].m_flatLodTechniqueIndex);
				printf("%20s", tempBuf);
			}
			printf("\n");
		}
		printf("%*s   Positive LODs:   ", indent, "");

		for (U32 i = 0; i < usedVarCount; i++) {
			sprintf(tempBuf, "%i / %i", matInstance->m_techniqueTable[i].m_flatLodTechniqueIndex,
					matInstance->m_techniqueTable[i].m_transitionLodTechniqueIndex);
			printf("%20s", tempBuf);
		}
		printf("\n");
	}

	if (verbosity > 0) {
		/* TODO: print material parameter data somehow (for each technique? identify unique ones?) */
	}
}

static void DumpDiscretePmRenderable(const DiscretePmRenderable *renderable, U32 startLod,
									 U32 objLodCount, U32 tessCount, bool hasDmData,
									 bool lastLodGroup, unsigned verbosity, int indent)
{
	printf("%*smaterial instance:\n", indent, "");
	DumpMaterialInstance(renderable->m_materialInstance, objLodCount, tessCount, verbosity, indent+3);

	// Calculate effective delta stream count (highest stream used by any technique)
	U32 effectiveDeltaStreamCount = 0;
	U16 cumulativeDeltaStreamUsageBits = 0;
	MaterialDescriptor *matDesc = renderable->m_materialInstance->m_materialDescriptor;
	for (U64 iTech = 0; iTech < matDesc->m_techniqueCount; iTech++) {
		cumulativeDeltaStreamUsageBits |= U16(matDesc->m_techniques[iTech].m_deltaStreamUsageBits & 0xFFFF);
	}
	U16 bitMask = 0x1;
	for (U64 iBit = 0; iBit < 16; iBit++) {
		if (cumulativeDeltaStreamUsageBits & bitMask) {
			effectiveDeltaStreamCount = iBit;
		}
		bitMask <<= 1;
	}

	// At all LOD groups except the last, the number of actual LODs is one less than the row
	// count of the LOD vertex table (this should be the same for all vertex sets in the
	// renderable, so we just grab it from the first).
	U64 rowCount = renderable->m_vertexSets[0]->m_lodVertexTable[0].m_rowCount;
	U64 lodCount = lastLodGroup ? rowCount : rowCount - 1;

	printf("%*s%i vertex sets:\n", indent, "", renderable->m_vertexSetCount);
	for (U64 i = 0; i < renderable->m_vertexSetCount; i++) {
		printf("%*s   vertex set[%lli]:\n", indent, "", i);
		DumpVertexSet(renderable->m_vertexSets[i], startLod, lodCount, hasDmData, verbosity, indent+6);
		if (renderable->m_vertexSetBlendShapes && renderable->m_vertexSetBlendShapes[i]) {
			DumpVertexSetBlendShapes(renderable->m_vertexSetBlendShapes[i], effectiveDeltaStreamCount, startLod, lodCount, verbosity, indent+6);
		}
	}
}

static void DumpDiscretePmLodGroup(const DiscretePmLodGroup *lodGroup, U32 objLodCount,
								   U32 tessCount, bool lastLodGroup,
								   unsigned verbosity, int indent)
{
	bool isDm = lodGroup->m_flags & kDiscretePmLodGroupDisplacementMapping;
	if (isDm) {
		printf("%*sDM LOD group - ", indent, "");
	} else {
		printf("%*sPM LOD group - ", indent, "");
	}
	printf("start LOD: %i, end LOD: %i, flags: 0x%x\n", lodGroup->m_startLod, lodGroup->m_endLod, lodGroup->m_flags);
	bool hasParts = lodGroup->m_flags & kDiscretePmLodGroupHasParts;
	if (hasParts) {
		printf("%*s%i parts:\n", indent, "", lodGroup->m_count);
		for (U64 i = 0; i < lodGroup->m_count; i++) {
			printf("%*s   part[%lli]:\n", indent, "", i);
			DiscretePmPart *part = lodGroup->m_parts[i];
			if (part) {
				printf("%*s   %i renderables:\n", indent, "", part->m_renderableCount);
				for (U64 j = 0; j < part->m_renderableCount; j++) {
					printf("%*s      renderable[%lli]:\n", indent, "", j);
					DumpDiscretePmRenderable(part->m_renderables[j], lodGroup->m_startLod, objLodCount, tessCount, isDm, lastLodGroup, verbosity, indent+9);
				}
			} else {
				printf("%*s   NULL part\n", indent, "");
			}
		}
	} else {
		printf("%*s%i renderables:\n", indent, "", lodGroup->m_count);
		for (U64 i = 0; i < lodGroup->m_count; i++) {
			printf("%*s   renderable[%lli]:\n", indent, "", i);
			DumpDiscretePmRenderable(lodGroup->m_renderables[i], lodGroup->m_startLod, objLodCount, tessCount, isDm, lastLodGroup, verbosity, indent+6);
		}
	}
}

void Ice::Graphics::DumpTransform(const Transform *t, U32 /*verbosity*/, int indent)
{
	printf("%*s%f %f %f\n", indent, "", (float)t->GetXAxis().X(), (float)t->GetXAxis().Y(), (float)t->GetXAxis().Z());
	printf("%*s%f %f %f\n", indent, "", (float)t->GetYAxis().X(), (float)t->GetYAxis().Y(), (float)t->GetYAxis().Z());
	printf("%*s%f %f %f\n", indent, "", (float)t->GetZAxis().X(), (float)t->GetZAxis().Y(), (float)t->GetZAxis().Z());
	printf("%*s%f %f %f\n", indent, "", (float)t->GetTranslation().X(), (float)t->GetTranslation().Y(), (float)t->GetTranslation().Z());
}

// For debugging. High verbosity will dump out numerical values and table contents.
void Ice::Graphics::DumpDiscretePmObject(const DiscretePmObject *object, unsigned verbosity, int indent)
{
	printf("%*sDumping Discrete PM Object:\n", indent, "");

	// Calculate number of DM tesselations
	U32 tessCount = 0;
	for (U64 i = 0; i < object->m_lodGroupCount; i++) {
		if (object->m_lodGroups[i]->m_flags & kDiscretePmLodGroupDisplacementMapping) tessCount++;
	}
	if (tessCount) printf("%*s   %i DM Tesselations\n", indent, "", tessCount);

	printf("%*s   %i LODs:\n", indent, "", object->m_lodCount);
	for (U64 i = 0; i < U64(object->m_lodCount) - 1; i++) {
		printf("%*s      distances[%lli]: near: %f, far: %f\n", indent, "", i, object->m_lodDistances[i].m_near, object->m_lodDistances[i].m_far);
	}
	printf("%*s      distances[%lli]: Not Applicable\n", indent, "", U64(object->m_lodCount) - 1);
	printf("%*s   %i LOD groups:\n", indent, "", object->m_lodGroupCount);
	for (U64 i = 0; i < object->m_lodGroupCount; i++) {
		printf("%*s      LOD group[%lli]:\n", indent, "", i);
		DumpDiscretePmLodGroup(object->m_lodGroups[i], object->m_lodCount, tessCount, i == U64(object->m_lodGroupCount) - 1, verbosity, indent+9);
	}
	printf("%*s   %i transforms\n", indent, "", object->m_transformCount);
}

static void DumpContinuousPmNode(const ContinuousPmNode *node, const U32 *lodGroupStartLods, bool lastLodGroup, unsigned verbosity, int indent)
{
	// At all LOD groups except the last, the number of actual LODs is one less than the row
	// count of the LOD vertex table.
	U64 rowCount = node->m_vertexSet->m_lodVertexTable[0].m_rowCount;
	U64 lodCount = lastLodGroup ? rowCount : rowCount - 1;

	printf("%*sDumping vertex set:\n", indent, "");
	DumpVertexSet(node->m_vertexSet, *lodGroupStartLods, lodCount, false, verbosity, indent+3);
	if (verbosity > 0) {
		printf("%*sWorld-space bounding sphere:\n%*s   %f, %f, %f, %f\n", indent, "", indent, "",
			   (float)node->m_boundingSphere.X(), (float)node->m_boundingSphere.Y(),
			   (float)node->m_boundingSphere.Z(), (float)node->m_boundingSphere.W());
	}
	printf("%*s%i children:\n", indent, "", node->m_childCount);
	for (U64 i = 0; i < node->m_childCount; i++) {
		printf("%*s   child[%lli]:\n", indent, "", i);
		// Only the tree root is at the last LOD group
		DumpContinuousPmNode(node->m_childNodes[i], lodGroupStartLods - 1, false, verbosity, indent+6);
	}
}

static void DumpContinuousPmRenderable(const ContinuousPmRenderable *renderable, const U32 *lodGroupStartLods, U32 objLodCount, unsigned verbosity, int indent)
{
	printf("%*smaterial instance:\n", indent, "");
	DumpMaterialInstance(renderable->m_materialInstance, objLodCount, 0, verbosity, indent+3);

	printf("%*s%i continuous PM trees:\n", indent, "", renderable->m_continuousPmTreeCount);
	for (U64 i = 0; i < renderable->m_continuousPmTreeCount; i++) {
		printf("%*s   tree[%lli]:\n", indent, "", i);
		// The tree root is at the last LOD group
		DumpContinuousPmNode(renderable->m_continuousPmTrees[i], lodGroupStartLods, true, verbosity, indent+6);
	}
}

// For debugging. High verbosity will dump out numerical values and table contents.
void Ice::Graphics::DumpContinuousPmObject(const ContinuousPmObject *object, unsigned verbosity, int indent)
{
	printf("%*sDumping Continous PM Object:\n", indent, "");
	printf("%*s   %i LODs:\n", indent, "", object->m_lodCount);
	for (U64 i = 0; i < U64(object->m_lodCount) - 1; i++) {
		printf("%*s      distances[%lli]: near: %f, far: %f\n", indent, "", i, object->m_lodDistances[i].m_near, object->m_lodDistances[i].m_far);
	}
	printf("%*s      distances[%i]: Not Applicable\n", indent, "", object->m_lodCount - 1);
	printf("%*s   %i LOD groups - start LODs:\n", indent, "", object->m_lodGroupCount);
	for (U64 i = 0; i < object->m_lodGroupCount; i++) {
		printf("%*s      [%lli]: %i\n", indent, "", i, object->m_lodGroupStartLods[i]);
	}
	printf("%*s   %i renderables:\n", indent, "", object->m_renderableCount);
	for (U64 i = 0; i < object->m_renderableCount; i++) {
		printf("%*s      renderable[%lli]:\n", indent, "", i);
		DumpContinuousPmRenderable(object->m_renderables[i], object->m_lodGroupStartLods + object->m_lodGroupCount - 1,
								   object->m_lodCount, verbosity, indent+9);
	}
}

// Helper functions
const char *BooleanString(bool var)
{
	if (var) return "true";
		else return "false";
}

const char *AttributeValueTypeString(U32 type)
{
	switch(type) {
		case kF32: return "float";
		case kF16: return "half";
		case kI16n: return "s16n";
		case kI16: return "s16";
		case kX11Y11Z10n: return "x11y11z10n";
		case kU8n: return "u8n";
		case kU8: return "u8";
		default: return "ERROR!!!";
	}
}

const char *RenderAttributeValueTypeString(Render::AttribType type)
{
	switch(type) {
		case Render::kAttribSignedShortNormalized: return "s16n";
		case Render::kAttribFloat: return "float";
		case Render::kAttribHalfFloat: return "half";
		case Render::kAttribUnsignedByteNormalized: return "u8n";
		case Render::kAttribSignedShort: return "s16";
		case Render::kAttribX11Y11Z10Normalized: return "x11y11z10n";
		case Render::kAttribUnsignedByte: return "u8";
		default: return "ERROR!!!";
	}
}

static void DumpFixedStreamFormatInfo(const FixedStreamFormatInfo *info, int indent)
{
	printf("%*sfixed stream format info @ %p:\n", indent, "", info);

	// TODO: write a version for the new format structs. The previous code is below and can be
	// used as a starting point since the structures are fairly similar. The first hurdle is
	// that there used to be a handy place to grab the total count of formats and now there
	// isn't - the new code will have to search through some other data structure to get that
	// piece of information first.
/*
	printf("%*s   %i stream formats:\n", indent, "", materialDescriptor->m_streamFormatCount);
	for (U32 i = 0; i < materialDescriptor->m_streamFormatCount; i++) {
		printf("%*s      stream format[%i]:\n", indent, "", i);
		StreamFormat *sf = materialDescriptor->m_streamFormats[i];
		printf("%*s         element size: %i\n", indent, "", sf->m_elementSize);
		printf("%*s         %i attribute formats:\n", indent, "", sf->m_attributeCount);
		for (U32 k = 0; k < sf->m_attributeCount; k++) {
			printf("%*s            attribute format[%i]:\n", indent, "", k);
			AttributeFormat &af = sf->m_attributeFormat[k];
			printf("%*s               attribute name: %s\n", indent, "", AttributeNameString((AttributeName)af.m_name));
			printf("%*s               attribute value count: %i\n", indent, "", af.m_valueCount);
			printf("%*s               attribute value type: %s\n", indent, "", AttributeValueTypeString(af.m_valueType));
			printf("%*s               attribute byte offset in stream: %i\n", indent, "", af.m_attributeOffset);
		}
	}
*/

}

// For debugging
void Ice::Graphics::DumpMaterialDescriptor(const MaterialDescriptor *materialDescriptor, U32 /*verbosity*/, int indent)
{
	printf("%*sMaterialDescriptor:\n", indent, "");

	printf("%*s   effect pointer: 0x%p\n", indent, "", materialDescriptor->m_fxEffect);

	DumpFixedStreamFormatInfo(materialDescriptor->m_fixedFormatInfo.m_streamFormatInfo, indent + 3);

	printf("%*s         %i variations used.\n", indent, "", materialDescriptor->m_variationCount);
	U32 variationsUsedSoFar = 0;
	for (U32 i = 0; variationsUsedSoFar < materialDescriptor->m_variationCount; i++) {
		if (materialDescriptor->m_variationMappingTable[i] != kVariationNotPresent) {
			printf("%*s            variation %d used in column %i\n", indent, "", i, materialDescriptor->m_variationMappingTable[i]);
			variationsUsedSoFar++;
		}
	}

	printf("%*s   %i techniques:\n", indent, "", materialDescriptor->m_techniqueCount);
	for (U32 i = 0; i < materialDescriptor->m_techniqueCount; i++) {
		printf("%*s      technique[%i]:\n", indent, "", i);
		const MaterialTechnique &matTech = materialDescriptor->m_techniques[i];

		if (matTech.m_meshProcessingFrontEndFunction == kRenderGeneralPmVertexSet) {
			printf("%*s         general mesh program pointer: 0x%p\n", indent, "", matTech.m_meshProcCommandList);
		} else {
			Mesh::MeshProgramInfo *mInfo = matTech.m_meshProcInfo;
			printf("%*s         mesh program info pointer: 0x%p\n", indent, "", mInfo);
			if (mInfo) {
				printf("%*s         mesh processing front end function: 0x%x, NV input mask: 0x%x, SW input mask: 0x%x, NV output mask: 0x%x, skinning bits: 0x%x\n",
					   indent, "", matTech.m_meshProcessingFrontEndFunction, mInfo->m_nvInputMask, mInfo->m_swInputMask, mInfo->m_nvOutputMask, mInfo->m_skinningBits);
				printf("%*s            NV stream parameter masks:\n", indent, "");
				for (U64 j = 0; j < 8; j++) {
					if (mInfo->m_nvStreamParameterMask[j]) {
						printf("%*s               mask[%lld] = 0x%x\n", indent, "", j, mInfo->m_nvStreamParameterMask[j]);
					}
				}
			}
		}
		printf("%*s         effect technique: %s\n", indent, "", Fx::GetTechniqueName(matTech.m_fxTechnique));
		if (matTech.m_meshProcessingFlags & kDoTrimming) {
			printf("%*s         technique does trimming\n", indent, "");
		} else {
			printf("%*s         technique does not do trimming\n", indent, "");
		}
		if (matTech.m_meshProcessingFlags & kDoGpuSkinning) {
			printf("%*s         technique does GPU skinning with matrices starting at constant: %d\n", indent, "", matTech.m_gpuSkinningConstantIndex);
		} else {
			printf("%*s         technique does not do GPU skinning\n", indent, "");
		}
		if (matTech.m_meshProcessingFlags & kDoShadowVolumeExtrusion) {
			printf("%*s         technique does shadow volume extrusion\n", indent, "");
		} else {
			printf("%*s         technique does not do shadow volume extrusion\n", indent, "");
		}
		if (matTech.m_meshProcessingFlags & kDoubleSided) {
			printf("%*s         technique trimming is double sided\n", indent, "");
		} else {
			printf("%*s         technique trimming is not double sided\n", indent, "");
		}
		printf("%*s         %i material parameter bindings:\n", indent, "", matTech.m_materialParameterCount);
		for (U32 j = 0; j < matTech.m_materialParameterCount; j++) {
			printf("%*s            material parameter binding[%i]:\n", indent, "", j);
			const ParameterBinding &paramBind = matTech.m_materialParameterBindingTable[j];
			printf("%*s               byte offset into material data block: %d\n", indent, "", paramBind.m_valueSource);
			printf("%*s               parameter name: %s\n", indent, "", Cg::GetParameterName(paramBind.m_parameter));
		}
		printf("%*s         %i environment parameter bindings:\n", indent, "", matTech.m_envParameterCount);
		for (U32 j = 0; j < matTech.m_envParameterCount; j++) {
			printf("%*s            environment parameter binding[%i]:\n", indent, "", j);
			const ParameterBinding &paramBind = matTech.m_envParameterBindingTable[j];
			printf("%*s               environment source: %d\n", indent, "", paramBind.m_valueSource);
			printf("%*s               parameter name: %s\n", indent, "", Cg::GetParameterName(paramBind.m_parameter));
		}
		printf("%*s         %i gpu attribute array formats:\n", indent, "", matTech.m_attributeArrayFormatCount);
		for (U32 j = 0; j < matTech.m_attributeArrayFormatCount; j++) {
			printf("%*s            gpu attribute array format[%i]:\n", indent, "", j);
			const AttributeArrayFormatInfo &attribArrayFormatInfo = matTech.m_attributeArrayFormatTable[j];
			printf("%*s               gpu attribute array hardware index: %i\n", indent, "", attribArrayFormatInfo.m_hardwareIndex);
			printf("%*s               gpu attribute array stride: %i\n", indent, "", attribArrayFormatInfo.m_elementStride);
			printf("%*s               gpu attribute array component count: %i\n", indent, "", attribArrayFormatInfo.m_attribCount);
			printf("%*s               gpu attribute array value type: %s\n", indent, "", RenderAttributeValueTypeString((Render::AttribType)attribArrayFormatInfo.m_attribType));
		}
		printf("%*s         attribute usage:\n", indent, "");
		for (U32 j = 0; j < 16; j++) {
			U8 attrUsage = matTech.m_vertexInputInfo.m_attribIdMap[j];
			if (attrUsage > 0x00 && attrUsage < 0xF0) {
				 printf("%*s            ATTR%i is output from mesh processing as ID %d\n", indent, "", j, attrUsage);
			} else if (attrUsage >= 0xF0) {
				U8 attrOffset = matTech.m_vertexInputInfo.m_attribOffset[j];
				printf("%*s            ATTR%i goes directly to the GPU from stream %d with byte offset 0x%x\n", indent, "", j, attrUsage & 0x7, attrOffset);
			}
		}
		printf("%*s         delta stream usage bits: 0x%x, count: %d\n", indent, "", matTech.m_deltaStreamUsageBits & 0xFFFF, matTech.m_deltaStreamUsageBits >> 16);
	}
}


