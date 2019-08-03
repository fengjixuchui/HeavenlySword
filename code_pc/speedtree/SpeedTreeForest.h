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

#include "SpeedWind.h"
#include "SpeedTreeUtil.h"
#include "core/nt_std.h"
#include "gfx/texture.h"

class CSpeedTreeWrapper;
class FXMaterial;
class CHierarchy;



///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeForest declaration

class CSpeedTreeForest
{
public:
                                    CSpeedTreeForest( );
virtual                                 ~CSpeedTreeForest( );

    //const float*                    GetExtents(void) const                      { return m_afForestExtents; }
    bool                            Load(const char* pFilename); // requires valid graphics context

    // wind management
	void AdvanceWind(float fTimeInSecs);
	const CSpeedWind*               GetWind(void) const                 { return &m_cSpeedWind; }
    float                           GetWindStrength(void) const                 { return m_fWindStrength; }
    void                            SetWindStrength(float fStrength);
    //void                            SetupWindMatrices(float fTimeInSecs);
    void                            BuildLeafAngleMatrices(const float* pCameraDir) { m_cSpeedWind.BuildLeafAngleMatrices(pCameraDir); }

	bool FinaliseLoad(void);

	void UploadWindMatrixInternal(unsigned int uiLocation, const float* pMatrix, FXMaterial* pMaterial) const;
	void UploadLeafRockMatrixInternal(unsigned int uiLocation, const float* pMatrix, FXMaterial* pMaterial) const;

	//void Render(unsigned long ulRenderBitVector);
	//void RenderDepth(unsigned long ulRenderBitVector);
	//void RenderShadowMap(unsigned long ulRenderBitVector);
	//void RenderRecievers(unsigned long ulRenderBitVector);

	void UpdateForLight();
	void UpdateForCamera();
	void UpdateWind();
	void SetCamera(bool bBillboard);

	SpeedTreeStat GetStat();

	void LoadWindMatrix(FXMaterial* pMaterial);
	void LoadLeafMatrix(FXMaterial* pMaterial);

	u32 GetRenderBitVector() {return m_ulRenderBitVector;}
	void SetRenderBitVector(u32 ulRenderBitVector) {m_ulRenderBitVector=ulRenderBitVector;}
protected:
    //std::vector<CSpeedTreeWrapper*> m_referenceTrees;
    std::vector<CSpeedTreeWrapper*> m_treesInForest;

private:
    void                            AdjustExtents(float x, float y, float z);
    bool                            LoadFromStfFile(const char* pFilename);
    void                            SetLodLimits(void);

private:
    //float                           m_afForestExtents[6];   // [0] = min x, [1] = min y..., [3] = max x, [4] = max y...
    float                           m_fWindStrength;        // 0.0 = no wind, 1.0 = full strength
    CSpeedWind                      m_cSpeedWind;
private:

	//void RenderInternal(unsigned long ulRenderBitVector, const std::string& techName);
	
	u32 m_ulRenderBitVector;

	Texture::Ptr	m_texCompositeMap;      // composite texture

	float m_afCameraPos[3];
	float m_afCameraDir[3];

	float m_afLightPos[3];
	float m_afLightDir[3];

	CHierarchy* m_pHierarchy;
};


#endif // end of _SPEEDTREEFOREST_H_
