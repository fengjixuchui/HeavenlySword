/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that sontains video playback functionality
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinvideo.h"
#include "guimanager.h"
#include "anim/hierarchy.h"


#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"

// Convertion for bitwise enum values to strings - null terminated for converter
/*const CStringUtil::STRING_FLAG astrImageFlags[] = 
{	
	{ CGuiE3SkinImageStatic::FADE_IN,			"FADE_IN"	},
	{ CGuiE3SkinImageStatic::FADE_OUT,			"FADE_OUT"	},
	{ CGuiE3SkinImageStatic::FULL_SCREEN,		"FULL_SCREEN"	},
	{ 0,										0			} 
};*/

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinVideo(); }

// Register this class under it's XML tag
bool g_bVIDEO = CGuiManager::Register( "VIDEO", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::CGuiSkinVideo
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinVideo::CGuiSkinVideo( void )
{
	m_pcFileName = 0;
	m_fAlpha = 1.0f;
	//m_iImageFlags = 0;
	m_pobFullScreenTexture = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::~CGuiSkinVideo
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinVideo::~CGuiSkinVideo( void )
{
	if (m_pcFileName)
		NT_DELETE_ARRAY( m_pcFileName );

	if ( m_pobFullScreenTexture )
		NT_DELETE ( m_pobFullScreenTexture );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinVideo::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitStatic::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "file" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}

		if ( strcmp( pcTitle, "imagesize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fWidth, &m_fHeight );
		}

		if ( strcmp( pcTitle, "imagealpha" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fAlpha );
		}

		/*if ( strcmp( pcTitle, "imageflags" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrImageFlags[0], &m_iImageFlags );
		}*/

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinVideo::ProcessEnd( void )
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

	m_obVideo.SetPosition( Pos );
	m_obVideo.SetWidth( Size.X() );
	m_obVideo.SetHeight( Size.Y() );

	/*if ( m_iImageFlags & FADE_IN )
		m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );		
	else*/
		m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );

	m_pobFullScreenTexture = NT_NEW TextureXDRAM(m_pcFileName);
	m_obVideo.SetTexture( m_pobFullScreenTexture->GetTexture() );
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSkinVideo::Render( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	m_obVideo.SetTexture( m_pobFullScreenTexture->GetTexture() );
	m_obVideo.Render();
	
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::SetStateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::SetStateEnter( void )
{
	CGuiUnit::SetStateEnter();
	//if ( m_iImageFlags & FADE_IN )
	{
		m_obFadeTime.Set( ANIM_BLEND_TIME );
		//m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
	}
	/*else
	{
		m_obFadeTime.Set( 0.0f );
	}*/
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::UpdateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::UpdateEnter( void )
{
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateIdle();
		m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );
		return;
	}

	m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() ) * m_fAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::SetStateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::SetStateExit( void )
{
	CGuiUnit::SetStateExit();

	//if ( m_iImageFlags & FADE_OUT )
	{
		m_obFadeTime.Set( ANIM_BLEND_TIME );
		//m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,m_fAlpha).GetNTColor() );
	}
	/*else
	{
		m_obFadeTime.Set( 0.0f );
	}*/
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::UpdateExit( void )
{	
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateDead();
		m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
		return;
	}

	m_obVideo.SetColour( CVector(1.0f,1.0f,1.0f,CMaths::SmoothStep( m_obFadeTime.NormalisedTime() ) * m_fAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::SetStateIdle
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::SetStateIdle( void )
{
	CGuiUnit::SetStateIdle();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinVideo::UpdateIdle
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinVideo::UpdateIdle( void )
{	
	CGuiUnit::UpdateIdle();
}
