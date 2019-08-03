#ifndef _SPEEDTREEMANAGER_H_
#define _SPEEDTREEMANAGER_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeManager.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------



#include "SpeedTreeWrapper.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"
#include "camera/camman_public.h"
#include "anim/transform.h"

class CSpeedTreeForest;
class FXMaterial;

class SpeedTreeManager: public Singleton<SpeedTreeManager>
{
public:
	// global material, used for all trees and frond (with different parameters)
	FXMaterial* m_pBranchMaterial;
	FXMaterial* m_pFrondMaterial;
	FXMaterial* m_pLeafMaterial;
	FXMaterial* m_pBillboardMaterial;
	
	std::vector<FXMaterial*> m_allMaterial;

	CSpeedTreeForest* m_pForest;
	float m_fAccumTime;
	unsigned long m_ulRenderBitVector;
	bool m_bSpeedTreeContext;
	bool m_bSpeedTreeAnimate;
	bool m_bSpeedTreeRender;
public:
	//! constructor
	SpeedTreeManager();
	
	//! destructor
	~SpeedTreeManager();

	void PerFrameUpdate(float fElapsedTime);
	void UpdateForLight();
	void UpdateForCamera();
protected:
	void UpdateWind();
	
	// animate
	//void Animate(float fElapsedTime);
	
	// shortcut
	void CheckKey();

	// debug rendering: stat
	void DebugRender();

}; // end of class SpeedTreeManager

#endif // end of _SPEEDTREEMANAGER_H_
