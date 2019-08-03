//------------------------------------------------------------------------------------------
//!
//!	\file syncdmovement.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "syncdmovement.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "movement.h"
#include "anim/animator.h"
#include "movementcontrollerinterface.h"
#include "anim/transform.h"

//------------------------------------------------------------------------------------------
//!
//!	SyncdMovementDefinition::SyncdMovementDefinition
//!	Construction
//!
//------------------------------------------------------------------------------------------
SyncdMovementDefinition::SyncdMovementDefinition( void )
:	m_obEntityMovements()
{
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovementDefinition::~SyncdMovementDefinition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SyncdMovementDefinition::~SyncdMovementDefinition( void )
{
	// We need to clean up all the copies of movement definitions we have made
	ntstd::List< MovementSet >::const_iterator obEnd = m_obEntityMovements.end();
	for ( ntstd::List< MovementSet >::const_iterator obIt = m_obEntityMovements.begin(); obIt != obEnd; ++obIt )
		NT_DELETE( ( *obIt ).pobControllerDef );
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovementDefinition::SetMovement
//!	Set the details for an entity that will be joining this action
//!
//------------------------------------------------------------------------------------------
void SyncdMovementDefinition::SetMovement(	CEntity*						pobAuxillary, 
											const MovementControllerDef&	obMovementDef,
											float							fBlend,
											bool							bChainMe,
											CMovement::DYNAMICS_MOVEMENT_MODE			eMoveMode )
{
	// Store the movement controller/entity information
	MovementSet obMovementSet;
	obMovementSet.pobControllerDef = obMovementDef.Clone();
	obMovementSet.pobEntity = pobAuxillary;
	obMovementSet.fBlend = fBlend;
	obMovementSet.bChainMe = bChainMe;
	obMovementSet.eMoveMode = eMoveMode;

	// Make sure we have some reasonable data
	ntAssert( obMovementSet.pobControllerDef );

	if ( obMovementSet.pobControllerDef )
		m_obEntityMovements.push_back( obMovementSet );
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovementDefinition::PushMovements
//!	Set the details for an entity that will be joining this action
//!
//------------------------------------------------------------------------------------------
void SyncdMovementDefinition::PushMovements( void ) const
{
	// Loop through all the movement sets and push them on to the relevant movement componenent
	ntstd::List< MovementSet >::const_iterator obEnd = m_obEntityMovements.end();
	for ( ntstd::List< MovementSet >::const_iterator obIt = m_obEntityMovements.begin(); obIt != obEnd; ++obIt )
	{
		if (!obIt->bChainMe)
		{
			// HACK HACK HACK - need to clear everything off the movement component and animator to get smooth change (since SCE animator)
			obIt->pobEntity->GetMovement()->ClearControllers();
			obIt->pobEntity->GetAnimator()->RemoveAllAnimations();
			obIt->pobEntity->GetAnimator()->ClearAnimWeights();
			obIt->pobEntity->GetMovement()->BringInNewController( *obIt->pobControllerDef, obIt->eMoveMode, obIt->fBlend );
		}
		else
			obIt->pobEntity->GetMovement()->AddChainedController( *obIt->pobControllerDef, obIt->eMoveMode, obIt->fBlend );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovement::SyncdMovement
//!	Construction
//!
//------------------------------------------------------------------------------------------
SyncdMovement::SyncdMovement( void )
:	m_obMovementsToInitiate()
{
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovement::~SyncdMovement
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SyncdMovement::~SyncdMovement( void )
{
	// We'll be using a bit of this
	using namespace ntstd;

	// If we have any defintions we have not used we need to free them 
	ntstd::List< const SyncdMovementDefinition* >::iterator obEnd = m_obMovementsToInitiate.end();
	for ( ntstd::List< const SyncdMovementDefinition* >::iterator obIt = m_obMovementsToInitiate.begin(); obIt != obEnd; ++obIt )
	{	
		// Delete the whole definition
		NT_DELETE( ( *obIt ) );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovement::Update
//!	This updates stuff.  I was thinking about calling it something different but this seemed
//! to sum up its functionality.  If anyone else has any better ideas please come and speak
//!	to me - but you better be tooled up, fool!
//!
//------------------------------------------------------------------------------------------
void SyncdMovement::Update( void )
{
	// Loop through our list of movements to initiate and do it
	ntstd::List< const SyncdMovementDefinition* >::iterator obEnd = m_obMovementsToInitiate.end();
	for ( ntstd::List< const SyncdMovementDefinition* >::iterator obIt = m_obMovementsToInitiate.begin(); obIt != obEnd; ++obIt )
	{
		// Push all the movement controllers onto the relevant entities movement components
		( *obIt )->PushMovements();

		// Kill this defintion
		NT_DELETE( *obIt );
	}

	// Clear the list
	m_obMovementsToInitiate.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovement::InitiateSyncronisedMovement
//!	
//!
//------------------------------------------------------------------------------------------
void SyncdMovement::InitiateSyncronisedMovement( const SyncdMovementDefinition* pobDefinition )
{
	// Make sure our data is sensible
	ntAssert( pobDefinition );
	ntAssert( pobDefinition->IsCorrectlySet() );

	// Just add the request to our list
	m_obMovementsToInitiate.push_back( pobDefinition );
}
