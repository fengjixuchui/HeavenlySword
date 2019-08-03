//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : generic mouse interface

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_MOUSE_INL
#define FP_MOUSE_INL

//--------------------------------------------------------------------------------------------------
//	CHECKS
//--------------------------------------------------------------------------------------------------

#ifndef FP_MOUSE_H
#error This header file should not be included directly.
#endif	//FP_MOUSE_H

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return current state of a given button 

	@param		id	Id of the button (see FpMouse::ButtonId)

	@see		FpMouse::IsButtonPressed
	@see		FpMouse::IsButtonReleased
**/
//--------------------------------------------------------------------------------------------------

inline bool FpMouse::GetButtonState(const FpMouse::ButtonId id) const
{
	return !!(m_buttons.m_state&(1<<id));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Test is a button has been "pressed" during last update.

	"Pressed" means there was a transition (0->1).

	@param		id		Id of the button (see FpMouse::ButtonId)
	@retval		true	Button has been pressed
	@retval		false	Button has not been pressed

	@see		FpMouse::GetButtonState
	@see		FpMouse::IsButtonReleased
**/
//--------------------------------------------------------------------------------------------------

inline bool FpMouse::IsButtonPressed(const FpMouse::ButtonId id) const
{
	return !!(m_buttons.m_pressed&(1<<id));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Test is a button has been "released" during last update.

	"Released" means there was a transition (1->0).

	@param		id		Id of the button (see FpMouse::ButtonId)
	@retval		true	Button has been released
	@retval		false	Button has not been released

	@see		FpMouse::GetButtonState
	@see		FpMouse::IsButtonPressed
**/
//--------------------------------------------------------------------------------------------------

inline bool FpMouse::IsButtonReleased(const FpMouse::ButtonId id)  const
{
	return !!(m_buttons.m_released&(1<<id));
}	

//--------------------------------------------------------------------------------------------------
/**
	@brief		Pointer mode : pointer x position in pixels

	@see		FpMouse for more details on coordinate systems
**/
//--------------------------------------------------------------------------------------------------

inline int FpMouse::GetPointerPixelsX(void) const
{
	return m_pointer.m_pixelsX;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Pointer mode : pointer y position in pixels

	@see		FpMouse for more details on coordinate systems
**/
//--------------------------------------------------------------------------------------------------

inline int FpMouse::GetPointerPixelsY(void) const
{
	return m_pointer.m_pixelsY;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Pointer mode : pointer x position in normalised device coordinates

	@see		FpMouse for more details on coordinate systems
**/
//--------------------------------------------------------------------------------------------------

inline float FpMouse::GetPointerNdcX(void) const
{
	return m_pointer.m_ndcX;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Pointer mode : pointer y position in normalised device coordinates

	@see		FpMouse for more details on coordinate systems
**/
//--------------------------------------------------------------------------------------------------

inline float FpMouse::GetPointerNdcY(void) const	
{
	return m_pointer.m_ndcY;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Controller mode : return delta x for last update

	@see		FpMouse for more details on units
**/
//--------------------------------------------------------------------------------------------------

inline int FpMouse::GetControllerDeltaX(void) const
{
	return ( int ) ( m_controller.m_deltaX * m_mouseResolution );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Controller mode : return delta y for last update

	@see		FpMouse for more details on units
**/
//--------------------------------------------------------------------------------------------------

inline int FpMouse::GetControllerDeltaY(void) const
{
	return ( int ) ( m_controller.m_deltaY * m_mouseResolution );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Test to see if the mouse is connected.

	@return	True if connected, false if not. 
**/
//--------------------------------------------------------------------------------------------------

inline bool FpMouse::IsConnected() const
{
	return ( GetStatus() == kStatusConnected );
}

//--------------------------------------------------------------------------------------------------

#endif	//FP_MOUSE_INL
