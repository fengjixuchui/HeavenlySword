//------------------------------------------------------------------------------------------
//!
//!	\file moviesprite.h
//! A screen space quad sprite with 3 or 4 textures and special pixel shader.
//!
//------------------------------------------------------------------------------------------

#ifndef	MOVIE_SPRITE_H_
#define	MOVIE_SPRITE_H_

#include "gfx/texture.h"
#include "effect/quadlist.h"

class DebugShader;

//------------------------------------------------------------------------------------------
//!
//!	MovieSprite
//! simple screen / rendertarget space 2D textured primitive
//!
//------------------------------------------------------------------------------------------
class MovieSprite
{
	public:
		//
		//	Accessors.
		//
				// Set the texture
		enum TextureType
		{
			Y	= 0,
			cR,
			cB,
			Alpha,

			NumTextureTypes
		};
		void 	SetTexture			( Texture::Ptr texture, TextureType type );

				// Set the size of a sprite - in normalised coordinates, (1,1) = full-screen.
		void 	SetWidth			( float width );
		void 	SetHeight			( float height );

				// Set the colour of the sprite
		void	SetColour			( const CVector &col );

				// Set the position - in normalised coordinates (0,0)=centre-of-screen, (1,1)=top-right, (-1,-1)=bottom-left.
		void	SetPosition			( const CPoint &pos );

	public:
		//
		//	Draw the sprite(s)
		//
		void	Render				( bool bForced = false );

	public:
		//
		//	Ctor.
		//
		MovieSprite( int32_t pip_debug_id, Texture::Ptr Y, Texture::Ptr cR, Texture::Ptr cB, Texture::Ptr Alpha = Texture::Ptr() );

	private:
		//
		//	Aggregated members.
		//
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
				m_pos[0] = 0.0f;			m_pos[1] = 0.0f;
				m_tex[0] = 0.0f;			m_tex[1] = 0.0f;
				m_widthheight[0] = 0.0f;	m_widthheight[1] = 0.0f;
				m_col[0] = 1.0f;			m_col[1] = 1.0f;
				m_col[2] = 1.0f;			m_col[3] = 1.0f;
				m_corner[0] = 0.0f;			m_corner[1] = 0.0f;
			}
			
			void SetPos( float x, float y ) { m_pos[0] = x; m_pos[1] = y; }
			void SetTex( float u, float v ) { m_tex[0] = u; m_tex[1] = v; }
			void SetCorner( float x, float y ) { m_corner[0] = x; m_corner[1] = y; }
			void SetWH( float w, float h ) { m_widthheight[0] = w; m_widthheight[1] = h; }
			void SetW( float w ) { m_widthheight[0] = w; }
			void SetH( float h ) { m_widthheight[1] = h; }
			void SetCol( uint32_t col )			{ NTCOLOUR_EXTRACT_FLOATS( col, m_col[0], m_col[1], m_col[2], m_col[3] ); }
			void SetCol( const CVector& col )	{ m_col[0] = col.X(); m_col[1] = col.Y(); m_col[2] = col.Z(); m_col[3] = col.W(); }

			float		m_pos[2];
			float		m_widthheight[2];
			float		m_tex[2];
			float		m_col[4];
			float		m_corner[2];
		};

		DebugShader*	m_VertexShader;
		DebugShader*	m_PixelShader;
		QuadList		m_SpriteData;
		Texture::Ptr	m_Texture[ 4 ];
		int32_t			m_PIPDebugID;
};

#endif // !MOVIE_SPRITE_H_
