//------------------------------------------------------------------------------------------
//!
//!	\file screensprite_ps3.h
//! A screen space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#ifndef	SCREEN_SPRITE_PS3_H
#define	SCREEN_SPRITE_PS3_H

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#ifndef GFX_PROC_VB
#include "gfx/proc_vertexbuffer.h"
#endif

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

class CCamera;

//------------------------------------------------------------------------------------------
//!
//!	ScreenSprite
//! simple screen / rendertarget space 2D textured primitive
//!
//------------------------------------------------------------------------------------------
class ScreenSprite
{
public:

	// Construction / Destruction
	ScreenSprite( int iNumberOfSprites = 1 );

	// Set the texture
	void SetTexture( const char* pName );
	void SetTexture( Texture::Ptr pTex );

	// Set the size of a sprite
	void SetSize( float fSize, int iItem = 0 );
	void SetWidth( float fWidth, int iItem = 0 );
	void SetHeight( float fHeight, int iItem = 0 );
	void SetSizeAll( float fSize );

	// Set the colour of the sprite
	void SetColour( uint32_t iCol, int iItem = 0 );
	void SetColourAll( uint32_t iCol );

	// Set the position
	void SetPosition( const CPoint& pos, int iItem = 0 );

	// Draw the sprite(s)
	void Render( const CCamera& );

private:

	enum VERTEX_ELEMENTS
	{
		VE_POSITON		= PV_ELEMENT_POS,
		VE_WIDTHHEIGHT	= PV_ELEMENT_2,
		VE_TEXCOORD		= PV_ELEMENT_3,
		VE_COLOUR		= PV_ELEMENT_4,
	};

	struct Vertex
	{
		Vertex()
		{
			m_pos[0] = 0.0f;			m_pos[1] = 0.0f;
			m_tex[0] = 0.0f;			m_tex[1] = 0.0f;
			m_widthheight[0] = 0.0f;	m_widthheight[1] = 0.0f;
			m_col = NTCOLOUR_ARGB( 0xff, 0xff, 0xff, 0xff );
		}
		
		void SetPos( float x, float y ) { m_pos[0] = x; m_pos[1] = y; }
		void SetTex( float u, float v ) { m_tex[0] = u; m_tex[1] = v; }
		void SetWH( float w, float h ) { m_widthheight[0] = w; m_widthheight[1] = h; }
		void SetW( float w ) { m_widthheight[0] = w; }
		void SetH( float h ) { m_widthheight[1] = h; }
		void SetCol( uint32_t col ) { m_col = col; }

		float		m_pos[2];
		float		m_widthheight[2];
		float		m_tex[2];
		uint32_t	m_col;
	};

	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	ProceduralVB	m_spriteData;
	Texture::Ptr	m_texture;
	bool			m_bBilinear;
};

#endif // SCREEN_SPRITE_PS3_H
