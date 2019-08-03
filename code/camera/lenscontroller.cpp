//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file lenscontroller.cpp                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/lenscontroller.h"
#include "camera/motioncontroller.h"
#include "camera/lookatcontroller.h"
#include "camera/converger.h"
#include "camera/basiccamera.h"
#include "camera/sceneelementcomponent.h"
#include "camera/elementmanager.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/combatcamdef.h"
#include "camera/combatcam.h"

#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"

#ifdef _DEBUG
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkMath.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplay.h>
#endif
#endif


//------------------------------------------------------------------------------------------
// LensControllerDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LensControllerDef, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bFOVDynamicZoom, false, FOVDynamicZoom)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fIdealFOV, 35.0f, IdealFOV)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fIdealInterestingRatio, 0.75f, InterestingRatio)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxInterestingRatio, 0.95f, MaxInterestingRatio)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinInterestingRatio, 0.5f, MinInterestingRatio)
END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LensControllerDef::Create
//!	Create an LensController from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LensController* LensControllerDef::Create(const BasicCamera* pParent)
{
	ntError_p(m_fIdealFOV > 0.f, ("%s FOV < 0.0\n", ntStr::GetString(pParent->GetCameraName())));
	ntError_p(m_fIdealInterestingRatio > 0.f, ("%s Interesting Ratio < 0.0\n", ntStr::GetString(pParent->GetCameraName())));
	ntError_p(m_fMinInterestingRatio > 0.f, ("%s Min Ratio < 0.0\n", ntStr::GetString(pParent->GetCameraName())));
	ntError_p(m_fMaxInterestingRatio > 0.f, ("%s Max Ratio < 0.0\n", ntStr::GetString(pParent->GetCameraName())));
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LensController(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::LensController
*
*	DESCRIPTION		Construct the lens controller from a debug definition
*
***************************************************************************************************/
LensController::LensController(const BasicCamera* pParent, const LensControllerDef& def) :  
	m_pParent(0),
	m_def(def),
	m_pobPushConverger(0),
	m_pobFOVConverger(0),
	m_fIdealFOV(0),
	m_obLastLook(CONSTRUCT_CLEAR),
	m_fMaxRotation(0.f)
#ifdef _DEBUG
  	,m_obLOSStart(CONSTRUCT_CLEAR), m_obLOSEnd(CONSTRUCT_CLEAR)
#endif
{
	m_pParent = pParent;

	// Copy working item defaults out of the definition
	m_fIdealFOV = m_def.m_fIdealFOV;
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::Reset
*
*	DESCRIPTION		also acts as initialiser if we had anything to init...
*
***************************************************************************************************/
void	LensController::Reset()
{
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::LensController
*
*	DESCRIPTION		Construct the lens controller from a debug definition
*
***************************************************************************************************/
LensController::~LensController()
{
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::Update
*
*	DESCRIPTION		calc FOV based on position and look at
*
***************************************************************************************************/
float	LensController::Update(MotionController* pMC, LookAtController* pLAC, float fTimeDelta)
{
	ntAssert(pMC);
	ntAssert(pLAC);

	// Find the ideal zoom to include all the interesting elements
	float fFOV = m_fIdealFOV;

	// Detect if the player is obscured or not...
	// If she is then we should rotate the camera to get a clearer shot
	////////////////////////////////////////////////////////////////////
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	CPoint ptPlayerPos(CONSTRUCT_CLEAR);
	if(pobPlayer)
		ptPlayerPos = pobPlayer->GetCamPosition();

	CPoint ptCamPos = pMC->GetLastNoRotation();
	static CDirection dirUp(0.0f, 0.5f, 0.0f);
	CDirection dirView = ptCamPos ^ ptPlayerPos;
	CDirection dirCross = dirView.Cross(dirUp);
	dirCross.Normalise();
	dirCross *= 0.5f;

	if(!IsPlayerVisible(ptCamPos, ptPlayerPos)         || !IsPlayerVisible(ptCamPos, ptPlayerPos + dirUp)    ||
	   !IsPlayerVisible(ptCamPos, ptPlayerPos - dirUp) || !IsPlayerVisible(ptCamPos, ptPlayerPos + dirCross) || 
	   !IsPlayerVisible(ptCamPos, ptPlayerPos - dirCross))
	{
		// Some rotation is required...
		// Check a partial rotation to decide whether to maintain or continue rotation
		ptCamPos = pMC->GetLastPartRotation();
		dirView = ptCamPos ^ ptPlayerPos;
		dirCross = dirView.Cross(dirUp);
		dirCross.Normalise();
		dirCross *= 0.5f;
		if(!IsPlayerVisible(ptCamPos, ptPlayerPos)            ||
		   !IsPlayerVisible(ptCamPos, ptPlayerPos + dirUp)    || !IsPlayerVisible(ptCamPos, ptPlayerPos - dirUp) ||
		   !IsPlayerVisible(ptCamPos, ptPlayerPos + dirCross) || !IsPlayerVisible(ptCamPos, ptPlayerPos - dirCross))
		{
			pMC->SetRotation(m_fMaxRotation); // Continue rotation to maximum
		}
		else
			pMC->MaintainRotation();
	}
	
	// Use an unsmoothed look at point, zoom will smooth with lookat
	m_obLastLook = pLAC->GetLastTracked();


	// Now Consider Zoom ins and outs
	//////////////////////////////////
	float fZoom(0.f);
	if(m_pParent->GetCombatCam())
	{
		float fFOVRad = 
			m_pParent->GetCombatCam()->UpdateFOV(m_fIdealFOV * DEG_TO_RAD_VALUE, fTimeDelta);

		//scee.sbashow: need to update the local fFov with the updated FOV from the combat cam
		//				as this method returns this fFov, a return value which is picked 
		// 					up by the basic camera, and used as its internal fov value.
		// 				So this fixes the issue that the fov on the combat cam was not having an effect.
		fFOV = fFOVRad * RAD_TO_DEG_VALUE;

        fZoom = m_pParent->GetCombatCam()->CalcZoom(fTimeDelta, pMC->GetLastNoZoom(), pLAC->GetLastTracked(), 
			                                        fFOVRad, 
													m_def.m_bFOVDynamicZoom ? m_def.m_fMinInterestingRatio : -1.f, 
													m_def.m_bFOVDynamicZoom ? m_def.m_fMaxInterestingRatio : -1.f);
	}
	else if(m_def.m_bFOVDynamicZoom)
	{
		float fTargetFOV = fatanf(ftanf(m_fIdealFOV * DEG_TO_RAD_VALUE) * m_def.m_fIdealInterestingRatio);
		fZoom = m_pParent->GetElementManager().CalcPlayerZoom(pMC->GetLastNoZoom(), pLAC->GetLastTracked(),
															 CamMan::GetPrimaryView()->GetAspectRatio(),
                                                             fTargetFOV);

		// scee.sbashow: note, no converger available for zoom value when combat cam not defined. 
		//				The zoom converger used to be this in this lens controller, is now part of combat cam.
	}

	//float fZoom = m_pParent->GetCombatCam() ? m_pParent->GetCombatCam()->CalcZoom(m_fIdealFOV, fTimeDelta, m_def.m_fInterestingRatio, .5f) : 0.f;


#ifdef _WIN32
//	if(m_pParent->GetType() == CT_BASIC)
//		ntPrintf("Zoom: %s - %.2f\n", m_pParent->GetCameraName(), fNewZoom);
#endif 

//	if(m_def.m_bFOVDynamicZoom)
//	{
//		ntPrintf("Zoom: %.2f (%.2f, %.2f, %.2f)\n", fZoom, obVec.X(), obVec.Y(), obVec.Z());
//	}

	// Move Required...
	if(fZoom > 0.f)
	{
		CDirection obVec = m_obLastLook ^ pMC->GetLastNoZoom();
		obVec.Normalise();
		obVec *= fZoom;
		pMC->SetPushVec(obVec);
	}

	return fFOV;
}


/***************************************************************************************************
*
*	FUNCTION		LensController::SetCombatCamera
*
*	DESCRIPTION		Setup / Adjust the combat camera
*
***************************************************************************************************/
void LensController::SetCombatCamera(CCombatCamDef *pobCombatCamDef)
{
	if(pobCombatCamDef)
	{
		m_fMaxRotation = pobCombatCamDef->m_fMaxRotation * DEG_TO_RAD_VALUE;
	}
	else
	{
		m_fMaxRotation = 0.0f;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::IsPlayerVisible
*
*	DESCRIPTION		Determine if we can see the player along this ray
*
***************************************************************************************************/
bool LensController::IsPlayerVisible(const CPoint &obPos, const CPoint &obTarg)
{
#ifdef _DEBUG
	m_obLOSStart = obPos;
	m_obLOSEnd   = obTarg;
#endif

	Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
	// [Mus] - What settings for this cast ?
	obFlag.flags.i_am = Physics::LINE_SIGHT_BIT;
	obFlag.flags.i_collide_with = (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | 
                                   Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
                                   Physics::SMALL_INTERACTABLE_BIT          |
                                   Physics::LARGE_INTERACTABLE_BIT			|
								   Physics::RAGDOLL_BIT                     );
	
	//CGatso::Start("A - Player Visible");
	const CEntity *pobEntity = Physics::CPhysicsWorld::Get().CastRay(obPos, obTarg, obFlag);
	//CGatso::Stop("A - Player Visible");
	if(!pobEntity) // Should hit something
		return true;

	return pobEntity->IsPlayer();
}


/***************************************************************************************************
*	
*	FUNCTION		LensController::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
#ifdef _DEBUG
void	LensController::RenderInfo(int iX, int iY)
{
	// Draw LOS line
	// In game
	CCamUtil::Render_Line(m_obLOSStart, m_obLOSEnd, 1.0f, 0.0f, 0.0f, 1.0f);
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	// And in havok visualiser
	hkVector4 obHStart(m_obLOSStart.X(), m_obLOSStart.Y(), m_obLOSStart.Z());
	hkVector4 obHEnd(m_obLOSEnd.X(), m_obLOSEnd.Y(), m_obLOSEnd.Z());
	HK_DISPLAY_LINE(obHStart, obHEnd, hkColor::RED);
#else
	UNUSED(iX);
	UNUSED(iY);
#endif

	//CCamUtil::DebugPrintf(iX, iY, "LensController: %s", GetNameC());
}


#endif
