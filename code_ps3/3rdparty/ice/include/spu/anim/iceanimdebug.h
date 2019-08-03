/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICEANIMDEBUG_H
#define ICEANIMDEBUG_H

#define ANIMJOB_DISPATCHER_AUDITS 0
#define ANIMJOB_DISPATCHER_ASSERTS 0			// 1 to enable assertions in iceanimtask
#define ANIMJOB_DISPATCHER_DPRINTS 1			// 1 to enable basic DPRINTs in iceanimtask
#define BATCHJOB_DISPATCHER_AUDITS ANIMJOB_DISPATCHER_AUDITS
#define BATCHJOB_DISPATCHER_ASSERTS ANIMJOB_DISPATCHER_ASSERTS
#define BATCHJOB_DISPATCHER_DPRINTS ANIMJOB_DISPATCHER_DPRINTS
#define BATCHJOB_NAMESPACE Ice::Anim

#include "icebatchjobdebug.h"

// Control debug printing per command; generally to keep size of SPU task small enough to fit in memory
//0xFF80000000000000 : utility
//0x007FFFFC00000000 : clip decompression
//0x00000003F8000000 : clip blending
//0x0000000007800000 : parenting, finalize
//0x00000000007FC000 : sdk
//0x0000000000003F00 : constraint
#define HIGH_BIT64 0x8000000000000000ULL
#if ICE_TARGET_PS3_SPU
# define ANIMJOB_DISPATCHER_DPRINT_CMDS 0x0000000007FFFF00ULL		// mask of 0x8000000000000000>>cmd controlling which commands print verbose output
#else
# define ANIMJOB_DISPATCHER_DPRINT_CMDS 0x0000000007FFFF00ULL		// mask of 0x8000000000000000>>cmd controlling which commands print verbose output
//# define ANIMJOB_DISPATCHER_DPRINT_CMDS 0xFFFFFFFFFFFFFFFFULL		// mask of 0x8000000000000000>>cmd controlling which commands print verbose output
#endif
#if ANIMJOB_DISPATCHER_DPRINTS
# define ANIMJOB_DPRINT_FOR_COMMAND(cmd, code)								if ((HIGH_BIT64 >> (U64)cmd) & ANIMJOB_DISPATCHER_DPRINT_CMDS) { code }
# define ANIMJOB_DPRINT_FOR_2COMMANDS(cmd0, cmd1, code)						if (((HIGH_BIT64 >> (U64)cmd0) | (HIGH_BIT64 >> (U64)cmd1)) & ANIMJOB_DISPATCHER_DPRINT_CMDS) { code }
# define ANIMJOB_DPRINT_FOR_3COMMANDS(cmd0, cmd1, cmd2, code)				if (((HIGH_BIT64 >> (U64)cmd0) | (HIGH_BIT64 >> (U64)cmd1) | (HIGH_BIT64 >> (U64)cmd2)) & ANIMJOB_DISPATCHER_DPRINT_CMDS) { code }
# define ANIMJOB_DPRINT_FOR_4COMMANDS(cmd0, cmd1, cmd2, cmd3, code)			if (((HIGH_BIT64 >> (U64)cmd0) | (HIGH_BIT64 >> (U64)cmd1) | (HIGH_BIT64 >> (U64)cmd2) | (HIGH_BIT64 >> (U64)cmd3)) & ANIMJOB_DISPATCHER_DPRINT_CMDS) { code }
# define ANIMJOB_DPRINT_FOR_5COMMANDS(cmd0, cmd1, cmd2, cmd3, cmd4, code)	if (((HIGH_BIT64 >> (U64)cmd0) | (HIGH_BIT64 >> (U64)cmd1) | (HIGH_BIT64 >> (U64)cmd2) | (HIGH_BIT64 >> (U64)cmd3) | (HIGH_BIT64 >> (U64)cmd4)) & ANIMJOB_DISPATCHER_DPRINT_CMDS) { code }
#else
# define ANIMJOB_DPRINT_FOR_COMMAND(cmd, code)								{}
# define ANIMJOB_DPRINT_FOR_2COMMANDS(cmd0, cmd1, code)						{}
# define ANIMJOB_DPRINT_FOR_3COMMANDS(cmd0, cmd1, cmd2, code)				{}
# define ANIMJOB_DPRINT_FOR_4COMMANDS(cmd0, cmd1, cmd2, cmd3, code)			{}
# define ANIMJOB_DPRINT_FOR_5COMMANDS(cmd0, cmd1, cmd2, cmd3, cmd4, code)	{}
#endif

#if ANIMJOB_DISPATCHER_ASSERTS || ANIMJOB_DISPATCHER_DPRINTS
// For sanity checks via TASK_ASSERT, assert on joint indices:
// Current total joint limit is set by  (kWorkBufSize(48K) - tempBufSize(4896B)) / (sizeof(JointParam) + sizeof(JointTransform)) -> 395
static const U16 kDEBUG_MaxJoints = 395;
#endif

// Forward declarations
namespace SMath {
	class Scalar;
	class Vec4;
	class Mat34;
	class Mat44;
}
namespace Ice {
	namespace Anim {
		struct ValidBits;
		struct JointParams;
		class JointTransform;

		struct ConstraintInfoQuad;
		struct AimConstraintDestQuad;
		struct SdkDrivenRotDestQuad;
		struct SdkDrivenPair;
		struct SdkDriversRotSourceQuad;
		struct SdkDriversQuad;
		struct JointParentingQuad;
		struct SourceQuad;
	}
}

#if ANIMJOB_DISPATCHER_DPRINTS
# if ICE_TARGET_PS3_SPU
#  define DEBUG_ExtractWord(validBits, i)	spu_extract((VU32)((validBits).m_bits), i)
# else
#  define DEBUG_ExtractWord(validBits, i)	((U32*)((validBits).m_bits))[i]
# endif

U32F DEBUG_CountConstraints(U32F numInfo, Ice::Anim::ConstraintInfoQuad const *pInfo);
U32F DEBUG_CountAimUpVectors(U32F numDests, Ice::Anim::AimConstraintDestQuad const *pInfo);
U32F DEBUG_SizeofUniformKeyFrame(U32F numJoints, U8 const *pSizeData, U32F sizeStep);
U32F DEBUG_SizeofNonUniformKeys(U32F numJoints, U8 const *pKeys);

void ANIMJOB_DPRINT_Mem_SdkRotSourceQuad(void const *p, U32F size);	// prints data as {U8,U8,U16}[]
void ANIMJOB_DPRINT_ValidBits(Ice::Anim::ValidBits const &validBits);
void ANIMJOB_DPRINT_JointParams(Ice::Anim::JointParams const *pJointParams, U32F numJoints, U32F joint0=0);
void ANIMJOB_DPRINT_JointParamAtOffset(Ice::Anim::JointParams const *pJointParams, U32F offset);
void ANIMJOB_DPRINT_JointParamsAtOffsets(Ice::Anim::JointParams const *pJointParams, U32F numOffsets, U16 const *pOffsets);
void ANIMJOB_DPRINT_JointParamsByValidBits(Ice::Anim::JointParams const *pJointParams, Ice::Anim::ValidBits &validBits);
void ANIMJOB_DPRINT_JointParamsForDecompress(Ice::Anim::JointParams const *pJointParams, U32F numIndices, U16 const *pIndices, U32F componentOffset);
void ANIMJOB_DPRINT_JointParamsByConstraintInfo(Ice::Anim::JointParams const *pJointParams, U32F numInfo, Ice::Anim::ConstraintInfoQuad const *pInfo);
void ANIMJOB_DPRINT_JointParamsByEulerToQuatInfo(Ice::Anim::JointParams const *pJointParams, U32F numInfo, Ice::Anim::SdkDrivenRotDestQuad const *pInfo);
void ANIMJOB_DPRINT_JointParamsForStoreVecs(Ice::Anim::JointParams const *pJointParams, U32F numItems, Ice::Anim::SdkDrivenPair const *pDrivenData);
void ANIMJOB_DPRINT_JointTransforms(Ice::Anim::JointTransform const* pTransforms, U32F numJoints, U32F joint0=0);
void ANIMJOB_DPRINT_JointTransformsByParentInfo(Ice::Anim::JointTransform const *pTransforms, U32F numParentInfo, Ice::Anim::JointParentingQuad const *pInfo);
void ANIMJOB_DPRINT_Matrices34(SMath::Mat34 const* pMatrices, U32F numJoints, U32F joint0=0);
void ANIMJOB_DPRINT_Matrices44(SMath::Mat44 const* pMatrices, U32F numJoints, U32F joint0=0);
void ANIMJOB_DPRINT_Scalars(SMath::Scalar const *pScalars, U32F numItems, U32F scalar0=0);
void ANIMJOB_DPRINT_ScalarsForQuaternionToEuler(SMath::Scalar const *pScalars, U32F numItems, Ice::Anim::SdkDriversRotSourceQuad const *pSourceData, U16 const *pDestOffset);
void ANIMJOB_DPRINT_ScalarsForLoadVecs(SMath::Scalar const *pScalars, U32F numItems, Ice::Anim::SdkDriversQuad const* pInputData);
void ANIMJOB_DPRINT_ScalarsForEvalCurves(SMath::Scalar const *pScalars, U32F numItems, U16 const* pInputTable);
void ANIMJOB_DPRINT_ScalarsForSdkCopy(SMath::Scalar const *pScalars, U32F numItems, U16 const* pInputTable);
void ANIMJOB_DPRINT_DefaultsForStoreVecs(SMath::Vec4 const *pVec4, U32F numItems, Ice::Anim::SdkDrivenPair const *pDrivenData);
void ANIMJOB_DPRINT_UniformKeyFrame(U32F numJoints, U8 const *pKeyframeTable, U8 const *pSizeData, U32F sizeStep);
void ANIMJOB_DPRINT_NonUniformKeyFrames(U32F numJoints, U8 const *pKeyTable, U8 const *pDataTable, U8 const *pSizeData, U32F sizeStep);
void ANIMJOB_DPRINT_FloatsAtOffsets(F32 const *pFloats, U32F numOffsets, U16 const *pOffsets);
void ANIMJOB_DPRINT_FloatsForPatchConstraintData(F32 const *pFloats, U32F numOffsets, U16 const *pOffsets);

static inline void ANIMJOB_DPRINT_Mem_SdkRotSourceQuad(char const *label, void const *p, U32F size)	// prints data as {U8,U8,U16}[]
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_Mem_SdkRotSourceQuad(p, size);
}
static inline void ANIMJOB_DPRINT_ValidBits(char const* label, Ice::Anim::ValidBits const &validBits)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_ValidBits(validBits);
}
static inline void ANIMJOB_DPRINT_JointParams(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numJoints, U32F joint0=0)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParams(pJointParams, numJoints, joint0);
}
static inline void ANIMJOB_DPRINT_JointParamsAtOffsets(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numOffsets, U16 const *pOffsets)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsAtOffsets(pJointParams, numOffsets, pOffsets);
}
static inline void ANIMJOB_DPRINT_JointParamsByValidBits(char const *label, Ice::Anim::JointParams const *pJointParams, Ice::Anim::ValidBits &validBits)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsByValidBits(pJointParams, validBits);
}
static inline void ANIMJOB_DPRINT_JointParamsForDecompress(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numIndices, U16 const *pIndices, U32F componentOffset)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsForDecompress(pJointParams, numIndices, pIndices, componentOffset);
}
static inline void ANIMJOB_DPRINT_JointParamsByConstraintInfo(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numInfo, Ice::Anim::ConstraintInfoQuad const *pInfo)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsByConstraintInfo(pJointParams, numInfo, pInfo);
}
static inline void ANIMJOB_DPRINT_JointParamsByEulerToQuatInfo(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numInfo, Ice::Anim::SdkDrivenRotDestQuad const *pInfo)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsByEulerToQuatInfo(pJointParams, numInfo, pInfo);
}
static inline void ANIMJOB_DPRINT_JointParamsForStoreVecs(char const *label, Ice::Anim::JointParams const *pJointParams, U32F numInfo, Ice::Anim::SdkDrivenPair const *pDrivenData)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointParamsForStoreVecs(pJointParams, numInfo, pDrivenData);
}
static inline void ANIMJOB_DPRINT_JointTransforms(char const *label, Ice::Anim::JointTransform const* pTransforms, U32F numJoints, U32F joint0=0)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointTransforms(pTransforms, numJoints, joint0);
}
static inline void ANIMJOB_DPRINT_JointTransformsByParentInfo(char const *label, Ice::Anim::JointTransform const *pTransforms, U32F numParentInfo, Ice::Anim::JointParentingQuad const *pInfo)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_JointTransformsByParentInfo(pTransforms, numParentInfo, pInfo);
}
static inline void ANIMJOB_DPRINT_Matrices34(char const *label, SMath::Mat34 const* pMatrices, U32F numJoints, U32F joint0=0)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_Matrices34(pMatrices, numJoints, joint0);
}
static inline void ANIMJOB_DPRINT_Matrices44(char const *label, SMath::Mat44 const* pMatrices, U32F numJoints, U32F joint0=0)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_Matrices44(pMatrices, numJoints, joint0);
}
static inline void ANIMJOB_DPRINT_Scalars(char const *label, SMath::Scalar const *pScalars, U32F numItems, U32F scalar0=0)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_Scalars(pScalars, numItems, scalar0);
}
static inline void ANIMJOB_DPRINT_ScalarsForQuaternionToEuler(char const *label, SMath::Scalar const *pScalars, U32F numItems, Ice::Anim::SdkDriversRotSourceQuad const *pSourceData, U16 const *pDestOffset)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_ScalarsForQuaternionToEuler(pScalars, numItems, pSourceData, pDestOffset);
}
static inline void ANIMJOB_DPRINT_ScalarsForLoadVecs(char const *label, SMath::Scalar const *pScalars, U32F numItems, Ice::Anim::SdkDriversQuad const* pInputData)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_ScalarsForLoadVecs(pScalars, numItems, pInputData);
}
static inline void ANIMJOB_DPRINT_ScalarsForEvalCurves(char const *label, SMath::Scalar const *pScalars, U32F numItems, U16 const* pInputTable)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_ScalarsForEvalCurves(pScalars, numItems, pInputTable);
}
static inline void ANIMJOB_DPRINT_ScalarsForSdkCopy(char const *label, SMath::Scalar const *pScalars, U32F numItems, U16 const* pInputTable)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_ScalarsForSdkCopy(pScalars, numItems, pInputTable);
}
static inline void ANIMJOB_DPRINT_DefaultsForStoreVecs(char const *label, SMath::Vec4 const *pVec4, U32F numItems, Ice::Anim::SdkDrivenPair const *pDrivenData)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_DefaultsForStoreVecs(pVec4, numItems, pDrivenData);
}
static inline void ANIMJOB_DPRINT_UniformKeyFrame(char const *label, U32F numJoints, U8 const *pKeyframeTable, U8 const *pSizeData, U32F sizeStep)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_UniformKeyFrame(numJoints, pKeyframeTable, pSizeData, sizeStep);
}
static inline void ANIMJOB_DPRINT_NonUniformKeyFrames(char const *label, U32F numJoints, U8 const *pKeyTable, U8 const *pDataTable, U8 const *pSizeData, U32F sizeStep)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_NonUniformKeyFrames(numJoints, pKeyTable, pDataTable, pSizeData, sizeStep);
}
static inline void ANIMJOB_DPRINT_FloatsAtOffsets(char const *label, F32 const *pFloats, U32F numOffsets, U16 const *pOffsets)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_FloatsAtOffsets(pFloats, numOffsets, pOffsets);
}
static inline void ANIMJOB_DPRINT_FloatsForPatchConstraintData(const char *label, F32 const *pFloats, U32F numOffsets, U16 const *pOffsets)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	ANIMJOB_DPRINT_FloatsForPatchConstraintData(pFloats, numOffsets, pOffsets);
}
#else	//#if ANIMJOB_DISPATCHER_DPRINTS

# if ICE_COMPILER_GCC
# define ANIMJOB_DPRINT_Mem_SdkRotSourceQuad(...)
# define ANIMJOB_DPRINT_ValidBits(...)
# define ANIMJOB_DPRINT_JointParams(...)
# define ANIMJOB_DPRINT_JointParamAtOffset(...)
# define ANIMJOB_DPRINT_JointParamsAtOffsets(...)
# define ANIMJOB_DPRINT_JointParamsByValidBits(...)
# define ANIMJOB_DPRINT_JointParamsForDecompress(...)
# define ANIMJOB_DPRINT_JointParamsByConstraintInfo(...)
# define ANIMJOB_DPRINT_JointParamsByEulerToQuatInfo(...)
# define ANIMJOB_DPRINT_JointParamsForStoreVecs(...)
# define ANIMJOB_DPRINT_JointTransforms(...)
# define ANIMJOB_DPRINT_JointTransformsByParentInfo(...)
# define ANIMJOB_DPRINT_Matrices34(...)
# define ANIMJOB_DPRINT_Matrices44(...)
# define ANIMJOB_DPRINT_Scalars(...)
# define ANIMJOB_DPRINT_ScalarsForQuaternionToEuler(...)
# define ANIMJOB_DPRINT_ScalarsForLoadVecs(...)
# define ANIMJOB_DPRINT_ScalarsForEvalCurves(...)
# define ANIMJOB_DPRINT_ScalarsForSdkCopy(...)
# define ANIMJOB_DPRINT_DefaultsForStoreVecs(...)
# define ANIMJOB_DPRINT_UniformKeyFrame(...)
# define ANIMJOB_DPRINT_NonUniformKeyFrames(...)
# define ANIMJOB_DPRINT_FloatsAtOffsets(...)
# define ANIMJOB_DPRINT_FloatsForPatchConstraintData(...)
# else
// __noop() (and sizeof() for that matter) don't seem to keep Visual C from processing the parameters
#  define DEBUG_ExtractWord(validBits, i)								(0)
#  define DEBUG_CountConstraints(numInfo, pInfo)						(0)
#  define DEBUG_CountAimUpVectors(numDests, pInfo)						(0)
#  define DEBUG_SizeofUniformKeyFrame(numJoints, pSizeData, sizeStep)	(0)
#  define DEBUG_SizeofNonUniformKeys(numJoints, pKeys)					(0)

#  define ANIMJOB_DPRINT_Mem_SdkRotSourceQuad			__noop
#  define ANIMJOB_DPRINT_ValidBits						__noop
#  define ANIMJOB_DPRINT_JointParams					__noop
#  define ANIMJOB_DPRINT_JointParamAtOffset				__noop
#  define ANIMJOB_DPRINT_JointParamsAtOffsets			__noop
#  define ANIMJOB_DPRINT_JointParamsByValidBits			__noop
#  define ANIMJOB_DPRINT_JointParamsForDecompress		__noop
#  define ANIMJOB_DPRINT_JointParamsByConstraintInfo	__noop
#  define ANIMJOB_DPRINT_JointParamsByEulerToQuatInfo	__noop
#  define ANIMJOB_DPRINT_JointParamsForStoreVecs		__noop
#  define ANIMJOB_DPRINT_JointTransforms				__noop
#  define ANIMJOB_DPRINT_JointTransformsByParentInfo	__noop
#  define ANIMJOB_DPRINT_Matrices34						__noop
#  define ANIMJOB_DPRINT_Matrices44						__noop
#  define ANIMJOB_DPRINT_Scalars						__noop
#  define ANIMJOB_DPRINT_ScalarsForQuaternionToEuler	__noop
#  define ANIMJOB_DPRINT_ScalarsForLoadVecs				__noop
#  define ANIMJOB_DPRINT_ScalarsForEvalCurves			__noop
#  define ANIMJOB_DPRINT_ScalarsForSdkCopy				__noop
#  define ANIMJOB_DPRINT_DefaultsForStoreVecs			__noop
#  define ANIMJOB_DPRINT_UniformKeyFrame				__noop
#  define ANIMJOB_DPRINT_NonUniformKeyFrames			__noop
#  define ANIMJOB_DPRINT_FloatsAtOffsets				__noop
#  define ANIMJOB_DPRINT_FloatsForPatchConstraintData	__noop
# endif

#endif	//#if ANIMJOB_DISPATCHER_DPRINTS ... #else

#endif // ICEANIMDEBUG_H
