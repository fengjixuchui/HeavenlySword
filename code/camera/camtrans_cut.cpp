//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtrans_poirot.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtrans_cut.h"
#include "camera/camerainterface.h"
#include "camera/camutils.h"
#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(CamTrans_CutDef)
	PUBLISH_PTR_AS(m_pDst,  Destination)

//	PUBLISH_VAR_AS(m_fTotalTime, Time)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fControlTransitionTotalTime, 2.0f, ControlTime)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	CamTrans_CutDef::Create
//!	Transition Creator
//!
//------------------------------------------------------------------------------------------
CamTransition* CamTrans_CutDef::Create(const CameraInterface* pSrc, const CameraInterface* pDst) const
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_Cut(pSrc, pDst, this);
}

//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Cut::Update
//!	
//!
//------------------------------------------------------------------------------------------
void CamTrans_Cut::Update( float fTimeDelta)
{
#ifndef PLATFORM_PS3
	fTimeDelta;
#endif
	m_obLookAt = m_pDst->GetLookAt();
	m_fFOV = m_pDst->GetFOV();

	m_obTransform = m_pDst->GetTransform();
}
