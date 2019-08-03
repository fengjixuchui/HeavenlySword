//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Chase.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/coolcam_chase.h"
#include "camera/camutils.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camview.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
// Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CoolCam_ChaseDef, Mem::MC_CAMERA)
  PUBLISH_VAR_AS(dCameraOffset, 		Camera_Offset)
  PUBLISH_VAR_AS(dLookAtOffset, 		LookAt_Offset)
  PUBLISH_VAR_AS(fFOV,		    		FOV)
  PUBLISH_VAR_AS(fMaxAngle,     		MaxAngle)
  PUBLISH_VAR_AS(bEnabled,      		Enabled)
  PUBLISH_PTR_AS(pTimeCurve,    		TimeCurve)
  PUBLISH_VAR_AS(m_fInitAngleCheck,		InitAngleCheck)
  PUBLISH_VAR_AS(m_fInitRangeCheck,		InitRangeCheckSquared)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_Chase::CoolCam_Chase
//!	Construction                                                                      
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_Chase::CoolCam_Chase(const CamView& view, const CEntity& entity, const CoolCam_ChaseDef& def)
	: 
	CoolCamera(view), 
    m_pEntity(&entity),
	m_pEntityRoot(entity.GetHierarchy()->GetRootTransform()),
	m_def(def),
	m_fAngle( 0.f ),
	m_fMaxAngle( 0.f ),
	m_pSmoother( 0 )
{
	ntAssert_p(entity.GetSceneElement(), ("You cannot apply a chase camera to an entity without a scene element."));

	SetTimeCurve( def.pTimeCurve );
	m_fFOV = def.fFOV;
	m_fMaxAngle = def.fMaxAngle * DEG_TO_RAD_VALUE;

	CSmootherDef smootherDef(20, .5f, .25f);
	m_pSmoother = NT_NEW_CHUNK( Mem::MC_CAMERA ) CPointSmoother(smootherDef);

	Init();
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_Chase::~CoolCam_Chase                                                                  
//!	Destruction                                                                      
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_Chase::~CoolCam_Chase()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pSmoother );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Chase::Init                                                                     
//!	Update the Chase Camera                                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_Chase::Init()
{
	// Get the look at target and the ideal position for the camera.
	m_obLookAt   = CalcIdealLookAt();
	CPoint obPos = CalcIdealPos();

	// Difference in both the angle and distance checks 
	const float fCosAngle	= (m_obLookAt / m_obLookAt.Length()).Dot( GetView().GetCurrMatrix().GetZAxis() );
	const float fRange		= (obPos - GetView().GetCurrMatrix().GetTranslation()).LengthSquared();

	if( fCosAngle > m_def.m_fInitAngleCheck && fRange < m_def.m_fInitRangeCheck )
		obPos = GetView().GetCurrMatrix().GetTranslation();

	// Set intial transform
	CCamUtil::CreateFromPoints(m_obTransform, obPos, m_obLookAt);

	// Set the initial angle
	float      fX;
	CDirection dCam = obPos ^ m_obLookAt;
	CCamUtil::SphericalFromCartesian(dCam, fX, m_fAngle);

	// Reset the smoother
	m_pSmoother->Reset();
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_Chase::Update                                                                  
//!	Update the Chase Camera                                                   
//!                                                                  
//------------------------------------------------------------------------------------------
void CoolCam_Chase::Update(float fTimeDelta)
{
	// Get the look at target and the ideal position for the camera.
	m_obLookAt   = CalcIdealLookAt();
	CPoint obPos = CalcIdealPos();

	// Smooth the change in angle.
	CDirection dCam = obPos ^ m_obLookAt;

	float fX, fY;
	CCamUtil::SphericalFromCartesian(dCam, fX, fY);

	// Calculate the delta angle required.
	float fDeltaY = fY - m_fAngle;
	if(fDeltaY > PI)
		fDeltaY -= TWO_PI;
	else if(fDeltaY < -PI)
		fDeltaY += TWO_PI;

	// Apply spring type behaviour
	float fFactor = 16.f - (abs(fDeltaY) / PI) * 14.75f;
	fDeltaY /= fFactor;

	// Limit the maximum change in angle
	fDeltaY = ntstd::Clamp(fDeltaY, -PI*fTimeDelta, PI*fTimeDelta);

	// Set the angle
	m_fAngle += fDeltaY;

	if(m_fAngle > TWO_PI)
		m_fAngle -= TWO_PI;
	else if(m_fAngle < 0.f)
		m_fAngle += TWO_PI;

	// Get the angle of the players forward direction
	const CDirection& dPlayer = m_pEntity->GetMatrix().GetZAxis();
	float fXLook, fYLook;
	CCamUtil::SphericalFromCartesian(dPlayer, fXLook, fYLook);

	if(fYLook > TWO_PI)
		fYLook -= TWO_PI;
	else if(fYLook < 0.f)
		fYLook += TWO_PI;

	float fAngleOffset = fYLook - m_fAngle;

	if(fAngleOffset > TWO_PI)
		fAngleOffset -= TWO_PI;
	else if(fAngleOffset < 0.f)
		fAngleOffset += TWO_PI;

	if(fAngleOffset > PI + m_fMaxAngle)
	{
		m_fAngle = fYLook - m_fMaxAngle - PI;	

		if(m_fAngle > TWO_PI)
			m_fAngle -= TWO_PI;
		else if(m_fAngle < 0.f)
			m_fAngle += TWO_PI;
	}
	else if(fAngleOffset < PI - m_fMaxAngle)
	{
		m_fAngle = fYLook + m_fMaxAngle + PI;

		if(m_fAngle > TWO_PI)
			m_fAngle -= TWO_PI;
		else if(m_fAngle < 0.f)
			m_fAngle += TWO_PI;
	}

	dCam = CCamUtil::CartesianFromSpherical(fX, m_fAngle);
	obPos = m_obLookAt + dCam * m_def.dCameraOffset.Length();

	// Smooth the velocity
	m_pSmoother->Update(obPos, fTimeDelta);

	// Calculate the transform...
	CCamUtil::CreateFromPoints(m_obTransform, obPos, m_obLookAt);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Chase::CalcIdealLookAt                                                                  
//!	Update the Chase Camera                                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
inline CPoint CoolCam_Chase::CalcIdealLookAt() const
{
	return m_pEntity->GetSceneElement()->GetPosition() + m_def.dLookAtOffset * m_pEntityRoot->GetWorldMatrix();
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_Chase::CalcIdealPos                                                                  
//!	Update the Chase Camera                                                   
//!                                                                  
//------------------------------------------------------------------------------------------
inline CPoint CoolCam_Chase::CalcIdealPos() const
{
	CPoint obPos = CPoint(m_obLookAt + m_def.dCameraOffset * m_pEntityRoot->GetWorldMatrix());

	return obPos;
}
