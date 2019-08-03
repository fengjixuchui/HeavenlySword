//--------------------------------------------------
//!
//!	\file mouse.h
//!	mouse input singleton.
//!
//--------------------------------------------------

#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H

#include "core/timer.h"

//--------------------------------------------------
//!
//!	SysTimeStamped
//! A system time-stamped variable, updated if
//! CTimer::Get().GetSystemTicks() is new
//!
//--------------------------------------------------
template<typename T> class SysTimeStamped
{
public:
	SysTimeStamped( void )
	{
		m_initialised = false;	
	}

	// have we been updated this frame??
	bool	SetThisFrame( void ) const
	{
		return	((!m_initialised) || (m_iCurrentTick != CTimer::Get().GetSystemTicks())) ? false : true;
	}

	// set our current value
	void	Set( const T& newVal )
	{
		if (!m_initialised)
		{
			m_initialised = true;
			m_iCurrentTick = CTimer::Get().GetSystemTicks();
			m_fLastTime = m_fCurrTime = _R(CTimer::Get().GetSystemTime());
			m_last = m_current = newVal;
		}
		else
		{
			if (!SetThisFrame())
			{
				m_iCurrentTick = CTimer::Get().GetSystemTicks();
				m_fLastTime = m_fCurrTime;
				m_last = m_current;
				m_fCurrTime = _R(CTimer::Get().GetSystemTime());
				m_current = newVal;
			}
		}
	}
	
	const T&	GetCurr( void )		const { ntAssert( m_initialised ); return m_current;	}
	const T&	GetLast( void )		const { ntAssert( m_initialised ); return m_last;	}
	float		GetCurrTime( void )	const { ntAssert( m_initialised ); return m_fCurrTime;	}
	float		GetLastTime( void )	const { ntAssert( m_initialised ); return m_last;	}
	float		GetDuration( void )	const { ntAssert( m_initialised ); return (m_fCurrTime - m_fLastTime); }
	bool		GetInited( void )	const { return m_initialised; }

private:
	bool	m_initialised;
	T		m_current;
	T		m_last;
	float	m_fCurrTime;
	float	m_fLastTime;
	u_int	m_iCurrentTick;
};

//--------------------------------------------------
//!
//!	enum MOUSE_BUTTON
//!
//--------------------------------------------------
enum MOUSE_BUTTON
{
	MOUSE_LEFT		= 0,
	MOUSE_MID		= 1,
	MOUSE_RIGHT		= 2,
	MOUSE_X1		= 3,
	MOUSE_X2		= 4,

	MOUSE_BUTTON_MAX,
};

enum WHEEL_STATE
{
	WHEEL_SPIN_UP	= 0,
	WHEEL_SPIN_NONE	= 1,
	WHEEL_SPIN_DOWN	= 2,

	WHEEL_SPIN_MAX,
};

//--------------------------------------------------
//!
//!	MouseButton
//! Holds button info
//!
//--------------------------------------------------
class	MouseButton
{
public:
	MouseButton( void ) {}

	void	Initialise( void )
	{
		m_down.Set( false );
		m_lastDown = CVector(0.0f,0.0f,0.0f,0.0f);
		m_lastUp = CVector(0.0f,0.0f,0.0f,0.0f);
		m_fPressTime = -1.0f;
	}

	void	Update( const CVector& pos );
	void	SetState( bool bDown ) { m_down.Set( bDown ); }

	bool	GetChanged( void )		const { return m_down.SetThisFrame(); }
	bool	GetPressed( void )		const { return (GetChanged() && m_down.GetCurr()); }
	bool	GetReleased( void )		const { return (GetChanged() && !m_down.GetCurr()); }
	bool	GetHeld( void )			const { return (!GetChanged() && m_down.GetCurr()); }

	CVector	GetDownPos( void )	const { return m_lastDown; }
	CVector	GetUpPos( void )	const { return m_lastUp; }
	
private:
	static const float		fCLICK_TIME;	// time to define a double click over

	SysTimeStamped<bool>	m_down;			// are we down?
	CVector					m_lastDown;		// position we were in when pressed
	CVector					m_lastUp;		// position we were in when released
	bool					m_bDBLClick;	// 2 Pressed within	fCLICK_TIME
	float					m_fPressTime;	// time we last we're pressed
};

// include the platform specific mouse bit
#if defined(PLATFORM_PC)
#include "input/mouse_pc.h"
#elif defined(PLATFORM_PS3)
#include "input/mouse_ps3.h"
#endif

//--------------------------------------------------
//!
//!	MouseInput
//! checks for mouse input, records input, position, velocities etc.
//!
//--------------------------------------------------
class	MouseInput : public Singleton<MouseInput>
{
public:
	friend class MouseInputPlatform;

	MouseInput( int winX, int winY );

	void	Initialise( void );

	// convert window->normalised
	CVector GetNormalisedCoords( const CVector& from ) const
	{
		return CVector( from.X() / _R(m_winX), from.Y() / _R(m_winY), 0.0f, 0.0f );
	}

	// convert normalised->window
	CVector	GetWinRelativeCoords( const CVector& from ) const
	{
		return CVector( from.X() * _R(m_winX), from.Y() * _R(m_winY), 0.0f, 0.0f );
	}

	// update state of singleton
	void	Update( void );
	void	Render( float fX, float fY, uint32_t col );

	// get mouse info
	CVector				GetMousePos( void )		const	{ return m_mousePos.GetCurr(); }
	CVector				GetMouseVel( void )		const	{ return m_mouseVel; }
	WHEEL_STATE			GetWheelState( void )	const	{ return m_wheelState; }
	int					GetWheelPos( void )		const	{ return m_wheelPos.GetCurr(); }
	const MouseButton&	GetButtonState( MOUSE_BUTTON eButton ) const { return m_buttons[eButton]; }

	static const char*	GetMouseButtonString( MOUSE_BUTTON eButton )
	{
		switch( eButton )
		{
		case MOUSE_LEFT:	return "Left";
		case MOUSE_MID:		return "Middle";
		case MOUSE_RIGHT:	return "Right";
		case MOUSE_X1:		return "Misc 1";
		case MOUSE_X2:		return "Misc 2";
		default:			return "INVALID";
		}
	}

	static CDirection GetWorldRayFromScreenPos( float fX, float fY );
	void GetWorldRayFromMousePos( CDirection& ray, CPoint& origin );

	MouseInputPlatform			m_Platform;		//!< Platform specific bits

private:
	// member vars
	int							m_winX, m_winY;	//!< dimensions of the parent window
	bool						m_bInactive;	//!< is the mouse unused for n seconds?

	SysTimeStamped<CVector>		m_mousePos;		//!< current normalised mouse position
	SysTimeStamped<int>			m_wheelPos;		//!< current normalised mouse position

	WHEEL_STATE					m_wheelState;	//!< current mouse wheel status
	CVector						m_mouseVel;		//!< current normalised mouse velocity
	float						m_fButtonT;		//!< last system time for button activity

	MouseButton					m_buttons[MOUSE_BUTTON_MAX]; //!< button arrays

	static const float			fINACTIVE_TIME;		//<!



};


#endif // end INPUT_MOUSE_H
