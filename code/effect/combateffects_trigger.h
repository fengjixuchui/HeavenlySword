//--------------------------------------------------
//!
//!	\file combateffects_trigger.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _CEFFECTS_TRIGGER_H
#define _CEFFECTS_TRIGGER_H

#include "effect.h"
#include "game/luaglobal.h"

class CEntity;
class CombatEffectsDefinition;
class CE_EventTriggerNode;
class Transform;
class CE_ImpactTriggerNode;

//--------------------------------------------------
//!
//!	CombatEffectsTrigger
//!	Class that's configured by lua scripts called
//! by the combat system. Acts as the interface
//! between combat system and effects.
//!
//--------------------------------------------------
class CombatEffectsTrigger : public Effect
{
public:
	friend class CombatEffectsDefinition;
	~CombatEffectsTrigger();

	virtual bool UpdateEffect();
	virtual bool WaitingForResources() const { return false; }
	virtual void ForceNextTimeDelta( float fTimeDelta );

	void TriggerHitEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerHitUninterruptableEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerKOEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerBlockEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerStaggerEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerDeathEffect( const CEntity* pAttacker, const CEntity* pAttackee );
	void TriggerInvulnerableEffect( const CEntity* pAttacker, const CEntity* pAttackee );

	//DGF
	static void SetUseTransform(Transform* pobT);

	// functions exposed to lua, designed to be called after we've been
	// constructed but before we've been finalised.
	static void Register();

	static int CFX_SetupTrailEffect( NinjaLua::LuaState* pState );
	static int CFX_SetupParticleEffect( NinjaLua::LuaState* pState );
	static int CFX_SetupWakeEffect( NinjaLua::LuaState* pState );
	
	static int CFX_AddHitEffect( NinjaLua::LuaState* pState );
	static int CFX_AddUninterruptEffect( NinjaLua::LuaState* pState );
	static int CFX_AddKOEffect( NinjaLua::LuaState* pState );
	static int CFX_AddBlockEffect( NinjaLua::LuaState* pState );
	static int CFX_AddStaggerEffect( NinjaLua::LuaState* pState );
	static int CFX_AddDeathEffect( NinjaLua::LuaState* pState );
	static int CFX_AddInvulnerableEffect( NinjaLua::LuaState* pState );
	
	static int CFX_DummyFunc( NinjaLua::LuaState* ) { return 0; }

private:
	typedef ntstd::List<CE_ImpactTriggerNode*, Mem::MC_EFFECTS>	ImpactList;
	typedef ntstd::List<CE_EventTriggerNode*, Mem::MC_EFFECTS>	EventList;
	
	typedef ntstd::List<u_int, Mem::MC_EFFECTS>					EffectList;

	static void AddImpactEffectToList( NinjaLua::LuaState* pState, ImpactList& list, const char* pDebugContext );
	static void FireTriggerList( ImpactList& list, const CEntity* pAttacker, const CEntity* pAttackee );

	// only CombatEffectsDefinition can use these
	CombatEffectsTrigger(	const CEntity* pParent,
							const CEntity* pTarget,
							float fAttackDuration ); 
	void FinaliseConstruct();

	// global static that is used during our construction period
	static CombatEffectsTrigger* g_pCurrConfigObject;
	// DGF - Added this for strike volumes, so attackcomponent can pass in a transform pointer that isn't on the parent to use for the effect
	static Transform* m_pobUseTransform; // Only used in CFX_SetupWakeEffect for the moment

	// accessors for common lua table ops
	static bool GetTableNumber( NinjaLua::LuaObject& table, const char* pName, float& fResult );
	static bool GetTableString( NinjaLua::LuaObject& table, const char* pName, const char** ppName );
	
	// should be the time this attack lasts, we trigger all our events
	// using normalised times based on this 

	float	m_fAttackDuration;
	float	m_fEventTimer;
	bool	m_bFinalised;

	// one shot firing effects
	ImpactList		m_hitEffects;
	ImpactList		m_noInterruptEffects;
	ImpactList		m_KOEffects;
	ImpactList		m_blockEffects;
	ImpactList		m_staggerEffects;
	ImpactList		m_deathEffects;
	ImpactList		m_invulnerableEffects;

	// owned / attached event based effects
	EventList		m_events;
	EffectList		m_ownedEffects;

	const CEntity*	m_pParent;
	const CEntity*	m_pTarget;
};

#endif //_CEFFECTS_TRIGGER_H
