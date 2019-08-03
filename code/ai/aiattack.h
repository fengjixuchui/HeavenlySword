//------------------------------------------------------------------------------------------
//!
//!	\file aiattack.h
//!
//------------------------------------------------------------------------------------------


#ifndef _AIATTACK_H
#define _AIATTACK_H



//------------------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------------------

#include "editable/enums_ai.h"
#include "game/randmanager.h"

class CAIAttackRoot;
class CAIAttackState;
class CAIAttackData;
class AICombatComponent;

typedef ntstd::List<CAIAttackData*, Mem::MC_AI> AIAttackDataList;

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

class CAIAttackRoot
{
	HAS_INTERFACE( CAIAttackRoot )
public:


	struct StateChange
	{
		float			m_fRange;
		float			m_fTime;
		float			m_fAttackFree;
		ATTACK_CLASS	m_eStyle;
		COMBAT_STATE	m_eLocalState;
		int				m_iTargetAttackCount;
		int				m_iTargetStringDepth;

		AICombatComponent*		m_pCombatComp;
		const AIAttackDataList* m_pList;
	};

	CAIAttackRoot();

	// Return a random attack time
	float RequestAttackTime( void ) const { return m_fAttackRequestMin + grandf( m_fAttackRequestMax - m_fAttackRequestMin ); }

	// Return a random attack radius
	float RequestMovementRadius( void ) const { return m_fMovementRadiusMin + grandf( m_fMovementRadiusMax - m_fMovementRadiusMin ); }

	// Return a random attack radius
	float RequestMovementSpeed( void ) const { return m_fMovementSpeedMin + grandf( m_fMovementSpeedMax - m_fMovementSpeedMin ); }

	// Return a random attack pause
	float RequestMovementPause( void ) const { return m_fMovementPauseMin + grandf( m_fMovementPauseMax - m_fMovementPauseMin ); }

	// Return the distance the target has to move from their current position to tigger this entity to move
	float RequestMovementTargetThreshold( void ) const { return m_fMovementTargetThreshold; }

	// Request an attack
	void CombatRequestAttack( AICombatComponent& rCombatComp, float fRange, int iTargetAttackCount, int iTargetStringDepth, float fAttackFree ) const;

	// Request an post strike recovery
	void CombatRequestPostStrikeRecovery( AICombatComponent& rCombatComp, float fRange, float fAttackFree, float fTime, int iTargetAttackCount, int iTargetStringDepth ) const;

	// Process the state change
	void ProcessStateChange( const AICombatComponent* pAICombat, const StateChange& rStateChangeTable ) const;

	// 
	const CAIAttackState* GetLocalState(void) const { return m_pobLocal; }
	const CAIAttackState* GetActiveState(void) const { return m_pobActive; }
	const CAIAttackState* GetPassiveState(void) const { return m_pobPassive; }


private:

	CAIAttackState*		m_pobLocal;
	CAIAttackState*		m_pobActive;
	CAIAttackState*		m_pobPassive;

	float				m_fMovementRadiusMin;
	float				m_fMovementRadiusMax;
	
	float				m_fMovementSpeedMin;
	float				m_fMovementSpeedMax;

	float				m_fMovementPauseMin;
	float				m_fMovementPauseMax;

	float				m_fMovementTargetThreshold;

	float				m_fAttackRequestMin;
	float				m_fAttackRequestMax;

	// Debug features
	bool				m_bDisableAttacks;
	bool				m_bEnableDebugPrint;
};

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

class CAIAttackState
{
	HAS_INTERFACE( CAIAttackState )
public:

	AIAttackDataList			m_listStandard;
	AIAttackDataList			m_listAttacking;
	AIAttackDataList			m_listAttack;
	AIAttackDataList			m_listRecoiling;
	AIAttackDataList			m_listBlocking;
	AIAttackDataList			m_listDeflecting;
	AIAttackDataList			m_listRiseWait;
	AIAttackDataList			m_listBlockStaggering;
	AIAttackDataList			m_listImpactStaggering;
	AIAttackDataList			m_listRecovering;
	AIAttackDataList			m_listFloored;
	AIAttackDataList			m_listInstantRecover;
	AIAttackDataList			m_listKO;
	AIAttackDataList			m_listHeld;
	AIAttackDataList			m_listDying;
	AIAttackDataList			m_listDead;
	AIAttackDataList			m_listCountering;
	AIAttackDataList			m_listPostStrikeRecovery;
};

//------------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------------

class CAIAttackData
{
	HAS_INTERFACE( CAIAttackData )
public:

	CAIAttackData();

	// Helper field
	ntstd::String	m_obHelp; 

	// Debug Name
	ntstd::String	m_obDebug; 

	// What weighting does this combat attack have?
	float m_fWeight;

	// Grace is a period that the entity must wait from the last attack from performing this one
	float m_fGrace;

	// The style the target is in
	ATTACK_CLASS m_eStyle;

	// State this entity is in
	COMBAT_STATE m_eLocalState;

	// Range to the target the entity must be
	float m_fRangeMin;				float m_fRangeMax;

	// The number of attackers the target should have
	int m_iTargetAttackCountMin;	int m_iTargetAttackCountMax;

	// The depth of the targets combo to trigger this attack
	int m_iTargetStringDepthMin;	int m_iTargetStringDepthMax;

	// Attack actions and stop chances ( 100% will stop the action )
	AI_ATTACK_TYPE m_eAction1;		float m_fActionStopChance1;
	AI_ATTACK_TYPE m_eAction2;		float m_fActionStopChance2;
	AI_ATTACK_TYPE m_eAction3;		float m_fActionStopChance3;
	AI_ATTACK_TYPE m_eAction4;		float m_fActionStopChance4;
	AI_ATTACK_TYPE m_eAction5;		float m_fActionStopChance5;
	AI_ATTACK_TYPE m_eAction6;		float m_fActionStopChance6;
	AI_ATTACK_TYPE m_eAction7;		float m_fActionStopChance7;
	AI_ATTACK_TYPE m_eAction8;		float m_fActionStopChance8;

	// 
	bool	m_bDisabled;
};


#endif //_AIATTACK_H

