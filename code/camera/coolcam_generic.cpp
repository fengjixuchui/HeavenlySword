//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Generic.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/coolcam_generic.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"
#include "camera/sceneelementcomponent.h"

#include "core/osddisplay.h"
#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/randmanager.h"
#include "game/inputcomponent.h"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"

//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------

// Cool Cam Parameters
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(GenericCoolCamProps, Mem::MC_CAMERA)
	PUBLISH_VAR_AS(fFOV_Editor,          FOV)
	PUBLISH_VAR_AS(fMinElevation_Editor, ElevationMin)
	PUBLISH_VAR_AS(fMaxElevation_Editor, ElevationMax)
	PUBLISH_VAR_AS(fAttackerPOIBias,     AttackerPOIBias)
	PUBLISH_VAR_AS(fPreferredAngle,      PreferredAngle)
	PUBLISH_VAR_AS(bEnableDoF,           EnableDepthOfField)
	PUBLISH_VAR_AS(fDoFNearOffset,       DepthOfField_NearBlurOffset)
	PUBLISH_VAR_AS(fDoFFarOffset,        DepthOfField_FarBlurOffset)
	PUBLISH_VAR_AS(fDoFNearMin,          DepthOfField_NearBlurMinimum)

	PUBLISH_VAR_AS(fDOFMayaNear,         DepthOfField_Maya_Near_Default)
	PUBLISH_VAR_AS(fDOFMayaFocal,        DepthOfField_Maya_Focal_Default)
	PUBLISH_VAR_AS(fDOFMayaFar,          DepthOfField_Maya_Far_Default)

	DECLARE_POSTCONSTRUCT_CALLBACK(Init) 
	DECLARE_EDITORCHANGEVALUE_CALLBACK(ReInit)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Generate a ranged random number
//------------------------------------------------------------------------------------------
static inline int Rnd(int nL, int nH)
{
	if(nL == nH)
		return nL;
	ntAssert(nH >= nL);

	int nRange = nH - nL;
	return nL + int(grandf(1.0f) * nRange);
}



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CastRayCallBack
//!	This function is used as a callback from CPhysicsWorld::CastRayFiltered
//!	we're ignoring players and enemies...  Just want to see if any scenery
//!	is going to impede the view from our camera of the player or his enemies
//!                                                                                         
//------------------------------------------------------------------------------------------

class CastRayCallBack : public CastRayFilter
{
public:
	bool operator()(CEntity *pobEntity) const
	{
		if(!pobEntity ||(pobEntity->IsPlayer() || pobEntity->IsEnemy()))
		{
			// This isn't an interesting entity
			return false;
		}

		// Stop the raycast here
		return true;
	}
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::CoolCam_Generic
//!	Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
CoolCam_Generic::CoolCam_Generic(const CamView& view)
  :  CoolCamera(view),
     m_pobAttacker(0), 
	 m_pobAttackee(0), 
	 m_obOffset( CONSTRUCT_CLEAR ),
	 m_bFirstFrame( true ),
	 m_obSmoothDef(5, 0.3f, 1.0f),
     m_obSmoothPOI(m_obSmoothDef), 
	 m_obSmoothPOS(m_obSmoothDef),
	 m_bRotating( false ),
	 m_fRadians( 0.f ),
	 m_ptAttacker( CONSTRUCT_CLEAR ),
	 m_ptAttackee( CONSTRUCT_CLEAR )
{
	Reset();	
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::CoolCam_Generic
//!	Parameterised Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
CoolCam_Generic::CoolCam_Generic(const CamView& view, float fTime, 
								 const CEntity *pobAttacker, const CEntity *pobAttackee)
  : CoolCamera(view),
    m_obSmoothDef(5, 0.3f, 1.0f), 
	m_obSmoothPOI(m_obSmoothDef), 
	m_obSmoothPOS(m_obSmoothDef)
{
	Init(fTime, pobAttacker, pobAttackee, PI);
	Reset();	
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::CoolCam_Generic
//!	Resets stuff, well it might do when we have some stuff...
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_Generic::Reset()
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::Init
//!	Initialises the generic cool cam, picking a good view to cut to without walls and stuff 
//! getting in the way.  This one doesn't rotate and is just static - most often used in 
//! conjuction with a transition to achieve a rotation or other effect.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_Generic::Init(float fTime, const CEntity *pobAttacker, const CEntity *pobAttackee)
{
	m_fFOV =  GenericCoolCamProps::Get().fFOV;

	m_pobAttacker = pobAttacker;
	m_pobAttackee = pobAttackee;

	m_obSmoothPOI.Reset();
	m_obSmoothPOS.Reset();

	ResetTime(fTime);
	UpdateTime(0.0f);
	m_bRotating = false;
	m_bFirstFrame = true;

	m_ptAttackee.Clear();
	m_ptAttacker.Clear();

	OSD::Add(OSD::CAMERA, 0xffffffff, "Generic Cool Cam Start.");
	return FindGoodRandomOffset();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::Init
//!	Initialises the generic cool cam, this version rotates.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_Generic::Init(float fTime, const CEntity *pobAttacker, const CEntity *pobAttackee, float fRadians)
{
	m_fFOV =  GenericCoolCamProps::Get().fFOV;

	m_pobAttacker = pobAttacker;
	m_pobAttackee = pobAttackee;

	m_obSmoothPOI.Reset();
	m_obSmoothPOS.Reset();

	m_fRadians = fRadians;

	ResetTime(fTime);
	m_pCurve = 0;

	m_bRotating = true;

	// Randomise the direction of the camera...
	m_obLookAt = m_view.GetElementManager().GetPointOfInterest();

	// Get the position and direction from the attacker as we want to have the camera from behind them
	// to aid with aiming before we throw the object...
	CPoint obPOS = pobAttacker->GetCamPosition();
	CDirection obOldDir;

	if(m_pobAttacker != m_pobAttackee) // HACK! Bad!
	{
		const CInputComponent* pInput = m_pobAttacker->GetInputComponent();

		if(pInput && pInput->GetInputSpeed() > 0.5f)
		{
			obOldDir = pInput->GetInputDir();
			OSD::Add(OSD::CAMERA, 0xffff00ff, "[%.2f, %.2f, %.2f]", obOldDir.X(), obOldDir.Y(), obOldDir.Z());
		}
		else
			m_pobAttacker->GetLookDirection(obOldDir);

		obOldDir.Normalise();
		obPOS -= obOldDir;
	}
	else
		obOldDir = m_obLookAt ^ obPOS;

	// Get polar co-ords for the current camera
	float fX, fY;
	CCamUtil::SphericalFromCartesian(obOldDir, fX, fY);

	fY -= fRadians / 2.0f;
	//fX -= grandf(QUARTER_PI / 4.0f);

	m_obOffset = -CCamUtil::CartesianFromSpherical(fX, fY);
	OSD::Add(OSD::CAMERA, 0xffffffff, "Rotating Cool Cam Start. [%.2f, %.2f]", fX, fY);

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::FindGoodRandomOffset
//!	Finds a nice random offset for the camera without going into walls.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_Generic::FindGoodRandomOffset()
{
	// Randomise the direction of the camera...
	// What's our current direction?
	m_obLookAt = m_view.GetElementManager().GetPointOfInterest();
	CPoint obPOS = CamMan::GetPrimaryView()->GetCurrMatrix().GetTranslation();
	CDirection obOldDir = m_obLookAt ^ obPOS;
	obOldDir.Normalise();

	// Get polar co-ords for the current camera
	float fX, fY;
	CCamUtil::SphericalFromCartesian(obOldDir, fX, fY);

	fX = grandf(GenericCoolCamProps::Get().fMinElevation) + GenericCoolCamProps::Get().fRangeElevation;

	// Cast rays out at PI/10 radians (18 degrees) spacing all about the POI
	for(int iCount = 0; iCount < 19; iCount++)
	{
		float fY2 = fY + (iCount * PI / 10.0f);

		m_obDir[iCount] = -CCamUtil::CartesianFromSpherical(fX, fY2);
		if(m_obDir[iCount].Y() < 0.0f)
			m_obDir[iCount].Y() = 0.0f;
		m_obDir[iCount].Normalise();
		m_obOffset = m_obDir[iCount];

		// Now lets check that this won't put the camera on the other side of
		// a wall or anything...
		m_obLookAt = GetPOI();
		obPOS = GetPOS(m_obLookAt);

		// Zoom back a bit further just to be sure...
		obPOS += m_obDir[iCount] * 2.5f;

		// Would be better to replace the CastRayFiltered !
		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		// [Mus] - What settings for this cast ?
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);


		CastRayCallBack filter; 
		const CEntity* pobEntity = Physics::CPhysicsWorld::Get().CastRayFiltered(m_obLookAt, obPOS, &filter, obFlag);

		// If we hit the player or an enemy then the camera is probably ok
		// If we don't hit anything at all it's also ok as the POI will be
		// between the combatants.  Anything else we hit is likely to obscure
		// the view and not look very nice.
		if(!pobEntity ||(pobEntity->IsPlayer() || pobEntity->IsEnemy()))
			m_bOK[iCount] = true;
		else
			m_bOK[iCount]    = false;
	}

	// Find the extents...
	int iRLimit;
	int iLLimit;
	for(iLLimit = 0; iLLimit < 9; iLLimit++)
	{
		if(!m_bOK[iLLimit])
			break;
	}
	iLLimit--;

	for(iRLimit = 18; iRLimit >= 11; iRLimit--)
	{
		if(!m_bOK[iRLimit])
			break;
	}
	iRLimit++;

	if(iRLimit < 0)
		iRLimit = 0;
	//ntPrintf("Limits LEFT(%d) RIGHT(%d)\n", iLLimit, iRLimit);
	

	///////////////////////////////////////////////////////////////
	// We've found the limits that we can start the camera between.
	// Now pick a direction in our range.
	///////////////////////////////////////////////////////////////
	if(iLLimit < 3)
	{
		// Not enough room on the left...

		if(iRLimit > 17)
		{
			// No decent viewing...  Just stick with original
			m_obOffset = -obOldDir;
			return true;
		}
		else
		{
			// Pick a good vector on the right
			// Between 11(iRLimit) & 17
			int rnd = Rnd(iRLimit, ntstd::Min(17,iRLimit+4));
			m_obOffset = m_obDir[rnd];
			//ntPrintf("Picked %d\n", rnd);
			return true;
		}
	}
	else if(iRLimit > 17)
	{
		// Pick a good vector on the left
		// Between 3 and 9(iLimit)
		int rnd = Rnd(ntstd::Max(3, iLLimit-4), iLLimit);
		m_obOffset = m_obDir[rnd];
		//ntPrintf("Picked %d\n", rnd);
		return true;
	}
	else
	{
		if(grandf(1.0f) < 0.5f)
		{
			// Pick a good vector on the right
			// Between 11(iRLimit) & 17
			int rnd = Rnd(iRLimit, ntstd::Min(17,iRLimit+4));
			m_obOffset = m_obDir[rnd];
			//ntPrintf("Picked %d\n", rnd);
			return true;
		}
		else
		{
			// Pick a good vector on the left
			// Between 3 and 9(iLimit)
			int rnd = Rnd(ntstd::Max(3, iLLimit-4), iLLimit);
			m_obOffset = m_obDir[rnd];
			//ntPrintf("Picked %d\n", rnd);
			return true;
		}
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::HasFinished
//!	Have we finished our CoolCamera?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_Generic::HasFinished() const
{
	return m_bFinished;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::Update
//!	Update the generic cool camera.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_Generic::Update(float fTimeDelta)
{
//	ntPrintf("GCC - ");
	if(m_bRotating)
	{
		float fAngDelta = fTimeDelta * m_fRadians / m_fTotalTime;
		float fX, fY;

		CCamUtil::SphericalFromCartesian(m_obOffset, fX, fY);
		fY += fAngDelta;

		m_obOffset = CCamUtil::CartesianFromSpherical(fX, fY);
		m_obOffset *= 0.5f;

		if(m_fRealTime > m_fTotalTime)
			EndCamera();
	}

	// Get camera position and target
	m_obLookAt = GetPOI();
	m_obSmoothPOI.Update(m_obLookAt, fTimeDelta);
	m_obLookAt = m_obSmoothPOI.GetTargetMean();

	CPoint obPOS = GetPOS(m_obLookAt);
	//m_obSmoothPOS.Update(obPOS, fTimeDelta);
	//obPOS = m_obSmoothPOS.GetTargetMean();

	// Calculate the transform...
	CCamUtil::CreateFromPoints(m_obTransform, obPOS, m_obLookAt);

	// Update the time scalar
	if(m_fTime > m_fTotalTime)
		m_bFinished = true;
	else
        UpdateTime(fTimeDelta);

	if(m_bRotating)
	{
		if(m_fTotalTime < m_fTime)
		{
			EndCamera();
			m_fTimeScalar = 1.0f;
		}
		else
			m_fTimeScalar = ROTATING_CAM_TIME_SCALAR;
	}
	else if(m_bFirstFrame)
		m_bFirstFrame = false;
	else
		m_bFinished = true;


	// Set the DoF parameters to focus on the combatants
	if(GenericCoolCamProps::Get().bEnableDoF)
	{
		m_bUseDoF = true;
		m_fFocalDepth    = CDirection(m_obLookAt ^ obPOS).Length();

		float fDistAttacker = CDirection(m_ptAttacker ^ obPOS).Length();
		float fDistAttackee = CDirection(m_ptAttackee ^ obPOS).Length();

		m_fNearBlurDepth = ntstd::Min(fDistAttacker, fDistAttackee) - GenericCoolCamProps::Get().fDoFNearOffset;
		m_fFarBlurDepth  = ntstd::Max(fDistAttacker, fDistAttackee) + GenericCoolCamProps::Get().fDoFFarOffset;
		m_fNearBlurDepth = ntstd::Max(m_fNearBlurDepth, GenericCoolCamProps::Get().fDoFNearMin);

		//ntPrintf("FOCAL: %.2f (%.2f,%.2f)\n", m_fFocalDepth, m_fNearBlurDepth, m_fFarBlurDepth);
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::GetPOI
//!	Calculates the POI between two combatants
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint CoolCam_Generic::GetPOI()
{
	if(m_pobAttacker)
	{
		m_ptAttacker = m_pobAttacker->GetCamPosition();

		if(m_pobAttackee)
		{
			m_ptAttackee = m_pobAttackee->GetCamPosition();

			return CPoint(m_ptAttacker *      GenericCoolCamProps::Get().fAttackerPOIBias + 
				          m_ptAttackee * (1.f-GenericCoolCamProps::Get().fAttackerPOIBias));
		}
		else
			return m_ptAttacker;
	}
	else
	{
		if(m_pobAttackee)
		{
			m_ptAttackee = m_pobAttackee->GetCamPosition();
			return m_ptAttackee;
		}
		else
			return CPoint(CONSTRUCT_CLEAR);
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::GetPOS
//!	Calculates the position of the camera
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint CoolCam_Generic::GetPOS(const CPoint& obPOI)
{
	// Calculate the Zoomed Position
	CPoint obPOS(obPOI + m_obOffset);
	float fFOV = m_fFOV;
	float fAspect = CamMan::Get().GetPrimaryView()->GetAspectRatio();

	float fZoom = m_view.GetElementManager().CalcCoolCamZoom(obPOS, obPOI, fAspect, fFOV, 
                                                             m_pobAttackee, m_pobAttacker);

	//ntPrintf("TS = %.2f\n", CTimer::Get().GetGameTimeScalar());
	//ntPrintf("POI(%.2f, %.2f, %.2f)\n", obPOI.X(), obPOI.Y(), obPOI.Z(), obPOI.W());
	//ntPrintf("POS(%.2f, %.2f, %.2f)\n", obPOS.X(), obPOS.Y(), obPOS.Z(), obPOS.W());
	//ntPrintf("Z:%.2f\n", fZoom);
	//obPOS += -m_obOffset * fZoom;
	//ntPrintf("END(%.2f, %.2f, %.2f)\n", obPOS.X(), obPOS.Y(), obPOS.Z(), obPOS.W());

	return obPOS - (m_obOffset * fZoom) + m_obOffset;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Generic::RenderDebugInfo
//!	Display some debuging info for the collision detection system.
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _RELEASE
void CoolCam_Generic::RenderDebugInfo()
{
	int iGuide = int( g_VisualDebug->GetDebugDisplayHeight() ) - 50;

	CCamUtil::DebugPrintf(20, iGuide-36, "Active Camera: Generic Cool Cam (ID: %d)", GetID());	

	//for(int i = 0; i < 19; i++)
	//{
	//	if(m_bOK[i])
	//		CCamUtil::Render_Line(GetPOI(), GetPOI()+m_obDir[i]*(float)i, 0.0f, 1.0f, 0.0f, 1.0f);
	//	else
	//		CCamUtil::Render_Line(GetPOI(), GetPOI()+m_obDir[i]*(float)i, 1.0f, 0.0f, 0.0f, 1.0f);
	//}
}
#endif
