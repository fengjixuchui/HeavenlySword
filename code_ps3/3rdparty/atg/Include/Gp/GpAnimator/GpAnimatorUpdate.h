//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Private header file - GpAnimatorUpdate data structures 

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ANIMATOR_UPDATE_H
#define GP_ANIMATOR_UPDATE_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__
#	include <../Fp/FpAnim/FpAnimClip.h>
#	include <../Gp/GpAnimator/GpAnimator.h>
#	include <../Gp/GpSkeleton/GpSkeleton.h>
#else //__SPU__
#	include <Fp/FpAnim/FpAnimClip.h>
#	include <Gp/GpAnimator/GpAnimator.h>
#	include <Gp/GpSkeleton/GpSkeleton.h>
#endif//__SPU__

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Update data 

	@warning	This structure must now has the same size both on PPU and SPU since it's allocated
				by a sizeof on the PPU.. It's poor, I know.. 
**/
//--------------------------------------------------------------------------------------------------

FW_ALIGN_BEGIN( 128 )
class GpAnimatorUpdate
{
public:
	// This is the core update code (PC/PPU version)
	GpAnimator*		Update( GpAnimator* pAnimator );

	// Gathered keyframe (full precision version) 
	// (different from the the real FpAnimKeyframeHermite because we DMA-in two consecutive keyframes)
	class	EvalKeyframeHermite
	{
	public:
		FwVector4	m_valueA;
		FwVector4	m_tanA;	
		FwVector4	m_valueB;
		FwVector4	m_tanB;	
	};

	// Gathered keyframe (half precision version)
	class	EvalKeyframeHalfHermite
	{
	public:
		u64			m_valueA;	// FP16 - not a u64
		u64			m_tanA;		// FP16 - not a u64
		u64			m_valueB;	// FP16 - not a u64
		u64			m_tanB;		// FP16 - not a u64
	};

private:
	// Internal functions used by Update
	void		CmdEvaluate( int destSlot, int idxAnimClipHeader, GpAnimation* pPPUAnimation, float timeValue, int loopCount, int evalFlags );
	void		CmdEvaluatePrefetch( int destSlot, int idxAnimClipHeader, GpAnimation* pPPUAnimation, float timeValue, int evalFlags );
	void		CmdEvaluateHermiteChannels( const int destSlot, const FpAnimTimeblock* FW_RESTRICT pTimeblock, const float timeValue );
	void		CmdEvaluateHalfHermiteChannels( const int destSlot, const FpAnimTimeblock* FW_RESTRICT pTimeblock, const float timeValue );
	void		CmdEvaluatePackedHermiteChannels( const int destSlot, const FpAnimTimeblock* FW_RESTRICT pTimeblock, const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpecArray, const u16* FW_RESTRICT pPackingSpecIndex,	const FpAnimKeyframeTransform* FW_RESTRICT pTransformArray, const u16* FW_RESTRICT pTransformIndex, const float timeValue );
	void		CmdBlend( int destSlot, int lhsSlot, int rhsSlot, float alpha );
	void		CmdCompose( int destSlot, int lhsSlot, int rhsSlot );
	void		CmdInvert( int destSlot, int srcSlot );
	void		CmdApplyDelta( int destSlot, int baseSlot, int deltaSlot );
	void		CmdInputSkeletalJoints( int destSlot );
	void		CmdOutput( int srcSlot );
	void		CmdOutputSkeletalJoints( int srcSlot, GpJoint* pDestAddress );
	void		CmdOutputSkeletalTransforms( int srcSlot, FwTransform* pDestAddress );
	void		CmdOutputUserChannels( int srcSlot, FwVector4* pDestAddress );

	// Compute space for our blend weight scalars, local->slot bindings, and current time block pointers. These are all DMA'd in one chunk.
	enum
	{
		kBlendScalarSize		= ( ( ( sizeof( float ) * FpAnimClipDef::kMaxChannels ) + 127 ) & ~127 ),
		kLocalSlotBindingSize	= ( ( ( sizeof( s16 ) * FpAnimClipDef::kMaxChannels ) + 127 ) & ~127 ),
		kAnimationInfoSize		= kBlendScalarSize + kLocalSlotBindingSize  
	};

	enum
	{
		kAnimClipDefMaxSize		= 20480,					// Anim clip header. Arbitrarily limited to this size - more or less temporarily.
		kEvalBatchSize			= 256,						// Above this we don't get any performance improvement but just waste memory
		kGatherBatchSize		= 16						// To alternate small search and gather DMAs
	};

	// GpAnimator and GpAnimation objects are defined as u8 arrays, as constructors & destructors are private.
	FW_ALIGN_BEGIN( 128 )	u8						m_animator[ sizeof( GpAnimator ) ]												FW_ALIGN_END( 128 );	// R/W	:	Control GpAnimator
	FW_ALIGN_BEGIN( 128 )	GpJoint					m_bindPoseData[ FpAnimClipDef::kMaxItems ]										FW_ALIGN_END( 128 );	// R	:	Optional bind pose data
	FW_ALIGN_BEGIN( 128 )	GpJointLinkage			m_jointLinkage[ FpAnimClipDef::kMaxItems ]										FW_ALIGN_END( 128 );	// R	:	Optional joint linkage data
	FW_ALIGN_BEGIN( 128 )	FwTransform				m_skeletalMatrices[ FpAnimClipDef::kMaxItems ]									FW_ALIGN_END( 128 );	// W	:	Optional skeletal transforms
	FW_ALIGN_BEGIN( 128 )	FwTransform				m_skinningMatrices[ FpAnimClipDef::kMaxItems ]									FW_ALIGN_END( 128 );	// R/W	:	Optional skinning transforms
	FW_ALIGN_BEGIN( 128 )	u8						m_animationHeader[ GpAnimator::kNumberOfSlots ][ sizeof( GpAnimator ) ]			FW_ALIGN_END( 128 );	// R/W	:	GpAnimation
	FW_ALIGN_BEGIN( 128 )	u8						m_animationInfo[ GpAnimator::kNumberOfSlots ][ kAnimationInfoSize ]				FW_ALIGN_END( 128 );	// TEMP	:	Scalars,bindings,pointers,FpAnimClipDef

	// Command List block space
	FW_ALIGN_BEGIN( 128 )	GpAnimator::AnimCommand	m_commandBlock[ GpAnimator::kDefaultCommandListSize ]							FW_ALIGN_END( 128 );	// R	:	A chunk of the command list for parsing

	// Stuff that lives regardless of PPU/SPU usage mode...
	FW_ALIGN_BEGIN( 128 )	FwVector4				m_evaluationSlot[ GpAnimator::kNumberOfSlots ][ FpAnimClipDef::kMaxChannels ]	FW_ALIGN_END( 128 );	// TEMP	:	Evaluation slots
	FW_ALIGN_BEGIN( 128 )	s32						m_slotToLocalIndex[ GpAnimator::kNumberOfSlots ][ FpAnimClipDef::kMaxChannels ]	FW_ALIGN_END( 128 );	// TEMP	:	Channel->Anim Item Bindings

	// Timeblock - currently only one at a time but should be pipelined better (padded to 128, to allow unaligned DMA)
	FW_ALIGN_BEGIN( 128 )	u8						m_timeblock[ 1 ][ FpAnimTimeblock::kMaxTimeblockSize + 128]						FW_ALIGN_END( 128 );	// R	:	Timeblock buffer + 128 bytes to realign DMA

	// AnimClipDef (padded to 128, to allow unaligned DMA)
	// Note that there is currently a limitation on the size of the anim object. It can be annoying because the timeblock index is in it.
	// In the future we should be able to "stream" parts of the timeblock index (obviously at an additional cost)
	FW_ALIGN_BEGIN( 128 )	u8						m_animClipDef[ 2 ][ kAnimClipDefMaxSize + 128 ];								FW_ALIGN_END( 128 );	// R	:	AnimClipDef buffer + 128 bytes to realign DMA

	// JointFlags buffer; since joints flags are misaligned in main memory, we leave an extra 128 bytes to align DMA to the same boundary 
	FW_ALIGN_BEGIN( 128 )	u8						m_jointFlags[ FpAnimClipDef::kMaxItems + 128 ];									FW_ALIGN_END( 128 );	// R	:	AnimClipDef buffer + 128 bytes to realign DMA

	// Room for mirroring specification array.. this is wasteful, but will be sorted once we rearrange memory layout of this system
	FW_ALIGN_BEGIN( 128 )	u32						m_mirrorSpecsArray[ FpAnimClipDef::kMaxItems ];									FW_ALIGN_END( 128 );	// R	:	Array of mirroring behaviour specifications

	// Evaluator temporary structures (far too big, space should be shared with other structures, since they don't live at the same time)
	FW_ALIGN_BEGIN( 128 )	u32						m_evalKfDmaList[ 2 * kEvalBatchSize ]											FW_ALIGN_END( 128 );	// TEMP	:	Gather DMA list for keyframes (see m_evalKeyFrames)
	FW_ALIGN_BEGIN( 128 )	u8						m_evalKeyFrames[ kEvalBatchSize * sizeof( EvalKeyframeHermite ) ]				FW_ALIGN_END( 128 );	// TEMP	:	Keyframes for one batch - Gathered with DMA GETL
	FW_ALIGN_BEGIN( 16 )	float					m_evalTimeIntervalStart[ kEvalBatchSize  ]										FW_ALIGN_END( 16 );		// TEMP	:	Interval end time (per gathered keyframe )
	FW_ALIGN_BEGIN( 16 )	float					m_evalTimeIntervalEnd[ kEvalBatchSize ]											FW_ALIGN_END( 16 );		// TEMP	:	Interval start time (per gathered keyframe )
	FW_ALIGN_BEGIN( 16 )	u32						m_evalBitOffset[ kEvalBatchSize ]												FW_ALIGN_END( 16 );		// TEMP	:	Keyframe bit offset (per gathered keyframe )
	
	// We need to track whether an evaluation slot has locomotion influence in it.. 
	FW_ALIGN_BEGIN( 16 )	bool					m_locomotionState[ GpAnimator::kNumberOfSlots ]									FW_ALIGN_END( 16 );

	// Temporary
	FW_ALIGN_BEGIN( 16 )	int						m_idxAnimClipHeader;															FW_ALIGN_END( 16 );	

	// Indirection allows more code to be the same between PPU and SPU, as workspace requirements are different.
	FW_ALIGN_BEGIN( 16 )	GpAnimator*				m_pAnimator																		FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	GpAnimator*				m_pPPUAnimator																	FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	const FpAnimClipDef*	m_pAnimClipDef[ GpAnimator::kNumberOfSlots ]									FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	const GpJoint*			m_pBindPoseData																	FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	const GpJointLinkage*	m_pJointLinkage																	FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	GpAnimation*			m_pAnimation[ GpAnimator::kNumberOfSlots ]										FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	const float*			m_pBlendWeightScalarArray[ GpAnimator::kNumberOfSlots ]							FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	s16*					m_pLocalToSlotBindingArray[ GpAnimator::kNumberOfSlots ]						FW_ALIGN_END( 16 );
	FW_ALIGN_BEGIN( 16 )	const u32*				m_pMirrorSpecsArray;															FW_ALIGN_END( 16 );

} FW_ALIGN_END( 128 );

//--------------------------------------------------------------------------------------------------
//	DMA TAG ALLOCATION - we assume job manager wants us to allocate from 31 downwards
//--------------------------------------------------------------------------------------------------

enum
{
	kDmaTagSlotBaseGet			= 32 - GpAnimator::kNumberOfSlots,	
	kDmaTagSlotBasePut			= kDmaTagSlotBaseGet - GpAnimator::kNumberOfSlots,
	kDmaTagGpAnimator			= kDmaTagSlotBasePut - 1,
	kDmaTagGpAnimation			= kDmaTagGpAnimator - GpAnimator::kNumberOfSlots,
	kDmaTagFpAnimClipDef		= kDmaTagGpAnimation - 2,		// (we only use on of the 2 clipdef storage slot because prefetching is disabled)
	kDmaTagKeyframeGatherBase	= kDmaTagFpAnimClipDef - 2,		// (batch evaluator, 2 in flight at the the same time)
	kDmaTagTimeblock			= kDmaTagKeyframeGatherBase - 1,	
	kDmaTagBindPose				= kDmaTagTimeblock - 1,
	kDmaTagJointLinkage 		= kDmaTagBindPose - 1,
	kDmaTagMatrices				= kDmaTagJointLinkage - 1,
	kDmaTagJointFlags			= kDmaTagMatrices - 1,	
	kDmaTagSkinningTransforms   = kDmaTagJointFlags - 1,
	kDmaTagCommandBlock			= kDmaTagSkinningTransforms - 1,	// just single buffered at the moment

	kDmaTagMirrorSpecs			= kDmaTagJointFlags,				// IMPORTANT: This is reusing a tag!

	kDmaTagMaskAny  = ( 1 << ( kDmaTagSlotBaseGet + 0 ) )
					| ( 1 << ( kDmaTagSlotBaseGet + 1 ) )
					| ( 1 << ( kDmaTagSlotBaseGet + 2 ) )
					| ( 1 << ( kDmaTagSlotBaseGet + 3 ) )
					| ( 1 << ( kDmaTagSlotBasePut + 0 ) )
					| ( 1 << ( kDmaTagSlotBasePut + 1 ) )
					| ( 1 << ( kDmaTagSlotBasePut + 2 ) )
					| ( 1 << ( kDmaTagSlotBasePut + 3 ) )
					| ( 1 << ( kDmaTagGpAnimator ) )
					| ( 1 << ( kDmaTagGpAnimation + 0) )
					| ( 1 << ( kDmaTagGpAnimation + 1) )
					| ( 1 << ( kDmaTagGpAnimation + 2) )
					| ( 1 << ( kDmaTagGpAnimation + 3) )
					| ( 1 << ( kDmaTagFpAnimClipDef + 0) )
					| ( 1 << ( kDmaTagFpAnimClipDef + 1) )
					| ( 1 << ( kDmaTagKeyframeGatherBase + 0 ) )	
					| ( 1 << ( kDmaTagKeyframeGatherBase + 1 ) )
					| ( 1 << ( kDmaTagTimeblock ) )
					| ( 1 << ( kDmaTagBindPose ) )
					| ( 1 << ( kDmaTagJointLinkage ) )
					| ( 1 << ( kDmaTagMatrices ) )		
					| ( 1 << ( kDmaTagJointFlags ) )
					| ( 1 << ( kDmaTagSkinningTransforms ) )
					| ( 1 << ( kDmaTagCommandBlock ) )
					| ( 1 << ( kDmaTagMirrorSpecs ) )
};


//--------------------------------------------------------------------------------------------------
//	ASM OR C++ FUNCTIONS 
//--------------------------------------------------------------------------------------------------
// /!\ WARNING !
// - ASM function don't explicitely take a "count4" parameter anymore, and will silentely overrun 
//   by up to 3 iterations, unless specified differently.
// - This has been done this way to be compatible with quadword validation in C++ implementation
//   (as opposed to having C++ implementation overrun the same way)
//--------------------------------------------------------------------------------------------------

extern "C"
void GpAnimatorBlendJoint( 			GpJoint* FW_RESTRICT  pDestJoint, 
						   			s32* FW_RESTRICT pDestSlotToLocalIndex,
						   			const GpJoint * FW_RESTRICT pLhsJoint,
						   			s32* FW_RESTRICT pLhsSlotToLocalIndex,
						   			const GpJoint* FW_RESTRICT pRhsJoint,
						   			s32* FW_RESTRICT pRhsSlotToLocalIndex,
						   			const float* FW_RESTRICT pBlendScalars,
 					 	   			float alpha,
						   			u32 count );
extern "C"
void GpAnimatorBlendUser( 			FwVector4* FW_RESTRICT  pDest, 
						  			s32* FW_RESTRICT pDestSlotToLocalIndex,
						  			const FwVector4* FW_RESTRICT pLhs,
						  			s32* FW_RESTRICT pLhsSlotToLocalIndex,
						  			const FwVector4* FW_RESTRICT pRhs,
						  			s32* FW_RESTRICT pRhsSlotToLocalIndex,
						  			const float* FW_RESTRICT pBlendScalars,
						  			float alpha,
						  			u32 count );

extern "C"
void GpAnimatorComposeJoint( GpJoint* FW_RESTRICT  pDestJoint, 
							 s32* FW_RESTRICT pDestSlotToLocalIndex,
							 const GpJoint * FW_RESTRICT pLhsJoint,
							 s32* FW_RESTRICT pLhsSlotToLocalIndex,
							 const GpJoint* FW_RESTRICT pRhsJoint,
							 s32* FW_RESTRICT pRhsSlotToLocalIndex,
							 u32 count );
extern "C"
void GpAnimatorComposeUser( FwVector4* FW_RESTRICT  pDest, 
							s32* FW_RESTRICT pDestSlotToLocalIndex,
							const FwVector4* FW_RESTRICT pLhs,
							s32* FW_RESTRICT pLhsSlotToLocalIndex,
							const FwVector4* FW_RESTRICT pRhs,
							s32* FW_RESTRICT pRhsSlotToLocalIndex,
							u32 count );

extern "C"
void GpAnimatorInvertJoint( GpJoint* FW_RESTRICT  pDestJoint, 
							s32* FW_RESTRICT pDestSlotToLocalIndex,
							const GpJoint * FW_RESTRICT pSrcJoint,
							s32* FW_RESTRICT pSrcSlotToLocalIndex,
							u32 count );
extern "C"
void GpAnimatorInvertUser( FwVector4* FW_RESTRICT  pDest, 
							s32* FW_RESTRICT pDestSlotToLocalIndex,
							const FwVector4* FW_RESTRICT pSrc,
							s32* FW_RESTRICT pSrcSlotToLocalIndex,
							u32 count );

extern "C"
void GpAnimatorApplyDeltaJoint( GpJoint* FW_RESTRICT  pDestJoint, 
								s32* FW_RESTRICT pDestSlotToLocalIndex,
								const GpJoint * FW_RESTRICT pLhsJoint,
								s32* FW_RESTRICT pLhsSlotToLocalIndex,
								const GpJoint* FW_RESTRICT pRhsJoint,
								s32* FW_RESTRICT pRhsSlotToLocalIndex,
								u32 count );
extern "C"
void GpAnimatorApplyDeltaUser( FwVector4* FW_RESTRICT  pDest, 
							   s32* FW_RESTRICT pDestSlotToLocalIndex,
							   const FwVector4* FW_RESTRICT pLhs,
							   s32* FW_RESTRICT pLhsSlotToLocalIndex,
							   const FwVector4* FW_RESTRICT pRhs,
							   s32* FW_RESTRICT pRhsSlotToLocalIndex,
							   u32 count );

extern "C"
void GpAnimatorLocalMatrices( 		FwTransform* FW_RESTRICT pOutput, 
							  		const GpJoint* FW_RESTRICT pInputRts, 
							  		const GpJointLinkage* FW_RESTRICT pJointLinkage,
							  		int count );
extern "C"
void GpAnimatorWorldMatrices( 		FwTransform* pOutput, 
							  		const FwTransform* FW_RESTRICT pRoot,
							  		const FwTransform* FW_RESTRICT pInputLocal, 	
							  		const GpJointLinkage* FW_RESTRICT pJointLinkage, 								  
							  		int count );
extern "C"
void GpAnimatorEulerToQuat(			FwQuat* FW_RESTRICT pOut, u32* FW_RESTRICT pOutIndices, 
									FwVector4* FW_RESTRICT pIn,  int count );
extern "C"
void GpAnimatorQuatToEuler( 		FwVector4* FW_RESTRICT pOut, FwQuat* FW_RESTRICT pIn,
						    		u32* FW_RESTRICT pInIndices, int count );

extern "C" 
void GpAnimatorEvalConstantDirect(	FwVector4* FW_RESTRICT pEvaluationSlot, 
						     		const s16* FW_RESTRICT pLocalToSlotBindingArray, 
						     		const FwVector4* FW_RESTRICT pConstantData, 
						     		const int channelStart, const int channelCount );

extern "C" 
void GpAnimatorEvalConstantIndirect(FwVector4* FW_RESTRICT pEvaluationSlot, 
						     		const s16* FW_RESTRICT pLocalToSlotBindingArray, 
						     		const FwVector4* FW_RESTRICT pConstantDataBase, 
									const u16* FW_RESTRICT pConstantDataIndices, 
						     		const int channelStart, const int channelCount );

extern "C" 
void GpAnimatorSetJointFlags( 		u8* FW_RESTRICT pJointFlags,
							  		const int numJoints,
							  		const u8 andFlags,
							  		const u8 orFlags );

extern "C" 
void GpAnimatorMulTransforms( 		FwTransform* pOutput, 
							  		const FwTransform* pInputA,
 							  		const FwTransform* pInputB, 		
 							  		int count );

extern "C" 
void GpAnimatorMirrorJoint(			const u32* FW_RESTRICT pMirrorSpecs,
									GpJoint* FW_RESTRICT pJoints,
									const int numJoints );
	
extern "C"
void GpAnimatorEvalRenorm(			GpJoint* FW_RESTRICT pJoint, 						   
									int jointCount );

//--------------------------------------------------------------------------------------------------
//	SPU_SPECIFIC ASM FUNCTIONS 
//--------------------------------------------------------------------------------------------------
// /!\ WARNING !
// - ASM function don't explicitely take a "count4" parameter anymore, and will silentely overrun 
//   by up to 3 iterations, unless specified differently.
// - This has been done this way to be compatible with quadword validation in C++ implementation
//   (as opposed to having C++ implementation overrun the same way)
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__

extern"C"
void GpAnimatorEvalMul( 			FwVector4* FW_RESTRICT pBase, FwVector4 mulFactor,
					    			u32 stride, u32 count);

extern "C" 
void GpAnimatorEvalInit( 			s32* FW_RESTRICT pSlotToLocal, const s16* FW_RESTRICT pLocalToSlot,
						 			FwVector4* FW_RESTRICT pEvaluationSlot, 
						 			const GpJoint* FW_RESTRICT pBindPose,
						 			u32 localChannelCount, u32 maxChannelCount,
						 			u32 jointCount, u32 userChannelStart, u32 userChannelCount);

extern "C" 
void GpAnimatorEvalHermite( 		FwVector4* FW_RESTRICT pEvaluationSlot,
						    		const float* FW_RESTRICT pTimeIntervalStart,
						    		const float* FW_RESTRICT pTimeIntervalEnd,
						    		const s16* FW_RESTRICT pLocalToSlot,
						    		const GpAnimatorUpdate::EvalKeyframeHermite* FW_RESTRICT pKeyframes,
						    		int numChannels,
						    		float time );	
 
extern "C" 
void GpAnimatorEvalPackedHermite( 	FwVector4* FW_RESTRICT pEvaluationSlot,
							    	const float* FW_RESTRICT pTimeIntervalStart,
							    	const float* FW_RESTRICT pTimeIntervalEnd,
							    	const s16* FW_RESTRICT pLocalToSlot,
							    	const u8* FW_RESTRICT pKeyframes,
									const u32* FW_RESTRICT pKeyframeBitOffsetArray,
									const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpecArray, 
									const u16* FW_RESTRICT pPackingSpecIndexArray,
									const FpAnimKeyframeTransform* FW_RESTRICT pTransformArray,
									const u16* FW_RESTRICT pTransformIndexArray,
							    	int numChannels,
							    	float time );	

extern "C" 
void GpAnimatorEvalHalfHermite( 	FwVector4* FW_RESTRICT pEvaluationSlot,
							    	const float* FW_RESTRICT pTimeIntervalStart,
							    	const float* FW_RESTRICT pTimeIntervalEnd,
							    	const s16* FW_RESTRICT pLocalToSlot,
							    	const GpAnimatorUpdate::EvalKeyframeHalfHermite* FW_RESTRICT pKeyframes,
							    	int numChannels,
							    	float time );	

extern "C" 
u32 GpAnimatorEvalGatherFloatUnaligned(	float* FW_RESTRICT pEvalTimeIntervalStart,
										float* FW_RESTRICT pEvalTimeIntervalEnd,
										u32* FW_RESTRICT pEvalKfDmaList,
										u32* FW_RESTRICT pEvalKfBitOffset,
										u32 initialKfBitOffset,
										const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockHeader,
										const float* FW_RESTRICT pTimeArray, 
										const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpecArray, 
										const u16* FW_RESTRICT pPackingSpecIndexArray,
										const u32 sizeKeyframe,
										const void* FW_RESTRICT pPPUKeyframeArray,
										const int channelCount,
										const float timeValue );

extern "C" 
u32 GpAnimatorEvalGatherByteUnaligned(	float* FW_RESTRICT pEvalTimeIntervalStart,
										float* FW_RESTRICT pEvalTimeIntervalEnd,
										u32* FW_RESTRICT pEvalKfDmaList,
										u32* FW_RESTRICT pEvalKfBitOffset,
										u32 initialKfBitOffset,
										const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockHeader,
										const u8* FW_RESTRICT pTimeArray, 
										const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpecArray, 
										const u16* FW_RESTRICT pPackingSpecIndexArray,
										const u32 sizeKeyframe,
										const void* FW_RESTRICT pPPUKeyframeArray,
										const int channelCount,
										const float timeValue,
										const float time0, 
										const float timeStep );

extern "C" 
u32 GpAnimatorEvalGatherShortUnaligned(	float* FW_RESTRICT pEvalTimeIntervalStart,
										float* FW_RESTRICT pEvalTimeIntervalEnd,
										u32* FW_RESTRICT pEvalKfDmaList,
										u32* FW_RESTRICT pEvalKfBitOffset,
										u32 initialKfBitOffset,
										const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockHeader,
										const u16* FW_RESTRICT pTimeArray, 
										const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpecArray, 
										const u16* FW_RESTRICT pPackingSpecIndexArray,
										const u32 sizeKeyframe,
										const void* FW_RESTRICT pPPUKeyframeArray,
										const int channelCount,
										const float timeValue,
										const float time0, 
										const float timeStep );

#endif //__SPU__

//--------------------------------------------------------------------------------------------------

#endif	// GP_ANIMATOR_UPDATE_H
