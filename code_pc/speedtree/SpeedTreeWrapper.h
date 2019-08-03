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

//#pragma once
//#pragma warning (disable : 4786)

#ifndef _SPEEDTREEWRAPPER_H_
#define _SPEEDTREEWRAPPER_H_



///////////////////////////////////////////////////////////////////////  
//  Include files

#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"
#include "gfx/texture.h"
#include "SpeedTreeBuffer.h"
#include "core/boostarray.h"
#include "SpeedTreeUtil.h"

class FXMaterial;
class CSpeedTreeForest;
class CMatrix;
class Transform;
class RenderableSpeedTree;


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeWrapper declaration



class CSpeedTreeWrapper
{
public:
	CSpeedTreeWrapper(CSpeedTreeRT* pTree, CSpeedTreeForest* pForest);
	~CSpeedTreeWrapper( );
    
    // geometry 
    bool                        LoadTree(const std::string& pszSptFile, unsigned int nSeed = 1, float fSize = -1.0f, float fSizeVariance = -1.0f);
	CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void);

    const float*                GetBoundingBox(void) const                      { return m_afBoundingBox; }

    CSpeedTreeRT*               GetSpeedTree(void) const                        { return m_pSpeedTree; }
                                                                                
    // lighting                                                                 
    const SpeedTreeMaterials*GetMaterials(void) const { return &m_materials; }
    const SpeedTreeTextures* GetTextures(void) const { return &m_textures; }
	
	// get geometry cache
	CSpeedTreeRT::SGeometry* UpdateGeometryCache(
		unsigned long ulBitVector = SpeedTree_AllGeometry,
		short sOverrideBranchLodValue = -1,
		short sOverrideFrondLodValue = -1,
		short sOverrideLeafLodValue = -1);

    // wind                                                                     
    void                        Advance(void);

    // utility
    void                        CleanUpMemory(void);

	const SpeedTreeBranchBuffers* GetBranchBuffers() const {return &m_branchBuffers;}
	const SpeedTreeFrondBuffers* GetFrondBuffers() const {return &m_frondBuffers;}
	const SpeedTreeLeafBuffers* GetLeafBuffers() const {return &m_leafBuffers;}

private:
    void                        SetupBuffers(void);
    void                        SetupBranchBuffers(void);
    void                        SetupFrondBuffers(void);
    void                        SetupLeafBuffers(void);
private:
	// SpeedTreeRT data
    CSpeedTreeRT*                   m_pSpeedTree;                   // SpeedTree object
    CSpeedTreeRT::STextures*        m_pTextureInfo;                 // cached texture info

	// geometry cache
    CSpeedTreeRT::SGeometry*        m_pGeometryCache;               // geometry cache

    //  geometry
    SpeedTreeBranchBuffers m_branchBuffers;
    SpeedTreeFrondBuffers m_frondBuffers;
    SpeedTreeLeafBuffers m_leafBuffers;

	// tree properties
    float                           m_afBoundingBox[6];             // tree's bounding box
	
	SpeedTreeMaterials m_materials;
	SpeedTreeTextures m_textures;
	
	CSpeedTreeWrapper* m_pInstanceOf;

public:
	CSpeedTreeForest* m_pForest;
	unsigned long m_ulRenderBitVector;
	Transform* m_pTransform;
	float m_boundingBox[6];
	typedef enum
	{
		LEAVES = 0,
		FROND = 1,
		BRANCH = 2,
		BILLBOARD = 3,
	} Renderable;
	Array<RenderableSpeedTree*,4> m_renderables;
public:
	void SetRenderable(Transform* pTransform);
	Transform* GetTransform() {return m_pTransform;}
	SpeedTreeStat GetStat();
	float* GetBoundingBox() {return m_boundingBox;}
	unsigned long GetRenderBitVector();
};


#endif // end of _SPEEDTREEWRAPPER_H_
