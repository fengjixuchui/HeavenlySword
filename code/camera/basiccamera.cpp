//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file basiccamera.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/basiccamera.h"

#include "camera/motioncontroller.h"
#include "camera/lookatcontroller.h"
#include "camera/lenscontroller.h"
#include "camera/combatcam.h"
#include "camera/camvolumes.h"
#include "camera/matrixtweakereditor.h"
#include "camera/camtransition.h"

#include "objectdatabase/dataobject.h"
#include "gui/guimanager.h"
#include "game/entity.h"
#include "game/entity.inl"

// Included for activating cameras - TEMP - GH
#include "camera/camman.h"
#include "camera/camview.h"

#include "core/visualdebugger.h"


//------------------------------------------------------------------------------------------
// Debug Includes
//------------------------------------------------------------------------------------------
#ifndef _RELEASE
#include "gfx/renderer.h"
#include "camera/camutils.h"
#endif

//------------------------------------------------------------------------------------------
// Basic Camera Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( BasicCameraTemplate, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pMotionControllerDef, MotionController)
	PUBLISH_PTR_AS(m_pLookAtControllerDef, LookAtController)
	PUBLISH_PTR_AS(m_pLensControllerDef, LensController)
	PUBLISH_PTR_AS(m_pCombatCamDef, CombatCamDef)
	PUBLISH_PTR_AS(m_pTweaker, Tweaker)

	PUBLISH_PTR_AS(m_pParentEntity, ParentEntity)

	PUBLISH_PTR_AS(m_pDoFEnt, DoF_Entity)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDoFFarDist,  -1.f, DoF_Far)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDoFNearDist, -1.f, DoF_Near)

	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(PostPostConstruct)
	DECLARE_CSTRUCTURE_PTR_CONTAINER(m_transitionList, Transitions)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::BasicCameraTemplate
//!	Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
BasicCameraTemplate::BasicCameraTemplate() :
	m_pMotionControllerDef(0),
	m_pLookAtControllerDef(0),
	m_pLensControllerDef(0),
	m_pTweaker(0),
	m_pCombatCamDef(0),
	m_pParentEntity(0),
	m_bDebugCam(false)
{
	ATTACH_LUA_INTERFACE(BasicCameraTemplate);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::PostConstruct
//!	Postconstruction, add the camera to the camera manager.   
//!                                                                                         
//------------------------------------------------------------------------------------------
void BasicCameraTemplate::PostConstruct()
{
	CamMan::Get().AddLevelCameraTemplate(this);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	scee.sbashow -- used to test for construct order.
//!                                                                                          
//------------------------------------------------------------------------------------------
void BasicCameraTemplate::PostPostConstruct()
{
//	ntPrintf("Say Hello to my little friend %s!\n", 
//			 this->GetCameraName().GetDebugString());
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::GetCameraName
//!	Get the name of the camera.   
//!                                                                                         
//------------------------------------------------------------------------------------------
CHashedString BasicCameraTemplate::GetCameraName() const
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);

	if(pDO)
		return CHashedString(pDO->GetName());
	else
		return CHashedString(HASH_STRING_ANON_CAM);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::Instance
//!	Instance a basic camera from this template.
//!                                                                                         
//------------------------------------------------------------------------------------------
BasicCamera* BasicCameraTemplate::Instance(const CamView& view)
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) BasicCamera(*this, view);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::Activate                                                            
//!	Directly activate this camera - this should be temporary, need to look at this contruct    
//!                                                                                         
//------------------------------------------------------------------------------------------
void BasicCameraTemplate::Activate()
{
	// Try and get the primary camera view
	CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();

	// If that was successful then activate this camera
	if (pobPrimaryCameraView)
		pobPrimaryCameraView->ActivateLevelCamera(this);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::GetTransitionTo                                                            
//!	See if there is a transition listed for going to the specified camera.                  
//!                                                                                         
//------------------------------------------------------------------------------------------
const CamTransitionDef* BasicCameraTemplate::GetTransitionTo(const BasicCameraTemplate* pTo) const
{
	ntAssert(pTo);

	for(ntstd::List<CamTransitionDef*,Mem::MC_CAMERA>::const_iterator itLink = m_transitionList.begin(); 
		itLink != m_transitionList.end(); itLink++)
	{
		if(!*itLink)
		{
			ntPrintf("Null link in %s Transitions list\n", ntStr::GetString(GetCameraName()));
		}

		if(*itLink && pTo == (*itLink)->GetDestination())
		{
			return *itLink;
		}
	}

	// Couldn't find a transition.
#ifndef _RELEASE
	ntPrintf("Could not find a transition from %s to %s\n", ntStr::GetString(GetCameraName()), ntStr::GetString(pTo->GetCameraName()));
#endif

	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCameraTemplate::GetDefaultTransition                                                       
//!	Return any default (entrance) transition for this camera.                               
//!                                                                                         
//------------------------------------------------------------------------------------------
const CamTransitionDef* BasicCameraTemplate::GetDefaultTransition() const
{
	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCamera::BasicCamera                                                                
//!	Construction                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
BasicCamera::BasicCamera(const BasicCameraTemplate& bct, const CamView& view) : 
	CameraInterface(view),
	m_template(bct),
	m_pMotionController(0),
	m_pLookAtController(0),
	m_pLensController(0),
	m_pTweaker(0),
	m_pCombatCamDef(0),
	m_pCombatCam(0),
	m_pDoFEnt(0),
	m_fDoFFar(0.f),
	m_fDoFNear(0.f),
	m_pOldDoFEnt(0),
	m_fOldDoFFar(0.f),
	m_fOldDoFNear(0.f),
	m_fTransitionTime(0.f),
	m_fTotalTransitionTime(0.f),
	m_fTweakTime(0.f),
	m_pobScratchTransition(0)
{
	if(bct.IsDebug())
		return;

	ntError_p(bct.m_pMotionControllerDef && bct.m_pLookAtControllerDef && bct.m_pLensControllerDef, ("%s could not be constructed due to missing one or more controllers.\n", ntStr::GetString(bct.GetCameraName())));

	// Create all the components from the template definitions
	m_pMotionController   = bct.m_pMotionControllerDef->Create(this, bct.m_pParentEntity);
	m_pLookAtController   = bct.m_pLookAtControllerDef->Create(this);
	m_pLensController     = bct.m_pLensControllerDef->Create(this);
	m_pCombatCamDef	      = bct.m_pCombatCamDef;
	if(m_pCombatCamDef)
		m_pCombatCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CombatCam(this, *m_pCombatCamDef);
	m_pTweaker		      = bct.m_pTweaker;
	ntAssert_p(m_pMotionController && m_pLookAtController && m_pLensController, ("%s could not be constructed due to missing one or more controllers.\n", ntStr::GetString(bct.GetCameraName())));

	m_pDoFEnt    = 0; //bct.m_pDoFEnt ? bct.m_pDoFEnt : m_view.GetDoFEntity();
	m_fDoFFar    = bct.m_fDoFFarDist > 0.f ? bct.m_fDoFFarDist : m_view.GetDofFar();
	m_fDoFNear   = bct.m_fDoFNearDist > 0.f ? bct.m_fDoFNearDist : m_view.GetDoFNear();

	Reset();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCamera::~BasicCamera                                                               
//!	Destruction                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
BasicCamera::~BasicCamera()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pMotionController);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pLookAtController);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pLensController);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pCombatCam);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCamera::Update                                                                     
//!	The per frame update                                                                    
//!                                                                                         
//------------------------------------------------------------------------------------------
void BasicCamera::Update(float fTimeDelta)
{
	if(!m_pMotionController || !m_pLookAtController || !m_pLensController)
		return;

	// Check for automatic activation of combat cameras...
	if(m_pCombatCam && m_pCombatCam->AutoActivate())
	{
		CamMan::SetCombatAware(GetElementManager().CountEnemiesInfluencing() > 0);
	}


	CPoint	obCurrPos =	m_pMotionController->Update(fTimeDelta);
	m_obTransform	  = m_pLookAtController->Update(obCurrPos, fTimeDelta);
	m_fFOV			  = m_pLensController->Update(m_pMotionController, m_pLookAtController, fTimeDelta);

	if(m_pTweaker && !m_view.IsCoolCamActive())
	{
		if(m_fTweakTime < 2.f)
		{
			m_fTweakTime += fTimeDelta;
		}
		else
		{
			m_obTransform = m_pTweaker->ApplyTweak(m_obTransform, fTimeDelta);
		}
	}
	else
	{
		m_fTweakTime = 0.f;
	}

	// have to calculate our look at position for use by posible transitions
	// doing it this way allows for modification of the transform either in the
	// LAC (via ModifyLookat()) or by the camera's matrix tweaker
	m_obLookAt = CCamUtil::CalcAdjustedLookat(m_obTransform, m_pLookAtController->GetLastTracked());

	// Depth of Field
	if(m_pDoFEnt)
	{
		m_bUseDoF = true;

		if(m_pOldDoFEnt)
		{
			// Blending in from the old values
			float fRatio = m_fTransitionTime / m_fTotalTransitionTime;

			CPoint pt        = m_pDoFEnt->GetCamPosition()*fRatio + m_pOldDoFEnt->GetCamPosition()*(1.f-fRatio);
			m_fFocalDepth    = CDirection(obCurrPos ^ pt).Length();
			m_fFarBlurDepth  = m_fFocalDepth + m_fDoFFar*fRatio  + m_fOldDoFFar*(1.f-fRatio);
			m_fNearBlurDepth = m_fFocalDepth - m_fDoFNear*fRatio - m_fOldDoFNear*(1.f-fRatio);

			// Increment the transition time and terminate if done.
			m_fTransitionTime += fTimeDelta;
			if(m_fTransitionTime > m_fTotalTransitionTime)
				m_pOldDoFEnt = 0;
		}
		else
		{
			m_fFocalDepth    = CDirection(obCurrPos ^ m_pDoFEnt->GetCamPosition()).Length();
			m_fFarBlurDepth  = m_fFocalDepth + m_fDoFFar;
			m_fNearBlurDepth = m_fFocalDepth - m_fDoFNear;
		}

#ifndef _RELEASE
/*		if(fabsf(m_fFocalDepth)    > 100000.f ||
		   fabsf(m_fFarBlurDepth)  > 100000.f ||
		   fabsf(m_fNearBlurDepth) > 100000.f ||
		   fabsf(m_fConfusionHigh) > 100000.f ||
		   fabsf(m_fConfusionLow)  > 100000.f)
		{
			ntPrintf("Camera Bad DOF (%.2f, %.2f, %.2f - %.2f,%.2f) %s\n", m_fFocalDepth, m_fFarBlurDepth, m_fNearBlurDepth, m_fConfusionHigh, m_fConfusionLow, ntStr::GetString(GetCameraName()));
			ntPrintf("Cam Pos:       (%.2f, %.2f, %.2f, %.2f)\n", obCurrPos.X(), obCurrPos.Y(), obCurrPos.Z(), obCurrPos.W());
			ntPrintf("Focal Ent Pos: (%.2f, %.2f, %.2f, %.2f)\n", m_pDoFEnt->GetCamPosition().X(), m_pDoFEnt->GetCamPosition().Y(), m_pDoFEnt->GetCamPosition().Z(), m_pDoFEnt->GetCamPosition().W());
			if(m_pOldDoFEnt)
				ntPrintf("Focal Ent Pos2: (%.2f, %.2f, %.2f, %.2f)\n", m_pOldDoFEnt->GetCamPosition().X(), m_pOldDoFEnt->GetCamPosition().Y(), m_pOldDoFEnt->GetCamPosition().Z(), m_pOldDoFEnt->GetCamPosition().W());
		}*/
#endif //_RELEASE
	}
	else
	{
		m_bUseDoF = false;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	BasicCamera::Reset                                                                      
//!	Reset the camera                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
void BasicCamera::Reset()
{
	// Apply the combat camera definition to the components
	if(m_pCombatCamDef)
	{
		m_pMotionController->SetCombatCamera(m_pCombatCamDef);
		m_pLensController->SetCombatCamera(m_pCombatCamDef);
	}
}

//------------------------------------------------------------------------------------------
//!    scee.sbashow:modified
//!	BasicCamera::GetTransitionTo                                                                      
//! When the code tests if a transition exists, and it finds one through the scratch, it will
//!	reset the scratch transition to null - otherwise, will use one of the mapped 
//! transitions as per usual
//------------------------------------------------------------------------------------------
const CamTransitionDef* BasicCamera::GetTransitionTo(BasicCamera* pTo)   
{
	if (m_pobScratchTransition)
	{
		if (m_pobScratchTransition->GetDestination() == &pTo->m_template)
		{
			const CamTransitionDef* const pobReturnTrans = 
				m_pobScratchTransition;
			m_pobScratchTransition = 0;
			return pobReturnTrans;
		}
	}

	return m_template.GetTransitionTo(&pTo->m_template);
}



#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	BasicCamera::RenderDebugInfo
//!	Display some debug information
//!
//------------------------------------------------------------------------------------------
void BasicCamera::RenderDebugInfo()
{	
	int iGuide = int(g_VisualDebug->GetDebugDisplayHeight()) - 50;

	CCamUtil::DebugPrintf(20, iGuide-36, "Active Camera: %20s", "GameCam"); //GetNameC());	
}
#endif


//------------------------------------------------------------------------------------------
//!
//!	BasicCamera::SetDoF
//!	Set the Depth of Field settings
//!
//------------------------------------------------------------------------------------------
void BasicCamera::SetDoF(const CEntity* pEntFocal, float fFar, float fNear, float fTransitionTime)
{
	if(m_view.IsLevelCameraInUse(this) && fTransitionTime > 0.f)
	{
		// Blend into the new DoF settings
		m_fTotalTransitionTime = fTransitionTime;
		m_fTransitionTime      = 0.f;

		m_pOldDoFEnt  = m_pDoFEnt;
		m_fOldDoFFar  = m_fDoFFar;
		m_fOldDoFNear = m_fDoFNear;

		m_pDoFEnt              = pEntFocal;
		m_fDoFFar              = fFar;
		m_fDoFNear             = fNear;
	}
	else
	{
		// No need to blend.
		m_fTotalTransitionTime = 0.f;
		m_fTransitionTime      = 0.f;
		m_pDoFEnt              = pEntFocal;
		m_fDoFFar              = fFar;
		m_fDoFNear             = fNear;
		m_pOldDoFEnt           = 0;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	BasicCamera::DebugRender
//!	Render the debug volumes used by this camera, when the camera is selected in wielder.
//!
//------------------------------------------------------------------------------------------
void BasicCamera::DebugRender()
{
#ifndef _RELEASE
	int iGuide = int(g_VisualDebug->GetDebugDisplayHeight()) - 50;

	CCamUtil::DebugPrintf(20, iGuide-46, "Selected Camera: %s\n", ntStr::GetString(GetCameraName()));	
	m_pMotionController->Render();
	m_pLookAtController->Render();
	m_pLensController->Render();
#endif
}
