#include "inputsequence.h"
#include "game/inputcomponent.h"
#include <limits>

// Initialise statics.
const float CInputSequence::DURATION_DEFAULT = 0.1f;
const float CInputSequence::GAP_DEFAULT = 0.2f;
const float CInputSequence::GAP_SIMULTANEOUS = 0.0f;
const CDirection CInputSequence::DIRECTION_NONE( 0.0f, 0.0f, 0.0f );

using namespace ntstd;


void CInputSequence::Clear()
{
	Rewind();

	m_bPlaying = false;
	m_fInsertTime = 0.0f;
	m_fDuration = 0.0f;

	m_obButtonTimeLine.clear();
	m_obButtonTimeLine[ -FLT_MAX ] = 0;
	m_obButtonTimeLine[ FLT_MAX ] = 0;

	m_obVButtonTimeLine.clear();
	m_obVButtonTimeLine[ -FLT_MAX ] = 0;
	m_obVButtonTimeLine[ FLT_MAX ] = 0;
}


void CInputSequence::Play()
{
	m_bPlaying = true;
}


void CInputSequence::Stop()
{
	m_bPlaying = false;
}


void CInputSequence::Seek( float fAbsoluteTime )
{
	m_fPosition = fAbsoluteTime;
}


void CInputSequence::Rewind()
{
	m_fPosition = 0.0f;
	m_uiButtonState = 0;
	m_uiVButtonState = 0;
	m_uiLastFrameButtonState = 0;
	m_uiLastFrameVButtonState = 0;
}


float CInputSequence::GetDuration() const
{
	return m_fDuration;
}


bool CInputSequence::Update( float fTimeChange )
{
	if ( !m_bPlaying )
		return false;

	Seek( m_fPosition + fTimeChange );
	if ( m_fPosition > GetDuration() )
		Stop();

	return true;
}


void CInputSequence::Evaluate( CProcessedInput& obInput ) const
{
	m_uiButtonState = EvaluateButtonState( m_fPosition, CC_BUTTONS_ACTUAL );
	m_uiVButtonState = EvaluateButtonState( m_fPosition, CC_BUTTONS_VIRTUAL );

	obInput.m_obInputDir = EvaluateDirection( m_fPosition, CC_L_ANALOG_STICK );
	obInput.m_fSpeed = EvaluateSpeed( m_fPosition, CC_L_ANALOG_STICK );
	
	obInput.m_obInputDirAlt = EvaluateDirection( m_fPosition, CC_R_ANALOG_STICK );
	obInput.m_fSpeedAlt = EvaluateSpeed( m_fPosition, CC_R_ANALOG_STICK );
	
	obInput.m_uiPressed = EvaluatePressed( CC_BUTTONS_ACTUAL );
	obInput.m_uiHeld = EvaluateHeld( CC_BUTTONS_ACTUAL );
	obInput.m_uiReleased = EvaluateReleased( CC_BUTTONS_ACTUAL );
	
	obInput.m_uiVPressed = EvaluatePressed( CC_BUTTONS_VIRTUAL );
	obInput.m_uiVHeld = EvaluateHeld( CC_BUTTONS_VIRTUAL );
	obInput.m_uiVReleased = EvaluateReleased( CC_BUTTONS_VIRTUAL );
	
	obInput.m_bWiggled = EvaluateWiggled( m_fPosition );
	obInput.m_iWiggleSide = EvaluateWiggleSide( m_fPosition );

	m_uiLastFrameButtonState = m_uiButtonState;
	m_uiLastFrameVButtonState = m_uiVButtonState;
}


CDirection CInputSequence::EvaluateDirection( float fTime, CONTROLLER_COMPONENT eComponent ) const
{
	// TODO. We don't need it at present.
	FW_UNUSED( fTime );
	FW_UNUSED( eComponent );
	return DIRECTION_NONE;
}


float CInputSequence::EvaluateSpeed( float fTime, CONTROLLER_COMPONENT eComponent ) const
{
	// TODO. We don't need it at present.
	FW_UNUSED( fTime );
	FW_UNUSED( eComponent );
	return 0.0f;
}


bool CInputSequence::EvaluateWiggled( float fTime ) const
{
	// TODO. We don't need it at present.
	FW_UNUSED( fTime );
	return false;
}


int CInputSequence::EvaluateWiggleSide( float fTime ) const
{
	// TODO. We don't need it at present.
	FW_UNUSED( fTime );
	return 0;
}


u_int CInputSequence::EvaluatePressed( CONTROLLER_COMPONENT eComponent ) const
{
	switch ( eComponent )
	{
	case CC_BUTTONS_ACTUAL:
		return ( m_uiLastFrameButtonState ^ m_uiButtonState ) & m_uiButtonState;

	case CC_BUTTONS_VIRTUAL:
		return ( m_uiLastFrameVButtonState ^ m_uiVButtonState ) & m_uiVButtonState;

	default:
		return 0;
	}
}


u_int CInputSequence::EvaluateHeld( CONTROLLER_COMPONENT eComponent ) const
{
	switch ( eComponent )
	{
	case CC_BUTTONS_ACTUAL:
		return m_uiButtonState;

	case CC_BUTTONS_VIRTUAL:
		return m_uiVButtonState;

	default:
		return 0;
	}
}


u_int CInputSequence::EvaluateReleased( CONTROLLER_COMPONENT eComponent ) const
{
	switch ( eComponent )
	{
	case CC_BUTTONS_ACTUAL:
		return ( m_uiLastFrameButtonState ^ m_uiButtonState ) & m_uiLastFrameButtonState;

	case CC_BUTTONS_VIRTUAL:
		return ( m_uiLastFrameVButtonState ^ m_uiVButtonState ) & m_uiLastFrameVButtonState;

	default:
		return 0;
	}
}


void CInputSequence::Push( ATTACK_MOVE_TYPE eMove, float fRelativeTime, float fDuration )
{
	switch ( eMove )
	{

	// Push.

	case AM_SPEED_FAST:
		Push( AB_ATTACK_FAST, fRelativeTime, fDuration );
		break;

	case AM_SPEED_MEDIUM:
		Push( AB_ATTACK_MEDIUM, fRelativeTime, fDuration );
		break;

	case AM_SPEED_GRAB:
		Push( AB_GRAB, fRelativeTime, fDuration );
		break;

	case AM_ACTION:
		Push( AB_ACTION, fRelativeTime, fDuration );
		break;


	case AM_POWER_FAST:
		Push( AB_PSTANCE, fRelativeTime, fDuration );
		Push( AB_ATTACK_FAST, GAP_SIMULTANEOUS, fDuration );
		break;

	case AM_POWER_MEDIUM:
		Push( AB_PSTANCE, fRelativeTime, fDuration );
		Push( AB_ATTACK_MEDIUM, GAP_SIMULTANEOUS, fDuration );
		break;

	case AM_POWER_GRAB:
		Push( AB_PSTANCE, fRelativeTime, fDuration );
		Push( AB_GRAB, GAP_SIMULTANEOUS, fDuration );
		break;


	case AM_RANGE_FAST:
		Push( AB_RSTANCE, fRelativeTime, fDuration );
		Push( AB_ATTACK_FAST, GAP_SIMULTANEOUS, fDuration );
		break;

	case AM_RANGE_MEDIUM:
		Push( AB_RSTANCE, fRelativeTime, fDuration );
		Push( AB_ATTACK_MEDIUM, GAP_SIMULTANEOUS, fDuration );
		break;

	case AM_RANGE_GRAB:
		Push( AB_RSTANCE, fRelativeTime, fDuration );
		Push( AB_GRAB, GAP_SIMULTANEOUS, fDuration );
		break;


	// Mash.

	case AM_MASH_SPEED_FAST:
		Mash( AB_ATTACK_FAST, fRelativeTime );
		break;

	case AM_MASH_SPEED_MEDIUM:
		Mash( AB_ATTACK_MEDIUM, fRelativeTime );
		break;

	case AM_MASH_SPEED_GRAB:
		Mash( AB_GRAB, fRelativeTime );
		break;

	case AM_MASH_ACTION:
		Mash( AB_ACTION, fRelativeTime );
		break;


	case AM_MASH_POWER_FAST:
		Push( AB_PSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_ATTACK_FAST, GAP_SIMULTANEOUS );
		break;

	case AM_MASH_POWER_MEDIUM:
		Push( AB_PSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_ATTACK_MEDIUM, GAP_SIMULTANEOUS );
		break;

	case AM_MASH_POWER_GRAB:
		Push( AB_PSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_GRAB, GAP_SIMULTANEOUS );
		break;


	case AM_MASH_RANGE_FAST:
		Push( AB_RSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_ATTACK_FAST, GAP_SIMULTANEOUS );
		break;

	case AM_MASH_RANGE_MEDIUM:
		Push( AB_RSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_ATTACK_MEDIUM, GAP_SIMULTANEOUS );
		break;

	case AM_MASH_RANGE_GRAB:
		Push( AB_RSTANCE, fRelativeTime - 1.0f, 1.0f );
		m_fInsertTime += 1.0f;
		Mash( AB_GRAB, GAP_SIMULTANEOUS );
		break;


	// Dodge.

	case AM_DODGE_LEFT:
		Push( AB_DODGE_LEFT, fRelativeTime, fDuration );
		break;

	case AM_DODGE_RIGHT:
		Push( AB_DODGE_RIGHT, fRelativeTime, fDuration );
		break;

	case AM_DODGE_FORWARD:
		Push( AB_DODGE_FORWARD, fRelativeTime, fDuration );
		break;

	case AM_DODGE_BACK:
		Push( AB_DODGE_BACK, fRelativeTime, fDuration );
		break;

	case AM_NONE:
		m_fInsertTime += fRelativeTime;
		break;

	default:
		break;
	}
}


void CInputSequence::Push( VIRTUAL_BUTTON_TYPE eButton, float fRelativeTime, float fDuration )
{
	float fAbsoluteTime = m_fInsertTime + fRelativeTime;
	PressButton( eButton, fAbsoluteTime );
	ReleaseButton( eButton, fAbsoluteTime + fDuration );
	m_fInsertTime = fAbsoluteTime;
}


void CInputSequence::Mash( VIRTUAL_BUTTON_TYPE eButton, float fRelativeTime )
{
	// A "Mash" is achieved by rapidly mashing the button for one second prior to the time you want the mash to register.
	static const u_int PRESSES_PER_SECOND = 5u;

	m_fInsertTime += fRelativeTime;
	float fAbsoluteTime = m_fInsertTime - 1.0f;
	float fPressOrReleaseDuration = 1.0f / static_cast<float>( PRESSES_PER_SECOND * 2 );

	for ( u_int i = 0u; i < PRESSES_PER_SECOND; ++i )
	{
		PressButton( eButton, fAbsoluteTime );
		fAbsoluteTime += fPressOrReleaseDuration;
		ReleaseButton( eButton, fAbsoluteTime );
		fAbsoluteTime += fPressOrReleaseDuration;
	}
}


void CInputSequence::PressButton( VIRTUAL_BUTTON_TYPE eButton, float fAbsoluteTime )
{
	ButtonTimeLine::mapped_type& obButtonState = m_obVButtonTimeLine[ fAbsoluteTime ];
	obButtonState |= 1 << eButton;
	UpdateDuration( fAbsoluteTime );

	// Consider obButtonState = 0001, eButton = 1:
	//   (1 << eButton) == (1 << 1) == 0010
	//   (obButtonState | 0010) == (0001 & 0010) == 0011
}


void CInputSequence::ReleaseButton( VIRTUAL_BUTTON_TYPE eButton, float fAbsoluteTime )
{
	ButtonTimeLine::mapped_type& obButtonState = m_obVButtonTimeLine[ fAbsoluteTime ];
	obButtonState &= ~( 1 << eButton );
	UpdateDuration( fAbsoluteTime );

	// Consider obButtonState = 0111, eButton = 1:
	//   (1 << eButton) == (1 << 1) == 0010
	//   ~0010 == 1101
	//   (obButtonState & 1101) == (0111 & 1101) == 0101
}


void CInputSequence::UpdateDuration()
{
	// There should always be zero-input key-events at FLT_MIN and FLT_MAX.
	// If there are more than two key-events, the duration is the time of the last key-event.	
	m_fDuration = 0.0f;

	if ( m_obVButtonTimeLine.size() > 2 )
	{
		for ( ButtonTimeLine::const_iterator obIter = m_obVButtonTimeLine.begin(); obIter != m_obVButtonTimeLine.end(); ++obIter )
		{
			if ( obIter->first == FLT_MAX )
				break;

			m_fDuration = obIter->first;
		}
	}
}


void CInputSequence::UpdateDuration( float fAbsoluteTime )
{
	if ( fAbsoluteTime > m_fDuration )
		m_fDuration = fAbsoluteTime;
}


u_int CInputSequence::EvaluateButtonState( float fTime, CONTROLLER_COMPONENT eComponent ) const
{
	switch ( eComponent )
	{
	case CC_BUTTONS_ACTUAL:
		return ButtonStateAtTime( fTime, m_obButtonTimeLine );

	case CC_BUTTONS_VIRTUAL:
		return ButtonStateAtTime( fTime, m_obVButtonTimeLine );

	default:
		return 0;
	}
}


u_int CInputSequence::ButtonStateAtTime( float fTime, const ButtonTimeLine& obTimeLine ) const
{
	unsigned short usState = 0;
	for ( ButtonTimeLine::const_iterator obIter = obTimeLine.begin(); obIter != obTimeLine.end(); ++obIter )
	{
		if ( obIter->first > fTime )
			break;

		usState = obIter->second;
	}

	return static_cast< u_int >( usState );
}
