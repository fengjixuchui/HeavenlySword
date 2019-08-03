//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : generic mouse interface

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_MOUSE_H
#define FP_MOUSE_H

//--------------------------------------------------------------------------------------------------
//	SCE LIBRARY INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/synchronization.h>

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpInputDevice.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class FpMouse : public FpInputDevice
{
public:
	// Enumeration
	enum ButtonId
	{
		kButtonLeft = 0,
		kButtonMiddle,
		kButtonRight
	};

	// Construction
	FpMouse();
   ~FpMouse();

	// Operations (buttons)
	inline bool 	GetButtonState(const FpMouse::ButtonId id) const;
	inline bool 	IsButtonPressed(const FpMouse::ButtonId id) const;
	inline bool 	IsButtonReleased(const FpMouse::ButtonId id) const;

	// Operations (pointer mode)
	void			SetWindow(int left, int top, int right, int bottom);
	inline int		GetPointerPixelsX(void) const;
	inline int		GetPointerPixelsY(void) const;
	inline float	GetPointerNdcX(void) const;
	inline float	GetPointerNdcY(void) const;

	// Operations (controller mode)
	inline int		GetControllerDeltaX(void) const;
	inline int		GetControllerDeltaY(void) const;

	// Operations (update)
	bool			Update(void);

	// Connection status
	void			SetConnected( int lv2DeviceId );
	void			SetDisconnected( void );
	inline bool		IsConnected( void ) const;

	// Internal operations
	inline int		GetLv2DeviceId( void ) const {return m_lv2DeviceId;}
	void 			Invalidate(void);

protected:
	// Attributes: button data
	struct Buttons
	{
		inline			Buttons() {Invalidate();}

		void			Invalidate(void);
		void			InvalidateEvents(void);

		unsigned int 	m_state;			///< Current state of buttons 
		unsigned int 	m_released;			///< Warning, we're reporting events, this means we can have both released and pressed set for an intervalbetween two updates
		unsigned int 	m_pressed;			///< Warning, we're reporting events, this means we can have both released and pressed set for an intervalbetween two updates
	} m_buttons;

	// Attributes: pointer mode data
	struct Pointer
	{
		inline			Pointer() {Invalidate();}

		void			Invalidate(void);

		int   			m_pixelsX;			///< Pixels, origin is top left of the screen (we suppose that the low level part has some knowledge of the screen geometry)
		int	  			m_pixelsY;			///< Pixels, origin is top left of the screen (we suppose that the low level part has some knowledge of the screen geometry)
		float 			m_ndcX;				///< Gl-style normalised device coordinates (0=center, -1=left, +1=right)
		float 			m_ndcY;				///< Gl-style normalised device coordinates (0=center, -1=top, +1=bottom)
	}					m_pointer;			///< Data for mouse used as a 2d pointer (windows style)

	// Attributes: pseudo window data (to emulate pointer in pixel coords)
	struct Window
	{
		int				m_left;
		int				m_top;
		int				m_right;
		int				m_bottom;
	}					m_window;

	// Attributes : controller mode data
	struct Controller
	{
		inline			Controller() {Invalidate();}

		void			Invalidate(void);

		int				m_deltaX;			///< Delta in x units (NOT PIXELS - NO RELATION !!!) since last frame
		int				m_deltaY;			///< Delta in y units (NOT PIXELS - NO RELATION !!!) since last frame
	}					m_controller;		///< Date for mouse used as a game controller (fps-style)
	float				m_mouseResolution;	///< Internal value used when emulating a mouse pointer (very bad right now)

	// Attributes : LV2
	int					m_lv2DeviceId;		///< Which device number we have been allocated by Lv2. -1 if not connected.


	// Operations
	bool 				EmulateMousePointerFromController(void);
	bool 				UpdatePointerNdc(void);
};

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpMouse.inl>

//--------------------------------------------------------------------------------------------------

#endif	//FP_MOUSE_H
