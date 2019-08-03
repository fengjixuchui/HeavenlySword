//------------------------------------------------------------------------------------------
//!
//!	\file armycopymovementcontroller.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "armycopymovementcontroller.h"

#include "game/movement.h"
#include "game/entity.h"
#include "game/entity.inl"

#include "anim/animator.h"

#include "core/exportstruct_anim.h"
#include "core/osddisplay.h"


//------------------------------------------------------------------------------------------
//!
//!	ArmyCopyMovementControllerDef::AddAnimation
//!	Add an animation to the definition
//!
//------------------------------------------------------------------------------------------
void ArmyCopyMovementControllerDef::AddAnimation(CAnimationPtr& pAnim) 
{
	ntError(pAnim);

	CHashedString sName(pAnim->GetShortNameHash());
	ArmyCopyMovementControllerDef::AOW aow(sName, pAnim->GetTime(), pAnim->GetBlendWeight());
	m_AnimsToAdd.push_back(aow);
}


//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunControllerDef::CreateInstance
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
MovementController *ArmyCopyMovementControllerDef::CreateInstance(CMovement *pMovement) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) ArmyCopyMovementController(pMovement, *this);
}


//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunController::ArcherWalkRunController
//! Construction
//!
//------------------------------------------------------------------------------------------
ArmyCopyMovementController::ArmyCopyMovementController(CMovement* pMovement, const ArmyCopyMovementControllerDef &Definition)
: MovementController(pMovement)
{
	// Create all the animations requested
	for(ntstd::List<ArmyCopyMovementControllerDef::AOW>::const_iterator it = Definition.m_AnimsToAdd.begin(); 
		it != Definition.m_AnimsToAdd.end(); it++)
	{
		ACMAnim acmAnim;
		acmAnim.pAnim = m_pobAnimator->CreateAnimation(it->sName);
		acmAnim.pAnim->SetSpeed(0.f);
		acmAnim.pAnim->SetTime(it->fOffset);
		acmAnim.pAnim->SetBlendWeight(it->fWeight);
		acmAnim.pAnim->SetFlags(ANIMF_INHIBIT_AUTO_DESTRUCT);
		acmAnim.fWeight = it->fWeight;

		m_Anims.push_back(acmAnim);
		m_pobAnimator->AddAnimation(acmAnim.pAnim);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	ArmyCopyMovementController::ArmyCopyMovementController
//! Destructor
//!
//------------------------------------------------------------------------------------------
ArmyCopyMovementController::~ArmyCopyMovementController()
{
	for(ntstd::List<ACMAnim>::iterator it = m_Anims.begin(); it != m_Anims.end(); it++)
	{
		m_pobAnimator->RemoveAnimation(it->pAnim);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	ArmyCopyMovementController::Update
//! 
//!
//------------------------------------------------------------------------------------------
bool ArmyCopyMovementController::Update(float fTimeStep, const CMovementInput& MovementInput, 
										const CMovementStateRef& CurrentMovementState, CMovementState& PredictedMovementState)
{
	ntError(m_pobMovement != 0);
	ntError(m_pobMovement->GetParentEntity() != 0);

	UNUSED(fTimeStep);
	UNUSED(MovementInput);
	UNUSED(CurrentMovementState);
	UNUSED(PredictedMovementState);

	// Add all the animations with the appropriate weight
	for(ntstd::List<ACMAnim>::iterator it = m_Anims.begin(); it != m_Anims.end(); it++)
	{

		CAnimationPtr pAnim = it->pAnim;
		pAnim->SetBlendWeight(it->fWeight * m_fBlendWeight);
		if(!pAnim->IsActive())
		{
			m_pobAnimator->AddAnimation(pAnim);
		}
	}

	// We never finish...
	return false;
}


