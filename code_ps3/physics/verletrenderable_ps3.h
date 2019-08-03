#ifndef _VERLETRENDERABLE_H_
#define _VERLETRENDERABLE_H_

//--------------------------------------------------
//!
//!	\file verletrenderable.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#include "gfx/renderable.h"
#include "gfx/shaderid.h"
#include "gfx/renderer.h"
#include "core/rotationnalindex.h"

#include "verletmaterial_ps3.h"

namespace Physics
{

class VerletInstance;
class VerletInstanceDef;
class VerletMaterialInstanceDef;
class VerletGameLink;

//--------------------------------------------------
//!
//!	material and geometry information
//!
//--------------------------------------------------
class VerletRenderableInstance: public CRenderable
{
public:

#ifdef PLATFORM_PS3
	// handle
	IBHandle m_pIndexHandle;
	VBHandle m_pVertexHandleDynamic;
	VBHandle m_pVertexHandleStatic;
	// nb buffer
	static const uint16_t g_uiNbBuffers = 2;
	// buffer index
	RotationnalIndex<uint16_t,g_uiNbBuffers> m_multipleBufferIndex;
	// vertex buffer
	uint16_t m_uiNbVertices;
	typedef uint8_t* VertexBuffer;
	VertexBuffer m_pDynamicVertexBuffer[g_uiNbBuffers];
	VertexBuffer m_pStaticVertexBuffer;

	// material
	VerletMaterialInstance m_material;

	// shader id
	ShaderId m_currentShaderId;
	// trig count
	uint32_t m_triangleCount[ShaderId::_NB_RENDER_MODE];

#endif
	// instance
	const VerletInstance* m_pInstance;

	// index buffer
	uint16_t m_uiNbIndices;
	//uint16_t* m_pIndexBuffer;

public:

#ifdef PLATFORM_PS3
	//! constructor
	VerletRenderableInstance(const VerletInstance* pInstance, const VerletInstanceDef& def, const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link);
#else
	VerletRenderableInstance();
#endif
	//! dtor
	~VerletRenderableInstance();
	// init stream
	void InitVertexStream(const VerletInstanceDef& def, const VerletGameLink& link);
	// enable ?
	bool IsVerletRenderEnable();

	//! render depths for z pre-pass
	void RenderDepth();
	//! Renders the game material for this renderable.
	void RenderMaterial();
	//! Renders the shadow map depths.
	void RenderShadowMap();
	//! Renders with a shadow map compare only. 
	void RenderShadowOnly();

	// per frame update
	void RendererUpdate();

	// prepare shader consatnts and others, return true if we really want to render
	virtual bool PreRender();
	// render the goemetry
	virtual uint32_t SendGeometry();
	// finalise (almost nothing)
	virtual void PostRender();
	// set the bounding box according to m_pTreeWrapper
	virtual void UpdateBoundingBox();
	
	// set for shado map number n
	void SetShadowMap(uint32_t uIndex);
#ifdef PLATFORM_PS3
	// set current shader
	void SetCurrentShaderId(ShaderId::RenderMode renderMode);
	// get current shader
	const CShaderGraph* GetCurrentShader();
	// get material instance
	CGameMaterialInstance* GetMaterialInstance()
	{
		return m_material.m_pMaterialInstance.Get();
	}
	// get next buffer pointer
	VertexBuffer GetNextBufferPointer();
#endif
}; // end of class VerletRenderable



} //Physics

#endif // end of _VERLETRENDERABLE_H_
