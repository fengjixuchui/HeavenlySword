//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file attackselection.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

/*******************************************************************************************
AI Combat:
	Aims, with a simple interface allow access to the combat system, it must be robust.
		


*******************************************************************************************/


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "core/visualdebugger.h" 

#include "ai/aiattack.h"
#include "ai/aiattackselection.h"
#include "ai/aiformation.h"
#include "ai/aiformationslot.h"
#include "ai/aiformationattack.h"
#include "ai/aiformationcomponent.h"
// -- To be replaced soon (Dario)
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
// -- by this (Dario)
#include "ai/ainavigationsystem/ainavigsystemman.h"
// -------------------------------------------------

#include "anim/animator.h"

#include "game/shellconfig.h"
#include "game/luaexptypes.h"
#include "game/aicomponent.h"
#include "game/entityinfo.h"
#include "game/randmanager.h"
#include "game/query.h"
#include "game/entitymanager.h"
#include "game/attacks.h"
#include "game/messagehandler.h"
#include "game/luaglobal.h"
#include "game/hitcounter.h"
#include "game/keywords.h"
#include "lua/ninjalua.h"
#include "input/inputhardware.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

//**************************************************************************************************
// Start exposing the element to Lua
//**************************************************************************************************

LUA_EXPOSED_START(AICombatComponent)
	LUA_EXPOSED_METHOD(EmptyQueue,               EmptyQueue,               "", "", "")
	LUA_EXPOSED_METHOD(QueueAnim,                QueuePlayAnim,            "", "", "")
	LUA_EXPOSED_METHOD(QueueMove,                QueueMove,                "", "", "")
	LUA_EXPOSED_METHOD(ClearAttackQueue,         ClearAttackQueue,         "", "", "")
	LUA_EXPOSED_METHOD(GetTargetDistance,        GetTargetDistance,        "", "", "")
	LUA_EXPOSED_METHOD(PerformInstantKORecovery, PerformInstantKORecovery, "", "", "")

	LUA_EXPOSED_METHOD_GET(Radius,               GetRadius,              "")
	LUA_EXPOSED_METHOD_SET(Radius,               SetRadius,              "")
	LUA_EXPOSED_METHOD_SET(AttackTimer,          SetAttackTimer,         "")
	LUA_EXPOSED_METHOD_SET(MoveSpeed,            SetMoveSpeed,	         "")
	LUA_EXPOSED_METHOD_SET(MovePause,            SetMovePause,           "")
	
LUA_EXPOSED_END(AICombatComponent)


//------------------------------------------------------------------------------------------
// Constants                                                                                
//------------------------------------------------------------------------------------------

static const int MAX_QUEUEDATTACKS = 128;

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatDef::AICombatDef                                                                
//! Constructor                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
AICombatDef::AICombatDef()
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatDef::~AICombatDef
//! Destruction
//!                                                                                         
//------------------------------------------------------------------------------------------
AICombatDef::~AICombatDef()
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::AICombatComponent
//! Constructor
//!                                                                                         
//------------------------------------------------------------------------------------------
AICombatComponent::AICombatComponent(const AICombatDef* pDef, AI* pEntity)
	:	m_pEntity(pEntity),
		m_pAttackComp(pEntity->GetAttackComponent()),
		m_fAttackTargetTime(0.0f)
{
	ntAssert(pEntity);
	ntAssert(m_pAttackComp);

	ATTACH_LUA_INTERFACE(AICombatComponent);

	m_eCurrentAttack		= AAT_NONE;
	m_iCurrentAttackInfo    = 0;
	m_fBlockTimer			= -1.0f;
	m_fAttackTimer			= -1.0f;
	m_bConscious            = false;


	// Init the combat states
	m_eLastCombatState = CS_COUNT;
	m_eTargetLastCombatState = CS_COUNT;
	m_bProcessIncomingAttack = false;
	m_bProcessIncomingCounter = false;
	m_bWaitDeflectionComplete = false;
	
	m_bInLocalPostStrikeWindow = false;
	m_bInTargetPostStrikeWindow = false;

	m_bResetCombatMovement = false;
	m_AttackerCount = 0;

	m_eMovementState = MS_INITIALISE;

	// Initialise the combat state.
	m_eCombatState = AIC_IDLE;

	// 
	ApplyNewCombatDef( pDef );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::CAICombatTable                                                       
//! Destruction                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
AICombatComponent::~AICombatComponent()
{
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::RequestMovementRadius                                                       
//! Returns a movement radius relative from the target to this entity in which to travel to
//!                                                                                         
//------------------------------------------------------------------------------------------
float AICombatComponent::RequestMovementRadius( void ) const
{ 
	return (m_pAttackRoot) ? (m_pAttackRoot->RequestMovementRadius()) : 0.0f;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::RequestMovementSpeed                                                       
//! Return a speed in which the entity should travel to the next waypoint. 
//!                                                                                         
//------------------------------------------------------------------------------------------
float AICombatComponent::RequestMovementSpeed( void ) const
{ 
	float fReturnSpeed = (m_pAttackRoot) ? (m_pAttackRoot->RequestMovementSpeed()) : 1.0f;
	return fReturnSpeed < 0.3f ? 0.3f : fReturnSpeed;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::SetEntityToAttack
//! Set the attack target to a given entity
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::SetEntityToAttack(const CEntity* pEntity)
{
	const CEntity* pAttackTarget = m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack();

	// If there is a current attack target, then decrease the attacker count
	if (pAttackTarget && pAttackTarget->IsAI())
	{
		pAttackTarget->ToCharacter()->ToAI()->GetAIComponent()->GetCombatComponent().m_AttackerCount--;
	}

	// If there is an attack target, then increase the attacker count on that entity
	if (pEntity && pEntity->ToCharacter()->IsAI())
	{
		pEntity->ToCharacter()->ToAI()->GetAIComponent()->GetCombatComponent().m_AttackerCount++;
	}

	m_pEntity->GetAIComponent()->GetCAIMovement()->SetEntityToAttack(pEntity);
}

void AICombatComponent::SetEntityDescriptionsToAttack(const char* szAttackTargetTypes)
{
	m_AttackTargetDescriptions.Set(szAttackTargetTypes);
}

void AICombatComponent::UpdateAttackTarget(float fTimeDelta)
{
	CEntity* pAttackEntity = (Character*)m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack();

	// Target dead, so clear it.
	if (pAttackEntity && pAttackEntity->ToCharacter()->IsDead())
		pAttackEntity = 0;
 
	// Selection of targets available.
	if (m_AttackTargetDescriptions.GetKeywordCount())
	{
		// Clear target every so often.
		if ((m_fAttackTargetTime += fTimeDelta) >= 5.0f)
			pAttackEntity = 0;

		//Find new target.
		if (!pAttackEntity)
		{
			m_fAttackTargetTime = 0.0f;

			CEntityQuery obEntQuery;
			CEQCIsDescription obDescriptionClause;

			// Set up the query
			obDescriptionClause.SetKeywords(m_AttackTargetDescriptions);

			// Add the clause to the query
			obEntQuery.AddClause(obDescriptionClause);

			// Perform the search
			CEntityManager::Get().FindEntitiesByType(obEntQuery, CEntity::EntType_Character);

			// Obtain the results for the search
			const QueryResultsContainerType& obResults = obEntQuery.GetResults();

			float fBestDistanceSquared = -1.0f;
			pAttackEntity = 0;

			for (QueryResultsContainerType::const_iterator obIt = obResults.begin(); obIt != obResults.end(); ++obIt)
			{
				CEntity* pEntity = *obIt;

				if (pEntity != m_pEntity && !pEntity->ToCharacter()->IsDead())
				{
					float fDistanceSquared = (m_pEntity->GetPosition() - pEntity->GetPosition()).LengthSquared();

					if (!pAttackEntity || fDistanceSquared < fBestDistanceSquared)
					{
						pAttackEntity = pEntity;
						fBestDistanceSquared = fDistanceSquared;
					}
				}
			}
		}
	}

	m_pEntity->GetAIComponent()->SetEntityToAttack(pAttackEntity);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::IsAttackingPlayer															
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AICombatComponent::IsAttackingPlayer() const
{
	return (Character*)CEntityManager::Get().GetPlayer() == m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack()->ToCharacter();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::SetNewAttackTree
//! 
//! Allow access to setting a new attack tree type
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AICombatComponent::SetNewAttackTree(const CAIAttackRoot* pAIAttackTree)
{
	// I'm sure that there could be cases where this might fail or be bad - for the 
	// moment I can't think of any.
	m_pAttackRoot = pAIAttackTree;

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::Init															[STATIC]
//! Inititialise the system
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::Init()
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::CleanUp														[STATIC]
//! Cleanup system globals
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::CleanUp()
{
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::ApplyNewCombatDef
//! Apply a new AI def 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::ApplyNewCombatDef(const AICombatDef* pDef)
{
	if(pDef)
	{
		m_sScriptOverrides	= pDef->m_obScriptOverrides;
		//m_sFormationCombat	= pDef->m_obFormation;
		//m_sGroupCombat		= pDef->m_obGroupCombat;

		DataObject* pData = ObjectDatabase::Get().GetDataObjectFromName( pDef->m_obScriptOverrides );
		ntError_p( pData != NULL, ("Couldn't find the required combat def for entity: %s\n", ntStr::GetString(m_sScriptOverrides) ) );
		ntError_p( strcmp( pData->GetClassName(), "CAIAttackRoot" ) == 0, ("Error, the script type was wrong got %s, expected CAIAttackRoot", pData->GetClassName() ) );

		m_pAttackRoot = (CAIAttackRoot*) pData->GetBasePtr();
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::Update
//! Destruction
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::Update(float fTimeDelta,bool bUpdateMovement)
{
	// Update the combat movement
	if( bUpdateMovement )
		UpdateMovement( fTimeDelta );

	UpdateCombat(fTimeDelta);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::InsertPause
//! Insert the combo queue, from the front
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::InsertPause(float fDelay) const
{
	//ntPrintf("QueueMove %d\n", AAT_PAUSE);
	m_AttackQueue.push_front(QueuedAttack(fDelay));

}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::EmptyQueue
//! Clear all the moves out of the combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::EmptyQueue() const
{
	//ntPrintf("EmptyQueue %d\n", m_AttackQueue.size());

	while(!m_AttackQueue.empty())
	{
		//ntPrintf(" Removing: %s\n", ObjectDatabase::Get().GetGlobalEnum( "AI_ATTACK_TYPE" ).GetName( m_AttackQueue.front().eType ).c_str() );
		m_AttackQueue.pop_front();
	}

	// Clear out the local timers too
	m_fAttackTimer = -1.0f;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::QueueMove
//! Adds a move to our combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::QueueMove(int iType) const
{
	int iFlags = iType & 0xFFFF0000;

	AI_ATTACK_TYPE eType = (AI_ATTACK_TYPE) iType;
	eType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);

	if( AAT_QUEUE_FLUSH == eType )
	{
		ClearAttackQueue();
		return;
	}

	// Grab the hit counter - it shouldn't be possible to to have a player character without a hitcounter. 
	HitCounter* pHitCounter = CEntityManager::Get().GetPlayer()->GetAttackComponent()->GetHitCounter();

	if( AAT_GRAB_BLOCK  == eType && pHitCounter &&
		pHitCounter->GetCurrentStyleProgressionLevel() == HL_SPECIAL &&
		pHitCounter->GetCurrentHitLevel() == HL_SPECIAL )
	{
		return;
	}

	if( g_ShellOptions->m_bDisableAIBlocking && 
		(AAT_SPEED_BLOCK  == eType || AAT_POWER_BLOCK  == eType || 
		 AAT_RANGE_BLOCK  == eType || AAT_GRAB_BLOCK  == eType) )
	{
		return;
	}

	QueuedAttack obQueuedAttack = QueuedAttack(eType);

	// Should be perform a forced lockon?
	obQueuedAttack.m_bForceLockon = (iFlags & AAT_FORCE_LOCKON) == AAT_FORCE_LOCKON;

	// Obtain the current combat state
	COMBAT_STATE eState = m_pEntity->GetAttackComponent()->AI_Access_GetState();

	if( obQueuedAttack.m_bForceLockon && eState == CS_ATTACKING )
		obQueuedAttack.m_bForceLockon = false;

	//ntPrintf("Attack Queued %d (%d)\n", obQueuedAttack.eType, m_AttackQueue.size() );
	m_AttackQueue.push_back(obQueuedAttack);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::QueueFormationMsg
//! Adds a lua object in to our combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::QueueFormationMsg( const char* pcString ) const
{
	QueuedAttack obQueuedAttack = QueuedAttack( AAT_MSG_FORMATION );
	obQueuedAttack.m_strData = pcString;

	// If we've not got a combo to build on then start one immediately
//	if(m_AttackQueue.empty())
//		m_fTimeUntilCombo = 0.0f;

	m_AttackQueue.push_back(obQueuedAttack);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::QueuePlayAnim
//! Adds a lua object in to our combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::QueuePlayAnim( const char* pcString ) const
{
	QueuedAttack obQueuedAttack = QueuedAttack( AAT_PLAY_ANIM  );
	obQueuedAttack.m_strData = pcString;

	// If we've not got a combo to build on then start one immediately
//	if(m_AttackQueue.empty())
//		m_fTimeUntilCombo = 0.0f;

	m_AttackQueue.push_back(obQueuedAttack);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::QueueDirectAttack
//! Adds a lua object in to our combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::QueueDirectAttack( const char* pcString ) const
{
	QueuedAttack obQueuedAttack = QueuedAttack( AAT_DIRECT_ATTACK );
	obQueuedAttack.m_strData = pcString;

	// If we've not got a combo to build on then start one immediately
//	if(m_AttackQueue.empty())
//		m_fTimeUntilCombo = 0.0f;

	m_AttackQueue.push_back(obQueuedAttack);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::DirectAttack
//! Adds a lua object in to our combo queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AICombatComponent::DirectAttack( const char* pcString )
{
	if( m_eCombatState == AIC_ATTACKING || m_eCombatState == AIC_IDLE )
	{
		EmptyQueue();

		bool bRequestedAttack = m_pAttackComp->AI_Command_RequestDirectAttack(pcString);

		if( bRequestedAttack )
		{
			// Tell our lua state what we have been upto so it can recover properly
			if(m_pEntity->GetMessageHandler())
				m_pEntity->GetMessageHandler()->ReceiveMsg<msg_ai_attack>();

			m_eCombatState = AIC_ATTACKING;
			m_fBlockTimer = 0.0f;

			return true;
		}
		else
		{
			m_eCombatState = AIC_IDLE;
		}
	}

	// 
	return false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::PerformInstantKORecovery
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::PerformInstantKORecovery(void) const
{
	// Combat ending - is the entity on the floor waiting to get up?
	COMBAT_STATE eState = m_pEntity->GetAttackComponent()->AI_Access_GetState();

	// Floored - then get up
	if( eState == CS_KO || eState == CS_FLOORED || eState == CS_RISE_WAIT )
	{
		m_pEntity->GetAttackComponent()->AI_Command_DoInstantKORecovery();
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::PopAttack
//! Pops a move off the queue.
//!                                                                                         
//------------------------------------------------------------------------------------------
QueuedAttack AICombatComponent::PopAttack()
{
	if(m_AttackQueue.empty())
		return QueuedAttack(AAT_NONE);

	QueuedAttack attack = m_AttackQueue.front();
	m_AttackQueue.pop_front();

	return attack;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::HasAttackQueued                                                      
//! This AI is either performing an attack or has an attack queued to perform.              
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AICombatComponent::HasAttackQueued() const 
{
    return m_eCurrentAttack != AAT_NONE || m_AttackQueue.size() > 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::ClearAttackQueue
//! Different from empty queue in that this method really reset the state of the
//! of the combat component
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::ClearAttackQueue() const
{
	//ntPrintf("Clear Combat Queue\n");

	// Combat ending - is the entity on the floor waiting to get up?
	// COMBAT_STATE eState = m_pEntity->GetAttackComponent()->AI_Access_GetState();

	// Clear out the attack queue
	for(QueuedAttackList::iterator obIt = m_AttackQueue.begin(); obIt != m_AttackQueue.end();)
	{
		if( !obIt->IsBlock())
			obIt = m_AttackQueue.erase( obIt );
		else
			++obIt;
	}

	m_eCurrentAttack = AAT_NONE;
	m_iCurrentAttackInfo = 0;
	m_fBlockTimer = -1.0f;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::Event_OnAttackWarning
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::Event_OnAttackWarning(float fImpactTime, const CEntity*) const
{
	// If in a formation but not a formation attack, set the formation to pause the movement for a little bit
	if( m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() && !m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack() )
		m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetMovementPause( fImpactTime * 0.8f );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::Event_OnAttacked
//! Calls a lua function.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AICombatComponent::Event_OnAttacked(AI_ATTACK_TYPE eAttackType, float fTime, const CEntity* pEntity, bool, bool Incidental ) const
{
	// If hit by another AI, then make that AI our target. 
	if (pEntity->IsCharacter())
	{
		if ((m_pEntity->AttackAI() && pEntity->IsAI()) || pEntity->IsPlayer())
		{
			m_pEntity->GetAIComponent()->SetEntityToAttack(0);
		}
	}

	m_eIncomingAttackType = eAttackType;
	m_bProcessIncomingAttack = true;
	m_bProcessIncomingCounter = Incidental ? false : pEntity->GetAttackComponent()->AI_Access_IsPerformingCounter();
	m_fIncomingAttackStrikeTime = fTime + 0.3f;

	// If in a formation but not a formation attack, set the formation to pause the movement for a little bit
	if( m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() && !m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack() )
		m_pEntity->GetAIComponent()->GetAIFormationComponent()->SetMovementPause( fTime );
	
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::UpdateCombat
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AICombatComponent::UpdateCombat(float fTimeDelta)
{
	if( fTimeDelta <= 0.0f )
		return;

	// Check that the target is valid.
	if (!m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack()) 
		return;

	// Update the attack free timer. Has it really been that long since that last hit?
	m_AttackFreeTimer += fTimeDelta;

	// It could be interesting behaviour for the target to choose another target if an attack
	// hasn't executed after x seconds.
	if( m_AttackFreeTimer > 15.0f )
	{
		//m_pEntity->GetAIComponent()->SetEntityToAttack( 0 );
	}

	// We can't do anything if we're KOed or floored...
	COMBAT_STATE eState = m_pAttackComp->AI_Access_GetState();

	// Get the targets current state
	COMBAT_STATE eTargetState = m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetState();

	float fLocalPostStrikeRecoveryTime = 0.0f;
	if( !m_bInLocalPostStrikeWindow && (fLocalPostStrikeRecoveryTime = m_pAttackComp->AI_Access_GetPostStrikeRecovery()) != 0.0f )
	{
		// Set the post strike window to true preventing many updates to Local post strike window
		m_bInLocalPostStrikeWindow = true;

		if( m_pAttackRoot )
		{
			m_pAttackRoot->CombatRequestPostStrikeRecovery( *this, 
															(m_pEntity->GetPosition() - m_pEntity->GetAIComponent()->GetEntityToAttack()->GetPosition()).Length(), 
															m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_TimeSinceLastDeflect(), 
															fLocalPostStrikeRecoveryTime, 
															m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetAttackerCount(), 
															m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetStringDepth() );
		}
	}
	else if( m_bInLocalPostStrikeWindow && m_pAttackComp->AI_Access_GetPostStrikeRecovery() == 0.0f )
	{
		// Set the post strike window to false
		m_bInLocalPostStrikeWindow = false;
	}


	if( m_eTargetLastCombatState != eTargetState || m_bProcessIncomingAttack )
	{
		m_eLockonState = ( CS_ATTACKING == eTargetState && m_bProcessIncomingAttack ) ? AIC_ACTIVE : AIC_PASSIVE;

		if( CS_BLOCKING == eState  && AIC_PASSIVE == m_eLockonState )
		{
		}
		else if( m_bProcessIncomingCounter )
		{
			// Process the targets counter
			ProcessTargetStateChange( ACS_COUNTERING );
		}
		else
		{
			// Process the targets state change
			ProcessTargetStateChange( (AI_COMBAT_STATE)eTargetState );
		}
	}

	float fTime = 0.0f;

	if( !m_bInTargetPostStrikeWindow && (fTime = m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetPostStrikeRecovery()) != 0.0f )
	{
		// Set the post strike window to true preventing many updates to target post strike window
		m_bInTargetPostStrikeWindow = true;

		// Process the targets state change
		ProcessTargetStateChange( ACS_POST_STRIKE_RECOVERY, fTime );
	}
	else if( m_bInTargetPostStrikeWindow && m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetPostStrikeRecovery() == 0.0f )
	{
		// Set the post strike window to false
		m_bInTargetPostStrikeWindow = false;
	}


	// Record the targets state
	m_eTargetLastCombatState = eTargetState;

	// The targets current state is one of the following... then return now.
	if (eTargetState == CS_KO 
		|| eTargetState == CS_FLOORED 
		|| eTargetState == CS_RISE_WAIT 
		|| eTargetState == CS_HELD 
		|| eTargetState == CS_DYING 
		|| eTargetState == CS_DEAD 
		|| (m_pAttackComp->AI_Access_GetStruckStrike() && m_pAttackComp->AI_Access_GetStruckStrike()->IsCounter())
		|| (m_pAttackComp->IsInSuperStyleSafetyTransition())
		|| (m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent() && m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->GetDisabled())
		|| (m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent() && 
			m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() && 
			( (m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_HOLD) || 
			  (m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetCurrentAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) ) ) )
		return;

	switch( m_eCombatState )
	{
		case AIC_IDLE:

			if( m_AttackQueue.size() )
			{
				m_eCombatState = AIC_PROCESS_QUEUE;
			}
			else if( m_fBlockTimer > 0.0f )
			{
				if( m_bProcessIncomingAttack )
					m_eCombatState = AIC_PROCESS_QUEUE;

				m_fBlockTimer -= fTimeDelta;
				break;
			}
			// Check wether a new attack time should be obtained. 
			else if( m_fAttackTimer <= 0.0f )
			{
				if( m_pAttackRoot )
				{
					m_fAttackTimer = m_pAttackRoot->RequestAttackTime();
				}
				break;
			}
			// If the time change in the last frame is enough to place the attack count to a (sub) zero value.
			// Then request a new attack
			else if( (m_fAttackTimer -= fTimeDelta) <= 0.0f && m_pEntity->GetAIComponent()->GetAIFormationComponent()->CanAttack() )
			{
				if( m_pAttackRoot )
				{
					m_pAttackRoot->CombatRequestAttack(	*this, 
														(m_pEntity->GetPosition() - m_pEntity->GetAIComponent()->GetEntityToAttack()->GetPosition()).Length(), 
														m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetAttackerCount(), 
														m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_GetStringDepth(), 
														m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent()->AI_Access_TimeSinceLastDeflect() );

					goto REPROCESS_QUEUE;
				}

				break;
			}
			else
			{
				break;
			}
			

		REPROCESS_QUEUE:
		case AIC_PROCESS_QUEUE:
			{
				// Get the next attack from the queue
				QueuedAttack currentAttack = PopAttack();

				// If required, flush queue on the combat side. 
				if( currentAttack.IsCombatFlush() )
				{
					m_pAttackComp->AI_Command_RequestAttack( AM_NONE );
				}
				// Blocks are more important than attacks
				else if( currentAttack.IsBlock() )
				{
					// Get a block type to check CanBlock with
					BLOCK_TYPE eCheckBlockType;

					switch( currentAttack.eType )
					{
						case AAT_SPEED_BLOCK: eCheckBlockType = BT_SPEED; break;
						case AAT_POWER_BLOCK: eCheckBlockType = BT_POWER; break;
						case AAT_RANGE_BLOCK: eCheckBlockType = BT_RANGE; break;
						case AAT_GRAB_BLOCK:  eCheckBlockType = BT_GRAB; break;
						default: ntAssert( false ); eCheckBlockType = BT_COUNT;
					}

					// Blocks are more important than attacks
					if( (currentAttack.IsBlock() && m_pAttackComp->AI_Access_CanBlock(eCheckBlockType)) 
							|| (m_bProcessIncomingAttack && m_fBlockTimer >= 0.0f)
							&& m_pEntity->GetAIComponent()->GetAIFormationComponent()->CanBlock() )
					{

						BLOCK_TYPE eBlockType = BT_COUNT;

						if( (currentAttack.eType == AAT_SPEED_BLOCK) || (m_bProcessIncomingAttack && (m_eIncomingAttackType == AAT_SPEED_FAST || m_eIncomingAttackType == AAT_SPEED_MEDIUM)) )
							eBlockType = BT_SPEED;
						else if( (currentAttack.eType == AAT_POWER_BLOCK) || (m_bProcessIncomingAttack && (m_eIncomingAttackType == AAT_POWER_FAST || m_eIncomingAttackType == AAT_POWER_MEDIUM)) )
							eBlockType = BT_POWER;
						else if( (currentAttack.eType == AAT_RANGE_BLOCK) || (m_bProcessIncomingAttack && (m_eIncomingAttackType == AAT_RANGE_FAST || m_eIncomingAttackType == AAT_RANGE_MEDIUM)) )
							eBlockType = BT_RANGE;
						else if( (currentAttack.eType == AAT_GRAB_BLOCK) || (m_bProcessIncomingAttack && (m_eIncomingAttackType == AAT_SPEED_GRAB || m_eIncomingAttackType == AAT_POWER_GRAB || m_eIncomingAttackType == AAT_RANGE_GRAB)) )
							eBlockType = BT_GRAB;

						// Set a default block time.
						m_fBlockTimer = 0.7f;

						// If set, over write the block timer with a specific active attack time
						if( m_fIncomingAttackStrikeTime > 0.0f )
						{
							m_fBlockTimer = m_fIncomingAttackStrikeTime;
							m_fIncomingAttackStrikeTime = 0.0f;
						}

						// Request the block
						if( eBlockType != BT_COUNT && m_pAttackComp->AI_Command_RequestBlock( eBlockType, m_fBlockTimer ) )
						{
							m_bProcessIncomingAttack = false;
							m_eCombatState = AIC_BLOCKING;
						}
						else
						{
							m_fBlockTimer = 0.0f;
						}
						break;
					}
					else
					{
						//ntPrintf("Block failed... \n");
					}
				}
				else if( (currentAttack.IsDirectAttack() || currentAttack.IsAttack() || currentAttack.IsEvade() || currentAttack.IsGrab() || currentAttack.IsAction()) && m_pAttackComp->AI_Access_CanRequestAttack() )
				{
					//ntPrintf("Requesting attack\n");
					const CAttackComponent* pTargetAttackComp = m_pEntity->GetAIComponent()->GetEntityToAttack()->GetAttackComponent();

					bool bCanAttack = m_pEntity->GetAIComponent()->GetAIFormationComponent()->CanAttack();

					bool bRequestedAttack = false;

					// Perform the attack
					if( currentAttack.IsEvade() && bCanAttack )
					{
						bRequestedAttack = m_pAttackComp->AI_Command_RequestAttack(AAT_2_AM[currentAttack.eType], currentAttack.m_bForceLockon, pTargetAttackComp, ChooseAwesomeEvadeDirection(currentAttack.eType) );
					}
					else if( currentAttack.IsDirectAttack() )
					{
						bRequestedAttack = m_pAttackComp->AI_Command_RequestDirectAttack(currentAttack.m_strData.c_str());
						//ntPrintf("Requesting direct attack %d %s\n", bRequestedAttack, currentAttack.m_strData.c_str() );
					}
					else if( bCanAttack )
					{
						bRequestedAttack = m_pAttackComp->AI_Command_RequestAttack(AAT_2_AM[currentAttack.eType], currentAttack.m_bForceLockon, pTargetAttackComp);
					}

					if( bRequestedAttack )
					{
						// Tell our lua state what we have been upto so it can recover properly
						if(m_pEntity->GetMessageHandler())
							m_pEntity->GetMessageHandler()->ReceiveMsg<msg_ai_attack>();
		
						m_AttackFreeTimer = 0;
						m_eCombatState = AIC_ATTACKING;
						m_fBlockTimer = 0.0f;
					}
					else
					{
						//ntPrintf("Requesting attack failed %d\n", currentAttack.eType);
						m_eCombatState = AIC_IDLE;
					}
				}
				else if( currentAttack.IsRetToFormation() )
				{
				}
				else if( currentAttack.IsLuaCallback() )
				{
				}
				else if( currentAttack.IsMsgFormation() )
				{
				}
				else if( currentAttack.IsPlayAnim() )
				{
				}
				else
				{
					m_eCombatState = AIC_IDLE;
				}
			}
			break;

		case AIC_BLOCKING:
			
			if( m_AttackQueue.size() )
			{
				if( m_AttackQueue.front().IsBlock() )
				{
					m_eCombatState = AIC_PROCESS_QUEUE;
					goto REPROCESS_QUEUE;
				}

				if( m_pAttackComp->AI_Access_CanRequestAttack() && (m_AttackQueue.front().IsAttack() || m_AttackQueue.front().IsEvade() || m_AttackQueue.front().IsGrab()) )
				{
					m_pAttackComp->AI_Command_LeaveBlock();
					m_eCombatState = AIC_PROCESS_QUEUE;
				}
			}

			if( eState != CS_BLOCKING )
			{
				m_eCombatState = AIC_IDLE;
				m_pAttackComp->AI_Command_LeaveBlock();
			}
			else if( eTargetState != CS_ATTACKING )
			{
				m_eCombatState = AIC_IDLE;
				m_pAttackComp->AI_Command_LeaveBlock();
			}
			else if( (m_fBlockTimer -= fTimeDelta) <= 0.0f ) 
			{
				m_eCombatState = AIC_IDLE;
				m_pAttackComp->AI_Command_LeaveBlock();
			}

			break;

		case AIC_ATTACKING:

			if( m_AttackQueue.size() )
			{
				//ntPrintf("Attacking: %d %d\n", m_pAttackComp->AI_Access_CanRequestAttack(), m_AttackQueue.size() );

				// Get a block type to check CanBlock with
				BLOCK_TYPE eCheckBlockType = BT_COUNT;
				if (m_AttackQueue.front().eType == AAT_SPEED_BLOCK)
					eCheckBlockType = BT_SPEED;
				else if (m_AttackQueue.front().eType == AAT_POWER_BLOCK)
					eCheckBlockType = BT_POWER;
				else if( m_AttackQueue.front().eType == AAT_RANGE_BLOCK)
					eCheckBlockType = BT_RANGE;

				if( m_pAttackComp->AI_Access_CanBlock(eCheckBlockType) && m_AttackQueue.front().IsBlock() )
				{
					m_eCombatState = AIC_PROCESS_QUEUE;
					goto REPROCESS_QUEUE;
				}

				else if( m_AttackQueue.front().IsAttack() || m_AttackQueue.front().IsEvade() || m_AttackQueue.front().IsGrab() )
				{
					if( m_pAttackComp->AI_Access_CanRequestAttack() )
					{
						//ntPrintf("Attacking: AIC_PROCESS_QUEUE\n");
						m_eCombatState = AIC_PROCESS_QUEUE;

						// Processing of a combat request should occur this frame, nasty goto used
						goto REPROCESS_QUEUE;
					}
				}
				else 
				{
					m_eCombatState = AIC_PROCESS_QUEUE;
					goto REPROCESS_QUEUE;
				}
			}
			else if( eState != CS_ATTACKING )
			{
				m_eCombatState = AIC_IDLE;
				m_obMoveTarget = m_pEntity->GetPosition();
			}
			break;


		case AIC_BLOCK_RECOVER:
			break;

		case AIC_ATTACK_PENDING:
			break;
	};
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::ShouldQueueMove
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AICombatComponent::ShouldQueueMove( AI_ATTACK_TYPE eType, float fStopChance ) const
{
	if( eType == AAT_NONE )
		return false;

	// Adjust the returned result based of the attack type
	bool bIsBlock = eType == AAT_SPEED_BLOCK ||
					eType == AAT_POWER_BLOCK ||
					eType == AAT_RANGE_BLOCK ||
					eType == AAT_GRAB_BLOCK;

	// Adjust the returned result based of the attack type
	bool bIsAttack = eType == AAT_SPEED_FAST || eType == AAT_SPEED_MEDIUM ||
					 eType == AAT_POWER_FAST || eType == AAT_POWER_MEDIUM ||
					 eType == AAT_RANGE_FAST || eType == AAT_RANGE_MEDIUM;

	// If it's not an attack / block, the always queue the move
	if( !(bIsAttack||bIsBlock) )
		return true;

	int		iReqSkill		= (int) fStopChance;
	float	fSkillAdjust	= fStopChance - (float)iReqSkill;

	// Get the default skill adjust and scalar
	int	  iSkill	= m_pEntity->AttackSkill();
	float fAdjust	= m_pEntity->AttackAdjust();

	// If the attack type is block, then read out the special block values. 
	if( bIsBlock )
	{
		iSkill		= m_pEntity->BlockSkill();
		fAdjust		= m_pEntity->BlockAdjust();
	}

	if( iSkill < iReqSkill )
		return false;

	float fRandCheck = (fAdjust + ((100.0f - fAdjust) * fSkillAdjust ));
	float fRand = grandf(100.0f);

	// Calc whether the move will execute
	return fRandCheck > fRand;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::UpdateMovement
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AICombatComponent::UpdateMovement(float fTimeDelta)
{
	UNUSED( fTimeDelta );

	//// If there isn't a player - return now. 
	//if(!GetAttackTarget())
	//	return;

	//// Cache a copy of the AI component
	//CAIComponent* pobAI = m_pEntity->GetAIComponent();

	//// Cache the current points of the local entity and the target entity
	//CPoint ptMyPos = pobAI->GetParent()->GetPosition();
	//CPoint ptTarget = GetAttackTarget()->GetPosition();

	//if( m_pAttackComp->AI_Access_GetState() != CS_STANDARD )
	//{
	//	m_eMovementState = MS_ATPOINT;
	//	return;
	//}

	//// Update the movement state.
	//switch( m_eMovementState )
	//{
	//	case MS_INITIALISE:
	//	{
	//		ntAssert( m_pAttackRoot );

	//		// Get the radius
	//		m_fRadius = m_pAttackRoot->RequestMovementRadius();

	//		// Set the next state. 
	//		m_eMovementState = MS_WAITING_FOR_RADIUS;

	//		break;
	//	};

	//	case MS_WAITING_FOR_RADIUS:
	//	{
	//		m_bResetCombatMovement = false;

	//		// 
	//		//g_VisualDebug->RenderSphere( m_pEntity->GetMatrix(), 0xFFFF00FF );

	//		// Whilst the radius is invalid break out of the loop
	//		if( m_fRadius <= 0.0f )
	//		{
	//			// place the entity into a default wait anim
	//			pobAI->SetAction(ACTION_INFORMATION_ANIM);
	//			pobAI->SetActionStyle(AS_AGGRESSIVE);
	//			pobAI->SetActionDest(ptTarget);
	//			pobAI->SetActionMoveSpeed(m_fSpeedToMoveTarget);
	//			// Make sure the entity is facing the target
	//			pobAI->SetActionFacing(-CDirection(ptMyPos - ptTarget));

	//			// 
	//			m_eMovementState = MS_INITIALISE;
	//			break;
	//		}

	//		m_fSpeedToMoveTarget = 0.6f;

	//		// 
	//		//g_VisualDebug->RenderSphere( m_pEntity->GetMatrix(), 0xFFFF0000 );

	//		// Valid radius, next determine where to move to. If the chosen point isn't valid after 10 attempts, then bail. 
	//		for( int iTry = 10; iTry && m_fRadius > 1.0f; --iTry )
	//		{
			// 
	//			// Calc the movement angle.
	//			float fAngle		 = grandf(TWO_PI);

	//			// Set the watchdog timer
	//			m_fMoveTargetTimeLimit = grandf(5.0f) + 2.0f;
				
	//			// Save the targets position
	//			m_obTargetStart = ptTarget;

	//			// Generate a point relative to the target. 
	//			m_obMoveTarget = CPoint( ptTarget.X() + (m_fRadius * fcosf(fAngle)), 
	//									 ptMyPos.Y(), 
	//									 ptTarget.Z() + (m_fRadius * fsinf(fAngle))); 


	//			// Is the movement target valid? 
	//			if ( ( !(CAINavigationSystemMan::Get().IsNewAIActive()) && (CAINavGraphManager::Get().IsPosLegal( m_obMoveTarget, &m_pEntity->GetKeywords())) ) ||
	//				 ( (CAINavigationSystemMan::Get().IsNewAIActive()) && ( m_pVision->IsPosValidForFormation(m_obMoveTarget)) )
	//				)
	//			{
	//				if( (m_obMoveTarget - ptMyPos ).LengthSquared() < 1.0f )
	//				{
	//					continue;
	//				}

	//				// Move the state on to moving. 
	//				m_eMovementState = MS_MOVING;
	//				break;
	//				
	//			}
	//			else
	//			{
	//				//g_VisualDebug->RenderPoint( m_obMoveTarget, 2.0f, 0xFF00FF00 );
	//				m_fRadius -= m_fRadius / 5.0f;
	//				m_fSpeedToMoveTarget -= m_fSpeedToMoveTarget / 5.0f;
	//			}
	//		}

	//		if( m_eMovementState == MS_WAITING_FOR_RADIUS )
	//		{
	//			// place the entity into a default wait anim
	//			pobAI->SetAction(ACTION_INFORMATION_ANIM);
	//			pobAI->SetActionStyle(AS_AGGRESSIVE);
	//			pobAI->SetActionDest(ptTarget);
	//			pobAI->SetActionMoveSpeed(m_fSpeedToMoveTarget);

	//			// Make sure the entity is facing the target
	//			pobAI->SetActionFacing(-CDirection(ptMyPos - ptTarget));
					
	//			// Failed to find a new point - ask the AI to try again
	//			m_eMovementState = MS_INITIALISE;
	//		}
	//	}break;

	//	case MS_MOVING:
	//		{
	//			// Make sure the Y position for the target stays with the target
	//			m_obMoveTarget.Y() = ptTarget.Y();

	//			// Attack target relative vectors.
	//			CDirection dirTrMe	= CDirection(ptMyPos - ptTarget);
	//			CDirection dirTrDest = CDirection(m_obMoveTarget - ptTarget);

	//			// Normalise the directions
	//			dirTrMe.Normalise();
	//			dirTrDest.Normalise();

	//			// 
	//			if( (ptMyPos - ptTarget).LengthSquared() > (10.0f * 10.0f) )
	//			{
	//				//NinjaLua::LuaObject obMsg = CMessageHandler::Make(m_pEntity, "OnCombatFarFromTarget" );

	//				// Set the context
	//				//obMsg.Set("_context", (int) MSG_CONTEXT_COMBAT );

	//				//m_pEntity->GetMessageHandler()->Receive( obMsg );
	//				m_eMovementState = MS_ATPOINT;
	//			}

	//			// 
	//			if( m_bProcessIncomingAttack || m_bResetCombatMovement )
	//			{
	//				m_obMoveTarget = ptMyPos;
	//			}
				// 
	//			// Excessive calculation of a dist
	//			float fGuessDist = -(dirTrMe.Dot( dirTrDest ) - 1.0f) * 0.5f * PI * m_fRadius;
	//			CPoint obDestPoint = m_obMoveTarget;

	//			// Smooth the rotation around the target if the _rough_ circlar distance is greater than a meter
	//			if( fGuessDist > 1.0f )
	//			{
	//				// Get the direction of the dest point from our position
	//				CDirection dirTemp = CDirection(m_obMoveTarget - ptMyPos);
	//				dirTemp.Normalise();
				
	//				// Get the new dest point in world space. 
	//				obDestPoint = CPoint(dirTemp) + ptMyPos;

	//				// From the new dest point, work out a direction from the traget
	//				dirTemp = CDirection(obDestPoint - ptTarget);
	//				dirTemp.Normalise();

	//				// Based on the distance from the original dest point, the entities current radius and the final radius,
	//				// project a world space to walk to. 
	//				float fDestRadius = (m_obMoveTarget - ptTarget).Length();
	//				float fMyRadius =	(ptMyPos - ptTarget).Length();

	//				// Get the new dest point in world space. 
	//				dirTemp *= ((fDestRadius-fMyRadius) * (1.0f / fGuessDist)) + fMyRadius;
	//				obDestPoint = CPoint(dirTemp) + ptTarget;

	//				// Make sure that the entity isn't being stupid
	//				//CAINavGraphManager::Get().GetRepulsion()->RemoveEntity( m_pEntity );
	//				// (Dario)	if( !CAINavGraphManager::Get().InGraph( obDestPoint, 1.0f ) || CAINavGraphManager::Get().GetRepulsion()->IsObstructed(obDestPoint, 1.0f) )

	//				if ( ( !(CAINavigationSystemMan::Get().IsNewAIActive()) && (!CAINavGraphManager::Get().InGraph( obDestPoint, 1.0f ) || CAINavGraphManager::Get().GetRepulsion()->IsObstructed(obDestPoint, 1.0f)) ) ||
	//					 ( (CAINavigationSystemMan::Get().IsNewAIActive()) && ( m_pVision->IsPosValidForFormation(obDestPoint)) )
	//				   )
	//				{
	//					//CAINavGraphManager::Get().GetRepulsion()->AddEntity( m_pEntity );
	//					m_obMoveTarget = ptMyPos;
	//					break;
	//				}
	//				//CAINavGraphManager::Get().GetRepulsion()->AddEntity( m_pEntity );
	//			}

	//			//
	//			if( CInputHardware::Get().GetContext() == INPUT_CONTEXT_AI )
	//			{
	//				g_VisualDebug->RenderLine( ptMyPos + CPoint(0.0f, 0.3f, 0.0f) , obDestPoint + CPoint(0.0f, 0.3f, 0.0f), 0xFF8080FF );
	//				g_VisualDebug->RenderLine( ptMyPos + CPoint(0.0f, 0.3f, 0.0f) , m_obMoveTarget + CPoint(0.0f, 0.3f, 0.0f), 0xFFFF80FF );
	//			}
				//
	//			// If too far from the move target, then keep moving there.
	//			if( (ptMyPos - m_obMoveTarget).LengthSquared() > (0.5f * 0.5f) && ((m_fMoveTargetTimeLimit -= fTimeDelta) > 0.0f) )
	//			{
	//				pobAI->SetAction(ACTION_STRAFE);
	//				pobAI->SetActionStyle(AS_AGGRESSIVE);
	//				pobAI->SetActionFacing(-dirTrMe);
	//				pobAI->SetActionDest(obDestPoint);
	//				pobAI->SetActionMoveSpeed(m_fSpeedToMoveTarget);
	//			}
	//			else
	//			{
	//				// Move the state on to moving. 
	//				m_eMovementState = MS_ATPOINT;
				
	//				// place the entity into a default wait anim
	//				pobAI->SetAction(ACTION_INFORMATION_ANIM);
	//				pobAI->SetActionStyle(AS_AGGRESSIVE);

	//				if( m_pAttackRoot )
	//				{
	//					// Set the default wait time, 
	//					m_fNewMoveTargetPause = m_pAttackRoot->RequestMovementPause();
	//				}
	//			}
	//		}
	//		break;

	//	case MS_ATPOINT:

	//		m_fNewMoveTargetPause -= fTimeDelta;

	//		pobAI->SetAction(ACTION_INFORMATION_ANIM);
	//		pobAI->SetActionStyle( AS_AGGRESSIVE );
	//		pobAI->SetFormationIdleAnim( "formationclose_inc_strafe" );

	//		// Make sure the entity is facing the target
	//		pobAI->SetActionFacing(-CDirection(ptMyPos - ptTarget));

	//		// Wait for the timer to complete, or if the target has moved by a given threshold, then restart
	//		if( (m_fNewMoveTargetPause > 0.0f) && 
	//			((m_obTargetStart - ptTarget).LengthSquared() <= (0.5f * 0.5f) ) )
	//			break;

	//		// Back to the start
	//		m_eMovementState = MS_INITIALISE;
	//		break;
	//};
}

//------------------------------------------------------------------------------------------
//!	AICombatComponent::RequestMovementPause                                                                 
//------------------------------------------------------------------------------------------
float AICombatComponent::RequestMovementPause( void ) const
{ 
	ntAssert( m_pAttackRoot );
	return (m_pAttackRoot->RequestMovementPause()); 
}

//------------------------------------------------------------------------------------------
//!  public constant  RequestMovementTargetThreshold
//!
//!  @return float	Return the distance the target has to move from their current position 
//!					to tigger this entity to move
//!
//!  @author GavB @date 06/07/2006
//------------------------------------------------------------------------------------------
float AICombatComponent::RequestMovementTargetThreshold( void ) const
{
	ntAssert( m_pAttackRoot );
	return (m_pAttackRoot->RequestMovementTargetThreshold()); 
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::GetTargetDistance
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

float AICombatComponent::GetTargetDistance(void) const
{
	// If there isn't a target - return now. 
	if (!m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack())
		return -1.f;

	// Cache a copy of the AI component
	CAIComponent* pobAI = m_pEntity->GetAIComponent();

	// Cache the current points of the local entity and the target entity
	CPoint ptMyPos = pobAI->GetParent()->GetPosition();
	CPoint ptTarget = m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack()->GetPosition();

	// Return the distance between the two entities
	return (ptMyPos - ptTarget).Length();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::CombatStateChanged
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AICombatComponent::CombatStateChanged( COMBAT_STATE eCombatState )
{
	// Only process state changes when not in group combat
	if( m_pEntity->GetAIComponent()->GetAIFormationComponent()
		&& m_pEntity->GetAIComponent()->GetAIFormationComponent()->CanAttack() )
	{
		if( m_pAttackRoot )
		{
			const CEntity* pTargetEntity = m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack();

			CAIAttackRoot::StateChange obStateChanged;

			obStateChanged.m_fAttackFree		= pTargetEntity ? pTargetEntity->GetAttackComponent()->AI_Access_TimeSinceLastDeflect() : 0.0f;
			obStateChanged.m_fRange				= pTargetEntity ? (m_pEntity->GetPosition() - pTargetEntity->GetPosition()).Length() : 0.0f;
			obStateChanged.m_fTime				= 0.0f;
			obStateChanged.m_eStyle				= AC_COUNT;
			obStateChanged.m_eLocalState		= GetEntity()->GetAttackComponent()->AI_Access_GetState();
			obStateChanged.m_iTargetAttackCount = pTargetEntity ? pTargetEntity->GetAttackComponent()->AI_Access_GetAttackerCount() : 0;
			obStateChanged.m_iTargetStringDepth = pTargetEntity ? pTargetEntity->GetAttackComponent()->AI_Access_GetStringDepth() : 0;
			obStateChanged.m_pCombatComp		= this;

			// Is there an attack being formed
			const CAttackData* pAttackData = pTargetEntity ? pTargetEntity->GetAttackComponent()->AI_Access_GetCurrentAttackDataP() : 0;

			if( pAttackData )
				obStateChanged.m_eStyle	= pAttackData->m_eAttackClass;

			// Is the entity in an active or passive state?
			const CAIAttackState* pState = m_pAttackRoot->GetLocalState();

			// 
			switch( eCombatState )
			{
				case CS_STANDARD:				obStateChanged.m_pList = &pState->m_listStandard; break;
				case CS_ATTACKING:				obStateChanged.m_pList = &pState->m_listAttacking; break;
				case CS_RECOILING:				obStateChanged.m_pList = &pState->m_listRecoiling; break;
				case CS_BLOCKING:				obStateChanged.m_pList = &pState->m_listBlocking; break;
				case CS_DEFLECTING:				obStateChanged.m_pList = &pState->m_listDeflecting; break;
				case CS_KO:						obStateChanged.m_pList = &pState->m_listKO; break;
				case CS_FLOORED:				obStateChanged.m_pList = &pState->m_listFloored; break;
				case CS_RISE_WAIT:				obStateChanged.m_pList = &pState->m_listRiseWait; break;
				case CS_BLOCK_STAGGERING:		obStateChanged.m_pList = &pState->m_listBlockStaggering; break;
				case CS_IMPACT_STAGGERING:		obStateChanged.m_pList = &pState->m_listImpactStaggering; break;
				case CS_INSTANTRECOVER:			obStateChanged.m_pList = &pState->m_listInstantRecover; break;
				case CS_HELD:					obStateChanged.m_pList = &pState->m_listHeld; break;
				case CS_RECOVERING:				obStateChanged.m_pList = &pState->m_listRecovering; break;
				case CS_DYING:					obStateChanged.m_pList = &pState->m_listDying; break;
				case CS_DEAD:					obStateChanged.m_pList = &pState->m_listDead; break;
				default: ntError_p( false, ("Error: unknown state type defined: %d\n", eCombatState) );
			}									

			m_pAttackRoot->ProcessStateChange( this, obStateChanged );

		}
	}

	if( eCombatState == CS_STANDARD 
		&& m_pEntity->GetAIComponent()->GetAIFormationComponent()
		&& m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() 
		&& !m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack() )
	{
		const AIFormation* pFormation = m_pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation();
		const AIFormationSlot* pSlot = pFormation->FindSlot( *m_pEntity );

		// Make sure the entity has a slot. 
		if( pSlot )
		{
			pSlot->SetInPosition( false );
		}
	}

}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::ProcessTargetStateChange
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AICombatComponent::ProcessTargetStateChange( AI_COMBAT_STATE eState, float fTime )
{
	// Pointer to the target entity
	const Character* pTarget = m_pEntity->GetAIComponent()->GetCAIMovement()->GetEntityToAttack()->ToCharacter();

	if( m_pAttackRoot )
	{
		CAIAttackRoot::StateChange obStateChanged;

		obStateChanged.m_fAttackFree		= pTarget->GetAttackComponent()->AI_Access_TimeSinceLastDeflect();
		obStateChanged.m_fRange				= (m_pEntity->GetPosition() - pTarget->GetPosition()).Length();
		obStateChanged.m_fTime				= fTime;
		obStateChanged.m_eStyle				= AC_COUNT;
		obStateChanged.m_eLocalState		= GetEntity()->GetAttackComponent()->AI_Access_GetState();
		obStateChanged.m_iTargetAttackCount = pTarget->GetAttackComponent()->AI_Access_GetAttackerCount();
		obStateChanged.m_iTargetStringDepth = pTarget->GetAttackComponent()->AI_Access_GetStringDepth();
		obStateChanged.m_pCombatComp		= this;

		// Is there an attack being formed
		const CAttackData* pAttackData = pTarget->GetAttackComponent()->AI_Access_GetCurrentAttackDataP();
		if( pAttackData )
			obStateChanged.m_eStyle	= pAttackData->m_eAttackClass;

		// Is the entity in an active or passive state?
		const CAIAttackState* pState = (m_eLockonState == AIC_ACTIVE) ? m_pAttackRoot->GetActiveState() : m_pAttackRoot->GetPassiveState();

		// 
		switch( (int) eState )
		{
			case CS_STANDARD:				obStateChanged.m_pList = &pState->m_listStandard; break;
			case CS_ATTACKING:				obStateChanged.m_pList = &pState->m_listAttacking; break;
			case CS_RECOILING:				obStateChanged.m_pList = &pState->m_listRecoiling; break;
			case CS_BLOCKING:				obStateChanged.m_pList = &pState->m_listBlocking; break;
			case CS_DEFLECTING:				obStateChanged.m_pList = &pState->m_listDeflecting; break;
			case CS_KO:						obStateChanged.m_pList = &pState->m_listKO; break;
			case CS_FLOORED:				obStateChanged.m_pList = &pState->m_listFloored; break;
			case CS_RISE_WAIT:				obStateChanged.m_pList = &pState->m_listRiseWait; break;
			case CS_BLOCK_STAGGERING:		obStateChanged.m_pList = &pState->m_listBlockStaggering; break;
			case CS_IMPACT_STAGGERING:		obStateChanged.m_pList = &pState->m_listImpactStaggering; break;
			case CS_INSTANTRECOVER:			obStateChanged.m_pList = &pState->m_listInstantRecover; break;
			case CS_HELD:					obStateChanged.m_pList = &pState->m_listHeld; break;
			case CS_RECOVERING:				obStateChanged.m_pList = &pState->m_listRecovering; break;
			case CS_DYING:					obStateChanged.m_pList = &pState->m_listDying; break;
			case CS_DEAD:					obStateChanged.m_pList = &pState->m_listDead; break;
			case ACS_COUNTERING:			obStateChanged.m_pList = &pState->m_listCountering; break;
			case ACS_POST_STRIKE_RECOVERY:	obStateChanged.m_pList = &pState->m_listPostStrikeRecovery; break;
			default: ntError_p( false, ("Error: unknown state type defined: %d\n", eState) );
		}									

		m_pAttackRoot->ProcessStateChange( this, obStateChanged );
	}

	// remember the state for next frame. 
	m_bProcessIncomingAttack = false;
	m_bProcessIncomingCounter = false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AICombatComponent::ChooseAwesomeEvadeDirection
//! 
//! Tries to find a good point for the AI to evade to, not brilliant, but better                                                                                         
//------------------------------------------------------------------------------------------
CDirection AICombatComponent::ChooseAwesomeEvadeDirection(AI_ATTACK_TYPE eAttackType) const
{
	// Obtain the radius that the entity will travel.
	float fRadius = 3.5f; // This distance is AI specific, nothing on attack def for it anymore because it's all anim based

	ntAssert(fRadius > 0.0f);
	ntAssert(m_pEntity->GetAIComponent()->GetEntityToAttack());

	// My position
	CPoint ptMyPos = m_pEntity->GetPosition();

	// Targets position
	CPoint ptTargetPos = m_pEntity->GetAIComponent()->GetEntityToAttack()->GetPosition();

	// Generate a direction to the target
	CDirection dirToTarget( ptTargetPos - ptMyPos );

	// Obtain the distance to the target
	float fDistToTarget = dirToTarget.Length();

	// Normalise the direction vector
	dirToTarget /= fDistToTarget;

	// The entity isn't really interested in evading to the target, unless the target
	// is really far, but not too far. 
	if( (fRadius + 1.0f < fDistToTarget ) && (fRadius + 3.0f > fDistToTarget ) )
	{
		CPoint ptDest = ptMyPos + (dirToTarget * fRadius);

		// Check that evade target is valid ( not in a wall or something )
		// 	(Dario) if( CAINavGraphManager::Get().InGraph( ptDest, 1.0f ) && 
		//				!CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, 0.5f) )

		//if ( ( !(CAINavigationSystemMan::Get().IsNewAIActive()) && (CAINavGraphManager::Get().InGraph( ptDest, 1.0f ) && CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, 0.5f)) ) ||
		if ( CAINavigationSystemMan::Get().IsPosValidForFormation(m_pEntity->GetPosition(),ptDest,0.75f) )
			return dirToTarget;
	}

	// The entity is within a known distance of the target. If the entity is
	// too close to the target, just make the entity evade backwards. 
	if( fDistToTarget < 1.0f )
	{
		CPoint ptDest = ptMyPos + (-dirToTarget * fRadius);

		// Check that evade target is valid ( not in a wall or something )
		// (Dario) if( CAINavGraphManager::Get().InGraph( ptDest ) && 
		//			!CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, 0.5f) )
		//( (CAINavigationSystemMan::Get().IsNewAIActive()) && ( m_pVision->IsPosValidForFormation(ptDest)) )

	//	if ( ( !(CAINavigationSystemMan::Get().IsNewAIActive()) && (CAINavGraphManager::Get().InGraph( ptDest ) && CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, 0.5f)) ) ||
		if ( CAINavigationSystemMan::Get().IsPosValidForFormation(m_pEntity->GetPosition(),ptDest,0.75f) )
			return -dirToTarget;
	}

	// Remove the entity
	//CAINavGraphManager::Get().GetRepu

	// Try to find another position to place the entity
	static const float gTestAngles[] = {10.0f, -10.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f}; 
	float afTestAnglesRandom[] = { grandf( 180.0f ) + 90.0f, grandf( 180.0f ) + 90.0f, grandf( 180.0f ) + 90.0f, grandf( 180.0f ) + 90.0f };

	const float *TestAngles = gTestAngles;
	int	TestAngleCount = sizeof(gTestAngles) / sizeof(float);


	if( eAttackType == AAT_EVADE_LEFT )
	{
		static const float gTestAnglesLeft[] = { 90.0f, 100.0f, 80.0f, 110.0f, 70.0f}; 
		TestAngles = gTestAnglesLeft;
		TestAngleCount = sizeof(gTestAnglesLeft) / sizeof(float);
	}
	else if( eAttackType == AAT_EVADE_RIGHT )
	{
		static const float gTestAnglesRight[] = { 270.0f, 280.0f, 260.0f, 290.0f, 250.0f}; 
		TestAngles = gTestAnglesRight;
		TestAngleCount = sizeof(gTestAnglesRight) / sizeof(float);
	}
	else if( eAttackType == AAT_EVADE_BACK )
	{
		static const float gTestAnglesBack[] = { 180.0f, 170.0f, 190.0f, 160.0f, 200.0f }; 
		TestAngles = gTestAnglesBack;
		TestAngleCount = sizeof(gTestAnglesBack) / sizeof(float);
	}
	else if( eAttackType == AAT_EVADE )
	{
		TestAngles = afTestAnglesRandom;
		TestAngleCount = sizeof(afTestAnglesRandom) / sizeof(float);
	}

	for( int iLoop = 0; iLoop < TestAngleCount; ++iLoop )
	{
		// Create a rotation based on the current index. 
		CMatrix obRotation( m_pEntity->GetMatrix().GetYAxis(), TestAngles[ iLoop ] * DEG_TO_RAD_VALUE );
		CDirection dirNewDirection = dirToTarget * obRotation;

		//float fObstructionRadius = 1.8f;
		bool bFalied = false;
		
		//g_VisualDebug->RenderLine( m_pEntity->GetPosition() + CPoint( 0.0f, 1.0f, 0.0f ), m_pEntity->GetPosition() + dirNewDirection + CPoint( 0.0f, 1.0f, 0.0f ), 0xFFFFFFFF );
		// Check that evade target is valid ( not in a wall or something )
		// (Dario) if( CAINavGraphManager::Get().InGraph( ptDest ) && 
		//			!CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, 0.5f) )
		//( (CAINavigationSystemMan::Get().IsNewAIActive()) && ( m_pVision->IsPosValidForFormation(ptDest)) )
/*
		// Check along the direction vector - I've requested that there'll be a system method for performing the same function
		for( float fRadiusCheck = 0.0f; fRadiusCheck <= (fObstructionRadius+fRadius) && !bFalied; fRadiusCheck += fObstructionRadius )
		{
			CPoint ptDest = ptMyPos + (dirNewDirection * fRadiusCheck);

			// Check that evade target is valid ( not in a wall or something )
			//if( !CAINavGraphManager::Get().InGraph( ptDest ) || CAINavGraphManager::Get().GetRepulsion()->IsObstructed(ptDest, fObstructionRadius) )
		}
*/
		// If this didn't fail, then return the direction
		if( !bFalied )
		{
			//CAINavGraphManager::Get().GetRepulsion()->AddEntity( m_pEntity );
			return dirNewDirection;
		}
	}

	// Readd the entity
	//CAINavGraphManager::Get().GetRepulsion()->AddEntity( m_pEntity );

	return -dirToTarget;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsQueueFlush
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsQueueFlush(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (AAT_QUEUE_FLUSH == eLocalType);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsCombatFlush
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsCombatFlush(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (AAT_COMBAT_FLUSH == eLocalType);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsBlock
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsBlock(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_SPEED_BLOCK) || (eLocalType == AAT_POWER_BLOCK) || (eLocalType == AAT_RANGE_BLOCK) || (eLocalType == AAT_GRAB_BLOCK);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsAttack
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsAttack(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return  (eLocalType == AAT_SPEED_FAST) || (eLocalType == AAT_SPEED_MEDIUM) || 
			(eLocalType == AAT_POWER_FAST) || (eLocalType == AAT_POWER_MEDIUM) || 
			(eLocalType == AAT_RANGE_FAST) || (eLocalType == AAT_RANGE_MEDIUM);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsEvade
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsEvade(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_EVADE) || (eLocalType == AAT_EVADE_LEFT) || (eLocalType == AAT_EVADE_RIGHT) || (eLocalType == AAT_EVADE_FORWARD) || (eLocalType == AAT_EVADE_BACK);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsGrab
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsGrab(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_SPEED_GRAB) || (eLocalType == AAT_POWER_GRAB) || (eLocalType == AAT_RANGE_GRAB);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsAction
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsAction(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_ACTION);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsRetToFormation
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsRetToFormation(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_RET_FORMATION);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsLuaCallback
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsLuaCallback(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_LUA_CALLBACK);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsMsgFormation
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsMsgFormation(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_MSG_FORMATION);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsPlayAnim
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsPlayAnim(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_PLAY_ANIM);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	QueuedAttack::IsDirectAttack
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool QueuedAttack::IsDirectAttack(void) const
{
	AI_ATTACK_TYPE eLocalType = static_cast<AI_ATTACK_TYPE>((int)eType & 0xFFFF);
	return (eLocalType == AAT_DIRECT_ATTACK);
}
