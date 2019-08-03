//--------------------------------------------------
//!
//!	\file combateffect_triggernode.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _CE_TRIGGER_NODE_H
#define _CE_TRIGGER_NODE_H

class EffectTrail_SimpleDef;
class EffectTrail_EdgeDef;
class Transform;
class EmitterDef;
class CEntity;

//--------------------------------------------------
//!
//!	CE_EventTriggerNode
//!	virtual base class for all event based trigger nodes
//!
//--------------------------------------------------
class CE_EventTriggerNode
{
public:
	typedef ntstd::List<u_int, Mem::MC_EFFECTS>	EffectList;

	virtual ~CE_EventTriggerNode(){};
	// all events that are triggerd must use this mechanism to fire.
	// fTimeN is a normalised time to fire the event at.
	// pNewNode is a possible new node to put into the list if
	// we've been fired. (say an end event)
	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode ) = 0;
};

//--------------------------------------------------
//!
//!	CE_TrailEventTriggerNode
//!	trigger a trail effect
//!
//--------------------------------------------------
class CE_TrailEventTriggerNode : public CE_EventTriggerNode
{
public:
	CE_TrailEventTriggerNode(	void* pDef,
								void* pAdditional,
								const Transform* pTransform,
								float fStartTime,
								float fEndTime );

	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode );

private:
	void* m_pDef;
	void* m_pAdditional;
	const Transform* m_pTransform;
	float m_fStartTime;
	float m_fEndTime;
};

//--------------------------------------------------
//!
//!	CE_LineTrailEventTriggerNode
//!	trigger a trail effect
//!
//--------------------------------------------------
class CE_LineTrailEventTriggerNode : public CE_EventTriggerNode
{
public:
	CE_LineTrailEventTriggerNode(	void* pDef,
									const CPoint& offset,
									const Transform* pTransform,
									float fStartTime,
									float fEndTime );

	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode );

private:
	void* m_pDef;
	CPoint m_offset;
	const Transform* m_pTransform;
	float m_fStartTime;
	float m_fEndTime;
};
//--------------------------------------------------
//!
//!	CE_ParticleEventTriggerNode
//!	trigger a trail effect
//!
//--------------------------------------------------
class CE_ParticleEventTriggerNode : public CE_EventTriggerNode
{
public:
	CE_ParticleEventTriggerNode(	void* pDef,
									const EmitterDef* pEmitterDef,
									const Transform* pTransform,
									float fStartTime,
									float fEndTime );

	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode );

private:
	void* m_pDef;
	const EmitterDef* m_pEmitterDef;
	const Transform* m_pTransform;
	float m_fStartTime;
	float m_fEndTime;
};
//--------------------------------------------------
//!
//!	CE_WakeEventTriggerNode
//!	trigger a trail effect
//!
//--------------------------------------------------
class CE_WakeEventTriggerNode : public CE_EventTriggerNode
{
public:
	CE_WakeEventTriggerNode(	void* pDef,
								const EmitterDef* pEmitterDef,
								const Transform* pTransform,
								float fStartTime,
								float fEndTime,
								float fFullEmitDistance, 
								float fNoEmitDistance,
								const CPoint& offset );

	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode );

private:
	void* m_pDef;
	const EmitterDef* m_pEmitterDef;
	const Transform* m_pTransform;
	float m_fStartTime;
	float m_fEndTime;
	float m_fFullEmitDistance;
	float m_fNoEmitDistance;
	CPoint m_offset;
};

//--------------------------------------------------
//!
//!	CE_EffectStopEventNode
//!	stop a trail effect
//!
//--------------------------------------------------
class CE_EffectStopEventNode : public CE_EventTriggerNode
{
public:
	CE_EffectStopEventNode( float fStopTime, u_int iEffectID ) : m_fStopTime(fStopTime), m_iEffectID(iEffectID) {}
	virtual bool FireIfReady( float fTimeN, EffectList& newItems, CE_EventTriggerNode** pNewNode );

private:
	float m_fStopTime;
	u_int m_iEffectID;
};




//--------------------------------------------------
//!
//!	CE_ImpactTriggerNode
//!	virtual base class for all one shot trigger nodes
//!
//--------------------------------------------------
class CE_ImpactTriggerNode
{
public:
	virtual ~CE_ImpactTriggerNode(){};

	// all one shot effects that are triggerd must use this mechanism to fire.
	virtual void Fire( const CEntity* pAttacker, const CEntity* pVictim ) = 0;
};

//--------------------------------------------------
//!
//!	CE_ImpactParticleTriggerNode
//!	trigger a particle effect
//!
//--------------------------------------------------
class CE_ImpactParticleTriggerNode : public CE_ImpactTriggerNode
{
public:
	CE_ImpactParticleTriggerNode(	void* pDef,
									const EmitterDef* pEmitterDef,
									const CPoint& offset );
	virtual void Fire( const CEntity* pAttacker, const CEntity* pVictim );

private:
	void* m_pDef;
	const EmitterDef* m_pEmitterDef;
	CPoint m_offset;
};



#endif // _CE_TRIGGER_NODE_H
