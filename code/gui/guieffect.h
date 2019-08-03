/***************************************************************************************************
*
*	DESCRIPTION		Simple effect to add gui world space sprites.
*
***************************************************************************************************/

#ifndef EFFECT_GUI_H
#define EFFECT_GUI_H

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
/*class CEffectMoonDef
{
public:
	CEffectMoonDef( void );
	virtual ~CEffectMoonDef(){};

	virtual void PostConstruct( void );

	float		m_fAngularSize;
	float		m_fLuminosity;
	CVector		m_obColour;
};*/

/***************************************************************************************************
*
*	CLASS			EffectGui
*
*	DESCRIPTION		wrapper round a sprite object
*
***************************************************************************************************/
class EffectGui : public Effect
{
public:
	EffectGui( WorldSprite*	pWorldSprite );
	virtual ~EffectGui();

	virtual bool UpdateEffect();
	virtual void RenderEffect();
	virtual bool WaitingForResources()const { return false; }
	virtual bool HighDynamicRange() const { return true; }

private:
	//const CEffectMoonDef*		m_pDef;
	u_int						m_iEffectId;
	WorldSprite*				m_pGuiSprite;
};

#endif // EFFECT_GUI_H
