//--------------------------------------------------
//!
//!	\file weaponchains.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _WEAPONCHAINS_H
#define _WEAPONCHAINS_H

#include "gfx/texture.h"
#include "gfx/proc_vertexbuffer.h"

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#endif

#ifdef PLATFORM_PC
#include "gfx/fxmaterial.h"
#endif

//--------------------------------------------------
//!
//!	WeaponChainDef
//! Definiton for chain effect
//!
//--------------------------------------------------
class WeaponChainDef
{
public:
	WeaponChainDef();
	
	ntstd::String	m_surfTexName;
	ntstd::String	m_normTexName;
	float		m_fChainWidth;
	float		m_fTextureLength;
	float		m_fSpecularPower;
	float		m_fAlphaRef;
	CVector		m_fDiffuseColour;
	CVector		m_fSpecularColour;
};

//--------------------------------------------------
//!
//!	WeaponChain
//! effect thats designed to be used as the guts of
//! a CRenderable such as the RangeStanceChain
//!
//--------------------------------------------------
class WeaponChain
{
public:
	WeaponChain( u_int iNumVerts = 2, const WeaponChainDef* pDef = 0 );
	~WeaponChain();
	void SetVertexPosition( const CPoint& pos, u_int iIndex );

	void RenderDepth();
	void RenderMaterial();
	void RenderShadowMap();
	void RenderRecieveShadowMap();
	void BuildVertexBuffer();

	// the parent renderable frame flags used to optimise shadowing casting
	unsigned int m_FrameFlags;

private:
	void InternalRender( bool bDepth, bool bShadowMap, bool bRecieveShadow );

	enum VERTEX_ELEMENTS
	{
		VE_POSITON		= PV_ELEMENT_POS,
		VE_NORMAL		= PV_ELEMENT_2,
		VE_TANGENT		= PV_ELEMENT_3,
		VE_BINORMAL		= PV_ELEMENT_4,
		VE_NMAP_TCOORD	= PV_ELEMENT_5,
		VE_SURF_TCOORD	= PV_ELEMENT_6,
	};

	struct Vertex
	{
		float pos[3];
		float normal[3];
		float tangent[3];
		float binormal[3];
		float nmaptex[2];
		float surftex[2];
	};

	static float BuildBasisVecs( const CPoint& startPos,
								const CPoint& endPos,
								const CPoint& eyePos,
								CDirection& normal,
								CDirection& binormal,
								CDirection& tangent );
								
	Texture::Ptr	m_pSurfaceTexture;
	Texture::Ptr	m_pNormalTexture;

	ProceduralVB	m_VB;
	CPoint*			m_pVerts;
	const WeaponChainDef* m_pDef;
	bool			m_bOwnsDef;

	u_int			m_iNumVerts;
	bool			m_bVBInvalid;

#ifdef PLATFORM_PC
	FXHandle*		m_ppFX;
#elif PLATFORM_PS3
	DebugShader*	m_pVertexShader_Colour;
	DebugShader*	m_pVertexShader_Depth;
	DebugShader*	m_pVertexShader_ShadowCast;
	DebugShader*	m_pPixelShader_Colour;
	DebugShader*	m_pPixelShader_Depth;
#endif

};


#endif // _WEAPONCHAINS_H
