/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES to use the rot sprite the 'rot' tag must be the first one read.			
*
***************************************************************************************************/

// Includes
#include "guiskinmenuimage.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"
#include "gui/guiscreen.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"
#include "core/timer.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuImage(); }

// Register this class under it's XML tag
bool g_bMENUIMAGE = CGuiManager::Register( "MENUIMAGE", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuImage::CGuiSkinMenuImage
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuImage::CGuiSkinMenuImage( void )
{
	m_fWidth = -1.0f;
	m_fHeight = -1.0f;
	m_bConstructing = true;
	m_eImageSizeType = SIZE_RELATIVE;
	m_fRadiansSecond = 0.0f;
	m_fCurrentAngle = 0.0f;
	m_bTextureSet = false;
	m_bUseRotSprite = false;

}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuImage::~CGuiSkinMenuImage
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuImage::~CGuiSkinMenuImage( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuImage::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuImage::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "rot" ) == 0 )
		{
			m_bUseRotSprite = true;
			return true;
		}
		else if ( strcmp( pcTitle, "image" ) == 0 )
		{
			return SetImage(pcValue);
		}
		else if ( strcmp( pcTitle, "imagesize" ) == 0 )
		{
			return SetImageSize( pcValue );
		}
		else if ( strcmp( pcTitle, "imagesizetype" ) == 0 )
		{
			return SetImageSizeType( pcValue );
		}
		else if ( strcmp( pcTitle, "rotinsec" ) == 0 )
		{
			GuiUtil::SetFloat( pcValue, &m_fRadiansSecond );
			m_fRadiansSecond = 6.28f / m_fRadiansSecond;
			return true;
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuImage::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuImage::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	float fade = GetParentScreen()->ScreenFade();
	if( m_bUseRotSprite )
	{
		m_obRotImage.SetColour( CVector(1.0f,1.0f,1.0f,fade).GetNTColor() );
	}
	else
	{
	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,fade).GetNTColor() );
	}

	m_bConstructing = false;

	UpdateImageSize();

	return true;
}

bool CGuiSkinMenuImage::SetImage(const char* pcValue)
{
	//skip invalid image names
	if (!pcValue || strcmp("", pcValue) == 0)
		return true;

	//assign to image
	if( m_bUseRotSprite )
	{
	   m_obRotImage.SetTexture(pcValue);
	}
	else
	{
	m_obImage.SetTexture(pcValue);
	}
	m_bTextureSet = true;

	return true;
}

bool CGuiSkinMenuImage::SetImageSizeType(const char* pcValue)
{
	if ( strcmp( pcValue, "RELATIVE" ) == 0 )
	{
		m_eImageSizeType = SIZE_RELATIVE;
		return true;
	}
	else if ( strcmp( pcValue, "PIXEL" ) == 0 )
	{
		m_eImageSizeType = SIZE_PIXEL;
		return true;
	}

	UpdateImageSize();

	return false;
}

void CGuiSkinMenuImage::UpdateImageSize()
{
	if (m_bConstructing)
		return;

	float fBBWidth = CGuiManager::Get().BBWidth();
	float fBBHeight = CGuiManager::Get().BBHeight();

	if (m_fWidth == -1.0f)
	{
		uint32_t w = 0;
		if( m_bUseRotSprite )
		{
			w = m_obRotImage.GetTextureWidth();
		}
		else
		{
			w = m_obImage.GetTextureWidth();
		}
		if (w != 0)
		{
			if (m_eImageSizeType == SIZE_RELATIVE)
				m_fWidth = w/fBBWidth;
			else
				m_fWidth = (float)w;
		}
	}

	if (m_fHeight == -1.0f)
	{
		uint32_t h;
		if( m_bUseRotSprite )
		{
			h = m_obRotImage.GetTextureHeight();
		}
		else
		{
			h = m_obImage.GetTextureHeight();
		}
		if (h != 0)
		{
			if (m_eImageSizeType == SIZE_RELATIVE)
				m_fHeight = h/fBBHeight;
			else
				m_fHeight = (float)h;
		}
	}

	if (m_eImageSizeType == SIZE_RELATIVE)
	{
		m_fWidth *= fBBWidth;
		m_fHeight *= fBBHeight;
	}

	//Update position from this new size aswell

	CPoint Pos( m_BasePosition );

	// Set the vertical offset of our image
	switch ( m_eVerticalJustification )
	{
		case JUSTIFY_TOP:		Pos.Y() -= (m_fHeight/2.0f);		break;
		case JUSTIFY_MIDDLE:										break;
		case JUSTIFY_BOTTOM:	Pos.Y() += (m_fHeight/2.0f);		break;
		default:				ntAssert( 0 );						break;
	}

	// Set the horizontal offset of our image
	switch ( m_eHorizontalJustification )
	{
		case JUSTIFY_LEFT:		Pos.X() -= (m_fWidth/2.0f);			break;
		case JUSTIFY_CENTRE:										break;
		case JUSTIFY_RIGHT:		Pos.X() += (m_fWidth/2.0f);			break;
		default:				ntAssert( 0 );						break;
	}

	if( m_bUseRotSprite )
	{
		m_obRotImage.SetPosition( Pos );

		m_obRotImage.SetWidth( m_fWidth );
		m_obRotImage.SetHeight( m_fHeight );
	}
	else
	{
	m_obImage.SetPosition( Pos );

	m_obImage.SetWidth( m_fWidth );
	m_obImage.SetHeight( m_fHeight );
}
}

bool CGuiSkinMenuImage::SetImageSize(const char* pcValue)
{
	GuiUtil::SetFloats(pcValue, &m_fWidth, &m_fHeight);
	UpdateImageSize();

	return true;
}

bool CGuiSkinMenuImage::Render()
{
	super::Render();

	if (m_bTextureSet)
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		if	( IsFading() )
		{
			CGuiSkinFader::FadeSpriteObject( m_obImage, ScreenFade() );
		}

		if( m_bUseRotSprite )
		{
			m_obRotImage.Render();
		}
		else
		{
		m_obImage.Render();
		}
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

		if( m_bUseRotSprite == true )
		{
			m_fCurrentAngle += m_fRadiansSecond * CTimer::Get().GetSystemTimeChange();
 			if( m_fCurrentAngle < 0.0f )
				m_fCurrentAngle += 6.28f;
			else if( m_fCurrentAngle > 6.28f )
				m_fCurrentAngle -= 6.28f;
			m_obRotImage.SetAngle( m_fCurrentAngle );
		}

	}
	return true;
}
