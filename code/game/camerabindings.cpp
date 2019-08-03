/***************************************************************************************************
*
*	FILE			camerabindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/
#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "Physics/projectilelg.h"

#include "game/camerabindings.h"
#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/timeofday.h"

#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camtrans_lerp.h"
#include "camera/coolcam_maya.h"
#include "camera/coolcam_generic.h"
#include "camera/coolcam_aftertouch.h"
#include "camera/coolcam_turret.h"
#include "camera/coolcam_chase.h"
#include "camera/coolcam_aim.h"
#include "camera/coolcam_chaseaimcombo.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camtools.h"

#include "objectdatabase/dataobject.h"

#include "core/osddisplay.h"
#include "core/timer.h"

using namespace LuaPlus;

//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Cam_SetCameraImportance(float fImportance)
// DESCRIPTION: Sets the importance of the entity to the camera system
//-------------------------------------------------------------------------------------------------
static int SetCameraImportance(LuaState* pobState)
{
	BIND_DEPRECATED(pobState->GetCState());
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Cam_SetCameraImportance(float fImportance, CEntity *pEntity)
// DESCRIPTION: Sets the importance of the entity to the camera system
//-------------------------------------------------------------------------------------------------
static void EntitySetCameraImportance(float fImportance, CEntity *pEntity)
{
	UNUSED(fImportance);
	UNUSED(pEntity);

	BIND_DEPRECATED(CLuaGlobal::Get().GetState().GetCState());
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Cam_SetCameraInfluenceRadius(float fIn, float fOut)
// DESCRIPTION: Sets the influence radii of this entity for the camera system
//-------------------------------------------------------------------------------------------------
static void SetCameraInfluenceRadius(float fIn, float fOut)
{
	UNUSED(fIn);
	UNUSED(fOut);

	BIND_DEPRECATED(CLuaGlobal::Get().GetState().GetCState());
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  ShakeCamera(float time, float intensity, float speed)
// DESCRIPTION:	Shake the camera.  Time in seconds, itensity in degrees, speed in shakes per second.
//-------------------------------------------------------------------------------------------------
static void ShakeCamera(float fTime, float fIntensity, float fSpeed)
{
	UNUSED(fTime);
	UNUSED(fIntensity);
	UNUSED(fSpeed);
	
	BIND_DEPRECATED(CLuaGlobal::Get().GetState().GetCState());
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:  Cam_ActivateRotatingCam(Entity* attacker, Entity* attackee, float time, float angle, int transitionIn (OPT), int transitionOut (OPT))
// DESCRIPTION: Start a rotating camera
// Returns a handle to the camera for referencing it in the future...
//-------------------------------------------------------------------------------------------------
static int ActivateRotatingCamera(LuaState* pobState)
{
	BIND_DEPRECATED(pobState->GetCState());

	LuaStack obArgs(pobState);
	ntAssert(obArgs[1].IsLightUserData() || obArgs[1].IsUserData());		// Invoking Character
	ntAssert(obArgs[2].IsLightUserData() || obArgs[2].IsUserData());		// Target Entity
	ntAssert(obArgs[3].IsNumber());				// Time
	ntAssert(obArgs[4].IsNumber());				// Angle
	
	CoolCam_Generic* pCam = FW_NEW CoolCam_Generic(*CamMan::GetPrimaryView());
	
	pCam->Init(obArgs[3].GetFloat(), CLuaGlobal::Get().GetEnt(pobState, 1), CLuaGlobal::Get().GetEnt(pobState, 2),
			   obArgs[4].GetFloat() * DEG_TO_RAD_VALUE);

	//TODO: FIX.
	CamTransitionDef *pDef;
	pDef = 0;


	CamMan::GetPrimaryView()->AddCoolCamera(pCam, pDef);
	ntPrintf("### Activated Rotating Cam ID%d.\n",pCam->GetID());
	pobState->PushInteger(pCam->GetID());

	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Cam_ActivateThrownCoolCam()
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int ActivateThrownCoolCam(LuaState* pState) //int iPickupCam = 0, CEntity* pThrower = 0)
{
	BIND_DEPRECATED(pState->GetCState());
/*
	LuaStack args(pState);
	int iPickupCam;
	CEntity* pThrower;

	if(args[1].IsNil() || args[1].IsNone())
		iPickupCam = 0;
	else
	{
		ntAssert(args[1].IsNumber());
		iPickupCam = args[1].GetInteger();
	}

	if(args[2].IsNil() || args[2].IsNone())
		pThrower = 0;
	else
	{
		ntAssert(args[2].IsLightUserData() || args[2].IsUserData());
		pThrower = CLuaGlobal::Get().GetEnt(pState, 2);
	}


	// Get the thrown entity...
	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	CDirection direction; // = pobSelf->GetPhysicsSystem()->GetLinearVelocity();
	if( pobSelf->GetPhysicsSystem() )
	{
		direction = pobSelf->GetPhysicsSystem()->GetLinearVelocity();
	}
	
	direction.Y() =  0.0f;
	direction.Normalise();
	direction.Y() = -0.01f;

	// Get the offset for this object

	CVector offset(CONSTRUCT_CLEAR);

	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobSelf);
	StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface(pDO);
	if(pInterface->GetFieldByName("Attributes") != 0)
	{
		CKeyString s = pInterface->Get<CKeyString>(pDO, "Attributes");
		DataObject* pSubDO = ObjectDatabase::Get().GetDataObjectFromName(*s);
		StdDataInterface* pSubInterface = ObjectDatabase::Get().GetInterface(pSubDO);
		if(pSubInterface->GetFieldByName("AftertouchCamOffset") != 0)
			offset = pSubInterface->Get<CVector>(pSubDO, "AftertouchCamOffset");
	}


	// Create a cool camera
	CoolCam_AfterTouch* pCam = FW_NEW CoolCam_AfterTouch(*CamMan::GetPrimaryView());;
	pCam->Init(pobSelf, direction, CDirection(offset));
	

	// If we're coming directly from the pickup camera then lerp into the throw cam...
	CamTransitionDef* pDef = 0;
	if(iPickupCam)
	{
		CameraInterface* pPickupCam = CamMan::GetPrimaryView()->GetCoolCam(iPickupCam);
		//ntAssert(pPickupCam && pPickupCam->GetType() == CT_ROTATING);

		if(pPickupCam && pPickupCam->GetType() == CT_ROTATING)
		{
			bool bLerp = true;

			if(pThrower)
			{
				ntAssert(pThrower->GetEntityInfo());
				CDirection d1 = direction;
				CDirection d2 = ((CoolCam_Generic*)pPickupCam)->GetOffset();
				d1.Y() = 0.0f;
				d1.Normalise();
				d2.Y() = 0.0f;
				d2.Normalise();
				if(d1.Dot(d2) >= 0.5f)
					bLerp = false;
			}

			if(bLerp)
			{
				pDef = FW_NEW CamTrans_LerpDef(1.f);
				//pDef->SetPriority(100);
			}
		}
	}
	else
	{
		// Lerp into the camera if we've come from an aiming or chasing camera
		CoolCamera* pCurrentCam = CamMan::GetPrimaryView()->GetCoolCam(CamMan::GetPrimaryView()->GetActiveCameraID());
		if(pCurrentCam && 
		   (pCurrentCam->GetType() == CT_AIM || pCurrentCam->GetType() == CT_CHASE || pCurrentCam->GetType() == CT_CHASEAIMCOMBO))
		{
			pDef = FW_NEW CamTrans_LerpDef(1.f);
			//pDef->SetPriority(100);
		}
	}

	// Add the camera to the view
	CamMan::GetPrimaryView()->AddCoolCamera(pCam, pDef);

	OSD::Add(OSD::CAMERA, DC_WHITE, "+++ Activated Thrown Cam ID%d.\n",pCam->GetID());

	// Return our new cameras id to lua
	pState->PushInteger(pCam->GetID());
	return 1;
	*/
	pState->PushInteger(0);
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Cam_ThrownCoolCamLookAt(Entity* HurtEnemy)
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int ThrownCoolCamLookAt(LuaState* pobState)
{
	BIND_DEPRECATED(pobState->GetCState());

	/*LuaStack obArgs(pobState);

	ntAssert(obArgs[1].IsLightUserData() || obArgs[1].IsUserData());
	CEntity* pEnt = CLuaGlobal::Get().GetEnt(pobState, 1);
	ntAssert(pEnt);
	
	ntAssert(obArgs[2].IsBoolean() || obArgs[2].IsNil() || obArgs[2].IsNone());
	bool bRebound = obArgs[2].IsBoolean() ? obArgs[2].GetBoolean() : false;

	// Should change this so that you pass in the camera id...
	CoolCam_AfterTouch* pCam = (CoolCam_AfterTouch*)CamMan::GetPrimaryView()->FindCoolCam(CT_AFTERTOUCH);
	pCam->LookAt(pEnt, bRebound);*/
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	int Cam_ActivateProjectileCam(string,entity)
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int ActivateProjectileCam (const char* pcCamProperties, CEntity* pobTargetEntity)
{
	BIND_DEPRECATED(CLuaGlobal::Get().GetState().GetCState());

	CProjectileCoolCamParameters* pobProperties=ObjectDatabase::Get().GetPointerFromName<CProjectileCoolCamParameters*>(pcCamProperties);

	ntAssert(pobProperties);
	Physics::ProjectileLG* pobProjectileState = ( Physics::ProjectileLG* ) pobTargetEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::PROJECTILE_LG );

	ntAssert(pobProjectileState);

	// Create a cool camera
	CProjectileCoolCam* pCam = FW_NEW CProjectileCoolCam(*CamMan::GetPrimaryView());;
	pCam->Init(pobProperties, pobTargetEntity, pobProjectileState->GetLinearVelocity());
	
	// And add it to the manager
	CamMan::GetPrimaryView()->AddCoolCamera(pCam);

	return pCam->GetID();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	cameraID Cam_ActivateTurretCam(string transform, number YOffset, number ZOffset)
// DESCRIPTION:	Returns the id of the turret camera
//-------------------------------------------------------------------------------------------------
static int ActivateTurretCamera(LuaState* state)
{
	LuaStack args(state);

	// Check everythings valid
	CEntity* pEnt = CLuaGlobal::Get().GetTarg();
	ntAssert(pEnt);
	ntAssert(args[1].IsString());
	ntAssert(args[2].IsNumber());
	ntAssert(args[3].IsNumber());

	// Create 
	CoolCam_Turret *pCam = FW_NEW CoolCam_Turret(*CamMan::GetPrimaryView());;
	pCam->Init(pEnt, args[1].GetString(), CDirection(0.f, args[2].GetFloat(), args[3].GetFloat()));
	CamMan::GetPrimaryView()->AddCoolCamera(pCam);
	
	state->PushInteger(pCam->GetID());
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	cameraID Cam_ActivateLadderCam()
// DESCRIPTION:	Returns the id of the ladder camera
//-------------------------------------------------------------------------------------------------
static int ActivateLadderCamera(LuaState* state)
{
	CEntity* pEnt = CLuaGlobal::Get().GetTarg();
	ntAssert(pEnt);

	LadderCam *pCam = FW_NEW LadderCam(*CamMan::GetPrimaryView(), *pEnt);
	CamMan::GetPrimaryView()->AddCoolCamera(pCam);
	
	state->PushInteger(pCam->GetID());
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	cameraID Cam_ActivateChaseAimComboCamera()
// DESCRIPTION:	Returns the id of the chase camera
//-------------------------------------------------------------------------------------------------
static int Cam_ActivateChaseAimComboCamera(LuaState* state)
{
	BIND_DEPRECATED(state->GetCState());

	LuaStack args(state);
	if(!args[1].IsLightUserData() && !args[1].IsUserData())
	{
		lua_bind_warn_msg(("Cam_ActivateChaseAimComboCamera, bad entity\n"));
		lua_debug_break_non_fatal;
		state->PushInteger(-1);
		return 1;
	}
	if(!args[2].IsString() || !args[3].IsString())
	{
		lua_bind_warn_msg(("Cam_ActivateChaseCamera, bad definition\n"));
		lua_debug_break_non_fatal;
		state->PushInteger(-1);
		return 1;
	}


	CEntity* pEnt = CLuaGlobal::Get().GetEnt(state, 1);
	

	CoolCam_ChaseDef* pChaseDef = ObjectDatabase::Get().GetPointerFromName<CoolCam_ChaseDef*>(args[2].GetString());
	CoolCam_AimDef*   pAimDef   = ObjectDatabase::Get().GetPointerFromName<CoolCam_AimDef*>(args[3].GetString());

	ntAssert(pEnt);
	ntAssert(pChaseDef);
	ntAssert(pAimDef);
	
	CoolCam_ChaseAimCombo *pCam = FW_NEW CoolCam_ChaseAimCombo(*CamMan::GetPrimaryView(), *pEnt, 
		                                                    *pChaseDef, *pAimDef);

	CamMan::GetPrimaryView()->AddCoolCamera(pCam);
	
	state->PushInteger(pCam->GetID());
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	cameraID Cam_ActivateAimingMode()                                                  
// DESCRIPTION:	Returns the id of the aiming camera                                                
//-------------------------------------------------------------------------------------------------
static int Cam_ActivateAimingMode(LuaState* state)
{ 
	BIND_DEPRECATED(state->GetCState());

/*	CoolCam_ChaseAimCombo* pCam = (CoolCam_ChaseAimCombo*)CamMan::GetPrimaryView()->FindCoolCam(CT_CHASEAIMCOMBO);

	if(pCam)
	{
		pCam->ActivateAim();
		state->PushInteger(pCam->GetID());
	}
	else
		state->PushInteger(0);*/

	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	cameraID Cam_ActivateChasingMode()
// DESCRIPTION:	Returns the id of the aiming camera
//-------------------------------------------------------------------------------------------------
static int Cam_ActivateChasingMode(LuaState* state)
{
	BIND_DEPRECATED(state->GetCState());

/*	CoolCam_ChaseAimCombo* pCam = (CoolCam_ChaseAimCombo*)CamMan::GetPrimaryView()->FindCoolCam(CT_CHASEAIMCOMBO);

	if(pCam)
	{
		pCam->ActivateChase();
		state->PushInteger(pCam->GetID());
	}
	else
		state->PushInteger(0);*/

	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	void Cam_KillCoolCam(int iCamID)
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int KillCoolCam(LuaState* state)
{
	BIND_DEPRECATED(state->GetCState());

	LuaStack obArgs(state);
	if(obArgs[1].IsInteger())
	{
		CamMan::GetPrimaryView()->RemoveCoolCamera(obArgs[1].GetInteger());

		CTimer::Get().SetCameraTimeScalar(1.0f); // HC: Temporary to restore cool cam timer scalar

		ntPrintf("--- Killed Cool Cam ID%d\n", obArgs[1].GetInteger());
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	void Cam_KillAllCoolCams()
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int KillAllCoolCams(LuaState* state)
{
	BIND_DEPRECATED(state->GetCState());

	CamMan::GetPrimaryView()->RemoveAllCoolCameras();

	CTimer::Get().SetCameraTimeScalar(1.0f); // Restore cool cam timer scalar

	ntPrintf("--- Killed All Cool Cams\n");
	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	void Cam_PlayMayaAnim()
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------
static int Cam_PlayMayaAnim(LuaState* pState)
{
	BIND_DEPRECATED(pState->GetCState())

	/*
	LuaStack obArgs(pState);
	// Create an entity to play our camera animations on...
	// ----------------------------------------------------

	const char* pcAnimName = obArgs[1].IsString() ? obArgs[1].GetString() : "camera_air_e3_fight";

	//
	CoolCam_Maya *pCoolCam = FW_NEW CoolCam_Maya(*CamMan::GetPrimaryView());;
	pCoolCam->Init(CKeyString(pcAnimName));

	// Add the cool camera to the manager.
	if(obArgs[1].IsTable())
		pCoolCam->SetEventHandler(obArgs[1]);

	if(obArgs[2].IsTable())
		pCoolCam->SetEventHandler(obArgs[2]);

	// If we've got an entity pointer then set the camera to start aligned with that entity.
	if(obArgs[3].IsLightUserData() || obArgs[3].IsUserData())
	{
		CEntity* pParent = CLuaGlobal::Get().GetEnt(pState, 3);
		pCoolCam->SetMatrix(pParent->GetTransformP("ROOT")->GetWorldMatrix());
	}

	CamMan::GetPrimaryView()->RemoveAllCoolCameras();
	CamMan::GetPrimaryView()->AddCoolCamera(pCoolCam);

	// Return the ID of the camera
	pState->PushInteger(pCoolCam->GetID());
	return 1;*/

	pState->PushInteger(0);
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Render_TOD_MoveToNextPreferedTime()
// DESCRIPTION:	Should be in a different file.
//-------------------------------------------------------------------------------------------------
static int Render_TOD_MoveToNextPreferedTime(LuaState* pState)
{
	LuaStack args(pState);

	pState->PushBoolean(TimeOfDay::Get().MoveToNextPreferedTime(args[1].GetFloat()));
	
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	Cam_UseDoF(bool)
// DESCRIPTION:	Enable or disable the Depth of Field effects on cameras.
//-------------------------------------------------------------------------------------------------
static int Cam_UseDoF(LuaState* /*pState*/)
{
	BIND_DEPRECATED(CLuaGlobal::Get().GetState().GetCState());
	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CCameraBindings::Register
*
*	DESCRIPTION		Register the functions with the scripting enviroment
*
***************************************************************************************************/
void CCameraBindings::Register()
{
	LuaState& obState = CLuaGlobal::Get().GetState();
	LuaAutoBlock autoBlock(&obState);
	LuaObject obGlobals = obState.GetGlobals();

	// Deprecating
	obGlobals.Register("SetCameraImportance", SetCameraImportance);
	obGlobals.RegisterDirect("SetCameraInfluenceRadius", SetCameraInfluenceRadius);
	obGlobals.RegisterDirect("EntitySetCameraImportance", EntitySetCameraImportance);
	obGlobals.RegisterDirect("ShakeCamera", ShakeCamera);
	obGlobals.Register("ActivateRotatingCamera", ActivateRotatingCamera);
	obGlobals.Register("ActivateThrownCoolCam", ActivateThrownCoolCam);
	obGlobals.Register("ThrownCoolCamLookAt", ThrownCoolCamLookAt);
	obGlobals.RegisterDirect("ActivateProjectileCam", ActivateProjectileCam);
	obGlobals.Register("ActivateTurretCamera", ActivateTurretCamera);
	obGlobals.Register("ActivateLadderCamera", ActivateLadderCamera);
	obGlobals.Register("KillCoolCam", KillCoolCam);
	obGlobals.Register("KillAllCoolCams", KillAllCoolCams);
	obGlobals.Register("DeactivateRotatingCamera", KillCoolCam);
	obGlobals.Register("DeactivateThrownCoolCam", KillCoolCam);

	// New prefix bindings
	// -------------------

	// Scene Element Bindings
	obGlobals.Register("Cam_SetCameraImportance", SetCameraImportance);						// DEPRECATED
	obGlobals.RegisterDirect("Cam_SetCameraInfluenceRadius", SetCameraInfluenceRadius);		// DEPRECATED
	obGlobals.RegisterDirect("Cam_EntitySetCameraImportance", EntitySetCameraImportance);	// DEPRECATED

	// Cool Cam Bindings
	obGlobals.Register("Cam_ActivateRotatingCam", ActivateRotatingCamera);					// DEPRECATED
	obGlobals.Register("Cam_ActivateThrownCoolCam", ActivateThrownCoolCam);					// DEPRECATED
	obGlobals.Register("Cam_ThrownCoolCamLookAt", ThrownCoolCamLookAt);						// DEPRECATED
	obGlobals.RegisterDirect("Cam_ActivateProjectileCam", ActivateProjectileCam);			// DEPRECATED
	obGlobals.Register("Cam_ActivateTurretCam", ActivateTurretCamera);
	obGlobals.Register("Cam_ActivateLadderCam", ActivateLadderCamera);
	obGlobals.Register("Cam_ActivateChaseAimComboCamera", Cam_ActivateChaseAimComboCamera);	// DEPRECATED
	obGlobals.Register("Cam_ActivateAimingMode", Cam_ActivateAimingMode);					// DEPRECATED
	obGlobals.Register("Cam_ActivateChasingMode", Cam_ActivateChasingMode);					// DEPRECATED
	obGlobals.Register("Cam_PlayMayaAnim", Cam_PlayMayaAnim);
	obGlobals.Register("Cam_KillCoolCam", KillCoolCam);										// DEPRECATED
	obGlobals.Register("Cam_KillAllCoolCams", KillAllCoolCams);								// DEPRECATED

	// General Bindings
	obGlobals.Register("Cam_UseDoF", Cam_UseDoF);


	// Not Camera
	obGlobals.Register("Render_TOD_MoveToNextPreferedTime", Render_TOD_MoveToNextPreferedTime);
}
