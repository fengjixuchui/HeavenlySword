/***************************************************************************************************
*
*	DESCRIPTION		Animation functions on SPUs.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <basetypes_spu.h>
#include <debug_spu.h>

#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/ntDmaList.h"

#include "animation_spu.h"
#include "animation_spu_structs.h"

#include "anim/ChannelWalker.h"
#include "anim/AnimationTarget.h"

#include "core/exportstruct_keyframe.h"
#include "core/exportstruct_anim.h"

//**************************************************************************************
//	
//**************************************************************************************
void SPUAnim::CalcLocomotion( CAnimation *anim, const CAnimationHeader *anim_header, bool moved_past_end, bool moved_past_start )
{
	const int iRootTransformIndex = ResolveOffset< short >( anim->m_HierarchyToAnimationLookupOffset, anim )[ ROOT_TRANSFORM ];

	const AnimationTarget *target_array = ResolveOffset< AnimationTarget >( anim->m_AnimationTargetArrayOffset, anim );
	const CQuat &obNewRotation = target_array[ iRootTransformIndex ].m_obRotation;
	const CPoint &obNewTranslation = target_array[ iRootTransformIndex ].m_obTranslation;

	ntAssert_p( fabsf( obNewRotation.Length() - 1.0f ) < 0.001f, ("length of obNewRotation=%3.4f", obNewRotation.Length()) );

	if( !anim->m_bInitialUpdate )	// m_obLastRootTranslation/m_obLastRootRotation not available on 1st update
	{
		ntError_p( fabsf( anim->m_obLastRootRotation.Length() - 1.0f ) < 0.001f, ("length of anim->m_obLastRootRotation=%3.4f", anim->m_obLastRootRotation.Length()) );
		ntError_p( fabsf( anim_header->m_RootEndRotation.Length() - 1.0f ) < 0.001f, ("length of anim->m_obLastRootRotation=%3.4f", anim_header->m_RootEndRotation.Length()) );

		// If we've wrapped around the start or end of the animation, we need to ensure the previous positions
		// and rotations are corrected so that we can still accurately compute deltas.
		if( moved_past_end )
		{
			anim->m_obLastRootTranslation -= anim_header->m_RootEndTranslation;
			anim->m_obLastRootRotation = ( ~anim_header->m_RootEndRotation ) * anim->m_obLastRootRotation;
			anim->m_obLastRootRotation.Normalise();
		}
		else if( moved_past_start )
		{
			anim->m_obLastRootTranslation += anim_header->m_RootEndTranslation;
			anim->m_obLastRootRotation = anim_header->m_RootEndRotation * anim->m_obLastRootRotation;
			anim->m_obLastRootRotation.Normalise();
		}

		ntError_p( fabsf( anim->m_obLastRootRotation.Length() - 1.0f ) < 0.001f, ("length of anim->m_obLastRootRotation=%3.4f", anim->m_obLastRootRotation.Length()) );

		// Work out a delta and save it.
//		anim->m_obRootRotationDelta = ( ( ~anim->m_obLastRootRotation ) * obNewRotation );
		// Expanded version of line above.
		CQuat inv_last_root_rotation = ~( anim->m_obLastRootRotation );
		CQuat new_root_rotation_delta = inv_last_root_rotation * obNewRotation;
		ntError_p( fabsf( new_root_rotation_delta.Length() - 1.0f ) < 0.001f, ("length of new_root_rotation_delta=%3.4f", new_root_rotation_delta.Length()) );
		anim->m_obRootRotationDelta = new_root_rotation_delta;
		ntError_p( fabsf( anim->m_obRootRotationDelta.Length() - 1.0f ) < 0.001f, ("length of m_obRootRotationDelta=%3.4f", anim->m_obRootRotationDelta.Length()) );

		CDirection obAnimDelta = obNewTranslation ^ anim->m_obLastRootTranslation;

/*
		// No support for CMatrix on SPUs. Need a CQuat implementation of this.
		CMatrix	obPreviousRotationInv( ~anim->m_obLastRootRotation, CVecMath::GetZeroPoint() );
		anim->m_obRootTranslationDelta = CPoint( obAnimDelta * obPreviousRotationInv );
*/
//		anim->m_obRootTranslationDelta = CPoint( ( ~anim->m_obLastRootRotation ).Rotate( obAnimDelta ) );
		// Expanded version of line above.
		CDirection rot_dir = inv_last_root_rotation.Rotate( obAnimDelta );
		CPoint new_root_translation_delta = CPoint( rot_dir );
		anim->m_obRootTranslationDelta = new_root_translation_delta;
	}

	// Rotations must be unit-length quaternions.
	ntError_p( fabsf( anim->m_obRootRotationDelta.Length() - 1.0f ) < 0.001f, ("length of m_obRootRotationDelta=%3.4f", anim->m_obRootRotationDelta.Length()) );

	// store rotation/translation for next delta
	anim->m_obLastRootRotation = obNewRotation;
	anim->m_obLastRootTranslation = obNewTranslation;
}

//**************************************************************************************
//	
//**************************************************************************************
void SPUAnim::CAnimation_Update(	CAnimation *anim, const CAnimationHeader *anim_header, float *available_blend_weights,
									const int8_t * const char_bone_to_index_array, uint32_t anim_header_ea )
{
	ntError( anim != NULL );
	ntError( anim_header != NULL );
	ntError( available_blend_weights != NULL );
	ntError( char_bone_to_index_array != NULL );

	bool bMovedPastStart		= false;
	bool bMovedPastEnd			= false;
	bool bHaveActiveTransform	= false;

	// ---------------------------------------------------------------------------------------------

	// We have to be linked in to an animator for an update to work..
	ntAssert( available_blend_weights != NULL );

	anim->m_obRootRotationDelta.SetIdentity();
	anim->m_obRootTranslationDelta.Clear();

	// Now we've updated the time, we need to ensure it lies within the animation...
	if ( anim->m_fTime < 0.0f )
	{
		if ( anim->GetFlags() & ANIMF_LOOPING )
		{
			bMovedPastStart = true;
			anim->m_fTime += anim_header->m_Duration;
		}
		else
		{
			anim->m_fTime = 0.0f;
			if ( !( anim->GetFlags() & ANIMF_INHIBIT_AUTO_DESTRUCT ) )
			{
				anim->m_ToBeDeleted = true;
			}
		}
	}
	else
	{
		if ( anim->m_fTime > anim_header->m_Duration )
		{
			if ( anim->GetFlags() & ANIMF_LOOPING )
			{
				bMovedPastEnd	= true;
				anim->m_fTime -= anim_header->m_Duration;

				// It may be that the animation is shorter than an update or that the framerate is
				// unstable, in this case we completely reset the time to retain robustness - GH
				if ( ( anim->m_fTime > anim_header->m_Duration ) || ( anim->m_fTime < 0.0f ) )
					anim->m_fTime = 0.0f;
			}
			else
			{
				anim->m_fTime = anim_header->m_Duration;
				if ( !( anim->GetFlags() & ANIMF_INHIBIT_AUTO_DESTRUCT ) )
				{
					anim->m_ToBeDeleted = true;
				}
			}
		}
	}

	// ---------------------------------------------------------------------------------------------

	// Update the channel times. You might be wondering why I'm updating these regardless of the locked state
	// or blend weight (either animation or the per-bone weight). Well, if we fail to update them all the time
	// it would be possible to incur a large penalty when blend weights transition from zero to non-zero, or 
	// when a transform moves from a locked to unlocked state. Imagine a 10 second long animation with a 
	// channel containing 10*60 keyframes. If you locked a transform at 0.0f, then unlocked it at 9.99 seconds
	// we'd have identified positive movement throughout the animation. At this point we'd end up iterating
	// forwards through 10*60 keyframes. Not good. 
	// 
	// Anyway, we'll have to look at this particular area of code once the systems are complete. It 
	// might be that we need to change the system used to advance through keyframes to be something
	// more optimal.

	ntError( anim != NULL );
	ntError( anim_header != NULL );
	const CAnimationTransform *	pobTransform					= ResolveOffset< CAnimationTransform >( anim_header->m_AnimationTransformArrayOffset, anim_header );
	const int16_t *				animation_to_hierarchy_lookup	= ResolveOffset< int16_t >( anim_header->m_AnimationToHierarchyLookupOffset, anim_header );
	float *						transform_blend_weight_array	= ResolveOffset< float >( anim->m_TransformBlendWeightArrayOffset, anim );
	ChannelWalker *				channel_walker_array			= ResolveOffset< ChannelWalker >( anim->m_ChannelWalkerArrayOffset, anim );

	ntError( pobTransform != NULL );
	ntError( animation_to_hierarchy_lookup != NULL );
	ntError( transform_blend_weight_array != NULL );
	ntError( channel_walker_array != NULL );

	ntError( (intptr_t)pobTransform < 0x39600 );
	ntError( (intptr_t)animation_to_hierarchy_lookup < 0x39600 );
	ntError( (intptr_t)transform_blend_weight_array < 0x39600 );
	ntError( (intptr_t)channel_walker_array < 0x39600 );

	// Keep track of a unique index per channel - we use this to store (and later access) the cached
	// keyframe that is the correct keyframe for each channel (found in the Seek(....) function).
	int32_t cumulative_channel_index = 0;

	for ( int32_t iTransformLoop = 0; iTransformLoop < anim_header->m_NumAnimationTransforms; iTransformLoop++, pobTransform++ )
	{
		// Preload our channel keyframes... We preload four keyframes; previous, current and the next two keyframes.
		// We don't care if the previous or any of the next two keyframes are invalid - we will never be seeking
		// into that data so it doesn't matter. This allows us to effectively avoid stalling on dma transfers in a 
		// linear seek through the keyframes - at the moment, general seeks through the keyframes are *bad*.
		const NAnim::Channel *pobChannel = ResolveOffset< NAnim::Channel >( pobTransform->m_AnimationChannelOffset, pobTransform );

		ntError( (intptr_t)pobChannel < 0x39600 );
		ntError( pobTransform->m_NumChannels >= 0 && pobTransform->m_NumChannels <= 3 );

		int32_t transform_ea = anim_header_ea + anim_header->m_AnimationTransformArrayOffset + iTransformLoop*sizeof( CAnimationTransform );
		int32_t channel_ea = transform_ea + pobTransform->m_AnimationChannelOffset;

		for ( int32_t iChannelLoop = 0; iChannelLoop < pobTransform->m_NumChannels; iChannelLoop++, pobChannel++ )
		{
 			ChannelWalker *pobWalker = channel_walker_array + pobChannel->m_WalkerArrayIndex;

			ntError( (intptr_t)pobWalker < 0x39600 );
			ntError( channel_ea == transform_ea + pobTransform->m_AnimationChannelOffset + (int32_t)sizeof( NAnim::Channel )*iChannelLoop );

			int32_t keyframe_ea = channel_ea + pobChannel->m_KeyframeArrayOffset;
			int32_t prev_keyframe_ea = keyframe_ea + ( pobWalker->m_iKeyframeIndex - 1 ) * (int32_t)sizeof( NAnim::Keyframe );

			ntError( iChannelLoop >= 0 && iChannelLoop < 3 );

			ntDMA::Params params;
			ntError( ChannelData[ iChannelLoop ].m_ID >= 8 && ChannelData[ iChannelLoop ].m_ID < 32 );
			params.Init32( &( ChannelData[ iChannelLoop ].m_Keyframes[ 0 ] ), prev_keyframe_ea, sizeof( NAnim::Keyframe ) * KeyframeSet::NumKeyframes, ChannelData[ iChannelLoop ].m_ID );
			ntDMA::DmaToSPU( params );

			ntError( (intptr_t)( ChannelData + iChannelLoop ) == (intptr_t)params.m_LSAddr );
			ntError( (intptr_t)( ChannelData + iChannelLoop + 1 ) > (intptr_t)params.m_LSAddr + (intptr_t)params.m_TransferSize );

			channel_ea += sizeof( NAnim::Channel );
		}

        // Compute our blend weight for this transform, based on what's been used, what our requested blend weight is,
		// and our blend-in/out times. We read/write to the main animation blend weight array at this stage..
		int32_t	iHierarchyIndex = animation_to_hierarchy_lookup[ iTransformLoop ];

		if ( anim->GetFlags() & ANIMF_CHARACTER_ANIM )
		{
			// For characters, the animation->hierarchy lookup table contains bone indices, so first
			// convert it to a real hierarchy index by looking into the hierarchies bone->index array.
			iHierarchyIndex = char_bone_to_index_array[ iHierarchyIndex ];

			// If the transform isn't valid for this hierarchy, we make sure it's got no effective 
			// influence and then we stop processing it (by using 'continue' to go to the next loop iteration
			if ( iHierarchyIndex < 0 )
			{
				transform_blend_weight_array[ iTransformLoop ] = 0.0f;
				cumulative_channel_index += pobTransform->m_NumChannels;

				continue;
			}
		}

		// We need to read the remaining blend weight from the parent CAnimator object. This will return 0.0f for
		// transforms that are locked (either local or world space)
		float fRemainingBlendWeight = available_blend_weights[ iHierarchyIndex ];

		// If we have weight available, then update our weights. 
		if ( fRemainingBlendWeight > 0.0f )
		{
			// Compute a used blend				
			if ( anim->m_fBlendWeight <= fRemainingBlendWeight )
			{
				fRemainingBlendWeight -= anim->m_fBlendWeight;
				transform_blend_weight_array[ iTransformLoop ] = anim->m_fBlendWeight;
			}
			else
			{
				transform_blend_weight_array[ iTransformLoop ] = fRemainingBlendWeight;
				fRemainingBlendWeight = 0;
			}

			available_blend_weights[ iHierarchyIndex ] = fRemainingBlendWeight;
		}
		else
		{
			available_blend_weights[ iHierarchyIndex ] = 0.0f;
			transform_blend_weight_array[ iTransformLoop ] = 0.0f;
		}

		// Make sure we track whether we have any active transforms. If we need to hold things for one frame
		// with a zero blend weight, we'll need an array of values that increase each frame that weights are zero.
		// We'd say a transform was active until it had processed a single zero blend weight. This will incur 
		// penalties for all transforms though, due to the maintenance of that field.
		if ( transform_blend_weight_array[ iTransformLoop ] > 0.0f )
		{
			bHaveActiveTransform = true;
		}

		// Now deal with our channels.. 
		pobChannel = ResolveOffset< NAnim::Channel >( pobTransform->m_AnimationChannelOffset, pobTransform );

		ntError( (intptr_t)pobChannel < 0x39600 );

		channel_ea = transform_ea + pobTransform->m_AnimationChannelOffset;

		for ( int32_t iChannelLoop = 0; iChannelLoop < pobTransform->m_NumChannels; iChannelLoop++, pobChannel++ )
		{
 			ChannelWalker *pobWalker = channel_walker_array + pobChannel->m_WalkerArrayIndex;

			ntError( (intptr_t)pobWalker < 0x39600 );

			int32_t keyframe_ea = channel_ea + pobChannel->m_KeyframeArrayOffset;

			ntError( iChannelLoop >= 0 && iChannelLoop < 3 );
			ntDMA::StallForCompletion( ChannelData[ iChannelLoop ].m_ID );

			SPUAnim::Seek( pobWalker, pobChannel, anim->m_fTime, ChannelData[ iChannelLoop ].m_Keyframes, keyframe_ea, anim_header->m_Duration, cumulative_channel_index );

			cumulative_channel_index++;

			channel_ea += sizeof( NAnim::Channel );
		}
	}

	// We mark animations that have had no influence at all on the final hierarchy as to be deleted
	// unless we're flagged as inhibiting the auto-destruct mechanism..
	if ( ( !bHaveActiveTransform ) && ( !( anim->GetFlags() & ANIMF_INHIBIT_AUTO_DESTRUCT ) ) )
	{
		anim->m_ToBeDeleted = true;	
	}

	// ---------------------------------------------------------------------------------------------

	// If the used blend weight for this entire animation is at 0.0f, and hence we have no active
	// transforms, then we might as well go home now as none of the stuff will be used.. ***Ah - 
	// except if this is a locomoting animation we need to keep track of what the root is doing. GH
	if ( ( !bHaveActiveTransform ) && ( !( anim->GetFlags() & ANIMF_LOCOMOTING ) ) )
	{
		return;
	}

	// ---------------------------------------------------------------------------------------------
	// Now it's time to go through all the channels performing processing based on the current 
	// position within the animation. 

	pobTransform = ResolveOffset< CAnimationTransform >( anim_header->m_AnimationTransformArrayOffset, anim_header );

	// Get the root transform index - root transform index is always at offset 0 in the hierarchy to animation lookup table.
	bool bChangedRootTransform = false;
	const int32_t iRootTransformIndex = ResolveOffset< int16_t >( anim->m_HierarchyToAnimationLookupOffset, anim )[ ROOT_TRANSFORM ];

	// Check that our assumptions are correct for locomoting animations
	//ntAssert_p( ( iRootTransformIndex == 0 ) || ( !( anim->GetFlags() & ANIMF_LOCOMOTING ) ), ("iRootTransformIndex=%i, anim->GetFlags()=%i",iRootTransformIndex, anim->GetFlags()) );

	// Calculate a pointer to our target array.
	AnimationTarget *animation_target_array = ResolveOffset< AnimationTarget >( anim->m_AnimationTargetArrayOffset, anim );

	// Reset the channel index before we loop through them again.
	cumulative_channel_index = 0;

	// So, first we need to go through all the CAnimationTransform objects
	for ( int32_t iTransformLoop = 0; iTransformLoop < anim_header->m_NumAnimationTransforms; iTransformLoop++, pobTransform++ )
	{
		// OK - I have made some slight changes here.  This loop used to not be called if the blend weight
		// of the animation was zero - but we always need to update at least the root on a locomoting
		// animation - this change allows that update to happen as cheaply as possible.
		if ( iTransformLoop == iRootTransformIndex )
		{
			// Note that we have updated the root.
			bChangedRootTransform = true;
		}
		else
		{
			// If we were marked as not having active transforms then we should break out of here
			if ( !bHaveActiveTransform )
				break;

			// If we don't need the transform (based on locked states, and bone-specific blend weight) then skip it.. 
			if ( transform_blend_weight_array[ iTransformLoop ] <= 0.0f )
			{
				cumulative_channel_index += pobTransform->m_NumChannels;
				continue;
			}
		}
		
		// For each CAnimationTransform object we need to go through all the channels
		NAnim::Channel *pobChannel = ResolveOffset< NAnim::Channel >( pobTransform->m_AnimationChannelOffset, pobTransform );

		for ( int iChannelLoop = 0; iChannelLoop < pobTransform->m_NumChannels; iChannelLoop++, pobChannel++ )
		{
			const ChannelWalker *pobWalker = channel_walker_array + pobChannel->m_WalkerArrayIndex;

			// Get a CVector pointer to the start of our AnimationTarget.
			CVector *vec_target = reinterpret_cast< CVector * >( animation_target_array + pobTransform->m_TransformIndex );

			// Offset the CVector pointer by the correct amount.
			// NOTE: Assumes that the keyframe class enum corresponds to the position of the different
			//       targets within the AnimationTarget structure - hence the static_asserts at the top
			//       of exportstruct_keyframe.h.
			vec_target += pobChannel->m_Class;

			// Update the animation target.
			vec_target->Quadword() = SPUAnim::Evaluate( pobWalker, pobChannel, CurrentKeyframes[ cumulative_channel_index ] );

#			ifdef _DEBUG
				if ( pobChannel->m_Class == KEYFRAME_CLASS_ROTATION )
				{
					ntError( fabsf( vec_target->Length() - 1.0f ) < 0.001f );
				}
#			endif

			cumulative_channel_index++;
		}
	}

	// We've finished updating all the AnimationTargets. So, for animations that are locomoting, we
	// need to perform some work to ensure we have up-to-date rotational & translational deltas. Note
	// that we don't update things if we've not had a locomoting root.. deltas stay as zero.
	// NOTE: bChangedRootTransform is a kludge. We need to be more careful with blendweights. BenC.
	// Maybe the correct thing to do is to _not_ early-out if a root transform has zero blendweight.
	if ( bChangedRootTransform )
	{
		SPUAnim::CalcLocomotion( anim, anim_header, bMovedPastEnd, bMovedPastStart );
	}

	// Make sure the intitial update flag is cleared
	anim->m_bInitialUpdate = false;
}






