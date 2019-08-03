//*************************************************************************************************
//	
//	TransformController.cpp.
//
//*************************************************************************************************

#include "game/transformcontroller.h"
#include "anim/transform.h"
#include "core/timer.h"
#include "input/inputhardware.h"
#include "gfx/renderer.h"
#include "core/osddisplay.h"
#include "game/tcdebugupdate.h"

//*************************************************************************************************
//	Ctor.
//*************************************************************************************************
CTransformController::CTransformController() 
  : m_obPosition(CONSTRUCT_CLEAR), 
	m_fPhi(0.0f), 
	m_fTheta(0.0f), 
	m_pobTransform(0),
	m_pobDebugUpdateMode( TransformControllerDebugUpdate::Create( TCDUM_STANDARD ) )
{}

//*************************************************************************************************
//	Dtor.
//*************************************************************************************************
CTransformController::~CTransformController()
{
	TransformControllerDebugUpdate::Destroy( m_pobDebugUpdateMode );
	m_pobDebugUpdateMode = NULL;
}

//*************************************************************************************************
//	Return the current debug update mode.
//*************************************************************************************************
TC_DEBUG_UPDATE_MODE CTransformController::GetDebugUpdateMode() const
{
	ntAssert_p( m_pobDebugUpdateMode != NULL, ("Debug update mode should never be NULL here.") );
	return m_pobDebugUpdateMode->GetMode();
}

//*************************************************************************************************
//	Set a new debug update mode.
//*************************************************************************************************
void CTransformController::SetDebugUpdateMode( TC_DEBUG_UPDATE_MODE mode )
{
	TransformControllerDebugUpdate::Destroy( m_pobDebugUpdateMode );
	m_pobDebugUpdateMode = TransformControllerDebugUpdate::Create( mode );

	OSD::Add(OSD::CAMERA, 0xffffffff, "Set \"%s\" debug controller.", m_pobDebugUpdateMode->GetDesc() );
	ntPrintf( "Set \"%s\" debug controller.\n", m_pobDebugUpdateMode->GetDesc() );
}

//*************************************************************************************************
//	Return the string description of the current update mode.
//*************************************************************************************************
const char * const CTransformController::GetDebugUpdateModeDesc() const
{
	if ( m_pobDebugUpdateMode != NULL )
	{
		return m_pobDebugUpdateMode->GetDesc();
	}

	return "";
}

//*************************************************************************************************
//	Advance to the next debug update mode.
//*************************************************************************************************
void CTransformController::AdvanceDebugUpdateMode()
{
	int new_mode_idx( int( GetDebugUpdateMode() ) + 1 );
	if ( new_mode_idx >= NUM_TC_DEBUG_UPDATE_MODES )
	{
		new_mode_idx = 0;
	}

	TC_DEBUG_UPDATE_MODE new_mode = TC_DEBUG_UPDATE_MODE( new_mode_idx );

	SetDebugUpdateMode( new_mode );
}

//*************************************************************************************************
//	
//*************************************************************************************************
void CTransformController::SetTransform(Transform* pobTransform)
{
	// cache the transform
	m_pobTransform = pobTransform;
	
	if(m_pobTransform)
	{
		ResetController( m_pobTransform->GetLocalMatrix() );
	}
}

//*************************************************************************************************
//	
//*************************************************************************************************
void CTransformController::ResetController( const CMatrix& obMat )
{
	// extract the position
	m_obPosition = obMat.GetTranslation();

	// extract the orientation
	CDirection obZAxis = obMat.GetZAxis();
	m_fPhi = fatan2f(obZAxis.X(), obZAxis.Z());
	float fC = fsqrtf(obZAxis.X()*obZAxis.X() + obZAxis.Z()*obZAxis.Z());
	m_fTheta = -fatan2f(obZAxis.Y(), fC);

	// update the transform
	UpdateTransform();
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetPosition
*
*	DESCRIPTION		Sets the camera position.
*
***************************************************************************************************/

void CTransformController::SetPosition(CPoint const& obPosition)
{
	m_obPosition = obPosition;
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetPhi
*
*	DESCRIPTION		Sets the camera phi value in spherical polar coordinates.
*
***************************************************************************************************/

void CTransformController::SetPhi(float fPhi)
{
	m_fPhi = fPhi;
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetTheta
*
*	DESCRIPTION		Sets the camera theta value in spherical polar coordinates.
*
***************************************************************************************************/

void CTransformController::SetTheta(float fTheta)
{
	m_fTheta = fTheta;
}

void CTransformController::DebugUpdateFromPad(PAD_NUMBER ePad)
{
	// use some default values
	float fTimeChange = ( CTimer::Get().GetGameTimeScalar() >= 1.0f ) ? CTimer::Get().GetGameTimeChange() : CTimer::Get().GetSystemTimeChange();
	DebugUpdateFromPad( ePad, fTimeChange*10.0f, fTimeChange*PI/2.0f );
}

void CTransformController::DebugUpdateFromPad( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor )
{
	// Update our rotations/position.
	m_pobDebugUpdateMode->Update( ePad, fMoveFactor, fRotateFactor, m_obPosition, m_pobTransform, m_fPhi, m_fTheta );

	// update the transform
	UpdateTransform();
}

void CTransformController::UpdateTransform()
{
	if(m_pobTransform)
	{
		// construct the orientation
		CQuat obOrientation = CQuat(CDirection(0.0f, 1.0f, 0.0f), m_fPhi)*CQuat(CDirection(1.0f, 0.0f, 0.0f), m_fTheta);
		obOrientation.Normalise();

		// construct the inverse view transform
		m_pobTransform->SetLocalMatrix(CMatrix(obOrientation, m_obPosition));
	}
}
