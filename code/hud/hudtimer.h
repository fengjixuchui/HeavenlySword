#ifndef _HUDTIMER_H
#define _HUDTIMER_H

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "effect/renderstate_block.h"
#include "editable/enumlist.h"

// Forward declarations

//------------------------------------------------------------------------------------------
//!
//!	TimerRenderDef
//!	Defines for TimerRendering
//!
//------------------------------------------------------------------------------------------
class TimerRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( TimerRenderDef );

public:
	TimerRenderDef();
	~TimerRenderDef() {};

	virtual CHudUnit* CreateInstance( void ) const;

	void PostConstruct( void );


	TIMER_UNITS m_eMaxUnits;

	float m_fTopLeftX;
	float m_fTopLeftY;
	float m_fOverallScale;

	float m_fFadeSpeed;	

	CVector m_obTimeColour;
	CVector m_obTimeGlowColour;
	EFFECT_BLENDMODE m_eTimeBlendmode;
	EFFECT_BLENDMODE m_eTimeGlowBlendmode;

	CPoint m_obDaysPos;
	float m_fDaysWidth;
	float m_fDaysHeight;
	TIMER_LEADING_ZERO m_eDaysZeros;

	CPoint m_obHrsPos;
	float m_fHrsWidth;
	float m_fHrsHeight;
	TIMER_LEADING_ZERO m_eHrsZeros;
    
	CPoint m_obMinsPos;
	float m_fMinsWidth;
	float m_fMinsHeight;
	TIMER_LEADING_ZERO m_eMinsZeros;

	CPoint m_obSecsPos;
	float m_fSecsWidth;
	float m_fSecsHeight;
	TIMER_LEADING_ZERO m_eSecsZeros;

	CPoint m_obDaysDigitsPos;
	CPoint m_obHrsDigitsPos;
	CPoint m_obMinsDigitsPos;
	CPoint m_obSecsDigitsPos;

	float m_fDigitWidth;
	float m_fDigitHeight;
	float m_fDigitSpacing;
	CVector m_obDigitColour;

	CKeyString m_obDaysImage;
	CKeyString m_obHoursImage;
	CKeyString m_obMinsImage;
	CKeyString m_obSecsImage;
	CKeyString m_aobDigitImage[10];

	float m_fTimeToCatchUp;
};

//------------------------------------------------------------------------------------------
//!
//!	TimerRenderer
//!	Code for drawing lifeclock
//!
//------------------------------------------------------------------------------------------
class TimerRenderer : public CBlendableHudUnit
{
public:
	TimerRenderer( TimerRenderDef*  pobRenderDef) 
	:	CBlendableHudUnit ( pobRenderDef )
	,	m_pobRenderDef ( pobRenderDef ) 
	,	m_dLastTime ( 0.0f )
	,	m_dAimTime ( 0.0f )
	,	m_dTimeDelta( 0.0f )
	,	m_fCatchupTime( 0.0f )
	{};

	virtual ~TimerRenderer() {};

	virtual bool Render( void );
	virtual bool Update( float fTimestep) { return TimerUpdate ( fTimestep, 1.0f); };
		
	bool TimerUpdate( float fTimestep, float fScalar = 1.0f);
	virtual bool Initialise( void );

	float GetRenderWidth( void );
	float GetRenderHeight( void );

	void SetTime( double dTime, bool bForce = false );

	// FIX ME could move down heirachy
	void SetPosition ( CPoint& obPos ) { m_obPosition = obPos; };

	bool IsCatchingUp( void );
	bool IsDraining( void );

private:
	void GetClockTime(double dTime, int& iDays, int& iHours, int& iMinutes, float& fSeconds);

	void RenderDigits( int iNumber, const CPoint& obPos, bool bLeadingZero = false );

	TimerRenderDef* m_pobRenderDef;

	float m_fScale;

	double m_dLastTime;
	double m_dAimTime;
	double m_dTimeDelta;

	float m_fCatchupTime;

	

	//ScreenSprite m_aobTimeToLive;
	ScreenSprite m_obDays;
	ScreenSprite m_obHours;
	ScreenSprite m_obMins;
	ScreenSprite m_obSecs;
	ScreenSprite m_aobDigit[10];

	ScreenSprite m_obBackground;
	ScreenSprite m_obGlow;

};

#endif // _HUDTIMER_H
