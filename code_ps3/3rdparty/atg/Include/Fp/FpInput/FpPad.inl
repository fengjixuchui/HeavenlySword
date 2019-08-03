//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : generic Dualshock2-like pad interface inlines

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_PAD_INL
#define FP_PAD_INL

//--------------------------------------------------------------------------------------------------
//	CHECKS
//--------------------------------------------------------------------------------------------------

#ifndef FP_PAD_H
#error This header file should not be included directly.
#endif//FP_PAD_H

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//	INLINES : MAIN STATE
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns analog value of a given button.

	@param	id	Id of the control in FpPadWin32::ButtonId

	@return	The value of the analog control, as a normalized float (in [0 1] range).
**/
//--------------------------------------------------------------------------------------------------

inline f32 FpPad::GetButtonAnalog(const FpPad::ButtonId id )const
{
	f32 res;
	res=m_buttons.m_analog[id];
	return res;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns digital state of a given button.

	@param	id	Id of the button in FpPadWin32::ButtonId

	@retval	true	Button is down. 
	@retval	false	Button is up. 

	@see	FpPad::GetButtonFlagsState(), for a bitfield with all buttons

	@warning THE NAME IS NOT OBVIOUS. TRY TO FIND SOMETHING BETTER? 

**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::GetButtonDigital(const ButtonId id) const
{
	return !!(m_buttons.m_flagsState&(1<<id));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get flags (bitfield) set for the current state of all buttons 

	@return	Bitfield with "down" status for each button. 
			N-th bit is for n-th button (n being a value of FpPad::ButtonId)

	@see	FpPad::GetButtonState(), for an individual button
**/
//--------------------------------------------------------------------------------------------------

inline u32 FpPad::GetButtonFlags(void) const
{	
	return m_buttons.m_flagsState;
}


//--------------------------------------------------------------------------------------------------
//	INLINES : DIGITAL BUTTONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
	@brief	Get flags (bitfield) set for all buttons that have been pressed

	@return	Bitfield with "pressed" status for each button. 
			N-th bit is for n-th button (n being a value of FpPad::ButtonId)

	@note	"Pressed" means that :
			- It was up, during previous frame
			- It is now down

	@see	FpPad::IsButtonPressed(), for an individual button
**/
//--------------------------------------------------------------------------------------------------

inline u32 FpPad::GetButtonFlagsPressed(void) const
{
	return m_buttons.m_flagsPressed;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get flags (bitfield) set for all buttons that are held

	@return	Bitfield with "held" status for each button. 
			N-th bit is for n-th button (n being a value of FpPad::ButtonId)
			- 1 : Held
			- 0 : Not held

	@note	"Held" means that :
			- It was down, during previous frame
			- It is still down

	@see	FpPad::IsButtonHeld(), for an individual button
**/
//--------------------------------------------------------------------------------------------------

inline u32 FpPad::GetButtonFlagsHeld(void) const
{
	return m_buttons.m_flagsHeld;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get flags (bitfield) set for all buttons that have been released

	@return	Bitfield with "released" status for each button. 
			N-th bit is for n-th button (n being a value of FpPad::ButtonId)
			- 1 : Released
			- 0 : Not released

	@note	"Released" means that :
			- It was down, during previous frame
			- It is now up

	@see	FpPad::IsButtonReleased(), for an individual button
**/
//--------------------------------------------------------------------------------------------------

inline u32 FpPad::GetButtonFlagsReleased(void) const
{
	return m_buttons.m_flagsReleased;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get flags (bitfield) set for all buttons that have changed

	@return	Bitfield with "changed" status for each button. 
			N-th bit is for n-th button (n being a value of FpPad::ButtonId);
			- 1 : Changed
			- 0 : Not changed

	@note	"Changed" means that it has either been :
			- Released
			- Pressed

	@see	FpPad::IsButtonChanged(), for an individual button
**/
//--------------------------------------------------------------------------------------------------

inline u32 FpPad::GetButtonFlagsChanged(void) const	
{
	return m_buttons.m_flagsChanged;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Tests if the button has been pressed.

	@param	id	Id of the button in FpPadWin32::ButtonId

	@retval	true	Button has been presseded. 
	@retval	false	Button has not been pressed. 

	@note	"Pressed" means that :
			- It was up, during previous frame
			- It is now down

	@see	FpPad::GetButtonFlagsPressed(), for a bitfield with all buttons

	@warning THE NAME IS NOT OBVIOUS. TRY TO FIND SOMETHING BETTER? 
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::IsButtonPressed(const ButtonId id) const
{
	return !!(m_buttons.m_flagsPressed&(1<<id));
}
//--------------------------------------------------------------------------------------------------
/**
	@brief	Tests if the button is held.

	@param	id	Id of the button in FpPadWin32::ButtonId

	@retval	true	Button is held. 
	@retval	false	Button button is not held. 

	@note	"Held" means that :
			- It was down, during previous frame
			- It is still down

	@see	FpPad::GetButtonFlagsHeld(), for a bitfield with all buttons
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::IsButtonHeld(const ButtonId id) const
{	
	return !!(m_buttons.m_flagsHeld&(1<<id));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Tests if the button has been released.

	@param	id	Id of the button in FpPadWin32::ButtonId

	@retval	true	Button has been released. 
	@retval	false	Button has not been released. 

	@note	"Released" means that :
			- It was down, during previous frame
			- It is now up

	@see	FpPad::GetButtonFlagsReleased(), for a bitfield with all buttons
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::IsButtonReleased(const ButtonId id) const
{
	return !!(m_buttons.m_flagsReleased&(1<<id));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Tests if the state of a button has changed.

	@param	id	Id of the button in FpPadWin32::ButtonId

	@retval	true	Button state has changed from previous frame. 
	@retval	false	Button state has not changed from previous frame. 

	@see	FpPad::GetButtonFlagsChanged(), for a bitfield with all buttons
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::IsButtonChanged(const ButtonId id) const
{
	return !!(m_buttons.m_flagsChanged&(1<<id));
}

//--------------------------------------------------------------------------------------------------
//	INLINES : STICKS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get stick corrected position.

	@return	X normalized (in [-1 1] range) position of the stick after correction.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::Stick::GetX(void) const
{
	return m_x;
}
//--------------------------------------------------------------------------------------------------
/**
	@brief	Get stick corrected position.

	@return	Y normalized (in [-1 +1] range) position of the stick after correction.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::Stick::GetY(void) const
{
	return m_y;
}
//--------------------------------------------------------------------------------------------------
/**
	@brief	Get stick raw (not corrected) position.

	@return	X normalized (in [-1 +1] range) raw position of the stick.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::Stick::GetRawX(void) const
{
	return m_rawX;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get stick raw (not corrected) position.

	@return	Y normalized (in [-1 +1] range) raw position of the stick.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::Stick::GetRawY(void) const
{
	return m_rawY;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Get stick raw (not corrected) position.

	@param	id	Id of the stick in FpPadWin32::StickId

	@return X normalized (in [-1 +1] range) raw (before any correction) position of the stick.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::GetStickRawX(const StickId id) const
{
	return m_stick[id].GetRawX();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Get stick raw (not corrected) position.

	@param	id	Id of the stick in FpPadWin32::StickId

	@return Y normalized (in [-1 +1] range) raw (before any correction) position of the stick.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::GetStickRawY(const StickId id) const
{
	return m_stick[id].GetRawY();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Get stick corrected position.

	@param	id	Id of the stick in FpPadWin32::StickId

	@return X normalized (in [-1 +1] range) position of the stick, after correction/remapping.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::GetStickX(const StickId id) const	
{
	return m_stick[id].GetX();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Get stick corrected position.

	@param	id	Id of the stick in FpPadWin32::StickId	
	
	@return Y normalized (in [-1 +1] range) position of the stick, after correction/remapping.
**/
//--------------------------------------------------------------------------------------------------

inline float FpPad::GetStickY(const StickId id) const		
{
	return m_stick[id].GetY();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Test to see if the pad is connected.

	@return	True if connected, false if not. 
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::IsConnected() const
{
	return ( GetStatus() == kStatusConnected );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Test to see if the pad has pressure sensitivity
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::HasPressure( void ) const
{
	return ( ( GetStatus() == kStatusConnected ) && ( m_padMode & kPadModePressure ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Test to see if the pad has sensors
**/
//--------------------------------------------------------------------------------------------------

inline bool FpPad::HasSensors( void ) const
{
	return ( ( GetStatus() == kStatusConnected ) && ( m_padMode & kPadModeSensor ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief Return number of sensor samples acquired for last Update() call
**/
//--------------------------------------------------------------------------------------------------

inline int FpPad::GetSensorSampleCountLastUpdate( void ) const
{
	return m_sensorSampleCountLastUpdate;
}

//--------------------------------------------------------------------------------------------------

#endif	//FP_PAD_INL
