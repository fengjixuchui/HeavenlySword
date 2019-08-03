/***************************************************************************************************
*
*	$Header:: /game/motioncontroller.cpp 2     13/08/03 17:22 Wil                                  $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "camera/motioncontroller.h"
#include "camera/combatcamdef.h"
#include "camera/camerainterface.h"
#include "camera/elementmanager.h"
#include "camera/camman_public.h"
#include "camera/smoother.h"
#include "camera/converger.h"
#include "camera/pointtransform.h"
#include "camera/curverail.h"
#include "camera/curveinterface.h"
#include "camera/camview.h"
#include "camera/sceneelementcomponent.h"
#include "game/entity.h"
#include "game/entity.inl"

#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// Interfaces
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(MCFixedPosDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)
	
	// Extended
	PUBLISH_VAR_WITH_DEFAULT_AS(m_ptPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
END_STD_INTERFACE


START_CHUNKED_INTERFACE(MCBoomDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_VAR_WITH_DEFAULT_AS(m_ptPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDistance, 0.0f, Distance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fBoomHeight, 0.0f, BoomHeight)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBoomIgnoreY, false, BoomIgnoreY)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBoomReverse, false, BoomReverse)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(MCPOIRelDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_VAR_WITH_DEFAULT_AS(m_direction, CDirection(0.0f, 0.0f, 0.0f), Direction)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(MCPOIChaseDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxDistance, 0.0f, MaxDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinDistance, 0.0f, MinDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStartAngle, 0.0f, StartAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMoveSpeed, 0.0f, MoveSpeed)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(MCPOIStrafeDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_PTR_AS(m_pobBaseConvergerDef, StrafeConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDistance, 0.0f, Distance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStartAngle, 0.0f, StartAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMoveSpeed, 0.0f, MoveSpeed)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(MCRailNearFarDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_PTR_AS(m_pobRailDef, Rail)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(MCRailGuideDef, Mem::MC_CAMERA)
	// Basic
	PUBLISH_PTR_AS(m_pPOITransDef, POITrans)
	PUBLISH_PTR_AS(m_pPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pPosConvergerDef, PosConvergerDef)
	PUBLISH_PTR_AS(m_pBoundingTransDef, BoundingTrans)

	// Extended
	PUBLISH_PTR_AS(m_pobRailDef, Rail)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOriginOffset, false, OriginOffset) 
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPOIMoveFactor, 0.0f, POIHeadingCompensateFactor)
END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MotionControllerDef::MotionControllerDef
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionControllerDef::MotionControllerDef()
: m_pPOITransDef(0),
  m_pPOISmootherDef(0),
  m_pPosConvergerDef(0)
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MotionControllerDef::Create
//!	Create a motion controller from this definition.
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MotionControllerDef::Create(const CameraInterface* pParent, CEntity* pEntParent)
{
	MotionController* pMC = Create(pParent);
	pMC->m_pParentEntity = pEntParent;

	return pMC;
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::MotionController
*
*	DESCRIPTION		Debug constructor for handmade MC's
*
***************************************************************************************************/
MotionController::MotionController(const CameraInterface* pParent, const MotionControllerDef& def)
:	m_pParent(pParent),
	m_pParentEntity(0),
	m_pPOISmooth(0),
	m_pPOITrans(0),
	m_pPosConv(0),
	m_pFinalTrans(0),
	m_pRotationConv(0),
	m_bReqPush(false),
	m_obPushOffset(CONSTRUCT_CLEAR),
	m_bLastTrackedValid(false),
	m_bLastUnmodifiedValid(false),
	m_bLastPointValid(false),
	m_bLastNoZoomValid(false),
	m_bLastNoRotationValid(false),
	m_obLastTracked(CONSTRUCT_CLEAR),
	m_obLastUnModified(CONSTRUCT_CLEAR),
	m_obLastPoint(CONSTRUCT_CLEAR),
	m_obLastNoZoom(CONSTRUCT_CLEAR),
	m_obLastNoRotation(CONSTRUCT_CLEAR),
	m_obLastPartRotation(CONSTRUCT_CLEAR),
	m_fRotation(0.0f),
	m_bRotation(false),
	m_fRotationHoldTime(0.0f),
	m_fRotationTimeOut(0.f),
	m_fLastRotation(0.f)
{
	if(def.m_pPOISmootherDef)
	{
#ifndef _RELEASE
		DataObject* pObj = ObjectDatabase::Get().GetDataObjectFromPointer(def.m_pPOISmootherDef);
		if(pObj)
		{
			ntError_p(!strcmp(pObj->GetClassName(), "SmootherDef"), ("Camera '%s' - MotionController Smoother of Incorrect Type.\n", ntStr::GetString(m_pParent->GetCameraName())));
		}
#endif

		m_pPOISmooth = NT_NEW_CHUNK(Mem::MC_CAMERA) CPointSmoother(*def.m_pPOISmootherDef);
	}

	if(def.m_pPosConvergerDef)
	{
#ifndef _RELEASE
		DataObject* pObj = ObjectDatabase::Get().GetDataObjectFromPointer(def.m_pPosConvergerDef);
		if(pObj)
		{
			ntError_p(!strcmp(pObj->GetClassName(), "ConvergerDef"), ("Camera '%s' - MotionController Converger of Incorrect Type.\n", ntStr::GetString(m_pParent->GetCameraName())));
		}
#endif
		m_pPosConv = NT_NEW_CHUNK(Mem::MC_CAMERA) CPointConverger(*def.m_pPosConvergerDef);
	}

	if(def.m_pPOITransDef)
	{
#ifndef _RELEASE
		DataObject* pObj = ObjectDatabase::Get().GetDataObjectFromPointer(def.m_pPOITransDef);

		if(pObj)
		{
			ntError_p(!strcmp(pObj->GetClassName(), "PTGuideDef") ||
					  !strcmp(pObj->GetClassName(), "PTRangeDef") ||
					  !strcmp(pObj->GetClassName(), "PTVolumeDef"), ("Camera '%s' - MotionController POITrans of Incorrect Type.\n", ntStr::GetString(m_pParent->GetCameraName())));
		}
#endif
		m_pPOITrans = def.m_pPOITransDef->Create();
	}

	if(def.m_pBoundingTransDef)
	{
#ifndef _RELEASE
		DataObject* pObj = ObjectDatabase::Get().GetDataObjectFromPointer(def.m_pBoundingTransDef);

		if(pObj)
		{
			ntError_p(!strcmp(pObj->GetClassName(), "PTGuideDef") ||
					  !strcmp(pObj->GetClassName(), "PTRangeDef") ||
					  !strcmp(pObj->GetClassName(), "PTVolumeDef"), ("Camera '%s' - MotionController BoundingTrans of Incorrect Type.\n", ntStr::GetString(m_pParent->GetCameraName())));
		}
#endif

		m_pFinalTrans = def.m_pBoundingTransDef->Create();
	}
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::Reset
*
*	DESCRIPTION		reset the MC
*
***************************************************************************************************/
void MotionController::Reset()
{
	if(m_pPOISmooth)
		m_pPOISmooth->Reset();

	if(m_pPOITrans)
		m_pPOITrans->Reset();

	if(m_pPosConv)
		m_pPosConv->Reset();

	if(m_pFinalTrans)
		m_pFinalTrans->Reset();

	if(m_pRotationConv)
		m_pRotationConv->Reset();

	m_bReqPush = false;
	m_bLastTrackedValid = false;
	m_bLastUnmodifiedValid = false;
	m_bLastNoZoomValid = false;
	m_bLastPointValid = false;
	m_bRotation = false;
	m_fRotation = 0.0f;
	m_fRotationTimeOut = 0.0f;
	m_fLastRotation = 0.0f;
}


/***************************************************************************************************
*
*	FUNCTION		MotionController::SetCombatCamera
*
*	DESCRIPTION		Setup / Adjust the combat camera

***************************************************************************************************/
void MotionController::SetCombatCamera(CCombatCamDef *pobCombatCamDef)
{
	if(m_pRotationConv)
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pRotationConv);
	}

	if(pobCombatCamDef && pobCombatCamDef->m_fRotateUpSpeed > 0.0f)
	{
		if(pobCombatCamDef->m_fRotateDownSpeed <= 0.0f)
			pobCombatCamDef->m_fRotateDownSpeed = pobCombatCamDef->m_fRotateUpSpeed;

		// Make our rotation converger
		CConvergerDef obConvergerDef("_RotConverge");
		obConvergerDef.SetSpeed(pobCombatCamDef->m_fRotateUpSpeed*DEG_TO_RAD_VALUE, pobCombatCamDef->m_fRotateDownSpeed*DEG_TO_RAD_VALUE);
		obConvergerDef.SetDamp(pobCombatCamDef->m_fRotateDamp);
		obConvergerDef.SetSpring(pobCombatCamDef->m_fRotateSpring);
		m_pRotationConv = NT_NEW_CHUNK(Mem::MC_CAMERA) CConverger(obConvergerDef);

		// Set the rotation hold time
		m_fRotationHoldTime = pobCombatCamDef->m_fRotationHoldTime;
	}
	else
	{
		// No Rotation converger or holding
		m_pRotationConv = 0;
		m_fRotation = 0.0f;
		m_fRotationHoldTime = 0.0f;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::~MotionController
*
*	DESCRIPTION		cleanup the motion controller
*
***************************************************************************************************/
MotionController::~MotionController()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pPOISmooth);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pPosConv);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pPOITrans);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pRotationConv);
	
//	PointTransform::Destroy(m_pFinalTrans);
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::CalcLastTracked
*
*	DESCRIPTION		track the current POI
*
***************************************************************************************************/
void MotionController::CalcLastTracked(float fTimeDelta)
{
	CPoint obPOI(m_pParent->GetElementManager().GetPointOfInterest());

	//--------------------------------------------------------------------------------------------
	// scee.sbashow: the move into factor is for taking into account 
	// 					whether the POI is moving into or out of the camera's 
	// 					looking direction. The greater m_fMoveIntoFactor, the more effect here.

	const float fPOIHeadingFactor = 
		GetPOIHeadingCompensateFactor();

	if (fPOIHeadingFactor>0.0f)
	{
		const CDirection obCamViewDir = 	m_pParent->GetTransform().GetZAxis();
		const CDirection obAheadVel   = 	m_pParent->GetElementManager().GetPrimaryElement()->GetVelocity();
		const float 	 fAheadvelSqr = 	obAheadVel.LengthSquared();

		if (fAheadvelSqr > (1e-02f*1e-02f))
		{
			const float fSpeed = sqrtf(fAheadvelSqr);
			const CDirection obAheadNorm = obAheadVel*(1.0f/fSpeed);

			const float fCamViewedCompensateFactor = (1.0f+obAheadNorm.Dot(-obCamViewDir))*0.5f*fPOIHeadingFactor;
			//ntPrintf("Using Heading Compensate Factor = %f, scaleb with time step = %f\n",fCamViewedCompensateFactor*fSpeed,fCamViewedCompensateFactor*fSpeed*fTimeDelta);
			obPOI+=obAheadVel*(fCamViewedCompensateFactor*fTimeDelta);
		}
	}

	SetLastTracked(obPOI);

	// feed this into our point transform if we have one
	if(m_pPOITrans)
		SetLastTracked(m_pPOITrans->Update(GetLastTracked(),m_pParent->GetElementManager(), fTimeDelta));

	// feed this into our point smoother if we have one
	if(m_pPOISmooth)
	{
		m_pPOISmooth->Update(GetLastTracked(), fTimeDelta);
		SetLastTracked(m_pPOISmooth->GetTargetMean());
	}
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::ModifyPos
*
*	DESCRIPTION		push back, converge on final point
*
***************************************************************************************************/
CPoint	MotionController::ModifyPos(const CPoint& obPos, float fTimeDelta)
{
	m_obLastUnModified = obPos;

	// First of all, if we're parented to an entity
	//if(m_pParentEntity)
	//{
	//	m_obLastUnModified = m_obLastUnModified * m_pParentEntity->GetRootTransformP()->GetWorldMatrixFast();
	//}

	m_obLastPoint =	m_obLastPartRotation = m_obLastNoRotation = m_obLastUnModified;

	m_bLastUnmodifiedValid = true;
	m_bLastPointValid = true;
	m_bLastNoRotationValid = true;

	// Handle rotations for player visibility
	//////////////////////////////////////////
	if(!m_bRotation || !m_pParent->GetView().IsUsingCombatCameras())
	{
		// We haven't been asked to rotate but we should stay rotated
		// for a while after player visible at zero rotation again
		if(m_fRotationTimeOut > 0.0f)
		{
			m_fRotationTimeOut -= fTimeDelta;
			m_fRotation = m_fLastRotation;
		}
		else
			m_fRotation = 0.0f;
	}
	else
		m_fRotationTimeOut = m_fRotationHoldTime;

	// Reset rotate flag
	m_bRotation = false;

	// Converger
	if(m_pRotationConv)
	{
		m_pRotationConv->Update(m_fRotation, fTimeDelta);
		m_fRotation = m_pRotationConv->GetDamped();
	}
	
	m_fLastRotation = m_fRotation;

	// If we have a rotation value
	if(m_fRotation > 0.0001f)
	{
		// Get a coordinate space local to the POI relative to POI
		CMatrix obPOILocal;
		CCamUtil::CreateFromPoints(obPOILocal, m_pParent->GetElementManager().GetPointOfInterest(), m_obLastPoint);
		CMatrix obInvPOILocal(obPOILocal.GetAffineInverse());

		m_obLastPoint = m_obLastPoint * obInvPOILocal;

		// Construct a rotation matrix about the X-Axis... If my memory of maths serves me...
		float fCos = fcosf(-m_fRotation);
		float fSin = fsinf(-m_fRotation);

		CMatrix obRot(1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, fCos, fSin, 0.0f,
					  0.0f,-fSin, fCos, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f);

		// Construct another matrix for the rotation -3 degrees for rotation hold checks....
		float partRotation = m_fRotation - PI/60.0f; // Expose this param for designers if useful...
		if(partRotation < 0.0f)
			partRotation = 0.0f;
		
		fCos = fcosf(-partRotation);
		fSin = fsinf(-partRotation);

		CMatrix obPartRot(1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, fCos, fSin, 0.0f,
					  0.0f,-fSin, fCos, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f);

		//obRot.Row(0)[0] = 1.0f;	obRot.Row(0)[1] = 0.0f;	obRot.Row(0)[2] = 0.0f;	obRot.Row(0)[3] = 0.0f;
		//obRot.Row(1)[0] = 0.0f;	obRot.Row(1)[1] = fCos;	obRot.Row(1)[2] = fSin;	obRot.Row(1)[3] = 0.0f;
		//obRot.Row(2)[0] = 0.0f;	obRot.Row(2)[1] =-fSin;	obRot.Row(2)[2] = fCos;	obRot.Row(2)[3] = 0.0f;
		//obRot.Row(3)[0] = 0.0f;	obRot.Row(3)[1] = 0.0f;	obRot.Row(3)[2] = 0.0f;	obRot.Row(3)[3] = 1.0f;

		// Perform rotation and convert back to world space co-ords
		m_obLastPoint = m_obLastPoint * obRot;
		m_obLastPartRotation = m_obLastPoint *obPartRot;
		m_obLastPoint = m_obLastPoint * obPOILocal;
		m_obLastPartRotation = m_obLastPartRotation * obPOILocal;
		//ntPrintf("ROT: %.2f\n", m_fRotation * RAD_TO_DEG_VALUE);
	}

	// This is our last position with rotation but no zoom applied to the results
	m_obLastNoZoom = m_obLastPoint;
	m_bLastNoZoomValid = true;

	// has something asked us to move from our current position
	if(m_bReqPush)
	{
		m_bReqPush = false;
		m_obLastPoint += m_obPushOffset;

		// The no-rotation position should include zoom
		m_obLastNoRotation += m_obPushOffset;
		m_obLastPartRotation += m_obPushOffset;
	}

	if(m_pFinalTrans)
		m_obLastPoint = 
			m_pFinalTrans->Update(m_obLastPoint, 
								  m_pParent->GetElementManager(),fTimeDelta);

	// check for a final converge to our target position
	if(m_pPosConv)
		m_obLastPoint = 
			m_pPosConv->Update(m_obLastPoint,fTimeDelta);

	return m_obLastPoint;
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MotionController::Render()
{
	if(m_pPOISmooth)
		m_pPOISmooth->Render();

	if(m_pPOITrans)
		m_pPOITrans->Render();	

//	if(m_pPosConv)
//		m_pPosConv->Render();

	if(m_pFinalTrans)
		m_pFinalTrans->Render();
}


/***************************************************************************************************
*	
*	FUNCTION		MotionController::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MotionController::RenderInfo(int iX, int iY)
{
	UNUSED(iX); UNUSED(iY);
	//if(!m_bRotation)
	//	m_fRotation = 0.0f;
	//CCamUtil::DebugPrintf(iX, iY, "MotionController: (%s) (%.2f)", GetTypeString(m_eTYPE), m_fRotation);
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCFixedPosDef::Create
//!	Create an MCFixedPos from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCFixedPosDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCFixedPos(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCFixedPos::MCFixedPos
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCFixedPos::MCFixedPos(const CameraInterface* pParent, const MCFixedPosDef& def)
:	MotionController(pParent, def),
	m_def(def)
{
}


/***************************************************************************************************
*	
*	FUNCTION		MCFixedPos::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCFixedPos::Update(float fTimeDelta)
{
	return ModifyPos(m_def.m_ptPosition, fTimeDelta);
}


/***************************************************************************************************
*	
*	FUNCTION		MCFixedPos::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCFixedPos::Render()
{
	MotionController::Render();

	CCamUtil::Render_Sphere(m_def.m_ptPosition, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
}




//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCBoomDef::Create
//!	Create an MCBoom from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCBoomDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCBoom(pParent, *this);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCBoom::MCBoom
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
MCBoom::MCBoom(const CameraInterface* pParent, const MCBoomDef& def)
 : MotionController(pParent, def),
   m_def(def)
{
}

/***************************************************************************************************
*	
*	FUNCTION		MCBoom::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCBoom::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);

	CPoint pt = m_def.m_ptPosition;
	if(m_pParentEntity)
	{
		pt = pt * m_pParentEntity->GetRootTransformP()->GetWorldMatrixFast();
	}
	
	CDirection obDir = GetLastTracked() ^ pt;

	if(m_def.m_bBoomIgnoreY)
		obDir.Y() = 0.0f;

	obDir.Normalise();

	CPoint	obNewPos(m_def.m_bBoomReverse ? 
					 pt + (obDir * m_def.m_fDistance) :
					 pt - (obDir * m_def.m_fDistance));

	if(m_def.m_bBoomIgnoreY)
		obNewPos.Y() += m_def.m_fBoomHeight;

	return ModifyPos(obNewPos, fTimeDelta);
}

/***************************************************************************************************
*	
*	FUNCTION		MCBoom::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCBoom::Render()
{
	MotionController::Render();

	CCamUtil::Render_Sphere(GetLastTracked(), 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
	CCamUtil::Render_Sphere(GetLastPoint(), 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
	CCamUtil::Render_Sphere(m_def.m_ptPosition, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
	
	CCamUtil::Render_Line(m_def.m_ptPosition, GetLastPoint(), 1.0f,1.0f,1.0f,1.0f);
	CCamUtil::Render_Line(GetLastTracked(), m_def.m_ptPosition, 1.0f,1.0f,1.0f,1.0f);
}







//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIRelDef::Create
//!	Create an MCPOIRel from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCPOIRelDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCPOIRel(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIRel::MCPOIRel
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCPOIRel::MCPOIRel(const CameraInterface* pParent, const MCPOIRelDef& def)
: MotionController(pParent, def),
  m_def(def)
{
	m_direction = m_def.m_direction;
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIRel::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCPOIRel::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);
	
	return ModifyPos(GetLastTracked() - m_direction, fTimeDelta);
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIRel::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCPOIRel::Render()
{
	MotionController::Render();

	CPoint obNewPos = GetLastTracked() - m_direction;

	CCamUtil::Render_Line(obNewPos, GetLastTracked(), 1.0f,1.0f,1.0f,1.0f);
	CCamUtil::Render_Line(obNewPos, GetLastPoint(), 1.0f,1.0f,1.0f,1.0f);
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIChaseDef::Create
//!	Create an MCPOIChase from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCPOIChaseDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCPOIChase(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIChase::MCPOIChase
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCPOIChase::MCPOIChase(const CameraInterface* pParent, const MCPOIChaseDef& def)
: MotionController(pParent, def),
  m_def(def),
  m_fLongditude(0.f),
  m_fLatitude(0.f),
  m_bInvertX(false),
  m_bInvertY(false)
{
	Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIChase::Reset
*
*	DESCRIPTION		Initialise the motion controller
*
***************************************************************************************************/
void MCPOIChase::Reset()
{
	// TBC. should come from a def struct somewhere
	m_bInvertX   = false;
	m_bInvertY   = true;
	m_bEasyTweak = false;
	m_bFixed     = false;

	MotionController::Reset();

	m_fLatitude = m_def.m_fStartAngle * DEG_TO_RAD_VALUE;
	
	// TBC we need tome kind of player facing direction to init this
	m_fLongditude = 0.0f;

	// we have to set up a last valid pos for us to move from in the first update
	CalcLastTracked(0.0f);

	CPoint obNewPos = GetLastTracked() - (CCamUtil::CartesianFromSpherical(m_fLatitude, m_fLongditude) * m_def.m_fMaxDistance);

	ModifyPos(obNewPos, 0.0f);

	// Reset smoothers and convergers
	if(m_pPOISmooth)
		m_pPOISmooth->Reset();

	if(m_pPosConv)
		m_pPosConv->Reset();

	m_dirLast = GetLastTracked() ^ GetLastPoint();
	m_dirLast.Normalise();
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIChase::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCPOIChase::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);
	
	// get our last world pos, update our lat and long from this and work out where we need to be
	// to maintain our distance from the last tracked pos
	CDirection obDir = GetLastTracked() ^ GetLastPoint();


	// Nasty little fudge for warping the camera, should only be necessary for debug cameras
	// Presumably a different game camera will always be selected after a warp.
	if(obDir.LengthSquared() > (30.0f*30.0f))
		Reset();
	
	
	float fActualDist = ntstd::Min(obDir.Length(), m_def.m_fMaxDistance);
	fActualDist = ntstd::Max(fActualDist, m_def.m_fMinDistance);
	
	obDir.Normalise();
	
	PAD_NUMBER iWhichPad = CamMan_Public::GetDebugCameraPadNumber();
	if(m_bEasyTweak || CInputHardware::Get().GetPad(iWhichPad).GetHeld() & PAD_LEFT)
	{
		// allow the player to adjust these
		float fTiltSpeed = m_def.m_fMoveSpeed * DEG_TO_RAD_VALUE * fTimeDelta;
		CCamUtil::SphericalFromCartesian(obDir, m_fLatitude, m_fLongditude);

		if(m_bInvertX)
			m_fLongditude += CInputHardware::Get().GetPad(iWhichPad).GetAnalogRXFrac() * fTiltSpeed;
		else
			m_fLongditude -= CInputHardware::Get().GetPad(iWhichPad).GetAnalogRXFrac() * fTiltSpeed;
		
		if(m_bInvertY)
			m_fLatitude -= CInputHardware::Get().GetPad(iWhichPad).GetAnalogRYFrac() * fTiltSpeed;
		else
			m_fLatitude += CInputHardware::Get().GetPad(iWhichPad).GetAnalogRYFrac() * fTiltSpeed;
		
		m_fLatitude = ntstd::Clamp(m_fLatitude, -80.0f * DEG_TO_RAD_VALUE, 80.0f * DEG_TO_RAD_VALUE);

		obDir = CCamUtil::CartesianFromSpherical(m_fLatitude, m_fLongditude);
	}
	else if(m_bFixed)// construct an actual position for us to be in
	{
		return ModifyPos(GetLastTracked() - m_dirLast * fActualDist, fTimeDelta);
	}
	
	CPoint obNewPos = GetLastTracked() - obDir * fActualDist;
	m_dirLast = obDir;
	
	return ModifyPos(obNewPos, fTimeDelta);
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIChase::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCPOIChase::Render()
{
	MotionController::Render();

	CPoint obNewPos = GetLastTracked() - (CCamUtil::CartesianFromSpherical(m_fLatitude, m_fLongditude) * m_def.m_fMaxDistance);

	CCamUtil::Render_Line(obNewPos, GetLastTracked(), 1.0f,1.0f,1.0f,1.0f);
	CCamUtil::Render_Line(obNewPos, GetLastPoint(), 1.0f,1.0f,1.0f,1.0f);

	CCamUtil::Render_Sphere(GetLastTracked(), m_def.m_fMaxDistance, 1.0f, 0.0f, 0.0f, 1.0f);
	CCamUtil::Render_Sphere(GetLastTracked(), m_def.m_fMinDistance, 1.0f, 0.0f, 0.0f, 1.0f);
}








//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIStrafeDef::Create
//!	Create an MCPOIStrafe from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCPOIStrafeDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCPOIStrafe(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIStrafe::MCPOIStrafe
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCPOIStrafe::MCPOIStrafe(const CameraInterface* pParent, const MCPOIStrafeDef& def)
 : MotionController(pParent, def),
   m_def(def)
{
	if(m_def.m_pobBaseConvergerDef)
	{
		m_pobLatConv  = NT_NEW_CHUNK(Mem::MC_CAMERA) CConverger(*m_def.m_pobBaseConvergerDef);
		m_pobLongConv = NT_NEW_CHUNK(Mem::MC_CAMERA) CConverger(*m_def.m_pobBaseConvergerDef);
	}
	else
	{
		m_pobLongConv = 0;
		m_pobLatConv = 0;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIStrafe::~MCPOIStrafe
//!	Destructor
//!                                                                                         
//------------------------------------------------------------------------------------------
MCPOIStrafe::~MCPOIStrafe()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA,  m_pobLongConv);
	NT_DELETE_CHUNK(Mem::MC_CAMERA,  m_pobLatConv);
}



/*void MCPOIStrafe::PostConstruct()
{
	if(m_pobBaseConvergerDef)
	{
		m_pobLatConv  = NT_NEW CConverger(*m_pobBaseConvergerDef);
		m_pobLongConv = NT_NEW CConverger(*m_pobBaseConvergerDef);
	}

	MotionController::PostConstruct();
}*/


/***************************************************************************************************
*	
*	FUNCTION		MCPOIStrafe::Reset
*
*	DESCRIPTION		Reset the POI Motion Controller
*
***************************************************************************************************/
void MCPOIStrafe::Reset()
{
	// TBC. should come from a def struct somewhere
	m_bInvertX = false;
	m_bInvertY = true;

	MotionController::Reset();

	m_fLatitude = m_def.m_fStartAngle * DEG_TO_RAD_VALUE;
	
	// TBC we need tome kind of player facing direction to init this
	m_fLongditude = 0.0f;

	m_pobLongConv->Reset();
	m_pobLatConv->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIStrafe::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCPOIStrafe::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);
	
	// update our latitude and long from pad
	float fTiltSpeed = m_def.m_fMoveSpeed * DEG_TO_RAD_VALUE * fTimeDelta;

	if(m_bInvertX)
		m_fLongditude += CInputHardware::Get().GetPad(PAD_0).GetAnalogRXFrac() * fTiltSpeed;
	else
		m_fLongditude -= CInputHardware::Get().GetPad(PAD_0).GetAnalogRXFrac() * fTiltSpeed;

	if(m_bInvertY)
		m_fLatitude -= CInputHardware::Get().GetPad(PAD_0).GetAnalogRYFrac() * fTiltSpeed;
	else
		m_fLatitude += CInputHardware::Get().GetPad(PAD_0).GetAnalogRYFrac() * fTiltSpeed;

	m_fLatitude = ntstd::Clamp(m_fLatitude, -80.0f * DEG_TO_RAD_VALUE, 80.0f * DEG_TO_RAD_VALUE);

	// TBC update our distance on the pad

	// converge to target ideals
	float fLongAct = m_pobLongConv->Update(m_fLongditude, fTimeDelta);
	float fLatAct = m_pobLatConv->Update(m_fLatitude, fTimeDelta);

	// convert to world space vector
	CPoint obNewPos = GetLastTracked() - (CCamUtil::CartesianFromSpherical(fLatAct, fLongAct) * m_def.m_fDistance);

	// ModifyPos() makes sure this isnt going to collide with anything
	CPoint obReturn = ModifyPos(obNewPos, fTimeDelta);

	// now calculate new lat and longs based on our actual positions
	// TBC we cant do this yet, as it sods up position convergence.
//	CDirection obToTarget = GetLastTracked() ^ obReturn;
//	CCamUtil::SphericalFromCartesian(obToTarget, m_fLatitude, m_fLongditude);

	return obReturn;
}


/***************************************************************************************************
*	
*	FUNCTION		MCPOIStrafe::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCPOIStrafe::Render()
{
	MotionController::Render();
	
	CPoint obNewPos = GetLastTracked() - (CCamUtil::CartesianFromSpherical(m_fLatitude, m_fLongditude) * m_def.m_fDistance);

	CCamUtil::Render_Line(obNewPos, GetLastTracked(), 1.0f,1.0f,1.0f,1.0f);
	CCamUtil::Render_Line(obNewPos, GetLastPoint(), 1.0f,1.0f,1.0f,1.0f);
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailNearFarDef::Create
//!	Create an MCRailNearFar from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCRailNearFarDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCRailNearFar(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailNearFar::MCRailNearFar
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCRailNearFar::MCRailNearFar(const CameraInterface* pParent, const MCRailNearFarDef& def)
 : MotionController(pParent, def),
   m_def(def)
{
	m_pCurveRail = NT_NEW_CHUNK(Mem::MC_CAMERA) CCurveRail(*def.m_pobRailDef);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailNearFar::~MCRailNearFar
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
MCRailNearFar::~MCRailNearFar()
{
	//FIX: //NT_DELETE(m_pobRail);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailNearFar::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCRailNearFar::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);

	m_pCurveRail->TrackPoint(GetLastTracked(), fTimeDelta);
	CPoint obNewPos = m_pCurveRail->GetDampedPoint();

	ntPrintf("%.2f, (%.2f, %.2f, %.2f) - %.2f, (%.2f, %.2f, %.2f)\n", m_pCurveRail->GetTargetVal(), m_pCurveRail->GetTargetPoint().X(), m_pCurveRail->GetTargetPoint().Y(), m_pCurveRail->GetTargetPoint().Z(),
																	  m_pCurveRail->GetDampedVal(), m_pCurveRail->GetDampedPoint().X(), m_pCurveRail->GetDampedPoint().Y(), m_pCurveRail->GetDampedPoint().Z());

	return ModifyPos(obNewPos, fTimeDelta);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailNearFar::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCRailNearFar::Render()
{
	MotionController::Render();
	m_pCurveRail->Render();
}


void MCRailNearFar::Reset()
{
	MotionController::Reset();
	m_pCurveRail->Reset();
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailGuideDef::Create
//!	Create an MCRailGuide from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
MotionController* MCRailGuideDef::Create(const CameraInterface* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) MCRailGuide(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailGuide::MCRailGuide
*
*	DESCRIPTION		Construct the MC
*
***************************************************************************************************/
MCRailGuide::MCRailGuide(const CameraInterface* pParent, const MCRailGuideDef& def)
 : MotionController(pParent, def),
   m_def(def),
   m_pobRail(0)
{
	// we must have a point transformer for this to work....
	ntAssert_p(GetPOITrans(), ("MCRailGuide must have a CPointTransform of type GUIDE_CURVE"));
	ntAssert_p(GetPOITrans()->GetTransformType() == PointTransform::GUIDE_CURVE,
				("MCRailGuide must have a CPointTransform of type GUIDE_CURVE"));
	//ntAssert(m_pobRail);

	if(m_def.m_pobRailDef)
		m_pobRail = NT_NEW_CHUNK(Mem::MC_CAMERA) CCurveRail(*m_def.m_pobRailDef);

	m_fLastTarget = 0.f;
}


/*void MCRailGuide::PostConstruct()
{
	if(m_pobRailDef)
		m_pobRail = NT_NEW CCurveRail(*m_pobRailDef);

	MotionController::PostConstruct();

}*/


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailGuide::~MCRailGuide
//!	Destructor
//!                                                                                         
//------------------------------------------------------------------------------------------
MCRailGuide::~MCRailGuide()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobRail);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailGuide::Update
*
*	DESCRIPTION		calc new position
*
***************************************************************************************************/
CPoint	MCRailGuide::Update(float fTimeDelta)
{
	CalcLastTracked(fTimeDelta);

	PTGuide* pPOI = static_cast<PTGuide*>(GetPOITrans());
	ntAssert(pPOI);

	// feed guide value into the rail as our target paramater
	m_fLastTarget = pPOI->GetGuideVal();

	m_pobRail->TrackTarget(m_fLastTarget, GetLastTracked(), fTimeDelta);

	CPoint obNewPos = m_pobRail->GetDampedPoint();

	// offset the result of the camera rail by the distance the player is from the
	// guide, in the direction of the normal of the guide at the closest point.
	if(m_def.m_bOriginOffset)
	{
		ntAssert(pPOI->GetCurve());

		const CCurveInterface* pobGuide = pPOI->GetCurve();

		CPoint obCloseset = pobGuide->GetPoint(m_fLastTarget);

		const CDirection obToTarget = 
			m_pParent->GetElementManager().GetPointOfInterest() ^ obCloseset;

		const CDirection obTangent = 
			pobGuide->Get1stDerivative(m_fLastTarget);

		CDirection obNormal = 		
			obTangent.Cross(CVecMath::GetYAxis());
        obNormal.Normalise();

		// get offset amount
		const CDirection obOffset = 
			obNormal * obNormal.Dot(obToTarget);

		obNewPos += obOffset;
	}
#if 0
	#ifdef MCNTRLER_DEBUG
		Render();
	#endif
#endif
	return ModifyPos(obNewPos, fTimeDelta);
}


/***************************************************************************************************
*	
*	FUNCTION		MCRailGuide::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void MCRailGuide::Render()
{
	MotionController::Render();
	
	CPoint obStart = GetPOITrans()->GetLastTransform();
	CPoint obEnd = m_pobRail->GetTargetPoint();

	CCamUtil::Render_Line(obStart, obEnd, 1.0f, 1.0f, 1.0f, 1.0f);

	m_pobRail->Render();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailGuide::Reset
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
void MCRailGuide::Reset()
{
	MotionController::Reset();
	m_pobRail->Reset();
	m_fLastTarget = 0.0f;
}
