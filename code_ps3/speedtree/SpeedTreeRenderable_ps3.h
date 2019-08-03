#ifndef _SPEEDTREERENDERABLE_H_
#define _SPEEDTREERENDERABLE_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeRenderable.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#include "gfx/renderable.h"
#include "core/exportstruct_clump.h"
#include "speedtree/SpeedTreeUtil_ps3.h"
#include "core/rotationnalindex.h"
#include "speedtree/SpeedTreeShaders_ps3.h"





class CSpeedTreeWrapper;
class CGameMaterialInstance;
class Shader;
class CShaderGraph;
class VBHandle;





class SpeedTreeGameLink;
class SpeedTreeBillboardData;


class RenderableSpeedTree: public CRenderable
{
public:
	//! constructor
	RenderableSpeedTree(CSpeedTreeWrapper* pTreeInForest, SpeedTreeRenderableEnum speedtreeType, bool visible);
	
	// Init
	void Initialise();
	// set material
	//virtual void SetMaterial(const SpeedTreeGameLink& link) = 0;
	//! render depths for z pre-pass
	void RenderDepth();

	virtual void RenderDepth(const CMatrix& screenTransform) = 0;
	//! Renders the game material for this renderable.
	void RenderMaterial();
	//! Renders the shadow map depths.
	void RenderShadowMap();
	//! Renders with a shadow map compare only. 
	void RenderShadowOnly();

	//! a custom sort key calculator
	void CalculateSortKey(const CMatrix *pTransform, uint32_t renderableType);

	//! get triangle count
	uint32_t GetTriangleCount(ShaderId::RenderMode mode = ShaderId::_NB_RENDER_MODE) const;
	//! Set lod enable flag
	virtual void SetLodCulling() = 0;

	static void ResetBatch();
	static void EndBatch();

protected:
	// prepare shader consatnts and others, return true if we really want to render
	//virtual bool PreRender() = 0;
	//// render the goemetry
	virtual uint32_t SendGeometry(const SpeedTreeIndexedBuffers* pBuffer);
	// finalise (almost nothing)
	virtual void PostRender();
	// set the bounding box according to m_pTreeWrapper
	virtual void SetBoundingBox();

	virtual void RenderMaterial(int) = 0;
	
	// set for shado map number n
	const CMatrix& SetShadowMap(uint32_t uIndex);
	// set current shader
	void SetCurrentShaderId(ShaderId::RenderMode renderMode);
	const CShaderGraph* GetCurrentShader();
	// is rendered
	bool IsSpeedTreeEnable();
	// disable this lod from all rendering path
	void EnableLod(bool bValue);

    template <class VertexShader, class PixelShader>
	void SetCommonConstants(const VertexShader& VS, const PixelShader& PS);

	template <class VertexShader, class PixelShader>
	void SetPerInstanceConstants(const VertexShader& VS, const PixelShader& PS);

	template <class VertexShader, class PixelShader>
	void PreRenderTransparent(const VertexShader& VS, const PixelShader& PS);

	template <class VertexShader, class PixelShader>
	void PreRenderOpaque(const VertexShader& VS, const PixelShader& PS);

	template <class VertexShader>
	void PreRenderDepth(const VertexShader& VS, const Texture::Ptr& texture, const CMatrix& screenTransform);

	template <class VertexShader>	
	void UploadWindMatrices(const VertexShader& VS);
	void UpdateBatch();

protected:
	// pointer to wrapper
	CSpeedTreeWrapper* m_pTreeWrapper;
	// pointer to material instance
	CScopedPtr<CGameMaterialInstance> m_pMaterialInstance;
	// trig count
	uint32_t m_triangleCount[ShaderId::_NB_RENDER_MODE];
	// material property (just here for scope really)
	CSharedArray<CMaterialProperty> m_gameMaterialProperty;
	// shader id (what render mode are we in ?)
	ShaderId m_currentShaderId;
	// speedtree type (leaf,frond, branch or billboard
	SpeedTreeRenderableEnum m_speedtreeType;

	float m_alphaTest;
	int	  m_lod;
}; // end of class RenderableSpeedTree











class RenderableSpeedtreeLeaf: public RenderableSpeedTree
{
public:

	//! constructor
	RenderableSpeedtreeLeaf(CSpeedTreeWrapper* pTreeInForest, bool visible);

protected:
	//! Set lod enable flag
	virtual void SetLodCulling();

	//! render depths for z pre-pass
	void RenderDepth(const CMatrix& screenTransform);
	//! Renders the game material for this renderable.
	void RenderMaterial(int);
	//! Renders the shadow map depths.
	//void RenderShadowMap();
	virtual void CalculateSortKey(const CMatrix *pTransform);


private:
	template <class PixelShader>
	unsigned int SendGeometry(int lod, float alphaTest, const PixelShader& PS);

	template <class VertexShader, class PixelShader>
	void		 RenderLeaves(const VertexShader& VS, const PixelShader& PS);

protected:
	float m_alphaTest2;
	int	  m_lod2;
}; // end of class RenderableSpeedtreeLeaf




class RenderableSpeedtreeFrond: public RenderableSpeedTree
{
public:

	//! constructor
	RenderableSpeedtreeFrond(CSpeedTreeWrapper* pTreeInForest, bool visible);

protected:
	//! Set lod enable flag
	virtual void SetLodCulling();

	// set material
	void SetMaterial(const SpeedTreeGameLink& link);

	//! render depths for z pre-pass
	void RenderDepth(const CMatrix& screenTransform);
	//! Renders the game material for this renderable.
	void RenderMaterial(int);
	//! Renders the shadow map depths.
	//void RenderShadowMap();

	virtual void CalculateSortKey(const CMatrix *pTransform);

}; // end of class RenderableSpeedtreeFrond




class RenderableSpeedtreeBranch: public RenderableSpeedTree
{
public:
	//! constructor
	RenderableSpeedtreeBranch(CSpeedTreeWrapper* pTreeInForest, bool visible);

protected:
	//! Set lod enable flag
	virtual void SetLodCulling();

	//! render depths for z pre-pass
	void RenderDepth(const CMatrix& screenTransform);
	//! Renders the game material for this renderable.
	void RenderMaterial(int);
	//! Renders the shadow map depths.
	//void RenderShadowMap();
	virtual void CalculateSortKey(const CMatrix *pTransform);

}; // end of class RenderableSpeedtreeBranch



namespace SpeedTreeBillboard
{
	class CCell;
}

class CRenderableSpeedTreeCell: public CRenderable
{
public:
	CRenderableSpeedTreeCell(const Transform* transform);

	void SetBounds(const CAABB& aabb);

	void SetBufferIndex(int index)
	{
		m_bufferIndex = index;
	}
	
	int GetBufferIndex() const
	{
		return m_bufferIndex;
	}

	void SetTreeData(CSpeedTreeBillboardBuffers* vb, CSpeedTreeWrapper** trees, unsigned int size);

	virtual void RenderDepth();

	virtual void RenderMaterial();

	virtual void RenderShadowMap()
	{
	}

	virtual void RenderShadowOnly()
	{
	}

	virtual void CalculateSortKey(const CMatrix *pTransform);

private:
	void PreComputeConstants();

	template <class VertexShader>
	void SetBillboardConstants(const VertexShader& VS, unsigned int speedTreeNum);

	template <class VertexShader>
	void SetBillboardLightingConstants(const VertexShader& VS, SpeedTreeBillboardData& sBillboard);

	template <class VertexShader>
	void RenderBillboards(const VertexShader& VS, unsigned int speedTreeNum, unsigned int numVerts);

private:
	CSpeedTreeWrapper**					m_pTreeWrappers;
	CSpeedTreeBillboardBuffers*			m_vertexBuffers;
	int									m_bufferIndex;
	unsigned int						m_num360Billboards;
	unsigned int						m_numBaseTrees;

};

#endif // end of _SPEEDTREERENDERABLE_H_
