//------------------------------------------------------------------------------------------
//!
//!	\file partialtest.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "partialtest.h"
#include "objectdatabase/dataobject.h"
#include "core/exportstruct_anim.h"
#include "anim/animator.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "inputcomponent.h"
#include "movement.h"
#include "Physics/advancedcharactercontroller.h"
#include "Physics/system.h"

START_STD_INTERFACE	( PartialTestDef )
		ISTRING ( PartialTestDef, FullAnimation )
		ISTRING ( PartialTestDef, PartialAnimation )
		IINT	( PartialTestDef, PartialPriority )
#ifndef _RELEASE
		DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	PartialTestDef::PartialTestDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
PartialTestDef::PartialTestDef( void )
:	m_obFullAnimation(),
	m_obPartialAnimation(),
	m_iPartialPriority( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	PartialTestDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* PartialTestDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) PartialTest( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	PartialTest::PartialTest
//!	Construction
//!
//------------------------------------------------------------------------------------------
PartialTest::PartialTest( CMovement* pobMovement, const PartialTestDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obFullAnimation(),
	m_obPartialAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	// Create our animations - full
	m_obFullAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obFullAnimation );
	m_obFullAnimation->SetBlendWeight( 0.0f );
	m_obFullAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obFullAnimation->SetPriority( 0 );

	// The partial animation
	m_obPartialAnimation = m_pobAnimator->CreateAnimation( obDefinition.m_obPartialAnimation );
	m_obPartialAnimation->SetBlendWeight( 0.0f );
	m_obPartialAnimation->SetFlagBits( ANIMF_LOOPING | ANIMF_INHIBIT_AUTO_DESTRUCT );
	m_obPartialAnimation->SetPriority( m_obDefinition.m_iPartialPriority );
}


//------------------------------------------------------------------------------------------
//!
//!	PartialTest::~PartialTest
//!	Destruction
//!
//------------------------------------------------------------------------------------------
PartialTest::~PartialTest( void )
{
	// Only remove the animations if we have done an update
	if ( !m_bFirstFrame )
	{
		m_pobAnimator->RemoveAnimation( m_obFullAnimation );
		m_pobAnimator->RemoveAnimation( m_obPartialAnimation );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	PartialTest::Update
//!
//------------------------------------------------------------------------------------------
bool PartialTest::Update(	float						fTimeStep, 
							const CMovementInput&		/* obMovementInput */,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState )
{
	// Add the animations on our first update
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obFullAnimation );
		m_pobAnimator->AddAnimation( m_obPartialAnimation );
	}

	// Set the blend weight of our animations - the full animation
	m_obFullAnimation->SetBlendWeight( m_fBlendWeight );

	// If the action button is pressed we give the partial animation the same weighting
	if ( ( m_pobMovement->GetParentEntity()->GetInputComponent() ) 
		 && 
		 ( m_pobMovement->GetParentEntity()->GetInputComponent()->GetVHeld() & ( 1 << AB_ACTION ) ) )
	{
		m_obPartialAnimation->SetBlendWeight( m_fBlendWeight );
	}

	// ...otherwise we set the blend value of the partial animation to zero
	else
	{
		m_obPartialAnimation->SetBlendWeight( 0.0f );
	}

	// Gravity setting
	ApplyGravity(true);
	
	// Remove the first frame flag if it is set
	if ( m_bFirstFrame )
		m_bFirstFrame = false;

	// We go on for ever - we will not finish
	return false;
}
