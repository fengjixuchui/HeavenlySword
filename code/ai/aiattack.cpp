//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file attackselection.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "game/entity.h"
#include "game/attacks.h"
#include "ai/aiattack.h"
#include "ai/aiattackselection.h"
#include "objectdatabase/dataobject.h"
#include "input/inputhardware.h"

// borrow this flag on lua to get rid of this message
extern bool g_bEnableLuaLogs;

#ifdef _RELEASE
#define AI_COMBAT_DEBUG( Message ) 
#else
#define AI_COMBAT_DEBUG( Message ) if( m_bEnableDebugPrint && g_bEnableLuaLogs && CInputHardware::Get().GetContext() == INPUT_CONTEXT_AI ) { ntPrintf Message; }
#endif


// Declare editable interface for CAIAttackRoot
START_CHUNKED_INTERFACE	(CAIAttackRoot, Mem::MC_AI)

	IREFERENCE	(CAIAttackRoot, Local)
	IREFERENCE	(CAIAttackRoot, Active)
	IREFERENCE	(CAIAttackRoot, Passive)

	IFLOAT		(CAIAttackRoot, MovementRadiusMin)
	IFLOAT		(CAIAttackRoot, MovementRadiusMax)
	IFLOAT		(CAIAttackRoot, MovementSpeedMin)
	IFLOAT		(CAIAttackRoot, MovementSpeedMax)
	IFLOAT		(CAIAttackRoot, MovementPauseMin)
	IFLOAT		(CAIAttackRoot, MovementPauseMax)
	IFLOAT		(CAIAttackRoot, MovementTargetThreshold)
	IFLOAT		(CAIAttackRoot, AttackRequestMin)
	IFLOAT		(CAIAttackRoot, AttackRequestMax)

	IBOOL		(CAIAttackRoot, DisableAttacks )
	IBOOL		(CAIAttackRoot, EnableDebugPrint )
END_STD_INTERFACE

// Declare editable interface for CAIAttackState
START_CHUNKED_INTERFACE	(CAIAttackState, Mem::MC_AI)
	PUBLISH_PTR_CONTAINER_AS( m_listStandard,				Standard)
	PUBLISH_PTR_CONTAINER_AS( m_listAttacking,				Attacking)
	PUBLISH_PTR_CONTAINER_AS( m_listAttack,					Attack)
	PUBLISH_PTR_CONTAINER_AS( m_listRecoiling,				Recoiling)
	PUBLISH_PTR_CONTAINER_AS( m_listBlocking,				Blocking)
	PUBLISH_PTR_CONTAINER_AS( m_listDeflecting,				Deflecting)
	PUBLISH_PTR_CONTAINER_AS( m_listRiseWait,				RiseWait)
	PUBLISH_PTR_CONTAINER_AS( m_listBlockStaggering,		BlockStaggering)
	PUBLISH_PTR_CONTAINER_AS( m_listImpactStaggering,		ImpactStaggering)
	PUBLISH_PTR_CONTAINER_AS( m_listRecovering,				Recovering)
	PUBLISH_PTR_CONTAINER_AS( m_listFloored,				Floored)
	PUBLISH_PTR_CONTAINER_AS( m_listInstantRecover,			InstantRecover)
	PUBLISH_PTR_CONTAINER_AS( m_listKO,						KO)
	PUBLISH_PTR_CONTAINER_AS( m_listHeld,					Held)
	PUBLISH_PTR_CONTAINER_AS( m_listDying,					Dying)
	PUBLISH_PTR_CONTAINER_AS( m_listDead,					Dead)
	PUBLISH_PTR_CONTAINER_AS( m_listCountering,				Countering)
	PUBLISH_PTR_CONTAINER_AS( m_listPostStrikeRecovery,		PostStrikeRecovery)
END_STD_INTERFACE

// Declare editable interface for CAIAttackData
START_CHUNKED_INTERFACE	(CAIAttackData, Mem::MC_AI)
	ISTRING		(CAIAttackData, Help)
	ISTRING		(CAIAttackData, Debug)
	IFLOAT		(CAIAttackData, Weight)
	IFLOAT		(CAIAttackData, Grace)
	IFLOAT		(CAIAttackData, RangeMin)
	IFLOAT		(CAIAttackData, RangeMax)
	IENUM_d		(CAIAttackData,	Style,				ATTACK_CLASS, AC_COUNT)
	IENUM_d		(CAIAttackData,	LocalState,			COMBAT_STATE, CS_COUNT)
	IINT		(CAIAttackData, TargetAttackCountMin)
	IINT		(CAIAttackData, TargetAttackCountMax)
	IINT		(CAIAttackData, TargetStringDepthMin)
	IINT		(CAIAttackData, TargetStringDepthMax)
	IENUM		(CAIAttackData,	Action1,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance1)
	IENUM		(CAIAttackData,	Action2,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance2)
	IENUM		(CAIAttackData,	Action3,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance3)
	IENUM		(CAIAttackData,	Action4,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance4)
	IENUM		(CAIAttackData,	Action5,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance5)
	IENUM		(CAIAttackData,	Action6,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance6)
	IENUM		(CAIAttackData,	Action7,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance7)
	IENUM		(CAIAttackData,	Action8,	AI_ATTACK_TYPE)
	IFLOAT		(CAIAttackData, ActionStopChance8)
	IBOOL		(CAIAttackData, Disabled)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// CAIAttackRoot::CombatRequestAttack
// Using the defined XML data and the given queries choose an attack and post any results
// back into the given AI Combat Component
//------------------------------------------------------------------------------------------

void CAIAttackRoot::CombatRequestAttack( AICombatComponent& rCombatComp, float fRange, int iTargetAttackCount, int iTargetStringDepth, float fAttackFree ) const
{
	if( m_bDisableAttacks || !m_pobLocal )
		return;

	StateChange obStateChanged;

	obStateChanged.m_fAttackFree		= fAttackFree;
	obStateChanged.m_fRange				= fRange;
	obStateChanged.m_fTime				= 0.0f;
	obStateChanged.m_eStyle				= AC_COUNT;
	obStateChanged.m_eLocalState		= rCombatComp.GetEntity()->GetAttackComponent()->AI_Access_GetState();
	obStateChanged.m_iTargetAttackCount = iTargetAttackCount;
	obStateChanged.m_iTargetStringDepth = iTargetStringDepth;
	obStateChanged.m_pCombatComp		= &rCombatComp;
	obStateChanged.m_pList				= &m_pobLocal->m_listAttack;

	ProcessStateChange( &rCombatComp, obStateChanged );
}

//------------------------------------------------------------------------------------------
// CAIAttackRoot::CombatRequestPostStrikeRecovery
// 
// 
//------------------------------------------------------------------------------------------

void CAIAttackRoot::CombatRequestPostStrikeRecovery( AICombatComponent& rCombatComp, float fRange, float fAttackFree, float fTime, int iTargetAttackCount, int iTargetStringDepth ) const
{
	StateChange obStateChanged;

	obStateChanged.m_fAttackFree		= fAttackFree;
	obStateChanged.m_fRange				= fRange;
	obStateChanged.m_fTime				= fTime;
	obStateChanged.m_eStyle				= AC_COUNT;
	obStateChanged.m_eLocalState		= rCombatComp.GetEntity()->GetAttackComponent()->AI_Access_GetState();
	obStateChanged.m_iTargetAttackCount = iTargetAttackCount;
	obStateChanged.m_iTargetStringDepth = iTargetStringDepth;
	obStateChanged.m_pCombatComp		= &rCombatComp;
	obStateChanged.m_pList				= &m_pobLocal->m_listPostStrikeRecovery;

	ProcessStateChange( &rCombatComp, obStateChanged );

}


//------------------------------------------------------------------------------------------
// CAIAttackRoot::ProcessStateChange
// Emulation of an old Lua combat script, created as a plug-in replacement the function
// takes a structure of variables that helps reduce the number of possible attacks to 
// process.
//------------------------------------------------------------------------------------------
void CAIAttackRoot::ProcessStateChange( const AICombatComponent* pAICombat, const StateChange& rStateChangeTable ) const
{
	uint64_t uiValidAttacks = 0;
	uint64_t uiBitIndex = 0;
	float fWeight = 0.0f;

	// Create an easy to access reference to the list.
	const AIAttackDataList& rList = *rStateChangeTable.m_pList;
	ntError_p( rList.size() < sizeof(uiValidAttacks) * 8, ("The number of attacks can't exceed %d", sizeof(uiValidAttacks) * 8 ) );

	if( !rStateChangeTable.m_pList || !rList.size() )
		return;

	// 
	AI_COMBAT_DEBUG( ("-Entity %s\n", rStateChangeTable.m_pCombatComp->GetEntity()->GetName().c_str() ) );
	
	// for each element in the list.. 
	for( AIAttackDataList::const_iterator obIt( rList.begin() ); obIt != rList.end(); ++obIt, ++uiBitIndex )
	{
		const CAIAttackData* pAttackData = *obIt;

		AI_COMBAT_DEBUG( ("--Trying attack %s %s\n", pAttackData->m_obDebug.c_str(), ntStr::GetString(ObjectDatabase::Get().GetDataObjectFromPointer( pAttackData )->GetName()) ) );

		if( pAttackData->m_bDisabled )
		{
			AI_COMBAT_DEBUG( ("--Disabled\n") );
			continue;
		}

		if( ((int)pAttackData->m_fActionStopChance1 > (int)pAICombat->GetEntity()->AttackSkill()) )
		{
			AI_COMBAT_DEBUG( ("--Skiil level failed. %f %f\n" , pAttackData->m_fActionStopChance1, pAICombat->GetEntity()->AttackSkill() ) );
			continue;
		}

		if( rStateChangeTable.m_fAttackFree < pAttackData->m_fGrace )
		{
			AI_COMBAT_DEBUG( ("--Grace failed %f %f\n" , rStateChangeTable.m_fAttackFree, pAttackData->m_fGrace ) );
			continue;
		}

		if( pAttackData->m_eStyle != AC_COUNT && rStateChangeTable.m_eStyle != pAttackData->m_eStyle )
		{
			AI_COMBAT_DEBUG( ("--Style failed %d %d\n" , rStateChangeTable.m_eStyle, pAttackData->m_eStyle ) );
			continue;
		}

		if( pAttackData->m_eLocalState != CS_COUNT && rStateChangeTable.m_eLocalState != pAttackData->m_eLocalState )
		{
			AI_COMBAT_DEBUG( ("--State failed %d %d\n" , rStateChangeTable.m_eLocalState, pAttackData->m_eLocalState ) );
			continue;
		}

		// Is the attack out of range. 
		if( rStateChangeTable.m_fRange < pAttackData->m_fRangeMin || rStateChangeTable.m_fRange >= pAttackData->m_fRangeMax )
		{
			AI_COMBAT_DEBUG( ("--Range failed %f\n" , rStateChangeTable.m_fRange ) );
			continue;
		}

		// Is the attack count out of range?
		if( rStateChangeTable.m_iTargetAttackCount < pAttackData->m_iTargetAttackCountMin || rStateChangeTable.m_iTargetAttackCount >= pAttackData->m_iTargetAttackCountMax )
		{
			AI_COMBAT_DEBUG( ("--Target attack count failed %d\n" , rStateChangeTable.m_iTargetAttackCount ) );
			continue;
		}

		// Is the attack count out of range?
		if( rStateChangeTable.m_iTargetStringDepth < pAttackData->m_iTargetStringDepthMin || rStateChangeTable.m_iTargetStringDepth >= pAttackData->m_iTargetStringDepthMax )
		{
			AI_COMBAT_DEBUG( ("--Target string depth failed %d\n" , rStateChangeTable.m_iTargetStringDepth ) );
			continue;
		}

		// Sanity Check
		ntAssert( uiBitIndex < (sizeof(uiBitIndex) * 8) );

		// Mark the valid index
		uiValidAttacks |= 1 << uiBitIndex;
		
		// Accumalate the weight
		fWeight += pAttackData->m_fWeight;
	}

	// If there aren't any valid attacks then return now. 
	if( !uiValidAttacks )
		return;

	// Decide on the attack to use
	float fRequestWeight = grandf( fWeight );

	// zero the weight acc
	fWeight = 0.0f;

	// Clear the bit index
	uiBitIndex = 0;

	// Store for the pointer to the new attack
	const CAIAttackData* pNewAttackData = 0;

	// again, run through each element in the list... 
	for( AIAttackDataList::const_iterator obIt( rList.begin() ); obIt != rList.end(); ++obIt, ++uiBitIndex )
	{
		// If the attack isn't valid, then continue
		if( !(uiValidAttacks & 1 << uiBitIndex) )
			continue;

		const CAIAttackData* pAttackData = *obIt;

		// Accumalate the weight
		fWeight += pAttackData->m_fWeight;

		// Is this attack to run?
		if( fWeight >= fRequestWeight )
		{
			pNewAttackData = pAttackData;
			break;
		}
	}

	// No new valid attack, return now.
	if( !pNewAttackData )
		return;

#ifndef _RELEASE
	// Show the debug message if required.
	if( m_bEnableDebugPrint && g_bEnableLuaLogs && pNewAttackData->m_obDebug.length() )
	{
		ntPrintf("Queuing %s %s\n", pNewAttackData->m_obDebug.c_str(), ntStr::GetString(ObjectDatabase::Get().GetDataObjectFromPointer( pNewAttackData )->GetName())  );
	}
#endif

	// Queue the moves.. 
	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction1, pNewAttackData->m_fActionStopChance1) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction1 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction2, pNewAttackData->m_fActionStopChance2) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction2 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction3, pNewAttackData->m_fActionStopChance3) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction3 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction4, pNewAttackData->m_fActionStopChance4) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction4 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction5, pNewAttackData->m_fActionStopChance5) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction5 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction6, pNewAttackData->m_fActionStopChance6) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction6 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction7, pNewAttackData->m_fActionStopChance7) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction7 );

	if( !pAICombat->ShouldQueueMove(pNewAttackData->m_eAction8, pNewAttackData->m_fActionStopChance8) )
		return;
	rStateChangeTable.m_pCombatComp->QueueMove( pNewAttackData->m_eAction8 );
}


//------------------------------------------------------------------------------------------
// 
// 
// 
//------------------------------------------------------------------------------------------
CAIAttackRoot::CAIAttackRoot()
{
	m_pobLocal = m_pobActive = m_pobPassive = 0;

	m_fMovementRadiusMin = 3.0f;
	m_fMovementRadiusMax = 3.5f;
	
	m_fMovementSpeedMin = 0.5f;
	m_fMovementSpeedMax = 0.5f;

	m_fMovementPauseMin = 1.0f;
	m_fMovementPauseMax = 2.0f;

	m_fMovementTargetThreshold = 2.0f;

	m_fAttackRequestMin = 0.0f;
	m_fAttackRequestMax = 1.5f;

	m_bDisableAttacks = false;
}

//------------------------------------------------------------------------------------------
// 
// 
// 
//------------------------------------------------------------------------------------------
CAIAttackData::CAIAttackData()
{
	m_obHelp = "John Li and Kiwi friut";
	m_obDebug = "Attack";

	// What weighting does this combat attack have?
	m_fWeight = 1.0f;

	// Grace is a period that the entity must wait from the last attack from performing this one
	m_fGrace = 0.0f;

	// The style requirement
	m_eStyle = AC_COUNT;

	// Range to the target the entity must be
	m_fRangeMin = 0.0f;
	m_fRangeMax = 0.0f;

	// The number of attackers the target should have
	m_iTargetAttackCountMin = 0;
	m_iTargetAttackCountMax = 1;

	// The depth of the targets combo to trigger this attack
	m_iTargetStringDepthMin = 0;	
	m_iTargetStringDepthMax = 10;

	// Attack actions and stop chances ( 100% will stop the action )
	m_eAction1 = m_eAction2 = m_eAction3 = m_eAction4 = m_eAction5 = m_eAction6 = m_eAction7 = m_eAction8 = AAT_NONE;		
	m_fActionStopChance1 = m_fActionStopChance2 = m_fActionStopChance3 = m_fActionStopChance4 = 
	m_fActionStopChance5 = m_fActionStopChance6 = m_fActionStopChance7 = m_fActionStopChance8 = 0.0f;

	m_bDisabled = false;
}
