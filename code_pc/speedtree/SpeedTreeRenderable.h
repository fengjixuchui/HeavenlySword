#ifndef _SPEEDTREERENDERABLE_H_
#define _SPEEDTREERENDERABLE_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeRenderable.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/renderable.h"

class CSpeedTreeWrapper;
class FXMaterial;

class RenderableSpeedTree: public CRenderable
{
protected:
	typedef enum
	{
		DEPTH = 0,
		SHADOW_MAP = 1,
		RECEIVE_SHADOW = 2,
		MATERIAL = 3,
		NBMODE = 4
	} RenderMode;
public:
	//! constructor
	RenderableSpeedTree(CSpeedTreeWrapper* pTreeInForest, FXMaterial* pMaterial);

	//! render depths for z pre-pass
	void RenderDepth();
	//! Renders the game material for this renderable.
	void RenderMaterial();
	//! Renders the shadow map depths.
	void RenderShadowMap();
	//! Renders with a shadow map compare only. 
	void RenderShadowOnly();
	//! get triangle count
	u32 GetTriangleCount(RenderMode mode = NBMODE) const;
protected:
	// prepare shader consatnts and others, return true if we really want to render
	virtual bool PreRender() = 0;
	// render the goemetry
	virtual u32 SendGeometry(const std::string& techName) = 0;
	// finalise (almost nothing)
	virtual void PostRender();
	// set the bounding box according to m_pTreeWrapper
	virtual void SetBoundingBox();

protected:
	CSpeedTreeWrapper* m_pTreeWrapper;
	FXMaterial* m_pMaterial;
	u32 m_triangleCount[NBMODE];

}; // end of class RenderableSpeedTree











class RenderableSpeedtreeLeaf: public RenderableSpeedTree
{
public:

	//! constructor
	RenderableSpeedtreeLeaf(CSpeedTreeWrapper* pTreeInForest);


protected:
	// render the goemetry
	virtual u32 SendGeometry(const std::string& techName);
	// prepare shader consatnts and others
	virtual bool PreRender();

	// load matrix orientaion for billboard
	void LoadLeafClusterS() const;
}; // end of class RenderableSpeedtreeLeaf



class RenderableSpeedtreeBranch: public RenderableSpeedTree
{
public:
	//! constructor
	RenderableSpeedtreeBranch(CSpeedTreeWrapper* pTreeInForest);


protected:
	// render the goemetry
	virtual u32 SendGeometry(const std::string& techName);
	// prepare shader consatnts and others
	virtual bool PreRender();
}; // end of class RenderableSpeedtreeBranch


class RenderableSpeedtreeFrond: public RenderableSpeedTree
{
public:

	//! constructor
	RenderableSpeedtreeFrond(CSpeedTreeWrapper* pTreeInForest);


protected:
	// render the goemetry
	virtual u32 SendGeometry(const std::string& techName);
	// prepare shader consatnts and others
	virtual bool PreRender();
}; // end of class RenderableSpeedtreeFrond


class RenderableSpeedtreeBillboard: public RenderableSpeedTree
{
public:

	//! constructor
	RenderableSpeedtreeBillboard(CSpeedTreeWrapper* pTreeInForest);

protected:
	// render the goemetry
	virtual u32 SendGeometry(const std::string& techName);
	// prepare shader consatnts and others
	virtual bool PreRender();
}; // end of class RenderableSpeedtreeBillboard

#endif // end of _SPEEDTREERENDERABLE_H_
