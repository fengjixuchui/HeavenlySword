//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtrans_fade.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtrans_fade.h"
#include "camera/camerainterface.h"
#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CamTrans_FadeDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pDst,      Destination)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_dwCol,     0, FadeColour)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFadeDown, 1.0f, FadeDownTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFadeUp,   1.0f, FadeUpTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fControlTransitionTotalTime, 2.0f, ControlTime)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Fade::CreateTransition
//!	Transition Creator
//!
//------------------------------------------------------------------------------------------
CamTransition* CamTrans_FadeDef::Create(const CameraInterface* pSrc, const CameraInterface* pDst) const
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_Fade(pSrc, pDst, this);
}


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Fade::Update
//!	Fade down, fade up, switch to new matrix after fade dn complete
//!
//------------------------------------------------------------------------------------------
void CamTrans_Fade::Update(float fTimeChange)
{
	float fGammaVal = 1.0f;

	m_fTime += fTimeChange;
	
	if(FadingDown())
	{
		fGammaVal = 1.0f - (m_fTime /  m_def.GetFadeDown());

		m_obTransform = m_pSrc->GetTransform();
		m_obLookAt = m_pSrc->GetLookAt();
		m_fFOV = m_pSrc->GetFOV();
	}
	else if(FadingUp())
	{
		fGammaVal = (m_fTime - m_def.GetFadeDown()) /  m_def.GetFadeUp();

		m_obTransform = m_pDst->GetTransform();
		m_obLookAt = m_pDst->GetLookAt();
		m_fFOV = m_pDst->GetFOV();
	}
	else
	{
		m_obTransform = m_pDst->GetTransform();
		m_obLookAt = m_pDst->GetLookAt();
		m_fFOV = m_pDst->GetFOV();

		m_bActive = false;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_FadeSlerp::Update
//!	Fade to new matrix, then slerp to new camera
//!
//------------------------------------------------------------------------------------------
/*bool CamTrans_FadeSlerp::Update(const CMatrix& m_srcMat, const CPoint& m_srcPOI, float fm_srcFOV,
                                const CMatrix& m_dstMat, const CPoint& m_dstPOI, float fDestFOV,
                                float          fTimeChange)
{
	bool bFinished = true;

	if(!m_pFadeCut->Update(m_srcMat, m_srcPOI, fm_srcFOV, GetDef().Getm_srcMat(), m_srcPOI, GetDef().Getm_srcFOV(), fTimeChange))
		bFinished = false;

	if(!m_pLerpSlerp->Update(GetDef().Getm_srcMat(), m_dstPOI, GetDef().Getm_srcFOV(), m_dstMat, m_dstPOI, fDestFOV, fTimeChange))
		bFinished = false;

	if(m_pFadeCut->FadingDown())
	{
		m_obTransform = m_pFadeCut->GetTransform();
		m_obLookAt = m_pFadeCut->GetPOI();
		m_fFOV = m_pFadeCut->GetFOV();
	}
	else
	{
		m_obTransform = m_pLerpSlerp->GetTransform();
		m_obLookAt = m_pLerpSlerp->GetPOI();
		m_fFOV = m_pLerpSlerp->GetFOV();
	}

	return bFinished;
}*/
