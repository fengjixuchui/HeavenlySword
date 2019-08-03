//------------------------------------------------------------------------------------------
//!
//!	\file animation.cpp
//!	Phase Linking?
//!	--------------
//!	
//!	The implementation of phase-linking between animations is quite different to the 
//!	system found in KFC. Instead of having to deal with all the mess of previous & next
//!	animations, the system utilises the fact that all phase-linked anims have the same
//!	animation-relative phase position, and also the same phase speed too.
//!	
//!	Each animations phase position (P) can be computed as p = ( t / d ), where "t" is the 
//!	position within the animation, and "d" is the duration of that animation. An animations
//!	phase speed (s) can be computed as ( t' / d ), where "t'" is the speed of the animation
//!	and, again, "d" is the duration. Each animation also has a usable blend weight 'w', which
//!	is distributed in priority order while taking into account blend in times.
//!	
//!	In an example system where we have animations A, B, and C, we compute a global phase
//!	position (gP) as follows:
//!	
//!	gP = ( ( pA * wA ) + ( pB * wB ) + pC * wC ) )
//!	 --------------------------------------
//!	  wA + wB + wC
//!	
//!	Global speed (gS) is computed similarly:
//!	
//!	gS = ( ( sA * wA ) + ( sB * wB ) + sC * wC ) )
//!	   --------------------------------------
//!	    wA + wB + wC
//!	
//!	We then apply speed to the global position utilising the timestep of the current frame
//!	as shown:
//!	
//!	gP = gP + ( gS * fTimeStep )
//!	
//!	Each animation then has its own position updated by reapplying the global position (gP).
//!	This is a simple multiply by the duration of the animation. After this the standard 
//!	Update() calls are invoked to determine pre/post sequence looping and to handle the
//!	positioning within the keyframe array.
//!	
//!	It should be noted that on an animations first frame in existence, it's effective
//!	blend weight contribution to the above calculations is forced to zero, as the initial
//!	position (P) of an animation is not correct for phase linked anims during the first
//!	frame (as it hasn't had an update to force positional reevaluation). This is similar
//!	to the mechanism used to inhibit root delta calculations for locomoting animations, as
//!	they also have no valid data to use prior to first update. It was more efficient in both
//!	of these cases to have a first-frame exclusion case, rather than add potentially time
//!	consuming patch-up cases.
//!
//! The ability for an animation to be phase linked and 180 degrees out of phase has been
//! added to allow the linking of animations that begin/end phase linked cycles - GH
//!	
//------------------------------------------------------------------------------------------

// Necessary includes
#include "anim/animation.h"
#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "anim/ChannelWalker.h"
#include "gfx/clump.h"
#include "core/exportstruct_keyframe.h"
#include "core/exportstruct_anim.h"
#include "anim/PriorityCache.h"
#include "game/entityanimcontainer.h"

#ifdef _DEBUG
	namespace AnimHelpers
	{
		void LogAnimationString( const char *str );
	}
#endif

void CheckRootType( GpAnimation *pAnimation, GpAnimation::LocomotionKey locomotion_key, const char *debug_anim_name )
{
	UNUSED( pAnimation );
	UNUSED( locomotion_key );
	UNUSED( debug_anim_name );

#	ifdef _DEBUG
	{
		union 
		{ 
			short						index[ 2 ]; 
			GpAnimation::LocomotionKey	key; 
		} crackKey; 

		crackKey.key = locomotion_key;

		// Validate that rotation/translation channels are not bitpacked.. 
		const FpAnimClipDef *pAnimClipDef = pAnimation->GetAnimClipDef(); 

		if (	( crackKey.index[ 0 ] != -1 ) &&
				( crackKey.index[ 0 ] >= pAnimClipDef->GetChannelTypeStart( FpAnimChannelInfo::kPackedHermite ) ) && 
				( crackKey.index[ 0 ] < ( pAnimClipDef->GetChannelTypeStart( FpAnimChannelInfo::kPackedHermite ) + pAnimClipDef->GetChannelTypeCount( FpAnimChannelInfo::kPackedHermite ) ) ) )
		{ 
			// rotation channel index is in range of packed hermite channels.
			user_warn_msg( ("Animation %s has bit-packed root rotation channel - export bug.", debug_anim_name) );

			char error_str[ 512 ];
			sprintf( error_str, "The following anim has a bit-packed root rotation node! %s\n", debug_anim_name );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( error_str );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
		} 

		if (	( crackKey.index[ 1 ] != -1 ) &&
				( crackKey.index[ 1 ] >= pAnimClipDef->GetChannelTypeStart( FpAnimChannelInfo::kPackedHermite ) ) && 
				( crackKey.index[ 1 ] < (  pAnimClipDef->GetChannelTypeStart( FpAnimChannelInfo::kPackedHermite ) + pAnimClipDef->GetChannelTypeCount( FpAnimChannelInfo::kPackedHermite ) ) ) ) 
		{ 
			// translation channel index is in range of packed hermite channels.. 
			user_warn_msg( ("Animation %s has bit-packed root translation channel - export bug.", debug_anim_name) );

			char error_str[ 512 ];
			sprintf( error_str, "The following anim has a bit-packed root translation node! %s\n", debug_anim_name );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( error_str );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
			AnimHelpers::LogAnimationString( "***********************************************************************************\n" );
		} 
	}
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::Create
//!	evaluate current position in a tracking channel
//!	Creates an instance of an animation suitable for attachment to a CAnimator
//!	object. In order to allow us more effectively manage our memory allocations, 
//!	all workspace is allocated as part of the construction of this object.
//!	
//!	INPUTS			pobAnimationHeader		-	A pointer to a loaded animation header
//!	
//!					pobHierarchy			-	A pointer to the hierarchy that this animation will
//!												be applied to.
//!	
//!	Unfortunately, a by-product of this scheme is that we need a pointer to the
//!	hierarchy that this animation will be applied to when constructing the object.
//!	I only use the hierarchy pointer to obtain the number of embedded transforms.
//!	
//!	Sorry about this.. but it's quite necessary.
//!	
//------------------------------------------------------------------------------------------
CAnimationPtr CAnimation::Create(	const CAnimator *		animator,
									const uint32_t			uiShortNameHash,
									const CAnimationHeader*	pobAnimationHeader,
									const char*				pDebugAnimName )
{
	// Validation.
	ntError_p( pobAnimationHeader != NULL, ("You can't have an animation without some animation data.") );

	UNUSED( pDebugAnimName );

	// Work out a total allocation size.
	int32_t	iAllocateSize = ROUND_POW2( sizeof( CAnimation ), 16 ) +
							ROUND_POW2( GpAnimation::QuerySizeInBytes( animator->GetGpAnimator(), pobAnimationHeader ), 16 );

	// Now grab some memory.
	void *anim_memory = (void *)NT_MEMALIGN_CHUNK( Mem::MC_ANIMATION, iAllocateSize, 128 );
	CAnimation *pobAnimation = NT_PLACEMENT_NEW( anim_memory ) CAnimation();

	// Initialise the fields.
	pobAnimation->m_uiShortNameHash			=	uiShortNameHash;

	pobAnimation->m_iFlags					=	0;
	pobAnimation->m_Priority				=	0;
	pobAnimation->m_LoopCount				=	0;
	pobAnimation->m_fTime					=	0.0f;
	pobAnimation->m_fPreviousTime			=	0.0f;
	pobAnimation->m_fSpeed					=	1.0f;

	pobAnimation->m_fBlendWeight			=	1.0f;

	pobAnimation->m_bInitialUpdate			=	true;
	pobAnimation->m_IsActive				=	false;
	pobAnimation->m_ToBeDeleted				=	false;
	pobAnimation->m_AddedToAnimatorThisFrame=	false;

	pobAnimation->m_RootRotationDelta.SetIdentity();
	pobAnimation->m_RootTranslationDelta.Clear();

	pobAnimation->m_AnimationData			=	GpAnimation::Create	(	animator->GetGpAnimator(),
																		pobAnimationHeader,
																		NULL,
																		(void *)ROUND_POW2( (uintptr_t)( pobAnimation + 1 ), 128 )
																	);
	pobAnimation->m_pAnimShortCut			=	0;

	pobAnimation->m_LocomotionKey			=	pobAnimation->m_AnimationData->GetLocomotionKey();

	pobAnimation->m_pobRelativeTransform	=	0;	// Speculative relative animation stuff - GILES

	pobAnimation->m_Priority = PriorityCache::Get().GetPriority( pobAnimation, animator->GetHierarchy() );

#	ifdef _DEBUG
	{
		CheckRootType( pobAnimation->m_AnimationData, pobAnimation->m_LocomotionKey, pDebugAnimName );
	}
#	endif

	// Return a pointer to the animation
	return CAnimationPtr( pobAnimation );
}

CAnimationPtr CAnimation::Create(	const CAnimator *	animator,
									const uint32_t		uiShortNameHash,
									const AnimShortCut*	pAnimShortcut,
									const char*			pDebugAnimName )
{
	ntError_p( pAnimShortcut != NULL, ("You can't have an animation without some animation data.") );
	CAnimationPtr result = Create( animator, uiShortNameHash, pAnimShortcut->GetHeader(), pDebugAnimName ); 
	result->m_pAnimShortCut = pAnimShortcut;
	return result;
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::Release
//!
//------------------------------------------------------------------------------------------
void CAnimation::Release() const
{
	ntError_p( m_RefCount > 0, ("This ref-count is <= 0, why isn't the object deleted already?") );
	if ( --m_RefCount == 0 )
	{
		this->~CAnimation();

		uint8_t* pData = (uint8_t*)this;
		NT_FREE_CHUNK( Mem::MC_ANIMATION, (uintptr_t)pData );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::~CAnimation()
//!
//------------------------------------------------------------------------------------------
CAnimation::~CAnimation()
{
	GpAnimation::Destroy( m_AnimationData );
	m_AnimationData = NULL;
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::UpdateTime
//!	For animations that don't need phase linking, this function is called to ensure
//!	that times are updated based on the input fTimeStep. Animation blend weights are
//!	also computed based on lead-in/out times. This processing used to be part of 
//!	the core Update() routine, but was split out in order to allow implementation of
//!	phase-linked animations.
//!
//------------------------------------------------------------------------------------------
void CAnimation::UpdateTime( float fTimeStep )
{
	if ( !m_bInitialUpdate )
	{
		// We keep our old time so we can look for transitions over specialised message keyframe types
		m_fPreviousTime = m_fTime;

		// Update our time..
		m_fTime += ( fTimeStep * GetSpeed() );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::InitialLinkageUpdate
//!	Whereas standard animations use UpdateTime() to ensure that times and blend
//!	weights are correctly updated, phase-linked animations call this instead. This
//!	is responsible for computing correct lead-in adjusted blend weights, as well as
//!	gathering data needed for correct update of phase position & speed.
//!
//------------------------------------------------------------------------------------------
void	CAnimation::InitialLinkageUpdate( float /*fTimeStep*/, float& fLinkedWeightSum, float& fLinkedPositionSum, float& fLinkedSpeedSum, float& fLinkedRemainingWeight )
{
	// We only apply this animations information to our sums if we've had a frame already, as
	// on first frame the current m_fTime for phase-linked anims is not actually valid for us.
	if( !m_bInitialUpdate )
	{
		// Update our linked weight sum
		float fAppliedBlendWeight = ntstd::Min( fLinkedRemainingWeight, m_fBlendWeight );
		fLinkedRemainingWeight -= fAppliedBlendWeight;
		ntAssert( fAppliedBlendWeight >= 0.0f );
	
		// Updated linked weight sum
		fLinkedWeightSum += fAppliedBlendWeight;

		// Update linked speed sum
		fLinkedSpeedSum += ( m_fSpeed / GetDuration() ) * fAppliedBlendWeight;

		// Update linked position sum - get the phase
		float fPhase = m_fTime / GetDuration();

		// If this animation only represents half a phase we need to chop it
		if ( GetFlags() & ANIMF_HALF_PHASE )
		{
			fPhase /= 2.0f;
		}

		// If this animation is 180 out of phase we need to shift it
		if ( GetFlags() & ANIMF_PHASE_OFFSET )
		{
			fPhase += 0.5f;
			if ( fPhase > 1.0f )
				fPhase -= 1.0f;
		}
		
		// Add this to the linked position sum
		fLinkedPositionSum += ( fPhase * fAppliedBlendWeight );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::ApplyLinkageChange
//!	After we've gathered phase-linkage related variables by calling
//!	InitialLinkageUpdate() for all our phase-linked animations, we compute a new
//!	position that's applied to all phase-linked anims. The position is independent
//!	of any specific animation duration at this time... the correct scaling is 
//!	performed here.
//!	
//!	After this function executes, it's quite possible for the new position to be
//!	outside of the range 0.0f to 1.0f. This occurs when the animations loop. The 
//!	core animation Update() call will notice this and handle looping events as
//!	necessary.
//!
//------------------------------------------------------------------------------------------
void CAnimation::ApplyLinkageChange( float fNewPosition )
{
	// Update the time
	m_fPreviousTime = m_fTime;

	// If this anim only represents a half phase we need to fix up the position
	if ( GetFlags() & ANIMF_HALF_PHASE )
	{
		// If the anim is 180 out phase offset the linkage
		if ( GetFlags() & ANIMF_PHASE_OFFSET )
		{
			// This animation represents a phase of 0.5f - 1.0f
			ntAssert( ( fNewPosition >= 0.5f ) && ( fNewPosition <= 1.0f ) );

			// Adjust the phase
			fNewPosition = ( ( fNewPosition - 0.5f ) * 2.0f );

			// Set the time based on the position
			m_fTime = fNewPosition * GetDuration();
		}
		else
		{
			// This animation represents a phase of 0.0f - 0.5f
			ntAssert( ( fNewPosition >= 0.0f ) && ( fNewPosition <= 0.5f ) );

			// Adjust the phase
			fNewPosition = ( fNewPosition * 2.0f );

			// Set the time based on the position
			m_fTime = fNewPosition * GetDuration();
		}
	}
	else
	{
		// If the anim is 180 out phase offset the linkage
		if ( GetFlags() & ANIMF_PHASE_OFFSET )
		{
			// Set the new position with a 50% phase shift
			fNewPosition -= 0.5f;
			if ( fNewPosition < 0.0f ) 
				fNewPosition += 1.0f;

			// Set the time based on the position
			m_fTime = fNewPosition * GetDuration();
		}
		else
		{
			// Set the time based on the position
			m_fTime = fNewPosition * GetDuration();
		}
	}

	// If this is not a looping animation stop it after one lap
	if ( !( GetFlags() & ANIMF_LOOPING ) )
	{
		// If we have return to animation start
		if ( m_fPreviousTime > m_fTime )
			m_fTime = GetDuration();
	}

	// If we've just added this animation and it's being locomoted from anywhere
	// but the start then we need to reset the locomotion data on the atg animation
	// object, otherwise we'll get jumping when we blend root deltas.
	if ( m_AddedToAnimatorThisFrame && m_fTime != 0.0f )
	{
		// We need to account for a locomoting anim here - the new time might be outside
		// the [0,GetDuration()] range.
		float looped_time = m_fTime;
		if ( m_iFlags & ANIMF_LOCOMOTING )
		{
			if ( looped_time < 0.0f )
				looped_time += GetDuration();
			else if ( looped_time > GetDuration() )
				looped_time -= GetDuration();
		}
		else
		{
			// Not looping - just clamp.
			if ( looped_time < 0.0f )
				looped_time = 0.0f;
			else if ( looped_time > GetDuration() )
				looped_time = GetDuration();
		}

		ntError( looped_time >= 0.0f && looped_time <= GetDuration() );
		m_AnimationData->ResetLocomotion( looped_time, m_LocomotionKey );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimation::GetFirstPhasePosition
*
*	DESCRIPTION		Find the phase position of an animation - the phase that a particular 
*					time/duration represents depends on a number of flags
*
***************************************************************************************************/
float CAnimation::GetPhasePosition( void ) const
{
	// Get the phase for a standard phase linked animation
	float fPhase = m_fTime / m_AnimationData->GetDuration();

	// If the animation only represents half a phase - half the phase!
	if ( m_iFlags & ANIMF_HALF_PHASE )
	{
		fPhase /= 2.0f;
	}

	// If this has an offset phase - offset the phase!
	if ( m_iFlags & ANIMF_PHASE_OFFSET )
	{
		fPhase += 0.5f;
		if ( fPhase > 1.0f )
			fPhase -= 1.0f;
	}

	// Can you tell what it is yet?
	return fPhase;
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::Update
//!	Updates an instance of an animation using a time step value. Normally this value
//!would be the equivalent of 1 or 2 frames (running at 50/60Hz).
//!
//!A pose of an animation at the current time, and returned flags informing the
//!caller of any special cases that need to be dealt with. Things like, do I need
//!deleting..
//!
//!The time step is scaled by the animation playback speed prior to being used.
//!We can shortcut all of the work potentially needed by the object if the 
//!required blend weight is 0.0f. In this case we know that none of the animation
//!is required by the final hierarchy blending process. 
//!
//!The only dependency on CAnimator within CAnimation::Update is
//!Get/SetAvailableBlendWeight(...).
//!
//------------------------------------------------------------------------------------------
void CAnimation::Update()
{
	m_LoopCount = 0;

	// Now we've updated the time, we need to ensure it lies within the animation...
	if ( m_fTime < 0.0f )
	{
		if ( GetFlags() & ANIMF_LOOPING )
		{
			m_LoopCount--;
			m_fTime += GetDuration();
		}
		else
		{
			m_fTime = 0.0f;
			if ( !( GetFlags() & ANIMF_INHIBIT_AUTO_DESTRUCT ) )
			{
				m_ToBeDeleted = true;
			}
		}
	}
	else if ( m_fTime > GetDuration() )
	{
		if ( GetFlags() & ANIMF_LOOPING )
		{
			m_LoopCount++;
			m_fTime -= GetDuration();

			// It may be that the animation is shorter than an update or that the framerate is
			// unstable, in this case we completely reset the time to retain robustness - GH
			if ( ( m_fTime > GetDuration() ) || ( m_fTime < 0.0f ) )
				m_fTime = 0.0f;
		}
		else
		{
			m_fTime = GetDuration();
			if ( !( GetFlags() & ANIMF_INHIBIT_AUTO_DESTRUCT ) )
			{
				m_ToBeDeleted = true;
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimation::CalcLocomotion
//!	Work out locomotion deltas for root node.
//!
//------------------------------------------------------------------------------------------
void CAnimation::CalcLocomotion()
{
	m_RootRotationDelta		= CQuat( m_AnimationData->GetRootRotationDelta().QuadwordValue() );
	m_RootTranslationDelta	= CPoint( m_AnimationData->GetRootTranslationDelta().QuadwordValue() );

	ntAssert( m_RootRotationDelta.IsNormalised() );
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::SetPercentage
//!	This is a more abstract interface to SetTime.  Some game systems want to see some 
//! variation in looping animations - this allows them to set offsets without prior
//!	knowledge of an animations duration.
//!
//------------------------------------------------------------------------------------------
void CAnimation::SetPercentage( float fPercentage )
{
	// Check the quality of our input
	ntAssert( fPercentage >= 0.0f );
	ntAssert( fPercentage <= 1.0f );

	// Set the time
	SetTime( GetDuration() * fPercentage );
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::SetTime
//!	We need to mess around with things here. This is, obviously, a little vague.. but
//!	suffice to say that we don't really support arbitary positioning within our
//!	keyframe set (yet). So, we have to set the animation to a position of 0.0f, we
//!	frig the state so that the system thinks we're on the first tick of the animations
//!	life, we clear the stored root rotations and translations, and then we cross our
//!	fingers and hope for the best.
//!	
//!	If all goes well, the animation system doesn't implode and the modified animation
//!	continues from the specified time..
//!
//! Ben has made some changes here.  Arbitrary positioning for an animation does work - its
//! just not very quick because the channel walkers are written for interative time updates.
//! A binary seek capability added here will make it easy to set any time on an animation.
//!
//------------------------------------------------------------------------------------------
void CAnimation::SetTime( float fTime )
{
	// We can only ever go to somewhere within the animation. This isn't going to change..
	ntAssert( ( fTime >= 0.0f ) && ( fTime <= GetDuration() ) );

	// We need to deal with this more robustly...
	if ( fTime > GetDuration() )
		fTime = GetDuration();

	if ( fTime < 0 )
		fTime = 0.0f;

	// Lets do our messing around..
	m_fPreviousTime				= fTime;
	m_fTime						= fTime;
	m_bInitialUpdate			= true;
	m_RootRotationDelta			= CQuat( CONSTRUCT_IDENTITY );
	m_RootTranslationDelta		= CPoint( CONSTRUCT_CLEAR );

	m_AnimationData->ResetLocomotion( fTime, m_LocomotionKey );
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::SetTimeRemaining
//! Sets the speed using the time the user would like the rest of the animation to play for
//! 
//------------------------------------------------------------------------------------------
void CAnimation::SetTimeRemaining( float fTimeRemaining )
{
	// Make sure we don't get divide by zero badness
	ntAssert( fTimeRemaining > EPSILON );

	// Set the speed - you do the math
	m_fSpeed = ( GetDuration() - m_fTime ) / fTimeRemaining;
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::GetRootTranslationAtTime
//! This finds the translation of the root at a particular point in an animation.
//! I'm sure that this isn't the quickest method in the world but i am just getting
//! stuff functional at this point.  It may not be used at all in the final title.
//! 
//! If this sort of stuff proves useful then its use should be considered in any
//! animation re-write.
//! 
//------------------------------------------------------------------------------------------
CPoint CAnimation::GetRootTranslationAtTime( float fTime, const CHierarchy *hierarchy, bool bIsRelative )
{
	// Do some basic checking on the input 
	ntAssert( ( fTime >= 0.0f ) && ( fTime <= GetDuration() ) );

	// If the time is zero we return no translation - that is the assumption we make
	if ( !bIsRelative && fTime <= 0.0f )
		return CPoint( CONSTRUCT_CLEAR );

	// If the input time is the duration time then we can get info from the animation header
	if ( fTime >= GetDuration() )
		return GetRootEndTranslation();

	ntError( m_AnimationData->GetAnimClipDef() != NULL );

	int32_t root_translation_channel_index = m_AnimationData->GetAnimClipDef()->GetChannelIndex( AnimNames::root, AnimNames::translate );

	//
	//	BASTARD HACK.
	//
	//		Some fuckers have been creating objects with root nodes that aren't named "root". Grr.
	//		If this is the case, we try and find the first translation channel and eval that instead.
	//
	if ( root_translation_channel_index < 0 )
	{
		ntError( hierarchy->GetGpSkeleton() != NULL );
		FwHashedString root_name = hierarchy->GetGpSkeleton()->GetJointName( GpSkeleton::kRootJointIndex );
		int32_t root_item_index = m_AnimationData->GetAnimClipDef()->GetItemIndex( root_name );
		if ( root_item_index == -1 )
		{
			ntError_p( false, ("There is no root of this animation! Are you being stupid and calling this on a partial anim?") );
			return CPoint( CONSTRUCT_CLEAR );
		}

		root_translation_channel_index = m_AnimationData->GetAnimClipDef()->GetItemChannelIndex( root_item_index, AnimNames::translate );
		if ( root_translation_channel_index == -1 )
		{
			return CPoint( CONSTRUCT_CLEAR );
		}
	}
	//
	//	END OF BASTARD HACK.
	//

	CPoint root_translation( m_AnimationData->GetAnimClipDef()->Evaluate( root_translation_channel_index, fTime ).QuadwordValue() );

	if ( !bIsRelative && m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() != NULL )
	{
		// If the animation isn't relative then we want to work out the delta to the start of the anim.
		root_translation -= CPoint( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetStartTranslation().QuadwordValue() );
	}

	return root_translation;
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimation::GetRootRotationAtTime
//! This finds the rotation of the root at a particular point in an animation.
//!	I'm sure that this isn't the quickest method in the world but i am just getting
//!	stuff functional at this point.  It may not be used at all in the final title.
//!	
//!	If this sort of stuff proves useful then its use should be considered in any
//!	animation re-write.
//!
//------------------------------------------------------------------------------------------
CQuat CAnimation::GetRootRotationAtTime( float fTime, const CHierarchy *hierarchy )
{
	// Do some basic checking on the input 
	ntAssert( ( fTime >= 0.0f ) && ( fTime <= GetDuration() ) );

	// If the time is zero we return no rotation - that is the assumption we make
	if ( fTime <= 0.0f )
		return CQuat( CONSTRUCT_IDENTITY );

	// If the input time is the duration time then we can get info from the animation header
	if ( fTime >= GetDuration() )
		return GetRootEndRotation();

	int32_t root_rotation_channel_index = m_AnimationData->GetAnimClipDef()->GetChannelIndex( AnimNames::root, AnimNames::rotate );

	//
	//	BASTARD HACK.
	//
	//		Some fuckers have been creating objects with root nodes that aren't named "root". Grr.
	//		If this is the case, we try and find the first rotation channel and eval that instead.
	//
	if ( root_rotation_channel_index < 0 )
	{
		ntError( hierarchy->GetGpSkeleton() != NULL );
		FwHashedString root_name = hierarchy->GetGpSkeleton()->GetJointName( GpSkeleton::kRootJointIndex );
		int32_t root_item_index = m_AnimationData->GetAnimClipDef()->GetItemIndex( root_name );
		if ( root_item_index == -1 )
		{
			ntError_p( false, ("There is no root of this animation! Are you being stupid and calling this on a partial anim?") );
			return CQuat( CONSTRUCT_IDENTITY );
		}

		root_rotation_channel_index = m_AnimationData->GetAnimClipDef()->GetItemChannelIndex( root_item_index, AnimNames::rotate );
		if ( root_rotation_channel_index == -1 )
		{
			return CQuat( CONSTRUCT_IDENTITY );
		}
	}
	//
	//	END OF BASTARD HACK.
	//

	ntError( root_rotation_channel_index < m_AnimationData->GetAnimClipDef()->GetChannelCount() );
	return CQuat( m_AnimationData->GetAnimClipDef()->Evaluate( root_rotation_channel_index, fTime ).QuadwordValue() );
}


//************************************************************************************************************
// Speculative relative animation stuff - GILES
//************************************************************************************************************
CPoint CAnimation::GetRootWorldPosition( const Transform *root ) const
{
	// Make sure that we are only calling this on the animations we should be
	ntAssert ( /*( GetFlags() & ANIMF_CHARACTER_ANIM ) &&*/ ( GetFlags() & ANIMF_RELATIVE_MOVEMENT ) );

	CPoint start_translation( CONSTRUCT_CLEAR );
	if ( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() != NULL )
	{
		start_translation = CPoint( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetStartTranslation().QuadwordValue() );
	}

	// If there is no base transform we just hand back the straight anim translation
	if ( !m_pobRelativeTransform )
	{
		return CPoint( m_AnimationData->GetRootTranslationDelta().QuadwordValue() ) + start_translation;
	}
	
	// If we have a pointer to a relative transform -take that into account
	else
	{
		ntError( root != NULL );
		CPoint root_translation = root->GetLocalTranslation();

		// Get the translation from the
//		CPoint root_translation = CPoint( m_AnimationData->GetRootTranslationDelta().QuadwordValue() ) + start_translation;
		return root_translation * m_pobRelativeTransform->GetWorldMatrix();
	}
}

//************************************************************************************************************
// Speculative relative animation stuff - GILES
//************************************************************************************************************
CQuat CAnimation::GetRootWorldOrientation( const Transform *root ) const
{
	// Make sure that we are only calling this on the animations we should be
	ntAssert ( /*( GetFlags() & ANIMF_CHARACTER_ANIM ) &&*/ ( GetFlags() & ANIMF_RELATIVE_MOVEMENT ) );

	FwQuat start_rotation( FwMaths::kIdentity );
	if ( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() != NULL )
	{
		start_rotation = m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetStartRotation();
	}

	// If we don't have a pointer to a relative transform
	if ( !m_pobRelativeTransform )
	{
		return CQuat( ( m_AnimationData->GetRootRotationDelta() * start_rotation ).QuadwordValue() );
	}

	// ...otherwise we need to take into account the rotation of the transform
	else
	{
		ntError( root != NULL );
		CQuat root_rotation = root->GetLocalRotation();

//		CQuat root_rotation( ( m_AnimationData->GetRootRotationDelta() * start_rotation ).QuadwordValue() );
		return m_pobRelativeTransform->GetWorldRotation() * root_rotation;
	}
}





void CAnimation::TestFunc( CHierarchy * /*hierarchy*/ )
{
	CPoint end_delta( CONSTRUCT_CLEAR );
	if ( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo() != NULL )
	{
		end_delta = CPoint( m_AnimationData->GetAnimClipDef()->GetLocomotionInfo()->GetEndDeltaTranslation().QuadwordValue() );
	}

	CPoint last_trans( m_AnimationData->GetLastRootTranslation().QuadwordValue() );

	ntPrintf( "id: 0x%08X - loop: %i, root_delta: %.3f, %.3f, %.3f, end_delta: %.3f, %.3f, %.3f, last_trans: %.3f, %.3f, %.3f, time: %.3f, weight: %.3f, ",
			m_uiShortNameHash, m_LoopCount, m_RootTranslationDelta.X(), m_RootTranslationDelta.Y(), m_RootTranslationDelta.Z(),
			end_delta.X(), end_delta.Y(), end_delta.Z(), last_trans.X(), last_trans.Y(), last_trans.Z(), m_fTime, m_fBlendWeight );

	if ( ( GetFlags() & ANIMF_LOCOMOTING ) || ( m_AnimationData->GetAnimClipDef()->GetFlags() & FpAnimClipDef::kIsLocomoting ) )
		ntPrintf( "Loco: " );

	if ( GetFlags() & ANIMF_LOCOMOTING )
		ntPrintf( "NT " );

	if ( m_AnimationData->GetAnimClipDef()->GetFlags() & FpAnimClipDef::kIsLocomoting )
		ntPrintf( "ATG" );

	if ( GetFlags() & ANIMF_RELATIVE_MOVEMENT )
		ntPrintf( ", REL" );

	ntPrintf( "\n" );
/*
//	if ( m_fTime >= 0.225f && m_fTime <= 0.235f )
	{
		FwHashedString item_name = hierarchy->GetGpSkeleton()->GetJointName( 1 );
		int32_t item_index = m_AnimationData->GetAnimClipDef()->GetItemIndex( item_name );
		int32_t item_channel_index = m_AnimationData->GetAnimClipDef()->GetItemChannelIndex( item_index, AnimNames::rotate );
		CQuat rot( m_AnimationData->GetAnimClipDef()->Evaluate( item_channel_index, m_fTime ).QuadwordValue() );
		ntPrintf( "id: 0x%08X, pelvis rot: %.3f, %.3f, %.3f, %.3f\n", m_uiShortNameHash, rot.X(), rot.Y(), rot.Z(), rot.W() );
	}
*/
}



