//--------------------------------------------------
//!
//!	\file aiwander.cpp
//!	Wander behaviour.
//!
//--------------------------------------------------

#include "aistatemachine.h"
#include "ainavgraphmanager.h"
#include "ainavnodes.h"

// XXX: remove these!!
//--------------------------------------------------
#include "aipatrolmanager.h"
#include "aipatrolpath.h"
#include "aipatrolnode.h"
//--------------------------------------------------

#include "game/aicomponent.h"
#include "game/randmanager.h"

#include "aiwander.h"


//--------------------------------------------------
//!
//!	AIWander::Initialise
//!
//--------------------------------------------------

void
AIWander::Initialise( void )
{
	// define a version of Initialise for the wander behaviour,
	// so that it gets called instead of the one in patrol
}


//--------------------------------------------------
//!
//!	AIWander::WalkInitialise
//!
//--------------------------------------------------

void
AIWander::WalkInitialise( void )
{
	m_obDest = CAINavGraphManager::Get().GetRandomReachablePosInGraph( m_pobEnt->GetPosition() );

	// drop a ray straight down from this point, and set the dest to be the point of intersection
	CAINavGraphManager::ClampPointToFloor( m_obDest );

	if (!CAINavGraphManager::Get().InGraph( m_obDest ))
		m_obDest = m_pobEnt->GetPosition();

	// set the action to walk. in theory, this is only being done from the behaviours,
	m_pobAIComp->SetAction( ACTION_WALK );
	m_pobAIComp->SetActionMoveSpeed( 0.5f );

	// give the action a destination
	m_pobAIComp->SetActionDest( m_obDest );
}


//--------------------------------------------------
//!
//!	AIWander::Walk
//!	Checks distance from current dest, and picks new
//! dest when the threshold distance is reached
//!
//--------------------------------------------------

void
AIWander::Walk( void )
{
	// Am i at the dest node
	CPoint obPos( m_pobEnt->GetPosition() );

	CPoint obDiff = obPos - m_obDest;
	obDiff.Y() = 0.0f;

	if (obDiff.LengthSquared() < 3.0f)
	{
		//WalkInitialise();
		SetState( STATE_LOOK );
	}
}


