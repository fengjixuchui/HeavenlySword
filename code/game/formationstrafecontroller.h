//------------------------------------------------------------------------------------------
//!
//!	\file strafecontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_FORMATIONSTRAFECONTROLLER_INC
#define	_FORMATIONSTRAFECONTROLLER_INC

// Necessary includes
#include "movementcontrollerinterface.h"

// Forward declarations
class CMovement;
class CMovementState;

//------------------------------------------------------------------------------------------
//!
//!	StrafeAnimSet
//!	Serialised container to hold all of the
//!	animations required to make a StrafeController work
//!
//------------------------------------------------------------------------------------------
class FormationStrafeAnimSet
{
public:

	// Construction
	FormationStrafeAnimSet() {;}

	// Animations
	CHashedString m_obStanding;
	CHashedString m_obMoveForwards;
	CHashedString m_obMoveBackwards;
	CHashedString m_obMoveLeft;
	CHashedString m_obMoveRight;
	CHashedString m_obMoveForwardLeft;
	CHashedString m_obMoveBackwardLeft;
	CHashedString m_obMoveForwardRight;
	CHashedString m_obMoveBackwardRight;
};

//------------------------------------------------------------------------------------------
//!
//!	StrafeControllerDef
//!	All the details needed to define strafing movement
//!
//------------------------------------------------------------------------------------------
class FormationStrafeControllerDef : public MovementControllerDef
{
public:

	// Construction 
	FormationStrafeControllerDef( void );
	virtual ~FormationStrafeControllerDef( void ) {}

	// To create an instance of what we describe
	virtual MovementController* CreateInstance(CMovement* pobMovement) const;

	// Make a clone of ourselves
	virtual MovementControllerDef* Clone( void ) const { return NT_NEW FormationStrafeControllerDef( *this ); }

	// The animation set to use
	FormationStrafeAnimSet* m_pobAnimSet;
	
	// How sticky do we want it to be - minimum input speed
	float m_fInputThreshold;
};

//------------------------------------------------------------------------------------------
//!
//!	StrafeController
//!	Strafing Movement - facing direction and movement direction are different
//!
//!	We have three internal states in this controller.  Each calculates the weight of their
//! animations as a percentage of one.  Then from above we blend in and out the states.
//!
//------------------------------------------------------------------------------------------
class FormationStrafeController : public MovementController
{
public:
	// Construction
	FormationStrafeController(CMovement* pobMovement, const FormationStrafeControllerDef& obDefinition);

	// Destruction
	virtual ~FormationStrafeController();

	// The main update details
	virtual bool Update(float                    fTimeStep, 
						const CMovementInput&    obMovementInput,
						const CMovementStateRef& obCurrentMovementState,
						CMovementState&          obPredictedMovementState);

protected:

	// The animations we use to move
	enum MOVING_ANIMATIONS	
	{
		MA_STANDING,
		MA_FORWARD,
		MA_BACK,
		MA_LEFT,
		MA_RIGHT,
		MA_FORWARDLEFT,
		MA_BACKLEFT,
		MA_FORWARDRIGHT,
		MA_BACKRIGHT,

		MA_COUNT
	};

	// Create an animation
	CAnimationPtr CreateAnimation(const CHashedString& obAnimationName, int iVariableFlags);

	// For adding and removing animations to the animator
	void InitialiseAnimations( void );
	void ActivateAnimations(CAnimationPtr* aAnimations, int iNumberOfAnims);
	void DeactivateAnimations(CAnimationPtr* aAnimations, int iNumberOfAnims);

	// Calculate the animation weights 
	void CalculateMoveWeights(const CMatrix& obRootMatrix, const CDirection& obMoveDirection, float fMoveSpeed);

private:
	// A COPY of our defintion
	FormationStrafeControllerDef m_obDefinition;

	// Good point to blend in new animations
	float m_fRecheckTime;

	// Our moving animations
	CAnimationPtr m_aobMovingAnims[MA_COUNT];
	float         m_afDesiredMovingWeights[MA_COUNT];
	float         m_afMovingWeights[MA_COUNT];
	float         m_fMoveSpeed;
};


#endif // _FORMATIONSTRAFECONTROLLER_INC
