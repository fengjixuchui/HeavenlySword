/***************************************************************************************************
*
*	$Header:: /game/animator.cpp 16    19/08/03 8:32 Dean                                          $
*
*	Animation Blending & Control
*
*	CHANGES
*
*	24/3/2003	Dean	Created
*
***************************************************************************************************/

#include "Gp/GpAnimator/GpAnimator.h"

// Necessary includes
#include "anim/animator.h"
#include "anim/animation.h"
#include "anim/hierarchy.h"
#include "anim/animloader.h"

#include "core/exportstruct_clump.h"
#include "core/exportstruct_anim.h"
#include "core/exportstruct_keyframe.h"
#include "core/visualdebugger.h"

#include "gfx/clump.h"

#include "game/EntityAnimSet.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/luaexptypes.h"
#include "lua/ninjalua.h"

#include "core/gatso.h"

#ifdef PLATFORM_PS3
#	include "exec/ppu/exec_ps3.h"
#	include "jobapi/joblist.h"
#endif

/***************************************************************************************************
*
*	FUNCTION		CAnimator::Create
*
*	DESCRIPTION		Just allocates and properly constructs member variables. CAnimator::Create
*					initialises.
*
***************************************************************************************************/
CAnimator::CAnimator() 
:	m_bEnabled		( true )
{}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::Create
*
*	DESCRIPTION		Creates an instance of an animator suitable for managing control of a list of 
*					CAnimation objects. In order to allow us more effectively manage our memory
*					allocations, all workspace is allocated as part of the construction of this object.
*
*	INPUTS			pobHierarchy			-	A pointer to the hierarchy that this animation will
*												be applied to.
*
*	NOTES			Unfortunately, a by-product of this scheme is that we need a pointer to the
*					hierarchy that this animation will be applied to when constructing the object.
*					I only use the hierarchy pointer to obtain the number of embedded transforms.
*
*					Sorry about this.. but it's quite necessary. And you'll find the same mechanism
*					used in CAnimation::Create() too..
*
***************************************************************************************************/
CAnimator*	CAnimator::Create( EntityAnimSet *animShortCuts, CHierarchy *hierarchy, CEntity *parentEntity/* = NULL*/, bool has_user_data_channels/* = false*/ )
{
	// Validation
	//ntAssert_p( animShortCuts != NULL, ("You must supply an EntityAnimSet object.") );
	ntError_p( hierarchy, ("We require a CHierarchy object to create a CAnimator object.") );
	ntError_p( (uint32_t)hierarchy->GetTransformCount() <= 128, ("We shouldn't be exporting hierarchies with >128 bones in them. This won't work.") );
	ntError_p( AnimData::UserBindingDef != NULL, ("You must call AnimData::Create() before CAnimator::Create() is callled.") );

	static const int32_t NumCommandListElements = 64;

	// Pre-calculate sizes of components
	uint32_t iAnimatorSize		=	ROUND_POW2( sizeof( CAnimator ), 128 );
	uint32_t GpAnimatorSize		=	ROUND_POW2( GpAnimator::QuerySizeInBytes( hierarchy->GetGpSkeleton(), has_user_data_channels ? AnimData::UserBindingDef : NULL, NumCommandListElements ), 128 );

	// Work out a total allocation size
	uint32_t iAllocateSize	=	iAnimatorSize +			// CAnimator object
								GpAnimatorSize;			// Room for the GpAnimator object.

	// Now grab some memory (enough for the animator and workspace)
	void* pvBlock = (void *)NT_MEMALIGN_CHUNK( Mem::MC_ANIMATION, iAllocateSize, 128 );

	// Placement new the animator into the start of it
	CAnimator* pobAnimator = NT_PLACEMENT_NEW( pvBlock ) CAnimator;

	// Initialise the fields..
	pobAnimator->m_Flags					=	0;
	pobAnimator->m_pobHierarchy				=	hierarchy;
	pobAnimator->m_AnimShortCutContainer	=	animShortCuts;
	pobAnimator->m_fPhasePosition			=	0.0f;
	pobAnimator->m_fDebugLastFramePhase		=	0.0f;
	pobAnimator->m_bOverridePhase			=	false;
	pobAnimator->m_AnimatorToClampTo		=	NULL;
	pobAnimator->m_NumAnimations			=	0;
	pobAnimator->m_HasUserBindings			=	has_user_data_channels;
	pobAnimator->m_LocalRootRotation		=	CQuat( CONSTRUCT_IDENTITY );
	pobAnimator->m_LocalRootTranslation		=	CPoint( CONSTRUCT_CLEAR );

	uintptr_t anim_data_offset = ROUND_POW2( (uintptr_t)( pobAnimator + 1 ), 128 );
	pobAnimator->m_AnimatorData				=	GpAnimator::Create(	hierarchy->GetGpSkeleton(),
																	has_user_data_channels ? AnimData::UserBindingDef : NULL,
																	(void *)anim_data_offset,
																	NumCommandListElements
																  );

	// Make sure our deltas are nicely initialised
	pobAnimator->m_obRootRotationDelta.SetIdentity();
	pobAnimator->m_obRootTranslationDelta.Clear();

	// Initialise the anim event handler
	// TODO: Animevents should just hang off the animations now
	// - we shouldn't have to deal with them separately like we do here.
	pobAnimator->m_obAnimEventHandler.SetEntity( parentEntity );
	pobAnimator->m_hasAnimEvents = ( parentEntity != NULL );

	// This is speculative stuff for the control of relative animations
	pobAnimator->m_obRelativeMovementPos.Clear();
	pobAnimator->m_obRelativeMovementRot.SetIdentity();
	pobAnimator->m_bRelativeMovement = false;
	pobAnimator->m_fRelativeMovementWeight = 0.0f;

	// Return a pointer to the animation
	return pobAnimator;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::Destroy
*
*	DESCRIPTION		Static destroy that matches our create
*
***************************************************************************************************/
void	CAnimator::Destroy( CAnimator* pAnimator )
{
	ntAssert_p( pAnimator, ("Must provide a vaild CAnimator ptr here") );
	pAnimator->~CAnimator();

	uint8_t* pData = (uint8_t*) pAnimator;
	NT_FREE_CHUNK( Mem::MC_ANIMATION, (uintptr_t)pData );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::~CAnimator
*
*	DESCRIPTION		Destroys an instance of an animator. This also destroys all CAnimation objects
*					linked to the animator. Freeing of memory is handled by the overloaded delete
*					operator on CAnimator, for those interested.
*
***************************************************************************************************/

CAnimator::~CAnimator()
{
	// remove the animations from the list
	RemoveAllAnimations();

	GpAnimator::Destroy( m_AnimatorData );
	m_AnimatorData = NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CAnimator::CreateAnimation
*
*	DESCRIPTION		Helper function that allows the creation of an animation using data already 
*					available via the CAnimator and associated CEntity.
*
*	INPUTS			pcShortName		-	Short animation name
*
***************************************************************************************************/
CAnimationPtr CAnimator::CreateAnimation( const CHashedString& shortName )
{
	ntAssert_p( m_AnimShortCutContainer != NULL, ("We do not have a valid EntityAnimSet, we can't find or create animations!") );
	if ( m_AnimShortCutContainer == NULL )
	{
		user_warn_p( 0,( "CAnimator: Failed to find animation %s we donot have a valid animation container\n", ntStr::GetString(shortName) ) );

		// Return error animation
		return CAnimation::Create( this, shortName.GetValue(), CAnimLoader::Get().GetErrorAnimation(), "INVALID" );
	}

	// Look for the animation short cut (we use anim pools when creating anims)
	const AnimShortCut* pobAnim = m_AnimShortCutContainer->FindAnimShortCut( shortName, true );

	// We'll fail as gracefully as we can...
	if ( !pobAnim )
	{
		// Make sure that everyone is well aware of this ntError
		user_warn_msg( ( "CAnimator for %s failed to find animation %s\n", m_AnimShortCutContainer->GetName().c_str(), ntStr::GetString(shortName) ));

		// Return error animation
		return CAnimation::Create( this, shortName.GetValue(), CAnimLoader::Get().GetErrorAnimation(), "INVALID" );
	}

	// If we are here then all is well
	return CAnimation::Create( this, shortName.GetValue(), pobAnim, shortName.GetDebugString() );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::AddAnimation
*
*	DESCRIPTION		Adds an animation to the current CAnimator object, using the animations 
*					priority. It's the callers responsibility to ensure that the priority, blend-in
*					and blend-out are all set prior to addition. These fields cannot be changed once
*					it's been added to the CAnimator's list. 
*
*					It's recommended that critical flags such as ANIMF_INHIBIT_AUTO_DESTRUCT are 
*					defined prior to addition too, to avoid potential deletion of an animation due
*					to blend processing.
*
*	INPUTS			pobAnimation		-		A pointer to a CAnimation object
*
*	NOTES			Add something to describe priority ordering within animation list here..
*
***************************************************************************************************/
bool CAnimator::AddAnimation( const CAnimationPtr& pobAnimation )
{
	// Make sure we're adding a valid animation, and also not adding an animation that's already owned by an animator.
	ntError( pobAnimation != NULL );

	if ( m_NumAnimations >= MaxNumAnims )
	{
		DebugPrint();
	}

#	ifndef _RELEASE
	{
		// Make sure that this animation has not already been added to this animator - that
		// can lead to bad badness when only one link to it is removed later on.
		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			ntError( m_Animations[ i ] != pobAnimation );
		}
	}
#	endif

	// Add our new item to the end of the array.
	ntError_p( m_NumAnimations < MaxNumAnims, ("Cannot add more animations to this animator - we already have %i!", m_NumAnimations) );
	if ( m_NumAnimations >= MaxNumAnims )
	{
		return false;
	}

	pobAnimation->ClearFlagBits( ANIMF_WAS_FORCIBLY_REMOVED );

	m_Animations[ m_NumAnimations ] = pobAnimation;
	m_NumAnimations++;

#ifdef _DEBUG
	/*
	// Output our maximum number of animations
	static uint32_t iMaxAnimations = 0;
	if ( m_NumAnimations > iMaxAnimations )
	{
		iMaxAnimations = m_NumAnimations;
		ntPrintf( "The maximum number of animations on an animator has reached %i.\n", iMaxAnimations );
	}
	*/
#endif

	// Run the newly added item up the array until it's in the correct place according to its priority.
	// Items with greater priority should be closer to the start of the array.
	for ( int32_t i=m_NumAnimations-2;i>=0;i-- )
	{
		if ( m_Animations[ i ]->GetPriority() >= m_Animations[ i+1 ]->GetPriority() )
		{
			break;
		}

		CAnimationPtr temp = m_Animations[ i ];
		m_Animations[ i ] = m_Animations[ i+1 ];
		m_Animations[ i+1 ] = temp;
	}

	// Now set the owning animator
	pobAnimation->SetActive( true );

	// If this is a phase linked animation
	if ( pobAnimation->GetFlags() & ANIMF_PHASE_LINKED )
	{
		// This has just been added so don't take its phase into account
		// on the first blend
		pobAnimation->SetInitialUpdate();
	}

	// Send a message to the anim event handler
	if ( m_hasAnimEvents )
	{
		m_obAnimEventHandler.SetAnimation(pobAnimation);
	}

	// Make sure the times are reset before we do anything
	pobAnimation->SetTime( 0.0f );
	pobAnimation->m_AnimationData->ResetLocomotion( 0.0f, pobAnimation->m_LocomotionKey );
	pobAnimation->m_AddedToAnimatorThisFrame = true;

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAnimator::RemoveAnimation
*
*	DESCRIPTION		Removes an animation from the current animator. The animation should already
*					be owned by the current animator object.	
*
*	INPUTS			pobAnimation		-		Pointer to a valid CAnimation object
*
***************************************************************************************************/
void CAnimator::RemoveAnimation( const CAnimationPtr &pobAnimation )
{
	// Ensure that this animation is valid, and that it's active.
	ntAssert( pobAnimation );
	ntAssert( pobAnimation->IsActive() || ( pobAnimation->GetFlags() & ANIMF_WAS_FORCIBLY_REMOVED ) );

	// Lets go through all our animations
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		CAnimationPtr anim = m_Animations[ i ];

		// When we find our animation, destroy it, remove it from the list and then return to the caller
		if ( anim == pobAnimation )
		{
			// Send a message to the anim event handler
			if ( m_hasAnimEvents )
			{
				m_obAnimEventHandler.RemoveAnimation(pobAnimation);
			}

			DetachAnimation( anim );

			ntError( m_Animations[ i ] != NULL );
			for ( uint j=i;j<m_NumAnimations-1;j++ )
			{
				// Swap j and j+1.
				ntError( m_Animations[ j+1 ] != NULL );

				CAnimationPtr anim_temp = m_Animations[ j ];
				m_Animations[ j ] = m_Animations[ j+1 ];
				m_Animations[ j+1 ] = anim_temp;
			}

			m_NumAnimations--;
			m_Animations[ m_NumAnimations ] = CAnimationPtr( NULL );

			// Returning here assumes we've not added the same animation twice...
			// ...or at least, if we have, we don't want to remove both of them.
			return;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::RemoveAllAnimationsBelowWeight
*
*	DESCRIPTION		Removes all animations whose blend-weight is less than the argument.
*
***************************************************************************************************/
void CAnimator::RemoveAllAnimationsBelowWeight( float min_valid_weight )
{
	// Move any sub minimum-weight animations to the end of the array.
	uint32_t i;
	for ( i=0;i<m_NumAnimations;i++ )
	{
		if ( m_Animations[ i ]->GetBlendWeight() < min_valid_weight )
		{
			DetachAnimation( m_Animations[ i ] );

			// We need to swap this with the next valid animation.
			uint32_t j;
			for ( j=i+1;j<m_NumAnimations;j++ )
			{
				if ( m_Animations[ j ]->GetBlendWeight() >= min_valid_weight )
				{
					CAnimationPtr temp = m_Animations[ i ];
					m_Animations[ i ] = m_Animations[ j ];
					m_Animations[ j ] = temp;

					break;
				}
			}

			if ( j == m_NumAnimations )
			{
				// There was no animation to swap with, so we must be finished.
				break;
			}
		}
	}

	// Remove our sub minimum-weight animations, flagging them as forcibly removed.
	for ( uint32_t j=i;j<m_NumAnimations;j++ )
	{
		m_Animations[ j ]->SetFlagBits( ANIMF_WAS_FORCIBLY_REMOVED );
		m_Animations[ j ] = CAnimationPtr( NULL );
	}

	// Change the number of animations we have.
	m_NumAnimations = i;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::RemoveAllAnimations
*
*	DESCRIPTION		Removes all animation from the current animator. 
*
***************************************************************************************************/

void	CAnimator::RemoveAllAnimations()
{
	// Lets go through all our animations
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		CAnimationPtr anim = m_Animations[ i ];

		// Send a message to the anim event handler
		if ( m_hasAnimEvents )
		{
			m_obAnimEventHandler.RemoveAnimation( anim );
		}

		// We destroy every animation..
		DetachAnimation( anim );
	}

	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		m_Animations[ i ] = CAnimationPtr( NULL );
	}

	m_NumAnimations = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::ClearAnimWeights
*
*	DESCRIPTION		
*
***************************************************************************************************/

void	CAnimator::ClearAnimWeights()
{
	// Lets go through all our animations
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		m_Animations[ i ]->SetBlendWeight( 0.0f );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAnimator::IsPlayingAnimation
*
*	DESCRIPTION		Checks whether a specified animation is currently being played
*					Added JML for netcode
*
***************************************************************************************************/
bool CAnimator::IsPlayingAnimation( CAnimationPtr pAnim ) const
{
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		if ( m_Animations[ i ] == pAnim )
		{
			return true;
		}
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::IsPlayingAnimation
*
*	DESCRIPTION		Checks whether a specified animation is currently being played
*					Added JML for netcode
*
***************************************************************************************************/
bool CAnimator::IsPlayingAnimation( uint32_t uiAnimHash ) const
{
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		if ( m_Animations[ i ]->GetShortNameHash() == uiAnimHash )
		{
			return true;
		}
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::GetPlayingAnimation
*
*	DESCRIPTION		Get a pointer to an animation currently playing on this animator.
*					Added JML for netcode.
*
***************************************************************************************************/
CAnimationPtr CAnimator::GetPlayingAnimation( uint32_t uiAnimHash )
{
	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		if ( m_Animations[ i ]->GetShortNameHash() == uiAnimHash )
			return m_Animations[ i ];
	}
	
	return CAnimationPtr( NULL );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::DestroyAnimation
*
*	DESCRIPTION		Internal function used to safely remove (and destroy) an animation from the
*					current animator object, updating internal states as necessary.
*
***************************************************************************************************/
void CAnimator::DetachAnimation( const CAnimationPtr &pobAnimation )
{
	pobAnimation->SetActive( false );
}


/***************************************************************************************************
*
*	FUNCTION		CAnimator::CreateBlends
*
*	DESCRIPTION		Updates all animations for the current animator object in priority order. Once
*					per-animation updates have been performed, the blending between animations (on
*					a per-transform level) is applied with resultant rotational & translational 
*					information being applied to the appropriate transforms. For locomoting 
*					animations, no modification of the root transform is applied. Instead deltas
*					are calculated that can be applied by the physics/dynamics system.
*
*	INPUTS			fTimeStep		-		Time step to apply to all animations
*
***************************************************************************************************/
//#define DEBUG_ANIM_COMMAND_LIST
#ifdef DEBUG_ANIM_COMMAND_LIST
#	define ANIM_DEBUG_OUT( v ) do { if ( has_interesting_anim ) ntPrintf v ; } while( 0 )
#else
#	define ANIM_DEBUG_OUT( v ) do {} while( 0 )
#endif

bool	CAnimator::CreateBlends( float fTimeStep, bool batched /*= false*/ )
{
#	ifdef DEBUG_ANIM_COMMAND_LIST
		bool has_interesting_anim = false;
#	endif
/*
#	ifndef DEBUG_ANIM_COMMAND_LIST
		bool has_interesting_anim = false;
#	endif

	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		if ( m_Animations[ i ]->GetShortNameHash() == 0xcc389a3b )
		{
			has_interesting_anim = true;
			int32_t blah = 0;
			UNUSED( blah );
		}
	}

	if ( m_pobHierarchy->GetHierarchyKey() == 0x496cc1f1 )//&& has_interesting_anim )
	{
		CPoint wpos = m_pobHierarchy->GetRootTransform()->GetWorldTranslation();
		ntPrintf( "Current heroine world pos = %.3f, %.3f, %.3f\n\n", wpos.X(), wpos.Y(), wpos.Z() );

		if ( wpos.Y() > -0.1f )
		{
			int blah( 0 );
			UNUSED( blah );
		}
	}
//*/
	if ( !m_bEnabled )
	{
		return false;
	}

	// Simple anims can't be phase-linked or anything clever like that...
	if ( ( GetFlags() & ANIMATORF_SIMPLE_ANIMATOR ) == 0 )
	{
		//
		//	Phase linking update. Always done on PPU.
		//
		float	fLinkedWeightSum		= 0.0f;
		float	fLinkedPositionSum		= 0.0f;
		float	fLinkedSpeedSum			= 0.0f;
		float	fLinkedRemainingWeight	= 1.0f;
		int		iLinkedAnimationCount	= 0;

		float	fFirstPhasePosition		= 0.0f;
		bool	bFoundFirstPhase		= false;
		
		// We need to run through the list of animations, updating times & animation-related blend weight information.
		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			CAnimationPtr anim = m_Animations[ i ];

			if ( !( anim->GetFlags() & ANIMF_PHASE_LINKED ) ) 
			{
				anim->UpdateTime( fTimeStep );
			}
			else
			{
				// Increase the phase linked anim count
				iLinkedAnimationCount++;

				// If the phase has been overridden then the animation must be treated as on its first frame
				if ( m_bOverridePhase )
				{
					anim->SetInitialUpdate();
				}

				if ( !bFoundFirstPhase )
				{
					fFirstPhasePosition = anim->GetPhasePosition();
					bFoundFirstPhase = true;
				}

				anim->InitialLinkageUpdate( fTimeStep, fLinkedWeightSum, fLinkedPositionSum, fLinkedSpeedSum, fLinkedRemainingWeight );
			}
		}

		// If we have no phase linked animations reset the current phase
		if ( !bFoundFirstPhase )
		{
			m_fPhasePosition = 0.0f;
		}

		// If we have more than one linked animation, then we've got work to do.
		if ( iLinkedAnimationCount > 0 ) 
		{
			if ( m_bOverridePhase )
			{
			}
			else if ( fLinkedWeightSum > EPSILON )
			{
				m_fPhasePosition = ( fLinkedPositionSum / fLinkedWeightSum ) + ( ( fLinkedSpeedSum / fLinkedWeightSum ) * fTimeStep );
			}
			else
			{
				m_fPhasePosition = fFirstPhasePosition;
			}

			for ( uint32_t iLinkageLoop = 0; iLinkageLoop < m_NumAnimations; iLinkageLoop++ )
			{
				if ( m_Animations[ iLinkageLoop ]->GetFlags() & ANIMF_PHASE_LINKED )	
				{
					m_Animations[ iLinkageLoop ]->ApplyLinkageChange( m_fPhasePosition );
				}
			}

#			ifdef _DEBUG
				m_fDebugLastFramePhase = m_fPhasePosition;
#			endif
		}

		if ( m_bOverridePhase )
		{
			m_bOverridePhase = false;
		}
	}
	else
	{
		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			m_Animations[ i ]->UpdateTime( fTimeStep );
		}
	}









	// Make sure our deltas are wiped clean..
	m_obRootRotationDelta.SetIdentity();
	m_obRootTranslationDelta.Clear();

	// Set up the details for relative movement.
	m_obRelativeMovementPos.Clear();
	m_obRelativeMovementRot.SetIdentity();
	m_bRelativeMovement = false;
	m_fRelativeMovementWeight = 0.0f;

	// We initially say that we've not got a valid delta to apply..
	ClearFlagBits( ANIMATORF_HAS_DELTAS_AVAILABLE );

	// If we don't have any animations playing, then we might as well leave..
	if ( m_NumAnimations == 0 )
	{
		return false;;
	}
	


	//
	//	Work out whether we should disable the locomoting flag.
	//
	const GpJoint &local_space = m_pobHierarchy->GetRootTransform()->GetLocalSpace();
	m_LocalRootRotation = CQuat( local_space.m_rotation.QuadwordValue() );
	m_LocalRootTranslation = CPoint( local_space.m_translation.QuadwordValue() );
	int32_t ignore_locomtion = 0;
	if ( ( m_Flags & ANIMATORF_SIMPLE_ANIMATOR ) )
	{
//		ignore_locomtion = GpAnimator::kDisableLocomotion;
	}
	else
	{
		// If any of our animations are locomoting then we need to have locomotion info.
		uint32_t i = 0;
		for ( ;i<m_NumAnimations;i++ )
		{
			CAnimationPtr anim = m_Animations[ i ];

			// If the animation is blended in *and* it is marked as being either locomoting or having relative movement then break.
			if ( anim->GetBlendWeight() > 0.0f )
			{
				if ( anim->GetFlags() & ANIMF_LOCOMOTING )
				{
					break;
				}

				if ( anim->GetFlags() & ANIMF_RELATIVE_MOVEMENT )
				{
					ignore_locomtion = GpAnimator::kDisableLocomotion;
					break;
				}
			}
		}

		if ( i == m_NumAnimations )
		{
			// We have no locomoting anims.
			ignore_locomtion = GpAnimator::kDisableLocomotion;
		}
	}




	//
	//	Update all our animations.
	//
	for ( uint32_t anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
	{
		m_Animations[ anim_idx ]->Update();
	}



	//
	//	Work out and cache all of our animation local blend weights.
	//
	float local_blend_weights[ MaxNumAnims ];		// Local blend weight accounting for blends gone beforehand.
	float total_partial_anim_blend_weight = 0.0f;	// The sum of the blend-weights of all partial anims.
	bool has_some_blend_weight = false;
	
	{
		float work_blend_factor = 0.0f;

		//
		//	Calculate partial anim blend-weights first.
		//
		uint32_t anim_idx = 0;
		for ( ;anim_idx<m_NumAnimations;anim_idx++ )
		{
			CAnimationPtr anim = m_Animations[ anim_idx ];

			if ( anim->m_Priority == 0 )
			{
				break;
			}

			float blend_weight = anim->GetBlendWeight();
			local_blend_weights[ anim_idx ] = 0.0f;
			if ( blend_weight > EPSILON )
			{
				local_blend_weights[ anim_idx ] = blend_weight;// / ( blend_weight + work_blend_factor );
			}

			total_partial_anim_blend_weight += blend_weight;
			work_blend_factor += blend_weight;
		}

		total_partial_anim_blend_weight = Clamp( total_partial_anim_blend_weight, 0.f, 1.f );

		if ( work_blend_factor > EPSILON )
		{
			has_some_blend_weight = true;
		}

		// Reset the work blend factor.
		work_blend_factor = 0.0f;

		//
		//	Now update the non-partial anims.
		//
		for ( ;anim_idx<m_NumAnimations;anim_idx++ )
		{
			CAnimationPtr anim = m_Animations[ anim_idx ];

			//
			//	Work out the blend-weights.
			//
			float blend_weight = 0.0f;
			if ( anim_idx == m_NumAnimations-1 )
			{
				// Last animation gets remaining blend-weight.
				if ( work_blend_factor < EPSILON && anim->GetBlendWeight() < EPSILON )
				{
					blend_weight = 0.0f;
				}
				else
				{
					blend_weight = 1.0f - work_blend_factor;
				}
			}
			else
			{
				blend_weight = anim->GetBlendWeight();
			}

			local_blend_weights[ anim_idx ] = 0.0f;
			if ( blend_weight > EPSILON )
			{
				local_blend_weights[ anim_idx ] = blend_weight / ( blend_weight + work_blend_factor );
			}

			//
			//	Update the working blend factor.
			//
			work_blend_factor += blend_weight;
		}

		if ( work_blend_factor > EPSILON )
		{
			has_some_blend_weight = true;
		}
	}

	// If we have no anims, or all our anims have zero blend-weights then don't continue
	// with the update - there's not really much point.
	if ( !has_some_blend_weight )
	{
		return false;
	}



	// Debug code to check for missing animations.
#	ifndef _RELEASE
	{
		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			CAnimationPtr anim = m_Animations[ i ];
			if ( CAnimLoader::Get().IsErrorAnim( anim->GetAnimationHeader() ) && anim->GetBlendWeight() > 0.0f )
			{
				ntPrintf( "MISSING ANIMATION: %s\n", m_AnimShortCutContainer->GetAnimName( anim->GetAnimationHeader() ) );
			}
		}
	}
#	endif // !_RELEASE



	//
	//	This is the main update loop. Animations are evaluated and blended here.
	//
	int32_t	last_output_slot	= -1;
	int32_t	eval_slot			= 2;
	int32_t	active_anim_count	= 0;
	int32_t last_partial_anim_output_slot = -1;

	{
		uint32_t anim_idx;

		// Start up the animator command list.
		m_AnimatorData->StartCommandList();
		ANIM_DEBUG_OUT( ("StartCommandList()\n") );

		//
		//	Any zero-weight, phase-linked, anims need to be updated but not blended
		//	because we need the locomotion information to be update.
		//
		for ( anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
		{
			if ( local_blend_weights[ anim_idx ] == 0.0f && ( m_Animations[ anim_idx ]->GetFlags() & ANIMF_PHASE_LINKED ) )
			{
				CAnimationPtr anim = m_Animations[ anim_idx ];
				m_AnimatorData->AddCmdEvaluate( 0, anim->m_AnimationData, anim->GetTime(), anim->m_LoopCount, ignore_locomtion );
			}
		}

		//
		//	Update all the partial anims.
		//
		for ( anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
		{
			CAnimationPtr anim = m_Animations[ anim_idx ];

			if ( anim->m_Priority == 0 )
			{
				break;
			}

			if ( local_blend_weights[ anim_idx ] > 0.0f )
			{
				m_AnimatorData->AddCmdEvaluate( eval_slot, anim->m_AnimationData, anim->GetTime(), anim->m_LoopCount, ignore_locomtion );
				ANIM_DEBUG_OUT( ("AddCmdEvaluate( %i, ANIM[0x%08X, %s], %f, %i, %i )\n", eval_slot, anim->GetShortNameHash(), m_AnimShortCutContainer->GetAnimName( anim->GetAnimationHeader() ), anim->GetTime(), anim->m_LoopCount, ignore_locomtion) );

				if ( active_anim_count > 0 )
				{
					ntError( last_partial_anim_output_slot >= 0 );
					m_AnimatorData->AddCmdBlend( eval_slot, last_partial_anim_output_slot, eval_slot, local_blend_weights[ anim_idx ] );
					ANIM_DEBUG_OUT( ("AddCmdBlend( %i, %i, %i, %f )\n", eval_slot, last_partial_anim_output_slot, eval_slot, local_blend_weights[ anim_idx ]) );
				}

				// Keep track of the last output slot and flip-flop which slot we use for evaluating animations.
				last_partial_anim_output_slot = eval_slot;

				if ( eval_slot == 2 )
					eval_slot = 3;
				else
					eval_slot = 2;

				active_anim_count++;
			}
		}

		// Reset the work blend factor and other variables.
		eval_slot			= 0;
		active_anim_count	= 0;

		//
		//	Now update the non-partial anims.
		//
		for ( ;anim_idx<m_NumAnimations;anim_idx++ )
		{
			CAnimationPtr anim = m_Animations[ anim_idx ];

			//
			//	Now blend the next animation into the result of the previous ones.
			//
			if ( local_blend_weights[ anim_idx ] > 0.0f )
			{
				m_AnimatorData->AddCmdEvaluate( eval_slot, anim->m_AnimationData, anim->GetTime(), anim->m_LoopCount, ignore_locomtion );
				ANIM_DEBUG_OUT( ("AddCmdEvaluate( %i, ANIM[0x%08X, %s], %f, %i, %i )\n", eval_slot, anim->GetShortNameHash(), m_AnimShortCutContainer->GetAnimName( anim->GetAnimationHeader() ), anim->GetTime(), anim->m_LoopCount, ignore_locomtion) );

				if ( active_anim_count > 0 )
				{
					ntError( last_output_slot >= 0 );
					m_AnimatorData->AddCmdBlend( eval_slot, last_output_slot, eval_slot, local_blend_weights[ anim_idx ] );
					ANIM_DEBUG_OUT( ("AddCmdBlend( %i, %i, %i, %f )\n", eval_slot, last_output_slot, eval_slot, local_blend_weights[ anim_idx ]) );
				}

				// Keep track of the last output slot and flip-flop which slot we use for evaluating animations.
				last_output_slot = eval_slot;
				eval_slot = eval_slot ^ 1;
				active_anim_count++;
			}
		}

		// Do we need to blend in the result of partial anims?
		if ( last_partial_anim_output_slot != -1 )
		{
			if ( last_output_slot != -1 )
			{
				//
				// We have both partial and non-partial anims evaluated.
				// We blend the non-partial anims into the partial anims. Non-partial anims will have a weight of zero,
				// partial anims a weight of one. This works because the partial anims will only affect a subset of the 
				// joints on this skeleton - these will be blended at 100% of the partial anim and so won't be affected
				// by the non-partial anim. The joints that aren't in the partial-anim will take 100% of the non-partial
				// anim result as the blend command is defined in this way.
				//
				//	Look at "IMPORTANT note" here:
				//	http://ship.scea.com/confluence/display/SCEEATGDOCS/Runtime+API+Overview+-+GpAnimator#RuntimeAPIOverview-GpAnimator-commandblend
				//
//				m_AnimatorData->AddCmdBlend( last_output_slot, last_partial_anim_output_slot, last_output_slot, 0.0f /*1.0f - total_partial_anim_blend_weight*/ );
				m_AnimatorData->AddCmdBlend( last_output_slot, last_partial_anim_output_slot, last_output_slot, 1.0f - total_partial_anim_blend_weight );
				m_AnimatorData->AddCmdOutput( last_output_slot );

				ANIM_DEBUG_OUT( ("AddCmdBlend( %i, %i, %i, %i )\n", last_output_slot, last_partial_anim_output_slot, last_output_slot, 0.0f/*1.0f - total_partial_anim_blend_weight*/) );
				ANIM_DEBUG_OUT( ("AddCmdOutput( %i )\n", last_output_slot) );
			}
			else
			{
				// Weird case. Probably never happens. We have partial-anims, but no non-partial anims.
				m_AnimatorData->AddCmdOutput( last_partial_anim_output_slot );
				ANIM_DEBUG_OUT( ("AddCmdOutput( %i )\n", last_partial_anim_output_slot) );
			}
		}
		else
		{
			// If we did some work then put it back into our hierarchy.
			if ( last_output_slot >= 0 )
			{
				m_AnimatorData->AddCmdOutput( last_output_slot );
				ANIM_DEBUG_OUT( ("AddCmdOutput( %i )\n", last_output_slot) );
			}
		}

		// Finished ATG update.
		m_AnimatorData->EndCommandList();
		ANIM_DEBUG_OUT( ("EndCommandList()\n") );

		if ( !batched )
		{
#			ifdef PLATFORM_PS3
				m_AnimatorData->StartUpdate( GpAnimator::kRunOnSPU, Exec::GetWWSJobList(), Exec::NumberOfSPUsInSpurs() );
				ANIM_DEBUG_OUT( ("StartUpdate( kRunOnSPU, wws job list, num spus )\n") );
//				m_AnimatorData->StartUpdate( GpAnimator::kRunOnPPU );
//				ANIM_DEBUG_OUT( ("StartUpdate( kRunOnPPU )\n") );
#			else
				m_AnimatorData->StartUpdate( GpAnimator::kRunOnPPU );
				ANIM_DEBUG_OUT( ("StartUpdate( kRunOnPPU )\n") );
#			endif

			m_AnimatorData->WaitForUpdate();
			ANIM_DEBUG_OUT( ("WaitForUpdate()\n") );
		}
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::UpdateResults
*
*	DESCRIPTION		Update the animation just before the rendering gets done.
*
***************************************************************************************************/
void CAnimator::UpdateResults()
{
	if(!m_bEnabled)
	{
		return;
	}

	// We don't do any of this locomoting or bone-offset specialness for simple animators - you don't even get root-deltas... 
	if ( ( GetFlags() & ANIMATORF_SIMPLE_ANIMATOR ) == 0 )
	{
		bool locomoting = false;
		float work_blend_factor = 0.0f;

		for ( uint32_t anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
		{
			CAnimationPtr anim = m_Animations[ anim_idx ];

			float blend_weight = 0.0f;
			if ( anim_idx == m_NumAnimations-1 )
			{
				// Last animation gets remaining blend-weight.
				if ( work_blend_factor < EPSILON && anim->GetBlendWeight() < EPSILON )
				{
					blend_weight = 0.0f;
				}
				else
				{
					blend_weight = 1.0f - work_blend_factor;
				}
			}
			else
			{
				blend_weight = anim->GetBlendWeight();
			}

			//
			//	Evaluate speculative stuff - can i do some cunning locomotion variations. GILES.
			//
			if ( blend_weight > 0.0f && anim->GetFlags() & ANIMF_RELATIVE_MOVEMENT )
			{
				// Add on the weight of this animation
				m_fRelativeMovementWeight += anim->GetBlendWeight();
				ntError_p( m_fRelativeMovementWeight >= 0.0f && m_fRelativeMovementWeight <= ( 1.0f + EPSILON ), ("Movement controller badness! The sum of the weights on relative animations is %.3f - which is bad.", m_fRelativeMovementWeight) );
#				ifndef _RELEASE
				{
/*					for ( uint32_t a=0;a<m_NumAnimations;a++ )
					{
						ntPrintf( "Relative anim info:\n" );
						CAnimationPtr curr_anim = m_Animations[ a ];
						if ( curr_anim->GetFlags() & ANIMF_RELATIVE_MOVEMENT )
						{
							ntPrintf( "Anim hash: 0x%08X, weight: %.3f\n", curr_anim->m_uiShortNameHash, curr_anim->GetBlendWeight() );
						}
					}*/
				}
#				endif

				// If this is our first relative movement animation - we take the position directly
				if ( !m_bRelativeMovement )
				{
					m_obRelativeMovementRot = anim->GetRootWorldOrientation( m_pobHierarchy->GetRootTransform() );
					m_obRelativeMovementPos = anim->GetRootWorldPosition( m_pobHierarchy->GetRootTransform() );
				}
				else
				{
					m_obRelativeMovementRot = CQuat::Slerp( m_obRelativeMovementRot, anim->GetRootWorldOrientation( m_pobHierarchy->GetRootTransform() ), ( anim->GetBlendWeight() / m_fRelativeMovementWeight ) );
					m_obRelativeMovementPos = CPoint::Lerp( m_obRelativeMovementPos, anim->GetRootWorldPosition( m_pobHierarchy->GetRootTransform() ), ( anim->GetBlendWeight() / m_fRelativeMovementWeight ) );
				}

				m_obRelativeMovementRot.Normalise();
				ntError( m_obRelativeMovementRot.IsNormalised() );

				// Make sure flags are set
				m_bRelativeMovement = true;
			}
			else if ( anim->GetFlags() & ANIMF_LOCOMOTING )
			{
				locomoting = true;
			}

			work_blend_factor += blend_weight;
		}

		if ( m_bRelativeMovement )
		{
			// Restore the root hierarchy if required.
			m_pobHierarchy->GetRootTransform()->SetLocalSpace( m_LocalRootRotation, m_LocalRootTranslation );
		}




		//
		//	We now need to loop through again and finalise any locomotion data that we've calculated.
		//
		for ( uint32_t anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
		{
//			if ( m_Animations[ anim_idx ]->GetFlags() & ANIMF_LOCOMOTING )
			{
				m_Animations[ anim_idx ]->CalcLocomotion();
			}

			// Mark this animation as having been updated at least once already.
			m_Animations[ anim_idx ]->m_bInitialUpdate = false;
			m_Animations[ anim_idx ]->m_AddedToAnimatorThisFrame = false;
		}





		//
		//	This section of code deals with adding bind-pose offsets onto the animation.
		//	This if rather slow as we have to touch every transform for each entity so we don't really want to do this in the long-run.
		//	NOTE: If we decide to remove this section then we still need to update the root-rotation/translation-deltas!
		//

//		static_assert_in_class( ROOT_TRANSFORM == 0, Must_be_zero_for_the_following_code_to_work );	// Only required if we do bone-offset funkiness.

		if ( locomoting )
		{
			// Locomoting anims don't update the root in the hierarchy. They only update our deltas.
			m_obRootRotationDelta		= CQuat( m_AnimatorData->GetRootRotationDelta().QuadwordValue() );
			m_obRootTranslationDelta	= CPoint( m_AnimatorData->GetRootTranslationDelta().QuadwordValue() );	// We don't use the root offset.. 
			SetFlagBits( ANIMATORF_HAS_DELTAS_AVAILABLE );
		}
		else if ( m_bRelativeMovement )
		{
		}
		else
		{
			// Set the local space transform to be the matrix formed by the quaternion & translation.
			CPoint new_local_translation = m_pobHierarchy->GetTransform( ROOT_TRANSFORM )->GetLocalTranslation() + m_pobHierarchy->GetBoneOffsetForJoint( ROOT_TRANSFORM );
			m_pobHierarchy->GetTransform( ROOT_TRANSFORM )->SetLocalTranslation( new_local_translation );
		}
/*
		for ( int32_t transform_idx=1;transform_idx<m_pobHierarchy->GetTransformCount();transform_idx++ )
		{
			// Set the local space transform to be the matrix formed by the quaternion & translation.
			CPoint new_local_translation = m_pobHierarchy->GetTransform( transform_idx )->GetLocalTranslation() + m_pobHierarchy->GetBoneOffsetForJoint( transform_idx );
			m_pobHierarchy->GetTransform( transform_idx )->SetLocalTranslation( new_local_translation );
		}
*/
/*
		for ( int32_t transform_idx=0;transform_idx<m_pobHierarchy->GetTransformCount();transform_idx++ )
		{
			if ( transform_idx == ROOT_TRANSFORM && locomoting )
			{
				// Locomoting anims don't update the root in the hierarchy. They only update our deltas.
				if ( m_AnimatorData->GetFlags() & GpAnimator::kDeltasAvailable )
				{
					m_obRootRotationDelta		= CQuat( m_AnimatorData->GetRootRotationDelta().QuadwordValue() );
					m_obRootTranslationDelta	= CPoint( m_AnimatorData->GetRootTranslationDelta().QuadwordValue() );	// We don't use the root offset...
					SetFlagBits( ANIMATORF_HAS_DELTAS_AVAILABLE );
				}
			}
			else if ( transform_idx == ROOT_TRANSFORM && m_bRelativeMovement )
			{
			}
			else
			{
				// Set the local space transform to be the matrix formed by the quaternion & translation.
				CPoint new_local_translation = m_pobHierarchy->GetTransform( transform_idx )->GetLocalTranslation() + m_pobHierarchy->GetBoneOffsetForJoint( transform_idx );
				m_pobHierarchy->GetTransform( transform_idx )->SetLocalTranslation( new_local_translation );
			}
		}
*/
	}
	else
	{
		// This flag is normally cleared in CAnimation::CalcLocomotion, but we don't want to do everything
		// else in that function within a simple animator, so we just clear it here. Dirty, but quick.
		for ( uint32_t anim_idx=0;anim_idx<m_NumAnimations;anim_idx++ )
		{
			m_Animations[ anim_idx ]->m_bInitialUpdate = false;
			m_Animations[ anim_idx ]->m_AddedToAnimatorThisFrame = false;
		}
	}




/*
	if ( has_interesting_anim )
	{
		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			m_Animations[ i ]->TestFunc( m_pobHierarchy );
		}

		ntPrintf( "Animator, root_delta: %.3f, %.3f, %.3f, rel_pos: %.3f, %.3f, %.3f\n",
					m_obRootTranslationDelta.X(), m_obRootTranslationDelta.Y(), m_obRootTranslationDelta.Z(),
					m_obRelativeMovementPos.X(), m_obRelativeMovementPos.Y(),m_obRelativeMovementPos.Z() );

		Transform *xform = m_pobHierarchy->GetTransform( 1 );
		CQuat rot = xform->GetLocalRotation();
		CPoint pos = xform->GetLocalTranslation();
		ntPrintf(	"Animator, joint 1 local rot: %.3f, %.3f, %.3f, %.3f, local pos: %.3f, %.3f, %.3f\n",
					rot.X(), rot.Y(), rot.Z(), rot.W(), pos.X(), pos.Y(), pos.Z() );
		rot = xform->GetWorldRotation();
		pos = xform->GetWorldTranslation();
		ntPrintf(	"Animator, joint 1 world rot: %.3f, %.3f, %.3f, %.3f, world pos: %.3f, %.3f, %.3f\n",
					rot.X(), rot.Y(), rot.Z(), rot.W(), pos.X(), pos.Y(), pos.Z() );

		ntPrintf( "\n\n" );
	}
//*/





	// Now we've sorted everything out, we could do with deleting the animations that have expired.
	// We no longer actually delete the animations here, we just move them to the end of the array,
	// the animations are actually released in PreRenderUpdate.
	for ( uint32_t i=0;i<m_NumAnimations; )
	{
		if ( m_Animations[ i ]->ShouldDelete() )
		{
			ntError( m_Animations[ i ] != NULL );
			for ( uint32_t j=i;j<m_NumAnimations-1;j++ )
			{
				// Swap j and j+1.
				ntError( m_Animations[ j+1 ] != NULL );

				CAnimationPtr anim_temp = m_Animations[ j ];
				m_Animations[ j ] = m_Animations[ j+1 ];
				m_Animations[ j+1 ] = anim_temp;
			}

			m_NumAnimations--;
		}
		else
		{
			i++;
		}
	}





	// Delete our animations;
	for ( uint32_t i=m_NumAnimations;i<MaxNumAnims;i++ )
	{
		if ( m_Animations[ i ] == NULL )
		{
			// If we find a NULL pointer then there are no more anims beyond this.
			break;
		}

		// Send a message to the anim event handler
		if ( m_hasAnimEvents )
		{
			m_obAnimEventHandler.RemoveAnimation( m_Animations[ i ] );
		}

		DetachAnimation( m_Animations[ i ] );

		// Setting the anim to NULL is always valid and will automatically Release
		// the anim if required.
		m_Animations[ i ] = CAnimationPtr( NULL );
	}

	// Update our anim event handler
	if ( m_hasAnimEvents )
	{
		m_obAnimEventHandler.Update();
	}

	// Only do this if it's not a simple animator.
	if ( ( GetFlags() & ANIMATORF_SIMPLE_ANIMATOR ) == 0 )
	{
		// now see if our deltas should be applied explicity
		if	(
			( GetFlags() & ANIMATORF_HAS_DELTAS_AVAILABLE ) &&
			( GetFlags() & ANIMATORF_APPLY_DELTAS_EXPLICITY )
			)
		{
			CPoint obPos = m_pobHierarchy->GetRootTransform()->GetWorldMatrix().GetTranslation();
			obPos += m_obRootTranslationDelta * m_pobHierarchy->GetRootTransform()->GetWorldMatrix();

			CQuat obRot( m_pobHierarchy->GetRootTransform()->GetWorldMatrix() );
			obRot = obRot * m_obRootRotationDelta;

			m_pobHierarchy->GetRootTransform()->SetLocalSpace( obRot, obPos );
		}

		if ( m_AnimatorToClampTo != NULL )
		{
			for (	ChildToParentBoneMappingArray::const_iterator it = m_ChildToParentBoneMappingArray.begin();
					it != m_ChildToParentBoneMappingArray.end();
					++it )
			{
				ChildToParentBoneMappingArray::value_type bone_idx_pair = *it;

				CMatrix clamp_to_matrix = m_AnimatorToClampTo->GetHierarchy()->GetTransform( bone_idx_pair.second )->GetWorldMatrix();
				m_pobHierarchy->GetTransform( bone_idx_pair.first )->SetLocalMatrixFromWorldMatrix( clamp_to_matrix );
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::GetTrackingData
*
*	DESCRIPTION		Retrieves tracking data for the specified transform. Tracking data is formed
*					from up to 4 floating point values stored deep within animations. These values
*					are blended in the same way as any other curve based vector. The values can be
*					interpreted by the project in whatever way is useful. Note that the 4 values 
*					(held in the X, Y, Z, and W fields of the returned vector) will have specific 
*					meanings once this functionality is used.
*
*	INPUTS			iTransformIndex		-		Transform index (must be valid for the hierarchy)
*
***************************************************************************************************/
CVector CAnimator::GetTrackingData( int32_t iTransformIndex ) const
{
	ntError( ( iTransformIndex >= 0 ) && ( iTransformIndex <= m_pobHierarchy->GetTransformCount() ) );

	FwHashedString joint_name = m_pobHierarchy->GetGpSkeleton()->GetJointName( iTransformIndex );

	const FwVector4 *user_channel = m_HasUserBindings ? m_AnimatorData->GetUserChannel( joint_name, AnimNames::tracking ) : NULL;
	if ( user_channel == NULL )
		return CVector( CONSTRUCT_CLEAR );

	return CVector( user_channel->QuadwordValue() );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::GetPredominantAnim
*
*	DESCRIPTION		Returns the pointer to the more influential animation in the animator's list.
*					Useful if we want to query an entity to find out what they are doing.
*
*	INPUTS			None.
*
***************************************************************************************************/
const CAnimationPtr CAnimator::GetPredominantAnim () const
{
	if ( m_NumAnimations > 0 )
	{
		CAnimationPtr pobAnimation;
		float fWeight = 0.0f;

		for ( uint32_t i=0;i<m_NumAnimations;i++ )
		{
			CAnimationPtr anim = m_Animations[ i ];

			float fBlendWeight = anim->GetBlendWeight();

			if ( fBlendWeight > fWeight )
			{
				fWeight = fBlendWeight;
				pobAnimation = anim;
			}
		}

		return pobAnimation;
	}

	return CAnimationPtr();
}

/***************************************************************************************************
*
*	FUNCTION		CAnimator::DebugPrint
*
*	DESCRIPTION		Dump debugging info.
*
*	INPUTS			None.
*
***************************************************************************************************/
void CAnimator::DebugPrint() const
{
#ifndef _RELEASE

	ntPrintf( "CAnimator: Animation list for entity %s\n", ntStr::GetString(m_AnimShortCutContainer->GetName()) );

	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		CAnimationPtr anim = m_Animations[ i ];

		CHashedString obAnimName("");
		ntPrintf("    %-24s  Priority=%d  Time=%0.3f/%0.3f (%0.2f)  Weight=%0.3f  %s\n",
			ntStr::GetString(obAnimName),
            anim->GetPriority(),
			anim->GetTime(),
			anim->GetDuration(),
			anim->GetTime()/anim->GetDuration(),
			anim->GetBlendWeight(),
			anim->IsActive() ? "ACTIVE" : "INACTIVE");
	}

#endif // _RELEASE
}


/***************************************************************************************************
*
*	FUNCTION		CAnimator::DebugRenderPrint
*
*	DESCRIPTION		Dump debugging info.
*
*	INPUTS			None.
*
***************************************************************************************************/
void CAnimator::DebugRenderPrint( float fX, float fY, uint32_t ulColour ) const
{
	UNUSED(fX);
	UNUSED(fY);
	UNUSED(ulColour);
#ifndef _RELEASE
	float fOffsetY = 0.0f;
	char acString[ 1024 ];

	for ( uint32_t i=0;i<m_NumAnimations;i++ )
	{
		CAnimationPtr anim = m_Animations[ i ];

		sprintf	(		
						acString,
						"0x%08X: %-32s %.3fs/%.3fs (%.3f) W=%.3f",
						anim->GetShortNameHash(),
						m_AnimShortCutContainer->GetAnimName( anim->GetAnimationHeader() ),
						anim->GetTime(),
						anim->GetDuration(),
						anim->GetTime()/anim->GetDuration(),
						anim->GetBlendWeight()
			   	);

		if ( CAnimLoader::Get().IsErrorAnim( anim->GetAnimationHeader() ) )
		{
			strcat( acString, " - MISSING ANIM" );
		}

		g_VisualDebug->Printf2D( fX, fY + fOffsetY, ulColour, 0, acString );

		fOffsetY += 10.0f;
	}
#endif
}


//
//
//	XXX Animator batch update functions - TODO: Move to separate file.
//
//
#define USE_ANIM_BATCH_UPDATE
void AnimatorBatchUpdate::StartBatch()
{
#	ifdef USE_ANIM_BATCH_UPDATE
	{
		for ( uint32_t i=0;i<NumBatches;i++ )
		{
			m_AnimBatcher[ i + m_BatchOffset ].StartBatch();
		}
	}
#	endif
}

void AnimatorBatchUpdate::AddAnimator( CAnimator *animator, float time_step )
{
	ntError( animator != NULL );

#	ifdef USE_ANIM_BATCH_UPDATE
	{
		// Create the command list for the GpAnimator but don't
		// kick it off as we're running in batched mode.
		if ( animator->CreateBlends( time_step, true ) )
		{
			// Tell the batcher we want to batch update this GpAnimator.
			ntError( m_CurrentBatch < NumBatches );
			ntError( m_CurrentBatch + m_BatchOffset < 2 * NumBatches );
			m_AnimBatcher[ m_CurrentBatch + m_BatchOffset ].AddAnimator( animator->GetGpAnimator() );
			m_CurrentBatch = ( m_CurrentBatch + 1 ) % NumBatches;

			// If we've added the required number of animators then
			// start this set of batches off on the SPUs and move the
			// offset onto the next batch-set.
			m_Counter++;
			if ( m_Counter >= m_BatchSize*NumBatches )
			{
				m_Counter = 0;

				CreateBlends();

				// Ping-pong between batch-sets.
				m_BatchOffset = m_BatchOffset == 0 ? NumBatches : 0;

				// If it needs it...
				if ( m_NumSetSwaps > 0 )
				{
					// ...finalise the batch we've just moved onto so we can go again.
					InternalFinishBatch();
				}

				StartBatch();

				m_NumSetSwaps++;
			}
		}
	}
#	else
	{
		// Not running in batched mode so just do a standard CreateBlends update.
		animator->CreateBlends( time_step, false );
	}
#	endif
}

void AnimatorBatchUpdate::CreateBlends()
{
#	ifdef USE_ANIM_BATCH_UPDATE
	{
		for ( uint32_t i=0;i<NumBatches;i++ )
		{
			uint32_t idx = i + m_BatchOffset;

			// We've stopped adding animators to this batch so let the batcher know.
			m_AnimBatcher[ idx ].EndBatch();

			// Kick off the batch - run on an SPU for PS3 and on the main-processor for PC.
#			ifdef PLATFORM_PS3
				m_AnimBatcher[ idx ].StartUpdate( GpAnimator::kRunOnSPU, Exec::GetWWSJobList(), Exec::NumberOfSPUsInSpurs() );
#			else
				m_AnimBatcher[ idx ].StartUpdate( GpAnimator::kRunOnPPU );
#			endif
		}
	}
#	endif
}

void AnimatorBatchUpdate::InternalFinishBatch()
{
#	ifdef USE_ANIM_BATCH_UPDATE
	{
		for ( uint32_t i=0;i<NumBatches;i++ )
		{
			// Make sure we've finished the batch before continuing.
			m_AnimBatcher[ i + m_BatchOffset ].WaitForUpdate();
		}
	}
#	endif
}

void AnimatorBatchUpdate::FinishBatch()
{
#	ifdef USE_ANIM_BATCH_UPDATE
	{
		// If we swapped batch-set at least once...
		if ( m_NumSetSwaps > 0 )
		{
			m_BatchOffset = m_BatchOffset == 0 ? NumBatches : 0;

			// ...then we might need to wait to make sure the SPUs are completely done.
			InternalFinishBatch();

			// Swap back to the previous batch-set and finish that off.
			m_BatchOffset = m_BatchOffset == 0 ? NumBatches : 0;
		}

		CreateBlends();
		InternalFinishBatch();
	}
#	endif
}

AnimatorBatchUpdate::AnimatorBatchUpdate( uint32_t batch_size /*= 20*/ )
:	m_BatchSize		( batch_size )
,	m_CurrentBatch	( 0 )
,	m_BatchOffset	( 0 )
,	m_Counter		( 0 )
,	m_NumSetSwaps	( 0 )
{}











