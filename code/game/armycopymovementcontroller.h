//------------------------------------------------------------------------------------------
//!
//!	\file armycopymovementcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ARMYCOPYMOVEMENTCONTROLLER_INC_
#define	ARMYCOPYMOVEMENTCONTROLLER_INC_

#include "game/movementcontrollerinterface.h"

//------------------------------------------------------------------------------------------
//!
//!	ArmyCopyMovementControllerDef
//!
//!
//------------------------------------------------------------------------------------------
class ArmyCopyMovementControllerDef : public MovementControllerDef
{
public:
	ArmyCopyMovementControllerDef() {;}
	virtual ~ArmyCopyMovementControllerDef() {;}

	void AddAnimation(CAnimationPtr& pAnim);


public:
	virtual MovementController *CreateInstance(CMovement *movement) const;
	virtual MovementControllerDef *Clone() const {return NT_NEW ArmyCopyMovementControllerDef(*this);}

	struct AOW
	{
		AOW(CHashedString& _sName, float _fOffset, float _fWeight) : sName(_sName), fOffset(_fOffset), fWeight(_fWeight) {;}

		CHashedString sName;
		float         fOffset;
		float         fWeight;
	};

	ntstd::List<AOW> m_AnimsToAdd;
};


//------------------------------------------------------------------------------------------
//!
//!	ArmyCopyMovementController
//!	
//!
//!
//------------------------------------------------------------------------------------------
class ArmyCopyMovementController : public MovementController
{
public:
	//
	//	Ctor, dtor.
	//
	ArmyCopyMovementController(CMovement *pMovement, const ArmyCopyMovementControllerDef &Definition);
	~ArmyCopyMovementController();

	// The main update details
	virtual bool Update(float fTimeStep, const CMovementInput& MovementInput,	
						const CMovementStateRef& CurrentMovementState, CMovementState& PredictedMovementState);

protected:
	struct ACMAnim
	{
		CAnimationPtr pAnim;
		float fWeight;
	};

	ntstd::List<ACMAnim> m_Anims;
};

#endif // ARMYCOPYMOVEMENTCONTROLLER_INC_

