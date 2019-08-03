//--------------------------------------------------
//!
//!	\file mouse.cpp
//!	mouse input singleton.
//!
//--------------------------------------------------

#include "input/mouse.h"
#include "core/visualdebugger.h"
#include "gfx/camera.h" //!< TODO core needs renderer
#include "camera/camman.h" //!< TODO core needs camera
#include "camera/camview.h" //!< TODO core needs camera
#include "camera/camman_public.h" //!< TODO core needs camera

const float MouseButton::fCLICK_TIME = 10.0f / 60.0f;

//--------------------------------------------------
//!
//! update the state of a mouse button
//!
//--------------------------------------------------
void	MouseButton::Update( const CVector& pos )
{
	if (GetPressed())
	{
		m_lastDown = m_lastUp = pos;

		if ((CTimer::Get().GetSystemTime() - m_fPressTime) < fCLICK_TIME)
			m_bDBLClick = true;

		m_fPressTime = _R(CTimer::Get().GetSystemTime());
	}
	else
	{
		m_bDBLClick = false;
		if (GetHeld())
			m_lastUp = pos;
	}	
}
const float MouseInput::fINACTIVE_TIME = 1.0f;

//--------------------------------------------------
//!
//! update states of buttons, record current velocities etc
//!
//--------------------------------------------------
void MouseInput::Initialise( void )
{
	// to initialise our time stamped vars, the singletion must be created after CTimer
	m_mousePos.Set( CVector(0.0f, 0.0f, 0.0f, 0.0f) );
	m_wheelPos.Set(0);

	// other vars we care about
	m_mouseVel = CVector(0.0f, 0.0f, 0.0f, 0.0f);
	m_wheelState = WHEEL_SPIN_NONE;

	for (int i = 0; i < MOUSE_BUTTON_MAX; i++)
		m_buttons[i].Initialise();

	m_fButtonT = -1.0f;
}

//--------------------------------------------------
//!
//! update states of buttons, record current velocities etc
//!
//--------------------------------------------------
void MouseInput::Update( void )
{
	m_Platform.Update();
	// update mouse state
	if (m_mousePos.SetThisFrame() && (m_mousePos.GetDuration() > EPSILON))
	{
		m_mouseVel.X() = (GetMousePos().X() - m_mousePos.GetLast().X()) / m_mousePos.GetDuration();
		m_mouseVel.Y() = (GetMousePos().Y() - m_mousePos.GetLast().Y()) / m_mousePos.GetDuration();
	}
	else
	{
		m_mouseVel.X() = 0.0f;
		m_mouseVel.Y() = 0.0f;
	}

	if (m_wheelPos.SetThisFrame())
	{
		if (m_wheelPos.GetCurr() > m_wheelPos.GetLast())
			m_wheelState = WHEEL_SPIN_UP;
		else if (m_wheelPos.GetCurr() < m_wheelPos.GetLast())
			m_wheelState = WHEEL_SPIN_DOWN;
		else
			m_wheelState = WHEEL_SPIN_NONE;
	}
	else
	{
		m_wheelState = WHEEL_SPIN_NONE;
	}

	// check for inactive
	float fInactivityTime = _R(CTimer::Get().GetSystemTime()) - fINACTIVE_TIME;
	m_bInactive = true;

	if ( m_mousePos.GetCurrTime() > fInactivityTime )
		m_bInactive = false;
	if ( m_wheelPos.GetCurrTime() > fInactivityTime )
		m_bInactive = false;

	for (int i = 0; i < MOUSE_BUTTON_MAX; i++)
	{
		m_buttons[i].Update( GetMousePos() );
		if	(
			(m_buttons[i].GetChanged()) ||
			(m_buttons[i].GetHeld())
			)
			m_fButtonT = _R(CTimer::Get().GetSystemTime());
	}

	if ( m_fButtonT > fInactivityTime )
		m_bInactive = false;
}

//--------------------------------------------------
//!
//! render mouse state
//!
//--------------------------------------------------
void MouseInput::Render( float fX, float fY, uint32_t colour )
{
#ifndef _GOLD_MASTER
	uint32_t col = colour;
	if (m_bInactive)
	{
		col =	(((colour >> 25) & 0xff) << 24) |
			(((colour >> 17) & 0xff) << 16) |
			(((colour >>  9) & 0xff) << 8)  |
			(((colour >>  1) & 0xff));
	}

	g_VisualDebug->Printf2D(fX, fY,		col, 0, "Mouse X: %.4f", MouseInput::Get().GetMousePos().X() );
	g_VisualDebug->Printf2D(fX, fY+12.0f,	col, 0, "Mouse Y: %.4f", MouseInput::Get().GetMousePos().Y() );
	g_VisualDebug->Printf2D(fX, fY+24.0f,	col, 0, "Mouse DX: %.4f", MouseInput::Get().GetMouseVel().X() );
	g_VisualDebug->Printf2D(fX, fY+36.0f,	col, 0, "Mouse DY: %.4f", MouseInput::Get().GetMouseVel().Y() );

	switch ( MouseInput::Get().GetWheelState() )
	{
	case WHEEL_SPIN_UP:		g_VisualDebug->Printf2D(fX, fY+48.0f, col, 0, "Mouse Wheel UP" );		break;
	case WHEEL_SPIN_NONE:	g_VisualDebug->Printf2D(fX, fY+60.0f, col, 0, "Mouse Wheel NONE" );	break;
	case WHEEL_SPIN_DOWN:	g_VisualDebug->Printf2D(fX, fY+72.0f, col, 0, "Mouse Wheel DOWN" );	break;
	default: break;
	}

	g_VisualDebug->Printf2D(fX, fY+84.0f, col, 0, "Mouse wheel: %d", MouseInput::Get().GetWheelPos() );

	float fStartY = fY+100.0f;

	for (int i = 0; i < MOUSE_BUTTON_MAX; i++)
	{
		MOUSE_BUTTON eState = (MOUSE_BUTTON)i;

		if ( GetButtonState(eState).GetPressed() )
			g_VisualDebug->Printf2D(fX, fStartY, col, 0, "%s Mouse PRESSED", GetMouseButtonString(eState) );
		else if ( GetButtonState(eState).GetHeld() )
		{
			g_VisualDebug->Printf2D(fX, fStartY, col, 0, "%s Mouse HELD", GetMouseButtonString(eState) );

			float fWidth = g_VisualDebug->GetDebugDisplayWidth();
			float fHeight = g_VisualDebug->GetDebugDisplayHeight();

			CPoint topLeft(		GetButtonState(eState).GetDownPos().X() * fWidth,	GetButtonState(eState).GetDownPos().Y() * fHeight,	0.0f );
			CPoint botLeft(		GetButtonState(eState).GetDownPos().X() * fWidth,	GetButtonState(eState).GetUpPos().Y() * fHeight,		0.0f );
			CPoint botRight(	GetButtonState(eState).GetUpPos().X() * fWidth,		GetButtonState(eState).GetUpPos().Y() * fHeight,		0.0f );
			CPoint topRight(	GetButtonState(eState).GetUpPos().X() * fWidth,		GetButtonState(eState).GetDownPos().Y() * fHeight,	0.0f );

			g_VisualDebug->RenderLine( topLeft, botLeft, col, DPF_DISPLAYSPACE );
			g_VisualDebug->RenderLine( botLeft, botRight, col, DPF_DISPLAYSPACE );
			g_VisualDebug->RenderLine( botRight, topRight, col, DPF_DISPLAYSPACE );
			g_VisualDebug->RenderLine( topRight, topLeft, col, DPF_DISPLAYSPACE );
		}
		fStartY += 12.0f;
	}
#endif
}


//--------------------------------------------------
//!
//! this doesn't belong here, but what the hey
//!
//--------------------------------------------------
CDirection MouseInput::GetWorldRayFromScreenPos( float fX, float fY )
{
	// expected input is normalised screen space 0,0 being top left,
	// 1,1 bottom right, just like the mouse coords.

	fX = (1.0f - ntstd::Clamp( fX, 0.0f, 1.0f )) - 0.5f;
	fY = (1.0f - ntstd::Clamp( fY, 0.0f, 1.0f )) - 0.5f;

	
	float fVPHeight = CamMan::GetPrimaryView()->GetZNear() * tanf( CamMan::GetPrimaryView()->GetFOVAngle() * 0.5f ) * 2.0f;
	float fVPWidth = fVPHeight * CamMan::GetPrimaryView()->GetAspectRatio();

	CDirection	ray( fVPWidth * fX, fVPHeight * fY, CamMan_Public::Get().GetZNear() );
	ray.Normalise();

	return ray * CamMan_Public::GetCurrMatrix();
}

void MouseInput::GetWorldRayFromMousePos( CDirection& ray, CPoint& origin )
{
	ray = GetWorldRayFromScreenPos(  GetMousePos().X(), GetMousePos().Y() );
	origin = CamMan_Public::GetCurrMatrix().GetTranslation();
}
