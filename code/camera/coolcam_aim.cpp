//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Aim.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/coolcam_aim.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"

//------------------------------------------------------------------------------------------
// Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CoolCam_AimDef, Mem::MC_CAMERA)
  // Offsets
  PUBLISH_VAR_AS(m_dOffset1,      Offset1)
  PUBLISH_VAR_AS(m_dOffset2,      Offset2)
  PUBLISH_VAR_AS(m_dTargetOffset, TargetOffset)

  PUBLISH_VAR_AS(m_fFOV,		   FOV)
  PUBLISH_VAR_AS(m_fFocalDepth,    FocalDepth)

  // Depth of Field
  PUBLISH_VAR_AS(m_bUseDoF, EnableDepthOfField)
  PUBLISH_VAR_AS(m_fNearOffset, DoF_NearOffset)
  PUBLISH_VAR_AS(m_fFarOffset, DoF_FarOffset)
  PUBLISH_VAR_AS(m_fLerpFactor, DoF_BlendSpeed)
  PUBLISH_VAR_AS(m_fMaxFocalDist, DoF_MaxFocalDistance)

  PUBLISH_PTR_AS(m_pTimeCurve, TimeCurve)

  // Transition Attributes
  PUBLISH_VAR_AS(m_fTransitionTimeIn, TransitionTimeIn)
  PUBLISH_VAR_AS(m_fTransitionTimeOut, TransitionTimeOut)

  PUBLISH_VAR_AS(m_bUpsideDownCam, UpsideDownCam)
END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!
//!	CoolCam_AimDef::CoolCam_AimDef
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoolCam_AimDef::CoolCam_AimDef()
: m_dOffset1(-.5f,.4f,0.f),         // Relative to the character
  m_dOffset2(0.0f,0.0f,-2.0f),      // Relative to the first offset (including pitch rotation)
  m_dTargetOffset(0.0f,0.25f,10.0f) // Relative to the character
{
	m_fFOV					= 20.0f;
	m_fFocalDepth			= 0.0f;
	m_fTransitionTimeIn		= 0.5f;
	m_fTransitionTimeOut	= 1.0f;
	m_bUpsideDownCam		= false;
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Aim::CoolCam_Aim
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoolCam_Aim::CoolCam_Aim(const CamView& view, const CEntity& ent, CoolCam_AimDef& def)
: CoolCamera(view),	
  m_def(def),
  m_parentEnt(ent),
  m_fPitch(0.f),
  m_fFoVBlendTime(0.f),
  m_fFoVBlendTarget(def.m_fFOV),
  m_fFoVBlendOriginal(def.m_fFOV)
{
	m_iPriority   = 2;
	m_fFOV        = def.m_fFOV;
	m_fFocalDepth = def.m_fFocalDepth;
	SetTimeCurve( def.m_pTimeCurve );
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Aim::~CoolCam_Aim
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CoolCam_Aim::~CoolCam_Aim()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Aim::Update
//!	Update the aiming camera.
//!
//------------------------------------------------------------------------------------------
void CoolCam_Aim::Update(float fTimeDelta)
{
	CMatrix matParentWorld = m_parentEnt.GetMatrix();
	CPoint ptParent(m_parentEnt.GetSceneElement()->GetPosition());

	CMatrix matCameraTilt;
	CCamUtil::MatrixFromEuler_XYZ(matCameraTilt, m_fPitch, 0.f, 0.f);
	CMatrix matCameraOrientation = matCameraTilt * matParentWorld;

	CPoint ptCamera(ptParent);
	ptCamera += m_def.m_dOffset1 * matParentWorld;
	ptCamera += m_def.m_dOffset2 * matCameraOrientation;

	m_obLookAt = ptParent + m_def.m_dTargetOffset * matCameraOrientation;
	CCamUtil::CreateFromPoints(m_obTransform, ptCamera, m_obLookAt);

	if( m_def.m_bUpsideDownCam )
	{
		m_obTransform.SetXAxis( m_obTransform.GetXAxis() * -1.f );
		m_obTransform.SetYAxis( m_obTransform.GetYAxis() * -1.f );
	}

	static bool b = false;
	if(b)
	{
		float fFoV = 20.f;
		float fTime = 1.f;
		SetFoV(fFoV, fTime);
	}

	// Apply any blend on the FoV
	if(m_fFoVBlendTime > 0.f)
	{
		m_fFoVBlendTime -= fTimeDelta;
		float fSigmoid = CCamUtil::Sigmoid(max(m_fFoVBlendTime,0.f), m_fFovBlendTotalTime);

		m_fFOV = fSigmoid*m_fFoVBlendOriginal + (1.f-fSigmoid)*m_fFoVBlendTarget;
	}

	// Apply depth of field settings
	if(m_def.m_bUseDoF)
	{
		m_bUseDoF = true; 

		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;

		// Note: This test no longer intersects with HERO otherwise close cool cam's focus on the heroine shoulder 
		// rather than the target. Long term a better solution will be required if we have multiple heroines
		obFlag.flags.i_collide_with = (Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									   Physics::RAGDOLL_BIT						|
									   Physics::SMALL_INTERACTABLE_BIT			|
									   Physics::LARGE_INTERACTABLE_BIT			);

		CDirection dLook = m_obLookAt ^ ptCamera;
		dLook.Normalise();

		CPoint ptFarLook  = ptCamera + dLook * m_def.m_fMaxFocalDist;
		CPoint ptNearLook = ptCamera + dLook * 1.5f;
		
		float fFocalDepth;
		Physics::TRACE_LINE_QUERY stQuery;
		if(Physics::CPhysicsWorld::Get().TraceLine(ptNearLook, ptFarLook, 0, stQuery, obFlag))
			fFocalDepth = CDirection(stQuery.obIntersect ^ ptCamera).Length();
		else
			fFocalDepth = m_def.m_fMaxFocalDist;
		
		m_fFocalDepth    = CMaths::Lerp(m_fFocalDepth, fFocalDepth, m_def.m_fLerpFactor*fTimeDelta);
		m_fNearBlurDepth = m_fFocalDepth - m_def.m_fNearOffset;
		m_fFarBlurDepth  = m_fFocalDepth + m_def.m_fFarOffset;
	}
	else
	{
		m_bUseDoF = false;
	}

	// Some extra debug info
	//ntPrintf("Focus: %.2f (%.2f,%.2f)\n", m_fFocalDepth, m_fNearBlurDepth, m_fFarBlurDepth);
	//ntPrintf("M: (%.2f, %.2f, %.2f, %.2f),(%.2f, %.2f, %.2f, %.2f),(%.2f, %.2f, %.2f, %.2f),(%.2f, %.2f, %.2f, %.2f)\n",
	//matParentWorld.GetXAxis().X(),  matParentWorld.GetXAxis().Y(),  matParentWorld.GetXAxis().Z(),  matParentWorld.GetXAxis().W(),
	//matParentWorld.GetYAxis().X(),  matParentWorld.GetYAxis().Y(),  matParentWorld.GetYAxis().Z(),  matParentWorld.GetYAxis().W(),
	//matParentWorld.GetZAxis().X(),  matParentWorld.GetZAxis().Y(),  matParentWorld.GetZAxis().Z(),  matParentWorld.GetZAxis().W(),
	//matParentWorld.GetTranslation().X(),  matParentWorld.GetTranslation().Y(),  matParentWorld.GetTranslation().Z(),  matParentWorld.GetTranslation().W());
}
