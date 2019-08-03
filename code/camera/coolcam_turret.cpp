//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Turret.cpp
//!
//------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////
// Required Includes
////////////////////////////////////////////////////////////////////////////////////////////
#include "camera/coolcam_turret.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"

#include "anim/hierarchy.h"
#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Turret::CoolCam_Turret
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoolCam_Turret::CoolCam_Turret(const CamView& view) : 
	CoolCamera(view),
	m_pTurretTransform( 0 ),
	m_dOff( CONSTRUCT_CLEAR )
{
	m_fFOV = 45.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Turret::~CoolCam_Turret
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CoolCam_Turret::~CoolCam_Turret()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Turret::Init
//!	Initialise the turret cool camera.
//!
//! pTurret - Turret Entity, pcTransform - Name of the transform to attach camera too
//! fYOff, fZOff - Offset values, should change to take a vector...
//!
//------------------------------------------------------------------------------------------
void CoolCam_Turret::Init(CEntity* pTurret, CHashedString pcTransform, const CDirection& dOff)
{
	ntAssert(pTurret); 
	ntAssert(pTurret->GetHierarchy());

	m_pTurretTransform = pTurret->GetHierarchy()->GetTransform(pcTransform);
	
	m_dOff = dOff; 
}

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Turret::Update
//!	Update the turret camera
//!
//------------------------------------------------------------------------------------------
void CoolCam_Turret::Update(float)
{
	// Get camera position and target
	CPoint pos = m_pTurretTransform->GetWorldTranslation();
	pos += m_dOff * m_pTurretTransform->GetWorldMatrix();

	m_obLookAt = pos + m_pTurretTransform->GetWorldMatrix().GetZAxis();

	// Calculate the transform...
	CCamUtil::CreateFromPoints(m_obTransform, pos, m_obLookAt);	

	// Aiming sight
#ifdef _DEBUG
	if(CamMan::GetPrimaryView()->GetActiveCameraID() == GetID() &&
	  !CamMan::GetPrimaryView()->IsTransitionActive())
	{
		float fX = g_VisualDebug->GetDebugDisplayWidth()*0.5f;
		float fY = g_VisualDebug->GetDebugDisplayHeight()*0.5f-6.0f;
		
		g_VisualDebug->Printf2D( fX, fY, 0xffff7777, DTF_ALIGN_HCENTRE, "X");
	}
#endif
}
