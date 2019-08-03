//------------------------------------------------------------------------------------------
//!
//!	\file AnimBuilder.cpp
//!	
//------------------------------------------------------------------------------------------

#include "anim/AnimBuilder.h"

#include "anim/hierarchy.h"
#include "anim/animation.h"
#include "core/exportstruct_anim.h"
#include "core/exportstruct_keyframe.h"

// Initialise our animation duration value
const float AnimBuilder::s_fStaticAnimDuration = 1.0f;

//------------------------------------------------------------------------------------------
//!
//!	CreateAnimKeyframes
//!	CReate all the keyframes for an animation from a hierarchy
//!
//------------------------------------------------------------------------------------------
void AnimBuilder::CreateAnimKeyframes(	const CHierarchy&		obHierarchy, 
										short*					psAnimationToHierarchyLookup,
										CAnimationTransform*	pobAnimationTransformArray,
										int						iNumberOfAnimationTransforms,
										const CAnimationHeader * )
{
	// Loop through all our hierarchy transforms and save the pose to keyframes
	for ( short iTransform = 0; iTransform < iNumberOfAnimationTransforms; ++iTransform )
	{
		// Set up the basic details
		pobAnimationTransformArray[iTransform].m_sTransformIndex = iTransform;
		pobAnimationTransformArray[iTransform].m_sNumberOfChannels = 2;

		// Create the two animation channels
		NAnim::Channel *animation_transform_array = FW_NEW NAnim::Channel[ 2 ];
		pobAnimationTransformArray[ iTransform ].m_pobAnimationChannelArrayOffset = int32_t( reinterpret_cast< intptr_t >( animation_transform_array ) - reinterpret_cast< intptr_t >( &pobAnimationTransformArray[ iTransform ] ) );

		// Create a direct pointer to our translation channel
		NAnim::Channel &obTranslationChannel = animation_transform_array[ 0 ];

		// Fill out the translation channel details
		obTranslationChannel.m_eKeyframeClass = KEYFRAME_CLASS_TRANSLATION;
		obTranslationChannel.m_eKeyframeType = KEYFRAME_TYPE_CURVE_TRANSLATION;
		obTranslationChannel.m_iNumberOfKeyframes = 2;
		obTranslationChannel.m_iWalkerArrayIndex = ( iTransform * 2 );

		// Two keyframes - one at the start time, one at the duration time
		float *keyframe_times = FW_NEW float[ 2 ];
		keyframe_times[ 0 ] = 0.0f;
		keyframe_times[ 1 ] = s_fStaticAnimDuration;
		obTranslationChannel.m_KeyframeTimesOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_times ) - reinterpret_cast< intptr_t >( &obTranslationChannel ) );

		// Both the keyframes are constant
		char *keyframe_flags = FW_NEW char[ 2 ];
		keyframe_flags[ 0 ] = KEYF_IS_CONSTANT;
		keyframe_flags[ 1 ] = KEYF_IS_CONSTANT;
		obTranslationChannel.m_KeyframeFlagsOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_flags ) - reinterpret_cast< intptr_t >( &obTranslationChannel ) );

		// Both the keyframes describe the same translation
		NAnim::Keyframe *keyframe_array = FW_NEW NAnim::Keyframe[ 2 ];
		obTranslationChannel.m_KeyframeArrayOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_array ) - reinterpret_cast< intptr_t >( &obTranslationChannel ) );
		keyframe_array[ 0 ].m_obA = static_cast<CVector>( obHierarchy.GetTransform( iTransform )->GetLocalTranslation() );
		keyframe_array[ 0 ].m_obB = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 0 ].m_obC = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 0 ].m_obD = CVector( CONSTRUCT_CLEAR );

		keyframe_array[ 1 ].m_obA = static_cast<CVector>( obHierarchy.GetTransform( iTransform )->GetLocalTranslation() );
		keyframe_array[ 1 ].m_obB = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 1 ].m_obC = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 1 ].m_obD = CVector( CONSTRUCT_CLEAR );

		// Create a direct pointer to our rotation channel
		NAnim::Channel& obRotationChannel = animation_transform_array[1];

		// Fill out the translation channel
		obRotationChannel.m_eKeyframeClass = KEYFRAME_CLASS_ROTATION;
		obRotationChannel.m_eKeyframeType = KEYFRAME_TYPE_CURVE_ROTATION;
		obRotationChannel.m_iNumberOfKeyframes = 2;
		obRotationChannel.m_iWalkerArrayIndex = ( iTransform * 2 ) + 1;

		// Two keyframes - one at the start time, one at the duration time
		keyframe_times = FW_NEW float[ 2 ];
		keyframe_times[ 0 ] = 0.0f;
		keyframe_times[ 1 ] = s_fStaticAnimDuration;
		obRotationChannel.m_KeyframeTimesOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_times ) - reinterpret_cast< intptr_t >( &obRotationChannel ) );

		// Both the keyframes are constant
		keyframe_flags = FW_NEW char[ 2 ];
		keyframe_flags[ 0 ] = KEYF_IS_CONSTANT;
		keyframe_flags[ 1 ] = KEYF_IS_CONSTANT;
		obRotationChannel.m_KeyframeFlagsOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_flags ) - reinterpret_cast< intptr_t >( &obRotationChannel ) );

		// Both the keyframes describe the same rotation
		keyframe_array = FW_NEW NAnim::Keyframe[ 2 ];
		obRotationChannel.m_KeyframeArrayOffset = int32_t( reinterpret_cast< intptr_t >( keyframe_array ) - reinterpret_cast< intptr_t >( &obRotationChannel ) );
		CQuat obRotation( obHierarchy.GetTransform( iTransform )->GetLocalMatrix() );
		obRotation.Normalise();
		keyframe_array[ 0 ].m_obA = static_cast<CVector>( obRotation );
		keyframe_array[ 0 ].m_obB = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 0 ].m_obC = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 0 ].m_obD = CVector( CONSTRUCT_CLEAR );

		keyframe_array[ 1 ].m_obA = static_cast<CVector>( obRotation );
		keyframe_array[ 1 ].m_obB = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 1 ].m_obC = CVector( CONSTRUCT_CLEAR );
		keyframe_array[ 1 ].m_obD = CVector( CONSTRUCT_CLEAR );

		// We might not have a animation to hierarchy lookup because this is only used for character anims
		if ( psAnimationToHierarchyLookup )
		{
			// Just to make sure we found it
			bool bBoneFound = false;

			// We need to do a reverse lookup here...
			for ( short iBone = 0; iBone < NUM_CHARACTER_BONES; ++iBone )
			{
				// See if this bone id maps back to our current transform index
				if ( obHierarchy.GetCharacterBoneToIndexArray()[iBone] == iTransform )
				{
					psAnimationToHierarchyLookup[iTransform] = iBone;
					bBoneFound = true;
					break;
				}
			}

			// If we failed to find the bone something is screwy
			ntError_p( bBoneFound, ( "We failed to find a bone when we generated the data from the hierarchy - weird.\n" ) );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuilder::CreateAnimation
//!	Create an animation from a hierarchy - uses the pose for keyframes
//!
//------------------------------------------------------------------------------------------
CAnimationHeader* AnimBuilder::CreateAnimation( const CHierarchy& obHierarchy )
{
	// Create our new animation header
	CAnimationHeader* pobAnimHeader = FW_NEW CAnimationHeader;

	// Set the animation version tag to the current version
	pobAnimHeader->m_uiVersionTag = ANIM_VERSION;

	// Is this a character or environmental animation
	pobAnimHeader->m_iFlags = ( obHierarchy.GetCharacterBoneToIndexArray() ) ? ANIMF_CHARACTER_ANIM : 0;

	// This animation will be suitable to play on the hierarchy we used to generate it
	pobAnimHeader->m_uiHierarchyKey = obHierarchy.GetHierarchyKey();				

	// A "full body" animation so set the priority to zero
	pobAnimHeader->m_iPriority = 0;

	// This is a single frame so set the duration to a single second for our static animations - this
	// initial duration will need to be changed if we start thinking of recording animated sequences.
	pobAnimHeader->m_fDuration = s_fStaticAnimDuration;

	// All transforms on the hierarchy will be effected by this animation
	pobAnimHeader->m_iNumberOfAnimationTransforms = obHierarchy.GetTransformCount();

	// Create enough CAnimationTransforms for the given hierarchy
	CAnimationTransform *animation_transform_array( FW_NEW CAnimationTransform[ pobAnimHeader->m_iNumberOfAnimationTransforms ] );
	pobAnimHeader->m_pobAnimationTransformArrayOffset = int32_t( reinterpret_cast< intptr_t >( animation_transform_array ) - reinterpret_cast< intptr_t >( pobAnimHeader ) );

	// We will have enough channels for translation and rotation on every transform
	pobAnimHeader->m_iTotalNumberOfChannels = pobAnimHeader->m_iNumberOfAnimationTransforms * 2;

	// This is a static pose at the moment so we can assume that the locomoting effect will be zero
	pobAnimHeader->m_obRootEndRotation = CQuat( CONSTRUCT_IDENTITY );
	pobAnimHeader->m_obRootEndTranslation = CPoint( CONSTRUCT_CLEAR );

	// We will create animation data for all the transforms on the given hierarchy
	int16_t *anim_to_hierarchy_lookup( NULL );
	if ( pobAnimHeader->m_iFlags == ANIMF_CHARACTER_ANIM )
	{
		anim_to_hierarchy_lookup = FW_NEW int16_t[ pobAnimHeader->m_iNumberOfAnimationTransforms ];
		pobAnimHeader->m_psAnimationToHierarchyLookupOffset = int32_t( reinterpret_cast< intptr_t >( anim_to_hierarchy_lookup ) - reinterpret_cast< intptr_t >( pobAnimHeader ) );
	}
	else
	{
		pobAnimHeader->m_psAnimationToHierarchyLookupOffset = 0;
	}

	// Set the minimum blend in time to zero - this value isn't used yet
	pobAnimHeader->m_fMinimumBlendInTime = 0.0f;

	// Now we need to create all the keyframes
	CreateAnimKeyframes(	obHierarchy, 
							anim_to_hierarchy_lookup,
							animation_transform_array,
							pobAnimHeader->m_iNumberOfAnimationTransforms,
							pobAnimHeader );

	// Return our new animation header - someone is going to have to clear up all the mess we made
	return pobAnimHeader;
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuilder::DestroyAnimation
//!	Destroy a dynamically generated animation header - normally the data we have constructed 
//!	sits in memory as a loaded resource.
//!
//------------------------------------------------------------------------------------------
void AnimBuilder::DestroyAnimation( CAnimationHeader* pobAnimationHeader )
{
	// We can't really do anything unless we are given sensible data
	if ( !pobAnimationHeader )
	{
		ntAssert_p( 0, ( "We can't delete a dynamic animation header if we're not given a pointer." ) );
		return;
	}

	CAnimationTransform *anim_transform_array = ResolveOffset< CAnimationTransform >( pobAnimationHeader->m_pobAnimationTransformArrayOffset, pobAnimationHeader );

	// Loop through all the transform data and clean up what we constructed
	for ( int iTransform = 0; iTransform < pobAnimationHeader->m_iNumberOfAnimationTransforms; ++iTransform )
	{
		NAnim::Channel *anim_channel_array = ResolveOffset< NAnim::Channel >( anim_transform_array[ iTransform ].m_pobAnimationChannelArrayOffset, &anim_transform_array[ iTransform ] );

		// Loop through each of the channels for the current transform
		for ( int iChannel = 0; iChannel < anim_transform_array[iTransform].m_sNumberOfChannels; ++iChannel )
		{
			// Delete all the arrays of data we constructed here
			FW_DELETE_ARRAY( ResolveOffset< float >( anim_channel_array[ iChannel ].m_KeyframeTimesOffset, &anim_channel_array[ iChannel ] ) );
			FW_DELETE_ARRAY( ResolveOffset< int8_t >( anim_channel_array[ iChannel ].m_KeyframeFlagsOffset, &anim_channel_array[ iChannel ] ) );

			ntError(	anim_channel_array[iChannel].m_eKeyframeType == KEYFRAME_TYPE_CURVE_ROTATION ||
						anim_channel_array[iChannel].m_eKeyframeType == KEYFRAME_TYPE_CURVE_TRANSLATION );
			
			FW_DELETE_ARRAY( ResolveOffset< NAnim::Keyframe >( anim_channel_array[ iChannel ].m_KeyframeArrayOffset, &anim_channel_array[ iChannel ] ) );
		}

		// Now delete all the channel data for this transform
		FW_DELETE_ARRAY( anim_channel_array );
	}

	// Now delete the entire array of transform data
	FW_DELETE_ARRAY( anim_transform_array );

	// Now delete the lookup table if we have one
	int16_t *anim_to_hierarchy_lookup = ResolveOffset< int16_t >( pobAnimationHeader->m_psAnimationToHierarchyLookupOffset, pobAnimationHeader );
	if ( anim_to_hierarchy_lookup )
		FW_DELETE_ARRAY( anim_to_hierarchy_lookup );

	// And finally kill off the header itself
	FW_DELETE_ARRAY( pobAnimationHeader );
}



