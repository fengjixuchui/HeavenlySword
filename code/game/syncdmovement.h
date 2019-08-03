//------------------------------------------------------------------------------------------
//!
//!	\file syncdmovement.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_SYNCDMOVEMENT_H
#define	_SYNCDMOVEMENT_H

// Forward declarations
class CEntity;
class MovementControllerDef;
class Transform;
class SyncdMovement;

#include "game/movement.h"

//------------------------------------------------------------------------------------------
//!
//!	SyncdMovementDefinition
//!	Defines a syncronised movement set to take place between two or more entities.
//!
//------------------------------------------------------------------------------------------
class SyncdMovementDefinition
{
public:

	// Construction destruction
	SyncdMovementDefinition( void );
	~SyncdMovementDefinition( void );

	// Define the movement that needs to take place on the entities
	void SetMovement( CEntity* pobOriginator, const MovementControllerDef& obMovementDef, float fBlend, bool bChainMe, CMovement::DYNAMICS_MOVEMENT_MODE eMoveMode = CMovement::DMM_SOFT_RELATIVE );

	// Check that all is well before we use this item
	bool IsCorrectlySet( void ) const { return ( !m_obEntityMovements.empty() ); }

private:

	// This guy can do things that we wouldn't let just anyone do
	friend class SyncdMovement;

	// Helper function - to be called by our best friend
	void PushMovements( void ) const;

	// For storing movement with entities
	struct MovementSet
	{
		MovementSet( void ) : pobEntity( 0 ), pobControllerDef( 0 ), fBlend( 0.0f ), bChainMe( false ) {}

		CEntity*				pobEntity;
		MovementControllerDef*	pobControllerDef;
		float					fBlend;
		bool					bChainMe; // Chain this controller on rather than forcing it on?
		CMovement::DYNAMICS_MOVEMENT_MODE	eMoveMode;
	};

	// A list of all the movements for all our entities
	ntstd::List< MovementSet > m_obEntityMovements;
};


//------------------------------------------------------------------------------------------
//!
//!	SyncdMovement
//!	Used for coordinating syncronised movement on multiple entities.
//!
//------------------------------------------------------------------------------------------
class SyncdMovement : public Singleton< SyncdMovement >
{
public:

	// Construction destruction
	SyncdMovement( void );
	~SyncdMovement( void );

	// Some update action - doesn't need a time step for now
	void Update( void );

	// When syncronised movement is required - we ask this guy to do it
	// The definition should be in the heap - we'll clear it up
	void InitiateSyncronisedMovement( const SyncdMovementDefinition* pobDefinition );

private:

	// A list of the movements we need to kick off
	ntstd::List< const SyncdMovementDefinition* >	m_obMovementsToInitiate;

	// A structure for reference tracking transforms we own
	struct OwnedTransform
	{
		Transform*	pobTransform;
		int			iUsers;
	};
};

#endif // _SYNCDMOVEMENT_H
