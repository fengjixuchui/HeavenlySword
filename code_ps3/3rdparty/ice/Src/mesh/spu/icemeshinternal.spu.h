;
; Copyright (c) 2005 Naughty Dog, Inc.
; A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
; Use and distribution without consent strictly prohibited
;

; This must match EXACTLY what is in ice/mesh/icemesh.h
.define kMeshInputBufferSize	32 * 1024
.define kMeshWorkBufferSize	64 * 1024
.define kMeshOutputBufferSize	30 * 1024

; Format types
.define kFormatIndex3U16	0
.define kFormatIndex2U16	1
.define kFormatEdgeU32		0
.define kFormatPmParentU16	0
.define kFormatWeightU8		0
.define kFormatWeightF32	1
.define kFormatMatrix44		0
.define kFormatMatrix43		1
.define kFormatDmDispF32	0

; Defined IDs
.define kPositionId		1
.define kNormalId		2
.define kTangentId		3
.define kDispNormalId		4

; Scale and bias defines
.define kScaleAndBiasOn		0x80
.define kScaleAndBiasVariable	0x40
.define kHasIntOff		0x20

; This must match EXACTLY what is in ice/mesh/icemeshinternal.h, with the exception that increments are
; multiplied by 4 due to them being byte offsets rather than word offsets
.define kMaxLightCount		4
.define kMaxLodCount		8

; all sizes are in bytes
.define kSizeOfNvControl	(48 + 24 * 4)
.define kSizeOfObjectInfo	4
.define kSizeOfLodInfo		(8 * 8)
.define kSizeOfViewportInfo	104
.define kSizeOfRigidXformInfo	64
.define kSizeOfShadowInfo	(16 * 4)
.define kSizeOfContinuousPmInfo	(16 + 8 * 8)
.define kSizeOfDiscretePmInfo	8
.define kSizeOfDiscreteDmInfo	8

.define kStaticAtomic				0x000
.define kStaticJobCompletion			0x080
.define kStaticRingBufferStartPtr		0x0F0
.define kStaticRingBufferEndPtr			0x0F4
.define kStaticRingBufferFreeStartPtr		0x0F8
.define kStaticRingBufferSemaphorePtr		0x0FC
.define kStaticInputBufferPtr			0x100
.define kStaticInputBufferTagId			0x104
.define kStaticWorkBufferTagId                  0x108
.define kStaticWorkFreeStart			0x110
.define kStaticWorkFreeEnd			0x120
.define kStaticOutputBufferNum 			0x130
.define kStaticOutputBufferPtr			0x134
.define kStaticOutputFree			0x140
.define kStaticOutputKickPtr			0x150
.define kStaticCmdParsePtr			0x160
.define kStaticCmdParseCallDepth		0x164
.define kStaticCmdParseCallStack		0x168
.define kStaticVertexCount			0x170
.define kStaticHaloVertexCount			0x174
.define kStaticOutputVertexCount		0x178
.define kStaticIndexCount			0x180
.define kStaticHaloIndexCount			0x184
.define kStaticIndexPtr				0x190
.define kStaticIndexOutputPtr			0x194
.define kStaticEdgeCount			0x1A0
.define kStaticGeoEdgeCount			0x1A4
.define kStaticDmEdgeFlag			0x1A8
.define kStaticEdgePtr				0x1B0
.define kStaticSkinControlPtr			0x1C0
.define kStaticSkinSamePtr			0x1C4
.define kStaticSkinDiffPtr			0x1C8
.define kStaticSkinWeightPtr			0x1CC
.define kStaticMatrixPtr			0x1D0
.define kStaticPmParentPtr			0x1E0
.define kStaticDmDisplacementPtr		0x200
.define kStaticPixelShaderPtr			0x210
.define kStaticPixelShaderSize			0x214
.define kStaticPixelShaderPatchPtr		0x218
.define kStaticPixelShaderConstPtr		0x21C
.define kStaticPixelShaderOutputPtr		0x220
.define kStaticCmdPtr				0x230
.define kStaticNvControlPtr			0x240
.define kStaticViewportInfoPtr			0x250
.define kStaticRigidObjectXformPtr		0x260
.define kStaticShadowInfoPtr			0x270
.define kStaticDiscretePmInfoPtr		0x280
.define kStaticContinuousPmInfoPtr		0x290
.define kStaticDmInfoPtr			0x2A0
.define kStaticObjInfoPtr			0x2B0
.define kStaticUniformCount			0x2C0
.define kStaticUniformPtr			0x2D0
.define kStaticAttributeId			0x310
.define kStaticUniformPosPtr			0x320
.define kStaticUniformNormPtr			0x324
.define kStaticUniformTanPtr			0x328
.define kStaticUniformDnormPtr			0x32C
.define kStaticFixedFormatPtr			0x330
.define kStaticVariableFormatPtr		0x334
.define kStaticDeltaFormatPtr			0x338
.define kStaticStreamWorkPtr			0x340
.define kStaticStreamOutputPtr			0x360
.define kStaticCustomCompressionCodePtr		0x380
.define kStaticReindexPtr			0x3A0
.define kStaticReindexCount			0x3A4
.define kStaticReindexIsDummy			0x3A8
.define kStaticRenamePtr			0x3AC
.define kStaticTriFacingPtr			0x3B0
.define kStaticProfileEdgePtr			0x3B4
.define kStaticPmHighestLod			0x3C0
.define kStaticPmVertexLodInfo			0x3D0
.define kStaticDmFlag				0x410
.define kStaticIndexTablePtr			0x420
.define kStaticNumIndexesInTable		0x424
.define kStaticShadowProfileEdgeOutputPtr	0x430
.define kStaticShadowProfileEdgeVertexCount	0x440
.define kStaticShadowCapIndexOutputPtr		0x450
.define kStaticShadowCapIndexCount		0x460
.define kStaticShadowCapVertexOutputPtr		0x470
.define kStaticShadowCapVertexFormat		0x474
.define kStaticOutputGenerateDrawCalls		0x480
.define kStaticOutputBufferAllocationFailure	0x490
.define kStaticOutputNumPatchPtrs		0x4A0
.define kStaticOutputPatchPtr			0x4B0
.define kStaticSize				0x4F0

