//------------------------------------------------------------------------------------------
//!
//!	\file animbuildtest.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "animbuildtest.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "game/entity.h"
#include "inputcomponent.h"
#include "movement.h"
#include "anim/AnimBuilder.h"

START_STD_INTERFACE	( AnimBuildTestDef )
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTestDef::AnimBuildTestDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimBuildTestDef::AnimBuildTestDef( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTestDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* AnimBuildTestDef::CreateInstance( CMovement* pobMovement ) const
{
	return FW_NEW AnimBuildTest( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTest::AnimBuildTest
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimBuildTest::AnimBuildTest( CMovement* pobMovement, const AnimBuildTestDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_popDynamicAnimationHeader( 0 ),
	m_obDynamicAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Build an animation that describes the current pose
	ntAssert( m_pobMovement->GetParentEntity()->GetHierarchy() );
	m_popDynamicAnimationHeader = AnimBuilder::CreateAnimation( *m_pobMovement->GetParentEntity()->GetHierarchy() );

	// Construct a usable animation
	m_obDynamicAnimation = CAnimation::Create( 0, m_popDynamicAnimationHeader, m_pobMovement->GetParentEntity()->GetHierarchy() );
	m_obDynamicAnimation->SetBlendWeight( 0.0f );
	m_obDynamicAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTest::~AnimBuildTest
//!	Destruction
//!
//------------------------------------------------------------------------------------------
AnimBuildTest::~AnimBuildTest( void )
{
	// If we have added our animation to the animator then we must take it away
	if ( !m_bFirstFrame )
		m_pobAnimator->RemoveAnimation( m_obDynamicAnimation );

	// Delete the animation header that we created
	AnimBuilder::DestroyAnimation( m_popDynamicAnimationHeader );
}


//------------------------------------------------------------------------------------------
//!
//!	AnimBuildTest::Update
//!
//------------------------------------------------------------------------------------------
bool AnimBuildTest::Update(	float						/* fTimeStep */, 
							const CMovementInput&		/* obMovementInput */,
							const CMovementStateRef&	/* obCurrentMovementState */,
							CMovementState&				/* obPredictedMovementState */ )
{

	// Add the animation on our first frame
	if ( m_bFirstFrame )
		m_pobAnimator->AddAnimation( m_obDynamicAnimation );

	// Set the blend weight to our blend weight
	m_obDynamicAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if requested
	//ApplyGravity( fTimeStep, obCurrentMovementState, obPredictedMovementState, 1.0f, true );
	
	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// We go on for ever - we will not finish
	return false;
}
