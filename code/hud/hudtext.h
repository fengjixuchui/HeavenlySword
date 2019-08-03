#ifndef _HUDTEXT_H
#define _HUDTEXT_H

// Necessary includes
#include "hud/blendablehudunit.h"

#include "editable/enumlist.h"

#include "gui/guiutil.h"
#include "gui/guitext.h"

// Forward Declarations
class CString;
class Transform;

// Forward declarations

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderDef
//!	Defines sprites and sizes for HudTextRenderer to use
//!
//------------------------------------------------------------------------------------------
class HudTextRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( HudTextRenderDef );

public:
	HudTextRenderDef();
	~HudTextRenderDef();

	virtual CHudUnit* CreateInstance( void ) const;

	float m_fTopLeftX;
	float m_fTopLeftY;

	// Size of string bounds
	float	m_fBoundsWidth;
	float	m_fBoundsHeight;

	HORIZONTAL_JUSTFICATION	m_eJustifyHorizontal;
	VERTICAL_JUSTIFICATION	m_eJustifyVertical;

	ntstd::String	m_obStringTextID;
	ntstd::String	m_obFontName;

	float	m_fOverallScale;

	friend class HudTextRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer
//!	Renders the Hud Text
//!
//------------------------------------------------------------------------------------------
class HudTextRenderer : public CBlendableHudUnit
{
public:
	HudTextRenderer(HudTextRenderDef*  pobRenderDef);
	~HudTextRenderer();

	virtual bool Render( void );
	virtual bool Update( float fTimeChange);
	virtual bool Initialise( void );

	float RenderWidth();
	float RenderHeight();

	void SetPosition ( float fX, float fY );

	void SetColour ( CVector& obColour );

private:
	HudTextRenderDef* m_pobRenderDef;

	CString*		m_pobString;
	Transform*		m_pobBaseTransform;	// FIX ME Could move down heirachy
};

#endif // _HUDTEXT_H
