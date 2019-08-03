/*
 * Copyright (c) 2003, 2004 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ANIMSTACK_H
#define ICE_ANIMSTACK_H

#include "icejointanim.h"

namespace Ice {
	namespace Anim {
		/// The purpose of this stack is to be able to set up animation nodes more quickly
		struct AnimationStack {
			AnimationNode **apStackNodes;			//!< Allocated array of AnimationNode pointers on stack
			U32 maxStackSize;						//!< Number of AnimationNode pointers allocated
			I32 top;								//!< Current top element
			BatchJob::MemoryBuffer *pBuffer;		//!< Pointer to a buffer to allocate nodes from (must persist only until job is batched)
			BatchJob::MemoryBuffer *pDataBuffer;	//!< Pointer to a buffer to allocate data from (must persist until job is complete)
		};

		/// Creates a stack initialized to the given size.
		///		Note: we differentiate node allocation from data allocation, since nodes
		///		only need to exist until the call to AnimateJointsBatched(), while data
		///		must persist until the batched task has finished running.
		///
		///		Typically, pDataBuffer can be shared with the buffer passed to
		///		AnimateJointsBatched(), which will be Reset() once per frame, while
		///		pMemoryBuffer can be a separate buffer, which can be Reset() after each
		///		call to AnimateJointsBatched().
		///
		///		Only the helper calls which build data (as noted below) allocate from
		///		pDataBuffer.
		///
		/// \return 				Resulting stack
		/// \param maxStackSize 	Number of pointers to allocate
		/// \param pMemoryBuffer	Pointer to a memory buffer to output AnimationNodes into
		/// \param pDataMemoryBuffer	Pointer to a memory buffer to output data into
		AnimationStack *InitializeStack(U32 maxStackSize, BatchJob::MemoryBuffer *pMemoryBuffer, BatchJob::MemoryBuffer *pDataMemoryBuffer);

		/// Finalizes the AnimationStack, deleting all temporary data associated with it.
		///		Note that pStack is an invalid pointer after this call.
		/// \return 				Root node resulting from evaluating the stack
		/// \param pStack 			Stack to be finalized
		AnimationNode  *FinalizeStack(AnimationStack *pStack);

		/// Creates a ClipNode evaludated at the given frame on the top of the stack.
		/// \return					A pointer to the node constructed
		/// \param pStack 			Stack on which to push the node
		/// \param pClip 			Clip from which to create the ClipNode
		/// \param fFrame 			Animation frame for which the clip will be evaluated.
		/// \param pValidMask		Optional pointer to an array of JointHierarchy::m_numChannelGroups ValidBits structures which will modify which joints this clip affects.
		ClipNode*
		PushClipAtFrame(AnimationStack *pStack, ClipData const *pClip, float fFrame, ValidBits const *pValidMask = NULL);
		/// Creates a ClipNode evaluated at the given time on the top of the stack.
		/// \return					A pointer to the node constructed
		/// \param pStack 			Stack on which to push the node
		/// \param pClip 			Clip from which to create the ClipNode
		/// \param fTime 			Animation time for which the clip will be evaluated.
		/// \param pValidMask		Optional pointer to an array of JointHierarchy::m_numChannelGroups ValidBits structures which will modify which joints this clip affects.
		ClipNode*
		PushClipAtTime(AnimationStack *pStack, ClipData const *pClip, float fTime, ValidBits const *pValidMask = NULL);
		/// Legacy function, identical to PushClipAtTime() for backwards compatibility
		ClipNode*
		PushClip(AnimationStack *pStack, ClipData const *pClip, float fTime, ValidBits const *pValidMask = NULL);
		/// Creates a ClipNode evaluated at the given phase on the top of the stack.
		/// \return					A pointer to the node constructed
		/// \param pStack 			Stack on which to push the node
		/// \param pClip 			Clip from which to create the ClipNode
		/// \param fPhase 			Animation phase (0.0f - 1.0f) for which the clip will be evaluated.
		/// \param pValidMask		Optional pointer to an array of JointHierarchy::m_numChannelGroups ValidBits structures which will modify which joints this clip affects.
		ClipNode*
		PushClipAtPhase(AnimationStack *pStack, ClipData const *pClip, float fPhase, ValidBits const *pValidMask = NULL);

		/// Creates a PoseNode with the given parameters on the top of the stack.
		/// \return					A pointer to the node constructed
		/// \param pStack			Stack on which to push the node
		/// \param pJointParams		Pointer to the joints in a pose, which should contain JointHierarchy::m_numJointsInSet[0] JointParams.
		/// \param pFloatChannels	Pointer to the float channels in a pose, which should contain JointHierarchy::m_numFloatChannels floats.
		/// \param aValidBits		Pointer to an array of JointHierarchy::m_numChannelGroups ValidBits structures.
		PoseNode*
		PushPose(AnimationStack *pStack, JointParams const *pJointParams, float const *pFloatChannels, ValidBits const *pValidBits);

		/// Creates a SnapshotNode with the given parameters on the top of the stack,
		/// 	consuming the top node as an input.
		/// \return					A pointer to the node constructed
		/// \param pStack			Stack on which to push the node
		/// \param pJointParams		Pointer to a buffer to output JointHierarchy::m_numAnimatedJoints JointParams as a snapshot pose.
		/// \param pFloatChannels	Pointer to a buffer to output JointHierarchy::m_numFloatChannels floats as a snapshot pose.
		/// \param aValidBits		Pointer to an array of JointHierarchy::m_numChannelGroups ValidBits structures in which to return the valid joints and float channels.
		SnapshotNode*
		IssueSnapshot(AnimationStack *pStack, JointParams *pJointParams, float *pFloatChannels, ValidBits *pValidBits);

		/// Creates a BlendNode to blend the two top elements with the given parameters, on the top of the stack,
		///		consuming the top two nodes as input.
		///		Provided data must be valid - properly sorted into joint groups with ChannelFactor
		///		arrays 16 byte aligned in memory that will persist until the animation task completes.
		/// \return							A pointer to the node constructed
		/// \param pStack					Stack on which to push the node
		/// \param eMode					Blend mode
		/// \param fBlendFactor				Global blending factor, if not overridden by pChannelFactors
		/// \param numChannelFactorGroups	Number of channel groups (== JointHierarchy::m_numChannelGroups)
		/// \param pNumChannelFactors		An array containing the number of channel factors per channel group.
		/// \param ppChannelFactors			Per-channel blending factors, one array per channel group, ordered by increasing id.
		BlendNode*
		IssueBlend(AnimationStack *pStack, BlendMode eMode, float fBlendFactor, U32 numChannelFactorGroups, U32 *pNumChannelFactors, ChannelFactor **ppChannelFactors);
		/// Creates a BlendNode to blend the two top elements with the given parameters, on the top of the stack.
		///		consuming the top two nodes as input.
		/// Blends with a constant blend factor (numJointGroups = 0, pNumChannelFactors = NULL, ppChannelFactors = NULL).
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		/// \param eMode				Blend mode
		/// \param fBlendFactor			Global blending factor, if not overridden by pChannelFactors
		/// \param numChannelFactors	The number of elements in pChannelFactors
		BlendNode*
		IssueBlend(AnimationStack *pStack, BlendMode eMode, float fBlendFactor);
		/// Creates a BlendNode to blend the two top elements with the given parameters, on the top of the stack,
		///		consuming the top two nodes as input.
		///		Allocates data from pDataBuffer to split the provided data into groups.
		///		Expects the input list data to be ordered by increasing group id.
		///		(i.e. sorted by increasing jointId/kJointGroupSize).
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		/// \param eMode				Blend mode
		/// \param fBlendFactor			Global blending factor, if not overridden by pChannelFactors
		/// \param numAnimatedJoints	The number of animated joints in the hierarchy (== JointHierarchy::m_numAnimatedJoints)
		/// \param numJointFactors		The number of elements in pJointFactors
		/// \param pJointFactors		Per-joint blending factors, ordered by increasing id.
		/// \param numFloatChannels		The number of float channels for the hierarchy (== JointHierarchy::m_numFloatChannels)
		/// \param numFloatFactors		The number of elements in pFloatFactors
		/// \param pFloatFactors		Per-float channel blending factors, ordered by increasing id.
		BlendNode*
		IssueBlend(AnimationStack *pStack, BlendMode eMode, float fBlendFactor, U32 numAnimatedJoints, U32 numJointFactors, ChannelFactor const *pJointFactors, U32 numFloatChannels, U32 numFloatFactors, ChannelFactor const *pFloatFactors);

		/// Creates a FlipNode with the given parameters on the top of the stack,
		///		consuming the top node as an input.
		///		Provided data must be valid - properly sorted into joint groups with FlipOp and JointOffset
		///		arrays 16 byte aligned in memory that will persist until the animation task completes.
		/// \return					A pointer to the node constructed
		/// \param pStack			Stack on which to push the node
		/// \param eType			Type of flip to perform (kFlipUnary or kFlipBinary)
		/// \param numJointGroups	Number of joint groups (== ((JointHierarchy::m_numAnimatedJoints + kJointGroupSize - 1)/kJointGroupSize))
		/// \param pNumFlips		Array pNumFlipOps[numJointGroups]: number of joints or joint pairs to flip per group (must be a multiple of 2 if kFlipUnary)
		/// \param ppFlipOps		Array ppFlipOps[numJointGroups][ pNumFlips[iGroup] ]  of numFlipOps U32s.  The kFlipOp* constants may be or'd together to create these.
		/// \param ppJointOffsets	Array ppJointOffsets[numJointGroups[ pNumFlips[iGroup] (*2 if kFlipUnary) ] of offsets (for kFlipUnary) or offset pairs (for kFlipBinary).  Offsets are joint indices * sizeof(JointParams).
		FlipNode*
		IssueFlip(AnimationStack *pStack, FlipType eType, U32 numJointGroups, U16 *pNumFlips, U32 **ppFlipOps, U16 **ppJointOffsets);
		/// Creates a FlipNode with the given parameters on the top of the stack,
		///		consuming the top node as an input.
		///		Allocates data from pDataBuffer to split the provided data into groups.
		///		expects the input list data to be ordered by increasing group id.
		///		(i.e. sorted by increasing jointId/kJointGroupSize).
		///		Note that binary flip data may not include joint pairs that flip joints in different groups.
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		/// \param eType				Type of flip to perform (kFlipUnary or kFlipBinary)
		/// \param numAnimatedJoints	The number of animated joints in the hierarchy (== JointHierarchy::m_numAnimatedJoints)
		/// \param numFlips				Total number of joints or joint pairs to flip.
		/// \param pFlipOps				Array pFlipOps[numFlips]  of numFlipOps U32s.  The kFlipOp* constants may be or'd together to create these.
		/// \param pJointIds			Array pJointIds[numFlips (*2 if kFlipUnary) ] of joint indices (for kFlipUnary) or joint index pairs (for kFlipBinary).
		FlipNode*
		IssueFlip(AnimationStack *pStack, FlipType eType, U32 numAnimatedJoints, U32 numFlips, U32 const *pFlipOps, U16 const *pJointIds);

		/// Creates a PlugInUnaryNode with the given parameters on the top of the stack,
		///		which will create a pose for later consumption.
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		///	\param batchCallback		A callback function for batched mode execution
		///	\param immediateCallback	A callback function for immediate mode execution
		///	\param pContext				A pointer to arbitrary context data to pass the callback
		UnaryPlugInNode*
		PushPosePlugIn(AnimationStack *pStack, UnaryPlugInNodeBatchCallback batchCallback, UnaryPlugInNodeImmediateCallback immediateCallback, void *pContext);

		/// Creates a PlugInUnaryNode with the given parameters on the top of the stack,
		///		which consume the top node as input.
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		///	\param batchCallback		A callback function for batched mode execution
		///	\param immediateCallback	A callback function for immediate mode execution
		///	\param pContext				A pointer to arbitrary context data to pass the callback
		UnaryPlugInNode*
		IssueUnaryPlugIn(AnimationStack *pStack, UnaryPlugInNodeBatchCallback batchCallback, UnaryPlugInNodeImmediateCallback immediateCallback, void *pContext);

		/// Creates a PlugInBinaryNode with the given parameters on the top of the stack,
		///		which consume the top two nodes as input.
		/// \return						A pointer to the node constructed
		/// \param pStack				Stack on which to push the node
		///	\param batchCallback		A callback function for batched mode execution
		///	\param immediateCallback	A callback function for immediate mode execution
		///	\param pContext				A pointer to arbitrary context data to pass the callback
		BinaryPlugInNode*
		IssueBinaryPlugIn(AnimationStack *pStack, BinaryPlugInNodeBatchCallback batchCallback, BinaryPlugInNodeImmediateCallback immediateCallback, void *pContext);
	}
}

#endif // ICE_ANIMSTACK_H
