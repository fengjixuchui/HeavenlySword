#ifndef _STYLEBAR_H
#define _STYLEBAR_H

class HitCounter;
class StyleLabelRenderDef;

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "editable/enumlist.h"


#define STYLE_LEVELS (HL_SPECIAL)
#define STYLE_SOUNDS (HL_SPECIAL + 1)

class StyleBarFillDef
{
	HAS_INTERFACE( StyleBarFillDef );

public:
	float m_fBarMin;			// Texture offset for bar min, so we don't scale over unused texture
	float m_fBarMax;			// Texture offset for bar max, so we don't scale over unused texture

protected:

	friend class HitCounterRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderDef
//!
//------------------------------------------------------------------------------------------
class HitCounterRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( HitCounterRenderDef );

public:
	// Construction destruction
	HitCounterRenderDef( void );
	~HitCounterRenderDef( void );

	virtual CHudUnit* CreateInstance( void ) const;

	void PostConstruct( void );

	// Layout
	float		m_fTopLeftX;		// Position for style bar
	float		m_fTopLeftY;		// Position for style bar
	float		m_fScale;			// Scale for style bar
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

	float		m_fAlphaForBarHigh;	// Bar Glow Alpha parameters
	float		m_fAlphaForBarLow;
	float		m_fAlphaForBarDelta;
	
	float		m_fScalarForBarPulseHigh;	// Glow Size parameters
	float		m_fScalarForBarPulseLow;
	float		m_fScalarForBarPulseDelta;
									
	int			m_iMaxBlobs;
	float		m_fGlowXDelta[STYLE_LEVELS];			// Separation between style bar glows
	float		m_fGlowWidth;				// Width of glow texture
	float		m_fGlowHeight;			// Height of glow texture
	float		m_fGlowXOffset;			// Offset for inital glow
	float		m_fGlowYOffset;
	float		m_fGlowPercent;			// Style point percent before glow starts to blend in

	float		m_fGlowFadeTime;			// Fade time for style bar glows
	float		m_fGlowGrowTime;			// Blend time for style bar glows

	ntstd::List<StyleBarFillDef*> m_aobBarDefList;	// Definitions for the fill bars
	float m_fBarFadeTime;		// Fade time for style bar
	float m_fBarGrowTime;		// Grow time for style bar

	float m_fInactivityTimeout;
	
	CHashedString m_aobStyleLevelSounds[STYLE_SOUNDS];

	CHashedString m_obStyleTexture;
	float m_fStyleTextureRatio;

	CKeyString m_obSSBarTexture;
	CKeyString m_obSSBarGlowTexture;
	CKeyString m_obSSBaseTexture[STYLE_LEVELS];

	CKeyString m_obSSGlowTexture;

	StyleLabelRenderDef* m_pobLabelRenderDef;

	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer
//!
//------------------------------------------------------------------------------------------
class HitCounterRenderer : public CBlendableHudUnit
{
public:
	// Construction destruction
	HitCounterRenderer( HitCounterRenderDef*  pobRenderDef );
	~HitCounterRenderer( void );

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

	float m_fBarPluseAlpha;
	float m_fBarPulseScalar;

	float m_fTopLeftX;
	float m_fTopLeftY;
	//float m_fScale;
	float m_fDigitHeight;
	float m_fGrabHeight;
	float m_fStyleHeight;
	float m_fStyleWidth;

	float m_fGlowAlpha[STYLE_LEVELS];
	float m_fSSBarAlpha;

	HitCounterRenderDef* m_pobDef;
	HitCounter* m_pobHitCounter;

	HIT_LEVEL m_eLastHitLevel;

	float m_fLastStylePoints;
	float m_fAimStylePoints;
	float m_fFadeStylePoints;

	float m_fBarFillBlendTime;
	float m_fStyleBlendTime;
	float m_fStyleFadeTime;

	float m_fStyleThisFrame;

	float m_fInactivityTimer;

	int m_iFadeBlobs;

	ScreenSprite m_obStyleBarSprite;
	ScreenSprite m_obStyleBarGlowSprite;
	ScreenSprite m_obStyleBaseSprite;
	ScreenSprite m_obStyleGlowSprite;

	ntstd::Vector<CHudUnit*>   m_aobLabelRenderList;
};

typedef ntstd::Vector<CHudUnit*>::iterator  LabelRenderIter;

#endif // _STYLEBAR_H
