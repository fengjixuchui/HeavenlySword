//--------------------------------------------------
//!
//!	\file effect_trigger.cpp
//!	XML struct that can create effects of an unknown
//! type from their definition.
//!
//--------------------------------------------------

#include "effect_trigger.h"
#include "effect/effect_manager.h"
#include "effect_error.h"
#include "effect_util.h"
#include "effect/psystem_utils.h"
#include "effecttrail_utils.h"
#include "objectdatabase/dataobject.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/shellmain.h"

START_STD_INTERFACE( EffectTrigger )
	I2REFERENCE	( m_pDefinition, Definition )	
	I2REFERENCE	( m_pAdditional, Additional )	
	ISTRING		( EffectTrigger, ParentEntity )
	ISTRING		( EffectTrigger, ParentTransform )
	IBOOL		( EffectTrigger, AutoCreate )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	EffectTrigger ctor
//!
//--------------------------------------------------
EffectTrigger::EffectTrigger() :
	m_pDefinition(0),
	m_pAdditional(0),
	m_bAutoCreate( true ),
	m_iLastChangeTriggerGUID( 0 )
{
}

//--------------------------------------------------
//!
//!	PostConstruct
//!
//--------------------------------------------------
void EffectTrigger::PostConstruct()
{
	if (m_bAutoCreate)
		EffectManager::Get().AddEffect( NT_NEW_CHUNK( Mem::MC_EFFECTS ) EffectTriggerInternal( this ) );
}

//--------------------------------------------------
//!
//!	EditorChangeValue
//!
//--------------------------------------------------
bool EffectTrigger::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	// kill the last effect that was triggered like this
	if (m_iLastChangeTriggerGUID != 0)
	{
		EffectManager::Get().KillEffectNow(m_iLastChangeTriggerGUID);
	}

	// trigger effect and remember its ID
	m_iLastChangeTriggerGUID = ForceTrigger();
	return true;
}

//--------------------------------------------------
//!
//!	EffectTrigger::ForceTrigger
//! Manual (re: programmer) firing of this trigger
//!
//--------------------------------------------------
u_int EffectTrigger::ForceTrigger(CEntity* pOverideEnt)
{
	EffectTriggerInternal trigger(this);
	trigger.TriggerEffect(pOverideEnt);
	return trigger.GetTriggeredGUID();
}

//--------------------------------------------------
//!
//!	EffectTrigger::ForceTrigger
//! Manual (re: programmer) firing of this trigger
//!
//--------------------------------------------------
u_int EffectTrigger::ForceTrigger( const CMatrix& frame )
{
	EffectTriggerInternal trigger(this);
	trigger.m_emissionFrame = frame;
	trigger.TriggerEffect(0);
	return trigger.GetTriggeredGUID();
}


//--------------------------------------------------
//!
//!	TriggerEffect
//! this should only happen once!
//!
//--------------------------------------------------
void EffectTriggerInternal::TriggerEffect(CEntity* pOverideEnt)
{
	ntAssert(!m_bTriggered);
	ntAssert(m_pDefinition);

#ifndef _RELEASE
	static char aErrors[MAX_PATH];
#endif

	m_iTriggerdGUID = 0;

	if (m_pDefinition->m_pDefinition)
	{
		const char* pType =  EffectUtils::GetInterfaceType( m_pDefinition->m_pDefinition );
		if (strstr( pType, "PSystem" ))
		{
			// check for an emitter overide in the anonymous additional field
			const EmitterDef* pOverideEmit = 0;
			if (m_pDefinition->m_pAdditional)
			{
				const char* pAdditionalType =  EffectUtils::GetInterfaceType( m_pDefinition->m_pAdditional );
				if (strstr( pAdditionalType, "Emitter" ))
					pOverideEmit = (const EmitterDef*)m_pDefinition->m_pAdditional;
			}

			if (pOverideEnt)
			{
				Transform* pTrans = EffectUtils::FindNamedTransform( pOverideEnt, m_pDefinition->m_obParentTransform );

				if (!pTrans)
				{
					#ifndef _RELEASE
					sprintf( aErrors, "Entity %s does not have transform %s used in trigger %s.\n", ntStr::GetString(pOverideEnt->GetName()), ntStr::GetString(m_pDefinition->m_obParentTransform),  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pDefinition )) );
					EffectErrorMSG::AddDebugError( aErrors );
					#endif
				}
				else
					m_iTriggerdGUID = PSystemUtils::ConstructParticleEffect( m_pDefinition->m_pDefinition, pTrans, pOverideEmit );
			}
			else if (m_pDefinition->m_obParentEntity.IsNull())
			{
				m_iTriggerdGUID = PSystemUtils::ConstructParticleEffect( m_pDefinition->m_pDefinition, m_emissionFrame, pOverideEmit );
			}
			else
			{
				CHashedString pName = m_pDefinition->m_obParentEntity;
				CHashedString pTrans = m_pDefinition->m_obParentTransform;
				m_iTriggerdGUID = PSystemUtils::ConstructParticleEffect( m_pDefinition->m_pDefinition, pName, pTrans, pOverideEmit );
			}
		}
		else if (strstr( pType, "EffectTrail" ) )
		{
			if	(
				(pOverideEnt) ||
				(!m_pDefinition->m_obParentEntity.IsNull())
				)
			{
				// check for an edge overide in the anonymous additional field
				void* pOverideEdge = 0;
				if (m_pDefinition->m_pAdditional)
				{
					const char* pAdditionalType =  EffectUtils::GetInterfaceType( m_pDefinition->m_pAdditional );
					if (strcmp( pAdditionalType, "EffectTrail_EdgeDef" ) == 0)
						pOverideEdge = m_pDefinition->m_pAdditional;
				}

				// construct the trail
				if (pOverideEnt)
				{
					Transform* pTrans = EffectUtils::FindNamedTransform( pOverideEnt, m_pDefinition->m_obParentTransform );

					if (!pTrans)
					{
						#ifndef _RELEASE
						sprintf( aErrors, "Entity %s does not have transform %s used in trigger %s.\n", ntStr::GetString(pOverideEnt->GetName()), ntStr::GetString(m_pDefinition->m_obParentTransform),  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pDefinition ) ) );
						EffectErrorMSG::AddDebugError( aErrors );
						#endif
					}
					else
						m_iTriggerdGUID = TrailUtils::ConstructTrailEffect( m_pDefinition->m_pDefinition, pTrans, pOverideEdge );
				}
				else
				{
					CHashedString pName = m_pDefinition->m_obParentEntity;
					CHashedString pTrans = m_pDefinition->m_obParentTransform;
					m_iTriggerdGUID = TrailUtils::ConstructTrailEffect( m_pDefinition->m_pDefinition, pName, pTrans, pOverideEdge );
				}
			}
			else
			{
				#ifndef _RELEASE
				sprintf( aErrors, "Effect Trails require a parent ent to construct" );
				EffectErrorMSG::AddDebugError( aErrors );
				#endif
			}
		}
		else
		{
			#ifndef _RELEASE
			sprintf( aErrors, "Unrecognised effect type %s being triggered", pType );
			EffectErrorMSG::AddDebugError( aErrors );
			#endif
		}
	}
	else
	{
		#ifndef _RELEASE
		sprintf( aErrors, "Invalid effect trigger %s being triggered",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pDefinition ) ) );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
	}

	m_bTriggered = true;
}

//--------------------------------------------------
//!
//!	Update
//! this should only happen once!
//!
//--------------------------------------------------
bool EffectTriggerInternal::UpdateEffect()
{
	if (ShellMain::Get().HaveLoadedLevel())
	{
		TriggerEffect(0);
		return true;
	}
	else
	{
		return false;
	}
}
