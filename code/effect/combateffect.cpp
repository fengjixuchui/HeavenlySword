//--------------------------------------------------
//!
//!	\file combateffect.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "combateffect.h"
#include "combateffects_trigger.h"
#include "game/attacks.h"
#include "objectdatabase/dataobject.h"
#include "game/luaglobal.h"
#include "effect_error.h"
#include "effect_manager.h"

#ifndef _RELEASE
static char errorMSG[512];
#endif

bool CombatEffectsDefinition::s_bCombatEffectsDisabled = false;

START_STD_INTERFACE( CombatEffectsDefinition )
	
	PUBLISH_VAR_AS( m_luaFunctions, LuaFunctionsFile )

	PUBLISH_VAR_AS(	m_defaultHitFunc,				DefaultHit )
	PUBLISH_VAR_AS( m_defaultHitUninterruptedFunc,	DefaultUninterruptedHit)
	PUBLISH_VAR_AS(	m_defaultKOFunc,				DefaultKO )
	PUBLISH_VAR_AS(	m_defaultBlockFunc,				DefaultBlock )
	PUBLISH_VAR_AS(	m_defaultStaggerFunc,			DefaultStagger )
    
	PUBLISH_VAR_AS(	m_defaultSFFunc,	DefaultSpeedFast )
	PUBLISH_VAR_AS(	m_defaultSMFunc,	DefaultSpeedMedium )
	PUBLISH_VAR_AS(	m_defaultPFFunc,	DefaultPowerFast )
	PUBLISH_VAR_AS(	m_defaultPMFunc,	DefaultPowerMedium )
	PUBLISH_VAR_AS(	m_defaultRFFunc,	DefaultRangeFast )
	PUBLISH_VAR_AS(	m_defaultRMFunc,	DefaultRangeMedium )
	PUBLISH_VAR_AS(	m_defaultGFunc,		DefaultGrab )
	PUBLISH_VAR_AS(	m_defaultEFunc,		DefaultEvade )

	DECLARE_POSTCONSTRUCT_CALLBACK(		PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK(	JustEdited )
			
END_STD_INTERFACE

//--------------------------------------------------
//!
//! post construct call back checks to see if script file
//! is presnet and loads it into the global lua state
//!
//--------------------------------------------------
void CombatEffectsDefinition::PostConstruct()
{
	if (!ntStr::IsNull(m_luaFunctions))
		m_luaFuncFile.SetFile( ntStr::GetString(m_luaFunctions) );
}

bool CombatEffectsDefinition::JustEdited(CallBackParameter param, CallBackParameter)
{
	CHashedString pField(param);

	if (HASH_STRING_LUAFUNCTIONSFILE == pField)
	{
		PostConstruct();
	}

	return true;
}

//--------------------------------------------------
//!
//! PCallFunction
//! Call this lua function if its present in a
//! safe manner
//!
//--------------------------------------------------
void CombatEffectsDefinition::PCallFunction( const char* pName ) const
{
	ntError( pName );

	lua_State* L = &(*CLuaGlobal::Get().State());

	// store stack top for restore
	int stackTop = lua_gettop(L);

	lua_getglobal(L, pName);
	if (lua_isfunction(L, -1))
	{
		// all of our pcall effect construc functions should 
		// by definiton take no arguments and return no 
		// results, so we just call it

		if (lua_pcall(L, 0, 0, 0) != 0)
		{
			#ifndef _RELEASE
			// whoops, failed in the call
			sprintf( errorMSG, "ERROR calling lua function '%s': %s\n", pName, lua_tostring(L, -1) );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif
		}
	}
#ifndef _RELEASE
	else if (stricmp(pName,"empty") != 0)
	{
		sprintf( errorMSG, "ERROR: Cannot find lua function '%s'\n", pName );
		EffectErrorMSG::AddDebugError( errorMSG );
	}
#endif

	// restore stack
	lua_settop(L, stackTop);
}

//--------------------------------------------------
//!
//! We create a new CombatEffectsTrigger, then call
//! all our revelant lua functions to configure
//! this particular combat effects trigger. Once
//! its configed, we finalise it and add to the 
//! effect manager.
//!
//--------------------------------------------------
u_int CombatEffectsDefinition::SetupEffectsTrigger( const CEntity* pParent, const CEntity* pTarget, const CAttackData* pData, float fAttackDuration ) const
{
	if (s_bCombatEffectsDisabled)
		return 0xffffffff;

	ntError( pData );

	CombatEffectsTrigger* pCurrConfigObject = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CombatEffectsTrigger( pParent, pTarget, fAttackDuration );

	// now configure our trigger object
	if ( ntStr::IsNull(pData->m_obEffectsScript))
	{
		switch( pData->m_eAttackClass )
		{
		case AC_SPEED_FAST:		if (!ntStr::IsNull(m_defaultSFFunc)) PCallFunction( ntStr::GetString( m_defaultSFFunc ) ); break;
		case AC_SPEED_MEDIUM:	if (!ntStr::IsNull(m_defaultSMFunc)) PCallFunction( ntStr::GetString( m_defaultSMFunc ) ); break;
		case AC_POWER_FAST:		if (!ntStr::IsNull(m_defaultPFFunc)) PCallFunction( ntStr::GetString( m_defaultPFFunc ) ); break;
		case AC_POWER_MEDIUM:	if (!ntStr::IsNull(m_defaultPMFunc)) PCallFunction( ntStr::GetString( m_defaultPMFunc ) ); break;
		case AC_RANGE_FAST:		if (!ntStr::IsNull(m_defaultRFFunc)) PCallFunction( ntStr::GetString( m_defaultRFFunc ) ); break;
		case AC_RANGE_MEDIUM:	if (!ntStr::IsNull(m_defaultRMFunc)) PCallFunction( ntStr::GetString( m_defaultRMFunc ) ); break;
		
		case AC_GRAB_GOTO:	
		case AC_GRAB_HOLD:	
		case AC_GRAB_STRIKE:	
			if (!ntStr::IsNull(m_defaultGFunc)) PCallFunction( ntStr::GetString( m_defaultGFunc ) ); break;
		case AC_EVADE:			if (!ntStr::IsNull(m_defaultEFunc)) PCallFunction( ntStr::GetString( m_defaultEFunc ) ); break;

		default:
			// dont add anything for unclassified attack
			break;
		}

		if (!ntStr::IsNull(m_defaultHitFunc))
			PCallFunction( ntStr::GetString( m_defaultHitFunc ) );

		if (!ntStr::IsNull(m_defaultHitUninterruptedFunc))
			PCallFunction( ntStr::GetString( m_defaultHitUninterruptedFunc ) );

		if (!ntStr::IsNull(m_defaultKOFunc))
			PCallFunction( ntStr::GetString( m_defaultKOFunc ) );

		if (!ntStr::IsNull(m_defaultBlockFunc))
			PCallFunction( ntStr::GetString( m_defaultBlockFunc ) );

		if (!ntStr::IsNull(m_defaultStaggerFunc))
			PCallFunction( ntStr::GetString( m_defaultStaggerFunc ) );
	}
	else
	{
		PCallFunction( ntStr::GetString( pData->m_obEffectsScript ) );
	}

	// object should be configured, add to the effect manager & return;
	pCurrConfigObject->FinaliseConstruct();
	u_int iEffectID = EffectManager::Get().AddEffect( pCurrConfigObject );

	return iEffectID;
}

//--------------------------------------------------
//!
//! Simliar to the above method, but allows you to 
//! configure a set of effects specified by a particular
//! function.
//!
//--------------------------------------------------
u_int CombatEffectsDefinition::SetupEffectsTrigger( const CEntity* pParent, const char* pFunction, float fEffectDuration, Transform* pobUseTransform ) const
{
	if (s_bCombatEffectsDisabled)
		return 0xffffffff;

	if ((!pFunction) || (strcmp(pFunction,"NULL") == 0))
		return 0xffffffff;

	CombatEffectsTrigger* pCurrConfigObject = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CombatEffectsTrigger( pParent, 0, fEffectDuration );
	pCurrConfigObject->SetUseTransform(pobUseTransform);
	PCallFunction( pFunction );
	
	// object should be configured, add to the effect manager & return;
	pCurrConfigObject->FinaliseConstruct();
	u_int iEffectID = EffectManager::Get().AddEffect( pCurrConfigObject );

	return iEffectID;
}

//--------------------------------------------------
//!
//! find and release this effect trigger if its still about
//!
//--------------------------------------------------
void CombatEffectsDefinition::ReleaseEffectsTrigger( u_int iEffectID )
{
	Effect* pEffect = EffectManager::Get().GetEffect( iEffectID );
	if (pEffect)
		pEffect->KillMeWhenReady();
}

//--------------------------------------------------
//!
//! find and kill this effect trigger if its still about
//!
//--------------------------------------------------
void CombatEffectsDefinition::AbortEffectsTrigger( u_int iEffectID )
{
	Effect* pEffect = EffectManager::Get().GetEffect( iEffectID );
	if (pEffect)
		pEffect->KillMeNow();
}

//--------------------------------------------------
//!
//! find and return effect if its still about. 
//!
//--------------------------------------------------
CombatEffectsTrigger* CombatEffectsDefinition::GetEffectsTrigger( u_int iEffectID )
{
	Effect* pEffect = EffectManager::Get().GetEffect( iEffectID );

	if (pEffect)
		return static_cast<CombatEffectsTrigger*>(pEffect);

	return NULL;
}
