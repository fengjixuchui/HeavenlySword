//------------------------------------------------------------------------------------------
//!
//!	\file worldsprite.h
//! A world space quad sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#ifndef	WORLD_SPRITE_H
#define	WORLD_SPRITE_H

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#ifndef _QUAD_LIST_H
#include "effect/quadlist.h"
#endif

class DebugShader;

//------------------------------------------------------------------------------------------
//!
//!	WorldSprite
//! simple screen / rendertarget space 2D textured primitive
//!
//------------------------------------------------------------------------------------------
class WorldSprite
{
public:

	// Construction / Destruction
	WorldSprite( int iNumberOfSprites = 1 );

	// Set the texture
	void SetTexture( const char* pName );
	void SetTexture( Texture::Ptr pTex );

	// Set normalised texture UVs. Default settings are:
	// fTop = 0.0f; fLeft = 0.0f; fBottom = 1.0f; fRight = 1.0f;
	void SetUV( float fTop, float fLeft, float fBottom, float fRight, int iItem = 0 );

	// Set the size of a sprite
	void SetSize( float fSize, int iItem = 0 );
	void SetWidth( float fWidth, int iItem = 0 );
	void SetHeight( float fHeight, int iItem = 0 );
	void SetSizeAll( float fSize );

	// Set the colour of the sprite
	void SetColour( uint32_t iCol, int iItem = 0 );
	void SetColourAll( uint32_t iCol );
	void SetColour( const CVector& col, int iItem = 0 );
	void SetColourAll( const CVector& col );

	// Set the position
	void SetPosition( const CPoint& pos, int iItem = 0 );

	// Draw the sprite(s)
	void Render();

private:

	enum VERTEX_ELEMENTS
	{
		VE_POSITON		= PV_ELEMENT_POS,
		VE_WIDTHHEIGHT	= PV_ELEMENT_2,
		VE_TEXCOORD		= PV_ELEMENT_3,
		VE_COLOUR		= PV_ELEMENT_4,
		VE_CORNER		= PV_ELEMENT_5,
	};

	struct Vertex
	{
		Vertex()
		{
			m_pos[0] = 0.0f;			m_pos[1] = 0.0f;			m_pos[2] = 0.0f;
			m_tex[0] = 0.0f;			m_tex[1] = 0.0f;
			m_widthheight[0] = 0.0f;	m_widthheight[1] = 0.0f;
			m_col[0] = 1.0f;			m_col[1] = 1.0f;
			m_col[2] = 1.0f;			m_col[3] = 1.0f;
			m_corner[0] = 0.0f;			m_corner[1] = 0.0f;
		}
		
		void SetPos( float x, float y, float z ) { m_pos[0] = x; m_pos[1] = y; m_pos[2] = z; }
		void SetTex( float u, float v ) { m_tex[0] = u; m_tex[1] = v; }
		void SetCorner( float x, float y ) { m_corner[0] = x; m_corner[1] = y; }
		void SetWH( float w, float h ) { m_widthheight[0] = w; m_widthheight[1] = h; }
		void SetW( float w ) { m_widthheight[0] = w; }
		void SetH( float h ) { m_widthheight[1] = h; }
		void SetCol( uint32_t col )			{ NTCOLOUR_EXTRACT_FLOATS( col, m_col[0], m_col[1], m_col[2], m_col[3] ); }
		void SetCol( const CVector& col )	{ m_col[0] = col.X(); m_col[1] = col.Y(); m_col[2] = col.Z(); m_col[3] = col.W(); }

		float		m_pos[3];
		float		m_widthheight[2];
		float		m_tex[2];
		float		m_col[4];
		float		m_corner[2];
	};

	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	QuadList		m_spriteData;
	Texture::Ptr	m_texture;
	bool			m_bBilinear;
};

#endif // WORLD_SPRITE_H
