//------------------------------------------------------------------------------------------
//!
//!	\file worldsprite.cpp
//! A world space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#include "effect/worldsprite.h"
#include "gfx/renderer.h"
#include "gfx/texturemanager.h"
#include "gfx/shader.h"
#include "gfx/rendercontext.h"
#include "anim/transform.h"

//------------------------------------------------------------------------------------------
//!
//!	WorldSprite::ctor
//!
//------------------------------------------------------------------------------------------
WorldSprite::WorldSprite( int iNumberOfSprites )
{
	ntAssert_p( iNumberOfSprites > 0, ("Invalid size for sprite list") );

	// allocate storage for our sprites
	m_spriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3,	VE_POSITON,		"IN.position" );
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
	m_pVertexShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_VERTEX, "fxshaders/worldsprite_vs.hlsl", "main", "vs_2_0" );
	m_pPixelShader = DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_PIXEL, "fxshaders/screensprite_ps.hlsl", "main", "ps_2_0" );
#elif defined(PLATFORM_PS3)
	m_pVertexShader = DebugShaderCache::Get().LoadShader( "worldsprite_vp.sho" );
	m_pPixelShader = DebugShaderCache::Get().LoadShader( "screensprite_fp.sho" );
#endif

	m_bBilinear = true;
}

//------------------------------------------------------------------------------------------
//!
//!	WorldSprite::SetTexture
//!
//------------------------------------------------------------------------------------------
void WorldSprite::SetTexture( const char* pName )
{
	SetTexture( TextureManager::Get().LoadTexture_Neutral( pName ) );
}

void WorldSprite::SetTexture( Texture::Ptr pTex )
{
	m_texture = pTex;
	m_bBilinear = GFXFormat::Is32F( m_texture->GetFormat() ) ? false : true;	
}

void WorldSprite::SetUV( float fTop, float fLeft, float fBottom, float fRight, int iItem )
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
//!	WorldSprite::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void WorldSprite::SetSize( float fSize, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);	pSprite++;
	pSprite->SetWH(fSize,fSize);
}

void WorldSprite::SetWidth( float fWidth, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);	pSprite++;
	pSprite->SetW(fWidth);
}

void WorldSprite::SetHeight( float fHeight, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);	pSprite++;
	pSprite->SetH(fHeight);
}

void WorldSprite::SetSizeAll( float fSize )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
		pSprite->SetWH(fSize,fSize);
}

//------------------------------------------------------------------------------------------
//!
//!	WorldSprite::SetColour
//! methods to set the colour(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void WorldSprite::SetColour( uint32_t iCol, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);	pSprite++;
	pSprite->SetCol(iCol);
}

void WorldSprite::SetColourAll( uint32_t iCol )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
		pSprite->SetCol(iCol);
}

void WorldSprite::SetColour( const CVector& col, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);	pSprite++;
	pSprite->SetCol(col);
}

void WorldSprite::SetColourAll( const CVector& col )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex(0);
	for (u_int i = 0; i < m_spriteData.GetGPUData().GetMaxVertices(); i++, pSprite++)
		pSprite->SetCol(col);
}
//------------------------------------------------------------------------------------------
//!
//!	WorldSprite::SetPosition
//! we could use width and height here to get the actual corners of the sprite. BUT, doing
//! it in the vertex shader is effectivly free as the shader is piss simple, and means we
//! dont have to enforce a usage model on the order of data setting.
//!
//------------------------------------------------------------------------------------------
void WorldSprite::SetPosition( const CPoint& pos, int iItem )
{
	Vertex* pSprite = (Vertex*)m_spriteData.GetGPUData().GetVertex( iItem*4 );
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());	pSprite++;
	pSprite->SetPos(pos.X(),pos.Y(),pos.Z());
}

//------------------------------------------------------------------------------------------
//!
//!	WorldSprite::Render
//! Draw the sprite in world space.
//!
//------------------------------------------------------------------------------------------
void WorldSprite::Render()
{
	// NB we dont require a camera here. this is a legacy from a very old interface descision
	// for a dead effect system. Requires a minor refactor on the PC side to loose this.

	Renderer::Get().SetVertexShader( m_pVertexShader );
	Renderer::Get().SetPixelShader( m_pPixelShader );

	// ATTN! assumes a valid render context when we're called, not unreasonable!
	// NOTE, we flip the camera axis so we can make our screen / world sprite vertex data match.
	CDirection cameraUnitAxisX = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetXAxis();
	CDirection cameraUnitAxisY = -RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix().GetYAxis();
	const CMatrix& worldViewProj = RenderingContext::Get()->m_worldToScreen;

#ifdef PLATFORM_PC

	Renderer::Get().SetVertexShaderConstant( 1, cameraUnitAxisX );
	Renderer::Get().SetVertexShaderConstant( 2, cameraUnitAxisY );
	Renderer::Get().SetVertexShaderConstant( 3, worldViewProj );

#elif defined(PLATFORM_PS3)

	m_pVertexShader->SetVSConstantByName( "cameraUnitAxisX", cameraUnitAxisX );
	m_pVertexShader->SetVSConstantByName( "cameraUnitAxisY", cameraUnitAxisY );
	m_pVertexShader->SetVSConstantByName( "worldViewProj", worldViewProj );

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
