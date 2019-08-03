//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtrans_poirot.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtrans_poirot.h"
#include "camera/camerainterface.h"
#include "camera/camutils.h"

#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CamTrans_POIRotDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pDst,  Destination)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTotalTime, 2.0f, Time)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fControlTransitionTotalTime, 2.0f, ControlTime)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_POIRotDef::Create
//!	Transition Creator
//!
//------------------------------------------------------------------------------------------
CamTransition* CamTrans_POIRotDef::Create(const CameraInterface* pSrc, const CameraInterface* pDst) const
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_POIRot(pSrc, pDst, this);
}

//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Fade::Update
//!	Rotate camera theta abound the POI while lerping the POI, distance to POI and fov.
//!
//------------------------------------------------------------------------------------------
void CamTrans_POIRot::Update(float fTimeDelta)
{
	m_fTime += fTimeDelta;

	float fInterpValue = CCamUtil::Sigmoid(m_fTime, m_fTotalTime);
	if(fInterpValue > 1.f || fInterpValue < 0.f)
	{
		ntPrintf("Camera - Bad Sigmoid interpolation value.\n");
		fInterpValue = clamp(fInterpValue, 0.f, 1.f);
	}


	// Calc interpolated POI and FOV
	// ------------------------------
	m_obLookAt = m_pSrc->GetLookAt() + ((m_pDst->GetLookAt() - m_pSrc->GetLookAt()) * fInterpValue);
	m_fFOV = m_pSrc->GetFOV() + ((m_pDst->GetFOV() - m_pSrc->GetFOV()) * fInterpValue);

	// Calc thetas and distances w.r.t. POI
	// -------------------------------------
	float fSY, fSX, fSD;
	CDirection obOffset = m_pSrc->GetTransform().GetTranslation() ^ m_pSrc->GetLookAt();
	fSD = obOffset.Length();
	obOffset.Normalise();
	CCamUtil::SphericalFromCartesian(obOffset, fSX, fSY);

	float fDY, fDX, fDD;
	obOffset = m_pDst->GetTransform().GetTranslation() ^ m_pDst->GetLookAt();
	fDD = obOffset.Length();
	obOffset.Normalise();
	CCamUtil::SphericalFromCartesian(obOffset, fDX, fDY);

	// Go by the shortest arc
	if(fDY-fSY > PI)
		fDY -= TWO_PI;
	else if(fDY-fSY < -PI)
		fDY += TWO_PI;

	// Interpolate this
	fSD = fSD + ((fDD-fSD)*fInterpValue);
	fSY = fSY + ((fDY-fSY)*fInterpValue);
	fSX = fSX + ((fDX-fSX)*fInterpValue);

	//  Generate new matrix
	// ---------------------
	obOffset = CCamUtil::CartesianFromSpherical(fSX, fSY);
	obOffset *= fSD;
	CCamUtil::CreateFromPoints(m_obTransform, m_obLookAt+obOffset, m_obLookAt);

	CMatrix mtSrcRoll(m_obTransform);
	CMatrix mtDstRoll(m_obTransform);

	// scee.sbashow: This was the previous code: 
	//						mtSrcRoll.SetYAxis(m_pSrc->GetTransform().GetYAxis());
	//						mtSrcRoll.BuildXAxis();
	// 
	//						mtDstRoll.SetYAxis(m_pDst->GetTransform().GetYAxis());
	//						mtDstRoll.BuildXAxis();
	// 
	// scee.sbashow: However, it is not enough just to set the Y Axis as above DIRECTLY to the transitioning camera's y axis.
	// 						We must first make sure the y-axes in question, whilst still BASED on 
	// 						the corresponding axes of the transitioning cameras, remain perpendicular to the 
	// 						z-axis calculated in CCamUtil::CreateFromPoints above.
	// 

	//	scee.sbashow: Now for the new code.

		CDirection		obYAXisZPerpSrc = m_pSrc->GetTransform().GetYAxis();
		obYAXisZPerpSrc = 	m_obTransform.GetZAxis().Cross(obYAXisZPerpSrc.Cross(m_obTransform.GetZAxis()));
		obYAXisZPerpSrc.Normalise();
		mtSrcRoll.SetYAxis(obYAXisZPerpSrc);
		mtSrcRoll.BuildXAxis();

		CDirection		obYAXisZPerpDst = m_pDst->GetTransform().GetYAxis();
		obYAXisZPerpDst = 	m_obTransform.GetZAxis().Cross(obYAXisZPerpDst.Cross(m_obTransform.GetZAxis()));
		obYAXisZPerpDst.Normalise();
		mtDstRoll.SetYAxis(obYAXisZPerpDst);
		mtDstRoll.BuildXAxis();

	//

	CQuat	qtSrc(mtSrcRoll);
	CQuat	qtDst(mtDstRoll);
	CQuat	qtFinal = CQuat::Slerp(qtSrc, qtDst, fInterpValue);

	m_obTransform = CMatrix(qtFinal, m_obTransform.GetTranslation());

	m_bActive = m_fTime < m_fTotalTime;

	// Lerp the DoF settings
	// ----------------------
	if(m_pSrc->UsingDoF() && m_pDst->UsingDoF())
	{
		m_bUseDoF = true;

		m_fFocalDepth    = m_pSrc->GetFocalDepth()    + (m_pDst->GetFocalDepth()    - m_pSrc->GetFocalDepth())    * fInterpValue;
		m_fFarBlurDepth  = m_pSrc->GetFarBlurDepth()  + (m_pDst->GetFarBlurDepth()  - m_pSrc->GetFarBlurDepth())  * fInterpValue;
		m_fNearBlurDepth = m_pSrc->GetNearBlurDepth() + (m_pDst->GetNearBlurDepth() - m_pSrc->GetNearBlurDepth()) * fInterpValue;
		m_fConfusionHigh = m_pSrc->GetConfusionHigh() + (m_pDst->GetConfusionHigh() - m_pSrc->GetConfusionHigh()) * fInterpValue;
		m_fConfusionLow  = m_pSrc->GetConfusionLow()  + (m_pDst->GetConfusionLow()  - m_pSrc->GetConfusionLow())  * fInterpValue;
	}
	else
	{
		m_bUseDoF = false;
	}

	// And Motion Blurring
	// -------------------
	if(m_pSrc->UsingMotionBlur())
	{
		m_bUseMotionBlur = true;
		m_fMotionBlur    = m_pSrc->GetMotionBlur();
	}
	else
	{
		m_fMotionBlur = -100.f;
	}

	if(m_pDst->UsingMotionBlur())
	{
		m_bUseMotionBlur = true;
		m_fMotionBlur    = max(m_fMotionBlur, m_pDst->GetMotionBlur());
	}
}
