//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camview.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/camview.h"
#include "camera/camcool.h"
#include "camera/basiccamera.h"
#include "camera/debugcam.h"
#include "camera/camutils.h"
#include "camera/camtransition.h"
#include "camera/coolcam_chaseaimcombo.h"
#include "camera/coolcam_chase.h"
#include "camera/coolcam_aim.h"
#include "camera/coolcam_generic.h"
#include "camera/coolcam_aftertouch.h"
#include "camera/coolcam_maya.h"
#include "camera/coolcam_turret.h"
#include "camera/coolcam_boss.h"
#include "camera/coolcam_gladiatorgeneral.h"
#include "camera/coolcam_kingbossfight.h"
#include "camera/coolcam_aerialgeneral.h"
#include "camera/coolcam_watergeneral.h"
#include "camera/coolcam_kingbossfightv2.h"
#include "camera/camtrans_lerp.h"
#include "camera/camtrans_poirot.h"
#include "camera/camman.h"

#include "anim/hierarchy.h"

#include "core/osddisplay.h"
#include "core/timer.h"
#include "tbd/functor.h"

#include "game/entitymanager.h"
#include "game/inputcomponent.h"

#include "jamnet/netman.h"
#include "gfx/depthoffield.h"
#include "gfx/renderer.h"
#include "gfx/pictureinpicture.h"
#include "gfx/sector.h"

#include "Physics/debugdraw.h"
#include "Physics/system.h"
#include "Physics/projectilelg.h"
#include "Physics/world.h"

#include "objectdatabase/dataobject.h"

#include "game/luaglobal.h"

#include "game/entityboss.h"

//------------------------------------------------------------------------------------------
// CamViewInfo XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CamViewInfo, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iPriority,       0,		Priority)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFinalTop,       0.0f,	Top)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFinalBottom,    1.0f,	Bottom)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFinalLeft,      0.0f,	Left)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFinalRight,     1.0f,	Right)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInitialTop,     0.0f,	InitialTop)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInitialBottom,  1.0f,	InitialBottom)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInitialLeft,    0.0f,	InitialLeft)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fInitialRight,   1.0f,	InitialRight)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTotalTransTime, 0.0f,	TransitionTime)
	PUBLISH_PTR_AS(m_pPrimaryEntity,						FocalEntity)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPrimaryLock,    false,	PrimaryLock)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAllowShakes,    false,   AllowShakes)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// CamView Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CamView)
	LUA_EXPOSED_METHOD(Activate,                   ActivateLevelCamera,       "Activate a level camera", "camera Camera", "Choose the camera from the Camera.* Set")
	LUA_EXPOSED_METHOD_SET(UseCombatCameras,       UseCombatCameras,          "Enable combat cameras?")
	LUA_EXPOSED_METHOD_GET(UseCombatCameras,       IsUsingCombatCameras,      "Enable combat cameras?")

	// View Info
	LUA_EXPOSED_METHOD(SetPrimaryEntity,           SetPrimaryEntity,          "Change the primary focus entity", "entity FocusEnt", "")
	LUA_EXPOSED_METHOD(DestroyView,                DestroyView,               "Destroy this view", "", "")
	LUA_EXPOSED_METHOD(MoveView,                   MoveView,                  "Reposition the view", "number top, number left, number bottom, number right, number time", "")
	LUA_EXPOSED_METHOD(FadeTo,                     FadeTo,                    "Fade this view to a flat colour",   "number colour, number time", "colour to fade to|duration of fade")
	LUA_EXPOSED_METHOD(FadeFrom,                   FadeFrom,                  "Fade this view from a flat colour", "number colour, number time", "colour to fade from|duration of fade")

	// Cool Cameras
	LUA_EXPOSED_METHOD(KillCoolCam,                RemoveCoolCamera,          "Stop a cool camera",          "coolcamID id", "The ID is returned when you start a cool camera.")
	LUA_EXPOSED_METHOD(KillAllCoolCams,            Lua_RemoveAllCoolCameras,  "Stop all cool cameras",       "", "")
	LUA_EXPOSED_METHOD(ActivateAfterTouchCoolCam,  ActivateAfterTouchCoolCam, "Start an aftertouch camera",  "entity FollowObject, string Definition", "The Entity the camera will follow|The name of an XML definition of an aftertouch camera")
	LUA_EXPOSED_METHOD(AfterTouchCoolCamLookAt,    AfterTouchCoolCamLookAt,   "Look at a struck entity.",    "entity LookAtEnt, bool Rebound", "The Entity which the camera will try to look at|Has the object rebounded or been destroyed?")
	LUA_EXPOSED_METHOD(ActivateRotatingCam,        ActivateRotatingCamera,    "Start a generic cool camera", "entity ent1, entity ent2, number time, number angle", "First Participating Entity|Second Participating Entity|Duration of the camera|Angle (in degrees) the camera will rotate")
	LUA_EXPOSED_METHOD(ActivateChaseAimCam,        ActivateChaseAimCam,       "Start a chase/aim camera.",   "entity ChasedEntity, string ChaseDef, string AimDef", "The Entity the camera will chase|The name of an XML definition of a chase camera|The name of an XML definition of an aim camera")
	LUA_EXPOSED_METHOD(AddFixedEndTrans,		   AddFixedEndTrans,		  "Add fadeout transition",      "cam id, time to blend", "")
	LUA_EXPOSED_METHOD(ActivateChaseCam,           ActivateChaseCam,          "Start a chase camera.",       "entity ChasedEntity, string ChaseDef", "The Entity the camera will chase|The name of an XML definition of a chase camera")
	LUA_EXPOSED_METHOD(ActivateAimCam,             ActivateAimCam,            "Start an aiming camera.",     "entity ChasedEntity, string AimDef", "The Entity the camera will chase|The name of an XML definition of an aiming camera")
	LUA_EXPOSED_METHOD(ActivateMayaCam,            ActivateMayaCam,           "Start a maya cool camera.",   "string Animation, entity InformEntity, entity ParentEntity", "The name of the animation to play|An Entity to send a finished message to|An Entity to parent the camera to")
	LUA_EXPOSED_METHOD(ActivateBossCamera,		   ActivateBossCamera,        "Start a boss camera.",		 "entity Player, entity Boss", "Player then boss")

//	LUA_EXPOSED_METHOD(ActivateGladiatorGeneralCamera,   ActivateGladiatorGeneralCamera, "Start a glad. gen. camera.",	"entity Boss", "Just boss")
	
	LUA_EXPOSED_METHOD(ActivateMappedGladiatorGeneralCamera,   ActivateMappedGladiatorGeneralCamera, "Start a mapped glad. gen. camera.",	"string defname", "Camera def")
	LUA_EXPOSED_METHOD(ActivateMappedKingBossFightCamera,   ActivateMappedKingBossFightCamera, "Start a mapped king boss camera.",	"string defname", "Camera def")
	LUA_EXPOSED_METHOD(ActivateMappedAerialGeneralCamera,   ActivateMappedAerialGeneralCamera, "Start a mapped aerial gen. camera.",	"string defname", "Camera def")
	LUA_EXPOSED_METHOD(ActivateMappedWaterGeneralCamera,   ActivateMappedWaterGeneralCamera, "Start a mapped water gen. camera.",	"string defname", "Camera def")
	LUA_EXPOSED_METHOD(ActivateMappedKingBossFightCamerav2,   ActivateMappedKingBossFightCamerav2, "Start a mapped king boss camera.",	"string defname", "Camera def")

	LUA_EXPOSED_METHOD(ActivateBossCameraReverseAngle,	ActivateBossCameraReverseAngle, "Trigger the reverse camera angle mode for a boss cam.", "string defname", "Camera def")
	LUA_EXPOSED_METHOD(DeactivateBossCameraReverseAngle,	DeactivateBossCameraReverseAngle, "Turn off the reverse camera angle mode for a boss cam.", "string defname", "Camera def")

	LUA_EXPOSED_METHOD(ActivateBossCamReverseAngleLeadIn,	ActivateReverseAngleLeadIn, "Trigger the reverse camera angle lead-in mode for a boss cam.", "string defname", "Camera def")
	LUA_EXPOSED_METHOD(DeactivateBossCamReverseAngleLeadIn,	DeactivateReverseAngleLeadIn, "Turn off the reverse camera angle lean-in mode for a boss cam.", "string defname", "Camera def")

	LUA_EXPOSED_METHOD(SetBossCameraFOV,	SetBossCameraFOV, "Set the field of view for a boss cam.", "string defname, number fov", "Camera def|Field of View")
	LUA_EXPOSED_METHOD(SetBossCameraFOVBlendTime,	SetBossCameraFOVBlendTime, "Set the field of view blend time for a boss cam.", "string defname, number blendtime", "Camera def|Blend time")

	// Others
	LUA_EXPOSED_METHOD(SetDoF,                     SetDoF,                    "Set the depth of field parameters.", "entity FocalEntity, number FarRange, number NearRange, number TransitionTime", "Entity to focus on|Maximum blur after this distance|Maximum blur before this distance|Time to blend from old settings to new")
LUA_EXPOSED_END(CamView)


//------------------------------------------------------------------------------------------
// Constants                                                                        
//------------------------------------------------------------------------------------------
static const float DEFAULT_VIEW_FOV = 30.0f * DEG_TO_RAD_VALUE;

#ifdef USER_JohnL
//  #define CAMERA_DEBUG
#endif



//------------------------------------------------------------------------------------------
//!
//!	CamViewInfo::CamViewInfo
//!	Parameterised Construction
//!
//------------------------------------------------------------------------------------------
CamViewInfo::CamViewInfo(int iID, float fTop, float fLeft, float fWidth, float fHeight, CEntity* pEnt)
: m_pPrimaryEntity(pEnt), m_bPrimaryLock(false), m_iID(iID), m_iPriority(0),
  m_fTop(fTop), m_fLeft(fLeft), m_fWidth(fWidth), m_fHeight(fHeight),
  m_bFading(false), m_bTransitioning(false), m_bAllowShakes(false)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CamViewInfo::Update
//!	Update any lerp that's happening
//!
//------------------------------------------------------------------------------------------
void CamViewInfo::Update(float fTimeDelta)
{
	// Resize Transition?
	if(m_bTransitioning)
	{
		if(m_fTransTime >= m_fTotalTransTime)
			m_bTransitioning = false;

		float fSigmoid = CCamUtil::Sigmoid(m_fTransTime, m_fTotalTransTime);
		m_fTop    =  fSigmoid*m_fFinalTop    + (1.f-fSigmoid)*m_fInitialTop;
		m_fLeft   =  fSigmoid*m_fFinalLeft   + (1.f-fSigmoid)*m_fInitialLeft;
		m_fWidth  =  fSigmoid*m_fFinalRight  + (1.f-fSigmoid)*m_fInitialRight  - m_fLeft;
		m_fHeight =  fSigmoid*m_fFinalBottom + (1.f-fSigmoid)*m_fInitialBottom - m_fTop;

		PIPView& vp = Renderer::Get().m_pPIPManager->GetView(m_iID);
		vp.SetViewPos(m_fLeft,  m_fTop);
		vp.SetViewDim(m_fWidth, m_fHeight);																		

		m_fTransTime += fTimeDelta;
	}

	// Colour Fade Transition?
	if(m_bFading)
	{
		if(m_fFadeTime >= m_fFadeTotalTime)
			m_bFading = false;

		PIPView& vp = Renderer::Get().m_pPIPManager->GetView(m_iID);
		vp.SetFadeColour(m_iFadeColour);

		if(m_bFadeTo)
		{
			float fFrac = clamp(m_fFadeTime / m_fFadeTotalTime, 0.f, 1.f);
			vp.SetFadeFraction(fFrac);
		}
		else
		{
			float fFrac = 1.f - clamp(m_fFadeTime / m_fFadeTotalTime, 0.f, 1.f);
			vp.SetFadeFraction(fFrac);
		}
		
		m_fFadeTime += fTimeDelta;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamViewInfo::MoveView
//!	Reposition the view on the screen
//!
//------------------------------------------------------------------------------------------
void CamViewInfo::MoveView(float fTop, float fLeft, float fBottom, float fRight, float fTime)
{
	m_fFinalTop    = fTop;
	m_fFinalLeft   = fLeft;
	m_fFinalBottom = fBottom;
	m_fFinalRight  = fRight;
	m_fInitialTop    = m_fTop;
	m_fInitialLeft   = m_fLeft;
	m_fInitialBottom = m_fTop + m_fHeight;
	m_fInitialRight  = m_fLeft + m_fWidth;

	m_fTransTime = 0.f;
	m_fTotalTransTime = fTime;
	m_bTransitioning = true;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::CamView
//!	Construction
//!
//------------------------------------------------------------------------------------------
CamView::CamView(const CamViewInfo& info)
 :	m_info(info),
	m_bActive(true),
	m_pCoolAnimator(0),
	m_pCurrCamera(0),
	m_pCurrLevelCamera(0),
	m_elementMan(info.GetPrimaryEntity(), info.IsPrimaryLocked()),
	m_shake(0.f, 0.f, 0.f),
	m_pDoFEntity(info.GetPrimaryEntity()),
	m_fDoFFar(120.f),
	m_fDoFNear(15.f),
	m_fConfusionHigh(5.f),
	m_fConfusionLow(2.5f),
	m_bUseDoF(true),
	m_bUseCombatCameras(true),
	m_eDebugMode(DM_NORMAL),
	m_pDebugCamera(0),
	m_fDebugFOV(35.f),
	m_fDebugDist(4.5f),
	m_fDebugCameraSpeed(10.0f)

{
	ATTACH_LUA_INTERFACE(CamView);

	// View Attributes
	m_transform.SetLocalMatrix(CMatrix(CONSTRUCT_IDENTITY));
	SetViewTransform(&m_transform);
	CHierarchy::GetWorld()->GetRootTransform()->AddChild(&m_transform);

	SetFOVAngle(DEFAULT_VIEW_FOV);

	// Finalise the initialisation
	Init();

	// Inital info update
	m_info.Update(0.f);

	// Add the view to the PIPManager
	PIPView& vp = Renderer::Get().m_pPIPManager->GetView(info.GetID());

	vp.SetCamera(this);
	vp.SetViewDim(m_info.GetWidth(), m_info.GetHeight());																		
	vp.SetViewPos(m_info.GetLeft(),  m_info.GetTop());
	vp.SetActive(true);
	vp.SetViewPriority(m_info.GetPriority());
	vp.SetFadeColour(NTCOLOUR_ARGB(0xFF,0x00,0x00,0x00));
	vp.SetFadeFraction(0.f);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::~CamView
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CamView::~CamView()
{
	m_transform.RemoveFromParent();

	// Remove the view from the PIPManager
	PIPView& vp = Renderer::Get().m_pPIPManager->GetView(m_info.GetID());
	vp.SetCamera(0);
	vp.SetActive(false);

	m_transMan.Reset();

	// Clean up any active cool cameras
	while(!m_coolCameras.empty())
	{
		CoolCamera* pCoolCam = m_coolCameras.back();
		m_coolCameras.pop_back();
		NT_DELETE_CHUNK(Mem::MC_CAMERA, pCoolCam);
	}
	// And associated animator for maya cool cameras
	if (m_pCoolAnimator)
{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pCoolAnimator);
}

	// Clean up the active basic cameras
	while(!m_levelCameras.empty())
	{
		BasicCamera* pLevelCam = m_levelCameras.back();
		m_levelCameras.pop_back();
		NT_DELETE_CHUNK(Mem::MC_CAMERA, pLevelCam);
	}

	// Clean up the debug camera
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pDebugCamera);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::Init                                                                           
//!	Initialise the view.                                                                    
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamView::Init()
{
	m_elementMan.CalcPointOfInterest();

	// Create the Maya Camera Animator

	// Really need to be able to look in other anim containers too...
	const char* pAnimContainerName = "HeroCombatCamAnimContainer";
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( pAnimContainerName );
	if (pDO)
	{
		m_pCoolAnimator = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_MayaAnimator("entities/camera/thecamera.clump", pAnimContainerName);
		// ALEXEY_TODO
		m_pCoolAnimator->InstallGetName(NT_NEW_CHUNK( Mem::MC_CAMERA ) SpecificFunctor<CamView, ntstd::String, true>(this, &CamView::GetName));
	}
	else
		m_pCoolAnimator = 0;

	// Create the Debug Camera
	DebugCameraTemplate dct;
	m_pDebugCamera = NT_NEW_CHUNK( Mem::MC_CAMERA ) DebugChaseCamera(*this, dct);

	m_pDebugCamera->SetFOV(m_fDebugFOV);
	m_pDebugCamera->SetZoom(m_fDebugDist);
	m_debugControl.SetTransform(&m_transform);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::Update                                                                         
//!	Camera View per-frame update.                                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamView::Update()
{
	// Take care of pausing
	float fTimeDelta       = CTimer::Get().GetGameTimeChange();
	float fSystemTimeDelta = CTimer::Get().GetSystemTimeChange();

#ifndef _RELEASE
	// Enable/Disalbe topdown viewing
	if(CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_F6, KEYM_NONE))
	{
		if(m_eDebugMode == DM_TOPDOWN)
			SetDebugCameraMode(DM_NORMAL);
		else
			SetDebugCameraMode(DM_TOPDOWN);
	}
#endif

	//--------------------------------------------------------------------------------------------
	// Update the view on screen if we're transitioning
	//--------------------------------------------------------------------------------------------
	m_info.Update(fTimeDelta);

	//--------------------------------------------------------------------------------------------
	// Update our POI
	//--------------------------------------------------------------------------------------------
	CPoint ptPOI = m_elementMan.CalcPointOfInterest();

	//--------------------------------------------------------------------------------------------
	// Update Cool Cameras
	//--------------------------------------------------------------------------------------------
	ntstd::List<CoolCamera*, Mem::MC_CAMERA>::iterator itCoolCam = m_coolCameras.begin();
	while(itCoolCam != m_coolCameras.end())
	{
		CoolCamera* pCam = *itCoolCam;
		pCam->Update(fTimeDelta);

		// Life-cycle
		if(pCam->RemovingFromView())
		{
			// Remove the camera as soon as it isn't being used as part of a transition anymore.
			if(!m_transMan.IsUsing(pCam))
			{
				ntAssert(m_pCurrCamera != pCam);

				itCoolCam++;
				m_coolCameras.remove(pCam);
				NT_DELETE_CHUNK(Mem::MC_CAMERA, pCam);
				continue;
			}
		}
		else if(pCam->HasFinished())
		{
			// The camera has finished, apply any ending transitions and mark it as being removed.
			pCam->RemovingFromView(true);

			// Stop using the camera.
			if(m_pCurrCamera == pCam)
			{
				m_pCurrCamera = 0;

				// Activate the ending transition if we have one.
				if(!IsTransitionActive())
				{
					CamTransitionDef* pDef = pCam->GetEndingTransition();
					if(pDef)
					{
						ActivateEndingTransition(pCam, pDef);
						// Done with the definition, it'll be cleaned up automatically
						pCam->SetEndingTransition(0);
					}
				}
			}
		}
		else
		{
			// Swap to this cool camera if it's active and has a higher priority than our current camera
			if(pCam->IsActive() 
			    && (!m_pCurrCamera || !m_pCurrCamera->IsActive() ||pCam->GetPriority() > m_pCurrCamera->GetPriority())
			    && m_eDebugMode == DM_NORMAL)
			{
				m_pCurrCamera = pCam;
			}
		}

		// Next cool camera
		itCoolCam++;
	}

	//--------------------------------------------------------------------------------------------
	// If the current camera isn't good anymore choose an alternative.
	//--------------------------------------------------------------------------------------------
	if((!m_pCurrCamera || !m_pCurrCamera->IsActive()) || (m_pCurrCamera == m_pDebugCamera && m_eDebugMode == DM_NORMAL))
	{
		if(m_pCurrLevelCamera)
			m_pCurrCamera = m_pCurrLevelCamera;
		else
			m_pCurrCamera = m_pDebugCamera;
	}


	//--------------------------------------------------------------------------------------------
	// Update Level Cameras
	//--------------------------------------------------------------------------------------------
	BasicCamera* pCamToKill = 0;
	for(ntstd::List<BasicCamera*, Mem::MC_CAMERA>::iterator itLevelCam = m_levelCameras.begin(); 
		itLevelCam != m_levelCameras.end(); itLevelCam++)
	{
		BasicCamera* pBasicCam = *itLevelCam;

		// If we're now using a different basic camera and we're not still
		// transitioning out of this one then clean it up.
		if(pBasicCam != m_pCurrLevelCamera && !m_transMan.IsUsing(pBasicCam))
		{
			OSD::Add(OSD::CAMERA, DC_RED, "De-instancing level cam (%s)\n", ntStr::GetString(pBasicCam->GetCameraName()));
			pCamToKill = pBasicCam;
			continue;
		}

		// Update the game camera
		pBasicCam->Update(fSystemTimeDelta);
	}

	// Clean up any dead level camera.
	if(pCamToKill)
	{
		m_levelCameras.remove(pCamToKill);
		NT_DELETE_CHUNK(Mem::MC_CAMERA, pCamToKill);
		pCamToKill = 0;
	}


	//--------------------------------------------------------------------------------------------
	// Update the Debug Camera
	//--------------------------------------------------------------------------------------------
	UpdateDebugCamera(CTimer::Get().GetSystemTimeChange());


	//--------------------------------------------------------------------------------------------
	// All the necessary cameras are updated, there might be a transition going on though.
	//--------------------------------------------------------------------------------------------

	// By now we'd better have a valid camera.
	ntAssert(m_pCurrCamera);

	// Update any transitions and use the transition view if it has higher priority.
	if(m_transMan.Update(fSystemTimeDelta))
	{
		// There's a transition occuring, if it's going to the destination then use it.
		// Otherwise the entire transition tree can now be pruned.
		if(m_pCurrCamera == m_transMan.GetDestination())
			m_pViewCamera = m_transMan.GetCameraInterface();
		else
		{
			m_transMan.Reset();
			m_pViewCamera = m_pCurrCamera;
		}
	}
	else
	{
		m_pViewCamera = m_pCurrCamera;
	}

	//--------------------------------------------------------------------------------------------
	// Check the transform is valid...
	//--------------------------------------------------------------------------------------------
	if(m_pViewCamera->CheckTransformValid())
	{
		//--------------------------------------------------------------------------------------------
		// Set our views transform, FOV and DoF settings.
		//--------------------------------------------------------------------------------------------
		if(m_eDebugMode != DM_FREE)
			m_transform.SetLocalMatrix(m_pViewCamera->GetTransform());
		SetFOVAngle(m_pViewCamera->GetFOV() * DEG_TO_RAD_VALUE * m_info.GetWidth());

		// Only set the time scalar if we're the primary view.
		if(this == CamMan::Get().GetPrimaryView())
		CTimer::Get().SetCameraTimeScalar(m_pViewCamera->GetTimeScalar());
	}
	else
	{
		ntPrintf("Camera %s transform is invalid...\n", ntStr::GetString(m_pViewCamera->GetCameraName()));
	}

	// 1-2-1 mapping between camviews and pipview indexes
	PIPView& viewport = Renderer::Get().m_pPIPManager->GetView(m_info.GetID());

	// Apply Any Camera Shake
	UpdateShake();


	//--------------------------------------------------------------------------------------------
	// Apply Depth of Field.
	//--------------------------------------------------------------------------------------------
	if(m_pViewCamera->UsingDoF() && m_bUseDoF)
	{
		// Check the DOF settings are sensible...
#ifndef _RELEASE
		if(fabsf(m_pViewCamera->GetFocalDepth())    > 100000.f  || fabsf(m_pViewCamera->GetNearBlurDepth()) > 100000.f  ||
		   fabsf(m_pViewCamera->GetFarBlurDepth())  > 100000.f  || fabsf(m_pViewCamera->GetConfusionHigh()) > 100000.f ||
		   fabsf(m_pViewCamera->GetConfusionHigh())  > 100000.f)
		{
			one_time_assert_p(0xCA4E7A, true, ("Bad DOF settings - %s\n", ntStr::GetString(m_pViewCamera->GetCameraName())))
			viewport.m_DOFSettings.m_bApplyDepthOfField = false;
		}
		else
#endif //_RELEASE
		{
			viewport.m_DOFSettings.m_bApplyDepthOfField = true;
			viewport.m_DOFSettings.SetFocalPlaneDepth(m_pViewCamera->GetFocalDepth());
			viewport.m_DOFSettings.SetNearBlurDepth(m_pViewCamera->GetNearBlurDepth());
			viewport.m_DOFSettings.SetFarBlurDepth(m_pViewCamera->GetFarBlurDepth());
			viewport.m_DOFSettings.SetCircleMaxHiRez(m_pViewCamera->GetConfusionHigh());
			viewport.m_DOFSettings.SetCircleMaxLoRez(m_pViewCamera->GetConfusionLow());

			//ntPrintf("%s - %.2f, %.2f, %.2f\n", pViewCamera->GetCameraName(), pViewCamera->GetNearBlurDepth(), pViewCamera->GetFocalDepth(), pViewCamera->GetFarBlurDepth());
		}
	}
	else
		viewport.m_DOFSettings.m_bApplyDepthOfField = false;

	//--------------------------------------------------------------------------------------------
	// Apply Motion Blur
	//--------------------------------------------------------------------------------------------
#ifdef PLATFORM_PS3
	CSector::Get().GetMotionBlurObject().Enable(m_pViewCamera->m_bUseMotionBlur);
	CSector::Get().GetMotionBlurObject().SetMaskSize(m_pViewCamera->m_fMotionBlur);
#endif




#ifndef _GOLD_MASTER
	//--------------------------------------------------------------------------------------------
	// Render debug info and publish the camera transform to the havok debugger.
	//--------------------------------------------------------------------------------------------
	if(m_pCurrCamera == m_pDebugCamera || OSD::IsChannelEnabled(OSD::CAMERA))
		DebugRender();

	#ifdef CAMERA_DEBUG
		ntPrintf("%.2f, %.2f, %.2f -> %.2f, %.2f, %.2f FOV:%.2f TS:%.2f DoF:%.2f\n", 
		        m_transform.GetWorldMatrix().GetTranslation().X(), m_transform.GetWorldMatrix().GetTranslation().Y(), m_transform.GetWorldMatrix().GetTranslation().Z(),
				m_transform.GetWorldMatrix().GetZAxis().X(), m_transform.GetWorldMatrix().GetZAxis().Y(), m_transform.GetWorldMatrix().GetZAxis().Z(), 
				GetFOVAngle(), CTimer::Get().GetCameraTimeScalar(), viewport.m_DOFSettings.GetCircleMaxHiRez());

//		ntPrintf("[%.2f, %.2f, %.2f,%.2f], [%.2f, %.2f, %.2f,%.2f], [%.2f, %.2f, %.2f,%.2f], [%.2f, %.2f, %.2f,%.2f]\n",
//			m_transform.GetWorldMatrix().GetXAxis().X(),       m_transform.GetWorldMatrix().GetXAxis().Y(),       m_transform.GetWorldMatrix().GetXAxis().Z(),       m_transform.GetWorldMatrix().GetXAxis().W(),
//			m_transform.GetWorldMatrix().GetYAxis().X(),       m_transform.GetWorldMatrix().GetYAxis().Y(),       m_transform.GetWorldMatrix().GetYAxis().Z(),       m_transform.GetWorldMatrix().GetYAxis().W(),
//			m_transform.GetWorldMatrix().GetZAxis().X(),       m_transform.GetWorldMatrix().GetZAxis().Y(),       m_transform.GetWorldMatrix().GetZAxis().Z(),       m_transform.GetWorldMatrix().GetZAxis().W(),
//			m_transform.GetWorldMatrix().GetTranslation().X(), m_transform.GetWorldMatrix().GetTranslation().Y(), m_transform.GetWorldMatrix().GetTranslation().Z(), m_transform.GetWorldMatrix().GetTranslation().W());
	#endif

	if(OSD::IsChannelEnabled(OSD::CAMERA))
		m_elementMan.RenderDebugInfo();

	// Visualise the camera in the havok debugger if it's attached.
	Physics::HavokDebugDraw::ViewCameraInHKVDB(*m_pCurrCamera);
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::DestroyView
//!	Kill this view
//!
//------------------------------------------------------------------------------------------
void CamView::DestroyView()
{
	CamMan::Get().KillView(m_info.GetID());
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::SetActive
//!	Activate/Deactivate this view
//!
//------------------------------------------------------------------------------------------
void CamView::SetActive(bool bActive)
{
	m_bActive = bActive;
	PIPView& vp = Renderer::Get().m_pPIPManager->GetView(m_info.GetID());
	vp.SetActive(m_bActive);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::AddCoolCamera
//!	Add a new cool camera to the manager
//!
//------------------------------------------------------------------------------------------
void CamView::AddCoolCamera(CoolCamera* pCoolCamera, CamTransitionDef* pTransition)
{
	// Don't do cool cams in network mode, we don't want the time scalars
	if(NetMan::IsRunning())
		return;

	//ntAssert(m_pCurrCamera);

	// Add the Camera
	m_coolCameras.push_back(pCoolCamera);

	// And the transition into this camera if we have one...
	if(pTransition && m_pCurrCamera)
	{
		m_transMan.ActivateTransition(m_pCurrCamera, pCoolCamera, pTransition);
		//ntPrintf("************* TRANSITION ON\n");
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::RemoveCoolCamera
//!	Remove a cool camera with a given id
//!
//------------------------------------------------------------------------------------------
void CamView::RemoveCoolCamera(int iCamID)
{
	for(ntstd::List<CoolCamera*, Mem::MC_CAMERA>::iterator itCoolCam = m_coolCameras.begin(); 
		itCoolCam != m_coolCameras.end(); itCoolCam++)
	{
		if((*itCoolCam)->GetID() == iCamID)
		{
			(*itCoolCam)->EndCamera();
			return;
		}
	}

	ntPrintf("%s(%d): Could not find cool camera with id %d. \n", __FILE__, __LINE__, iCamID);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::RemoveAllCoolCameras
//!	Purge all the cool camers
//!
//------------------------------------------------------------------------------------------
void CamView::RemoveAllCoolCameras(bool bIncludingMayaCams)
{
	for(ntstd::List<CoolCamera*, Mem::MC_CAMERA>::iterator itCoolCam = m_coolCameras.begin(); itCoolCam != m_coolCameras.end(); itCoolCam++)
	{
		// Don't remove ns cameras or cutscenes...
		// People have to stop calling this function and use the specific remove function instead!
		if((*itCoolCam)->GetPriority() < 999 || bIncludingMayaCams)
			(*itCoolCam)->EndCamera();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::ActivateLevelCamera
//!	
//!
//------------------------------------------------------------------------------------------
void CamView::ActivateLevelCamera(BasicCameraTemplate* pCamTemplate)
{
	if(!pCamTemplate || (m_pCurrLevelCamera && m_pCurrLevelCamera->IsInstanceOf(pCamTemplate)))
		return;
	
	BasicCamera* pCam = pCamTemplate->Instance(*this);

	if(pCam)
	{
		// Set off a transition if there is one...
		const CamTransitionDef* pTransition;

		if(m_pCurrCamera && m_pCurrCamera->IsType(CT_BASIC))
			pTransition = (static_cast<BasicCamera*>(m_pCurrCamera))->GetTransitionTo(pCam);
		else
			pTransition = pCam->GetDefaultTransition();

		if(pTransition)
		{		
			m_transMan.ActivateTransition(m_pCurrCamera, pCam, pTransition);
		}
		else
		{
			// implicit transition - camera cut
			CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
			if (pobPlayer)
			{
				CInputComponent* pInputComponent = pobPlayer->GetInputComponent();
				// set active transition - slightly icky; could add a GetTransition member to the trans tree manager
				// would eliminate the casts: trans->cam interface->trans.
				pInputComponent->HandleImplicitTransition( pCam );
			}
		}

		// Add it to the level camera list and set it as the current level camera
		OSD::Add(OSD::CAMERA, DC_GREEN, "Instancing level cam (%s)\n", ntStr::GetString(pCam->GetCameraName()));
		m_levelCameras.push_back(pCam);
		m_pCurrLevelCamera = pCam;

		// If we're not in debug mode then make it the current active camera too.
		if(m_pCurrCamera != m_pDebugCamera)
			m_pCurrCamera = m_pCurrLevelCamera;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetCoolCam
//!	Return a pointer to the cool cam with the corresponding id number.
//!
//------------------------------------------------------------------------------------------
CoolCamera* CamView::GetCoolCam(int iID)
{
	for(ntstd::List<CoolCamera*, Mem::MC_CAMERA>::iterator itCoolCam = m_coolCameras.begin(); 
		itCoolCam != m_coolCameras.end(); itCoolCam++)
	{
		if((*itCoolCam)->GetID() == iID)
		{
			return (*itCoolCam);
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::FindCoolCam
//!	Find an active cool camera of a given type.
//!
//------------------------------------------------------------------------------------------
CoolCamera* CamView::FindCoolCam(CAMTYPE eType) const
{
	for(ntstd::List<CoolCamera*, Mem::MC_CAMERA>::const_iterator itCoolCam = m_coolCameras.begin(); 
		itCoolCam != m_coolCameras.end(); itCoolCam++)
	{
		if((*itCoolCam)->GetType() == eType)
		{
			return (*itCoolCam);
		}
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetNumCoolCams
//!	Reports number of cool cameras -
//!
//------------------------------------------------------------------------------------------
int	CamView::GetNumCoolCams() const
{
	return m_coolCameras.size();
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetLevelCamera
//!	Returns the currently selected level camera.
//!
//------------------------------------------------------------------------------------------
const CameraInterface* CamView::GetLevelCamera() const
{
	if(m_pCurrLevelCamera)
		return static_cast<BasicCamera*>(m_pCurrLevelCamera);

	// No level camera...
	return m_pDebugCamera;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::IsLevelCameraInUse
//!	Are we currently using a specified level camera
//!
//------------------------------------------------------------------------------------------
bool CamView::IsLevelCameraInUse(BasicCamera* pCam) const
{
	if(pCam == m_pCurrCamera || m_transMan.IsUsing(pCam))
		return true;

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::MoveView
//!	Reposition this view on the screen
//!
//------------------------------------------------------------------------------------------
void CamView::MoveView(float fTop, float fLeft, float fBottom, float fRight, float fTime)
{
	m_info.MoveView(fTop, fLeft, fBottom, fRight, fTime);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::SetPrimaryEntity
//!	Change the primary entity which this camera should focus upon
//! Note the DoF target entity can differ from the primary entity, e.g. we can be looking at
//! the heroine whilst DoF is set to focus on the commander on the balcony.
//!
//------------------------------------------------------------------------------------------
void CamView::SetPrimaryEntity(CEntity* pEnt) 
{
	m_info.SetPrimaryEntity(pEnt);
	m_pDoFEntity = pEnt;
	m_elementMan.SetPrimaryEntity(pEnt);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::ShakeView
//!	Shake this camera view
//!
//------------------------------------------------------------------------------------------
void CamView::ShakeView(const CamShake& shake)
{
	if(!m_info.AllowShakes())
		return;

	const CPoint& pt = GetCurrMatrix().GetTranslation();

	if(shake.GetRelativeShake(pt) > m_shake.GetRelativeShake(pt))
	{
		m_shake = shake;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::IsTransitionActive
//!	Is a transition currently active?
//!
//------------------------------------------------------------------------------------------
bool CamView::IsTransitionActive()
{
	return m_transMan.IsTransitionActive();
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetDebugCamera
//!	Return a pointer to the debug camera
//!
//------------------------------------------------------------------------------------------
DebugChaseCamera* CamView::GetDebugCamera()
{
	if(!m_pDebugCamera)
		Init();

	return static_cast<DebugChaseCamera*>(m_pDebugCamera);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::UpdateDebugCamera
//!	Per-frame debug camera update
//!
//------------------------------------------------------------------------------------------
void CamView::UpdateDebugCamera(float fTimeDelta)
{
	m_pDebugCamera->Update(fTimeDelta);

	if(m_eDebugMode == DM_FREE)
	{
		if(CInputHardware::Get().GetPadContext() == PAD_CONTEXT_DEBUG)
		{
			float fTimeDelta = (CTimer::Get().GetGameTimeScalar() >= 1.0f) ?
				CTimer::Get().GetGameTimeChange() :	CTimer::Get().GetSystemTimeChange();

			m_debugControl.DebugUpdateFromPad(CamMan::Get().GetDebugCameraPadNumber(),
                                              fTimeDelta*m_fDebugCameraSpeed,
                                              fTimeDelta*PI/2.0f);

			float fFov = m_pDebugCamera->GetFOV();
			if (CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_TOP_1)
			{
				fFov -= 1.0f;
				fFov = max(fFov, 2.0f);
			}

			if (CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_TOP_2)
			{
				fFov += 1.0f;
				fFov = min(fFov, 89.0f);
			}
			m_pDebugCamera->SetFOV(fFov);

			if(CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_FACE_3)
			{
				// Set players position when you hit the X-button on the controller (PAD_FACE_3 is X, apparently).
				CDirection obDir	= GetDebugControllerMatrix().GetZAxis() * 100.0f;
				CPoint obRayStart	= GetDebugControllerMatrix().GetTranslation();
				float		fHitFraction;
				CDirection	HitNormal;

				Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
				obFlag.flags.i_collide_with = (Physics::LARGE_INTERACTABLE_BIT);

				if(Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails(obRayStart, obRayStart + obDir, fHitFraction, HitNormal, obFlag))
				{
					CPoint obHit = obRayStart + (obDir * fHitFraction);
#ifndef _GOLD_MASTER
					g_VisualDebug->RenderPoint(obHit, 3.0f, 0xffffffff, 0);
					g_VisualDebug->Printf3D(obHit, 0xffffffff, 0, "%.2f %.2f %.2f", obHit.X(), obHit.Y(), obHit.Z());
#endif					
					CEntityManager::Get().GetPlayer()->SetPosition(obHit);
				}
			}

#ifndef _GOLD_MASTER
			if(CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_FACE_2)
			{
				CPoint obRayStart	= GetDebugControllerMatrix().GetTranslation();
					
				g_VisualDebug->Printf2D(50.0f,50.0f, 0xffffffff, 0, "%.2f %.2f %.2f", obRayStart.X(), obRayStart.Y(), obRayStart.Z());
			}
#endif					
		}
	}
	else
	{
		static float l_fHeldTime = 0.f;
		float fZoomSpeed = clamp(l_fHeldTime*4.f, 2.f, 40.f);

		float fTimeDelta = CTimer::Get().GetGameTimeChange();
		if(CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_UP)
		{
			((DebugChaseCamera*)m_pDebugCamera)->Zoom(-fTimeDelta * fZoomSpeed);
			l_fHeldTime += fTimeDelta;
		}
		else if(CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_DOWN)
		{
			((DebugChaseCamera*)m_pDebugCamera)->Zoom(fTimeDelta * fZoomSpeed);
			l_fHeldTime += fTimeDelta;
		}
		else
			l_fHeldTime = 0.f;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CamView::SwitchDebugCameraStyle
//!	Changes debug free cam sytle.
//!
//------------------------------------------------------------------------------------------
void CamView::SwitchDebugCameraStyle()
{
	if(m_eDebugMode == DM_FREE)
	{
		m_debugControl.AdvanceDebugUpdateMode();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::SwitchDebugCameraMode
//!	Toggle the camera mode
//!
//------------------------------------------------------------------------------------------
void CamView::SwitchDebugCameraMode()
{
	SetDebugCameraMode(eDEBUG_MODE((m_eDebugMode + 1) % (DM_REL+1)));
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::SetDebugCameraMode
//!	Set the Debug Camera Mode
//!
//------------------------------------------------------------------------------------------
void CamView::SetDebugCameraMode(eDEBUG_MODE eMode)
{
	m_eDebugMode = eMode;
	
	if(m_eDebugMode == DM_NORMAL)
	{
		OSD::Add(OSD::CAMERA, DC_WHITE, "Game Camera Selected");
		m_pDebugCamera->SetTopDownMode(false);
		m_pCurrCamera = m_pCurrLevelCamera;
		m_pDebugCamera->Reset();
	}
	else if(m_eDebugMode == DM_FREE)
	{
		OSD::Add(OSD::CAMERA, DC_WHITE, "Debug Free Camera Selected");
		m_pDebugCamera->SetTopDownMode(false);
		m_debugControl.ResetController(GetCurrMatrix());
	    CInputHardware::Get().SetContext(INPUT_CONTEXT_CAMERA_DEBUG);
		m_pCurrCamera = m_pDebugCamera;
	}
	else if(m_eDebugMode == DM_REL)
	{
		OSD::Add(OSD::CAMERA, DC_WHITE, "Debug Chase Camera Selected");
		m_pDebugCamera->SetTopDownMode(false);
		m_pCurrCamera = m_pDebugCamera;
	}
	else
	{
		OSD::Add(OSD::CAMERA, DC_WHITE, "Debug Top-Down Camera Selected");
		m_pDebugCamera->SetTopDownMode(true);
		m_pCurrCamera = m_pDebugCamera;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::SetDebugCameraPlacement
//!	Place the debug free camera...
//!
//------------------------------------------------------------------------------------------
void CamView::SetDebugCameraPlacement(const CPoint& ptPos, const CPoint& ptTarg)
{
	CMatrix mat;
	CCamUtil::CreateFromPoints(mat, ptPos, ptTarg);

	m_eDebugMode = DM_FREE;
	
	OSD::Add(OSD::CAMERA, DC_WHITE, "Debug Free Camera Selected");
	m_debugControl.ResetController(mat);
	CInputHardware::Get().SetContext(INPUT_CONTEXT_CAMERA_DEBUG);
	m_pCurrCamera = m_pDebugCamera;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetTimeScalar
//!	Return the current time scalar specified by the current camera
//!
//------------------------------------------------------------------------------------------
float CamView::GetTimeScalar() const
{
	return m_pCurrCamera ? m_pCurrCamera->GetTimeScalar() : 1.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::GetPriority
//!	Return the priority of the currently active camera
//!
//------------------------------------------------------------------------------------------
int CamView::GetPriority() const
{
	return m_pCurrCamera ? m_pCurrCamera->GetPriority() : 1;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateThrownCoolCam
//!	Activate a thrown cool camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------

int CamView::ActivateAfterTouchCoolCam(CEntity* pentTarget, CHashedString pcAftertouchDef)
{
	if(!pentTarget || !pentTarget->GetPhysicsSystem())
	{
		lua_bind_warn_msg(("Null object for aftertouch camera or object lacks a physics system.\n"));
		return 0;
	}

	CoolCam_AfterTouchDef* pobDefinition = ObjectDatabase::Get().GetPointerFromName< CoolCam_AfterTouchDef* >(pcAftertouchDef);

	if(!pobDefinition)
	{
		lua_bind_warn_msg(("Aftertouch object lacks a camera definition.\n"));
		return 0;
	}

	return ActivateAfterTouchCoolCamDef(pentTarget,  pobDefinition);
}

int CamView::ActivateAfterTouchCoolCamDef(CEntity* pentTarget, CoolCam_AfterTouchDef* pobDefinition)
{
	if(!pentTarget || !pentTarget->GetPhysicsSystem())
	{
		lua_bind_warn_msg(("Null object for aftertouch camera or object lacks a physics system.\n"));
		return 0;
	}

	if(!pobDefinition)
	{
		lua_bind_warn_msg(("Aftertouch object lacks a camera definition.\n"));
		return 0;
	}

	CDirection direction;
	direction = pentTarget->GetPhysicsSystem()->GetLinearVelocity();
	direction.Y() =  0.0f;
	direction.Normalise();
	direction.Y() = -0.01f;

	// Create a cool camera
	CoolCam_AfterTouch* pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_AfterTouch(*this, pobDefinition, pentTarget, direction);

	// Lerp into the camera if we've come from an aiming or chasing camera.
	// Do a POI Rotation if we've come from a game camera.
	CoolCamera* pCurrentCam = GetCoolCam(GetActiveCameraID());
	UNUSED(pCurrentCam);

	/*
	if(pCurrentCam && (pCurrentCam->GetType() == CT_AIM || pCurrentCam->GetType() == CT_CHASE || pCurrentCam->GetType() == CT_CHASEAIMCOMBO))
	{
	*/
		// Add the camera to the view
		CamTrans_LerpDef def(1.f, 0.0f);
		AddCoolCamera(pCam, &def);
	/*
	}
	else
	{
		// Add the camera to the view
		CamTrans_POIRotDef def(1.f);
		AddCoolCamera(pCam, &def);
	}*/

	OSD::Add(OSD::CAMERA, DC_WHITE, "+++ Activated Thrown Cam ID%d.\n",pCam->GetID());

	// Return our new cameras id to lua
	return pCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::AfterTouchCoolCamLookAt
//!	Tweak the thrown cool camera to look at a struck entity.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamView::AfterTouchCoolCamLookAt(CEntity* pLookAtEnt, int iHitType)
{
	ntAssert(pLookAtEnt);

	// Should change this so that you pass in the camera id...
	CoolCam_AfterTouch* pCam = static_cast<CoolCam_AfterTouch*>(FindCoolCam(CT_AFTERTOUCH));

	if(pCam)
		pCam->LookAt(pLookAtEnt, CoolCam_AfterTouch::HIT_TYPE(iHitType));
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateRotatingCamera
//!	Activate a rotating cool camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
int	CamView::ActivateRotatingCamera(CEntity* pEnt1, CEntity* pEnt2, float fTime, float fAngle)
{
	CoolCam_Generic* pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Generic(*this);
	
	pCam->Init(fTime, pEnt1, pEnt2, fAngle * DEG_TO_RAD_VALUE);

	AddCoolCamera(pCam);
	ntPrintf("### Activated Rotating Cam ID%d.\n",pCam->GetID());
	return pCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateChaseAimCam
//!	Activate a chase/aim camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateChaseAimCam(CEntity* pEnt, CHashedString pcChaseDef, CHashedString pcAimDef)
{
	CoolCam_ChaseDef* pChaseDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_ChaseDef*>(pcChaseDef);
	CoolCam_AimDef*   pAimDef   = ObjectDatabase::Get().GetPointerFromName<CoolCam_AimDef*>(pcAimDef);

	ntAssert(pEnt);
	ntAssert(pChaseDef);
	ntAssert(pAimDef);
	
	return ActivateChaseAimCamDef(pEnt, pChaseDef, pAimDef);
}

int CamView::ActivateChaseAimCamDef(CEntity* pEnt, CoolCam_ChaseDef* pChaseDef, CoolCam_AimDef* pAimDef)
{
	ntAssert(pEnt);
	ntAssert(pChaseDef);
	ntAssert(pAimDef);
	
	CoolCam_ChaseAimCombo *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_ChaseAimCombo(*this, *pEnt, *pChaseDef, *pAimDef);
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::AddFixedEndTrans
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CamView::AddFixedEndTrans(int iCamID, float fTransTime )
{
	for(ntstd::List<CoolCamera*, Mem::MC_CAMERA>::iterator itCoolCam = m_coolCameras.begin(); 
		itCoolCam != m_coolCameras.end(); itCoolCam++)
	{
		CoolCamera* pCoolCam = (*itCoolCam);

		if(pCoolCam->GetID() == iCamID)
		{
			CamTrans_POIRotDef* pNewCameraTransition = NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_POIRotDef(fTransTime);
			pCoolCam->SetEndingTransition( pNewCameraTransition );

			return true;
		}
	}
	
	return false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateChaseCam
//!	Activate a chase camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateChaseCam(CEntity* pEnt, const char* pcChaseDef)
{
	CoolCam_ChaseDef* pChaseDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_ChaseDef*>(pcChaseDef);

	ntAssert(pEnt);
	ntAssert(pChaseDef);
	
	CoolCam_Chase *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Chase(*this, *pEnt, *pChaseDef);
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateAimCam
//!	Activate an aiming camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateAimCam(CEntity* pEnt, const char* pcAimDef)
{
	CoolCam_AimDef* pAimDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AimDef*>(pcAimDef);

	ntAssert(pEnt);
	ntAssert(pAimDef);
	
	CoolCam_Aim *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Aim(*this, *pEnt, *pAimDef);
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMayaCam
//!	Activate an animated maya camera on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMayaCam(const char* pcAnim, CEntity* pInformEntity, CEntity* pParentEntity)
{
	// we have no resources, so the maya cam cannot be activated.
	if (m_pCoolAnimator == 0)
	{
		user_error_p( 0, ("Activating a maya cam without loaded resources. Make sure globalscripts.lua is loaded in your manifest") );
		return -1;
	}

	// This needs a better solution, but probably deprecating this soon...
	CHashedString anim_name(pcAnim);
	CoolCam_MayaDef CoolCamDef;
	CoolCamDef.pCoolAnimator = m_pCoolAnimator;
	CoolCamDef.sAnim = anim_name;

	CoolCam_Maya *pCoolCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Maya(*this, CoolCamDef);
	pCoolCam->SetAutoFinish(true);
	//pCoolCam->Init(CHashedString(pcAnim));

	// Add the cool camera to the manager.
	if(pInformEntity)
	{
		pCoolCam->SetEventHandler(pInformEntity);
	}

	// If we've got an entity pointer then set the camera to start aligned with that entity.
	if(pParentEntity)
	{
		pCoolCam->SetMatrix(pParentEntity->GetRootTransformP()->GetWorldMatrix());
	}

	// Add the camera to this view
	AddCoolCamera(pCoolCam);

	// Return the ID of the camera
	return pCoolCam->GetID();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateTurretCamera(string transform, const CDirection& dirCamOffest)
//!	Activate a turret camera, for ballistas, cannons etc.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateTurretCamera(CEntity* pEnt, CHashedString pszTransform, const CDirection& dirCamOffest)
{
	// Check everythings valid
	ntAssert(pEnt);

	// Create the camera
	CoolCam_Turret *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Turret(*this);;
	pCam->Init(pEnt, pszTransform, dirCamOffest);
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateBossCamera(entity Player, entity Boss)
//!	Activate a boss camera, for boss battles.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateBossCamera(CEntity* pobPlayer, CEntity* pobBoss)
{
	// Check everythings valid
	ntError_p(pobPlayer, ("Attempting to activate boss camera with an invalid player-entity pointer"));
	ntError_p(pobBoss, ("Attempting to activate boss camera with an invalid boss-entity pointer"));


	// Create the camera
	CoolCam_Boss *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Boss(*this,pobPlayer,pobBoss);
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateGladiatorGeneralCamera(entity Player, entity Boss)
//!	Activate a boss camera, specific for the battle with the Gladiator General in this case.
//!                                                                                         
//------------------------------------------------------------------------------------------
//int CamView::ActivateGladiatorGeneralCamera(CEntity* pobBoss)
//{
//	// Check everythings valid
//	ntAssert(pobBoss);
//
//	// Create the camera
//	CoolCam_GladiatorGeneral *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_GladiatorGeneral(*this,pobBoss);
//	AddCoolCamera(pCam);
//	
//	return pCam->GetID();
//}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMappedGladiatorGeneralCamera( const char* pcCamDefName )
//!	Activate a boss camera, specific for the battle with the Gladiator General in this case.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMappedGladiatorGeneralCamera( const char* pcCamDefName )
{
	CoolCam_GladiatorGeneralDef* pGladGenCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_GladiatorGeneralDef*>(pcCamDefName);
	ntError_p(pGladGenCamDef, ("Attempting to activate gladiator general camera [%s] which doesn't exist", pcCamDefName));

	// Create the camera
	CoolCam_GladiatorGeneral *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_GladiatorGeneral( *this, pGladGenCamDef );
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMappedKingBossFightCamera( const char* pcCamDefName )
//!	Activate a boss camera, specific for the final battle with the King.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMappedKingBossFightCamera( const char* pcCamDefName )
{
	CoolCam_KingBossFightDef* pKingBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_KingBossFightDef*>(pcCamDefName);
	ntError_p(pKingBossCamDef, ("Attempting to activate king boss camera [%s] which doesn't exist", pcCamDefName));


	// Create the camera
	CoolCam_KingBossFight *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_KingBossFight( *this, pKingBossCamDef );
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMappedAerialGeneralCamera( const char* pcCamDefName )
//!	Activate a boss camera, specific for the aerial general boss fight.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMappedAerialGeneralCamera( const char* pcCamDefName )
{
	CoolCam_AerialGeneralDef* pAerialGenCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pAerialGenCamDef, ("Attempting to activate aerial general camera [%s] which doesn't exist", pcCamDefName));


	// Create the camera
	CoolCam_AerialGeneral *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_AerialGeneral( *this, pAerialGenCamDef );
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMappedWaterGeneralCamera( const char* pcCamDefName )
//!	Activate a boss camera, specific for the fight with the water general.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMappedWaterGeneralCamera( const char* pcCamDefName )
{
	CoolCam_WaterGeneralDef* pWaterGenCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_WaterGeneralDef*>(pcCamDefName);
	ntError_p(pWaterGenCamDef, ("Attempting to activate water general camera [%s] which doesn't exist", pcCamDefName));


	// Create the camera
	CoolCam_WaterGeneral*pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_WaterGeneral( *this, pWaterGenCamDef );
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateMappedKingBossFightCamerav2( const char* pcCamDefName )
//!	Activate a boss camera, specific for the final battle with the King - version 2.
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamView::ActivateMappedKingBossFightCamerav2( const char* pcCamDefName )
{
	CoolCam_KingBossFightv2Def* pKingBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_KingBossFightv2Def*>(pcCamDefName);
	ntError_p(pKingBossCamDef, ("Attempting to activate king boss camera [%s] which doesn't exist", pcCamDefName));


	// Create the camera
	CoolCam_KingBossFightv2 *pCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_KingBossFightv2( *this, pKingBossCamDef );
	AddCoolCamera(pCam);
	
	return pCam->GetID();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateBossCameraReverseAngle( const char* pcCamDefName )
//!	Activate reverse camera angle for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::ActivateBossCameraReverseAngle( const char* pcCamDefName )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to activate reverse angle mode on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->ActivateReverseAngleMode();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateBossCameraReverseAngle( const char* pcCamDefName )
//!	Activate reverse camera angle for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::DeactivateBossCameraReverseAngle( const char* pcCamDefName )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to deactivate reverse angle mode on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->DeactivateReverseAngleMode();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateBossCameraReverseAngle( const char* pcCamDefName )
//!	Activate reverse camera angle for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::ActivateReverseAngleLeadIn( const char* pcCamDefName )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to activate reverse angle mode on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->ReverseAngleLeadInOn();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateBossCameraReverseAngle( const char* pcCamDefName )
//!	Activate reverse camera angle for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::DeactivateReverseAngleLeadIn( const char* pcCamDefName )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to deactivate reverse angle mode on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->ReverseAngleLeadInOff();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::SetBossCameraFOV( const char* pcCamDefName, float fFOV )
//!	Set the Field of View for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::SetBossCameraFOV( const char* pcCamDefName, float fFOV )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to set FOV on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->SetFOV( fFOV );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::SetBossCameraFOVBlendTime( const char* pcCamDefName, float fFOVBlendTime )
//!	Set the Field of View blend time for a boss camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::SetBossCameraFOVBlendTime( const char* pcCamDefName, float fFOVBlendTime )
{
	CoolCam_RootBossDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_RootBossDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to set FOV on boss camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->SetFOVBlendTime( fFOVBlendTime );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateAerialGeneralOrbitMode( const char* pcCamDefName )
//!	Activate the Orbit mode of the aerial general boss camera. The DefName must be for an aerial
//!	general camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::ActivateAerialGeneralOrbitMode( const char* pcCamDefName )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to activate orbit mode on aerial general camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->ActivateOrbitMode();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::DeactivateAerialGeneralOrbitMode( const char* pcCamDefName )
//!	Deactivate the Orbit mode of the aerial general boss camera. The DefName must be for an aerial
//!	general camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::DeactivateAerialGeneralOrbitMode( const char* pcCamDefName )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to deactivate orbit mode on aerial general camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->DeactivateOrbitMode();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ActivateAerialGeneralOrbitAcceleration( const char* pcCamDefName )
//!	Activate acceleration for the Orbit mode of the aerial general boss camera. The DefName must be for an aerial
//!	general camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::ActivateAerialGeneralOrbitAcceleration( const char* pcCamDefName )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to activate acceleration on aerial general orbit camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->StartAccelerating();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::DeactivateAerialGeneralOrbitAcceleration( const char* pcCamDefName )
//!	Deactivate acceleration for the Orbit mode of the aerial general boss camera. The DefName must be for an aerial
//!	general camera.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::DeactivateAerialGeneralOrbitAcceleration( const char* pcCamDefName )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to deactivate acceleration on aerial general orbit camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->StopAccelerating();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::SetAerialGeneralOrbitAcceleration( const char* pcCamDefName, float fAcceleration )
//!	Set the acceleration for the Orbit mode of the aerial general boss camera. The DefName must be for 
//!	an aerial general camera. The Acceleration is in Degrees seconds ^-2.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::SetAerialGeneralOrbitAcceleration( const char* pcCamDefName, float fAcceleration )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to set acceleration on aerial general orbit camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->SetAngularAcceleration( fAcceleration );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::SetAerialGeneralOrbitMaxSpeed( const char* pcCamDefName, float fMaxSpeed )
//!	Set the maximum angular speed for the Orbit mode of the aerial general boss camera. The DefName 
//!	must be for an aerial general camera. The Acceleration speed is in Degrees per second.
//!                                                                                         
//------------------------------------------------------------------------------------------

void CamView::SetAerialGeneralOrbitMaxSpeed( const char* pcCamDefName, float fMaxSpeed )
{
	CoolCam_AerialGeneralDef* pBossCamDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_AerialGeneralDef*>(pcCamDefName);
	ntError_p(pBossCamDef, ("Attempting to set max anuglar speed on aerial general orbit camera [%s] which doesn't exist", pcCamDefName));

	pBossCamDef->SetMaxAngularSpeed( fMaxSpeed );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::ShakeView
//!	Shake the view.  At the moment the shake is only applied to basic level cameras.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamView::UpdateShake()
{
	if(!m_pCurrCamera->AllowShakes() || m_shake.m_fTime <= 0.f)
	{
		return;
	}

	float fShake = m_shake.GetRelativeShake(GetCurrMatrix().GetTranslation());
	float fPitch = fShake * fsinf(TWO_PI * m_shake.m_fTime * m_shake.m_fFrequency);
	float fYaw   = fShake * fsinf(TWO_PI * m_shake.m_fTime * m_shake.m_fFrequency * 0.8f);

	CMatrix mat = m_transform.GetLocalMatrix();
	CMatrix obRotX(mat.GetXAxis(), fPitch); 
	CMatrix obRotY(mat.GetYAxis(), fYaw);

	mat.SetZAxis(mat.GetZAxis() * obRotX);
	mat.SetXAxis(mat.GetXAxis() * obRotY);
	mat.BuildYAxis();

	m_transform.SetLocalMatrix(mat);

	m_shake.m_fTime -= CTimer::Get().GetGameTimeChange();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamView::SetDoF
//!	Set the default Depth of Field for all cameras on this view.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamView::SetDoF(const CEntity* pEntFocal, float fFar, float fNear, float fConfusionHigh, float fConfusionLow, float fTransitionTime)
{
	m_pDoFEntity     = pEntFocal;
	m_fDoFFar        = fFar;
	m_fDoFNear       = fNear;
	m_fConfusionHigh = fConfusionHigh;
	m_fConfusionLow  = fConfusionLow;

	if(m_pCurrLevelCamera)
		m_pCurrLevelCamera->SetDoF(pEntFocal, fFar, fNear, fTransitionTime);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::ActivateTransition
//!	Activate a specified transition between two cameras.
//!
//------------------------------------------------------------------------------------------
void CamView::ActivateTransition(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTransitionDef* pDef)
{
	if(!pSrc)
	{
		ntAssert(m_pCurrCamera);
		pSrc = m_pCurrCamera;
	}

	if(!pDst)
	{
		ntAssert(m_pCurrCamera);
		pDst = m_pCurrCamera;
	}

	m_transMan.ActivateTransition(pSrc, pDst, pDef);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::ActivateEndingTransition
//!	Activate a specified transition between two cameras.
//!
//------------------------------------------------------------------------------------------
void CamView::ActivateEndingTransition(CoolCamera* pSrc, CamTransitionDef* pDef)
{
	// Change this to consider cool cameras that might become active too?
	const CameraInterface* pDst = GetLevelCamera();

	ActivateTransition(pSrc, pDst, pDef);
}


//------------------------------------------------------------------------------------------
//!
//!	CamView::DebugRender
//!	Render some debugging information.
//!
//------------------------------------------------------------------------------------------
#ifndef _GOLD_MASTER
void CamView::DebugRender()
{
	if(m_transMan.IsTransitionActive())
	{
		g_VisualDebug->Printf2D(50.0f, 350.0f, DC_WHITE, 0, "%s", 
			                         ntStr::GetString(m_transMan.GetCameraInterface()->GetCameraName()));
	}
	else
	{
		static const char* s_pszDebugModes[DM_TOPDOWN+1] = 
		{
			"", 
			"(Free)", 
			"(Chase)", 
			"(Top-Down)"
		};

		g_VisualDebug->Printf2D(50.0f, 350.0f, DC_WHITE, 0, "%s %s %s", ntStr::GetString(m_pCurrCamera->GetCameraName()), 
																		s_pszDebugModes[m_eDebugMode],
																		CamMan::Get().IsCombatAware() ? "*CombatCam*" : "");
	}

	if(OSD::IsChannelEnabled(OSD::CAMERA))
		m_elementMan.RenderDebugInfo();
}
#endif
