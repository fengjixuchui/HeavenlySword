//------------------------------------------------------------------------------------------
//!
//!	\file archer3rdpController.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ARCHER_3RDP_CONTROLLER_H
#define	_ARCHER_3RDP_CONTROLLER_H

// Necessary includes
#include "movementcontrollerinterface.h"
#include "relativetransitions.h"
#include "game/entity.h"
#include "editable/flipflop.h"

class ThirdPersonAttackState;
class ThirdPersonAttackTransition;


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerTransDef
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAimControllerTransDef : public MovementControllerDef
{
public:

	// Construction
	ThirdPersonAimControllerTransDef( void );
	virtual ~ThirdPersonAimControllerTransDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ThirdPersonAimControllerTransDef( *this ); }

	// Pointer to the transition
	ThirdPersonAttackTransition*		m_pTransition;
};


//------------------------------------------------------------------------------------------
//!
//!	Archer3rdPControllerStart
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAimControllerTrans : public MovementController
{
public:

	// Construction destruction
	ThirdPersonAimControllerTrans( CMovement* pobMovement, const ThirdPersonAimControllerTransDef& obDefinition );
	virtual ~ThirdPersonAimControllerTrans( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:

	// A COPY of our defintion
	ThirdPersonAimControllerTransDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr m_obSingleAnimation;

	// Has a shot been fired?
	bool m_Fired;
	bool m_RequestFire;
};


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimControllerDef
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAimControllerDef : public MovementControllerDef
{
public:

	// Construction
	ThirdPersonAimControllerDef( void );
	virtual ~ThirdPersonAimControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW ThirdPersonAimControllerDef( *this ); }

	// Pointer the current node in the sequence
	const ThirdPersonAttackState*		m_pCurrentState;
};


//------------------------------------------------------------------------------------------
//!
//!	ThirdPersonAimController
//!
//------------------------------------------------------------------------------------------
class ThirdPersonAimController : public MovementController
{
	enum 
	{
		IDLE_FRONT,		IDLE_FRONT_UP,
		IDLE_LEFT,		IDLE_LEFT_UP,
		IDLE_RIGHT,		IDLE_RIGHT_UP,
		FIRE_FRONT,		FIRE_FRONT_UP,
		FIRE_LEFT,		FIRE_LEFT_UP,
		FIRE_RIGHT,		FIRE_RIGHT_UP,

		ANIMS_MAX,
	};	

public:

	// Construction destruction
	ThirdPersonAimController( CMovement* pobMovement, const ThirdPersonAimControllerDef& obDefinition );
	virtual ~ThirdPersonAimController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	
	const ThirdPersonAttackState* GetCurrentState(void) const { return m_obDefinition.m_pCurrentState; }

protected:

	// A COPY of our defintion
	ThirdPersonAimControllerDef m_obDefinition;

	// A pointer to our single animation
	CAnimationPtr	m_obAnims[ANIMS_MAX];

	// 
	CAnimationPtr	m_ReloadAnim;

	// Time in the state
	float m_StateTime;

	// The amount of time spent in the fire animation.
	float m_FireTime;
};


#endif // _Archer3rdPController_H
