#ifndef _HEALTH_BAR_H
#define _HEALTH_BAR_H

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "editable/enumlist.h"

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderDef
//!
//------------------------------------------------------------------------------------------
class HealthBarRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( HealthBarRenderDef );

public:
	// Construction destruction
	HealthBarRenderDef( void );
	~HealthBarRenderDef( void );

	virtual CHudUnit* CreateInstance( void ) const;

	// Layout
	float		m_fTopLeftX;	// Position for special style bar
	float		m_fTopLeftY;	// Position for special style bar
	float		m_fScale;		// Scale for special style bar
	float		m_fBackgroundHeight;		// Size of style bar images
	float		m_fBackgroundWidth;		// Size of style bar images
	
	float m_fBarMin;			// Texture offset for bar min, so we don't scale over unused texture
	float m_fBarMax;			// Texture offset for bar max, so we don't scale over unused texture

	float m_fBarFadeTime;		// Fade time for style bar
	float m_fBarGrowTime;		// Grow time for style bar

	float m_fAlphaForHealthBar;

	CHashedString m_obStyleTexture;

	CKeyString m_obBaseTexture;
	CKeyString m_obBarTexture;

	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderer
//!	Will render a number using sprites - not using a proper font because we don't have that
//!	yet.
//!
//------------------------------------------------------------------------------------------
class HealthBarRenderer : public CBlendableHudUnit
{
public:
	// Construction destruction
	HealthBarRenderer( HealthBarRenderDef*  pobRenderDef );
	~HealthBarRenderer( void );

	// Render our current settings
	virtual bool Render( void );
	virtual bool Update( float fTimeStep );
	virtual bool Initialise( void );

private:
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );

	float m_fTopLeftX;
	float m_fTopLeftY;
	
	float m_fBackgroundHeight;
	float m_fBackgroundWidth;

	HealthBarRenderDef* m_pobDef;

	float m_fHealthBlendTime;
	//float m_fHealthFadeTime;

	float m_fLastHealth;
	float m_fAimHealth;
	float m_fHealthThisFrame;

	ScreenSprite m_obHealthBaseSprite;
	ScreenSprite m_obHealthBarSprite;

	Player*  m_pobCharacter;
};

#endif // _HEALTH_BAR_H
