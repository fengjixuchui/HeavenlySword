//--------------------------------------------------
//!
//!	\file effect_error.h
//!	Simple ntError display class
//!
//--------------------------------------------------

#ifndef _EFFECT_ERROR_H
#define _EFFECT_ERROR_H

#include "effect.h"

#ifndef _GOLD_MASTER

//--------------------------------------------------
//!
//!	EffectErrorMSG
//! Effect used to show ntError messages
//!
//--------------------------------------------------
class EffectErrorMSG : public Effect
{
public: 
	EffectErrorMSG( const char* pMSG, float fDuration )
	{
		ntError_p( pMSG, ("your ntError is erronious\n") );
		
		m_pMSG = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) char [strlen(pMSG)+1];
		strcpy( m_pMSG, pMSG );
		m_fAge = m_fRemaining = fDuration;
	}

	~EffectErrorMSG() { NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pMSG ); }

	virtual bool UpdateEffect()
	{
		float fTimeChange = GetNextTimeDelta();
		m_fRemaining -= fTimeChange;
		return (m_fRemaining <= 0.0f) ? true : false;
	}

	virtual bool WaitingForResources() const { return false; }

	virtual void RenderEffect();

	static void AddDebugError( const char* pMSG, float fDuration = 10.0f );

private:
	char*	m_pMSG;
	float	m_fRemaining, m_fAge;
};

#endif

#endif
