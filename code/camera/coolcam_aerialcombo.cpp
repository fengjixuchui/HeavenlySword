//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_AerialCombo.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "camera/coolcam_aerialcombo.h"

#include "camera/sceneelementcomponent.h"
#include "camera/camutils.h"

#include "objectdatabase/dataobject.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/attacks.h" // We use the superstyle safty regions to check the camera is positioned ok.

#include "physics/world.h"

//------------------------------------------------------------------------------------------
// CoolCam_AerialComboDef Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CoolCam_AerialComboDef, Mem::MC_CAMERA)
	PUBLISH_VAR_AS(dCameraOffset, CameraOffset)
	PUBLISH_VAR_AS(dPOIOffset,    POIOffset)
	PUBLISH_VAR_AS(fFOV,          FoV)
	PUBLISH_VAR_AS(bAttackerRelative, AttackerRelative);
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditValue)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Statics
//------------------------------------------------------------------------------------------
CDirection CoolCam_AerialCombo::m_dCameraOffset(-2.f, .5f, -2.5f);
CDirection CoolCam_AerialCombo::m_dPOIOffset(0.f, .5f, .1f);
float      CoolCam_AerialCombo::m_fAerialFOV = 45.f;
bool       CoolCam_AerialCombo::m_bAttackerRelative = false;


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_AerialComboDef::PostConstruct
//!	Post Construction                      
//!                                                                  
//------------------------------------------------------------------------------------------
void CoolCam_AerialComboDef::PostConstruct()
{
	CoolCam_AerialCombo::m_dCameraOffset = dCameraOffset;
	CoolCam_AerialCombo::m_dPOIOffset = dPOIOffset;
	CoolCam_AerialCombo::m_fAerialFOV = fFOV;
	CoolCam_AerialCombo::m_bAttackerRelative = bAttackerRelative;
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_AerialComboDef::EditValue
//!	
//!                                                                  
//------------------------------------------------------------------------------------------
bool CoolCam_AerialComboDef::EditValue(CallBackParameter, CallBackParameter)
{
	CoolCam_AerialCombo::m_dCameraOffset = dCameraOffset;
	CoolCam_AerialCombo::m_dPOIOffset = dPOIOffset;
	CoolCam_AerialCombo::m_fAerialFOV = fFOV;
	CoolCam_AerialCombo::m_bAttackerRelative = bAttackerRelative;
	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_AerialCombo::CoolCam_AerialCombo
//!	Construction    
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_AerialCombo::CoolCam_AerialCombo(const CamView& view, const CEntity& obAttacker, const CEntity& obReceiver)
: CoolCamera(view),
  m_obAttacker(obAttacker), m_obReceiver(obReceiver),
  m_obSmoothPOI(CSmootherDef(5, 0.3f, 1.0f)), 
  m_obSmoothPOS(CSmootherDef(5, 0.3f, 1.0f))
{
	m_iPriority = 200;

	// Set the lens length - this remains constant
	m_fFOV = m_fAerialFOV;//CamMan::Get().GetFOVAngle() * RAD_TO_DEG_VALUE;
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_AerialCombo::~CoolCam_AerialCombo                                                                  
//!	Destruction                                                                      
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_AerialCombo::~CoolCam_AerialCombo()
{
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_AerialCombo::Update                                                                  
//!	Update the Aerial Combat Camera                                                   
//!                                                                  
//------------------------------------------------------------------------------------------
void CoolCam_AerialCombo::Update(float fTimeDelta)
{
	// Get the point of interest and smooth any movement
	m_obLookAt = GetPOI();
	m_obSmoothPOI.Update(m_obLookAt, fTimeDelta);
	m_obLookAt = m_obSmoothPOI.GetTargetMean();

	// Take the offset as relative to the attacker
	CMatrix obAttackerOrientation = m_obAttacker.GetMatrix();
	obAttackerOrientation.SetTranslation(CPoint(0.0f, 0.0f, 0.0f));
	CDirection dCameraOffset = m_bAttackerRelative ? (m_dCameraOffset * obAttackerOrientation) : m_dCameraOffset;

	// Get the world position of the camera
	CPoint ptPOS = m_obLookAt + dCameraOffset;

	CPoint obRayStart( ptPOS );
	CPoint obRayEnd( m_obLookAt );
	float fHitFraction = -1.0f;
	CDirection obHitNormal( CONSTRUCT_CLEAR );
	Physics::RaycastCollisionFlag obCollision;
	obCollision.base = 0;
	obCollision.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	obCollision.flags.i_collide_with = Physics::LARGE_INTERACTABLE_BIT | Physics::SMALL_INTERACTABLE_BIT;
	
	if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ))
	{
		dCameraOffset *= -1;
		ptPOS = m_obLookAt + dCameraOffset;
		obRayStart = ptPOS;
		if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ))
		{
			CMatrix obRot(CONSTRUCT_IDENTITY);
			obRot.SetFromAxisAndAngle( CDirection( 0.0f, 1.0f, 0.0f ), PI * 0.5f );
			dCameraOffset = dCameraOffset * obRot;
			ptPOS = m_obLookAt + dCameraOffset;
			obRayStart = ptPOS;
			if (Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( obRayStart, obRayEnd, fHitFraction, obHitNormal, obCollision ))
			{
				dCameraOffset *= -1;
				ptPOS = m_obLookAt + dCameraOffset;
			}
		}
	}

	m_obSmoothPOS.Update(ptPOS, fTimeDelta);
	ptPOS = m_obSmoothPOS.GetTargetMean();

	m_fFOV = m_fAerialFOV;

	// Calculate the transform...
	CCamUtil::CreateFromPoints(m_obTransform, ptPOS, m_obLookAt);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_AerialCombo::GetPOI                                                             
//!	Get the POI for this cool camera.                                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint CoolCam_AerialCombo::GetPOI() const
{
	// Create a return value
	CPoint obPointOfInterest;

	// Find the best position to look at on the attacker
	obPointOfInterest = m_obAttacker.GetPosition()/*GetLocation()*/;
	if(m_obAttacker.GetSceneElement())
		obPointOfInterest = m_obAttacker.GetSceneElement()->GetPosition();

	// Take the offset as relative to the attacker
	CMatrix obAttackerOrientation = m_obAttacker.GetMatrix();
	obAttackerOrientation.SetTranslation(CPoint(0.0f, 0.0f, 0.0f));
	CDirection dPOIOffset = (m_dPOIOffset * obAttackerOrientation);

	// Add the offset to the point of interest
	obPointOfInterest += dPOIOffset;

	return obPointOfInterest;
}
