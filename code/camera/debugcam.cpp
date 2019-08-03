//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file debugcam.cpp                                                                      
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/debugcam.h"
#include "camera/smoother.h"
#include "camera/converger.h"
#include "camera/motioncontroller.h"
#include "camera/lookatcontroller.h"
#include "camera/lenscontroller.h"
#include "game/entitymanager.h"
#include "game/shellconfig.h"

//------------------------------------------------------------------------------------------
// Statics
//------------------------------------------------------------------------------------------
static const float f_InitialTopDownDist = 40.f;
static const float f_InitialMaxDistance = 5.f;
static const float f_InitialMinDistance = 4.f;


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::DebugChaseCamera
//!	Allocate resources.
//!
//------------------------------------------------------------------------------------------
DebugChaseCamera::DebugChaseCamera(const CamView& view, DebugCameraTemplate dct) : 
	BasicCamera(dct, view),
	m_bTopDown( false ),
	m_fTopDownDist( f_InitialTopDownDist )
{
	// construct a chase type motion controller
	//--------------------------------------
	static CSmootherDef	obPOISmoothDef("DEBUG");
	obPOISmoothDef.SetNumSamples(20);
	obPOISmoothDef.SetTightness(0.1f);
	obPOISmoothDef.SetLagFrac(0.8f);

	static CConvergerDef obPosConvergerDef("DEBUG");
	obPosConvergerDef.SetSpeed(20.0f);
	obPosConvergerDef.SetSpring(10.0f);
	obPosConvergerDef.SetDamp(0.8f);

	static MCPOIChaseDef MCDef;
	MCDef.m_fMaxDistance = f_InitialMaxDistance;
	MCDef.m_fMinDistance = f_InitialMinDistance;
	MCDef.m_fStartAngle = 10.0f;
	MCDef.m_fMoveSpeed = 180.0f;
	MCDef.m_pPOISmootherDef = &obPOISmoothDef;
	MCDef.m_pPosConvergerDef = &obPosConvergerDef;

	m_pMotionController = NT_NEW_CHUNK( Mem::MC_CAMERA ) MCPOIChase(this, MCDef);
	m_pMotionController->SetRotation(PI);

	// construct a chase type look at controller
	//--------------------------------------
	static LACPOIRelDef LACDef;
	LACDef.m_pobPOISmootherDef = &obPOISmoothDef;

	m_pLookAtController = NT_NEW_CHUNK( Mem::MC_CAMERA ) LACPOIRel(this, LACDef);

	// construct a fixed lens controller
	//-------------------------------------
	static LensControllerDef LCDef;
	LCDef.m_fIdealFOV = 35.0f;

	m_pLensController = NT_NEW_CHUNK( Mem::MC_CAMERA ) LensController(this, LCDef);

	m_fFOV = 35.0f;


	// Finish construction.
	//----------------------
	Reset();
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::Reset
//!	Reset the camera
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::Reset()
{
	const MCPOIChaseDef* pDef = & ((MCPOIChase*)m_pMotionController)->m_def;
	MCPOIChaseDef* pNonConstDef = const_cast<MCPOIChaseDef*>(pDef);  // This is nasty but it's debug code...

	pNonConstDef->m_fMinDistance = f_InitialMaxDistance;
	pNonConstDef->m_fMaxDistance = f_InitialMinDistance;

	m_fTopDownDist = f_InitialTopDownDist;

	m_pMotionController->Reset();
	m_pLookAtController->Reset();
	m_pLensController->Reset();
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::~DebugChaseCamera
//!	Cleanup.
//!
//------------------------------------------------------------------------------------------
DebugChaseCamera::~DebugChaseCamera()
{
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::Update
//!	Compute the new matrix.
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::Update(float fTimeDelta)
{
	if(m_bTopDown)
	{
		CEntity* pPlayer = CEntityManager::Get().GetPlayer();

		const CPoint& pt = pPlayer ? pPlayer->GetPosition() /*pPlayer->GetLocation()*/ : CPoint(CONSTRUCT_CLEAR);

		m_obTransform  = CMatrix(1.f,  0.f,  0.f,  1.f,
								0.f,   0.f,  1.f,  1.f,
								0.f,  -1.f,  .1f,  1.f,
								pt.X(), pt.Y()+m_fTopDownDist, pt.Z(), pt.W());
		m_obLookAt     = pt;
	}
	else
	{
		// read from our config file what to do about X or Y inversion
		((MCPOIChase*)m_pMotionController)->SetInvertX(g_ShellOptions->m_bInvertDebugCamX);
		((MCPOIChase*)m_pMotionController)->SetInvertY(g_ShellOptions->m_bInvertDebugCamY);

		CPoint	obCurrPos		= m_pMotionController->Update(fTimeDelta);
		m_obTransform			= m_pLookAtController->Update(obCurrPos, fTimeDelta);
		m_fFOV					= m_pLensController->Update(m_pMotionController, m_pLookAtController, fTimeDelta);

		// have to calculate our look at position for use by posible transitions
		// doing it this way allows for modification of the transform either in the
		// LAC (via ModifyLookat()) or by the camera's matrix tweaker

		m_obLookAt = CCamUtil::CalcAdjustedLookat(m_obTransform, m_pLookAtController->GetLastTracked());
	}
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::Zoom
//!	Ziin the camera in a distance.
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::Zoom(float f)
{
	if(m_bTopDown)
	{
		m_fTopDownDist += f;
		if(m_fTopDownDist < 5.f)
			m_fTopDownDist = 5.f;
	}
	else
	{
		CDirection dir = m_obTransform.GetTranslation() ^ m_obLookAt;
		dir.Normalise();

		const MCPOIChaseDef* pDef = &((MCPOIChase*)m_pMotionController)->m_def;
		MCPOIChaseDef* pNonConstDef = const_cast<MCPOIChaseDef*>(pDef);  // This is nasty but it's debug code...

		if(pNonConstDef->m_fMinDistance + f >= 0.5f)
		{
			pNonConstDef->m_fMinDistance += f;
			pNonConstDef->m_fMaxDistance += f;
			m_pMotionController->SetPushVec(dir * f);
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::SetZoom
//!	Set the zoom of the camera.
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::SetZoom(float f)
{
	CDirection dir = m_obTransform.GetTranslation() ^ m_obLookAt;
	dir.Normalise();

	float fPush = f - ((MCPOIChase*)m_pMotionController)->m_def.m_fMinDistance - 0.5f;

	//FIX? //m_pMotionC->m_fMinDistance = f - 0.5f;
	//FIX? //m_pMotionC->m_fMaxDistance = f + 0.5f;
	m_pMotionController->SetPushVec(dir * fPush);
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::SetFOV
//!	Set the FOV for the debug camera.
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::SetFOV(float fFOV)
{
	m_pLensController->SetIdealFOV(fFOV);
}


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera::Fix
//!	Fix the direction the camera looks.
//!
//------------------------------------------------------------------------------------------
void DebugChaseCamera::Fix(bool b)
{
	((MCPOIChase*)m_pMotionController)->SetFixedMode(b);
}


