//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformationxml.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "ai/aiformationxml.h"
#include "game/entity.h"
#include "game/attacks.h"
#include "ai/aiattack.h"
#include "ai/aiattackselection.h"
#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboState
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CGromboState, Mem::MC_AI)
	PUBLISH_PTR_AS(m_pobNextStateIfKoed,		NextStateIfKoed)
	PUBLISH_PTR_AS(m_pobNextStateIfRecoiled,	NextStateIfRecoiled)
	PUBLISH_PTR_AS(m_pobNextStateIfBlocked,		NextStateIfBlocked)
	PUBLISH_PTR_AS(m_pobNextState,				NextState)
	PUBLISH_VAR_AS(m_bEnter1on1,				Enter1on1)
	PUBLISH_VAR_AS(m_bUntilKoed,				UntilKoed)
	PUBLISH_VAR_AS(m_bUntilRecoiling ,			UntilRecoiling)
	PUBLISH_VAR_AS(m_bFaceTarget,				FaceTarget)
	PUBLISH_VAR_AS(m_fTimeout,					Timeout)
	PUBLISH_VAR_AS(m_fUntilDist,				UntilDist)
	PUBLISH_VAR_AS(m_fEarlyAttackRange,			EarlyAttackRange)
	PUBLISH_VAR_AS(m_fDelay,					Delay)
	PUBLISH_VAR_AS(m_obSendMsg ,				SendMsg)
	PUBLISH_VAR_AS(m_obSimpleAnim ,				SimpleAnim)
	PUBLISH_PTR_CONTAINER_AS(m_obAttacks,		Attacks)
	PUBLISH_PTR_AS(m_pNewAIAttackTree,			NewAIAttackTree)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboAttack
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CGromboAttack, Mem::MC_AI)
	PUBLISH_VAR_AS(m_fRange, Range)
	PUBLISH_VAR_AS(m_fWeight, Weight)
	PUBLISH_VAR_AS(m_fTolerance, Tolerance)
	PUBLISH_VAR_AS(m_obAnimName, AnimName)
	PUBLISH_PTR_AS(m_pobNextState, NextState)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboEntity
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CGromboEntity, Mem::MC_AI)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)

	PUBLISH_VAR_AS(m_fFormationPositionOffset, FormationPositionOffset)
	PUBLISH_VAR_AS(m_fTimeNotInGrombo, TimeNotInGrombo)
	PUBLISH_VAR_AS(m_fDelay, Delay)
	PUBLISH_VAR_AS(m_fDistanceToTarget, DistanceToTarget)
	PUBLISH_PTR_AS(m_iAngle, Angle)
	PUBLISH_PTR_AS(m_iCamera, Camera)
	PUBLISH_PTR_AS(m_iRelative, Relative)

	PUBLISH_PTR_AS(m_pobStartState, StartState)
	PUBLISH_VAR_AS(m_obType, Type)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboInstance
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CGromboInstance, Mem::MC_AI)
	PUBLISH_VAR_AS(m_fWeight,				Weight)
	PUBLISH_VAR_AS(m_fInterruptible,		Interruptible)
	PUBLISH_VAR_AS(m_fPriority,				Priority)
	PUBLISH_VAR_AS(m_bSimultaneousAllowed , SimultaneousAllowed)
	PUBLISH_VAR_AS(m_bIncidental ,			Incidental)
	PUBLISH_VAR_AS(m_obPlayerCombo ,		PlayerCombo)
	PUBLISH_VAR_AS(m_obOneOnOneAttacker ,	OneOnOneAttacker)
	PUBLISH_VAR_AS(m_obMetadata,			Metadata)
	PUBLISH_PTR_CONTAINER_AS(m_obEntities,	Entities)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboAttackPattern
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CGromboAttackPattern, Mem::MC_AI)
	PUBLISH_VAR_AS(m_fStartDelay ,		StartDelay)
	PUBLISH_VAR_AS(m_fEndDelay ,		EndDelay)
	PUBLISH_VAR_AS(m_obOnComplete ,		OnComplete)
	PUBLISH_VAR_AS(m_obSquads ,			Squads)
	PUBLISH_PTR_AS(m_pGromboInstance,	GromboInstance)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for CGromboAttackList
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE (CGromboAttackList, Mem::MC_AI)
	PUBLISH_PTR_CONTAINER_AS(m_obAttacks,	Attacks)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ForceLinkFunction25
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void ForceLinkFunction25()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction25() !ATTN!\n");
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboState::CGromboState
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboState::CGromboState()
{
	m_pobNextStateIfKoed = m_pobNextStateIfBlocked = m_pobNextStateIfRecoiled = m_pobNextState = 0;
	m_bUntilKoed = m_bUntilRecoiling = m_bEnter1on1 = m_bFaceTarget = false;
	m_fUntilDist = 15.0f;
	m_fTimeout = 15.0f;
	m_fEarlyAttackRange = m_fDelay = 0.0f;
	m_pNewAIAttackTree = 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboState::HasEnter1On1Recursive
//! 
//!  Check whether the grombo state and any below it have a one on one state enabled. 
//!                                                                                         
//------------------------------------------------------------------------------------------

#define ONE_ON_ONERECURSE_CHECKER(VALUE)				\
	if(VALUE)											\
	{													\
		CGromboState* pTemp = VALUE;					\
		VALUE = NULL;									\
		bool bRet = pTemp->HasEnter1On1Recursive();		\
		VALUE = pTemp;									\
		if(bRet)	return bRet;						\
	}

bool CGromboState::HasEnter1On1Recursive()
{
	if(m_bEnter1on1)
		return true;

	ONE_ON_ONERECURSE_CHECKER(m_pobNextStateIfKoed);
	ONE_ON_ONERECURSE_CHECKER(m_pobNextStateIfBlocked);
	ONE_ON_ONERECURSE_CHECKER(m_pobNextStateIfRecoiled);
	ONE_ON_ONERECURSE_CHECKER(m_pobNextState);

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboAttack::CGromboAttack
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboAttack::CGromboAttack()
{
	m_fRange = 0.0f;
	m_fWeight = 1.0f;
	m_fTolerance = 100.0f;
	m_pobNextState = 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboEntity::CGromboEntity
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboEntity::CGromboEntity()
{
	m_fDistanceToTarget = 10.0f;
	m_fFormationPositionOffset = m_fTimeNotInGrombo = m_fDelay = 0.0f;
	m_iAngle  = m_iCamera = m_iRelative = 0;
	m_pobStartState = 0;
	m_bHas1On1 = false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboEntity::OnPostConstruct
//! 
//!  Here we allow the instance of CGromboEntity to determine if there any instances of 
//!  1 on 1 in the attack queue. This is to allow the archer to select only 1 on 1 attacks.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CGromboEntity::OnPostConstruct(void)
{
	if(m_pobStartState)
		m_bHas1On1 = m_pobStartState->HasEnter1On1Recursive();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboInstance::CGromboInstance
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboInstance::CGromboInstance()
{
	m_fWeight = 1.0f;
	m_fPriority	= 0.0f;
	m_fInterruptible = 60.0f; 
	m_bSimultaneousAllowed = false;
	m_bIncidental = false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboAttackPattern::CGromboAttackPattern
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboAttackPattern::CGromboAttackPattern()
{
	m_fStartDelay = m_fEndDelay = 0.0f;
	m_pGromboInstance = NULL;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CGromboAttackList::CGromboAttackList
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
CGromboAttackList::CGromboAttackList()
{
}
