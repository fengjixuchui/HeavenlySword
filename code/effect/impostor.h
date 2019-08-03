//------------------------------------------------------------------------------------------
//!
//!	\file impostor.h
//! A world space quad or point sprite with a single texture and colour
//!
//------------------------------------------------------------------------------------------

#ifndef	IMPOSTOR_H
#define	IMPOSTOR_H

#ifndef GFX_TEXTURE_H
#include "gfx/texture.h"
#endif

#ifndef _QUAD_LIST_H
#include "effect/quadlist.h"
#endif

class DebugShader;
class TextureAtlas;

//------------------------------------------------------------------------------------------
//!
//!	ImpostorDef
//! Defines what kind of impostor we are
//!
//------------------------------------------------------------------------------------------
class ImpostorDef
{
public:
	enum MATERIAL
	{
		I_LAMBERT_DH
	};

	enum TYPE
	{
		I_RANDOM,
		I_ANIMATED,
	};

	ImpostorDef() :
		m_material( I_LAMBERT_DH ),
		m_type( I_RANDOM ),
		m_fWidth( 1.0f ),
		m_fHeight( 1.0f ),
		m_fAlphaTest( 0.5f ),
		m_fCycleTime( 1.0f ),
		m_surfaceTex( "unknown.dds" ),
		m_normalTex( "unknown.dds" )
	{}
	
	MATERIAL		m_material;
	TYPE			m_type;
	
	float			m_fWidth;
	float			m_fHeight;
	float			m_fAlphaTest;
	float			m_fCycleTime;

	ntstd::String	m_surfaceTex;
	ntstd::String	m_normalTex;
};

//------------------------------------------------------------------------------------------
//!
//!	PointImpostor
//! simple list of point sprites with lit material shaders.
//!
//------------------------------------------------------------------------------------------
class PointImpostor
{
public:

	// Construction / Destruction
	PointImpostor( const ImpostorDef& def, int max = 1 );

	// Set the dimensions of an individual impostor
	void SetSize( float fSize, int iItem = 0 );

	// Set the position of an individual impostor
	void SetPosition( const CPoint& pos, int iItem = 0 );
	void SetSeed( const float seed, int iItem = 0 );

	// Draw the sprite(s)
	void RenderDepth();								// draw depth only
	void RenderMaterial();							// draw normally
	void RenderShadowMap();							// cast shadow

	// the parent renderable frame flags used to optimise shadowing casting
	unsigned int m_FrameFlags;

	// Set the number to render at this time
	void SetNumImpostors( int num ) 
	{ 
		m_num = num;
	}

	int GetNumImposters()
	{
		return m_num;
	}

private:
	void RenderDepthInternal( const CMatrix& projection );

	enum POINT_VERTEX_ELEMENTS
	{
		PVE_POSITON		= PV_ELEMENT_POS,
		PVE_SIZE		= PV_ELEMENT_2,
		PVE_RANDVALUE	= PV_ELEMENT_3,
	};

	struct PointVertex
	{
		PointVertex()
		{
			m_pos[0] = 0.0f;			m_pos[1] = 0.0f;			m_pos[2] = 0.0f;
			m_size = 1.0f;
			m_randseed = 0.0f;
		}
		
		void SetPos( float x, float y, float z ) { m_pos[0] = x; m_pos[1] = y; m_pos[2] = z; }
		void SetSize( float s ) { m_size = s; }
		void SetSeed( float s ) { m_randseed = s; }

		float		m_pos[3];
		float		m_size;
		float		m_randseed;
	};

	int				m_max;
	int				m_num;

	ImpostorDef		m_def;
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	DebugShader*	m_pDepthVS;
	DebugShader*	m_pDepthPS;

	QuadList			m_spriteData;
	const TextureAtlas*	m_pSurfaceAtlas;
	const TextureAtlas*	m_pNormalAtlas;
};

//------------------------------------------------------------------------------------------
//!
//!	QuadImpostor
//! simple list of quad sprites with lit material shaders.
//!
//------------------------------------------------------------------------------------------
class QuadImpostor
{
public:

	// Construction / Destruction
	QuadImpostor( const ImpostorDef& def, int max = 1 );

	// Set the dimensions of an individual impostor
	void SetWidth( float fWidth, int iItem = 0 );
	void SetHeight( float fHeight, int iItem = 0 );

	// Set the position of an individual impostor
	void SetPosition( const CPoint& pos, int iItem = 0 );
	void SetSeed( const float seed, int iItem = 0 );

	// Draw the sprite(s)
	void RenderDepth();								// draw depth only
	void RenderMaterial();							// draw normally
	void RenderShadowMap();							// cast shadow

	// the parent renderable frame flags used to optimise shadowing casting
	unsigned int m_FrameFlags;

	// Set the number to render at this time
	void SetNumImpostors( int num ) 
	{ 
		m_num = num;
	}

	int GetNumImposters()
	{
		return m_num;
	}

private:
	void RenderDepthInternal( const CMatrix& projection );

	enum QUAD_VERTEX_ELEMENTS
	{
		QVE_POSITON		= PV_ELEMENT_POS,
		QVE_WIDTHHEIGHT	= PV_ELEMENT_2,
		QVE_TEXCOORD	= PV_ELEMENT_3,
		QVE_RANDVALUE	= PV_ELEMENT_4,
	};

	struct QuadVertex
	{
		QuadVertex()
		{
			m_pos[0] = 0.0f;			m_pos[1] = 0.0f;			m_pos[2] = 0.0f;
			m_tex[0] = 0.0f;			m_tex[1] = 0.0f;
			m_widthheight[0] = 0.0f;	m_widthheight[1] = 0.0f;
			m_randseed = 0.0f;
		}
		
		void SetPos( float x, float y, float z ) { m_pos[0] = x; m_pos[1] = y; m_pos[2] = z; }
		void SetTex( float u, float v ) { m_tex[0] = u; m_tex[1] = v; }
		void SetWH( float w, float h ) { m_widthheight[0] = w; m_widthheight[1] = h; }
		void SetW( float w ) { m_widthheight[0] = w; }
		void SetH( float h ) { m_widthheight[1] = h; }
		void SetSeed( float s ) { m_randseed = s; }

		float		m_pos[3];
		float		m_widthheight[2];
		float		m_tex[2];
		float		m_randseed;
	};

	int				m_max;
	int				m_num;

	ImpostorDef		m_def;
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	DebugShader*	m_pDepthVS;
	DebugShader*	m_pDepthPS;

	QuadList			m_spriteData;
	const TextureAtlas*	m_pSurfaceAtlas;
	const TextureAtlas*	m_pNormalAtlas;
};

#endif // IMPOSTOR_H
