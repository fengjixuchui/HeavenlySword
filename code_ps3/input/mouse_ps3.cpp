//--------------------------------------------------
//!
//!	\file mouse_ps3.cpp
//!	mouse input ps3 specific bits
//!
//--------------------------------------------------

#include "input/mouse.h"
#include <Fp/FpInput/FpInput.h>
#include <Fp/FpInput/FpMouse.h>

//--------------------------------------------------
//!
//! ctor that correctly sets up platform specific bits
//!
//--------------------------------------------------
MouseInput::MouseInput( int winX, int winY ) : 
	m_Platform( *this ),
	m_winX(winX), 
	m_winY(winY) 
{
}

MouseInputPlatform::MouseInputPlatform( MouseInput& parent ) :
	m_Parent( parent )
{
#if defined (PLATFORM_PS3)
	// both mouse and input hardware require the FpInput singleton, so this checks that the other
	// one has initialised it
	if( !FpInput::Exists() )
	{
		// Initialise the input toolkit.
		FpInput::Initialise();
	}
#endif
}

void MouseInputPlatform::Update()
{
	// NOTE: in theory FpInput::Update() needs calling BUT InputHardware does that so I'm assuming that 
	// its already been done this frame...

	// normalised position
	CVector mousePos(FpInput::Get().GetMouse()->GetPointerNdcX(), FpInput::Get().GetMouse()->GetPointerNdcY(), 0.f, 0.f );
	m_Parent.m_mousePos.Set( mousePos );

	// left, mid and right mouse buttons
	m_Parent.m_buttons[ MOUSE_LEFT ].SetState( FpInput::Get().GetMouse()->IsButtonPressed(FpMouse::kButtonLeft) );
	m_Parent.m_buttons[ MOUSE_MID ].SetState( FpInput::Get().GetMouse()->IsButtonPressed(FpMouse::kButtonMiddle) );
	m_Parent.m_buttons[ MOUSE_RIGHT ].SetState( FpInput::Get().GetMouse()->IsButtonPressed(FpMouse::kButtonRight) );

	// currently no wheel or side button support
}
