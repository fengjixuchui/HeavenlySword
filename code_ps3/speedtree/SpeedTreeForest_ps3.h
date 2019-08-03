//////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForest Class
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



#ifndef _SPEEDTREEFOREST_H_
#define _SPEEDTREEFOREST_H_



///////////////////////////////////////////////////////////////////////  
//  Include Files



#include "SpeedWind_ps3.h"
#include "speedtree/SpeedTreeUtil_ps3.h"
#include "core/nt_std.h"
#include "gfx/texture.h"

#include "core/boundingvolumes.h"

#include "speedtree/SpeedTreeConfig_ps3.h"



class SpeedTreeXmlForest;
class CSpeedTreeWrapper;
class Shader;
class CHierarchy;
class SpeedTreeXmlTree;

namespace SpeedTreeBillboard
{
	class CCellSystem;
}

///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeForest declaration

class CSpeedTreeForest
{
public:
	typedef wind_float_vector WindVector;
	typedef ntstd::Vector<CSpeedTreeWrapper*>	InstanceContainer;

	CSpeedTreeForest( const SpeedTreeXmlForest * forestDef );
	virtual ~CSpeedTreeForest( );

    //const float*                    GetExtents(void) const                      { return m_afForestExtents; }
    bool                            Load(); // requires valid graphics context
	void							RegisterWithAreaSystem();
	void							OnAreaLoadStart();
	void							OnAreaLoadEnd();

    // wind management
	void AdvanceWind(float fTimeInSecs);
	const CSpeedWind*               GetWind(void) const                 { return &m_cSpeedWind; }
    float                           GetWindStrength(void) const                 { return m_fWindStrength; }
    void                            SetWindStrength(float fStrength);
    //void                            BuildLeafAngleMatrices(const float* pCameraDir) { m_cSpeedWind.BuildLeafAngleMatrices(pCameraDir); }

	void UpdateForLight();
	void UpdateForCamera();
	void PerFrameUpdate(float fElapsedTime);
	void PerViewportUpdate();

	SpeedTreeStat GetStat();

	void LoadWindMatrix(Shader* pMaterial);
	void LoadLeafMatrix(Shader* pMaterial);

	uint32_t GetRenderBitVector();
	void SetRenderBitVector(uint32_t ulRenderBitVector) {m_ulRenderBitVector=ulRenderBitVector;}
	
	const SpeedTreeXmlForest* GetForestDef() const {return m_pForestDef;}

	const WindVector& GetRockAngles()  const
	{
		return m_rockAngles;
	}

	const WindVector& GetRustleAngles() const
	{
		return m_rustleAngles;
	}

	void Enable(bool enable);

#ifdef SPEEDTREE_COLLECT_STATS
	CSpeedTreeWrapper*	DebugGetTree(const SpeedTreeXmlTree* treeDef);
#endif

protected:
    InstanceContainer		  m_treesInForest;
	WindVector				  m_rockAngles;
	WindVector				  m_rustleAngles;

private:
    void                            AdjustExtents(float x, float y, float z);
    bool                            LoadFromStfFile();
    void                            SetLodLimits(void);

private:
    //float                           m_afForestExtents[6];   // [0] = min x, [1] = min y..., [3] = max x, [4] = max y...
    float                           m_fWindStrength;        // 0.0 = no wind, 1.0 = full strength
    CSpeedWind                      m_cSpeedWind;
private:

	//void RenderInternal(unsigned long ulRenderBitVector, const std::string& techName);
	
	uint32_t m_ulRenderBitVector;

	CHierarchy* m_pHierarchy;
	const SpeedTreeXmlForest* m_pForestDef;

	CAABB								m_bounds;
	SpeedTreeBillboard::CCellSystem*	m_cellSystem;
};


#endif // end of _SPEEDTREEFOREST_H_
