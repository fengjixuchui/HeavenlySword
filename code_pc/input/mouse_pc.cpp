//--------------------------------------------------
//!
//!	\file mouse.cpp
//!	mouse input singleton. valid on PC only....
//!
//--------------------------------------------------

#include "input/mouse.h"
#include "core/visualdebugger.h"
#include "gfx/camera.h" //!< TODO core needs renderer
#include "camera/camman_public.h" //!< TODO core needs camera

//--------------------------------------------------
//!
//! ctor that correctly sets up platform specific bits
//!
//--------------------------------------------------

// c:\work\heavenly_sword\code_pc\input\mouse_pc.cpp(24)
// warning C4355: 'this' : used in base member initializer list
#pragma warning(disable:4355)

MouseInput::MouseInput( int winX, int winY ) : 
	m_Platform( *this ),
	m_winX(winX), 
	m_winY(winY) 
{
}

//--------------------------------------------------
//!
//! Platform spefic constructor
//!
//--------------------------------------------------
MouseInputPlatform::MouseInputPlatform( MouseInput& parent ) :
	m_Parent( parent )
{
}

//--------------------------------------------------
//!
//! PC specific mouse update does nothing as its
//! event driven
//!
//--------------------------------------------------
void MouseInputPlatform::Update()
{
	// intentionally left blank
}

//--------------------------------------------------
//!
//! singleton process function called by reading the windows message que.
//!	\return TRUE if we handled the message
//!
//--------------------------------------------------
bool MouseInputPlatform::ProcMouseInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(hWnd);

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			CVector temp( _R(M_GET_X_COORD(lParam)), _R(M_GET_Y_COORD(lParam)), 0.0f, 0.0f );
			m_Parent.m_mousePos.Set( m_Parent.GetNormalisedCoords( temp ) );
		}
		return true;

	case WM_MOUSEWHEEL:
		{
			m_Parent.m_wheelPos.Set( m_Parent.m_wheelPos.GetCurr() + M_GET_WHEEL_INC(wParam) );
		}
		return true;

	case WM_LBUTTONDOWN:
		m_Parent.m_buttons[MOUSE_LEFT].SetState(true);
		return true;

	case WM_LBUTTONUP:
		m_Parent.m_buttons[MOUSE_LEFT].SetState(false);
		return true;

	case WM_MBUTTONDOWN:
		m_Parent.m_buttons[MOUSE_MID].SetState(true);
		return true;

	case WM_MBUTTONUP:
		m_Parent.m_buttons[MOUSE_MID].SetState(false);
		return true;

	case WM_RBUTTONDOWN:
		m_Parent.m_buttons[MOUSE_RIGHT].SetState(true);
		return true;

	case WM_RBUTTONUP:
		m_Parent.m_buttons[MOUSE_RIGHT].SetState(false);
		return true;

	case WM_XBUTTONDOWN:
		if (M_IS_XBUTTON_1(wParam))
			m_Parent.m_buttons[MOUSE_X1].SetState(true);
		else
			m_Parent.m_buttons[MOUSE_X2].SetState(true);
		return true;

	case WM_XBUTTONUP:
		if (M_IS_XBUTTON_1(wParam))
			m_Parent.m_buttons[MOUSE_X1].SetState(false);
		else
			m_Parent.m_buttons[MOUSE_X2].SetState(false);
		return true;

	default:
		return false;
	}
}