//------------------------------------------------------------------------------------------
//!
//!	\file screensprite_ps3.cpp
//! A screen space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#include "effect/screensprite_ps3.h"
#include "gfx/renderer.h"
#include "gfx/texturemanager.h"
#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::ctor
//!
//------------------------------------------------------------------------------------------
ScreenSprite::ScreenSprite( int iNumberOfSprites )
{
	ntAssert_p( iNumberOfSprites > 0, ("Invalid size for sprite list") );

	// allocate storage for our sprites
	m_spriteData.PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_POSITON,		"IN.position" );
	m_spriteData.PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_WIDTHHEIGHT,	"IN.widthheight" );
	m_spriteData.PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_TEXCOORD,	"IN.texcoord" );
	m_spriteData.PushVertexElement( VD_STREAM_TYPE_UBYTE4N, VE_COLOUR,		"IN.colour" );

	m_spriteData.BuildMe( iNumberOfSprites * 4 );

	// initialise them (this really just sets tex coords and colour to white)
	Vertex	spriteTemplate[4];
	spriteTemplate[0].SetTex(0.0f, 0.0f);
	spriteTemplate[1].SetTex(1.0f, 0.0f);
	spriteTemplate[2].SetTex(1.0f, 1.0f);
	spriteTemplate[3].SetTex(0.0f, 1.0f);

	ntAssert_p( m_spriteData.GetVertexSize() == sizeof( Vertex ), ("something has gone horribly wrong") );

	for (int i = 0; i < iNumberOfSprites; i++)
	{
		void* pSprite = m_spriteData.GetVertex( i*4 );
		memcpy( pSprite, spriteTemplate, sizeof( Vertex ) * 4 );
	}

	// these shaders should really be owned by a global singleton somewhere...
	m_pVertexShader = DebugShaderCache::Get().LoadShader( "debugshaders/screensprite_vp.sho" );
	m_pPixelShader = DebugShaderCache::Get().LoadShader( "debugshaders/screensprite_fp.sho" );

	m_bBilinear = true;
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetTexture
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetTexture( const char* pName )
{
	SetTexture( TextureManager::Get().LoadTexture( pName ) );
}

void ScreenSprite::SetTexture( Texture::Ptr pTex )
{
	m_texture = pTex;
	m_bBilinear = RendererPlatform::Is32bitFloatFormat( m_texture->m_Platform.GetTexture()->GetFormat() ) ?
		false : true;	
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetSize( float fSize, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex( iItem*4 );
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);
}

void ScreenSprite::SetWidth( float fWidth, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex( iItem*4 );
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);
}

void ScreenSprite::SetHeight( float fHeight, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex( iItem*4 );
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);
}

void ScreenSprite::SetSizeAll( float fSize )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetMaxVertices(); i++, pSprite++)
		pSprite->SetWH(fSize,fSize);
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetColour
//! methods to set the colour(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetColour( uint32_t iCol, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex( iItem*4 );
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);
}

void ScreenSprite::SetColourAll( uint32_t iCol )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetMaxVertices(); i++, pSprite++)
		pSprite->SetCol(iCol);
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetPosition
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetPosition( const CPoint& pos, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetVertex( iItem*4 );
	pSprite->SetPos(pos.X(),pos.Y());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y());
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::Render
//! Draw the sprite in screen / rendertarget space.
//! the Sprites position and dimensions are assumed to be in render target space (i.e. a 
//! full screen sprite on a 720p target has pos = {640,360), dim = (1280,720)
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::Render( const CCamera& )
{
	// NB we dont require a camera here. this is a legacy from a very old interface descision
	// for a dead effect system. Requires a minor refactor on the PC side to loose this.

	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	// ATTN! assumes valid rendertarget is set when render is called. Not unreasonable!
	CVector rcpScreenDim(
			2.0f / _R( Renderer::Get().GetRenderTarget(0)->GetWidth() ),
			-2.0f / _R( Renderer::Get().GetRenderTarget(0)->GetHeight() ), 0.0f, 0.0f );

	m_pVertexShader->SetVSConstant( m_pVertexShader->GetConstantIndex("rcpScreenDim"), rcpScreenDim );

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.
	Renderer::Get().SetTexture( 0, m_texture );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );

	TEXTUREFILTER_TYPE filter = m_bBilinear ? TEXTUREFILTER_BILINEAR : TEXTUREFILTER_POINT;
	Renderer::Get().SetSamplerFilterMode( 0, filter );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.SubmitToGPU();
	Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, m_spriteData.GetMaxVertices() );
}
