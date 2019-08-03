#ifndef _GAMECOUNTER_H
#define _GAMECOUNTER_H

// Necessary includes
#include "hud/blendablehudunit.h"
#include "hud/hudtext.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "effect/renderstate_block.h"
#include "editable/enumlist.h"

// Forward declarations

//------------------------------------------------------------------------------------------
//!
//!	BodyCount
//!	Defines for BodyCount Gizmo
//!
//------------------------------------------------------------------------------------------
class BodyCount
{
public:
	BodyCount()
		: m_iTarget ( 0 )
		, m_pobCallbackEnt ( 0)
	{};

	~BodyCount() {};

	void Update ( void );

	void BodycountCallback ( int iTarget, CEntity* pobCallbackEnt );
private:

	int m_iTarget;
	CEntity* m_pobCallbackEnt;
};

//------------------------------------------------------------------------------------------
//!
//!	GameCounterRenderDef
//!	Defines for GameCounterRender HUD element
//!
//------------------------------------------------------------------------------------------
class GameCounterRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( GameCounterRenderDef );

public:
	GameCounterRenderDef();
	~GameCounterRenderDef() {};

	virtual CHudUnit* CreateInstance( void ) const;

	void PostConstruct( void );

	float m_fTopLeftX;
	float m_fTopLeftY;
	float m_fOverallScale;


	float m_fBackgroundWidth;
	float m_fBackgroundHeight;
	
	float m_fGlowWidth;
	float m_fGlowHeight;

	float m_fGlowXOffset;
	float m_fGlowYOffset;

	float m_fGlowAlphaHigh;
	float m_fGlowAlphaLow;


	float m_fFadeSpeed;	


	CPoint m_obDigitsPos;
	
	float m_fDigitWidth;
	float m_fDigitHeight;
	float m_fDigitSpacing;
	CVector m_obDigitColour;

	CKeyString m_obBackgroundImage;
	CKeyString m_aobDigitImage[10];
	CKeyString m_obBackgroundGlowImage;

	bool	m_bReversed;

	CHashedString m_obGameData;
	CHashedString m_obInitialValue;
	ntstd::String m_obLabelTextID;
	ntstd::String m_obFont;

	float m_fTextPosX;
	float m_fTextPosY;

	float m_fCatchUpTime;

	EFFECT_BLENDMODE m_eBlendmode;
};

//------------------------------------------------------------------------------------------
//!
//!	GameCounterRenderer
//!	Code for drawing lifeclock
//!
//------------------------------------------------------------------------------------------
class GameCounterRenderer : public CBlendableHudUnit
{
public:
	GameCounterRenderer( GameCounterRenderDef*  pobRenderDef) 
	:	CBlendableHudUnit ( pobRenderDef )
	,	m_pobRenderDef ( pobRenderDef ) 
	,	m_fScale ( 1.0f )
	,	m_pobTextRenderer ( 0 )
	,	m_iLastGameCounter ( 0 )
	,	m_iAimGameCounter ( 0 )
	,	m_iGameCounter ( 0 )
	,	m_fCatchUpTime ( 0.0f )
	,	m_fCurrGlowTime ( 0.0f )
	,	m_fCurrGlowAlpha ( 0.0f )
	{};

	virtual ~GameCounterRenderer();

	virtual bool Render( void );
	virtual bool Update( float fTimestep );
	virtual bool Initialise( void );

private:
	void RenderDigits( int iNumber, const CPoint& obPos, bool bLeadingZero = false );

	virtual void UpdateActive( float fTimestep );
	virtual void UpdateInactive( float fTimestep );

	GameCounterRenderDef* m_pobRenderDef;

	float m_fScale;

	ScreenSprite m_aobDigit[10];

	ScreenSprite m_obBackground;
	ScreenSprite m_obGlow;

	HudTextRenderer* m_pobTextRenderer;
	HudTextRenderDef m_obTextRenderDef;

	int m_iLastGameCounter;
	int m_iAimGameCounter;
	int m_iGameCounter;

	float m_fCatchUpTime;

	float m_fCurrGlowTime;
	float m_fCurrGlowAlpha;

};

#endif // _GAMECOUNTER_H
