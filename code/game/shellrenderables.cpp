//------------------------------------------------------------------------------------------
//!
//!	\file shellrenderables.cpp
//! helper classes for boot up display
//!
//------------------------------------------------------------------------------------------

#include "shellrenderables.h"
#include "effect/screensprite.h"
#include "gfx/renderer.h"

//----------------------------------------------------------------------------------------
//!
//!	ShellImage::ctor
//!
//----------------------------------------------------------------------------------------
ShellImage::ShellImage( const ShellImageDef& def ) :
	m_def( def ),
	m_eStatus( INVISIBLE ),
	m_fTransTime( 0.0f ),
	m_fAlpha( 0.0f )
{
	m_image = NT_NEW TextureXDRAM( m_def.m_pTexName );
	m_pSprite = NT_NEW ScreenSprite;
}

//----------------------------------------------------------------------------------------
//!
//!	ShellImage::dtor
//!
//----------------------------------------------------------------------------------------
ShellImage::~ShellImage()
{
	NT_DELETE( m_image );
	NT_DELETE( m_pSprite );
}

//----------------------------------------------------------------------------------------
//!
//!	ShellImage::fade control
//!
//----------------------------------------------------------------------------------------
void ShellImage::FadeUp()
{
	m_eStatus = FADING_UP;
	m_fTransTime = 0.0f;
}

void ShellImage::FadeDown()
{
	m_eStatus = FADING_DOWN;
	m_fTransTime = 0.0f;
}

//----------------------------------------------------------------------------------------
//!
//!	ShellImage::update fade
//!
//----------------------------------------------------------------------------------------
void ShellImage::Update( float fTimeChange )
{
	if ( m_eStatus == FADING_UP )
	{
		m_fTransTime += fTimeChange;
		if (m_fTransTime >= m_def.m_fadeUp)
			m_eStatus = VISIBLE;

		float fAlpha = CMaths::SmoothStep( m_fTransTime / m_def.m_fadeUp );
		m_fAlpha = ntstd::Clamp( fAlpha, 0.0f, 1.0f );
	}
	if ( m_eStatus == FADING_DOWN )
	{
		m_fTransTime += fTimeChange;
		if (m_fTransTime >= m_def.m_fadeDown)
			m_eStatus = INVISIBLE;

		float fAlpha = CMaths::SmoothStep( m_fTransTime / m_def.m_fadeUp );
		m_fAlpha = ntstd::Clamp( 1.0f - fAlpha, 0.0f, 1.0f );
	}
}

//----------------------------------------------------------------------------------------
//!
//!	ShellImage::Render
//!
//----------------------------------------------------------------------------------------
void ShellImage::Render()
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	float fWidth = Renderer::Get().m_targetCache.GetWidth();
	float fHeight = Renderer::Get().m_targetCache.GetHeight();

	m_pSprite->SetTexture( m_image->GetTexture() );
	m_pSprite->SetPosition( CPoint( fWidth*m_def.m_x, fHeight*m_def.m_y, 0.0f ) );
	m_pSprite->SetWidth( fWidth*m_def.m_width );
	m_pSprite->SetHeight( fHeight*m_def.m_height );

	CVector colour = m_def.m_rgba;
	colour.W() = m_def.m_rgba.W() * m_fAlpha;

	m_pSprite->SetColour( colour );
	m_pSprite->Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}




//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon::ctor
//!
//----------------------------------------------------------------------------------------
ShellAnimIcon::ShellAnimIcon( const ShellIconDef& def ) :
	m_def( def ),
	m_fAlpha( 1.0f )
{
	m_pSprite = NT_NEW ScreenSprite;
}

//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon::dtor
//!
//----------------------------------------------------------------------------------------
ShellAnimIcon::~ShellAnimIcon()
{
	NT_DELETE( m_pSprite );
	for (uint32_t i = 0; i < m_images.size(); ++i)
	{
		NT_DELETE( m_images[i] );
	}
}

//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon::add an image to the animation
//!
//----------------------------------------------------------------------------------------
void ShellAnimIcon::AddImage( const char* pImageName )
{
	uint32_t count = m_images.size();
	m_images.resize(count+1);
	m_images[count] = NT_NEW TextureXDRAM( pImageName );
}

//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon::RenderTime
//! render through the animation.
//!
//----------------------------------------------------------------------------------------
void ShellAnimIcon::RenderTime( float fTime, bool bBlend )
{
	float normalisedTime = (fTime / m_def.m_loopTime);
	normalisedTime -= floor(normalisedTime);
	RenderNormalised( normalisedTime, bBlend );
}

//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon::RenderNormalised
//! render through the animation.
//! We assume images are equally spaced, wrap, and do not duplicate
//!
//----------------------------------------------------------------------------------------
void ShellAnimIcon::RenderNormalised( float fTimeN, bool bBlend )
{
	uint32_t count = m_images.size();
	if ( count == 0 )
		return;

	fTimeN = ntstd::Clamp( fTimeN, 0.0f, 1.0f );
	float fractionalIndex = fTimeN * count;

	// get raw indices of textures
	uint32_t index1 = (uint32_t)floor(fractionalIndex);
	uint32_t index2 = index1+1;

	// get blend interval
	fractionalIndex -= _R(index1);

	// wrap to valid index
	if (index1 >= count)
		index1 -= count;

	if (index2 >= count)
		index2 -= count;

	ntError_p( (index1 >= 0) && (index1 < count), ("Should have index in valid bounds here") );
	ntError_p( (index2 >= 0) && (index2 < count), ("Should have index in valid bounds here") );
	
	// draw
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	float fWidth = Renderer::Get().m_targetCache.GetWidth();
	float fHeight = Renderer::Get().m_targetCache.GetHeight();

	m_pSprite->SetPosition( CPoint( fWidth*m_def.m_x, fHeight*m_def.m_y, 0.0f ) );
	m_pSprite->SetWidth( fWidth*m_def.m_width );
	m_pSprite->SetHeight( fHeight*m_def.m_height );

	CVector colour = m_def.m_rgba;

	// first texture
	if (bBlend)
		colour.W() = m_def.m_rgba.W() * m_fAlpha * (1.0f - fractionalIndex);
	else
		colour.W() = m_def.m_rgba.W() * m_fAlpha;

	m_pSprite->SetTexture( m_images[index1]->GetTexture() );
	m_pSprite->SetColour( colour );
	m_pSprite->Render();

	// second texture
	if (bBlend)
	{
		colour.W() = m_def.m_rgba.W() * m_fAlpha * fractionalIndex;

		m_pSprite->SetTexture( m_images[index2]->GetTexture() );
		m_pSprite->SetColour( colour );
		m_pSprite->Render();
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}
