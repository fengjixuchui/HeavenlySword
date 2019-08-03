//------------------------------------------------------------------------------------------
//!
//!	\file coolcam_aftertouch.cpp
//!
//------------------------------------------------------------------------------------------

// General Warning.  The design of the after touch cameras has been changed so many times
// and this code has been iterated so many times that it's a bit of a mess in here, once
// the way that the camera should work has been agreed on then this file really needs a
// serious tidying up.

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/coolcam_aftertouch.h"
#include "camera/converger.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/sceneelementcomponent.h"

#include "core/osddisplay.h"
#include "core/timer.h"
#include "core/visualdebugger.h"
#include "core/user.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/query.h"
#include "anim/animator.h"

#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/world.h"
#include "physics/advancedcharactercontroller.h"

#include "objectdatabase/dataobject.h"

#include "effect/effect_util.h"


//------------------------------------------------------------------------------------------
// Aftertouch Definition Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CoolCam_AfterTouchDef, Mem::MC_CAMERA)
	PUBLISH_VAR_AS(m_fFOV_Multiplier,            FOV_Multiplier)
	PUBLISH_VAR_AS(m_fTimeScalar_Min,            TimeScalar_Min)
	PUBLISH_VAR_AS(m_fTimeScalar_Max,            TimeScalar_Max)
	PUBLISH_VAR_AS(m_fTimeScalar_SlowPeriod,     TimeScalar_SlowPeriod)
	PUBLISH_VAR_AS(m_fTimeScalar_SpeedUpPeriod,  TimeScalar_SpeedUpPeriod)
	PUBLISH_VAR_AS(m_fRollSpeed_Editor,          RollSpeed)
	PUBLISH_VAR_AS(m_fPitch_Editor,              Pitch)
	PUBLISH_VAR_AS(m_dirLookAtOffset,            LookAtOffset)
	PUBLISH_VAR_AS(m_dirPositionOffset,          PositionOffset)
	PUBLISH_VAR_AS(m_bLookAtOffsetEntityRel,     LookAtOffset_EntityRelative)
	PUBLISH_VAR_AS(m_fMaxRoll_Editor,            MaxRoll)
	PUBLISH_VAR_AS(m_fVelocityThreshold,         VelocityThreshold)
	PUBLISH_VAR_AS(m_fResumeThreshold,           ResumeThreshold)
	PUBLISH_VAR_AS(m_fHaltTime,                  HaltTime)
	PUBLISH_VAR_AS(m_fNormalHitHoldTime,         NormalHitHoldTime)
	PUBLISH_VAR_AS(m_fNormalHitTimeScalar,       NormalHitTimeScalar)
	PUBLISH_VAR_AS(m_fEnemyHitHoldTime,          EnemyHitHoldTime)
	PUBLISH_VAR_AS(m_fEnemyHitTimeScalar,        EnemyHitTimeScalar)
	PUBLISH_VAR_AS(m_fReboundHitHoldTime,        ReboundlHitHoldTime)
	PUBLISH_VAR_AS(m_fReboundlHitTimeScalar,     ReboundlHitTimeScalar)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNormalHitZoomOutDist,      0.f, NormalHitZoomOutDist)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fEnemyHitZoomOutDist,       2.f, EnemyHitZoomOutDist)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fReboundlHitZoomOutDist,    0.f, ReboundlHitZoomOutDist)
	PUBLISH_VAR_AS(m_fHitInterpTime,             HitInterpTime)

	PUBLISH_VAR_AS(m_bEntityTimeScaleAdjust,		EntityTimeScaleAdjust )		
	PUBLISH_VAR_AS(m_fEntityTimeScaleSweep,			EntityTimeScaleSweep )			
	PUBLISH_VAR_AS(m_fEntityTimeScaleRange,			EntityTimeScaleRange )			
	PUBLISH_VAR_AS(m_fEntityTimeScaleMin,			EntityTimeScaleMin )		
	PUBLISH_VAR_AS(m_fEntityTimeScaleMax,			EntityTimeScaleMax )		
	PUBLISH_VAR_AS(m_fEntityTimeScaleAcc,			EntityTimeScaleAcc )		
	PUBLISH_VAR_AS(m_fEntityTimeScaleScoreModifier,	EntityTimeScaleScoreModifier )
	PUBLISH_VAR_AS(m_fEntityTimeScaleScoreSmoother,	EntityTimeScaleScoreSmoother )

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStartingBlur, -1.8f,  MotionBlurStart)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNormalBlur,    0.25f, MotionBlurNormal)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fBlurUpTime,    1.0f,   MotionBlurFadeInTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fBlurDownTime,  0.1f,   MotionBlurFadeOutTime)

	// These postconstructs convert degrees into radians.
	// Would be nice to have an angle datatype in XML that handles this automatically.
	DECLARE_POSTCONSTRUCT_CALLBACK(Init)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(ReInit)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::CoolCam_AfterTouch
//!	Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
CoolCam_AfterTouch::CoolCam_AfterTouch(const CamView& view, const CoolCam_AfterTouchDef* pDef, CEntity* pentTarget, const CDirection& dirVel) : 
	CoolCamera(view),
	m_pDef(pDef),
	m_pobEntity(0),
	m_ptLastPos(CONSTRUCT_CLEAR),
	m_fDist(0.f),
	m_obLastVel(CONSTRUCT_CLEAR),
	m_pobSmoothPOI(0),
	m_fLastX(0.f),
	m_fLastY(0.f),
	m_fLastdY(0.f),
	m_pHitEnt(0),
	m_fHitTime(0.f),
	m_fMinTimeScalar(pDef->m_fTimeScalar_Min),
	m_eHitType(HT_NORMAL),
	m_bCloseUp(false),
	m_bHalted(false),
	m_bCanBeResumed(true),
	m_fHaltedTime(0.f),
	m_fRollTarget(0.f),
	m_fRollSpeed(0.f),
	m_fRoll(0.f),
	m_fOrigFOV(0.f),
	m_fTimeScalarScalar(1.0f)
{
	m_fRollSpeed  = m_pDef->m_fRollSpeed;

	m_iPriority   = 10;

	Init(pentTarget, dirVel);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::CoolCam_AfterTouch
//!	Destruction, just call the cleanup method.
//!                                                                                         
//------------------------------------------------------------------------------------------
CoolCam_AfterTouch::~CoolCam_AfterTouch()
{
	CleanUp();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::CleanUp
//!	Delete all the convergers and smoothers.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::CleanUp()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobSmoothPOI);
	m_pobSmoothPOI = 0;
	
	// We've not collided with anyone yet...
	m_pHitEnt = 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::Init
//!	Initialise the camera.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_AfterTouch::Init(CEntity* pobEntity, const CDirection& obVel)
{
	ntAssert(pobEntity);

	// Clean up the convergers and smoothers from the previous 
	CleanUp();

	// Reset Roll
	m_fRoll = m_fRollTarget = 0.0f;
	m_fRollSpeed  = m_pDef->m_fRollSpeed;

	m_fOrigFOV = CamMan::Get().GetPrimaryView()->GetFOVAngle();
	m_fFOV = m_fOrigFOV * m_pDef->m_fFOV_Multiplier;

	m_pobEntity = pobEntity;

	// Calculate the initial cam position
	CPoint obOldPOI = m_view.GetElementManager().GetPointOfInterest();
	m_obLookAt = m_pobEntity->GetLocation();
	
	if(m_pobEntity->GetSceneElement())
		m_obLookAt = m_pobEntity->GetSceneElement()->GetPosition();
	else
		// Use the camera definition instead.
		m_obLookAt += m_pDef->m_dirLookAtOffset * m_pobEntity->GetMatrix();

	m_ptLastPos = CamMan::GetPrimaryView()->GetCurrMatrix().GetTranslation();
	m_fDist = 1.0f;
	m_obLastVel = obVel;
	m_obLastVel.Normalise();

	// Do our rotation in 1 second RT...
	if(!pobEntity->IsCharacter() || !pobEntity->ToCharacter()->IsDead())
		ResetTime(0.0f);
	else
		ResetTime(1.0f);

	if(pobEntity->IsCharacter() && pobEntity->ToCharacter()->IsDead())
	{
		// Calculate the orientations for the original camera and the new camera
		// in polar co-ords so we can converge between the rotatations
		CDirection obDir = m_ptLastPos ^ obOldPOI;
		float fX, fY, fX2, fY2;
		CCamUtil::SphericalFromCartesian(obDir, fX, fY);
		CCamUtil::SphericalFromCartesian(-m_obLastVel, fX2, fY2);

		// Pitch upwards
		fX -= m_pDef->m_fPitch;
		m_fLastX = fX;
		m_fLastY = fY;
	}
	else
	{
		m_fLastX = -1000.0f;
		m_fLastY = -1000.0f;
	}
	
	m_fLastdY = 0.0f;

	// Want to smooth out the jiggly motion of a thrown ragdoll
	CSmootherDef obSmoother("_SMOOTH");
	obSmoother.SetNumSamples(5);
	obSmoother.SetTightness(0.05f);
	obSmoother.SetLagFrac(1.0f);
	m_pobSmoothPOI = NT_NEW_CHUNK( Mem::MC_CAMERA ) CPointSmoother(obSmoother);
	m_pobSmoothPOI->Update(m_obLookAt,0.0f);

	// Very slow for the start
	m_fTimeScalar = m_fMinTimeScalar = m_pDef->m_fTimeScalar_Min;

#ifdef PLATFORM_PS3
	// Enable the aftertouch blur effect
	m_bUseMotionBlur = true;
	m_fMotionBlur    = m_pDef->m_fStartingBlur;
#endif

	OSD::Add(OSD::CAMERA, DC_WHITE, "Aftertouch Cam Start.");
	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::Update
//!	The per frame update.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::Update(float fTimeDelta)
{
	ntAssert(m_pobEntity);

	// -----------------------------------------------------------------------------------
	// If we've hit something then we do a special hit camera
	// -----------------------------------------------------------------------------------
	if(m_pHitEnt)
	{
		// Maintain our old position and look at the object for the duration of the hit timer...
		if(m_fHitTime > fTimeDelta)
			m_fHitTime -= fTimeDelta;
		else if(m_fHitTime > -EPSILON)
		{
			// The hit camera ends this frame
			m_fHitTime = m_pDef->m_fHitInterpTime;
			m_pHitEnt = 0;

			// If it's not a rebound then the aftertouch camera will finish now.
			if(m_eHitType != HT_REBOUND)
				EndCamera();
		}

		// Update the look at target...
		m_ptHitLookAt = m_obLookAt = m_pobEntity->GetLocation();
		m_ptHitCamPos = m_ptLastPos;
		if(m_fHitZoomOutDist > 0.f)
		{
			m_fHitZoomOutTime -= fTimeDelta;
			if(m_fHitZoomOutTime < 0.f)
			{
				m_fHitZoomOutTime = 0.f;
			}

			CDirection dZoom = m_ptHitCamPos ^ m_ptHitLookAt;
			dZoom.Normalise();

			m_ptHitCamPos = m_ptHitCamPos + dZoom * (1.f-CCamUtil::Sigmoid(m_fHitZoomOutTime, m_pDef->m_fEnemyHitHoldTime/2.f)) * m_fHitZoomOutDist;
		}
		
		CCamUtil::CreateFromPoints(m_obTransform, m_ptHitCamPos, m_obLookAt);

#ifdef PLATFORM_PS3
		// ...the blur...
		if(m_fRealTime < m_pDef->m_fBlurDownTime)
		{
			float fBlur = CCamUtil::Sigmoid(m_fRealTime, m_pDef->m_fBlurDownTime);
			fBlur = m_pDef->m_fStartingBlur * fBlur + m_pDef->m_fNormalBlur * (1.f-fBlur);

			m_bUseMotionBlur = true;
			m_fMotionBlur    = fBlur;
		}
		else
		{
			m_bUseMotionBlur = false;
		}
#endif

		// ...and the time.
		UpdateTime(fTimeDelta);
		return;
	}
	if(m_bCloseUp)
	{
		m_obLookAt  = m_ptCloseUpTarg;
		m_ptLastPos = m_ptCloseUpPos;
		CCamUtil::CreateFromPoints(m_obTransform, m_ptLastPos, m_obLookAt);

		m_fTime += fTimeDelta;
		if(m_fTime > EPSILON)
			EndCamera();
		return;
	}

#ifdef PLATFORM_PS3
	// -----------------------------------------------------------------------------------
	// Update the blur effect
	// -----------------------------------------------------------------------------------
	if(m_fTime < m_pDef->m_fBlurUpTime)
	{
		float fBlur = CCamUtil::Sigmoid(m_fRealTime, m_pDef->m_fBlurUpTime);
		fBlur = m_pDef->m_fNormalBlur * fBlur + m_pDef->m_fStartingBlur * (1.f-fBlur);

		m_bUseMotionBlur = true;
		m_fMotionBlur    = fBlur;
	}
	else
	{
		m_bUseMotionBlur = true;
		m_fMotionBlur    = m_pDef->m_fNormalBlur;
	}
#endif

	// -----------------------------------------------------------------------------------
	// Get the velocity of the object
	// -----------------------------------------------------------------------------------
	CDirection newVel;
	
	// Wobble applied to the camera when in flight
	CDirection obWobble(0,0,0);
	
	if(m_pobEntity->GetPhysicsSystem())
	{
		Physics::AdvancedCharacterController* pobCharacter = (Physics::AdvancedCharacterController*) m_pobEntity->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER);

		if (pobCharacter && pobCharacter->IsRagdollActive())
		{
			m_obLookAt = m_pobEntity->GetLocation();
			newVel = m_pobEntity->GetPhysicsSystem()->GetLinearVelocity();
			newVel.Y() = 0.f;
		}
		else
		{
			newVel = CDirection(m_pobEntity->GetPhysicsSystem()->GetLinearVelocity());
			
			// Work out a wobble amount by using the blur amount as a guide
			// Expose these as params...
			const float fAmount = max((m_fMotionBlur+0.5f) /1.5f, 0.f);

			m_obLookAt = m_pobEntity->GetPosition();

			// Make sure we wobble only in the cameras xy plane
			CDirection obN( m_obLookAt - m_obTransform.GetTranslation() );
			obN.Normalise();

			obWobble = CDirection( EffectUtils::RandomPointInCube() );
			obWobble = obWobble - obWobble.Dot(obN)*obN;
			obWobble.Normalise();

			// Scale the wobble down a lot
			obWobble *= 0.005f*fAmount;
		}
	}
	else
	{
		newVel = CDirection(CONSTRUCT_CLEAR);
		m_obLookAt = m_pobEntity->GetCamPosition();
		ntAssert(false);
	}
	

	// -----------------------------------------------------------------------------------
	// Check if we've come to rest and if so deactivate the camera.
	// -----------------------------------------------------------------------------------
	if(m_bHalted)
	{
		if(m_bCanBeResumed && newVel.Length() > m_pDef->m_fResumeThreshold)
		{
			m_bHalted = false;
			OSD::Add(OSD::CAMERA, DC_GREEN, "Resuming Aftertouch Camera");
		}

		// After a certain time halted the camera ends.
		m_fHaltedTime -= fTimeDelta;
		if(m_fHaltedTime <= 0.f)
			EndCamera();

		CCamUtil::CreateFromPoints(m_obTransform, m_ptLastPos, m_obLookAt);
		UpdateTime(fTimeDelta);
		return;
	}
	else
	{
		if(newVel.Length() < m_pDef->m_fVelocityThreshold && m_fTime > 1.f)
		{
			m_bHalted = true;
			m_fHaltedTime = m_pDef->m_fHaltTime;
			OSD::Add(OSD::CAMERA, DC_RED, "Halting Aftertouch Camera");
		}
	}

	// Normalise the direction of travel so we can get the polar coords.
	newVel.Normalise();
	m_obLastVel = newVel;

//#ifndef _RELEASE
//user_code_start(John)
//	ntPrintf("%.2f, %.2f, %.2f --- %.2f, %.2f, %.2f\n", m_obLookAt.X(), m_obLookAt.Y(), m_obLookAt.Z(),
//		                                                newVel.X(), newVel.Y(), newVel.Z());
//user_code_end()
//#endif //_RELEASE

	// -----------------------------------------------------------------------------------
	// Get a polar offset using our velocity
	// -----------------------------------------------------------------------------------
	float fX, fY;
	CCamUtil::SphericalFromCartesian(-m_obLastVel, fX, fY);

	// -----------------------------------------------------------------------------------
	// Get the Pitch and FOV Settings
	// -----------------------------------------------------------------------------------
	float fPitch   = m_pDef->m_fPitch;
	float fFOVMult = m_pDef->m_fFOV_Multiplier;

	// Set the field of view
	m_fFOV = m_fOrigFOV * RAD_TO_DEG_VALUE * fFOVMult;

	// -----------------------------------------------------------------------------------
	// Set the pitch rotation                                                             
	// -----------------------------------------------------------------------------------
	fX -= fPitch;

	// -----------------------------------------------------------------------------------
	// Calculate the travel matrix                                                        
	// -----------------------------------------------------------------------------------
	CDirection dirForward = CCamUtil::CartesianFromSpherical(fX, fY);
	CDirection dirPerp(dirForward.Z(), 0.f, -dirForward.X());
	dirPerp.Normalise();
	CMatrix matTravel;
	matTravel.SetXAxis(dirPerp);
	matTravel.SetZAxis(-dirForward);
	matTravel.SetYAxis(CDirection(0.f,1.f,0.f));

	// -----------------------------------------------------------------------------------
	// Apply Lookat Offsets                                                               
	// -----------------------------------------------------------------------------------
	if(m_pDef->m_bLookAtOffsetEntityRel)
		m_obLookAt += m_pDef->m_dirLookAtOffset * m_pobEntity->GetMatrix();
	else
		m_obLookAt += m_pDef->m_dirLookAtOffset * matTravel;

	m_pobSmoothPOI->Update(m_obLookAt, CTimer::Get().GetSystemTimeChange());
	m_obLookAt = m_pobSmoothPOI->GetTargetMean();


	// Remember these values for next time...
	m_fLastX = fX;
	m_fLastY = fY;

	CDirection dirOffset  = dirForward * m_fDist;
	dirOffset += m_pDef->m_dirPositionOffset * matTravel;

	CPoint newPos = m_obLookAt + dirOffset;

	// Add detection collision... camera can be stopped by invisible walls
	float fHitFraction;
	CDirection obIntersectNormal;
	Physics::RaycastCollisionFlag obFlag;
	obFlag.base = 0; 
	obFlag.flags.i_am = Physics::COOLCAM_AFTERTOUCH_BIT;
	obFlag.flags.i_collide_with = Physics::AI_WALL_BIT;

	if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( 
		m_ptLastPos, newPos, fHitFraction, obIntersectNormal, obFlag))	
	{
		m_bHalted = true;
		m_bCanBeResumed = false;
		m_fHaltedTime = m_pDef->m_fHaltTime;
		OSD::Add(OSD::CAMERA, DC_RED, "Halting Aftertouch Camera");
	}

	//Sigmoid smoothly out of the rebound camera back into the aftertouch camera
	if(m_fHitTime > 0.f)
	{
		m_fHitTime -= fTimeDelta;
		float fSigmoid = CCamUtil::Sigmoid(m_pDef->m_fHitInterpTime-m_fHitTime, m_pDef->m_fHitInterpTime);
		m_obLookAt = fSigmoid*m_obLookAt + (1.f-fSigmoid)*m_ptHitLookAt;
		newPos = fSigmoid*newPos + (1.f-fSigmoid)*m_ptHitCamPos;
	}

	m_ptLastPos = newPos;

	// Generate the transform
	CCamUtil::CreateFromPoints(m_obTransform, m_ptLastPos, m_obLookAt+obWobble);

	// -----------------------------------------------------------------------------------
	// Apply camera roll if necessary                                                     
	// -----------------------------------------------------------------------------------
	if(m_fRoll < m_fRollTarget)
	{
		m_fRoll += m_fRollSpeed*CTimer::Get().GetSystemTimeChange();

		if(m_fRoll>m_fRollTarget)
		{
			m_fRoll=m_fRollTarget;
		}
	}
	else if(m_fRoll > m_fRollTarget)
	{
		m_fRoll -= m_fRollSpeed*CTimer::Get().GetSystemTimeChange(); // fTimeDelta;

		if(m_fRoll<m_fRollTarget)
		{
			m_fRoll=m_fRollTarget;
		}
	}

	if(m_fRoll!=0.0f)
	{
		CMatrix obZRotMat(CQuat(CVecMath::GetZAxis(),-m_fRoll*m_pDef->m_fMaxRoll));

		CPoint obPosition(m_obTransform.GetTranslation());
		
		obZRotMat *= m_obTransform;

		m_obTransform = obZRotMat;

		m_obTransform.SetTranslation(obPosition);
	}

	// -----------------------------------------------------------------------------------
	// Update the camera time scalar                                                      
	// -----------------------------------------------------------------------------------
	UpdateTime(fTimeDelta);

	// If required, does this camera have it's time scalar modified by the presense of other
	// entities... In the case of the archer.
	if( m_pDef->m_bEntityTimeScaleAdjust )
	{
		CEntityQuery			obEntQuery;
		CEQCProximitySegment	obSegmentClause;

		// Set up the query
		obSegmentClause.SetMatrix( m_pobEntity->GetMatrix() );
		obSegmentClause.SetAngle( m_pDef->m_fEntityTimeScaleSweep * DEG_TO_RAD_VALUE );
		obSegmentClause.SetRadius( m_pDef->m_fEntityTimeScaleRange );

		// Add the clause to the query
		obEntQuery.AddClause( obSegmentClause );
	
		// Make sure the entitt is alive
		CEQCHealthLTE obIsAlive(0.0f);
		obEntQuery.AddUnClause( obIsAlive );
	
		// Perform the search
		CEntityManager::Get().FindEntitiesByType( obEntQuery, CEntity::EntType_AI | CEntity::EntType_Unknown );

		// Obtain the results for the search
		const QueryResultsContainerType& obResults = obEntQuery.GetResults();
	
		// The current best matching entity as a target. 
		CEntity* pBestEnt	= 0;
		float	 fBestScore = FLT_MAX;

		for( QueryResultsContainerType::const_iterator obIt = obResults.begin(); 
				obIt != obResults.end(); 
					++obIt )
		{
			// 
			CEntity* pEnt = *obIt;

			// If the entity type is unknown, then make sure it's flagged with the correct properties. 
			if( (pEnt->GetEntType() & CEntity::EntType_Unknown) && !pEnt->GetKeywords().ContainsAny( "SlowAftertouch" ) )
				continue;

			CPoint pt2Target		= pEnt->GetPosition() - m_pobEntity->GetPosition();
			float fDist2Target		= pt2Target.LengthSquared();
			CPoint pt2TargetNorm	= pt2Target / fDist2Target;

			// Generate a score based on the distance to the target and their angle to the parent entity
			float fScore = fDist2Target * (1.0f - pt2TargetNorm.Dot( m_pobEntity->GetMatrix().GetZAxis()));

			// Is this the best entity in the world?
			if( fScore < fBestScore ) 
			{
				pBestEnt = pEnt;
				fBestScore = fScore; 
			}
		}

		// Base some time scaling on the score given
		float fTimeScaleAcc = m_pDef->m_fEntityTimeScaleAcc;
		
		if( pBestEnt )
		{
			fTimeScaleAcc = -(m_fTimeScalarScalar - clamp( fBestScore * m_pDef->m_fEntityTimeScaleScoreModifier, m_pDef->m_fEntityTimeScaleMin, m_pDef->m_fEntityTimeScaleMax )) / m_pDef->m_fEntityTimeScaleScoreSmoother;
		}

		m_fTimeScalarScalar += fTimeScaleAcc * fTimeDelta; 
		m_fTimeScalarScalar = clamp( m_fTimeScalarScalar, 0.0f, 1.0f );

		// Scale the time scalar?
		m_fTimeScalar *= m_fTimeScalarScalar;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::UpdateTime
//!	At the beginning of the camera time is slowed right down while the camera rotates into
//! position then time speeds up to the normal aftertouch speed.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::UpdateTime(float fTimeDelta)
{
	// Update timers
	m_fTime += fTimeDelta;
	m_fRealTime += CTimer::Get().GetSystemTimeChange();

	if(!m_pHitEnt)
	{
		// Time dilation into the throw camera
		if(m_fRealTime < m_pDef->m_fTimeScalar_SlowPeriod)
			m_fTimeScalar = m_fMinTimeScalar;
		else if(m_fRealTime < m_pDef->m_fTimeScalar_SlowPeriod + m_pDef->m_fTimeScalar_SpeedUpPeriod)
		{
			float fScalar = m_fMinTimeScalar + (1.0f-m_fMinTimeScalar) * (m_fRealTime-m_pDef->m_fTimeScalar_SlowPeriod) / m_pDef->m_fTimeScalar_SpeedUpPeriod;
			fScalar *= m_pDef->m_fTimeScalar_Max;
			m_fTimeScalar = ntstd::Clamp(fScalar, m_fMinTimeScalar, m_pDef->m_fTimeScalar_Max);
		}
		else
			m_fTimeScalar = m_pDef->m_fTimeScalar_Max;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::EndCamera
//!	Terminate the camera
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::EndCamera()
{
#ifdef PLATFORM_PS3
	// Disable the aftertouch blur effect
	m_bUseMotionBlur = false;
#endif

	CoolCamera::EndCamera();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::HasFinished
//!	Are we done with this camera?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CoolCam_AfterTouch::HasFinished() const
{
	if(!m_bFinished)
		return false;

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::LookAt
//!	Set the camera to pay some attention to an entity that we've just hit.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::LookAt(CEntity* pEnt, HIT_TYPE eHitType)
{
	// Have we already been activated?
	if(m_pHitEnt && m_eHitType >= eHitType)
		return;

	// Set up the hit info.
	m_pHitEnt = pEnt;
	m_eHitType = eHitType;
	m_fRealTime = 0.f;

	switch(eHitType)
	{
	case HT_NORMAL:
		m_fHitTime = m_pDef->m_fNormalHitHoldTime;
		m_fTimeScalar = m_fMinTimeScalar = (m_pDef->m_fNormalHitTimeScalar > 0.f) ? m_pDef->m_fNormalHitTimeScalar : m_fTimeScalar;
		m_fHitZoomOutDist = m_pDef->m_fNormalHitZoomOutDist;
		m_fHitZoomOutTime = m_fHitTime / 2.f;
		break;
	case HT_ENEMY:
		m_fHitTime = m_pDef->m_fEnemyHitHoldTime;
		m_fTimeScalar = m_fMinTimeScalar = (m_pDef->m_fEnemyHitTimeScalar > 0.f) ? m_pDef->m_fEnemyHitTimeScalar : m_fTimeScalar;
		m_fHitZoomOutDist = m_pDef->m_fEnemyHitZoomOutDist;
		m_fHitZoomOutTime = m_fHitTime / 2.f;
		break;
	case HT_REBOUND:
		m_fHitTime = m_pDef->m_fReboundHitHoldTime;
		m_fTimeScalar = m_fMinTimeScalar = (m_pDef->m_fReboundlHitTimeScalar > 0.f) ? m_pDef->m_fReboundlHitTimeScalar : m_fTimeScalar;
		m_fHitZoomOutDist = m_pDef->m_fReboundlHitZoomOutDist;
		m_fHitZoomOutTime = m_fHitTime / 2.f;
		break;
	}
	
	OSD::Add(OSD::CAMERA, DC_WHITE, "Starting Collision Camera");
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::EndingCloseUp
//!	Setup the ending closeup shot
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_AfterTouch::ActivateEndingCloseUp(CPoint& pt)
{
	const CameraInterface* pLevelCam = m_view.GetLevelCamera();
	if(!pLevelCam)
	{
		return;
	}
	m_ptCloseUpPos = pLevelCam->GetTransform().GetTranslation();
	m_bCloseUp = true;
	m_ptCloseUpTarg = pt;
	m_fFOV = 10.f;
	m_fTime = 0.f;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AfterTouch::RenderDebugInfo
//!	
//!                                                                                         
//------------------------------------------------------------------------------------------
#ifndef _RELEASE
void CoolCam_AfterTouch::RenderDebugInfo()
{
	int iGuide = int(g_VisualDebug->GetDebugDisplayHeight()) - 50;

	CCamUtil::DebugPrintf(20, iGuide-36, "Active Camera: Throw Cool Cam (ID: %d)", GetID());	
}
#endif
