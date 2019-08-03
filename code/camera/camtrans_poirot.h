//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file CamTrans_POIRot.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANS_POIROT_INC
#define CAMTRANS_POIROT_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtransition.h"


//------------------------------------------------------------------------------------------
// External/Forward Declarations
//------------------------------------------------------------------------------------------
class CamTransition;


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_POIRotDef                                                                        
//!	Definition of a fade in, fade out transition
//!
//------------------------------------------------------------------------------------------
class CamTrans_POIRotDef : public CamTransitionDef
{
public:	
	CamTrans_POIRotDef(float fTime) { m_fTotalTime = fTime; m_pDst = 0; }
	CamTrans_POIRotDef(float fTime, BasicCameraTemplate* pobTemplate) { m_fTotalTime = fTime; m_pDst = pobTemplate; }

	virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const;
	virtual const char*	GetTransTypeName() const {return "POI Rotation Transition";}

public:
	float GetTotalTime() const {return m_fTotalTime;}

protected:
	CamTrans_POIRotDef() {m_fTotalTime = 0.f;}

private:
	float m_fTotalTime;
	friend class CamTrans_POIRotDefInterface;
};


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_POIRot::Update
//!	Fade down, fade up, switch to new matrix after fade dn complete
//!
//------------------------------------------------------------------------------------------
class CamTrans_POIRot : public CamTransition
{
public:
	CamTrans_POIRot(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTrans_POIRotDef* pDef)
		: CamTransition(pSrc, pDst)
	{
		m_fTime = 0.0f;
		// scee.sbashow - the m_fControlTransitionTotalTime 
		// 					for this camera type does not use the pDef's GetControlTransTotalTime
		m_fTotalTime = 
			pDef->GetTotalTime();
		m_fControlTransitionTotalTime = 
			pDef->GetControlTransTotalTime();
	}

	virtual void Update(float fTimeDelta);
	virtual const char*	GetTypeName() const	         {return "TRANS_POIROT";}

// Members
private:
	float       m_fTotalTime;
	float		m_fTime;
};


#endif // CAMTRANS_POIROT_INC
