//------------------------------------------------------------------------------------------
//!
//!	\file stylebar.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/combatstyle.h"
#include "hud/hudmanager.h"
#include "hud/MessageBox.h"
#include "objectdatabase/dataobject.h"
#include "gui/guimanager.h"
#include "hud/hudunit.h"
#include "hud/hudtext.h"
#include "gfx/texturemanager.h"

#include "game/nsmanager.h"

#include "core/visualdebugger.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( CMessageBoxDef )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fPositionX, 0.5f, PositionX )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fPositionY, 0.5f, PositionY )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxMinHeight, 0.5f, BoxMinHeight )	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxMinWidth, 0.5f, BoxMinWidth )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxMaxHeight, 0.75f, BoxMaxHeight )	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxMaxWidth, 0.75f, BoxMaxWidth )
END_STD_INTERFACE

START_STD_INTERFACE( CMessageBoxRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxImageHeight, 32, BoxImageHeight )	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxImageWidth, 32, BoxImageWidth )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxWatermarkHeight, 256, BoxWatermarkHeight )	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fBoxWatermarkWidth, 256, BoxWatermarkWidth )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fVerticalMargin, 16, VerticalMargin )	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fHorizontalMargin, 16, HorizontalMargin )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fAlphaMessageBox, 0.8f, AlphaMessageBox )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fAlphaWatermark, 0.8f, AlphaWatermark )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fAlphaText, 0.8f, AlphaText )

	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_TOP_LEFT ],		"hud/msg_box_topleft_colour_alpha_nomip.dds",		TopLeftImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_TOP ],			"hud/msg_box_top_colour_alpha_nomip.dds",			TopImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_TOP_RIGHT ],		"hud/msg_box_topright_colour_alpha_nomip.dds",		TopRightImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_LEFT ],			"hud/msg_box_left_colour_alpha_nomip.dds",			LeftImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_CENTRE ],		"hud/msg_box_middle_colour_alpha_nomip.dds",		MiddleImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_RIGHT ],			"hud/msg_box_right_colour_alpha_nomip.dds",			RightImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_BOTTOM_LEFT ],	"hud/msg_box_bottomleft_colour_alpha_nomip.dds",	BottomLeftImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_BOTTOM ],		"hud/msg_box_bottom_colour_alpha_nomip.dds",		BottomImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_BOTTOM_RIGHT ],	"hud/msg_box_bottomright_colour_alpha_nomip.dds",	BottomRightImage)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_aobBoxTexture [ MBI_WATERMARK ],		"hud/msg_box_watermark_colour_alpha_nomip.dds",		WatermarkImage)
	
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obFont, "Body", Font )

	PUBLISH_GLOBAL_ENUM_AS		(m_eJustifyHorizontal, JustifyHorizontal, HORIZONTAL_JUSTFICATION)
	PUBLISH_GLOBAL_ENUM_AS		(m_eJustifyVertical, JustifyVertical, VERTICAL_JUSTIFICATION)

	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)

	DECLARE_POSTCONSTRUCT_CALLBACK ( PostConstruct )
END_STD_INTERFACE

void ForceLinkFunctionCMessageBox()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCMessageBox() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBox::CMessageBox
//! Construction
//!
//------------------------------------------------------------------------------------------
CMessageBox::CMessageBox( CMessageBoxDef* pobDef )
{
	// Defaults from the welder exposed Def
	m_fPositionX		= pobDef->m_fPositionX;	
	m_fPositionY		= pobDef->m_fPositionY;	

	m_fBoxMinHeight		= pobDef->m_fBoxMinHeight;
	m_fBoxMinWidth		= pobDef->m_fBoxMinWidth;	

	m_fBoxMaxHeight		= pobDef->m_fBoxMaxHeight;
	m_fBoxMaxWidth		= pobDef->m_fBoxMaxWidth;
}

CMessageBox::CMessageBox( void )
:	m_fPositionX ( 0.5f )
,	m_fPositionY ( 0.5f )
,	m_fBoxMinHeight ( 0.5f )
,	m_fBoxMinWidth ( 0.5f )
,	m_fBoxMaxHeight ( 0.75f )
,	m_fBoxMaxWidth ( 0.75f )
{}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBox::~CMessageBox
//! Destruction
//!
//------------------------------------------------------------------------------------------
CMessageBox::~CMessageBox( void )
{
	for (TextDefIter obIt = m_aobHudTextDefList.begin(); 
		obIt != m_aobHudTextDefList.end(); ++obIt )
	{
		NT_DELETE_CHUNK (Mem::MC_MISC, *obIt);
	}
	m_aobHudTextDefList.clear();
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderDef::CMessageBoxRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
CMessageBoxRenderDef::CMessageBoxRenderDef( )
{
	m_eJustifyHorizontal = JUSTIFY_CENTRE;
	m_eJustifyVertical = JUSTIFY_MIDDLE;
	m_eBlendMode = EBM_LERP;
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderDef::~CMessageBoxRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
CMessageBoxRenderDef::~CMessageBoxRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderDef::PostConstruct
//! Construction
//!
//------------------------------------------------------------------------------------------
void CMessageBoxRenderDef::PostConstruct( void )
{
	for ( int i = 0; i < MSG_BOX_IMAGES; i++)
	{
		TextureManager::Get().LoadTexture_Neutral( m_aobBoxTexture[ i ].GetString() );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* CMessageBoxRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) CMessageBoxRenderer( (CMessageBoxRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::CMessageBoxRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
CMessageBoxRenderer::CMessageBoxRenderer( CMessageBoxRenderDef*  pobRenderDef )
:	CBlendableHudUnit ( pobRenderDef )
,	m_pobDef ( pobRenderDef )
,	m_obMessageBox ( 0 )
{
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::~CMessageBoxRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
CMessageBoxRenderer::~CMessageBoxRenderer( void )
{
	// Clean out any text
	for (TextRenderIter obIt = m_aobHudTextRendererList.begin(); 
			obIt != m_aobHudTextRendererList.end(); ++obIt )
	{
		NT_DELETE_CHUNK (Mem::MC_MISC, *obIt);
	}
	m_aobHudTextRendererList.clear();

	m_obMessageBox = CSharedPtr<CMessageBox,Mem::MC_MISC>(0);
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::Initialise
//! Initialise
//!
//------------------------------------------------------------------------------------------
bool CMessageBoxRenderer::Initialise( void )
{
	for ( int i = 0; i < MSG_BOX_IMAGES; i++)
	{
		m_aobBoxSprite[ i ].SetTexture( m_pobDef->m_aobBoxTexture[ i ] );
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::Initialise
//! Initialise - setup specific to the message box
//!
//------------------------------------------------------------------------------------------
bool CMessageBoxRenderer::Initialise(const CSharedPtr<CMessageBox,Mem::MC_MISC>& obMsgBox)
{
	CMessageBox* const pobMsgBox = obMsgBox.Get();
	
	ntAssert ( pobMsgBox );

	if (! m_pobDef )
		return true;

	// Keep refrence to our message box
	m_obMessageBox = obMsgBox;

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	float fTextScale = fBBWidth/1280.0f;

	float fActualHeight = 0.0f;
	float fActualWidth = 0.0f;

	float fInternalMinWidth		= pobMsgBox->m_fBoxMinWidth  * fBBWidth  - 2.0f * m_pobDef->m_fHorizontalMargin;
	float fInternalMinHeight	= pobMsgBox->m_fBoxMinHeight * fBBHeight - 2.0f * m_pobDef->m_fVerticalMargin;
	float fInternalMaxWidth		= pobMsgBox->m_fBoxMaxWidth  * fBBWidth  - 2.0f * m_pobDef->m_fHorizontalMargin;
	float fInternalMaxHeight	= pobMsgBox->m_fBoxMaxHeight * fBBHeight - 2.0f * m_pobDef->m_fVerticalMargin;

	// Setup any text
	for (TextDefIter obIt = pobMsgBox->m_aobHudTextDefList.begin(); 
			obIt != pobMsgBox->m_aobHudTextDefList.end(); ++obIt )
	{
		HudTextRenderDef* pobNewText = *obIt;
	
		// Box properties relavent to its text
		pobNewText->m_obFontName = m_pobDef->m_obFont;
		pobNewText->m_fBoundsWidth = fInternalMaxWidth;
		pobNewText->m_fBoundsHeight = fInternalMaxHeight;
		pobNewText->m_eJustifyHorizontal = m_pobDef->m_eJustifyHorizontal;
		pobNewText->m_eJustifyVertical = m_pobDef->m_eJustifyVertical;

		pobNewText->m_fOverallScale = fTextScale;

		// Create the text
		HudTextRenderer* pobNewTextRenderer = (HudTextRenderer*)pobNewText->CreateInstance();
		pobNewTextRenderer->Initialise();
		m_aobHudTextRendererList.push_back( pobNewTextRenderer );

		// Keep track of the maximum text width
		if ( pobNewTextRenderer->RenderWidth() > fActualWidth )
		{
			fActualWidth = pobNewTextRenderer->RenderWidth();
		}

		// Total the text heights
		fActualHeight += pobNewTextRenderer->RenderHeight();
	}

	fActualWidth  = ntstd::Clamp( fActualWidth,  fInternalMinWidth,  fInternalMaxWidth  );
	fActualHeight = ntstd::Clamp( fActualHeight, fInternalMinHeight, fInternalMaxHeight );

	float fPositionX = pobMsgBox->m_fPositionX * fBBWidth;
	float fPositionY = pobMsgBox->m_fPositionY * fBBHeight;

	switch ( m_pobDef->m_eJustifyHorizontal )
	{
	case JUSTIFY_LEFT:		fPositionX = fPositionX - 0.5f * fActualWidth /*+ m_pobDef->m_fHorizontalMargin*/;	break;
	case JUSTIFY_RIGHT:		fPositionX = fPositionX + 0.5f * fActualWidth /*- m_pobDef->m_fHorizontalMargin*/;	break;	
	case JUSTIFY_CENTRE:																					break;
	default:				ntAssert(0);																	break;
	}

	switch ( m_pobDef->m_eJustifyVertical )
	{
	case JUSTIFY_TOP:		fPositionY = fPositionY - 0.5f * fActualHeight /*+ m_pobDef->m_fVerticalMargin*/;	break;
	case JUSTIFY_BOTTOM:	fPositionY = fPositionY + 0.5f * fActualHeight /*- m_pobDef->m_fVerticalMargin*/;	break;
	case JUSTIFY_MIDDLE:																					break;
	default:				ntAssert(0);																	break;
	}

	for (TextRenderIter obIt = m_aobHudTextRendererList.begin(); 
		obIt != m_aobHudTextRendererList.end(); ++obIt )
	{
		// Position text
		(*obIt)->SetPosition( fPositionX / fBBWidth, fPositionY / fBBHeight  );
		fPositionY += (*obIt)->RenderHeight();
	}

	// Set actual box size
	m_fBoxHeight = ( fActualHeight + 2.0f * m_pobDef->m_fVerticalMargin   ) / fBBHeight;
	m_fBoxWidth  = ( fActualWidth  + 2.0f * m_pobDef->m_fHorizontalMargin ) / fBBWidth;

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool CMessageBoxRenderer::Render( void)
{
	CMessageBox* const pobMsgBox =	m_obMessageBox.Get();
	
	if (! pobMsgBox )
		return true;

	if (! m_pobDef )
		return true;

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	float fResolutionScale = fBBWidth/1280.0f;

	float fPositionX = pobMsgBox->m_fPositionX * fBBWidth;
	float fPositionY = pobMsgBox->m_fPositionY * fBBHeight;
	float fHeight	 = m_fBoxHeight * fBBHeight;
	float fWidth	 = m_fBoxWidth * fBBWidth;

	float fVerticalOffset   = 0.5f * (fHeight - m_pobDef->m_fBoxImageHeight);
	float fHorizontalOffset = 0.5f * (fWidth  - m_pobDef->m_fBoxImageWidth);
	
	float fWatermarkHeight = m_pobDef->m_fBoxWatermarkHeight * fResolutionScale ;
	float fWatermarkWidth = m_pobDef->m_fBoxWatermarkWidth * fResolutionScale ;

	float fInternalHeight = fHeight - 2.0f * m_pobDef->m_fBoxImageHeight;
	float fInternalWidth = fWidth - 2.0f * m_pobDef->m_fBoxImageWidth;

	if ( fWatermarkHeight > (fHeight - m_pobDef->m_fBoxImageHeight) )
	{
		float fWatermarkScale = (fHeight - m_pobDef->m_fBoxImageHeight) / fWatermarkHeight;
		fWatermarkHeight *= fWatermarkScale;
		fWatermarkWidth *= fWatermarkScale;
	}

	float fWatermarkX = 0.5f * (fWidth  - fWatermarkWidth);
	float fWatermarkY = 0.5f * (fHeight - fWatermarkHeight);

	CPoint obPosition = CPoint(	fPositionX, fPositionY, 0.0f);

	RenderStateBlock::SetBlendMode(m_pobDef->m_eBlendMode);

	// Top Left
	m_aobBoxSprite[MBI_TOP_LEFT].SetPosition( CPoint( fPositionX - fHorizontalOffset , fPositionY - fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_TOP_LEFT].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_TOP_LEFT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_TOP_LEFT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_TOP_LEFT].Render();

	// Top 
	m_aobBoxSprite[MBI_TOP].SetPosition( CPoint( fPositionX, fPositionY - fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_TOP].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_TOP].SetWidth( fInternalWidth );
	m_aobBoxSprite[MBI_TOP].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_TOP].Render();

	// Top Right
	m_aobBoxSprite[MBI_TOP_RIGHT].SetPosition( CPoint( fPositionX + fHorizontalOffset, fPositionY - fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_TOP_RIGHT].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_TOP_RIGHT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_TOP_RIGHT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_TOP_RIGHT].Render();
	
	// Left
	m_aobBoxSprite[MBI_LEFT].SetPosition( CPoint( fPositionX - fHorizontalOffset, fPositionY, 0.0f) );
	m_aobBoxSprite[MBI_LEFT].SetHeight( fInternalHeight );
	m_aobBoxSprite[MBI_LEFT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_LEFT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_LEFT].Render();

	// Middle 
	m_aobBoxSprite[MBI_CENTRE].SetPosition( CPoint( fPositionX, fPositionY, 0.0f) );
	m_aobBoxSprite[MBI_CENTRE].SetHeight( fInternalHeight );
	m_aobBoxSprite[MBI_CENTRE].SetWidth( fInternalWidth );
	m_aobBoxSprite[MBI_CENTRE].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_CENTRE].Render();

	// Right
	m_aobBoxSprite[MBI_RIGHT].SetPosition( CPoint( fPositionX + fHorizontalOffset, fPositionY, 0.0f) );
	m_aobBoxSprite[MBI_RIGHT].SetHeight( fInternalHeight );
	m_aobBoxSprite[MBI_RIGHT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_RIGHT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_RIGHT].Render();

	// Bottom Left
	m_aobBoxSprite[MBI_BOTTOM_LEFT].SetPosition( CPoint( fPositionX - fHorizontalOffset, fPositionY + fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_BOTTOM_LEFT].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_BOTTOM_LEFT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_BOTTOM_LEFT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_BOTTOM_LEFT].Render();

	// Bottom 
	m_aobBoxSprite[MBI_BOTTOM].SetPosition( CPoint( fPositionX, fPositionY + fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_BOTTOM].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_BOTTOM].SetWidth( fInternalWidth );
	m_aobBoxSprite[MBI_BOTTOM].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_BOTTOM].Render();

	// Bottom Right
	m_aobBoxSprite[MBI_BOTTOM_RIGHT].SetPosition( CPoint( fPositionX + fHorizontalOffset, fPositionY + fVerticalOffset, 0.0f) );
	m_aobBoxSprite[MBI_BOTTOM_RIGHT].SetHeight( m_pobDef->m_fBoxImageHeight );
	m_aobBoxSprite[MBI_BOTTOM_RIGHT].SetWidth( m_pobDef->m_fBoxImageWidth );
	m_aobBoxSprite[MBI_BOTTOM_RIGHT].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaMessageBox * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_BOTTOM_RIGHT].Render();

	// WaterMark 
	m_aobBoxSprite[MBI_WATERMARK].SetPosition( CPoint( fPositionX + fWatermarkX, fPositionY + fWatermarkY, 0.0f) );
	m_aobBoxSprite[MBI_WATERMARK].SetHeight( fWatermarkHeight );
	m_aobBoxSprite[MBI_WATERMARK].SetWidth( fWatermarkWidth );
	m_aobBoxSprite[MBI_WATERMARK].SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaWatermark * m_fOverallAlpha) );
	m_aobBoxSprite[MBI_WATERMARK].Render();

#ifdef _DEBUG
	CPoint topRight( fPositionX + 0.5f * fWidth, fPositionY - 0.5f * fHeight, 0.0f);
	CPoint topLeft( fPositionX - 0.5f * fWidth, fPositionY - 0.5f * fHeight, 0.0f);
	CPoint botRight( fPositionX + 0.5f * fWidth, fPositionY + 0.5f * fHeight, 0.0f);
	CPoint botLeft( fPositionX - 0.5f * fWidth, fPositionY + 0.5f * fHeight, 0.0f);
		
	g_VisualDebug->RenderLine( topLeft, topRight, DC_RED, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( topRight, botRight, DC_RED, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( botRight, botLeft, DC_RED, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( botLeft, topLeft, DC_RED, DPF_DISPLAYSPACE );
	
	topRight.X() -= m_pobDef->m_fHorizontalMargin;
	topRight.Y() += m_pobDef->m_fVerticalMargin;

	topLeft.X() += m_pobDef->m_fHorizontalMargin;
	topLeft.Y() += m_pobDef->m_fVerticalMargin;

	botRight.X() -= m_pobDef->m_fHorizontalMargin;
	botRight.Y() -= m_pobDef->m_fVerticalMargin;

	botLeft.X() += m_pobDef->m_fHorizontalMargin;
	botLeft.Y() -= m_pobDef->m_fVerticalMargin;

	g_VisualDebug->RenderLine( topLeft, topRight, DC_GREEN, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( topRight, botRight, DC_GREEN, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( botRight, botLeft, DC_GREEN, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( botLeft, topLeft, DC_GREEN, DPF_DISPLAYSPACE );

	CPoint Top	 ( fPositionX, fPositionY - 0.5f * fHeight, 0.0f);
	CPoint Left	 ( fPositionX - 0.5f * fWidth, fPositionY, 0.0f);
	CPoint Right ( fPositionX + 0.5f * fWidth, fPositionY, 0.0f);
	CPoint Bottom( fPositionX, fPositionY + 0.5f * fHeight, 0.0f);

	g_VisualDebug->RenderLine( Top, Bottom, DC_GREEN, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( Left, Right, DC_GREEN, DPF_DISPLAYSPACE );
#endif // _DEBUG

	// Text
	for (TextRenderIter obIt = m_aobHudTextRendererList.begin();
						obIt != m_aobHudTextRendererList.end(); obIt++ )
	{
		CVector obTextColour( 1.0f, 1.0f, 1.0f, m_fOverallAlpha * m_pobDef->m_fAlphaText );
		(*obIt)->SetColour( obTextColour );
		(*obIt)->Render();
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer::Update
//! Update our current settings
//!
//------------------------------------------------------------------------------------------
bool CMessageBoxRenderer::Update( float fTimeStep )
{
	ntAssert( m_pobDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	// When done we should request that we are removed
	if ( IsDeactive() )
		return false;

	// Text
	for (TextRenderIter obIt = m_aobHudTextRendererList.begin();
						obIt != m_aobHudTextRendererList.end(); obIt++ )
	{
		(*obIt)->Update(fTimeStep);
	}

	// If not active - 'cos we shouldn't update while blending
	//if ( ! IsActive() )
	//	return true;

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CMessageBoxRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CMessageBoxRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CMessageBoxRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CMessageBoxRenderer::UpdateInactive( float fTimestep )
{
	 CBlendableHudUnit::UpdateInactive ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		CMessageBoxRenderer::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool CMessageBoxRenderer::BeginExit( bool bForce )
{	
	return CBlendableHudUnit::BeginExit( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		CMessageBoxRenderer::BeginEnter
*
*	DESCRIPTION		Kicks off the enter state for the unit.
*
***************************************************************************************************/
bool CMessageBoxRenderer::BeginEnter( bool bForce )
{	
	return CBlendableHudUnit::BeginEnter( bForce );
}
