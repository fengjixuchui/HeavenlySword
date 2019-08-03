//--------------------------------------------------
//!
//!	\file combateffect_triggernode.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "combateffect_triggernode.h"
#include "effecttrail_simple.h"
#include "effect_manager.h"
#include "effect/psystem_utils.h"
#include "particle_wake.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "camera/camutils.h"

//--------------------------------------------------
//!
//!	CombatEffectsTrigger::CE_TrailEventTriggerNode
//!
//--------------------------------------------------
CE_TrailEventTriggerNode::CE_TrailEventTriggerNode(	void* pDef,
													void* pAdditional,
													const Transform* pTransform,
													float fStartTime,
													float fEndTime ) :
	m_pDef(pDef),
	m_pAdditional(pAdditional),
	m_pTransform(pTransform),
	m_fStartTime(fStartTime),
	m_fEndTime(fEndTime)
{
	ntAssert(m_pDef);
	ntAssert(m_pAdditional);
	ntAssert(m_pTransform);
}

//--------------------------------------------------
//!
//!	CE_TrailEventTriggerNode::FireIfReady
//!
//--------------------------------------------------
bool CE_TrailEventTriggerNode::FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode )
{
	if (fTimeN >= m_fStartTime)
	{
		u_int iEffectID = TrailUtils::ConstructTrailEffect( m_pDef, m_pTransform, m_pAdditional );

		newItems.push_back( iEffectID );
		*pNewNode = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CE_EffectStopEventNode( m_fEndTime, iEffectID );
		return true;
	}

	return false;
}





//--------------------------------------------------
//!
//!	CombatEffectsTrigger::CE_LineTrailEventTriggerNode
//!
//--------------------------------------------------
CE_LineTrailEventTriggerNode::CE_LineTrailEventTriggerNode(	void* pDef,
													const CPoint& offset,
													const Transform* pTransform,
													float fStartTime,
													float fEndTime ) :
	m_pDef(pDef),
	m_offset(offset),
	m_pTransform(pTransform),
	m_fStartTime(fStartTime),
	m_fEndTime(fEndTime)
{
	ntAssert(m_pDef);
	ntAssert(m_pTransform);
}

//--------------------------------------------------
//!
//!	CE_LineTrailEventTriggerNode::FireIfReady
//!
//--------------------------------------------------
bool CE_LineTrailEventTriggerNode::FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode )
{
	if (fTimeN >= m_fStartTime)
	{
		u_int iEffectID = TrailUtils::ConstructTrailEffect( m_pDef, m_pTransform, &m_offset );

		newItems.push_back( iEffectID );
		*pNewNode = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CE_EffectStopEventNode( m_fEndTime, iEffectID );
		return true;
	}

	return false;
}




//--------------------------------------------------
//!
//!	CE_ParticleEventTriggerNode::CE_ParticleEventTriggerNode
//!
//--------------------------------------------------
CE_ParticleEventTriggerNode::CE_ParticleEventTriggerNode(	void* pDef,
												const EmitterDef* pEmitterDef,
												const Transform* pTransform,
												float fStartTime,
												float fEndTime ) :
	m_pDef(pDef),
	m_pEmitterDef(pEmitterDef),
	m_pTransform(pTransform),
	m_fStartTime(fStartTime),
	m_fEndTime(fEndTime)
{
	ntAssert(m_pDef);
	ntAssert(m_pTransform);
}

//--------------------------------------------------
//!
//!	CE_ParticleEventTriggerNode::FireIfReady
//!
//--------------------------------------------------
bool CE_ParticleEventTriggerNode::FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode )
{
	if (fTimeN >= m_fStartTime)
	{
		u_int iEffectID = PSystemUtils::ConstructParticleEffect( m_pDef, m_pTransform, m_pEmitterDef );
		newItems.push_back( iEffectID );
		*pNewNode = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CE_EffectStopEventNode( m_fEndTime, iEffectID );
		return true;
	}

	return false;
}





//--------------------------------------------------
//!
//!	CE_WakeEventTriggerNode::CE_WakeEventTriggerNode
//!
//--------------------------------------------------
CE_WakeEventTriggerNode::CE_WakeEventTriggerNode(	void* pDef,
												const EmitterDef* pEmitterDef,
												const Transform* pTransform,
												float fStartTime,
												float fEndTime,
												float fFullEmitDistance,
												float fNoEmitDistance,
												const CPoint& offset ) :
	m_pDef(pDef),
	m_pEmitterDef(pEmitterDef),
	m_pTransform(pTransform),
	m_fStartTime(fStartTime),
	m_fEndTime(fEndTime),
	m_fFullEmitDistance(fFullEmitDistance),
	m_fNoEmitDistance(fNoEmitDistance),
	m_offset(offset)
{
	ntAssert(m_pDef);
	ntAssert(m_pTransform);
}

//--------------------------------------------------
//!
//!	CE_WakeEventTriggerNode::FireIfReady
//!
//--------------------------------------------------
bool CE_WakeEventTriggerNode::FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode )
{
	if (fTimeN >= m_fStartTime)
	{
		ParticleWake* pWake = NT_NEW_CHUNK( Mem::MC_EFFECTS ) ParticleWake( m_pTransform,
												m_pDef,
												m_pEmitterDef,
												m_fFullEmitDistance,
												m_fNoEmitDistance,
												m_offset );
		
		u_int iEffectID = EffectManager::Get().AddEffect( pWake );

		newItems.push_back( iEffectID );
		*pNewNode = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CE_EffectStopEventNode( m_fEndTime, iEffectID );
		return true;
	}

	return false;
}



//--------------------------------------------------
//!
//!	CE_ImpactParticleTriggerNode::CE_ImpactParticleTriggerNode
//!
//--------------------------------------------------
CE_ImpactParticleTriggerNode::CE_ImpactParticleTriggerNode(	void* pDef,
															const EmitterDef* pEmitterDef,
															const CPoint& offset ) :
	m_pDef(pDef),
	m_pEmitterDef(pEmitterDef),
	m_offset(offset)
{
	ntAssert(m_pDef);
}

//--------------------------------------------------
//!
//!	CE_ImpactParticleTriggerNode::Fire
//! specifically designed to fire a particle effect
//! between two entities with a given offset
//!
//--------------------------------------------------
void CE_ImpactParticleTriggerNode::Fire( const CEntity* pAttacker, const CEntity* pVictim )
{
	/*
	CPoint attackerPos( pAttacker->GetPosition() );
	CPoint victimPos( pVictim->GetPosition() );

	attackerPos.Y() = victimPos.Y();

	CMatrix mat;
	CCamUtil::CreateFromPoints(mat, victimPos, attackerPos);

	CMatrix frame( mat );
	frame.SetTranslation( m_offset * mat );

	PSystemUtils::ConstructParticleEffect( m_pDef, frame, m_pEmitterDef );
	*/

	CPoint obTemp(pAttacker->GetPosition());
	obTemp.Y()=pVictim->GetPosition().Y();

	CMatrix mat;

	CCamUtil::CreateFromPoints(mat,pVictim->GetPosition(),obTemp);

	obTemp=m_offset * mat;

	mat.SetTranslation( obTemp );

	PSystemUtils::ConstructParticleEffect( m_pDef, mat, m_pEmitterDef );
}





//--------------------------------------------------
//!
//!	CE_EffectStopEventNode::FireIfReady
//!
//--------------------------------------------------
bool CE_EffectStopEventNode::FireIfReady( float fTimeN, EffectList&, CE_EventTriggerNode** )
{
	if (fTimeN >= m_fStopTime)
	{
		EffectManager::Get().KillEffectWhenReady( m_iEffectID );
		return true;
	}

	return false;
}
