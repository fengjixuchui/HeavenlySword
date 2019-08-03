//------------------------------------------------------------------------------------------
//!
//!	\file buttonhinter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/hudtext.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "hud/hudmanager.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"
#include "gui/guiunit.h"
#include "gui/guiutil.h"
#include "anim/transform.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( HudTextRenderDef )

	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS( m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS( m_fTopLeftY, TopLeftY )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBoundsWidth, 0.0f, BoundsWidth )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fBoundsHeight, 0.0f, BoundsHeight )

	PUBLISH_GLOBAL_ENUM_AS		(m_eJustifyHorizontal, JustifyHorizontal, HORIZONTAL_JUSTFICATION)
	PUBLISH_GLOBAL_ENUM_AS		(m_eJustifyVertical, JustifyVertical, VERTICAL_JUSTIFICATION)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fOverallScale, 1.0f, OverallScale )

	PUBLISH_VAR_AS( m_obStringTextID, StringTextID )
	PUBLISH_VAR_AS( m_obFontName, FontName )
END_STD_INTERFACE

void ForceLinkFunctionHUDText()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHUDText() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderDef::HudTextRenderDef
//!
//------------------------------------------------------------------------------------------
HudTextRenderDef::HudTextRenderDef() 
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.1f )
,	m_fOverallScale ( 1.0f )
{
	m_eJustifyHorizontal = JUSTIFY_CENTRE;
	m_eJustifyVertical = JUSTIFY_MIDDLE;
}

CHudUnit* HudTextRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) HudTextRenderer( (HudTextRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderDef::~HudTextRenderDef
//!
//------------------------------------------------------------------------------------------
HudTextRenderDef::~HudTextRenderDef()
{
}


//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::HudTextRenderer
//!
//------------------------------------------------------------------------------------------
HudTextRenderer::HudTextRenderer(HudTextRenderDef*  pobRenderDef) 
:	CBlendableHudUnit( pobRenderDef )
,	m_pobRenderDef( pobRenderDef )
,	m_pobString ( 0 )
,	m_pobBaseTransform( 0 )
{
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::~HudTextRenderer
//!
//------------------------------------------------------------------------------------------
HudTextRenderer::~HudTextRenderer()
{
	m_pobBaseTransform->RemoveFromParent();
	if (m_pobString)
		CStringManager::Get().DestroyString( m_pobString );
	NT_DELETE ( m_pobBaseTransform );
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
bool HudTextRenderer::Initialise( void )
{
	ntAssert ( m_pobRenderDef );

	// Create a point with the data
	CPoint obBasePoint( m_pobRenderDef->m_fTopLeftX, m_pobRenderDef->m_fTopLeftY, 0.0f );
	//m_BasePosition = obBasePoint;

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	ntAssert( !m_pobBaseTransform );
	m_pobBaseTransform = NT_NEW Transform();
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );

	obBasePoint.X() *= CGuiManager::Get().BBWidth();
	obBasePoint.Y() *= CGuiManager::Get().BBHeight();
	obBaseMatrix.SetTranslation( obBasePoint );
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobBaseTransform );

	// Set up our string
	CStringDefinition obStringDef;
	obStringDef.m_fZOffset = 0.0f;
	obStringDef.m_fXOffset = 0.0f;


	obStringDef.m_fOverallScale = m_pobRenderDef->m_fOverallScale ;

	// Translate justification enums, hopefuly can change strings to use the global enum sometime
	switch ( m_pobRenderDef->m_eJustifyHorizontal )
	{
	case JUSTIFY_LEFT:		obStringDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;		break;
	case JUSTIFY_RIGHT:		obStringDef.m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;		break;	
	case JUSTIFY_CENTRE:	obStringDef.m_eHJustify = CStringDefinition::JUSTIFY_CENTRE;	break;
	default:				ntAssert(0);													break;
	}

	switch ( m_pobRenderDef->m_eJustifyVertical )
	{
	case JUSTIFY_TOP:		obStringDef.m_eVJustify = CStringDefinition::JUSTIFY_TOP;	  	break;
	case JUSTIFY_BOTTOM:	obStringDef.m_eVJustify = CStringDefinition::JUSTIFY_BOTTOM;	break;
	case JUSTIFY_MIDDLE:	obStringDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;	break;
	default:				ntAssert(0);													break;
	}

	if ( m_pobRenderDef->m_fBoundsWidth > 0.0f )
	{
		obStringDef.m_bDynamicFormat = true;
		obStringDef.m_fBoundsWidth = m_pobRenderDef->m_fBoundsWidth;
	}

	if ( m_pobRenderDef->m_fBoundsHeight > 0.0f )
	{
		obStringDef.m_bDynamicFormat = true;
		obStringDef.m_fBoundsHeight = m_pobRenderDef->m_fBoundsHeight;
	}

	ntstd::String obFontName = m_pobRenderDef->m_obFontName;

	obStringDef.m_pobFont = GuiUtil::GetFontAbstracted( obFontName.c_str() );
	
	if ( obStringDef.m_pobFont )
		m_pobString = CStringManager::Get().MakeString( m_pobRenderDef->m_obStringTextID.c_str(), obStringDef, m_pobBaseTransform, CGuiUnit::RENDER_SCREENSPACE );
	
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::Update
//!
//------------------------------------------------------------------------------------------
bool HudTextRenderer::Update( float fTimeChange )
{
	CBlendableHudUnit::Update( fTimeChange );
	
	if ( m_pobString )
	{
		CVector obCol = m_pobString->GetColour();
		obCol.W() = m_fOverallAlpha;
		m_pobString->SetColour(obCol);
	}

	// I'm done, remove me if needed
	if ( IsDeactive() )
		return false;

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool HudTextRenderer::Render( void )
{
	if ( m_pobString )
	{
		m_pobString->Render();
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::Render
//!
//------------------------------------------------------------------------------------------
float HudTextRenderer::RenderWidth()
{
	if (!m_pobString)
		return 0.0f;

	return m_pobString->RenderWidth();
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::Render
//!
//------------------------------------------------------------------------------------------
float HudTextRenderer::RenderHeight()
{
	if (!m_pobString)
		return 0.0f;

	return m_pobString->RenderHeight();
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::SetPosition
//!
//------------------------------------------------------------------------------------------
void HudTextRenderer::SetPosition ( float fX, float fY )
{
	CPoint obPos ( fX * CGuiManager::Get().BBWidth(), fY * CGuiManager::Get().BBHeight(), 0.0f);

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obPos );
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );
}

//------------------------------------------------------------------------------------------
//!
//!	HudTextRenderer::SetColour
//!
//------------------------------------------------------------------------------------------
void HudTextRenderer::SetColour ( CVector& obColour )
{
	if ( m_pobString )
	{
		m_pobString->SetColour(obColour);
	}
}

