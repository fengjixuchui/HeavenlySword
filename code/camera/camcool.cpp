/***************************************************************************************************
*
*	DESCRIPTION: Cool camera classes 
*
*	NOTES: The cool cameras are triggered by KO moves or by scripted events in the game
*		   they override the basic camera system for their duration then return control to
*		   the default camera via some transition.
*
\**************************************************************************************************/


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "camera/camcool.h"
#include "camera/timescalarcurve.h"
#include "camera/converger.h"
#include "camera/smoother.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/sceneelementcomponent.h"
#include "camera/basiccamera.h"

#include "core/timer.h"
#include "core/osddisplay.h"
#include "core/visualdebugger.h"
#include "game/entityinfo.h"
#include "game/entitymanager.h"

#include "physics/system.h"


//------------------------------------------------------------------------------------------
// Static Members                                                                           
//------------------------------------------------------------------------------------------
int CoolCamera::m_iNextID = 1;  // 0 is reserved for uncool cameras


/***************************************************************************************************
*	
*	FUNCTION		CoolCamera
*
*	DESCRIPTION		Construction
*
\**************************************************************************************************/
CoolCamera::CoolCamera(const CamView& view) : 
	CameraInterface(view),
	m_fTime(0.f),
	m_fTotalTime(1.f),
	m_fRealTime(0.f),
	m_bFinished(false),
	m_bRemovingFromView(false),
	m_pCurve(0),
	m_pEndTrans(0),

	m_fTimeScalar(1.f),
	m_iID(m_iNextID++),
	m_iPriority(1)
{
}


/***************************************************************************************************
*	
*	FUNCTION		CoolCamera
*
*	DESCRIPTION		Destruction
*
\**************************************************************************************************/
CoolCamera::~CoolCamera()
{

}


/***************************************************************************************************
*	
*	FUNCTION		CoolCamera::SetTimeCurve
*
*	DESCRIPTION		Set the time curve for the CoolCamera
*
\**************************************************************************************************/
void CoolCamera::SetTimeCurve(TimeScalarCurve *pCurve)
{
	m_pCurve = pCurve;
}


/***************************************************************************************************
*	
*	FUNCTION		CoolCamera::UpdateTime
*
*	DESCRIPTION		Modifies the time scalar according to our time curve
*
\**************************************************************************************************/
void CoolCamera::UpdateTime(float fTimeDelta)
{
	// Check the entity multiplier for the hero...
	CEntity* pentPlayer = CEntityManager::Get().GetPlayer();
	float fTimeMult = 1.0f;
	if(pentPlayer)
		fTimeMult = 1.0f / pentPlayer->GetTimeMultiplier();
	
	// Update the timers...
	m_fTime += fTimeDelta;
	m_fRealTime += CTimer::Get().GetSystemTimeChange();

	// Set the time scalar...
	if(m_fTime > m_fTotalTime)
	{
		// Maintain ending time scalar
	}
	else if(m_pCurve)
	{
		float f = m_pCurve->GetScalar(m_fTime/m_fTotalTime);
		m_fTimeScalar = f < fTimeMult ? f : fTimeMult;
	}
	else
		m_fTimeScalar = fTimeMult;

	//ntPrintf("TSC - %.2f %s\n", m_fTimeScalar, m_pCurve ? "y" : "n");
}


/***************************************************************************************************
*	
*	FUNCTION		CoolCamera::ResetTime
*
*	DESCRIPTION		Zeroes the cool cam timer and sets the total running time
*
\**************************************************************************************************/
void CoolCamera::ResetTime(float fTotalTime, float fTimeIn)
{
	ntAssert(fTotalTime >= 0.0f);
	ntAssert(fTimeIn >= 0.0f);

	m_fTotalTime = fTotalTime;
	m_fTime = fTimeIn;
	m_fRealTime = fTimeIn;
}



CoolCam_Null::CoolCam_Null(const CamView& view, TimeScalarCurve* pCurve, float fDuration)
	: CoolCamera(view) 
{
	ResetTime(fDuration);
	SetTimeCurve(pCurve);
}

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Null::Update
//!	Get the transform from the game camera, update the tscurve.                            
//!
//------------------------------------------------------------------------------------------
void CoolCam_Null::Update(float fTimeDelta)
{
	const CameraInterface* pCam = CamMan::GetPrimaryView()->GetLevelCamera();

	m_fFOV        = pCam->GetFOV();
	m_obTransform = pCam->GetTransform();
	m_obLookAt    = pCam->GetLookAt();

	// Update the time scalar
	UpdateTime(fTimeDelta);

	if(m_fTime > m_fTotalTime)
		SetFinished();
}

void CoolCam_Null::SetFinished()
{
	m_bFinished = true;
}


void CoolCam_Null::EndCamera()
{
	SetFinished();
}
