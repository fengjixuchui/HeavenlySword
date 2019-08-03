/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "e3guiskinimagestatic.h"
#include "guimanager.h"
#include "anim/hierarchy.h"


#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"

#define _XDRAM

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrImageFlags[] = 
{	
	{ CGuiE3SkinImageStatic::FADE_IN,			"FADE_IN"	},
	{ CGuiE3SkinImageStatic::FADE_OUT,			"FADE_OUT"	},
	{ CGuiE3SkinImageStatic::FULL_SCREEN,		"FULL_SCREEN"	},
	{ 0,										0			} 
};

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiE3SkinImageStatic(); }

// Register this class under it's XML tag
bool g_bIMAGE_STATIC = CGuiManager::Register( "IMAGE_STATIC", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::CGuiE3SkinImageStatic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiE3SkinImageStatic::CGuiE3SkinImageStatic( void )
{
	m_pcImageName = 0;
	m_fAlpha = 1.0f;
	m_iImageFlags = 0;
	m_pobFullScreenTexture = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::~CGuiE3SkinImageStatic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiE3SkinImageStatic::~CGuiE3SkinImageStatic( void )
{
	if (m_pcImageName)
		NT_DELETE_ARRAY( m_pcImageName );
#ifdef _XDRAM
	if ( m_pobFullScreenTexture )
		NT_DELETE_CHUNK( Mem::MC_GFX, m_pobFullScreenTexture );
#endif
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiE3SkinImageStatic::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitStatic::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "image" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcImageName );
		}

		if ( strcmp( pcTitle, "imagesize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fWidth, &m_fHeight );
		}

		if ( strcmp( pcTitle, "imagealpha" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fAlpha );
		}

		if ( strcmp( pcTitle, "imageflags" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrImageFlags[0], &m_iImageFlags );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiE3SkinImageStatic::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitStatic::ProcessEnd();

	// We should have all attributes here so set up the image sprite

	// Overide default render space.
	m_eRenderSpace = CGuiUnit::RENDER_SCREENSPACE;

	float fBBWidth = CGuiManager::Get().BBWidth();
	float fBBHeight = CGuiManager::Get().BBHeight();

	CPoint Pos( m_BasePosition.X()*fBBWidth, m_BasePosition.Y()*fBBHeight, 0.0f );
	CPoint Size( m_fWidth*fBBWidth, m_fHeight*fBBHeight, 0.0f );

	// Set the vertical offset of our image
	switch ( m_eVerticalJustification )
	{
	case JUSTIFY_TOP:		Pos.Y() -= (Size.Y()/2.0f);				break;
	case JUSTIFY_MIDDLE:											break;
	case JUSTIFY_BOTTOM:	Pos.Y() += (Size.Y()/2.0f);				break;
	default:								ntAssert( 0 );			break;														break;
	}

	// Set the horizontal offset of our image
	switch ( m_eHorizontalJustification )
	{
	case JUSTIFY_LEFT:		Pos.X() -= (Size.X()/2.0f);				break;
	case JUSTIFY_CENTRE:											break;
	case JUSTIFY_RIGHT:		Pos.X() += (Size.X()/2.0f);				break;
	default:								ntAssert( 0 );			break;														break;
	}

	m_obImage.SetPosition( Pos );
	m_obImage.SetWidth( Size.X() );
	m_obImage.SetHeight( Size.Y() );

	if ( m_iImageFlags & FADE_IN )
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );		
	else
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );

#ifdef _XDRAM
	if ( m_iImageFlags & FULL_SCREEN )
	{
		m_pobFullScreenTexture = NT_NEW_CHUNK( Mem::MC_GFX ) TextureXDRAM(m_pcImageName);
		m_obImage.SetTexture( m_pobFullScreenTexture->GetTexture() );
	}	
	else
#endif
	{
		m_obImage.SetTexture( m_pcImageName );		
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiE3SkinImageStatic::Render( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

#ifdef _XDRAM
	if ( m_iImageFlags & FULL_SCREEN )
	{
		m_obImage.SetTexture( m_pobFullScreenTexture->GetTexture() );
		m_obImage.Render();
	}
	else
#endif
	{
		m_obImage.Render();
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::SetStateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageStatic::SetStateEnter( void )
{
	CGuiUnit::SetStateEnter();
	if ( m_iImageFlags & FADE_IN )
	{
		m_obFadeTime.Set( ANIM_BLEND_TIME );
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
	}
	else
	{
		m_obFadeTime.Set( 0.0f );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::UpdateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageStatic::UpdateEnter( void )
{
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateIdle();
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );
		return;
	}

	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() ) * m_fAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::SetStateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageStatic::SetStateExit( void )
{
	CGuiUnit::SetStateExit();

	if ( m_iImageFlags & FADE_OUT )
	{
		m_obFadeTime.Set( ANIM_BLEND_TIME );
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );
	}
	else
	{
		m_obFadeTime.Set( 0.0f );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageStatic::UpdateExit( void )
{	
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateDead();
		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
		return;
	}

	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,CMaths::SmoothStep( m_obFadeTime.NormalisedTime() ) * m_fAlpha).GetNTColor() );
}
