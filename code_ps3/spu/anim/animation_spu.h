/***************************************************************************************************
*
*	DESCRIPTION		Animation functions on SPUs.
*
*	NOTES
*
***************************************************************************************************/

#ifndef ANIMATION_SPU_
#define ANIMATION_SPU_

#ifndef __SPU__
#	error This header file is for use in PS3 SPU projects only.
#endif

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "anim/animation.h"
#include "anim/animator.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
class ChannelWalker;
class CAnimator;
class CAnimation;
class CAnimationHeader;
namespace NAnim
{
	struct Channel;
	struct Keyframe;
	struct KeyframeData;
}

//**************************************************************************************
//	Static class for animations on SPUs. We use a class, not a namespace, so we
//	can make the class a friend on CAnimation to access internals.
//**************************************************************************************
class SPUAnim
{
	public:
		//
		//	Statics for SPU versions of CAnimator's functions.
		//
		static void CAnimator_Update	(	CAnimator_DMASection *animator, uint32_t num_transforms, uint32_t transform_array_ea, uint32_t transform_array_stride,
											v128 *bind_pose_array, const int8_t * const char_bone_to_index_array, uint32_t largest_anim_size, uint32_t largest_header_size );

	public:
		//
		//	Statics for SPU versions of CAnimation's functions.
		//
		static void CalcLocomotion		( CAnimation *anim, const CAnimationHeader *anim_header, bool moved_past_end, bool moved_past_start );
		static void	CAnimation_Update	(	CAnimation *anim, const CAnimationHeader *anim_header, float *available_blend_weights,
											const int8_t * const char_bone_to_index_array, uint32_t anim_header_ea );

	public:
		//
		//	Statics for SPU versions of ChannelWalker's functions.
		//
		static v128 Evaluate			( const ChannelWalker *channel_walker, const NAnim::Channel *channel, const NAnim::KeyframeData &keyframe_data );
		static void	Seek				(	ChannelWalker *channel_walker, const NAnim::Channel *channel, float time, const NAnim::Keyframe *keyframes,
											int32_t keyframe_array_ea, float anim_duration, int32_t channel_idx );

	private:
		//
		//	Prevent creation/destruction.
		//
		SPUAnim()	NOT_IMPLEMENTED;
		~SPUAnim()	NOT_IMPLEMENTED;
};



#endif // !ANIMATION_SPU_

