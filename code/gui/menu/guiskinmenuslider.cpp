/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenuslider.h"

#include "game/playeroptions.h"
#include "gfx/renderer.h"
#include "gui/guimanager.h"
#include "gui/guilua.h"
#include "gui/guiscreen.h"
#include "gui/guisettings.h"

#define VALUE_OFFSET 20.0f
#define CENTER_WIDTH 366.0f
#define	SLIDERBAR_MOVEMENT_MULTIPLIER 0.55f

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuSlider(); }

// Register this class under it's XML tag
bool g_bMENUSLIDER = CGuiManager::Register( "MENUSLIDER", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::CGuiSkinMenuSlider
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuSlider::CGuiSkinMenuSlider( void )
	: m_iMinValue(0)
	, m_iMaxValue(100)
	, m_iStep(1)
	, m_pcCallback(NULL)
	, m_pcCallbackActivationChange(NULL)
	, m_iCurrentValue(0)
	, m_fExternalInitialiser( 0.0f )
	, m_pobValueString(NULL)
	, m_fSliderXPos( 0.0f )
	, m_fSliderYPos( 0.0f )
	, m_fSliderBarDivisionWidth( 0.0f )
	, m_fGfxPositionX( 0.0f )
	, m_fPixelsShowing( 0.0f )
	, m_fPixelsToShowTarget( 0.0f )
	, m_iStyleBits( GS_VALUE )
{}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::~CGuiSkinMenuSlider
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuSlider::~CGuiSkinMenuSlider( void )
{
	NT_DELETE_ARRAY(m_pcCallback);
	NT_DELETE_ARRAY(m_pcCallbackActivationChange);

	if (m_pobValueString)
		CStringManager::Get().DestroyString(m_pobValueString);
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "callback" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcCallback );
		}
		else if ( strcmp( pcTitle, "callbackactivationchange" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcCallbackActivationChange );
		}
		else if ( strcmp( pcTitle, "min" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iMinValue);
		}
		else if ( strcmp( pcTitle, "max" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iMaxValue);
		}
		else if ( strcmp( pcTitle, "step" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iStep);
		}
		else if ( strcmp( pcTitle, "playeroptionsfloat" ) == 0 )
		{
			m_fExternalInitialiser = CPlayerOptions::Get().GetFloat( pcValue );
			return true;
		}
		else if ( strcmp( pcTitle, "gfxbaseposition" ) == 0 )
		{
			return ProcessGfxBasePositionValue( pcValue );
		}
		else if ( strcmp( pcTitle, "style" ) == 0 )
		{
			return ProcessStyleValue( pcValue );
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	ntAssert(m_pcCallback);
	ntAssert(m_iMinValue < m_iMaxValue);
	ntAssert(m_iStep > 0);

	if	( m_fExternalInitialiser > 0.0f )
	{
		m_iCurrentValue = (int)((float)m_iMaxValue * m_fExternalInitialiser);
	
		m_iCurrentValue = max( 0, m_iCurrentValue );
		m_iCurrentValue = min( m_iMaxValue, m_iCurrentValue );
	}
	else
	{
		m_iCurrentValue = m_iMinValue;
	}

	CreateValueString();

	InitialiseSliderSprites();
	UpdateSliderSprites( false );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::CreateValueString
*
*	DESCRIPTION		[Re]creates the string which holds and displays the toggle's value.
*
***************************************************************************************************/

void CGuiSkinMenuSlider::CreateValueString()
{
	if	( ! DisplayValue() )
	{
		return;
	}

	if (m_pobValueString)
		CStringManager::Get().DestroyString(m_pobValueString);

	char cBuf[32] = {0};
	sprintf(cBuf, "%d", m_iCurrentValue);

	size_t iTextLength = strlen(cBuf);

	WCHAR_T wcBuf[32] = {0};
	ntAssert(iTextLength <= 32);

	mbstowcs(wcBuf, cBuf, iTextLength);

	super::CalculateExtents();	// this will populate the extents struct with text extents
	super::SetExtentsDirty();

	CStringDefinition obDef = StringDef();
	obDef.m_fXOffset += m_obExtents.fWidth + VALUE_OFFSET;
	obDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;
	m_pobValueString = CStringManager::Get().MakeString(wcBuf, obDef, m_pobBaseTransform, m_eRenderSpace);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::MoveLeftAction
*
*	DESCRIPTION		Decreases the current value
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::MoveLeftAction( int iPads )
{
	UNUSED(iPads);
	if (m_iCurrentValue > m_iMinValue)
	{
		bool bUpdateCurrentValue = true;

		// Only allow the internal slider value to change if the display is close enough to the target.
		// This stops situations where the internal value races ahead of the display.
		if ( DisplayGraphic() )
		{
			bUpdateCurrentValue = SliderBarNearTargetPosition(); 
		}

		if ( bUpdateCurrentValue )
		{
			m_iCurrentValue -= m_iStep;

			if (m_iCurrentValue < m_iMinValue)
				m_iCurrentValue = m_iMinValue;

			CreateValueString();
			TriggerCallback();
		}
	}
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::MoveLeftAction
*
*	DESCRIPTION		Increases the current value
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::MoveRightAction( int iPads )
{
	UNUSED(iPads);
	if (m_iCurrentValue < m_iMaxValue)
	{
		bool bUpdateCurrentValue = true;

		// Only allow the internal slider value to change if the display is close enough to the target.
		// This stops situations where the internal value races ahead of the display.
		if ( DisplayGraphic() )
		{
			bUpdateCurrentValue = SliderBarNearTargetPosition(); 
		}

		if ( bUpdateCurrentValue )
		{
			m_iCurrentValue += m_iStep;

			if (m_iCurrentValue > m_iMaxValue)
				m_iCurrentValue = m_iMaxValue;

			CreateValueString();
			TriggerCallback();
		}
	}
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::TriggerCallback
*
*	DESCRIPTION		Triggers lua callback
*
***************************************************************************************************/

void CGuiSkinMenuSlider::TriggerCallback()
{
	NinjaLua::LuaFunction luaFunc = CGuiLua::GetLuaFunction(m_pcCallback);
	if (!luaFunc.IsNil())
	{
		float fMin = (float)m_iMinValue;
		float fMax = (float)m_iMaxValue;
		float fVal = (float)m_iCurrentValue;

		float fNormalised = (fVal - fMin)/(fMax - fMin);

        luaFunc(fNormalised);
	}
	else
	{
		ntPrintf("CGuiSkinMenuSlider: Failed to find lua function \"%s\"\n", m_pcCallback);
	}

}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::StartAction
*
*	DESCRIPTION		Ignore when user presses start
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::StartAction( int iPads )
{
	UNUSED(iPads);
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::SelectAction
*
*	DESCRIPTION		Ignore when user presses x
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::SelectAction( int iPads )
{
	UNUSED(iPads);
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::Render
*
*	DESCRIPTION		Show our current value
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::Render()
{
	super::Render();

	if	( m_pobValueString )
	{
		if	( IsFading() )
		{
			CGuiSkinFader::FadeStringObject( m_pobValueString, ScreenFade() );
		}
		
		m_pobValueString->Render();
	}

	if	( DisplayGraphic() )
	{
		UpdateSliderSprites( true );

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		if	( IsFading() )
		{
			FadeSliderSprites();
		}

		RenderSliderSprites();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::InitialiseSliderVisuals
//!
//! Load textures for slider bar and slider, calculate slider bar division width
//! and position the slider and bar.
//!
//--------------------------------------------------------------------------------------------------
void CGuiSkinMenuSlider::InitialiseSliderSprites( void )
{
	if	( ! DisplayGraphic() )
	{
		return;
	}

	// Get extents for the string object.
	super::CalculateExtents();	// this will populate the extents struct with text extents
	super::SetExtentsDirty();

	CStringDefinition obDef = StringDef();

	float fSliderBarPosY = m_obExtents.fY + obDef.m_fYOffset + ( m_obExtents.fHeight * 0.5f );

	//--------------------------------------------------------------------------------------------------------------------
	// Slider.
	//--------------------------------------------------------------------------------------------------------------------
	m_obSlider.SetTexture( "gui/frontend/textures/common/ow_options_sliding_cursor_colour_alpha.dds" );
	float fSliderWidth = (float)m_obSlider.GetTextureWidth();
	m_obSlider.SetWidth( fSliderWidth );
	m_obSlider.SetHeight( (float)m_obSlider.GetTextureHeight() );

	// Slider position.
	m_fSliderXPos = m_fGfxPositionX + (fSliderWidth * 0.5f);
	CPoint obSliderPos( m_fSliderXPos, fSliderBarPosY, 0.0f );
	m_obSlider.SetPosition( obSliderPos );
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	// Slider bar.
	//--------------------------------------------------------------------------------------------------------------------

	// Left Cap
	m_obStaticLeftCap.SetTexture( "gui/frontend/textures/common/ow_options_slider_track_left_cap_colour_alpha.dds" );
	m_obLeftCap.SetTexture( "gui/frontend/textures/common/ow_options_white_track_left_cap_colour_alpha.dds" );

	float fCapWidth = (float)m_obLeftCap.GetTextureWidth();
	m_fHalfCapWidth = fCapWidth * 0.5f;

	m_obStaticLeftCap.SetWidth( fCapWidth );
	m_obStaticLeftCap.SetHeight( (float)m_obStaticLeftCap.GetTextureHeight() );

	m_obLeftCap.SetWidth( fCapWidth );
	m_obLeftCap.SetHeight( (float)m_obLeftCap.GetTextureHeight() );
	//

	// Center
	m_obStaticBar.SetTexture( "gui/frontend/textures/common/ow_options_slider_track_colour_alpha.dds" );
	m_obBar.SetTexture( "gui/frontend/textures/common/ow_options_white_track_colour_alpha.dds" );

	float fCenterTextureWidth = (float)m_obStaticBar.GetTextureWidth();
	float fHalfCenterTextureWidth = fCenterTextureWidth * 0.5f;

	// Approximating from the design layout the slider bar is 366 pixels wide.
	// The end caps are 8 pixels at source but with alpha on edges are on 6 when displayed
	// Width of center bar should be 354, + 6 for both of the caps = 366.
	float fCenterWidth = CENTER_WIDTH;
	float fHalfCenterWidth = fCenterWidth * 0.5f;

	m_obStaticBar.SetWidth( fCenterWidth );
	m_obStaticBar.SetHeight( (float)m_obStaticBar.GetTextureHeight() );

	m_obBar.SetWidth( 0.0f );
	m_obBar.SetHeight( (float)m_obBar.GetTextureHeight() );
	//

	// Right Cap
	m_obStaticRightCap.SetTexture( "gui/frontend/textures/common/ow_options_slider_track_right_cap_colour_alpha.dds" );
	m_obRightCap.SetTexture( "gui/frontend/textures/common/ow_options_white_track_right_cap_colour_alpha.dds" );

	m_obStaticRightCap.SetWidth( (float)m_obStaticRightCap.GetTextureWidth() );
	m_obStaticRightCap.SetHeight( (float)m_obStaticRightCap.GetTextureHeight() );

	m_obRightCap.SetWidth( (float)m_obRightCap.GetTextureWidth() );
	m_obRightCap.SetHeight( (float)m_obRightCap.GetTextureHeight() );
	//

	// Set initial position for bar components.

	// Left Cap
	m_fLeftCapX = m_fGfxPositionX + m_fHalfCapWidth;
	CPoint obStaticLeftCapPos( m_fLeftCapX, fSliderBarPosY, 0.0f );
	m_obStaticLeftCap.SetPosition( obStaticLeftCapPos );
	m_obLeftCap.SetPosition( obStaticLeftCapPos );

	// Center.
	float fStaticCenterX = (m_fLeftCapX + m_fHalfCapWidth + fHalfCenterWidth);
	CPoint obStaticBarPos( fStaticCenterX, fSliderBarPosY, 0.0f );
	m_obStaticBar.SetPosition( obStaticBarPos );
	
	float fCenterX = (m_fLeftCapX + m_fHalfCapWidth + fHalfCenterTextureWidth);
	CPoint obBarPos( fCenterX, fSliderBarPosY, 0.0f );
	m_obBar.SetPosition( obBarPos );

	// Right Cap.
	float fRightCapX = (fStaticCenterX + fHalfCenterWidth + m_fHalfCapWidth);
	CPoint obStaticRightCapPos( fRightCapX, fSliderBarPosY, 0.0f );
	m_obStaticRightCap.SetPosition( obStaticRightCapPos );

	fRightCapX = (m_fLeftCapX + m_fHalfCapWidth + fHalfCenterTextureWidth );
	CPoint obRightCapPos( fRightCapX, fSliderBarPosY, 0.0f );
	m_obRightCap.SetPosition( obRightCapPos );

	//--------------------------------------------------------------------------------------------------------------------

	// Calc slider bar division size.
	m_fSliderBarDivisionWidth = (fCenterWidth / (float)m_iMaxValue);

	// Store x,y for slider positioning.
	m_fSliderXPos = obSliderPos.X();
	m_fSliderYPos = obSliderPos.Y();

	// Call activation change to set initial colour.
	ActivationChange();
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::UpdateSliderVisuals
//!
//! Position the slider using the current value.
//!
//--------------------------------------------------------------------------------------------------
void CGuiSkinMenuSlider::UpdateSliderSprites( const bool bSmooth )
{
	// Calc how much of center to show.
	m_fPixelsToShowTarget = ((float)m_iCurrentValue * m_fSliderBarDivisionWidth);

	float fPixelsToShow = m_fPixelsToShowTarget;

	if ( bSmooth )
	{
		float fIncrement = ((float)m_iMaxValue * SLIDERBAR_MOVEMENT_MULTIPLIER * CTimer::Get().GetGameTimeChange() );

		if ( m_fPixelsShowing < m_fPixelsToShowTarget )
		{
			m_fPixelsShowing += fIncrement;
		}
		else if ( m_fPixelsShowing > m_fPixelsToShowTarget )
		{
			m_fPixelsShowing -= fIncrement;
		}

		if ( fabsf(m_fPixelsToShowTarget - m_fPixelsShowing) < fIncrement )
		{
			m_fPixelsShowing = m_fPixelsToShowTarget;
		}

		fPixelsToShow = m_fPixelsShowing;
	}
	else
	{
		m_fPixelsShowing = fPixelsToShow;
	}

	// Update center sprite size.
	m_obBar.SetWidth( fPixelsToShow );

	// Update position of the bar.
	CPoint obBarPos( m_fLeftCapX + m_fHalfCapWidth + (fPixelsToShow * 0.5f), m_fSliderYPos, 0.0f );
	m_obBar.SetPosition( obBarPos );

	// Calc and set position for right cap
	float fRightCapX = (m_fLeftCapX + m_fHalfCapWidth + fPixelsToShow + m_fHalfCapWidth );
	CPoint obRightCapPos( fRightCapX, m_fSliderYPos, 0.0f );
	m_obRightCap.SetPosition( obRightCapPos );

	// Update pos for slider.
	//CPoint obPos( m_fSliderXPos + ((float)m_iCurrentValue * m_fSliderBarDivisionWidth), m_fSliderYPos, 0.0f );
	CPoint obPos( m_fSliderXPos + fPixelsToShow, m_fSliderYPos, 0.0f );
	m_obSlider.SetPosition( obPos );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiUnit::ProcessGfxBasePositionValue
*
*	DESCRIPTION		Deals with an x, y, z position from a string.  Set offset in the base transform
*					matrix.  Returns false if it fails.  Only implemented for screen space.
*
***************************************************************************************************/

bool CGuiSkinMenuSlider::ProcessGfxBasePositionValue( const char* pcValue )
{
	// We'll get three floats from the string
	float fX, fY, fZ;
	int iResult = sscanf( pcValue, "%f,%f,%f", &fX, &fY, &fZ ); 

	// Make sure we extracted three values
	ntAssert( iResult == 3 );
	UNUSED( iResult );

	// Create a point with the data
	CPoint obGfxBasePoint( fX, fY, fZ );

	m_fGfxPositionX = obGfxBasePoint.X() * CGuiManager::Get().BBWidth();

	return true;
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::ProcessStyleValue
//!
//! Deal with style attribute.
//!
//--------------------------------------------------------------------------------------------------
bool CGuiSkinMenuSlider::ProcessStyleValue( const char* pcValue )
{
	m_iStyleBits = 0;

	if	( strcmp( pcValue, "GS_VALUE" ) == 0 )
	{
		m_iStyleBits |= GS_VALUE;
	}
	else if	( strcmp( pcValue, "GS_GFX" ) == 0 )
	{
		m_iStyleBits |= GS_GFX;
	}
	else if	( strcmp( pcValue, "GS_VALUE_GFX" ) == 0 )
	{
		m_iStyleBits |= ( GS_VALUE | GS_GFX );
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::RenderSliderSprites
//!
//--------------------------------------------------------------------------------------------------
void CGuiSkinMenuSlider::RenderSliderSprites( void )
{
	m_obStaticLeftCap.Render();
	m_obStaticBar.Render();
	m_obStaticRightCap.Render();

	m_obLeftCap.Render();
	m_obBar.Render();
	m_obRightCap.Render();
	
	m_obSlider.Render();
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::FadeSliderSprites
//!
//--------------------------------------------------------------------------------------------------
void CGuiSkinMenuSlider::FadeSliderSprites( void )
{
	CGuiSkinFader::FadeSpriteObject( m_obSlider, ScreenFade() );

	CGuiSkinFader::FadeSpriteObject( m_obStaticBar, ScreenFade() );
	CGuiSkinFader::FadeSpriteObject( m_obStaticRightCap, ScreenFade() );
	CGuiSkinFader::FadeSpriteObject( m_obStaticLeftCap, ScreenFade() );

	CGuiSkinFader::FadeSpriteObject( m_obBar, ScreenFade() );
	CGuiSkinFader::FadeSpriteObject( m_obRightCap, ScreenFade() );
	CGuiSkinFader::FadeSpriteObject( m_obLeftCap, ScreenFade() );
}

//--------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuSlider::ActivationChange
//!
//!	Deal with slider bar colour change on activation/deactivation.
//!
//--------------------------------------------------------------------------------------------------
void CGuiSkinMenuSlider::ActivationChange( void )
{
	CVector obColour;

	m_obLeftCap.GetColourAll( obColour );

	if	( Active() )
	{
		obColour = CGuiManager::Get().GuiSettings()->DefaultTextSelectedColour();
	}
	else
	{
		obColour = CGuiManager::Get().GuiSettings()->DefaultTextColour();
	}

	TriggerActivationChangeCallback( Active() );

	// Update sprite colours.
	m_obLeftCap.SetColourAll( obColour );
	m_obBar.SetColourAll( obColour );
	m_obRightCap.SetColourAll( obColour );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuSlider::TriggerActivationChangeCallback
*
*	DESCRIPTION		Triggers lua callback
*
***************************************************************************************************/
void CGuiSkinMenuSlider::TriggerActivationChangeCallback( const bool bActive )
{
	if ( m_pcCallbackActivationChange )
	{
		NinjaLua::LuaFunction luaFunc = CGuiLua::GetLuaFunction( m_pcCallbackActivationChange );
		if	(!luaFunc.IsNil())
		{
			luaFunc( bActive );
		}
		else
		{
			ntPrintf( "CGuiSkinMenuSlider: Failed to find lua function \"%s\"\n", m_pcCallbackActivationChange );
		}
	}
}

void CGuiSkinMenuSlider::CalculateExtents()
{
	super::CalculateExtents();

	if (DisplayGraphic())
	{
		float fRightCapX = (m_fLeftCapX + m_fHalfCapWidth + CENTER_WIDTH + m_fHalfCapWidth*2.0f);
		//update our extents
		m_obExtents.fWidth = fRightCapX - m_obExtents.fX;
	}
	else if (DisplayValue())
	{
		float f = m_pobValueString ? m_pobValueString->RenderWidth() : 0.0f;
		m_obExtents.fWidth += m_obExtents.fWidth + VALUE_OFFSET + f;
	}
}

//-------------------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuSlider::SliderBarNearTargetPosition()
//!
//! Slider bar movement is smoothed which means the position displayed on screen is always
//! slightly behind the actual slider value.  This function checks to see how near the display is
//! to the actual value.
//!
//-------------------------------------------------------------------------------------------------
const bool CGuiSkinMenuSlider::SliderBarNearTargetPosition( void )
{
	// Calc how much difference there is between the target and actual position of the
	// slider bar.
	if	( fabsf(m_fPixelsShowing - m_fPixelsToShowTarget) < m_fSliderBarDivisionWidth )
	{
		return true;
	}

	return false;
}
