//--------------------------------------------------
//!
//!	\file effecttrail_buffer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _TRAIL_BUFFER_H
#define _TRAIL_BUFFER_H

#include "effecttrail_simple.h"
#include "gfx/proc_vertexbuffer.h"

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#endif

//--------------------------------------------------
//!
//!	EffectTrailBuffer
//!	Wrapper round a dynamic vertex buffer and static
//! index buffer that holds trail GPU info.
//! Analagous to particle handlers in parametic system
//!
//--------------------------------------------------
class EffectTrailBuffer : public CNonCopyable
{
public:
	EffectTrailBuffer(	const EffectTrail_SimpleDef* pDef,
						const EffectTrail_EdgeDef* pEdge );
	~EffectTrailBuffer();

	void PreRender( const RenderStateBlock& rs );
	void Render();
	void DebugRender();
	void EmitEdge( float fEmitTime, const CMatrix& frame );
	
#ifdef PLATFORM_PC
	ID3DXEffect* GetEffect()	{ return m_ppFX->Get(); }
#elif PLATFORM_PS3
	Shader& GetVertexShader()	{ return *m_pVertexShader; }
	Shader& GetPixelShader()	{ return *m_pPixelShader; }
#endif

private:
	void PlatformConstruct();

	const EffectTrail_SimpleDef*	m_pDef;
	const EffectTrail_EdgeDef*		m_pEdge;
	u_int							m_iCurrEdge;
	u_int							m_iVertsPerEdge;
	u_int							m_iMaxEdges;
	bool							m_bOnFirstLoop;

	enum EDGE_ELEMENT
	{
		EE_BIRTH_TIME	= PV_ELEMENT_0,
		EE_POSITON		= PV_ELEMENT_POS,
		EE_TEXTURE		= PV_ELEMENT_2,		
	};

	struct EdgeVertex
	{
		float birthTime;
		float posX, posY, posZ;
	};

#ifndef _NO_DBGMEM_OR_RELEASE
	CPoint*			m_pDebugPoints;				//!< allocated in Mem::MC_DEBUG
	CMatrix			m_debugFrame;
#endif

	ProceduralVB	m_VB; // GPU per-vertex data
	IBHandle		m_IB; // GPU index buffer

#ifdef PLATFORM_PC
	FXHandle*		m_ppFX;
#elif PLATFORM_PS3
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
#endif
};

#endif //_TRAIL_BUFFER_H
