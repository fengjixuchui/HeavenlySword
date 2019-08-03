//------------------------------------------------------------------------------------------
//!
//!	\file attackcameras.cpp
//!
//------------------------------------------------------------------------------------------

//------------------------
// Includes
//------------------------
#include "attackcameras.h"
#include "game/attacks.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/luaglobal.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camtools.h"
#include "camera/camtrans_poirot.h"
#include "anim/animator.h"
#include "anim/animation.h"
#include "core/exportstruct_anim.h"
#include "core/osddisplay.h"
#include "camera/coolcam_maya.h"
#include "camera/coolcam_generic.h"
#include "game/entitybindings.h"
#include "game/luaattrtable.h"
#include "objectdatabase/dataobject.h"
#include "tbd/functor.h"

//------------------------------------------------------------------------------------------
// CCombatCamProps Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(CCombatCamProps)
	IHELP		("General Combat Camera Properties Sheet.")
	PUBLISH_VAR_AS(m_fEarlyExitTime, EarlyExitTime)
	PUBLISH_VAR_AS(m_bEnableCombatCams, EnableCombatCams)
	PUBLISH_VAR_AS(m_bEnableTimeScaling, EnableTimeScaling)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// CAttackCamera Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(CAttackCamera)
	IHELP		("Association of an attack with a camera animation.")
	//IREFERENCE	(CAttackCamera, Attack)
	ISTRING		(CAttackCamera, Camera)
	IFLOAT		(CAttackCamera, Delay)
	IBOOL		(CAttackCamera, AllowEarlyExit)
	IREFERENCE  (CAttackCamera, TSCurve)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// CAttackCameraList Interface
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(CAttackCameraList)
	IHELP("Association of attacks with camera animations.")
	PUBLISH_VAR_AS(m_sCameraClump, CameraClump)
	PUBLISH_VAR_AS(m_sAnims, Anims)
	PUBLISH_VAR_AS(m_fTransitionOutTime, TransitionOutTime)
	PUBLISH_PTR_CONTAINER_AS(m_obCameras, Cameras)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(PostPostConstruct)
END_STD_INTERFACE


//---------------------------
// Constants
//---------------------------
const float DEFAULT_COOLCAM_BUTTON_HOLD = 0.50f; // Default time button must be held before cool cam kicks in


//------------------------------------------------------------------------------------------
//!
//!	CCombatCamProps::CCombatCamProps
//!	Construction
//!
//------------------------------------------------------------------------------------------
CCombatCamProps::CCombatCamProps()
{
	m_bEnableCombatCams		= true;
	m_bEnableTimeScaling	= true;
	m_fEarlyExitTime        = 0.f;
}


//------------------------------------------------------------------------------------------
//!
//!	CCombatCamProps::~CCombatCamProps
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CCombatCamProps::~CCombatCamProps()
{
	m_bEnableCombatCams = true;

	ntAssert_p(CombatCamProperties::Exists(), ("MUST have a CombatCamProperties singleton already"));
	CombatCamProperties::Kill();
}


//------------------------------------------------------------------------------------------
//!
//!	CCombatCamProps::PostConstruct
//!	Post Construction
//!
//------------------------------------------------------------------------------------------
void CCombatCamProps::PostConstruct()
{
	ntAssert_p(!CombatCamProperties::Exists(), ("Multiple CCombatCamProps exist in XML, please only define one time!"));

	NT_NEW CombatCamProperties(*this);
}


//------------------------------------------------------------------------------------------
//!
//!	CombatCamProperties::CombatCamProperties
//!	Construction
//!
//------------------------------------------------------------------------------------------
CombatCamProperties::CombatCamProperties(const CCombatCamProps& props) :
	m_data(props)
{}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCamera::CAttackCamera
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAttackCamera::CAttackCamera()
{
	m_obCamera		  = "GAME";
	m_pobTSCurve      = 0;
	m_fDelay		  = DEFAULT_COOLCAM_BUTTON_HOLD;
	m_bAllowEarlyExit = true;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::CAttackCameraList
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAttackCameraList::CAttackCameraList()
	:	m_pCurrentCamera(0)
{
	m_iCurrentCameraID = -1;

	for(int i=0; i<MAX_CAMS; i++)
		m_pCoolAnimator[i] = 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::CAttackCameraList
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CAttackCameraList::~CAttackCameraList()
{
	for(int i=0; i<MAX_CAMS; i++)
	{
		NT_DELETE(m_pCoolAnimator[i]);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::PostPostConstruct
//!	Post-Post-Construction
//!
//------------------------------------------------------------------------------------------
void CAttackCameraList::PostPostConstruct()
{
	for(int i=0; i<MAX_CAMS; i++)
	{
		m_pCoolAnimator[i] = NT_NEW CoolCam_MayaAnimator(ntStr::GetString(m_sCameraClump), ntStr::GetString(m_sAnims));
		m_pCoolAnimator[i]->InstallGetName(NT_NEW SpecificFunctor<CAttackCameraList, ntstd::String, true>(this, &CAttackCameraList::GetName));
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::GetName
//!	Get the name of this instance from the object database.
//!
//------------------------------------------------------------------------------------------
ntstd::String CAttackCameraList::GetName() const
{
	// ALEXEY_TODO:
	return ntStr::GetString(ObjectDatabase::Get().GetDataObjectFromPointer(this)->GetName());
}

//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::ActivateCombatCam
//!	See if there is a combat camera associated with this strike and if the conditions for
//! activation have been met activate it!
//!
//------------------------------------------------------------------------------------------
int CAttackCameraList::ActivateCombatCam(const CStrike* pStrike, float fButtonHeldTime, const Transform* pCamTransform)
{
	ntAssert(fButtonHeldTime >= 0.f);
	
	// Are combat cameras enabled?
	if(!CombatCamProperties::Get().AreCombatCamsEnabled())
		return false;

	// If the strike has no orinator then we shouldn't have a combat cam
	if (!pStrike->GetOriginatorP())
		return false;
	
	CAttackCamera* pobCamera = pStrike->GetAttackDataP()->m_pobAttackCamera;
	TimeScalarCurve* pCurve = 0;

	// Try and match the attack with an associated attack camera.*/
	if(pobCamera)
	{
		// We've got one, but has the attack button been held long enough?
		if(fButtonHeldTime < pobCamera->m_fDelay && pobCamera->m_fDelay > 0.f)
			return false;

		//ntPrintf("Activate Combat Camera '%s'\n", *(pobCamera->m_obCamera));

		// Ok everythings ready to go!

		// Set TSC
		if (CombatCamProperties::Get().IsTimeScalingEnabled() && pobCamera->m_pobTSCurve)
			pCurve = pobCamera->m_pobTSCurve;

		// Create and initialise the maya cool camera, using a CoolAnimator that's not already in use.
		UNUSED(pCamTransform);
		for(int iIdx = 0; iIdx < MAX_CAMS; iIdx++)
		{
			if(!m_pCoolAnimator[iIdx]->GetAnimator()->IsPlayingAnimation())
			{
				CoolCam_MayaDef CoolCamDef;
				CoolCamDef.pCoolAnimator = m_pCoolAnimator[iIdx];
				CoolCamDef.sAnim         = pobCamera->m_obCamera;
				CoolCamDef.pCurve        = pCurve;
				CoolCamDef.fDuration     = pStrike->GetAttackTime();

				CoolCam_Maya* pCoolCam = NT_NEW CoolCam_Maya(*CamMan::GetPrimaryView(), CoolCamDef);
				
				// Position the camera.
				// If we're a supermove following on from a previous camera then we use the previous root-transform...
				if(!pCamTransform)
					pCoolCam->SetMatrix(pStrike->GetOriginatorP()->GetRootTransformP()->GetWorldMatrix());
				else
					pCoolCam->SetMatrix(pCamTransform->GetWorldMatrix());

				// Request a transition back into the game camera when we're done.
				if(m_fTransitionOutTime > 0.f)
				{
					CamTrans_POIRotDef* pDef = NT_NEW CamTrans_POIRotDef(m_fTransitionOutTime);
					pCoolCam->SetEndingTransition(pDef);
				}

				// Add the cool camera to the manager.
				m_iCurrentCameraID = pCoolCam->GetID();

				// This should not be required - CamMan::GetPrimaryView()->RemoveAllCoolCameras();
				CamMan::GetPrimaryView()->AddCoolCamera(pCoolCam);

				// Save the old camera and we're done.
				m_pCurrentCamera = pobCamera;
				OSD::Add(OSD::CAMERA, DC_WHITE, "Activating Combat Cam");
				return m_iCurrentCameraID;
			}
		}
	}

	return -1;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::DeactivateCombatCam
//!	Try and stop the current combat cam.
//!
//------------------------------------------------------------------------------------------
int CAttackCameraList::DeactivateCombatCam(int iID, bool bForce)
{
	// Get the camera to end
	CoolCamera* pCam = CamMan::GetPrimaryView()->GetCoolCam(iID);

	// If we're not a valid attack camera container then just end the camera.
	if(!this)
	{
		if(pCam)
			pCam->EndCamera();

		return -1;
	}

	// If we have a camera check if it's safe to terminate.
	if(pCam)
	{
		if(m_iCurrentCameraID == iID)
		{
			if(bForce || (m_pCurrentCamera && m_pCurrentCamera->m_bAllowEarlyExit && pCam->GetTime() > CombatCamProperties::Get().GetEarlyExitMinimum()))
			{
				pCam->EndCamera();
				OSD::Add(OSD::CAMERA, 0xffffffff, "Deactivating Current Combat Cam");
				
				// No current camera
				m_iCurrentCameraID = -1;
				m_pCurrentCamera   = 0;
			}
		}
		else
		{
			// End the camera.
			pCam->EndCamera();
			OSD::Add(OSD::CAMERA, 0xffffffff, "Deactivating Non-Current Combat Cam");
		}
	}

	return m_iCurrentCameraID;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList::IsActive
//!	Is a combat camera currently active?
//!
//------------------------------------------------------------------------------------------
bool CAttackCameraList::IsActive() const
{
	return CamMan::GetPrimaryView()->IsCoolCamActive();
}

