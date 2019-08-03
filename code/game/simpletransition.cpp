//------------------------------------------------------------------------------------------
//!
//!	\file simpletransition.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "simpletransition.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE	(SimpleTransitionDef)
	IBOOL		(SimpleTransitionDef, ApplyGravity)
	ISTRING		(SimpleTransitionDef, AnimationName)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	SimpleTransitionDef::SimpleTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleTimedTransitionDef::SimpleTimedTransitionDef( void )
:	m_bApplyGravity( true ),
	m_bLooping( false ),
	m_obAnimationName(),
	m_fTime( 1.0f ),
	m_fExtraMovementSpeed( 0.0f )
{
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* SimpleTimedTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SimpleTimedTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::SimpleTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleTimedTransition::SimpleTimedTransition( CMovement* pobMovement, const SimpleTimedTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	m_fTimeInTransition = 0.0f;

	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	int iFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT;

	if ( m_obDefinition.m_bLooping == true )
		iFlags |= ANIMF_LOOPING;

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlags( iFlags );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::~SimpleTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SimpleTimedTransition::~SimpleTimedTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool SimpleTimedTransition::Update(	float						fTimeStep, 
									const CMovementInput&		/* obMovementInput */,
									const CMovementStateRef&	obCurrentMovementState,
									CMovementState&				obPredictedMovementState )
{
	// If we are on the first frame - add the animation to the animator
	if ( m_bFirstFrame )
	{
		m_fTimeInTransition = 0.0f;
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_bFirstFrame = false;
	}

	m_fTimeInTransition += fTimeStep;

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	obPredictedMovementState.m_obProceduralRootDelta = obCurrentMovementState.m_obFacing * m_obDefinition.m_fExtraMovementSpeed * fTimeStep;

	// When we are finished indicate that to the movement component
	if ( m_fTimeInTransition > m_obDefinition.m_fTime )
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	SimpleTransitionDef::SimpleTransitionDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleTransitionDef::SimpleTransitionDef( void )
:	m_bApplyGravity( true ),
	m_bLooping( false ),
	m_obAnimationName(),
	m_fSpeed(1.f),
	m_fTimeOffsetPercentage(0.0f)
{
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransitionDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController* SimpleTransitionDef::CreateInstance( CMovement* pobMovement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) SimpleTransition( pobMovement, *this );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::SimpleTransition
//!	Construction
//!
//------------------------------------------------------------------------------------------
SimpleTransition::SimpleTransition( CMovement* pobMovement, const SimpleTransitionDef& obDefinition )
:	MovementController( pobMovement ),
	m_obDefinition( obDefinition ),
	m_obSingleAnimation()
{
	// Register our definition for debugging purposes
	InternalRegisterDefinition( m_obDefinition );

	int iFlags = ANIMF_LOCOMOTING | ANIMF_INHIBIT_AUTO_DESTRUCT;

	if ( m_obDefinition.m_bLooping == true )
		iFlags |= ANIMF_LOOPING;

	// Create our animation and add it to the animator
	m_obSingleAnimation = m_pobAnimator->CreateAnimation( m_obDefinition.m_obAnimationName );
	m_obSingleAnimation->SetBlendWeight( 0.0f );
	m_obSingleAnimation->SetFlags( iFlags );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::~SimpleTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SimpleTransition::~SimpleTransition( void )
{
	// Remove the animation from the animator if it has been added
	if ( !m_bFirstFrame && m_obSingleAnimation->IsActive() )
		m_pobAnimator->RemoveAnimation( m_obSingleAnimation );
}


//------------------------------------------------------------------------------------------
//!
//!	SimpleTransition::Update
//!	Simply move our way through the single animation
//!
//------------------------------------------------------------------------------------------
bool SimpleTransition::Update(	float						fTimeStep, 
								const CMovementInput&		/* obMovementInput */,
								const CMovementStateRef&	/*obCurrentMovementState*/,
								CMovementState&				/*obPredictedMovementState*/ )
{
	// If we are on the first frame - add the animation to the animator
	if ( m_bFirstFrame )
	{
		m_pobAnimator->AddAnimation( m_obSingleAnimation );
		m_obSingleAnimation->SetSpeed( m_obDefinition.m_fSpeed );
		m_obSingleAnimation->SetPercentage( m_obDefinition.m_fTimeOffsetPercentage );
		m_bFirstFrame = false;
	}

	// Set the weight on our animation and update it
	m_obSingleAnimation->SetBlendWeight( m_fBlendWeight );

	// Apply gravity if required
	ApplyGravity(m_obDefinition.m_bApplyGravity);

	// When we are finished indicate that to the movement component
	if ( m_obDefinition.m_bLooping == false )
	{
		if ( m_obSingleAnimation->GetTime() > ( m_obSingleAnimation->GetDuration() - fTimeStep ) )
			return true;
	}

	// This return mechanism needs to be used to move to the next controller in future
	return false;
}
