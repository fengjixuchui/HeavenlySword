//------------------------------------------------------------------------------------------
//!
//!	\file screensprite.cpp
//! A screen space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#include "effect/screensprite.h"
#include "gfx/renderer.h"
#include "gfx/texturemanager.h"
#include "gfx/shader.h"
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
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_POSITON,		"IN.position" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_WIDTHHEIGHT,	"IN.widthheight" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_TEXCOORD,	"IN.texcoord" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT4, VE_COLOUR,		"IN.colour" );
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, VE_CORNER,		"IN.corner" );

	m_spriteData.Initialise( iNumberOfSprites, false );

	// initialise them (this really just sets tex coords and colour to white)
	Vertex	spriteTemplate[4];

#ifdef PLATFORM_PC
	spriteTemplate[0].SetTex(0.0f, 0.0f);
	spriteTemplate[1].SetTex(1.0f, 0.0f);
	spriteTemplate[2].SetTex(0.0f, 1.0f);
	spriteTemplate[3].SetTex(1.0f, 1.0f);
#else
	spriteTemplate[0].SetTex(0.0f, 0.0f);
	spriteTemplate[1].SetTex(1.0f, 0.0f);
	spriteTemplate[2].SetTex(1.0f, 1.0f);
	spriteTemplate[3].SetTex(0.0f, 1.0f);
#endif

	// NOTE! instead of a corner per sprite, could use a stream of 4 with the frequency
	// divider gubbins, but right now i just cant be bothered.
	// Also, before you go thinking, 'why is he not using point sprites?' its because
	// we want arbitary widths and heights for sprites.

#ifdef PLATFORM_PC
	spriteTemplate[0].SetCorner(-0.5f, -0.5f);
	spriteTemplate[1].SetCorner( 0.5f, -0.5f);
	spriteTemplate[2].SetCorner(-0.5f,  0.5f);
	spriteTemplate[3].SetCorner( 0.5f,  0.5f);
#else
	spriteTemplate[0].SetCorner(-0.5f, -0.5f);
	spriteTemplate[1].SetCorner( 0.5f, -0.5f);
	spriteTemplate[2].SetCorner( 0.5f,  0.5f);
	spriteTemplate[3].SetCorner(-0.5f,  0.5f);
#endif

	ntAssert_p( m_spriteData.GetGPUData().GetVertexSize() == sizeof( Vertex ), ("something has gone horribly wrong") );

	for (int i = 0; i < iNumberOfSprites; i++)
	{
		void* pSprite = m_spriteData.GetGPUData().GetVertex( i*4 );
		NT_MEMCPY( pSprite, spriteTemplate, sizeof( Vertex ) * 4 );
	}

#ifdef PLATFORM_PC	
	m_pVertexShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_VERTEX, "fxshaders/screensprite_vs.hlsl", "main", "vs_2_0" );
	m_pPixelShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_PIXEL, "fxshaders/screensprite_ps.hlsl", "main", "ps_2_0" );
#elif defined(PLATFORM_PS3)
	m_pVertexShader = DebugShaderCache::Get().LoadShader( "screensprite_vp.sho" );
	m_pPixelShader = DebugShaderCache::Get().LoadShader( "screensprite_fp.sho" );
#endif

	m_bBilinear = true;
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetTexture
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetTexture( const CKeyString& pName )
{
	SetTexture( TextureManager::Get().LoadTexture_Neutral( ntStr::GetString(pName) ) );
}

void ScreenSprite::SetTexture( Texture::Ptr pTex )
{
	m_texture = pTex;
	m_bBilinear = GFXFormat::Is32F( m_texture->GetFormat() ) ? false : true;	
}

void ScreenSprite::SetUV( float fTop, float fLeft, float fBottom, float fRight, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
#ifdef PLATFORM_PC
	pSprite->SetTex(fLeft,fTop);	pSprite++;
	pSprite->SetTex(fRight,fTop);	pSprite++;
	pSprite->SetTex(fLeft,fBottom);	pSprite++;
	pSprite->SetTex(fRight,fBottom);
#else
	pSprite->SetTex(fLeft,fTop);		pSprite++;
	pSprite->SetTex(fRight,fTop);		pSprite++;
	pSprite->SetTex(fRight,fBottom);	pSprite++;
	pSprite->SetTex(fLeft,fBottom);
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetSize( float fSize, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);
}

void ScreenSprite::SetWidth( float fWidth, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);
}

void ScreenSprite::SetHeight( float fHeight, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);
}

void ScreenSprite::SetSizeAll( float fSize )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
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
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);
}

void ScreenSprite::SetColourAll( uint32_t iCol )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
		pSprite->SetCol(iCol);
}

void ScreenSprite::SetColour( const CVector& col, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);
}

void ScreenSprite::SetColourAll( const CVector& col )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
		pSprite->SetCol(col);
}
//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::SetPosition
//! we could use width and height here to get the actual corners of the sprite. BUT, doing
//! it in the vertex shader is effectivly free as the shader is piss simple, and means we
//! dont have to enforce a usage model on the order of data setting.
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::SetPosition( const CPoint& pos, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
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
void ScreenSprite::Render()
{
	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	// ATTN! assumes valid rendertarget is set when render is called. Not unreasonable!
	CVector rcpScreenDim(
		2.0f / _R( Renderer::Get().m_targetCache.GetWidth() ),
		-2.0f / _R( Renderer::Get().m_targetCache.GetHeight() ), 0.0f, 0.0f );

#ifdef PLATFORM_PC
	Renderer::Get().SetVertexShaderConstant( 1, rcpScreenDim );
#elif defined(PLATFORM_PS3)
	m_pVertexShader->SetVSConstantByName( "rcpScreenDim", rcpScreenDim );
#endif

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.
	Renderer::Get().SetTexture( 0, m_texture );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );

	TEXTUREFILTER_TYPE filter = m_bBilinear ? TEXTUREFILTER_BILINEAR : TEXTUREFILTER_POINT;
	Renderer::Get().SetSamplerFilterMode( 0, filter );

	// write our dynamic buffer to the command buffer and render
	m_spriteData.Render();
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::GetColourAll
//! 
//! Returns uint32_t colour values stored in the first vertex.
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::GetColourAll( uint32_t& iCol )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	pSprite->GetCol( iCol );
}

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite::GetColourAll
//! 
//! Returns CVector colour values stored in the first vertex.
//!
//------------------------------------------------------------------------------------------
void ScreenSprite::GetColourAll( CVector& obColour )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	pSprite->GetCol( obColour );
}
