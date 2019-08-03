//------------------------------------------------------------------------------------------
//!
//!	\file catapultcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CATAPULTCONTROLLER_H
#define	_CATAPULTCONTROLLER_H

// Necessary includes
#include "movementcontrollerinterface.h"

//#include "relativetransitions.h"

#include "game/entity.h"

//Enum to track arm states

enum ARM_STATE {
	AS_EMPTY,
	AS_READY,
	AS_FIRE,
	AS_FIRING,
	AS_STOP,
	AS_RESET,
	AS_RESETING
};

typedef ntstd::Vector<ARM_STATE>::iterator StateIter;

//------------------------------------------------------------------------------------------
//!
//!	VehicleControllerDef
//!
//------------------------------------------------------------------------------------------
class VehicleControllerDef  : public MovementControllerDef
{
public:
	// Construction
	VehicleControllerDef( void );
	virtual ~VehicleControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

	float m_fMaxRotationPerSecond;
	bool m_bApplyGravity;
	CHashedString m_obMoveAnimName;
	float m_fMaxNormalPerSecond;
	float m_fMaxAcceleration;

	float m_fMaxHeightPerSecond;

	CEntity* m_pobHost;
};

//------------------------------------------------------------------------------------------
//!
//!	VehicleController
//!
//------------------------------------------------------------------------------------------
class VehicleController : public MovementController
{
public:

	// Construction destruction
	VehicleController( CMovement* pobMovement, const VehicleControllerDef& obDef );
	virtual ~VehicleController( void );

	// The major functionality of any movement constroller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	// A COPY of our defintion
	VehicleControllerDef m_obDefinition;

	CAnimationPtr m_obMoveAnimation;

	CDirection m_obLastHeading;

	float m_fLastSpeed;

	CQuat m_obLastRot;
	CQuat m_obAimRot;

	CDirection m_obLastNormal;
	CDirection m_obAimNormal;
};

//------------------------------------------------------------------------------------------
//!
//!	CatapultControllerDef
//!
//------------------------------------------------------------------------------------------
class CatapultControllerDef  : public VehicleControllerDef
{
public:
	// Construction
	CatapultControllerDef( void );
	virtual ~CatapultControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance( CMovement* pobMovement ) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const;

	ntstd::Vector<CHashedString, Mem::MC_ENTITY>	m_obFireAnimNameList;
	ntstd::Vector<CHashedString, Mem::MC_ENTITY>	m_obResetAnimNameList;

	ntstd::Vector<ARM_STATE>* m_pobStateList;
};

typedef ntstd::Vector<CHashedString, Mem::MC_ENTITY>::iterator HashIter;
typedef ntstd::Vector<CAnimationPtr, Mem::MC_ENTITY>::iterator AnimIter;

//------------------------------------------------------------------------------------------
//!
//!	CatapultController
//!
//------------------------------------------------------------------------------------------
class CatapultController : public VehicleController
{
public:

	// Construction destruction
	CatapultController( CMovement* pobMovement, const CatapultControllerDef& obDef );
	virtual ~CatapultController( void );

	// The major functionality of any movement controller
	virtual bool Update(	float						fTimeStep, 
							const CMovementInput&		obMovementInput,
							const CMovementStateRef&	obCurrentMovementState,
							CMovementState&				obPredictedMovementState );

protected:
	// A COPY of our defintion
	CatapultControllerDef m_obCatapultDefinition;

	ntstd::Vector<CAnimationPtr, Mem::MC_ENTITY>	m_aobFireAnimList;
	ntstd::Vector<CAnimationPtr, Mem::MC_ENTITY>	m_aobResetAnimList;
};

#endif // _CATAPULTCONTROLLER_H
