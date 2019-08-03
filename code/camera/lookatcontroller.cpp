//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file lookatcontroller.cpp                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/lookatcontroller.h"
#include "camera/basiccamera.h"
#include "camera/elementmanager.h"
#include "camera/sceneelementcomponent.h"
#include "camera/smoother.h"
#include "camera/converger.h"
#include "camera/pointtransform.h"
#include "camera/curverail.h"
#include "camera/curves.h"
#include "camera/camman.h"
#include "camera/combatcam.h"

#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// LACFixedPosDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACFixedPosDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	// New
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPosition, CPoint(0.0f, 0.0f, 0.0f), Position)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACFixedDirDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACFixedDirDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	// New
	PUBLISH_VAR_AS(m_obDirection, Direction)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACFixedYPRDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACFixedYPRDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	// New
	PUBLISH_VAR_AS(m_obYawPitchRoll, YawPitchRoll)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACPOIRelDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACPOIRelDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACRotRailDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACRotRailDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	//New
	PUBLISH_PTR_AS(m_pobRail, Rail)
	PUBLISH_PTR_AS(m_pobRotCurve, RotCurve)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACRotGuideDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACRotGuideDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	//New
	PUBLISH_PTR_AS(m_pobRail, Rail)
	PUBLISH_PTR_AS(m_pobRotCurve, RotCurve)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACGuideRailDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACGuideRailDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	//New
	PUBLISH_PTR_AS(m_pobRail, Rail)
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// LACRailDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(LACRailDef, Mem::MC_CAMERA)
	//Parent
	PUBLISH_PTR_AS(m_pobPOISmootherDef, POISmootherDef)
	PUBLISH_PTR_AS(m_pobPOITrans, POITrans)
	PUBLISH_PTR_AS(m_pobOffsetConvergerDef, OffsetConvergerDef)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bPOIFixedOffset, false, POIFixedOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTargetOffset, CDirection(0.0f, 0.0f, 0.0f), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bBound, false, Bound)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMax, CPoint(0.0f, 0.0f, 0.0f), BoundMax)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obBoundMin, CPoint(0.0f, 0.0f, 0.0f), BoundMin)

	//New
	PUBLISH_PTR_AS(m_pobRail, Rail)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LookAtControllerDef::LookAtControllerDef
//!	LookAtControllerDef Constructor
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtControllerDef::LookAtControllerDef() : 
	m_pobPOITrans(0),
	m_pobOffsetConvergerDef(0),
	m_pobPOISmootherDef(0),
	m_bPOIFixedOffset(false),
	m_obTargetOffset(CONSTRUCT_CLEAR),
	m_obBoundMax(CONSTRUCT_CLEAR),
	m_obBoundMin(CONSTRUCT_CLEAR),
	m_bBound(false)
{
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::Reset
*
*	DESCRIPTION		reset the LAC
*
***************************************************************************************************/
void	LookAtController::Reset(bool bRebuildSmoothers)
{
	m_bLastTrackedValid = false;
	m_bLastTransformValid = false;

	if(bRebuildSmoothers)
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobPOISmooth);
		m_pobPOISmooth = 0;
	}

	if(m_pobPOISmooth)
		m_pobPOISmooth->Reset();

	if(m_pobPOITrans)
		m_pobPOITrans->Reset();

	if (m_pobOffsetConverger)
		m_pobOffsetConverger->Reset();

//	if(m_pobRotSmooth)
//		m_pobRotSmooth->Reset();

//	if(m_pobRotConv)
//		m_pobRotConv->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::LookAtController
*
*	DESCRIPTION		Debug constructor for handmade LAC's
*
***************************************************************************************************/
LookAtController::LookAtController(const BasicCamera* pParent, const LookAtControllerDef& def) :
	m_pParent(pParent),
	m_bLastTrackedValid(false),
	m_obLastTracked(CONSTRUCT_CLEAR),
	m_bTargetValid(false),
	m_obTarget(CONSTRUCT_CLEAR),
	m_bLastTransformValid(false),
	m_obLastTransform(CONSTRUCT_CLEAR),
	m_pobPOISmooth(0),
	m_pobPOITrans(0),
	m_bPOIFixedOffset(false),
	m_obTargetOffset(CONSTRUCT_CLEAR),
	m_bBound(false),
	m_obBoundMax(CONSTRUCT_CLEAR),
	m_obBoundMin(CONSTRUCT_CLEAR)
{
	// Welder Attribute Defaults (Interface moved into defintion...)
	if(def.m_pobPOISmootherDef)
		m_pobPOISmooth = NT_NEW_CHUNK(Mem::MC_CAMERA) CPointSmoother(*def.m_pobPOISmootherDef);
	else
		m_pobPOISmooth = 0;

	m_bPOIFixedOffset = def.m_bPOIFixedOffset;
	m_obTargetOffset = def.m_obTargetOffset;

	if (def.m_pobOffsetConvergerDef && 
		def.m_pobOffsetConvergerDef->GetSpeed()>0.0f)
	{
		m_pobOffsetConverger = 
			NT_NEW_CHUNK(Mem::MC_CAMERA) CPointConverger(*def.m_pobOffsetConvergerDef);
	}
	else
	{
		m_pobOffsetConverger = 0;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::~LookAtController
*
*	DESCRIPTION		cleanup the look at controller
*
***************************************************************************************************/
LookAtController::~LookAtController()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobPOISmooth);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobPOITrans);
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobOffsetConverger);
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::CalcLastTracked
*
*	DESCRIPTION		Calc what we're looking at
*
***************************************************************************************************/
void LookAtController::CalcLastTracked(float fTimeChange)
{
	CPoint ptPOI(m_pParent->GetElementManager().GetPointOfInterest());

	// Check Bounding Restraints
	if(CamMan::Get().IsCombatAware())
	{
		CombatCam*                   pCC(m_pParent->GetCombatCam());
		const SceneElementComponent* pPrimary(m_pParent->GetElementManager().GetPrimaryElement());

		if(pCC && pPrimary) 
		{
			float fPOILimit = pCC->GetPOILimit();
			if(fPOILimit >= 0.f)
			{
				CPoint ptPrimary(pPrimary->GetPosition());
				CDirection dPrimary(ptPrimary - ptPOI);

				if(dPrimary.LengthSquared() > fPOILimit*fPOILimit)
				{
					dPrimary.Normalise();
					ptPOI = ptPrimary + dPrimary*fPOILimit;
				}
			}
		}
	}

	SetLastTracked(ptPOI);

	// feed this into our point transform if we have one
	if(m_pobPOITrans)
		SetLastTracked(m_pobPOITrans->Update(GetLastTracked(),m_pParent->GetElementManager(),fTimeChange));
	
	// add a fixed offset if we have one
	//if(m_bPOIFixedOffset && !CamMan::Get().IsCombatAware())
	//	SetLastTracked(GetLastTracked() + m_obTargetOffset*(m_pParent->GetTransform()));

	// scee.sbashow:
	//			Use a point converger to smoothly go from one LAC offset to another (in world space)
	const CDirection obAppliedLocalOffset = 
		(CamMan::Get().IsCombatAware() && m_pParent->GetCombatCam()) ?
			(m_pParent->GetCombatCam()->GetCombatOffset()) :
					(m_bPOIFixedOffset) ?  (m_obTargetOffset) : CDirection(CONSTRUCT_CLEAR);

    if (m_pobOffsetConverger)
	{
		m_pobOffsetConverger->Update(CPoint(obAppliedLocalOffset),fTimeChange);
		SetLastTracked(GetLastTracked()+(CDirection(m_pobOffsetConverger->GetDamped())*(m_pParent->GetTransform())));
	}
	else
	{
		SetLastTracked(GetLastTracked()+(obAppliedLocalOffset*(m_pParent->GetTransform())));
	}

	// This is our LAC target point.  Might not be there yet due to smoothing
	SetTarget(GetLastTracked());

	// feed this into our point smoother if we have one
	if(m_pobPOISmooth)
	{
		m_pobPOISmooth->Update(GetLastTracked(), fTimeChange);
		SetLastTracked(m_pobPOISmooth->GetTargetMean());
	}

	// Clamp to box if required
	if(m_bBound)
	{
		CPoint obPt = GetLastTracked();
		obPt.X() = ntstd::Clamp(obPt.X(), m_obBoundMin.X(), m_obBoundMax.X());
		obPt.Y() = ntstd::Clamp(obPt.Y(), m_obBoundMin.Y(), m_obBoundMax.Y());
		obPt.Z() = ntstd::Clamp(obPt.Z(), m_obBoundMin.Z(), m_obBoundMax.Z());
		SetLastTracked(obPt);
	}
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::ModifyLookat
*
*	DESCRIPTION		Adjust the transform if nessecary
*
***************************************************************************************************/
CMatrix	LookAtController::ModifyLookat(const CMatrix& obMat)
{
	// convert matrix to quat

	// check for smoothing of our rotations..
//	if(m_bUseSteadyFocus)
//		m_pobRotSmooth->Update(obNewDir);
//		obNewDir = m_pobRotSmooth->GetTargetMean();

	// check for a final converge to our target rotation
//	if(m_pobRotConv)
//		obNewDir = m_pobRotConv->Update(obNewDir);

	// convert quat back to matrix
	return obMat;
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LookAtController::Render()
{
	if(m_pobPOITrans)
		m_pobPOITrans->Render();

	if(m_pobPOISmooth)
		m_pobPOISmooth->Render();
}


/***************************************************************************************************
*	
*	FUNCTION		LookAtController::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LookAtController::RenderInfo(int iX, int iY)
{
	UNUSED(iX);
	UNUSED(iY);
	//CCamUtil::DebugPrintf(iX, iY, "LookAtController: %s", GetNameC());
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACFixedPosDef::Create
//!	Create an LACFixedPos from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
class LookAtController* LACFixedPosDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACFixedPos(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedPos::LACFixedPos
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACFixedPos::LACFixedPos(const LACFixedPosDef& def, const BasicCamera* pParent) :
	LookAtController(pParent, def),
	m_def(def)
{
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedPos::Update
*
*	DESCRIPTION		calc our current look at matrix
*
***************************************************************************************************/
CMatrix LACFixedPos::Update(const CPoint& obCurrPos, float fTimeChange)
{
	UNUSED(fTimeChange);

	SetLastTracked(m_def.m_obPosition);
	SetTarget(m_def.m_obPosition);

	CMatrix obTemp;
	CCamUtil::CreateFromPoints(obTemp, obCurrPos, GetLastTracked());

	SetLastTransform(ModifyLookat(obTemp));
	return GetLastTransform();
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedPos::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LACFixedPos::Render()
{
	LookAtController::Render();
	CCamUtil::Render_Sphere(m_def.m_obPosition, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);
}



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACFixedDirDef::Create
//!	Create an LACFixedPos from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACFixedDirDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACFixedDir(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedDir::LACFixedDir
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACFixedDir::LACFixedDir(const LACFixedDirDef& def, const BasicCamera* pParent) :
	LookAtController(pParent, def),
	m_def(def)
{
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedDir::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACFixedDir::Update(const CPoint& obCurrPos, float fTimeChange)
{
	UNUSED(fTimeChange);

	SetLastTracked(obCurrPos + m_def.m_obDirection);
	SetTarget(obCurrPos + m_def.m_obDirection);

	CMatrix obTempMat;
	CCamUtil::CreateFromPoints(obTempMat, obCurrPos, GetLastTracked());
	
	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}




//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACFixedYPRDef::Create
//!	Create an LACFixedPos from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACFixedYPRDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACFixedYPR(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedYPR::LACFixedYPR
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACFixedYPR::LACFixedYPR(const LACFixedYPRDef& def, const BasicCamera* pParent) : 
	LookAtController(pParent, def),
	m_def(def)
{
}


/***************************************************************************************************
*	
*	FUNCTION		LACFixedYPR::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACFixedYPR::Update(const CPoint& obCurrPos, float fTimeChange)
{
	UNUSED(fTimeChange);

	// construct our matrix from YPR
	CMatrix obTempMat(CCamUtil::QuatFromYawPitchRoll(m_def.m_obYawPitchRoll.X(), 
		                                             m_def.m_obYawPitchRoll.Y(), 
													 m_def.m_obYawPitchRoll.Z()), obCurrPos);

	// construct a fake last tracked for transitions to use.
	// NB this is 10 meters infront untill its a mappable parameter
	SetLastTracked(obCurrPos + (obTempMat.GetZAxis() * FOCAL_DISTANCE_HACK));
	SetTarget(GetLastTracked());

	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACPOIRelDef::Create
//!	Create an LACFixedPos from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACPOIRelDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACPOIRel(pParent, *this);
}


/***************************************************************************************************
*	
*	FUNCTION		LACPOIRel::LACPOIRel
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACPOIRel::LACPOIRel(const BasicCamera* pParent, const LACPOIRelDef& def)
: LookAtController(pParent, def)
{
}


/***************************************************************************************************
*	
*	FUNCTION		LACPOIRel::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACPOIRel::Update(const CPoint& obCurrPos, float fTimeChange)
{
	CalcLastTracked(fTimeChange);

	CMatrix obTempMat;
	CCamUtil::CreateFromPoints(obTempMat, obCurrPos, GetLastTracked());
	
	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACRotRailDef::Create
//!	Create an LACRotRail from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACRotRailDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACRotRail(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACRotRail::LACRotRail
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACRotRail::LACRotRail(const LACRotRailDef& def, const BasicCamera* pParent) : 
	LookAtController(pParent, def),
	m_def(def),
	m_fCurrPos(0.f)
{
	Reset();
}

LACRotRail::~LACRotRail()
{
	// NB, the rail will free our m_pobRotCurve for us
	//FIX: //NT_DELETE(m_pobRail);
}

/***************************************************************************************************
*	
*	FUNCTION		LACRotRail::Reset
*
*	DESCRIPTION		reset the LAC
*
***************************************************************************************************/
void	LACRotRail::Reset()
{
	LookAtController::Reset();

	m_fCurrPos =  0.0f;
	m_def.m_pobRail->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		LACRotRail::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACRotRail::Update(const CPoint& obCurrPos, float fTimeChange)
{
	CalcLastTracked(fTimeChange);

	m_def.m_pobRail->TrackPoint(GetLastTracked(), fTimeChange);
	m_fCurrPos = m_def.m_pobRail->GetDampedVal();

	CMatrix obTempMat = m_def.m_pobRotCurve->GetRotation(m_fCurrPos);
	obTempMat.SetTranslation(obCurrPos);
	
	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}

/***************************************************************************************************
*	
*	FUNCTION		LACRotRail::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LACRotRail::Render()
{
	LookAtController::Render();

	m_def.m_pobRail->Render();
}






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACRotGuideDef::Create
//!	Create an LACRotGuide from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACRotGuideDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACRotGuide(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACRotGuide::LACRotGuide
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACRotGuide::LACRotGuide(const LACRotGuideDef& def, const BasicCamera* pParent) : 
	LookAtController(pParent, def),
	m_def(def),
	m_fCurrPos(0.f)
{
	ntAssert_p(GetPOITrans(), ("LACRotGuide must have a CPointTransform of type GUIDE_CURVE"));
	ntAssert_p(GetPOITrans()->GetTransformType() == PointTransform::GUIDE_CURVE,
				("LACRotGuide must have a CPointTransform of type GUIDE_CURVE"));

	Reset();
}

LACRotGuide::~LACRotGuide()
{
	// NB, the rail will free our m_pobRotCurve for us
	//FIX: //NT_DELETE(m_pobRail);
}

/***************************************************************************************************
*	
*	FUNCTION		LACRotGuide::Reset
*
*	DESCRIPTION		reset the LAC
*
***************************************************************************************************/
void	LACRotGuide::Reset()
{
	LookAtController::Reset();

	m_fCurrPos =  0.0f;
	m_def.m_pobRail->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		LACRotGuide::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACRotGuide::Update(const CPoint& obCurrPos, float fTimeChange)
{
	CalcLastTracked(fTimeChange);

	// feed guide value into the rail as our target paramater
	PTGuide* pPOI = static_cast<PTGuide*>(GetPOITrans());
	m_fCurrPos = pPOI->GetGuideVal();
	m_def.m_pobRail->TrackTarget(m_fCurrPos, GetLastTracked(), fTimeChange);

	// now retrive the damped converged value and use that as our rotation
	m_fCurrPos = m_def.m_pobRail->GetDampedVal();

	CMatrix obTempMat = m_def.m_pobRotCurve->GetRotation(m_fCurrPos);
	obTempMat.SetTranslation(obCurrPos);

	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}

/***************************************************************************************************
*	
*	FUNCTION		LACRotGuide::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LACRotGuide::Render()
{
	LookAtController::Render();

	m_def.m_pobRail->Render();
}




//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACGuideRailDef::Create
//!	Create an LACGuideRail from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACGuideRailDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACGuideRail(*this, pParent);
}



/***************************************************************************************************
*	
*	FUNCTION		LACGuideRail::LACGuideRail
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACGuideRail::LACGuideRail(const LACGuideRailDef& def, const BasicCamera* pParent)
: LookAtController(pParent, def),
  m_def(def),
  m_fCurrPos(0.f)
{
	ntAssert_p(GetPOITrans(), ("LACGuideRail must have a CPointTransform of type GUIDE_CURVE"));
	ntAssert_p(GetPOITrans()->GetTransformType() == PointTransform::GUIDE_CURVE,
				("LACRotGuide must have a CPointTransform of type GUIDE_CURVE"));

	Reset();
}

LACGuideRail::~LACGuideRail()
{
	//FIX: //NT_DELETE(m_pobRail);
}

/***************************************************************************************************
*	
*	FUNCTION		LACGuideRail::Reset
*
*	DESCRIPTION		reset the LAC
*
***************************************************************************************************/
void	LACGuideRail::Reset()
{
	LookAtController::Reset();

	m_fCurrPos =  0.0f;
	m_def.m_pobRail->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		LACGuideRail::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACGuideRail::Update(const CPoint& obCurrPos, float fTimeChange)
{
	CalcLastTracked(fTimeChange);

	// feed guide value into the rail as our target paramater
	PTGuide* pPOI = static_cast<PTGuide*>(GetPOITrans());
	m_fCurrPos = pPOI->GetGuideVal();
	m_def.m_pobRail->TrackTarget(m_fCurrPos, GetLastTracked(), fTimeChange);

	// now retrive the damped converged positon and use that as our rotation target
	SetLastTracked(m_def.m_pobRail->GetDampedPoint());
	SetTarget(GetLastTracked());

	CMatrix obTempMat;
	CCamUtil::CreateFromPoints(obTempMat, obCurrPos, GetLastTracked());

	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}

/***************************************************************************************************
*	
*	FUNCTION		LACGuideRail::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LACGuideRail::Render()
{
	LookAtController::Render();

	m_def.m_pobRail->Render();
}









//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LACRailDef::Create
//!	Create an LACRail from this definition
//!                                                                                         
//------------------------------------------------------------------------------------------
LookAtController* LACRailDef::Create(const BasicCamera* pParent)
{
	return NT_NEW_CHUNK(Mem::MC_CAMERA) LACRail(*this, pParent);
}


/***************************************************************************************************
*	
*	FUNCTION		LACRail::LACRail
*
*	DESCRIPTION		Construct the LAC
*
***************************************************************************************************/
LACRail::LACRail(const LACRailDef& def, const BasicCamera* pParent)
 : LookAtController(pParent, def),
   m_def(def)
{
	Reset();
}

LACRail::~LACRail()
{
	//FIX: //NT_DELETE(m_pobRail);
}

/***************************************************************************************************
*	
*	FUNCTION		LACRail::Reset
*
*	DESCRIPTION		reset the LAC
*
***************************************************************************************************/
void	LACRail::Reset()
{
	LookAtController::Reset();

	m_def.m_pobRail->Reset();
}


/***************************************************************************************************
*	
*	FUNCTION		LACRail::Update
*
*	DESCRIPTION		calc our current lookat matrix
*
***************************************************************************************************/
CMatrix LACRail::Update(const CPoint& obCurrPos, float fTimeChange)
{
	CalcLastTracked(fTimeChange);

	// feed our POI position into our rail
	m_def.m_pobRail->TrackPoint(GetLastTracked(), fTimeChange);

	// now retrive the damped converged positon and use that as our rotation target
	SetLastTracked(m_def.m_pobRail->GetDampedPoint());
	SetTarget(GetLastTracked());

	CMatrix obTempMat;
	CCamUtil::CreateFromPoints(obTempMat, obCurrPos, GetLastTracked());

	SetLastTransform(ModifyLookat(obTempMat));
	return GetLastTransform();
}

/***************************************************************************************************
*	
*	FUNCTION		LACRail::Render
*
*	DESCRIPTION		render stuff
*
***************************************************************************************************/
void	LACRail::Render()
{
	LookAtController::Render();

	m_def.m_pobRail->Render();
}
