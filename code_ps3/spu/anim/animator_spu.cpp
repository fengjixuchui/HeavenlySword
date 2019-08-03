/***************************************************************************************************
*
*	DESCRIPTION		Animator functions on SPUs.
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
#include "ntlib_spu/ntDma.h"

#include "core/exportstruct_keyframe.h"
#include "core/exportstruct_anim.h"

#include "animation_spu.h"
#include "animator_spu_structs.h"
#include "animation_spu_structs.h"

#include "anim/ChannelWalker.h"
#include "anim/AnimationTarget.h"

//**************************************************************************************
//	Temp buffer for holding the current keyframe for each channel.
//	This is filled in the channel walker Seek function so that we don't have to
//	DMA the keyframe in again.
//**************************************************************************************
NAnim::KeyframeData *CurrentKeyframes;

//**************************************************************************************
//	
//**************************************************************************************
KeyframeSet *ChannelData;

//**************************************************************************************
//	
//**************************************************************************************
void SPUAnim::CAnimator_Update( CAnimator_DMASection *animator, uint32_t num_transforms, uint32_t transform_array_ea, uint32_t transform_array_stride,
								v128 *bind_pose_array, const int8_t * const char_bone_to_index_array, uint32_t largest_anim_size, uint32_t largest_header_size )
{
	ntError( animator != NULL );
	ntError( char_bone_to_index_array != NULL );

	// Allocate some store space for current keyframes - one per channel.
	CurrentKeyframes = static_cast< NAnim::KeyframeData * >( Allocate( sizeof( NAnim::KeyframeData ) * MaxNumChannels ) );
	ntError( CurrentKeyframes != NULL );

	// 
	ChannelData = static_cast< KeyframeSet * >( Allocate( sizeof( KeyframeSet ) * 3 ) );
	ntError( ChannelData != NULL );

	for ( int32_t i=0;i<3;i++ )
	{
		ChannelData[ i ].m_ID = ntDMA::GetFreshID();
	}

	//
	//	Inital resetting/clearing of animator variables.
	//

	// Make sure our deltas are wiped clean..
	animator->m_obRootRotationDelta.SetIdentity();
	animator->m_obRootTranslationDelta.Clear();

	// Set up the details for relative movement - speculative a the moment
	animator->m_obRelativeMovementPos.Clear();
	animator->m_obRelativeMovementRot.SetIdentity();
	animator->m_bRelativeMovement = false;
	animator->m_fRelativeMovementWeight = 0.0f;

	// We initially say that we've not got a valid delta to apply..
	animator->m_iFlags &= ~ANIMATORF_HAS_DELTAS_AVAILABLE;

	// This shouldn't be called with no animations set on the animator - it's not worth it.
	ntError_p( animator->m_NumAnimations > 0, ("Why are you calling the spu animator update with no animations? Don't.") );

	//
	//	Allocate some scratch space for blend weights and setup the pointer to the tracking data.
	//
	float *animation_blend_weights = static_cast< float * >( Allocate( num_transforms * sizeof( float ) ) );
	ntError( animation_blend_weights != NULL );
	CVector *tracking_data = ResolveOffset< CVector >( animator->m_TrackingDataOffset, animator );

	// Setup some temporaries.
	float *work_blend_factor = static_cast< float * >( Allocate( num_transforms * sizeof( float ) ) );
	ntError( work_blend_factor != NULL );
	uint32_t *affecting_anim_count = static_cast< uint32_t * >( Allocate( num_transforms * sizeof( uint32_t ) ) );
	ntError( affecting_anim_count != NULL );
	AnimationTarget *targets = static_cast< AnimationTarget * >( Allocate( num_transforms * sizeof( AnimationTarget ) ) );
	ntError( targets != NULL );
	bool locomoting = false;

	for ( uint32_t i=0;i<num_transforms;i++ )
	{
		// Clear the tracking data and initialise each blend-weight.
		tracking_data[ i ].Clear();
		animation_blend_weights[ i ] = 1.0f;

		// Reset our temporaries.
		work_blend_factor[ i ] = 0.0f;
		affecting_anim_count[ i ] = 0;
	}

	// Track which anims need deleting.
	bool *delete_anim = static_cast< bool * >( Allocate( animator->m_NumAnimations * sizeof( bool ) ) );
	ntError( delete_anim != NULL );
	
	//
	//	Allocate a set of LS arrays for animations.
	//
	static const uint32_t NumAnimStoreSlots = 2;		// MUST be at least 2.
	static_assert_in_class( NumAnimStoreSlots >= 2, NumAnimStoreSlots_Must_be_at_least_2 );

	Animator::AnimStoreSlot *AnimStore = static_cast< Animator::AnimStoreSlot * >( Allocate( sizeof( Animator::AnimStoreSlot ) * NumAnimStoreSlots ) );
	ntError( AnimStore != NULL );

	const int32_t anim_dma_size =	largest_anim_size +						// Largest animation size.
									largest_header_size;					// Largest animation header size.

	for ( uint32_t i=0;i<NumAnimStoreSlots;i++ )
	{
		AnimStore[ i ].m_ID = ntDMA::GetFreshID();
		AnimStore[ i ].m_MemBase = Allocate( anim_dma_size );
		ntError( AnimStore[ i ].m_MemBase != NULL );
		ntError( (intptr_t)AnimStore[ i ].m_MemBase + anim_dma_size < 0x39600 );
	}

	ntError( fabsf( animator->m_obRootRotationDelta.Length() - 1.0f ) < 0.001f );
	ntError( animator->m_obRootTranslationDelta.Length() < 2000.0f );

	//
	//	DMA in the first anim.
	//
	ntDMA::Params dma_anim0, dma_header0, dma_target_index;
	dma_anim0.Init32( AnimStore[ 0 ].m_MemBase, animator->m_Animations[ 0 ], largest_anim_size, AnimStore[ 0 ].m_ID );
	dma_header0.Init32( static_cast< int8_t * >( AnimStore[ 0 ].m_MemBase ) + largest_anim_size, animator->m_Headers[ 0 ], largest_header_size, AnimStore[ 0 ].m_ID );

	ntError( (intptr_t)AnimStore[ 0 ].m_MemBase > 0 && (intptr_t)AnimStore[ 0 ].m_MemBase < 0x39600 );
	ntError( animator != NULL );
	ntError( animator->m_Animations[ 0 ] != 0 );
	ntError( animator->m_Headers[ 0 ] != 0 );

	ntDMA::DmaToSPU( dma_anim0 );
	ntDMA::DmaToSPU( dma_header0 );

	for ( uint32_t anim_idx=0;anim_idx<animator->m_NumAnimations;anim_idx++ )
	{
		//
		//	Work out current slot id for AnimStore indexing and dma id.
		//
		ntError( anim_idx >= 0 && anim_idx < animator->m_NumAnimations );
		ntDMA_ID curr_slot_id = anim_idx % NumAnimStoreSlots;
		ntError( curr_slot_id >= 0 && curr_slot_id < NumAnimStoreSlots );

		//
		//	DMA in next anim, if required.
		//
		if ( anim_idx < animator->m_NumAnimations - 1 )
		{
			//
			//	This isn't the last animation, so do the dma.
			//

			// Work out which AnimStore slot/dma id we should use - sync' on it
			// because we don't know if it's still in use.
			ntDMA_ID next_slot_id = ( anim_idx+1 ) % NumAnimStoreSlots;
			ntError( next_slot_id >= 0 && next_slot_id < NumAnimStoreSlots );
			ntDMA::StallForCompletion( AnimStore[ next_slot_id ].m_ID );

			ntError( animator->m_Animations[ anim_idx+1 ] != 0 );
			ntError( animator->m_Headers[ anim_idx+1 ] != 0 );

			// Setup the dma params and perform the dma into local store.
			ntDMA::Params dma_animn1, dma_headern1;
			dma_animn1.Init32( AnimStore[ next_slot_id ].m_MemBase, animator->m_Animations[ anim_idx+1 ], largest_anim_size, AnimStore[ next_slot_id ].m_ID );
			dma_headern1.Init32( static_cast< int8_t * >( AnimStore[ next_slot_id ].m_MemBase ) + largest_anim_size, animator->m_Headers[ anim_idx+1 ], largest_header_size, AnimStore[ next_slot_id ].m_ID );

			ntError( (intptr_t)AnimStore[ next_slot_id ].m_MemBase > 0 && (intptr_t)AnimStore[ next_slot_id ].m_MemBase < 0x39600 );
			ntError( animator != NULL );
			ntError( animator->m_Animations[ anim_idx+1 ] != 0 );
			ntError( animator->m_Headers[ anim_idx+1 ] != 0 );

			ntDMA::DmaToSPU( dma_animn1 );
			ntDMA::DmaToSPU( dma_headern1 );
		}

		// Need to stall on the DMA, hopefully will have finished by now.
		ntDMA::StallForCompletion( AnimStore[ curr_slot_id ].m_ID );

		// Patch in some pointers.
		CAnimation *curr_anim = reinterpret_cast< CAnimation * >( AnimStore[ curr_slot_id ].m_MemBase );
		CAnimationHeader *curr_anim_header = reinterpret_cast< CAnimationHeader * >( static_cast< int8_t * >( AnimStore[ curr_slot_id ].m_MemBase ) + largest_anim_size );
		ntError_p( curr_anim_header->m_TotalNumberOfChannels <= MaxNumChannels, ("This animation has %i channels - max num channels supported by spu anim system is %i.", curr_anim_header->m_TotalNumberOfChannels, MaxNumChannels) );

		//
		//	Update current anim.
		//

		ntError( anim_idx >= 0 && anim_idx < animator->m_NumAnimations );
		uint32_t curr_anim_header_ea = animator->m_Headers[ anim_idx ];

		CAnimation_Update( curr_anim, curr_anim_header, animation_blend_weights, char_bone_to_index_array, curr_anim_header_ea );
		delete_anim[ anim_idx ] = curr_anim->ShouldDelete();

		//
		//	DMA current anim out to EA.
		//
		ntDMA::Params dma_out;
		dma_out.Init32( AnimStore[ curr_slot_id ].m_MemBase, animator->m_Animations[ anim_idx ], curr_anim->m_ClassSizeInBytes, AnimStore[ curr_slot_id ].m_ID );
		ntDMA::DmaToPPU( dma_out );

		// Find the blend weights and target data calculated for this anim.
		const float *transform_blend_weights = ResolveOffset< const float >( curr_anim->m_TransformBlendWeightArrayOffset, curr_anim );
		const AnimationTarget *animation_targets = ResolveOffset< AnimationTarget >( curr_anim->m_AnimationTargetArrayOffset, curr_anim );

		//
		//	Update internal stuff.
		//
		for ( uint32_t transform_idx=0;transform_idx<num_transforms;transform_idx++ )
		{
			// Grab the target index.
			int32_t target_idx = curr_anim->GetHierarchyTargetIndex( transform_idx );

			if ( target_idx < 0 )
			{
				continue;
			}

			// Pull the requested blend factor for the transform. Note that the last animation
			// gets all of the remaining blend factor.
			float blend_factor;
			if ( anim_idx == ( animator->m_NumAnimations - 1 ) )
			{
				if ( ( work_blend_factor[ transform_idx ] < EPSILON ) && 
					 ( curr_anim->GetBlendWeight() < EPSILON ) )
				{
					blend_factor = 0.0f;
				}
				else
				{
					blend_factor = 1.0f - work_blend_factor[ transform_idx ];

					if ( transform_blend_weights[ target_idx ] == 0.0f )
					{
						blend_factor = 0.0f;
					}
				}
			}
			else
			{
				blend_factor = transform_blend_weights[ target_idx ];
			}

			ntError( blend_factor >= 0.0f && blend_factor <= 1.0f );

			if ( blend_factor > 0.0f )
			{
				// Retrieve tracking data
				CVector tracking = animation_targets[ target_idx ].m_obTracking;
				CQuat rotation;
				CPoint translation;

				// Speculative stuff - can i do some cunning locomotion variations
				if ( ( transform_idx == ROOT_TRANSFORM ) && ( curr_anim->GetFlags() & ANIMF_RELATIVE_MOVEMENT ) )
				{
					// AARRRGHGHHHHHH. Giles you git! Can't do any of this on SPUs as the call to GetRootWorldOrientation
					// needs to evaluate a world-matrix on an arbitrary transform that probably isn't on the same
					// hierarchy. Buggerations!
					//
					// Not doing relative movement anims on SPUs for now.

					ntError_p( false, ("Shouldn't ever be here - this path not yet supported on SPUs.") );


// 					// Add on the weight of this animation
// 					animator->m_fRelativeMovementWeight += curr_anim->GetBlendWeight();
// 					ntError( animator->m_fRelativeMovementWeight >= 0.0f && animator->m_fRelativeMovementWeight <= 1.0f );
// 
// 					// If this is our first relative movement animation - we take the position directly
// 					if ( !animator->m_bRelativeMovement )
// 					{
// 						animator->m_obRelativeMovementRot = curr_anim->GetRootWorldOrientation();
// 						animator->m_obRelativeMovementPos = curr_anim->GetRootWorldPosition();
// 					}
// 					else
// 					{
// 						animator->m_obRelativeMovementRot = CQuat::Slerp( animator->m_obRelativeMovementRot, curr_anim->GetRootWorldOrientation(), ( curr_anim->GetBlendWeight() / animator->m_fRelativeMovementWeight ) );
// 						animator->m_obRelativeMovementPos = CPoint::Lerp( animator->m_obRelativeMovementPos, curr_anim->GetRootWorldPosition(), ( curr_anim->GetBlendWeight() / animator->m_fRelativeMovementWeight ) );
// 					}
// 
// 					ntError( fabsf( animator->m_obRelativeMovementRot.Length() - 1.0f ) < 0.001f );
// 
// 					// Make sure flags are set
// 					animator->m_bRelativeMovement = true;

					rotation.SetIdentity();
					translation.Clear();
				}
				else if ( ( transform_idx == ROOT_TRANSFORM ) && ( curr_anim->GetFlags() & ANIMF_LOCOMOTING ) )
				{
					// If we're locomoting and we're on the root transform, our rotations & translations come
					// from deltas stored on the animation objects.
					rotation = curr_anim->GetRootRotationDelta();

					// HACK HACK
//					ntError( fabsf( rotation.Length() - 1.0f ) < 0.001f );



					if( !(fabsf( rotation.Length() - 1.0f ) < 0.001f) )
					{
						rotation.Normalise();
					}







					translation = curr_anim->GetRootTranslationDelta();
					ntError( translation.LengthSquared() < 1000000.0f );
					locomoting = true;
				}
				else
				{
					// Normal use is to grab the rotation & translation from the animation target array.
					rotation = animation_targets[ target_idx ].m_obRotation;

					// HACK HACK
//					ntError( fabsf( rotation.Length() - 1.0f ) < 0.001f );





					if( !(fabsf( rotation.Length() - 1.0f ) < 0.001f) )
					{
						rotation.Normalise();
					}







					translation	= animation_targets[ target_idx ].m_obTranslation;
					ntError( translation.LengthSquared() < 1000000.0f );
				}

				// If we haven't updated this transform before then our working space won't have been
				// initialised so we need to copy the data over instead of (s)lerping.
				if ( affecting_anim_count[ transform_idx ] == 0 )
				{
					// HACK HACK
//					ntError( fabsf( rotation.Length() - 1.0f ) < 0.001f );
/*
					if( !(fabsf( rotation.Length() - 1.0f ) < 0.001f) )
					{
						rotation.Normalise();
					}
*/
					targets[ transform_idx ].m_obRotation = rotation;
					targets[ transform_idx ].m_obTranslation = translation;
					targets[ transform_idx ].m_obTracking = tracking;
				}
				else
				{
					// We perform a local blend between the working pair, and the current blend factor.
					// Effectively this is the ratio between the current blend factor and the summed
					// work blend factor. If this is all too baffling, ask Simon. He fixed it. Not me.
					float local_blend = blend_factor / ( blend_factor + work_blend_factor[ transform_idx ] );

					// Slerp version - slow.
					ntError( fabsf( targets[ transform_idx ].m_obRotation.Length() - 1.0f ) < 0.001f );
					// HACK HACK
					ntError( fabsf( rotation.Length() - 1.0f ) < 0.001f );
/*
					if( !(fabsf( rotation.Length() - 1.0f ) < 0.001f) )
					{
						rotation.Normalise();
					}
*/
					targets[ transform_idx ].m_obRotation = CQuat::Slerp( targets[ transform_idx ].m_obRotation, rotation, local_blend );
					ntError( fabsf( targets[ transform_idx ].m_obRotation.Length() - 1.0f ) < 0.001f );

					targets[ transform_idx ].m_obTranslation = CPoint::Lerp( targets[ transform_idx ].m_obTranslation, translation, local_blend );
					targets[ transform_idx ].m_obTracking = CVector::Lerp( targets[ transform_idx ].m_obTracking, tracking, local_blend );
				}

				// Increase the number of affected transforms
				affecting_anim_count[ transform_idx ]++;
			}

			work_blend_factor[ transform_idx ] += blend_factor;
			if ( work_blend_factor[ transform_idx ] > 1.0f )
			{
				work_blend_factor[ transform_idx ] = 1.0f;
			}
			ntError( work_blend_factor[ transform_idx ] >= 0.0f && work_blend_factor[ transform_idx ] <= 1.0f );
		}
	}

	// Free up our anim store ids.
	for ( uint32_t i=0;i<NumAnimStoreSlots;i++ )
	{
		ntDMA::FreeID( AnimStore[ i ].m_ID );
	}

	//
	//	We need to setup a ring buffer here so that we give the DMAs time to complete and also
	//	so the data doesn't go out of scope before the DMA has happened.
	//
	static const uint32_t NumLocalMatrices = 8;
	struct MatrixRingBuffer
	{
		CMatrix		m_Matrix;
		ntDMA_ID	m_ID;
	};

	MatrixRingBuffer *local_matrix_ring_buffer = static_cast< MatrixRingBuffer * >( Allocate( NumLocalMatrices * sizeof( MatrixRingBuffer ) ) );
	ntError( local_matrix_ring_buffer != NULL );
	ntError( (intptr_t)local_matrix_ring_buffer + (int32_t)( NumLocalMatrices * sizeof( MatrixRingBuffer ) ) < 0x36900 );
	uint32_t local_matrix_idx = 0;

	for ( uint32_t i=0;i<NumLocalMatrices;i++ )
	{
		local_matrix_ring_buffer[ i ].m_ID = ntDMA::GetFreshID();
	}

	for ( uint32_t transform_idx=0;transform_idx<num_transforms;transform_idx++ )
	{
		// If we've done anything that needs to affect this transform, do the work.
		if ( affecting_anim_count[ transform_idx ] > 0 )
		{
			// Ensure tracking data is written out..
			tracking_data[ transform_idx ] = targets[ transform_idx ].m_obTracking;
			
			if ( ( transform_idx == ROOT_TRANSFORM ) && locomoting )
			{
				// Locomoting anims don't update the root in the hierarchy. They only update our deltas.
				animator->m_obRootRotationDelta		= targets[ transform_idx ].m_obRotation;
				animator->m_obRootTranslationDelta	= targets[ transform_idx ].m_obTranslation;	// We don't use the root offset.. 
				animator->m_iFlags |= ANIMATORF_HAS_DELTAS_AVAILABLE;
			}
			else if ( ( transform_idx == ROOT_TRANSFORM ) && animator->m_bRelativeMovement )
			{
			}
			else
			{
				//
				//	Set the local space transform to be the matrix formed by the quaternion & translation.
				//

				// Add in the bind-pose bone-offset.
//				targets[ transform_idx ].m_obTranslation += hierarchy->GetBindPoseArray()[ transform_idx ].m_obBoneOffset;
				targets[ transform_idx ].m_obTranslation += CPoint( bind_pose_array[ transform_idx ] );

				// The two lines below are equivalent to: ( m_pobHierarchy->GetRootTransform() + transform_idx )->SetLocalMatrix( .... );
				CMatrix local_matrix( targets[ transform_idx ].m_obRotation, targets[ transform_idx ].m_obTranslation );
//				hierarchy->GetTransform( transform_idx )->SetLocalMatrix( local_matrix );

				// Make sure we're not overwriting anything still in use.
				ntDMA::StallForCompletion( local_matrix_ring_buffer[ local_matrix_idx ].m_ID );

				// Setup the local matrix in the buffer.
				local_matrix_ring_buffer[ local_matrix_idx ].m_Matrix = local_matrix;

				// DMA this matrix out to EA.
				ntDMA::Params local_matrix_dma;
				local_matrix_dma.Init32( &( local_matrix_ring_buffer[ local_matrix_idx ].m_Matrix ), transform_array_ea + transform_array_stride * transform_idx,
										 sizeof( CMatrix ), local_matrix_ring_buffer[ local_matrix_idx ].m_ID );
				ntDMA::DmaToPPU( local_matrix_dma );

				// Increment the ring buffer index.
				local_matrix_idx = ( local_matrix_idx + 1 ) % NumLocalMatrices;
			}
		}
	}

	// Free up our allocated dma ids.
	for ( uint32_t i=0;i<NumLocalMatrices;i++ )
	{
		ntDMA::FreeID( local_matrix_ring_buffer[ i ].m_ID );
	}

	// Now we've sorted everything out, we could do with deleting the animations that have expired..
	// We no longer actually delete the animations here, we just move them to the end of the array,
	// the animations are actually released in PreRenderUpdate.
	for ( uint32_t i=0;i<animator->m_NumAnimations; )
	{
		if ( delete_anim[ i ] )
		{
			ntError( animator->m_Animations[ i ] != NULL );
			ntError( animator->m_Headers[ i ] != NULL );
			for ( uint32_t j=i;j<animator->m_NumAnimations-1;j++ )
			{
				// Swap j and j+1.
				ntError( animator->m_Animations[ j+1 ] != NULL );
				ntError( animator->m_Headers[ j+1 ] != NULL );

				uint32_t anim_temp = animator->m_Animations[ j ];
				animator->m_Animations[ j ] = animator->m_Animations[ j+1 ];
				animator->m_Animations[ j+1 ] = anim_temp;

				uint32_t header_temp = animator->m_Headers[ j ];
				animator->m_Headers[ j ] = animator->m_Headers[ j+1 ];
				animator->m_Headers[ j+1 ] = header_temp;
			}

			animator->m_NumAnimations--;
		}
		else
		{
			i++;
		}
	}

	// Free up the ChannelData DMA ids.
	for ( int32_t i=0;i<3;i++ )
	{
		ntDMA::FreeID( ChannelData[ i ].m_ID );
	}
}

