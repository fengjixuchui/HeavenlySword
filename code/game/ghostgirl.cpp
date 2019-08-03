//------------------------------------------------------------------------------------------
//!
//!	ghostgirl.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/ghostgirl.h"
#include "game/entity.h"
#include "game/entity.inl"

#include "effect/effect_manager.h"
#include "effect/effect_util.h"
#include "effect/emission_function.h"
#include "effect/psystem_utils.h"

#include "objectdatabase/dataobject.h"

#include "core/visualdebugger.h"

struct GGEffectDef
{
	GGEffectDef( const char* p1, const char* p2, const char* p3 ) :
		m_pEffectDef(p1), m_pEmitterDef(p2), m_pJointName(p3) {};

	const char* m_pEffectDef;
	const char* m_pEmitterDef;
	const char* m_pJointName;

	uint32_t Create( const CEntity* pParentEnt ) const
	{
		// retrieve definitions from object database and make effects
		Transform* pTransform = EffectUtils::FindNamedTransform( pParentEnt, m_pJointName );
		ntAssert_p( pTransform != 0, ("Transform %s not found on entity %s\n", m_pJointName, pParentEnt->GetName().c_str() ) );
		if (pTransform == 0)
			return 0xffffffff;

		void* pEffectDef = ObjectDatabase::Get().GetPointerFromName<void*>( m_pEffectDef );
		ntAssert_p( pEffectDef != 0, ("Effect definition %s not found\n", m_pEffectDef ) );
		if (pEffectDef == 0)
			return 0xffffffff;

		EmitterDef* pEmitterDef = ObjectDatabase::Get().GetPointerFromName<EmitterDef*>( m_pEmitterDef );
		ntAssert_p( pEmitterDef != 0, ("Emitter definition %s not found\n", m_pEmitterDef ) );
		if (pEmitterDef == 0)
			return 0xffffffff;

		return PSystemUtils::ConstructParticleEffect( pEffectDef, pTransform, pEmitterDef );
	}

};

GGEffectDef	g_GhostGirlWhite[] = 
{
	// world space trail
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_HeadEmitterDef",		"head" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_TorsoEmitterDef",		"spine_00" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_ShoulderEmitterDef",	"spine_02" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_ChestEmitterDef",		"spine_02" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_LThighEmitterDef",	"l_leg" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_LShinEmitterDef", 	"l_knee" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_RThighEmitterDef", 	"r_leg" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_RShinEmitterDef", 	"r_knee" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_LArmEmitterDef", 		"l_arm" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_LForearmEmitterDef", 	"l_elbow" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_RArmEmitterDef", 		"r_arm" ),
	GGEffectDef( "GhostGirl_FlamesDef",	"GhostGirl_Flames_RForearmEmitterDef", 	"r_elbow" ),

	// core flames effects
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_HeadEmitterDef",		"head" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_TorsoEmitterDef",		"spine_00" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_ShoulderEmitterDef",	"spine_02" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_ChestEmitterDef",		"spine_02" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_LThighEmitterDef",	"l_leg" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_LShinEmitterDef", 	"l_knee" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_RThighEmitterDef", 	"r_leg" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_RShinEmitterDef", 	"r_knee" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_LArmEmitterDef", 		"l_arm" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_LForearmEmitterDef", 	"l_elbow" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_RArmEmitterDef", 		"r_arm" ),
	GGEffectDef( "GhostGirl_FlamesCoreDef",	"GhostGirl_FlamesCore_RForearmEmitterDef", 	"r_elbow" ),

	// little sparkler bit
	GGEffectDef( "GhostGirl_SparksDef",	"GhostGirl_SparksEmitterDef", 	"spine_00" ),

	GGEffectDef( NULL, NULL, NULL ),
};

GGEffectDef	g_GhostGirlBlue[] = 
{
	GGEffectDef( NULL, NULL, NULL ),
};

GGEffectDef	g_GhostGirlRed[] = 
{
	GGEffectDef( NULL, NULL, NULL ),
};




//------------------------------------------------------------------------------------------
//!
//!	GhostGirlController::ctor
//! Init our contoller class
//!
//------------------------------------------------------------------------------------------
GhostGirlController::GhostGirlController( GG_EFFECT_TYPE type, CEntity* pParentEnt, NSEnt* pNSEnt ) :
	m_type( type ),
	m_pParentEnt( pParentEnt ),
	m_pNSEnt( pNSEnt ),
	m_active( false )
{
	ntAssert_p( m_pParentEnt, ("Must have a valid source entity for this effect") );
	ntAssert_p( m_pNSEnt, ("Must have a valid NS entity for this to work") );

	Effect* pSubEffect = EffectManager::Get().GetEffect( m_effectID );
	if (pSubEffect)
		pSubEffect->KillMeNow();
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlController::dtor
//! Make sure any sub effects created by our owning NSEnt are cleaned up
//!
//------------------------------------------------------------------------------------------
GhostGirlController::~GhostGirlController()
{
	if (m_active)
	{
		Effect* pSubEffect = EffectManager::Get().GetEffect( m_effectID );
		if (pSubEffect)
			pSubEffect->KillMeNow();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlController::Activate
//! Fire off the correct type of effect
//!
//------------------------------------------------------------------------------------------
void GhostGirlController::Activate()
{
	ntAssert_p( m_active == false, ("This effect is already active") );
	
	switch( m_type )
	{
	case GG_WHITE:	m_effectID = EffectManager::Get().AddEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) GhostGirlWhite( m_pParentEnt, m_pNSEnt ) ); break;
	case GG_BLUE:	m_effectID = EffectManager::Get().AddEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) GhostGirlBlue( m_pParentEnt, m_pNSEnt ) ); break;
	case GG_RED:	m_effectID = EffectManager::Get().AddEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) GhostGirlRed( m_pParentEnt, m_pNSEnt ) ); break;
	}

	m_active = true;
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlController::Deactivate
//! Clean up our effects
//!
//------------------------------------------------------------------------------------------
void GhostGirlController::Deactivate()
{
	ntAssert_p( m_active == true, ("This effect is not already active") );
	
	Effect* pSubEffect = EffectManager::Get().GetEffect( m_effectID );
	if (pSubEffect)
		pSubEffect->KillMeWhenReady();

	m_active = false;
}




//------------------------------------------------------------------------------------------
//!
//!	GhostGirlWhite::ctor
//! Retrieve hard coded effect definitions, setup triggers and attach to parent entity
//!
//------------------------------------------------------------------------------------------
GhostGirlWhite::GhostGirlWhite( CEntity* pParentEnt, NSEnt* pNSEnt ) :
	m_pParentEnt( pParentEnt ),
	m_pNSEnt( pNSEnt )
{
	int curr = 0;
	while ( g_GhostGirlWhite[curr].m_pEffectDef != NULL )
	{
		// add sub effects to internal list for routing of control commands later
		uint32_t effectID = g_GhostGirlWhite[curr].Create( m_pParentEnt );
		
		if( effectID != 0xffffffff )
			m_subEffects.push_back( effectID );

		// move to next definition
		curr++;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlWhite::update
//! Pass on death commands to sub effects if required
//!
//------------------------------------------------------------------------------------------
bool GhostGirlWhite::UpdateEffect()
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf2D(500.0f, 250.0f, 0xffffffff, 0, ("GHOST GIRL WHITE ACTIVE") );
#endif

	if (m_bKillMeNow)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeNow();
		}

		return true;
	}

	if (m_bKillMeRequested)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeWhenReady();
		}

		// we're done here, finish up
		return true;
	}

	return false;
}




//------------------------------------------------------------------------------------------
//!
//!	GhostGirlBlue::ctor
//! Retrieve hard coded effect definitions, setup triggers and attach to parent entity
//!
//------------------------------------------------------------------------------------------
GhostGirlBlue::GhostGirlBlue( CEntity* pParentEnt, NSEnt* pNSEnt ) :
	m_pParentEnt( pParentEnt ),
	m_pNSEnt( pNSEnt )
{
	ntAssert_p( m_pParentEnt, ("Must have a valid source entity for this effect") );
	ntAssert_p( m_pNSEnt, ("Must have a valid NS entity for this to work") );

	int curr = 0;
	while ( g_GhostGirlWhite[curr].m_pEffectDef != NULL )
//	while ( g_GhostGirlBlue[curr].m_pEffectDef != NULL )
	{
		// add sub effects to internal list for routing of control commands later
		uint32_t effectID = g_GhostGirlWhite[curr].Create( m_pParentEnt );
		
		if( effectID != 0xffffffff )
			m_subEffects.push_back( effectID );

		// move to next definition
		curr++;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlBlue::update
//! Pass on death commands to sub effects if required
//!
//------------------------------------------------------------------------------------------
bool GhostGirlBlue::UpdateEffect()
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf2D(500.0f, 272.0f, 0xffffffff, 0, ("GHOST GIRL BLUE ACTIVE") );
#endif

	if (m_bKillMeNow)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeNow();
		}

		return true;
	}

	if (m_bKillMeRequested)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeWhenReady();
		}

		// we're done here, finish up
		return true;
	}

	return false;
}




//------------------------------------------------------------------------------------------
//!
//!	GhostGirlRed::ctor
//! Retrieve hard coded effect definitions, setup triggers and attach to parent entity
//!
//------------------------------------------------------------------------------------------
GhostGirlRed::GhostGirlRed( CEntity* pParentEnt, NSEnt* pNSEnt ) :
	m_pParentEnt( pParentEnt ),
	m_pNSEnt( pNSEnt )
{
	ntAssert_p( m_pParentEnt, ("Must have a valid source entity for this effect") );
	ntAssert_p( m_pNSEnt, ("Must have a valid NS entity for this to work") );

	int curr = 0;
	while ( g_GhostGirlWhite[curr].m_pEffectDef != NULL )
//	while ( g_GhostGirlRed[curr].m_pEffectDef != NULL )
	{
		// add sub effects to internal list for routing of control commands later
		uint32_t effectID = g_GhostGirlWhite[curr].Create( m_pParentEnt );
		
		if( effectID != 0xffffffff )
			m_subEffects.push_back( effectID );

		// move to next definition
		curr++;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	GhostGirlRed::update
//! Pass on death commands to sub effects if required
//!
//------------------------------------------------------------------------------------------
bool GhostGirlRed::UpdateEffect()
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf2D(500.0f, 294.0f, 0xffffffff, 0, ("GHOST GIRL RED ACTIVE") );
#endif

	if (m_bKillMeNow)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeNow();
		}

		return true;
	}

	if (m_bKillMeRequested)
	{
		for (	ntstd::List<uint32_t>::iterator it = m_subEffects.begin();
				it != m_subEffects.end(); ++it )
		{
			Effect* pSubEffect = EffectManager::Get().GetEffect( *it );
			if (pSubEffect)
				pSubEffect->KillMeWhenReady();
		}

		// we're done here, finish up
		return true;
	}

	return false;
}

