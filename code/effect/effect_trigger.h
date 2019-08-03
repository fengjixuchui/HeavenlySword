//--------------------------------------------------
//!
//!	\file effect_trigger.h
//!	XML struct that can create effects of an unknown
//! type from their definition.
//!
//--------------------------------------------------

#ifndef _E_TRIGGER_H
#define _E_TRIGGER_H

#include "effect/effect.h"

class CEntity;

//--------------------------------------------------
//!
//!	EffectTrigger
//!
//--------------------------------------------------
class EffectTrigger
{
public:
	EffectTrigger();
	virtual ~EffectTrigger() {};
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );
	u_int ForceTrigger(CEntity* pOverideEnt=0);
	u_int ForceTrigger(const CMatrix& frame);

	void*		m_pDefinition;
	void*		m_pAdditional;
	CHashedString	m_obParentEntity;
	CHashedString	m_obParentTransform;
	bool		m_bAutoCreate;

private:
	u_int		m_iLastChangeTriggerGUID;
};

//--------------------------------------------------
//!
//!	EffectTriggerInternal
//! Time delay device to auto create effects on 1st game
//! frame
//!
//--------------------------------------------------
class EffectTriggerInternal : public Effect
{
public:
	EffectTriggerInternal( EffectTrigger* pDefinition ) :
		m_emissionFrame(CONSTRUCT_IDENTITY),
		m_pDefinition(pDefinition),
		m_iTriggerdGUID(0),
		m_bTriggered(false) {};

	void TriggerEffect(CEntity* pOverideEnt);
	
	virtual bool UpdateEffect();
	virtual bool WaitingForResources()const { return false; }

	u_int GetTriggeredGUID() { return m_iTriggerdGUID; }
	CMatrix			m_emissionFrame;

private:
	EffectTrigger*	m_pDefinition;
	u_int			m_iTriggerdGUID;
	bool			m_bTriggered;
};

#endif // _E_TRIGGER_H
