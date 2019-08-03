//--------------------------------------------------
//!
//!	\file mouse.h
//!	mouse input singleton. valid on PC only....
//!
//--------------------------------------------------

#ifndef INPUT_MOUSE_PC_H
#define INPUT_MOUSE_PC_H

#include "core/timer.h"

// these seem to work on XP regardless of what the defines are, so we'll use them
#if (_WIN32_WINNT < 0x0400) || (_WIN32_WINDOWS <= 0x0400 )
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif

#if (_WIN32_WINNT < 0x0500 )
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D
#endif

// handy macros
#define M_GET_X_COORD(l)			static_cast<short>(LOWORD(l))
#define M_GET_Y_COORD(l)			static_cast<short>(HIWORD(l))
#define M_GET_WHEEL_INC(w)			static_cast<int>(GET_WHEEL_DELTA_WPARAM(w)/WHEEL_DELTA)

#define M_IS_XBUTTON_1(w)			(static_cast<short>(HIWORD(w)) == 1)
#define M_IS_XBUTTON_2(w)			(static_cast<short>(HIWORD(w)) == 2)

// forward decl
class MouseInput;

//--------------------------------------------------
//!
//!	MouseInputPlatform
//! PC specific bits of MouseInput
//!
//--------------------------------------------------
class	MouseInputPlatform
{
public:
	MouseInputPlatform( MouseInput& parent );

	void Update();

	// update state of the mouse
	bool	ProcMouseInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	MouseInput& m_Parent;
};

#endif _MOUSE_H