//--------------------------------------------------
//!
//!	\file combateffect.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _COMBATEFFECT_H
#define _COMBATEFFECT_H

#include "effect/effect_resourceman.h"

class CEntity;
class CAttackData;
class CombatEffectsTrigger;

//--------------------------------------------------
//!
//!	CombatEffectsDefinition
//!	XML interface that descibs all things combat
//! effecty for a given character.
//!
//--------------------------------------------------
class CombatEffectsDefinition
{
public:
	static bool s_bCombatEffectsDisabled;

	// returns handle to a CombatEffectsTrigger object
	u_int SetupEffectsTrigger( const CEntity* pParent, const CEntity* pTarget, const CAttackData* pData, float fAttackDuration ) const;
	u_int SetupEffectsTrigger( const CEntity* pParent, const char* pFunction, float fEffectDuration, Transform* pobTransform ) const;
	
	// these just help out the combat system somewhat
	static void ReleaseEffectsTrigger( u_int );
	static void AbortEffectsTrigger( u_int );
	static CombatEffectsTrigger* GetEffectsTrigger( u_int );

	void PostConstruct();
	bool JustEdited(CallBackParameter, CallBackParameter);

	// file containing trigger functions used by character
	ntstd::String m_luaFunctions; 
	
	// specify default functions
	ntstd::String m_defaultHitFunc;
	ntstd::String m_defaultHitUninterruptedFunc;
	ntstd::String m_defaultKOFunc;
	ntstd::String m_defaultBlockFunc;
	ntstd::String m_defaultStaggerFunc;

	// specify default attack functions
	ntstd::String m_defaultSFFunc; // AC_SPEED_FAST
	ntstd::String m_defaultSMFunc; // AC_SPEED_MEDIUM
	ntstd::String m_defaultPFFunc; // AC_POWER_FAST
	ntstd::String m_defaultPMFunc; // AC_POWER_MEDIUM
	ntstd::String m_defaultRFFunc; // AC_RANGE_FAST
	ntstd::String m_defaultRMFunc; // AC_RANGE_MEDIUM
	ntstd::String m_defaultGFunc;	// AC_GRAB
	ntstd::String m_defaultEFunc;	// AC_EVADE

private:
	void PCallFunction( const char* pName ) const;
	ScriptResource	m_luaFuncFile;
};

#endif // _COMBATEFFECT_H
