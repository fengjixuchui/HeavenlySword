//------------------------------------------------------------------------------------------
//!
//!	\file screensprite.cpp
//! A screen space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#include "effect/moviesprite.h"
#include "gfx/renderer.h"
#include "gfx/texturemanager.h"
#include "gfx/shader.h"
#include "gfx/pictureinpicture.h"
#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::ctor
//!
//------------------------------------------------------------------------------------------
MovieSprite::MovieSprite( int32_t pip_debug_id, Texture::Ptr Y, Texture::Ptr cR, Texture::Ptr cB, Texture::Ptr Alpha/* = NULL*/ )
:	m_PIPDebugID( pip_debug_id )
{
	// allocate storage for our sprites
	m_SpriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_POSITON,		"IN.position" );
	m_SpriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_WIDTHHEIGHT,	"IN.widthheight" );
	m_SpriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2,	VE_TEXCOORD,	"IN.texcoord" );
	m_SpriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT4, VE_COLOUR,		"IN.colour" );
	m_SpriteData.GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, VE_CORNER,		"IN.corner" );

	m_SpriteData.Initialise( 1, false );

	ntAssert_p( m_SpriteData.GetGPUData().GetVertexSize() == sizeof( Vertex ), ("something has gone horribly wrong") );

	Vertex *pSprite = static_cast< Vertex * >( m_SpriteData.GetGPUData().GetVertex( 0 ) );

	// initialise them (this really just sets tex coords and colour to white)
#ifdef PLATFORM_PC
	pSprite[ 0 ].SetTex( 0.0f, 0.0f );
	pSprite[ 1 ].SetTex( 1.0f, 0.0f );
	pSprite[ 2 ].SetTex( 0.0f, 1.0f );
	pSprite[ 3 ].SetTex( 1.0f, 1.0f );

	pSprite[ 0 ].SetCorner(-0.5f, -0.5f);
	pSprite[ 1 ].SetCorner( 0.5f, -0.5f);
	pSprite[ 2 ].SetCorner(-0.5f,  0.5f);
	pSprite[ 3 ].SetCorner( 0.5f,  0.5f);
#else
	pSprite[ 0 ].SetTex( 0.0f, 0.0f );
	pSprite[ 1 ].SetTex( 1.0f, 0.0f );
	pSprite[ 2 ].SetTex( 1.0f, 1.0f );
	pSprite[ 3 ].SetTex( 0.0f, 1.0f );

	pSprite[ 0 ].SetCorner(-0.5f, -0.5f);
	pSprite[ 1 ].SetCorner( 0.5f, -0.5f);
	pSprite[ 2 ].SetCorner( 0.5f,  0.5f);
	pSprite[ 3 ].SetCorner(-0.5f,  0.5f);
#endif

	m_Texture[ MovieSprite::Y ] = Y;
	m_Texture[ MovieSprite::cR ] = cR;
	m_Texture[ MovieSprite::cB ] = cB;
	m_Texture[ MovieSprite::Alpha ] = Alpha;

#	ifdef PLATFORM_PC	
		m_VertexShader	= DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_VERTEX, "fxshaders/moviesprite_vs.hlsl", "main", "vs_2_0" );
		m_PixelShader	= DebugShaderCache::Get().LoadShaderFILE( SHADERTYPE_PIXEL, "fxshaders/moviesprite_ps.hlsl", "main", "ps_2_0" );
#	elif defined(PLATFORM_PS3)
		m_VertexShader	= DebugShaderCache::Get().LoadShader( "moviesprite_vp.sho" );
		m_PixelShader	= DebugShaderCache::Get().LoadShader( "moviesprite_fp.sho" );
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::SetTexture
//!
//------------------------------------------------------------------------------------------
void MovieSprite::SetTexture( Texture::Ptr texture, TextureType type )
{
	ntError( type >= 0 && type < NumTextureTypes );
	m_Texture[ type ] = texture;
}

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::SetSize
//! methods to set the size(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void MovieSprite::SetWidth( float width )
{
	Vertex *pSprite = (Vertex *)m_SpriteData.GetGPUData().GetVertex( 0 );
	pSprite->SetW( width );	pSprite++;
	pSprite->SetW( width );	pSprite++;
	pSprite->SetW( width );	pSprite++;
	pSprite->SetW( width );
}

void MovieSprite::SetHeight( float height )
{
	Vertex *pSprite = (Vertex *)m_SpriteData.GetGPUData().GetVertex( 0 );
	pSprite->SetH( height );	pSprite++;
	pSprite->SetH( height );	pSprite++;
	pSprite->SetH( height );	pSprite++;
	pSprite->SetH( height );
}

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::SetColour
//! methods to set the colour(s) of a sprite / sprites
//!
//------------------------------------------------------------------------------------------
void MovieSprite::SetColour( const CVector &col )
{
	Vertex *pSprite = (Vertex *)m_SpriteData.GetGPUData().GetVertex( 0 );
	pSprite->SetCol( col );	pSprite++;
	pSprite->SetCol( col );	pSprite++;
	pSprite->SetCol( col );	pSprite++;
	pSprite->SetCol( col );
}

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::SetPosition
//! we could use width and height here to get the actual corners of the sprite. BUT, doing
//! it in the vertex shader is effectivly free as the shader is piss simple, and means we
//! dont have to enforce a usage model on the order of data setting.
//!
//------------------------------------------------------------------------------------------
void MovieSprite::SetPosition( const CPoint &pos )
{
	Vertex *pSprite = (Vertex *)m_SpriteData.GetGPUData().GetVertex( 0 );
	pSprite->SetPos( pos.X(), pos.Y() );	pSprite++;
	pSprite->SetPos( pos.X(), pos.Y() );	pSprite++;
	pSprite->SetPos( pos.X(), pos.Y() );	pSprite++;
	pSprite->SetPos( pos.X(), pos.Y() );
}

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite::Render
//! Draw the sprite in screen / rendertarget space.
//! the Sprites position and dimensions are assumed to be in render target space (i.e. a 
//! full screen sprite on a 720p target has pos = {640,360), dim = (1280,720)
//!
//------------------------------------------------------------------------------------------
void MovieSprite::Render( bool bForced )
{
	if ( false == bForced )
	{
		if ( Renderer::Get().m_pPIPManager->GetCurrentView().GetDebugID() != m_PIPDebugID )
		{
			return;
		}
	}

	Renderer::Get().SetVertexShader( m_VertexShader );
	Renderer::Get().SetPixelShader( m_PixelShader );

	// ATTN! assumes valid rendertarget is set when render is called. Not unreasonable!
	CVector rcpScreenDim(
		2.0f / _R( Renderer::Get().m_targetCache.GetWidth() ),
		-2.0f / _R( Renderer::Get().m_targetCache.GetHeight() ), 0.0f, 0.0f );

#ifdef PLATFORM_PC
	Renderer::Get().SetVertexShaderConstant( 1, rcpScreenDim );
#elif defined(PLATFORM_PS3)
	m_VertexShader->SetVSConstantByName( "rcpScreenDim", rcpScreenDim );
#endif

	// we assume that whoever calls our render has alread set / disabled the requried render states
	// so we just need to set texture states.
	Renderer::Get().SetTexture( 0, m_Texture[ MovieSprite::Y ] );
	Renderer::Get().SetTexture( 1, m_Texture[ MovieSprite::cR ] );
	Renderer::Get().SetTexture( 2, m_Texture[ MovieSprite::cB ] );
	Renderer::Get().SetTexture( 3, m_Texture[ MovieSprite::Alpha ] );

	m_Texture[ MovieSprite::Y ]->m_Platform.GetTexture()->SetGammaCorrect( 0 );
	m_Texture[ MovieSprite::cR ]->m_Platform.GetTexture()->SetGammaCorrect( 0 );
	m_Texture[ MovieSprite::cB ]->m_Platform.GetTexture()->SetGammaCorrect( 0 );

	if ( m_Texture[ MovieSprite::Alpha ] != NULL )
	{
		m_Texture[ MovieSprite::Alpha ]->m_Platform.GetTexture()->SetGammaCorrect( 0 );
	}

	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerAddressMode( 3, TEXTUREADDRESS_CLAMPALL );

	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );	// TEXTUREFILTER_POINT
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );	// TEXTUREFILTER_POINT
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR );	// TEXTUREFILTER_POINT
	Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_BILINEAR );	// TEXTUREFILTER_POINT

	// write our dynamic buffer to the command buffer and render
	m_SpriteData.Render();
}





