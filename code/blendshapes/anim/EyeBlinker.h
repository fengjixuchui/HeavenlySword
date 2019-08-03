//--------------------------------------------------
//!
//!	\file EyeBlinker.h
//!	El-cheapo eye blink controller
//! basically, it triggers an anim (or bsanim via events)
//! every now and then, based on frequency
//!
//--------------------------------------------------

#ifndef _EYEBLINKER_H_
#define _EYEBLINKER_H_

#include "game/anonymouscomponent.h"
#include "game/luaglobal.h"
#include "anim/animation.h"

class CEntity;
class Transform;
class CHierarchy;


struct EyeBlinkerDef
{
	HAS_INTERFACE(EyeBlinker)

	CHashedString			m_obAnimShortName;				//!< the blinking animation short name
	float					m_fBlinkInterval;				//!< aprox. blinking interval (in secs)
	float					m_fRandomness;
	bool					m_bEnabled;						//!< mystery variable. I won't tell you what it does...
	float					m_fBlendWeight;
};


class EyeBlinker : public CAnonymousEntComponent
{
public:
	HAS_LUA_INTERFACE()

	EyeBlinker( CEntity* pobEnt, EyeBlinkerDef* pobDef );
	~EyeBlinker();

	void Update( float fTimeStep );
	void Blink( void );
	
	bool IsEnabled( void ) const	{ return m_pobDef->m_bEnabled; }
	void Enable( void )				{ m_pobDef->m_bEnabled = true; }
	void Disable( void )			{ m_pobDef->m_bEnabled = false; m_fTimeSinceLastBlink = 0.0f; }

private:
	CEntity*						m_pobEnt;
	EyeBlinkerDef*					m_pobDef;
	float							m_fTimeSinceLastBlink;
	CAnimationPtr					m_pobBlinkAnimation;
};

LV_DECLARE_USERDATA(EyeBlinker);


#endif // end of _EYEBLINKER_H_
