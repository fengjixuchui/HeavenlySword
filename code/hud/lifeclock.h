#ifndef _LIFECLOCK_H
#define _LIFECLOCK_H

// Necessary includes
#include "hud/blendablehudunit.h"
#include "hud/hudtimer.h"

#include "game/combatstyle.h"
#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "effect/renderstate_block.h"
#include "editable/enumlist.h"

// Forward declarations

//------------------------------------------------------------------------------------------
//!
//!	LifeClockRenderDef
//!	Defines for LifeClockRendering
//!
//------------------------------------------------------------------------------------------
class LifeClockRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( LifeClockRenderDef );

public:
	LifeClockRenderDef();
	~LifeClockRenderDef() {};

	virtual CHudUnit* CreateInstance( void ) const;

	float m_fTopLeftX;
	float m_fTopLeftY;
	float m_fOverallScale;

	float m_fFadeSpeed;	

	CPoint m_obTimeToLivePos;
	float m_fTimeToLiveWidth;
	float m_fTimeToLiveHeight;
	CVector m_obTimeToLiveColour;
	CVector m_obTimeToLiveGlowColour;
	EFFECT_BLENDMODE m_eTimeToLiveBlendmode;
	EFFECT_BLENDMODE m_eTimeToLiveGlowBlendmode;

	CPoint m_obTimerPos;

	TimerRenderDef* m_pobTimerRenderDef;

	ScreenSprite m_aobTimeToLive;
};

//------------------------------------------------------------------------------------------
//!
//!	LifeClockRenderer
//!	Code for drawing lifeclock
//!
//------------------------------------------------------------------------------------------
class LifeClockRenderer : public CBlendableHudUnit
{
public:
	LifeClockRenderer( LifeClockRenderDef*  pobRenderDef);

	virtual ~LifeClockRenderer();

	virtual bool Render( void );
	virtual bool Update( float fTimestep );
	virtual bool Initialise( void );

private:
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );

	LifeClockRenderDef* m_pobRenderDef;

	LifeClock* m_pobLifeClock;

	TimerRenderer* m_pobTimerRenderer;

	bool m_bDrainingLastFrame;
};

#endif // _LIFECLOCK_H
