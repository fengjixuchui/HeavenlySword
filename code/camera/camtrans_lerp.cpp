//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtrans_lerp.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtrans_lerp.h"

#include "camera/camerainterface.h"
#include "camera/camutils.h"

#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CamTrans_LerpDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pDst,  Destination)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTime, 2.0f, Time)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fControlTransitionTotalTime, 2.0f, ControlTime)
END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!
//!	CamTrans_LerpDef::CreateTransition
//! Create our transition
//!
//------------------------------------------------------------------------------------------
CamTransition* CamTrans_LerpDef::Create(const CameraInterface* pSrc, const CameraInterface* pDst) const
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_Lerp(pSrc, pDst, this);
}


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Lerp::Update
//! A simple lerp transition update.
//!
//------------------------------------------------------------------------------------------
void CamTrans_Lerp::Update(float fTimeDelta)
{
	if(m_bStarted)
		m_fTime += fTimeDelta;
	else
		m_bStarted = true;

	float fRatio = 0.0f;

	// Are we doing the camera hold or the actual transition?
	if ( m_fTime <= m_fHoldTime )
	{
		// HOLDING
		fRatio = 0.0f;
	}
	else
	{
		// TRANSITION
		// Ease in and out
		fRatio = CCamUtil::Sigmoid(m_fTime - m_fHoldTime, m_fTransTime);
	}

	CPoint pos = m_pSrc->GetTransform().GetTranslation() + 
				((m_pDst->GetTransform().GetTranslation() - m_pSrc->GetTransform().GetTranslation()) * fRatio);

	m_fFOV = m_pSrc->GetFOV() + ((m_pDst->GetFOV() - m_pSrc->GetFOV()) * fRatio);

	CQuat	qm_src(m_pSrc->GetTransform());
	CQuat	qm_dst(m_pDst->GetTransform());
	CQuat	qFinal = CQuat::Slerp(qm_src, qm_dst, fRatio);

	m_obTransform = CMatrix(qFinal, pos);

	m_obLookAt = m_pSrc->GetLookAt() + ((m_pDst->GetLookAt() - m_pSrc->GetLookAt()) * fRatio);
	//m_obCurrPOI = CCamUtil::CalcAdjustedLookat(m_obCurrMat, m_obCurrPOI);


	// Lerp the DoF settings
	// ----------------------
	if(m_pSrc->UsingDoF() && m_pDst->UsingDoF())
	{
		m_bUseDoF = true;

		m_fFocalDepth    = m_pSrc->GetFocalDepth()    + (m_pDst->GetFocalDepth()    - m_pSrc->GetFocalDepth())    * fRatio;
		m_fFarBlurDepth  = m_pSrc->GetFarBlurDepth()  + (m_pDst->GetFarBlurDepth()  - m_pSrc->GetFarBlurDepth())  * fRatio;
		m_fNearBlurDepth = m_pSrc->GetNearBlurDepth() + (m_pDst->GetNearBlurDepth() - m_pSrc->GetNearBlurDepth()) * fRatio;
		m_fConfusionHigh = m_pSrc->GetConfusionHigh() + (m_pDst->GetConfusionHigh() - m_pSrc->GetConfusionHigh()) * fRatio;
		m_fConfusionLow  = m_pSrc->GetConfusionLow()  + (m_pDst->GetConfusionLow()  - m_pSrc->GetConfusionLow())  * fRatio;
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

	// Are we done yet?
	if(m_fTime > m_fTotalTime)
		m_bActive = false;
}
