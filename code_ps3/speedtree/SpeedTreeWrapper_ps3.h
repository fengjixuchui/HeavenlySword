///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRTExample Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//


//#pragma warning (disable : 4786)

#ifndef _SPEEDTREEWRAPPER_H_
#define _SPEEDTREEWRAPPER_H_



///////////////////////////////////////////////////////////////////////  
//  Include files


#include <SpeedTreeRT.h>

#include "speedtree/SpeedTreeMaterial_ps3.h"
#include "speedtree/SpeedTreeUtil_ps3.h"
#include "speedtree/SpeedTreeShaders_ps3.h"
#include "speedtree/SpeedTreeConfig_ps3.h"

#include "gfx/texture.h"
#include "core/boostarray.h"

#include "anim/transform.h"



class CMaterial;
class CSpeedTreeForest;
class CMatrix;
class RenderableSpeedTree;
class SpeedTreeXmlTree;
class CSpeedTreePhysicsRep;
class CUniquePtrContainer;

///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeWrapper declaration

class CSpeedTreeWrapper
{
public:
	// dtor ctor
	CSpeedTreeWrapper(CSpeedTreeRT* pTree, CSpeedTreeForest* pForest);
	~CSpeedTreeWrapper( );
    
    // load from .stf 
    bool LoadTree(const ntstd::String& pszSptFile, const SpeedTreeXmlTree* treeDef);

	void OnAreaLoadStart(const SpeedTreeXmlTree* treeDef);
	void OnAreaLoadEnd(const SpeedTreeXmlTree* treeDef);
	void RegisterTextures(uint32_t	sectorBits);
	
	// make an instance
	CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void);
	
	// get AABB
    const float*                GetBoundingBox(void) const                      { return m_afBoundingBox; }
	const float*				GetPosition(void) const
	{
		return m_pSpeedTree -> GetTreePosition();
	}
	
	// get CSpeedTreeRT 
    CSpeedTreeRT*               GetSpeedTree(void) const                        { return m_pSpeedTree; }
	CSpeedTreeWrapper*	GetParent() const										{ return m_pInstanceOf; }
                                                                                
    // lighting                                                                 
    const SpeedTreeMaterials* GetMaterials(void) const { return &m_materials; }
    const SpeedTreeTextures* GetTextures(void) const { return &m_textures; }
	
    // updates                                                                   
    void UpdateForCamera(const CDirection& cameraDirection);
    void UpdateForLight(const CDirection& cameraDirection);
    void UpdateLod();

    // utility
    void CleanUpMemory(void);
	
	// get buffers
	const SpeedTreeIndexedBuffers* GetBranchBuffers() const {return &m_branchBuffers;}
	const SpeedTreeIndexedBuffers* GetFrondBuffers() const {return &m_frondBuffers;}
	const SpeedTreeLeafBuffers* GetLeafBuffers() const {return &m_leafBuffers;}

	// set position (and finalise construction)
	void SetRenderable(bool visible);

	// get transform
	Transform* GetTransform() {return &m_Transform;}
	
	// get rendering stat
	SpeedTreeStat GetStat();

	//float* GetBoundingBox() {return m_boundingBox;}

	// redering tags
	unsigned long GetRenderBitVector();
	
	// get forest
	CSpeedTreeForest* GetForest() {return m_pForest;}

	void GetLeafScalars(float& rockScalar, float& rustleScalar)	const
	{
		if (m_pGeometryCache && m_pGeometryCache -> m_pLeaves)
		{
			rockScalar = m_pGeometryCache -> m_pLeaves[0].m_fLeafRockScalar;
			rustleScalar = m_pGeometryCache -> m_pLeaves[0].m_fLeafRustleScalar;
		}
	}

	float GetRotation()	const
	{
		return m_rotation;
	}

	void SetRotation(const CQuat& quat);

	float GetScale() const
	{
		return m_scale;
	}
	float GetWindOffset() const
	{
		return m_windOffset;
	}
	float GetCollisionScale() const
	{
		return m_collisionGeometryScale;
	}

	void Enable(bool enable);

	float GetSize() const;

	static bool SortPredicate(CSpeedTreeWrapper* lhs, CSpeedTreeWrapper* rhs)
	{
		return lhs -> m_pInstanceOf < rhs -> m_pInstanceOf;
	}

	float SetScale(float scale, float minLodDistance, float MaxLodDistance)
	{
		m_scale = scale;
		m_collisionGeometryScale *= scale;
		float posOffset = m_afBoundingBox[1] * (1.f - scale);
		for (int i = 0; i < 6; ++i )
		{
			m_afBoundingBox[i] *= scale;
		}

		float fHeight = m_afBoundingBox[4] - m_afBoundingBox[1];
		m_pSpeedTree->SetLodLimits(fHeight * minLodDistance, fHeight * MaxLodDistance);

		return posOffset;
	}


#ifdef SPEEDTREE_COLLECT_STATS
	SpeedTreeXmlTree*	GetXMLDef()
	{
		return m_def;
	}
#endif

private:

	friend class CSpeedTreeForest;

	void SetWindOffset(float offset)
	{
		m_windOffset = offset;
	}

	void CreatePhysicsRep();

    void                        SetupBuffers(void);
    void                        SetupBranchBuffers(void);
    void                        SetupFrondBuffers(void);
    void                        SetupLeafBuffers(void);
	void						SetupBillboardBuffers(void);
public:
	unsigned int				GetVertexFootprint();
	unsigned int				GetIndexFootprint();
	unsigned int				GetTextureFootprint(CUniquePtrContainer& texCont);

private:
	// SpeedTreeRT internal
    CSpeedTreeRT* m_pSpeedTree;
	CSpeedTreePhysicsRep*	m_physicsRep;

	// speedtree textures handle
    CSpeedTreeRT::SMapBank* m_pTextureInfo;
	// game textures handle
	SpeedTreeTextures m_textures;

	// geometry cache
    CSpeedTreeRT::SGeometry* m_pGeometryCache;

    // stream buffers (all shared except billboard)
    SpeedTreeLeafBuffers m_leafBuffers;
    SpeedTreeIndexedBuffers m_frondBuffers;
    SpeedTreeIndexedBuffers m_branchBuffers;
//    SpeedTreeBillboardBuffers m_billboardBuffers;

	// tree properties
    float m_afBoundingBox[6];

public:
	float m_minLODDistance;
	float m_maxLODDistance;

private:
	
	// materials property
	SpeedTreeMaterials m_materials;

	// instance of this one
	CSpeedTreeWrapper* m_pInstanceOf;
	
	// belong to this forest
	CSpeedTreeForest* m_pForest;

	// render bit
	uint32_t m_ulRenderBitVector;

	// game transform (position, NO ROTATION)
	Transform m_Transform;

	// speedtree renderables
	Array<RenderableSpeedTree*,4> m_renderables;

	float m_scale;
	float m_collisionGeometryScale;
	float m_windOffset;
	float m_rotation;

#ifdef SPEEDTREE_COLLECT_STATS
	SpeedTreeXmlTree* m_def;
#endif

};


#endif // end of _SPEEDTREEWRAPPER_H_
