#ifndef _SPECIAL_STYLEBAR_H
#define _SPECIAL_STYLEBAR_H


class HitCounter;
class StyleLabelRenderDef;

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "editable/enumlist.h"

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef
//!
//------------------------------------------------------------------------------------------
class SpecialHitCounterRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( SpecialHitCounterRenderDef );

public:
	// Construction destruction
	SpecialHitCounterRenderDef( void );
	~SpecialHitCounterRenderDef( void );

	virtual CHudUnit* CreateInstance( void ) const;

	// Layout
	float		m_fTopLeftX;	// Position for special style bar
	float		m_fTopLeftY;	// Position for special style bar
	float		m_fScale;		// Scale for special style bar
	float		m_fStyleHeight;		// Size of style bar images
	float		m_fStyleWidth;		// Size of style bar images
	float		m_fAlphaForLogoFadeHigh;	// Glow Alpha parameters
	float		m_fAlphaForLogoFadeLow;
	float		m_fAlphaForLogoFadeDelta;
	float		m_fAlphaForStyleBar;
	float		m_fAlphaForLogoActive;								
	float		m_fScalarForLogoPulseHigh;	// Glow Size parameters
	float		m_fScalarForLogoPulseLow;
	float		m_fScalarForLogoPulseDelta;

	float m_fGlowXDelta;			// Separation between style bar glows
	float m_fGlowWidth;				// Width of glow texture
	float m_fGlowHeight;			// Height of glow texture
	float m_fGlowXOffset;		// Offest for special glow
	float m_fGlowYOffset;	
	float m_fBarMin;			// Texture offset for bar min, so we don't scale over unused texture
	float m_fBarMax;			// Texture offset for bar max, so we don't scale over unused texture

	float m_fGlowFadeTime;			// Fade time for style bar glows
	float m_fGlowGrowTime;			// Blend time for style bar glows
	float m_fBarFadeTime;		// Fade time for style bar
	float m_fBarGrowTime;		// Grow time for style bar

	float m_fInactivityTimeout;

	float m_fTimeToHint;			// Time until NS button hint active
	float m_fButtonHintRadius;		// Proximity radius for hint

	ntstd::String m_obStyleLevelSound;

	CHashedString m_obStyleTexture;

	CKeyString m_obSSGlowTexture;
	CKeyString m_obSSBaseTexture;
	CKeyString m_obSSBarTexture;

	StyleLabelRenderDef* m_pobLabelRenderDef;

	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderer
//!	Will render a number using sprites - not using a proper font because we don't have that
//!	yet.
//!
//------------------------------------------------------------------------------------------
class SpecialHitCounterRenderer : public CBlendableHudUnit
{
public:
	// Construction destruction
	SpecialHitCounterRenderer( SpecialHitCounterRenderDef*  pobRenderDef );
	~SpecialHitCounterRenderer( void );

	// Render our current settings
	virtual bool Render( void );
	virtual bool Update( float fTimeStep );
	virtual bool Initialise( void );

	virtual bool	BeginExit( bool bForce = false );

	// Style label helpers
	float BarPosForStyle( int iStyle );

private:
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );

	float m_fBlobAlpha;
	float m_fPulseScalar;

	float m_fTopLeftX;
	float m_fTopLeftY;
	
	float m_fDigitHeight;
	float m_fGrabHeight;
	float m_fStyleHeight;
	float m_fStyleWidth;

	float m_fGlowAlpha;
	float m_fSSBarAlpha;

	SpecialHitCounterRenderDef* m_pobDef;
	HitCounter* m_pobHitCounter;

	HIT_LEVEL m_eLastHitLevel;

	float m_fLastStylePoints;
	float m_fAimStylePoints;
	float m_fFadeStylePoints;

	float m_fBarBlendTime;
	float m_fStyleBlendTime;
	float m_fStyleFadeTime;

	float m_fStyleThisFrame;
		
	float m_fInactivityTimer;

	float m_fButtonHintTime;

	int m_bHintActive;

	int m_iFadeBlobs;

	ScreenSprite m_obStyleBaseSprite;
	ScreenSprite m_obStyleBarSprite;
	ScreenSprite m_obStyleGlowSprite;

	ntstd::Vector<CHudUnit*>   m_aobLabelRenderList;
};

typedef ntstd::Vector<CHudUnit*>::iterator  LabelRenderIter;

#endif // _SPECIAL_STYLEBAR_H
