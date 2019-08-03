/***************************************************************************************************
*
*	DESCRIPTION		Simple moon sprite based effect.
*
***************************************************************************************************/

#ifndef EFFECT_MOON_H
#define EFFECT_MOON_H

#ifndef _EFFECT_H
#include "effect/effect.h"
#endif

class WorldSprite;

/***************************************************************************************************
*
*	CLASS			CEffectMoonDef
*
*	DESCRIPTION		XML moon definition structure
*
***************************************************************************************************/
class CEffectMoonDef
{
public:
	CEffectMoonDef( void );
	virtual ~CEffectMoonDef(){};

	virtual void PostConstruct( void );

	float		m_fAngularSize;
	float		m_fLuminosity;
	CVector		m_obColour;
};

/***************************************************************************************************
*
*	CLASS			EffectMoon
*
*	DESCRIPTION		wrapper round a sprite object
*
***************************************************************************************************/
class EffectMoon : public Effect
{
public:
	EffectMoon( const CEffectMoonDef* pDef );
	virtual ~EffectMoon();

	virtual bool UpdateEffect();
	virtual void RenderEffect();
	virtual bool HighDynamicRange() const { return true; }
	virtual bool WaitingForResources()const { return false; }

private:
	const CEffectMoonDef*		m_pDef;
	WorldSprite*				m_pMoonSprite;
};

#endif // EFFECT_MOON_H
