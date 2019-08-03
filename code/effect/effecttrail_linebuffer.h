//--------------------------------------------------
//!
//!	\file effecttrail_linebuffer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _TRAIL_BUFFER_H
#define _TRAIL_BUFFER_H

#include "effecttrail_utils.h"
#include "gfx/proc_vertexbuffer.h"

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#else
#include "gfx/fxmaterial.h"
#endif

class EffectTrail_LineDef;
class RenderStateBlock;

//--------------------------------------------------
//!
//!	EffectTrailLineBuffer
//!	Wrapper round a dynamic vertex buffer and static
//! index buffer that holds trail GPU info.
//! Analagous to particle handlers in parametic system
//!
//--------------------------------------------------
class EffectTrailLineBuffer : public CNonCopyable
{
public:
	EffectTrailLineBuffer(	const EffectTrail_LineDef* pDef );
	~EffectTrailLineBuffer();

	void PreRender( const RenderStateBlock& rs );
	void Render();
	void DebugRender();
	void EmitPoint( float fEmitTime, const CPoint& newpoint );
	
#ifdef PLATFORM_PC
	ID3DXEffect* GetEffect()	{ return m_ppFX->Get(); }
#elif PLATFORM_PS3
	Shader& GetVertexShader()	{ return *m_pVertexShader; }
	Shader& GetPixelShader()	{ return *m_pPixelShader; }
#endif

private:
	void PlatformConstruct();

	const EffectTrail_LineDef*	m_pDef;
	u_int						m_iCurrPoint;
	u_int						m_iLastPoint;
	u_int						m_iMaxPoints;
	bool						m_bOnFirstLoop;
	CPoint						m_lastPoint;

	enum LINE_ELEMENT
	{
		LE_BIRTH_TIME	= PV_ELEMENT_0,
		LE_POSITON		= PV_ELEMENT_POS,
		LE_TOLAST_VEC	= PV_ELEMENT_2,
		LE_TONEXT_VEC	= PV_ELEMENT_3,
		LE_TEXTURE		= PV_ELEMENT_4,		
	};

	struct EdgeVertex
	{
		float birthTime;
		float posX, posY, posZ;
		float toLastX, toLastY, toLastZ;
		float toNextX, toNextY, toNextZ;
	};

	EdgeVertex		m_lastVertex;

	CPoint*			m_pDebugPoints;
	ProceduralVB	m_VB; // GPU per-vertex data

#ifdef PLATFORM_PC
	FXHandle*		m_ppFX;
#elif PLATFORM_PS3
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
#endif
};

#endif // _TRAIL_BUFFER_H
