//--------------------------------------------------
//!
//!	\file effect.h
//!	Base class for all effects
//!
//--------------------------------------------------

#ifndef _EFFECT_H
#define _EFFECT_H

#include "core/timer.h"

//--------------------------------------------------
//!
//!	Effect
//! Effect base class that all other effects derive from
//!
//--------------------------------------------------
class Effect
{
public:
	Effect::Effect() : m_bKillMeNow( false ), m_bKillMeRequested( false ), m_fTimeDeltaOveride(-1.0f) {};
	virtual ~Effect() {};

	//! return true if we're dead and should be released by the effect manager.
	virtual bool UpdateEffect() = 0;

	//! let the caller know if we're dependant on resources that have yet to be built
	virtual bool WaitingForResources() const = 0;

	//! set the next update time to be this
	virtual void ForceNextTimeDelta( float fTimeDelta ) { m_fTimeDeltaOveride = fTimeDelta; }

	//! draw me
	virtual void RenderEffect(){};

	//! are we rendered in the HDR or LDR slot?
	virtual bool HighDynamicRange()	const { return false; }	

	//! set to destroy as soon as possible
	void	KillMeNow() { m_bKillMeNow = true; }

	//! set to destroy when effect feels like it
	void	KillMeWhenReady() { m_bKillMeRequested = true; }

protected:

	// a m_fTimeChangeOveride of -1 indicates use a normal time change.
	// calling ForceNextTimeChange changes the update delta for a given
	// effect for one frame only!
	float	GetNextTimeDelta()
	{
		if (m_fTimeDeltaOveride < 0.0f)
			return CTimer::Get().GetGameTimeChange();

		float timeChange = m_fTimeDeltaOveride;
		m_fTimeDeltaOveride = -1.0f;
		return timeChange;
	}

	bool	m_bKillMeNow;
	bool	m_bKillMeRequested;

private:
	float	m_fTimeDeltaOveride;
};

#endif
