//--------------------------------------------------
//!
//!	\file combateffects_trigger.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "combateffects_trigger.h"
#include "combateffect_triggernode.h"
#include "effect_error.h"
#include "effect_manager.h"
#include "effect_util.h"
#include "game/luaglobal.h"
#include "game/luahelp.h"
#include "objectdatabase/dataobject.h"
#include "effecttrail_simple.h"

// global construction object
CombatEffectsTrigger* CombatEffectsTrigger::g_pCurrConfigObject = 0;
Transform* CombatEffectsTrigger::m_pobUseTransform = 0;

#ifndef _RELEASE
static char errorMSG[512];
#endif

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::CombatEffectsTrigger
//! ctor can only be called within 
//! CombatEffectsDefinition::SetupEffectsTrigger
//!
//--------------------------------------------------
CombatEffectsTrigger::CombatEffectsTrigger( const CEntity* pParent, const CEntity* pTarget, float fAttackDuration ) :
	m_fAttackDuration( fAttackDuration ),
	m_fEventTimer( 0.0f ),
	m_bFinalised( false ),
	m_pParent( pParent ),
	m_pTarget( pTarget ) // this may be null,
{
	ntAssert( m_pParent );
	ntError_p( g_pCurrConfigObject == 0, ("can only construct one of these at a time") );
	g_pCurrConfigObject = this;
	m_pobUseTransform = 0;
}
 
void CombatEffectsTrigger::SetUseTransform(Transform* pobT)
{
	m_pobUseTransform = pobT; 
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::FinaliseConstruct
//! ctor can only be called within 
//! CombatEffectsDefinition::SetupEffectsTrigger
//!
//--------------------------------------------------
void CombatEffectsTrigger::FinaliseConstruct()
{
	ntError_p( m_bFinalised == false, ("soemthing has gone badly wrong here") ) ;
	ntError_p( g_pCurrConfigObject == this, ("soemthing has gone badly wrong here") ) ;
	g_pCurrConfigObject = 0;
	m_pobUseTransform = 0;
	m_bFinalised = true;
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::~CombatEffectsTrigger
//! Release any sub effects we're looking after
//!
//--------------------------------------------------
CombatEffectsTrigger::~CombatEffectsTrigger()
{
	while ( !m_hitEffects.empty() )			{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_hitEffects.back() );			m_hitEffects.pop_back(); }
	while ( !m_noInterruptEffects.empty() )	{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_noInterruptEffects.back() );	m_noInterruptEffects.pop_back(); }
	while ( !m_KOEffects.empty() )			{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_KOEffects.back() );			m_KOEffects.pop_back(); }
	while ( !m_blockEffects.empty() )		{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_blockEffects.back() );		m_blockEffects.pop_back(); }
	while ( !m_staggerEffects.empty() )		{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_staggerEffects.back() );		m_staggerEffects.pop_back(); }
	while ( !m_deathEffects.empty() )		{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_deathEffects.back() );		m_deathEffects.pop_back(); }
	while ( !m_invulnerableEffects.empty() ){ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_invulnerableEffects.back() );	m_invulnerableEffects.pop_back(); }
	while ( !m_events.empty() )				{ NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_events.back() );				m_events.pop_back(); }

	// release effects
	// note this is valid to do here, providing
	// we're always ahead of child effects in the
	// effect managers level cleanup code, which we should be

	while ( !m_ownedEffects.empty() )
	{
		Effect* pEffect = EffectManager::Get().GetEffect( m_ownedEffects.back() );
		if (pEffect)
			pEffect->KillMeWhenReady();

		m_ownedEffects.pop_back();
	}
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::UpdateEffect
//! See if we need to trigger any of our sub lists
//!
//--------------------------------------------------
bool CombatEffectsTrigger::UpdateEffect()
{
	// its perfectly valid to have m_fEventTimer larger than m_fAttackDuration
	// as its upto the combat sytem when this trigger is finished (say if
	// trails link into a different animation / attack )
	m_fEventTimer += GetNextTimeDelta();

	// update our events 
	float fEventTimeN = m_fEventTimer / m_fAttackDuration;

	for( EventList::iterator it = m_events.begin(); it != m_events.end();  )
	{
		// see if this event should fire
		CE_EventTriggerNode* pChildEvent = 0;
		if ( (*it)->FireIfReady( fEventTimeN, m_ownedEffects, &pChildEvent ) )
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, *it );
			it = m_events.erase( it );

			// event created a new event, push on the end, sort out iterators
			if (pChildEvent)
			{
				int iPosition = std::distance( m_events.begin(), it );
				m_events.push_back( pChildEvent );
				it = m_events.begin();
				std::advance( it, iPosition );
			}
		}
		else
		{
			++it;
		}
	}
	
	if (m_bKillMeRequested)
	{
		// let other sub effects go free and kill ourselves
		for(	EffectList::iterator it = m_ownedEffects.begin();
				it != m_ownedEffects.end(); ++it )
		{
			Effect* pEffect = EffectManager::Get().GetEffect( *it );
			if (pEffect)
				pEffect->KillMeWhenReady();
		}

		return true;
	}

	if (m_bKillMeNow)
	{
		// kill all sub effects and kill ourselves
		for(	EffectList::iterator it = m_ownedEffects.begin();
				it != m_ownedEffects.end(); ++it )
		{
			Effect* pEffect = EffectManager::Get().GetEffect( *it );
			if (pEffect)
				pEffect->KillMeNow();
		}

		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::ForceNextTimeDelta
//! force any attatched sub effects to use the same
//! time change as us
//!
//--------------------------------------------------
void CombatEffectsTrigger::ForceNextTimeDelta( float fTimeDelta )
{
	Effect::ForceNextTimeDelta( fTimeDelta );
	
	for(	EffectList::iterator it = m_ownedEffects.begin();
			it != m_ownedEffects.end(); )
	{
		Effect* pEffect = EffectManager::Get().GetEffect( *it );
		if (pEffect)
		{
			pEffect->ForceNextTimeDelta( fTimeDelta );
			++it;
		}
		else
		{
			it = m_ownedEffects.erase( it );
		}
	}
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerHitEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerHitEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_hitEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerHitUninterruptableEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerHitUninterruptableEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_noInterruptEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerKOEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerKOEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_KOEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerBlockEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerBlockEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_blockEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerStaggerEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerStaggerEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_staggerEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerDeathEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerDeathEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_deathEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::TriggerInvulnerableEffect
//!
//--------------------------------------------------
void CombatEffectsTrigger::TriggerInvulnerableEffect( const CEntity* pAttacker, const CEntity* pAttackee )
{
	FireTriggerList( m_invulnerableEffects, pAttacker, pAttackee );
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::FireTriggerList
//!
//--------------------------------------------------
void CombatEffectsTrigger::FireTriggerList( ImpactList& list, const CEntity* pAttacker, const CEntity* pVictim )
{
	ntAssert(pAttacker);
	ntAssert(pVictim);

	for (	ImpactList::iterator it = list.begin();
			it != list.end(); ++it )
	{
		(*it)->Fire(pAttacker,pVictim);
	}
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::GetTableNumber
//!
//--------------------------------------------------
bool CombatEffectsTrigger::GetTableNumber( NinjaLua::LuaObject& table, const char* pName, float& fResult )
{
	if (( !table[ pName ].IsNil() ) && ( table[ pName ].IsNumber() ))
	{
		fResult = (float)table[ pName ].GetNumber();
		return true;
	}
	return false;
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::GetTableNumber
//!
//--------------------------------------------------
bool CombatEffectsTrigger::GetTableString( NinjaLua::LuaObject& table, const char* pName, const char** ppName )
{
	if (( !table[ pName ].IsNil() ) && ( table[ pName ].IsString() ))
	{
		*ppName = table[ pName ].GetString();
		return true;
	}
	return false;
}







////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction Bindings
////////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddHitEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddHitEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_hitEffects, "CFX_AddHitEffect" );
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddUninterruptEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddUninterruptEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_noInterruptEffects, "CFX_AddUninterruptEffect" );
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddKOEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddKOEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_KOEffects, "CFX_AddKOEffect" );
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddBlockEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddBlockEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_blockEffects, "CFX_AddBlockEffect" );
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddStaggerEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddStaggerEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_staggerEffects, "CFX_AddStaggerEffect" );
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddDeathEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddDeathEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_deathEffects, "CFX_AddDeathEffect" );
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_AddInvulnerableEffect()
// DESCRIPTION:	process hit effect funtionality
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_AddInvulnerableEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	AddImpactEffectToList( pState, g_pCurrConfigObject->m_invulnerableEffects, "CFX_AddInvulnerableEffect" );
	return 0;
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::AddImpactEffectToList
//! Internal guts of a lua function call that
//! decodes the function args and adds them to a list
//!
//--------------------------------------------------
void CombatEffectsTrigger::AddImpactEffectToList( NinjaLua::LuaState* pState, ImpactList& list, const char* pDebugContext )
{
	NinjaLua::LuaStack  args(pState);
	NinjaLua::LuaObject table( args[1] );

	if (table.IsTable())
	{
		// find particle definition
		//------------------------------------------------------
		const char* pTemp = NULL;
		if ( !GetTableString(table,"ParticleDefinition",&pTemp) )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: '%s' requires a 'ParticleDefinition' table entry in its argument: (%s)\n", pDebugContext, pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#else
			UNUSED(pDebugContext);
			#endif


			return;
		}

		void* pParticleDef = ObjectDatabase::Get().GetPointerFromName<void*>(pTemp);
		if ( !pParticleDef )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: Invalid particle definition '%s' passed to '%s'- interface does not exist. (%s)\n", pTemp, pDebugContext, pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return;
		}

		// find emission definition overide, if required
		//------------------------------------------------------
		const EmitterDef* pEmitterDef = 0;

		if ( GetTableString(table,"EmitterDefinition",&pTemp) )
		{
			pEmitterDef = ObjectDatabase::Get().GetPointerFromName<EmitterDef*>(pTemp);

			if ( !pEmitterDef )
			{
				#ifndef _RELEASE
				const char* pInfo = pState->FileAndLine();
				sprintf( errorMSG, "ERROR: Invalid 'EmitterDefinition' '%s' passed to '%s'- interface does not exist. (%s)\n", pTemp, pDebugContext, pInfo );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif

				return;
			}
		}

		// find offset parameters, if required
		//------------------------------------------------------
		CPoint offset( CONSTRUCT_CLEAR );
		float fTemp;

		if ( GetTableNumber(table,"OffsetX",fTemp) )
			offset.X() = fTemp;

		if ( GetTableNumber(table,"OffsetY",fTemp) )
			offset.Y() = fTemp;

		if ( GetTableNumber(table,"OffsetZ",fTemp) )
			offset.Z() = fTemp;

		// success, create our node and add it to the list
		CE_ImpactParticleTriggerNode* pNewNode = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CE_ImpactParticleTriggerNode( pParticleDef, pEmitterDef, offset );
		list.push_back( pNewNode );
	}
	else
	{
		#ifndef _RELEASE
		const char* pInfo = pState->FileAndLine();
		sprintf( errorMSG, "ERROR: '%s' requires a table as an argument: (%s)\n", pDebugContext, pInfo );
		EffectErrorMSG::AddDebugError( errorMSG );
		#endif
	}

	return;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_SetupTrailEffect()
// DESCRIPTION:	config a trail effect
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_SetupTrailEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	ntError( g_pCurrConfigObject->m_pParent );

	NinjaLua::LuaStack args(pState);
	NinjaLua::LuaObject table( args[1] );

	if (table.IsTable())
	{
		// find transform to attach too
		//------------------------------------------------------
		const char* pTemp = NULL;
		if ( !GetTableString(table,"ParentTransform",&pTemp) && !m_pobUseTransform )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupWakeEffect requires a 'ParentTransform' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		const Transform* pTransform = m_pobUseTransform;
		if (!pTransform)
			pTransform = EffectUtils::FindNamedTransform( g_pCurrConfigObject->m_pParent, pTemp );

		// should already have spat out an ntError on this.
		if ( !pTransform )
			return 0;

		// find times if specified
		//------------------------------------------------------
		float fStartTimeN = 0.0f;
		float fEndTimeN = 1.0f;

		if ( GetTableNumber(table,"StartTime",fStartTimeN) )
			fStartTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"StartTimeN",fStartTimeN);

		if ( GetTableNumber(table,"EndTime",fEndTimeN) )
			fEndTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"EndTimeN",fEndTimeN);

		// find trail definition
		//------------------------------------------------------
		if ( !GetTableString(table,"TrailDefinition",&pTemp) )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupTrailEffect requires a 'TrailDefinition' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		void* pPtr = ObjectDatabase::Get().GetPointerFromName<void*>(pTemp);

		if ( !pPtr )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupTrailEffect has a 'TrailDefinition' that does not exist:%s. (%s)\n", pTemp, pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		const char* pType = EffectUtils::GetInterfaceType( pPtr );

		if (stricmp( pType, "EffectTrail_SimpleDef" ) == 0)
		{
			const EffectTrail_SimpleDef* pTrailDef = static_cast<EffectTrail_SimpleDef*>(pPtr);
			if ( !pTrailDef )
			{
				#ifndef _RELEASE
				const char* pInfo = pState->FileAndLine();
				sprintf( errorMSG, "ERROR: Invalid trail definition '%s' passed to CFX_SetupTrailEffect- interface does not exist. (%s)\n", pTemp, pInfo );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif

				return 0;
			}

			// find edge definition overide, if required
			//------------------------------------------------------
			const EffectTrail_EdgeDef* pEdgeDef = pTrailDef->m_pEdgeDefinition;

			if ( GetTableString(table,"EdgeDefinition",&pTemp) )
			{
				pEdgeDef = ObjectDatabase::Get().GetPointerFromName<EffectTrail_EdgeDef*>(pTemp);

				if ( !pEdgeDef )
				{
					#ifndef _RELEASE
					const char* pInfo = pState->FileAndLine();
					sprintf( errorMSG, "ERROR: Invalid 'EdgeDefinition' '%s' passed to CFX_SetupTrailEffect- interface does not exist. (%s)\n", pTemp, pInfo );
					EffectErrorMSG::AddDebugError( errorMSG );
					#endif

					return 0;
				}

				// we have a valid overide
			}
			else if ( !pTrailDef->m_pEdgeDefinition )
			{
				// we have no edge at all, return
				#ifndef _RELEASE
				const char* pInfo = pState->FileAndLine();
				sprintf( errorMSG, "ERROR: Missing edge definition in CFX_SetupTrailEffect- provide an overide 'EdgeDefinition' or a trail def with a default. (%s)\n", pInfo );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif

				return 0;
			}

			// success, create our node and add it too the list
			CE_TrailEventTriggerNode* pNewNode = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CE_TrailEventTriggerNode( pPtr, (void*)pEdgeDef, pTransform, fStartTimeN, fEndTimeN );
			g_pCurrConfigObject->m_events.push_back( pNewNode );
		}
		else if (stricmp( pType, "EffectTrail_LineDef" ) == 0)
		{
			// a line trail, add to the list
			CPoint offset( CONSTRUCT_CLEAR );
			float fTemp;

			if ( GetTableNumber(table,"OffsetX",fTemp) )
				offset.X() = fTemp;

			if ( GetTableNumber(table,"OffsetY",fTemp) )
				offset.Y() = fTemp;

			if ( GetTableNumber(table,"OffsetZ",fTemp) )
				offset.Z() = fTemp;

			CE_LineTrailEventTriggerNode* pNewNode = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CE_LineTrailEventTriggerNode( pPtr, offset, pTransform, fStartTimeN, fEndTimeN );
			g_pCurrConfigObject->m_events.push_back( pNewNode );
		}
	}
	else
	{
		#ifndef _RELEASE
		const char* pInfo = pState->FileAndLine();
		sprintf( errorMSG, "ERROR: CFX_SetupTrailEffect requires a table as an argument: (%s)\n", pInfo );
		EffectErrorMSG::AddDebugError( errorMSG );
		#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_SetupParticleEffect()
// DESCRIPTION:	config a particle effect
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_SetupParticleEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	ntError( g_pCurrConfigObject->m_pParent );

	NinjaLua::LuaStack args(pState);
	NinjaLua::LuaObject table( args[1] );

	if (table.IsTable())
	{
		// find transform to attach too
		//------------------------------------------------------
		const char* pTemp = NULL;
		if ( !GetTableString(table,"ParentTransform",&pTemp) && !m_pobUseTransform )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupWakeEffect requires a 'ParentTransform' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		const Transform* pTransform = m_pobUseTransform;
		if (!pTransform)
			pTransform = EffectUtils::FindNamedTransform( g_pCurrConfigObject->m_pParent, pTemp );

		// should already have spat out an ntError on this.
		if ( !pTransform )
			return 0;

		// find trail definition
		//------------------------------------------------------
		if ( !GetTableString(table,"ParticleDefinition",&pTemp) )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupParticleEffect requires a 'ParticleDefinition' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		void* pParticleDef = ObjectDatabase::Get().GetPointerFromName<void*>(pTemp);
		if ( !pParticleDef )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: Invalid particle definition '%s' passed to CFX_SetupParticleEffect- interface does not exist. (%s)\n", pTemp, pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		// find emission definition overide, if required
		//------------------------------------------------------
		const EmitterDef* pEmitterDef = 0;

		if ( GetTableString(table,"EmitterDefinition",&pTemp) )
		{
			pEmitterDef = ObjectDatabase::Get().GetPointerFromName<EmitterDef*>(pTemp);

			if ( !pEmitterDef )
			{
				#ifndef _RELEASE
				const char* pInfo = pState->FileAndLine();
				sprintf( errorMSG, "ERROR: Invalid 'EmitterDefinition' '%s' passed to CFX_SetupParticleEffect- interface does not exist. (%s)\n", pTemp, pInfo );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif

				return 0;
			}
		}

		// find times if specified
		//------------------------------------------------------
		float fStartTimeN = 0.0f;
		float fEndTimeN = 1.0f;

		if ( GetTableNumber(table,"StartTime",fStartTimeN) )
			fStartTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"StartTimeN",fStartTimeN);

		if ( GetTableNumber(table,"EndTime",fEndTimeN) )
			fEndTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"EndTimeN",fEndTimeN);

		// success, create our node and add it too the list
		CE_ParticleEventTriggerNode* pNewNode = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CE_ParticleEventTriggerNode( pParticleDef, pEmitterDef, pTransform, fStartTimeN, fEndTimeN );
		g_pCurrConfigObject->m_events.push_back( pNewNode );
	}
	else
	{
		#ifndef _RELEASE
		const char* pInfo = pState->FileAndLine();
		sprintf( errorMSG, "ERROR: CFX_SetupParticleEffect requires a table as an argument: (%s)\n", pInfo );
		EffectErrorMSG::AddDebugError( errorMSG );
		#endif
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	CFX_SetupWakeEffect()
// DESCRIPTION:	config a wake particle effect
//-------------------------------------------------------------------------------------------------
int CombatEffectsTrigger::CFX_SetupWakeEffect( NinjaLua::LuaState* pState )
{
	ntError( g_pCurrConfigObject );
	ntError( g_pCurrConfigObject->m_pParent );

	NinjaLua::LuaStack args(pState);
	NinjaLua::LuaObject table( args[1] );

	if (table.IsTable())
	{
		// find transform to attach too
		//------------------------------------------------------
		const char* pTemp = NULL;
		if ( !GetTableString(table,"ParentTransform",&pTemp) && !m_pobUseTransform )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupWakeEffect requires a 'ParentTransform' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		const Transform* pTransform = m_pobUseTransform;
		if (!pTransform)
			pTransform = EffectUtils::FindNamedTransform( g_pCurrConfigObject->m_pParent, pTemp );

		// should already have spat out an ntError on this.
		if ( !pTransform )
			return 0;

		// find trail definition
		//------------------------------------------------------
		if ( !GetTableString(table,"ParticleDefinition",&pTemp) )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: CFX_SetupWakeEffect requires a 'ParticleDefinition' table entry in its argument: (%s)\n", pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		void* pParticleDef = ObjectDatabase::Get().GetPointerFromName<void*>(pTemp);
		if ( !pParticleDef )
		{
			#ifndef _RELEASE
			const char* pInfo = pState->FileAndLine();
			sprintf( errorMSG, "ERROR: Invalid particle definition '%s' passed to CFX_SetupWakeEffect- interface does not exist. (%s)\n", pTemp, pInfo );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif

			return 0;
		}

		// find emission definition overide, if required
		//------------------------------------------------------
		const EmitterDef* pEmitterDef = 0;

		if ( GetTableString(table,"EmitterDefinition",&pTemp) )
		{
			pEmitterDef = ObjectDatabase::Get().GetPointerFromName<EmitterDef*>(pTemp);

			if ( !pEmitterDef )
			{
				#ifndef _RELEASE
				const char* pInfo = pState->FileAndLine();
				sprintf( errorMSG, "ERROR: Invalid 'EmitterDefinition' '%s' passed to CFX_SetupParticleEffect- interface does not exist. (%s)\n", pTemp, pInfo );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif

				return 0;
			}
		}

		// find times if specified
		//------------------------------------------------------
		float fStartTimeN = 0.0f;
		float fEndTimeN = 1.0f;

		if ( GetTableNumber(table,"StartTime",fStartTimeN) )
			fStartTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"StartTimeN",fStartTimeN);

		if ( GetTableNumber(table,"EndTime",fEndTimeN) )
			fEndTimeN /= g_pCurrConfigObject->m_fAttackDuration;
		else
			GetTableNumber(table,"EndTimeN",fEndTimeN);

		float fFullEmitDistance = 0.0f;
		float fNoEmitDistance = 1.0f;

		GetTableNumber(table,"FullEmitDistance",fFullEmitDistance);
		GetTableNumber(table,"NoEmitDistance",fNoEmitDistance);

		// find offset parameters, if required
		//------------------------------------------------------
		CPoint offset( CONSTRUCT_CLEAR );
		float fTemp;

		if ( GetTableNumber(table,"OffsetX",fTemp) )
			offset.X() = fTemp;

		if ( GetTableNumber(table,"OffsetY",fTemp) )
			offset.Y() = fTemp;

		if ( GetTableNumber(table,"OffsetZ",fTemp) )
			offset.Z() = fTemp;

		// success, create our node and add it too the list
		CE_WakeEventTriggerNode* pNewNode = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CE_WakeEventTriggerNode( pParticleDef, pEmitterDef, pTransform,
														fStartTimeN, fEndTimeN, fFullEmitDistance, fNoEmitDistance, offset );
		g_pCurrConfigObject->m_events.push_back( pNewNode );
	}
	else
	{
		#ifndef _RELEASE
		const char* pInfo = pState->FileAndLine();
		sprintf( errorMSG, "ERROR: CFX_SetupWakeEffect requires a table as an argument: (%s)\n", pInfo );
		EffectErrorMSG::AddDebugError( errorMSG );
		#endif
	}

	return 0;
}

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::Register
//! Register our functions with lua
//!
//--------------------------------------------------
void CombatEffectsTrigger::Register()
{
//#define _DISABLE_COMBAT_FX

#ifdef _DISABLE_COMBAT_FX
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_SetupTrailEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_SetupParticleEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddHitEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddUninterruptEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddKOEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddBlockEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddStaggerEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddDeathEffect", CFX_DummyFunc);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddInvulnerableEffect", CFX_DummyFunc);
#else
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_SetupTrailEffect", CFX_SetupTrailEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_SetupParticleEffect", CFX_SetupParticleEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_SetupWakeEffect", CFX_SetupWakeEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddHitEffect", CFX_AddHitEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddUninterruptEffect", CFX_AddUninterruptEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddKOEffect", CFX_AddKOEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddBlockEffect", CFX_AddBlockEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddStaggerEffect", CFX_AddStaggerEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddDeathEffect", CFX_AddDeathEffect);
	CLuaGlobal::Get().State().GetGlobals().RegisterRaw("CFX_AddInvulnerableEffect", CFX_AddInvulnerableEffect);
#endif
}

