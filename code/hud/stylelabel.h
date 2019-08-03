#ifndef _STYLELABEL_H
#define _STYLELABEL_H

// Necessary includes
#include "hud/blendablehudunit.h"
#include "hud/hudtext.h"

#include "effect/screensprite.h"
#include "editable/enumlist.h"
#include "game/combatstyle.h"

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderDef
//!
//------------------------------------------------------------------------------------------
class StyleLabelRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( StyleLabelRenderDef );

public:
	// Construction destruction
	StyleLabelRenderDef( void );
	~StyleLabelRenderDef( void );

	virtual CHudUnit* CreateInstance( void ) const;

	// Layout
	float		m_fTopLeftX;		// Position for style label
	float		m_fTopLeftY;		// Position for style label
	float		m_fScale;			// Scale for style label

	float		m_fPositiveYOffset;
	float		m_fNegitiveYOffset;

	CVector		m_obPositiveColour;
	CVector		m_obNegativeColour;

	float		m_fOnScreenTime;

	ntstd::String m_obStyleLabelFont;

	CPoint m_obPositiveVelocity;
	CPoint m_obPositiveAcceleration;

	CPoint m_obNegativeVelocity;
	CPoint m_obNegativeAcceleration;
	
	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	StyleLabelRenderer
//!
//------------------------------------------------------------------------------------------
class StyleLabelRenderer : public CBlendableHudUnit
{
public:
	// Construction destruction
	StyleLabelRenderer( StyleLabelRenderDef*  pobRenderDef );
	~StyleLabelRenderer( void );

	// Render our current settings
	virtual bool Render( void );
	virtual bool Update( float fTimeStep );
	bool Initialise( StyleEvent& obEvent, float fPosOffset );

private:
	StyleLabelRenderDef* m_pobDef;

	StyleEvent m_obEvent;
	

	HudTextRenderer* m_pobTextRenderer;
	HudTextRenderDef m_obTextRenderDef;

	float m_fCurrScreenTime;

#ifdef _DEBUG
	float m_fPosOffset;
#endif 

	CVector		m_obTextColour;

	CPoint m_obCurrPosition;
	CPoint m_obCurrVelocity;
	CPoint m_obCurrAcceleration;

	ScreenSprite m_obStyleLabelSprite;
};

#endif // _STYLELABEL_H
