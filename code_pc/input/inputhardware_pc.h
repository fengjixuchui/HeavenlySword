/***************************************************************************************************
*
*	DESCRIPTION		Header file for PC CInputHardware class
*
*
***************************************************************************************************/

#ifndef	_INPUTHARDWARE_PC_H
#define	_INPUTHARDWARE_PC_H

#define	DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

// Include definition of input contexts
#include "input/inputcontexts.h"
#include "game/commandresult.h"

#ifndef _INPUTHARDWARE_H
#error _INPUTHARDWARE_H
#endif

//--------------------------------------------------------------------------------------------------

#define	INITIAL_DEADZONE		0.35f
#define	FINAL_DEADZONE			0.25f

enum PAD_NUMBER
{
	PAD_0	=	0,
	PAD_1	=	1,
	PAD_2	=	2,
	PAD_3	=	3,

	PAD_NUM	=	4,

	// PAD_AI = 0x7fffffff,
};

// These are currently simplified - may need to be used in future
enum PAD_TYPE
{ 
	PAD_TYPE_NONE,
	PAD_TYPE_PRESENT,
};

#define	PAD_ATOD_SHIFT	( 8 )
#define GAMEPAD_MAX_CROSSTALK   (30)
#define PAD_BUTTON_COUNT		(16)

enum PAD_BUTTON
{
	PAD_UP				=	0x0001,						
	PAD_DOWN			=	0x0002, 					
	PAD_LEFT			=	0x0004,						
	PAD_RIGHT			=	0x0008,						
	PAD_START			=	0x0010,						
	PAD_BACK_SELECT		=	0x0020,						
	PAD_LEFT_THUMB		=	0x0040,						
	PAD_RIGHT_THUMB		=	0x0080,		

	PAD_FACE_1			=	0x01 << PAD_ATOD_SHIFT,		// Analog buttons are faked into digital presses
	PAD_FACE_2			=	0x02 << PAD_ATOD_SHIFT,		// by using unused bits in the button array.
	PAD_FACE_3			=	0x04 << PAD_ATOD_SHIFT,
	PAD_FACE_4			=	0x08 << PAD_ATOD_SHIFT,
	PAD_TOP_1			=	0x10 << PAD_ATOD_SHIFT,
	PAD_TOP_2			=	0x20 << PAD_ATOD_SHIFT,
	PAD_TOP_3			=	0x40 << PAD_ATOD_SHIFT,
	PAD_TOP_4			=	0x80 << PAD_ATOD_SHIFT
};	


enum	PAD_ANALOG_BUTTON
{	
	PAD_ANALOG_FACE_1,					// These must be in the same order as the digital
	PAD_ANALOG_FACE_2,					// representations defined above.
	PAD_ANALOG_FACE_3,	
	PAD_ANALOG_FACE_4,	
	PAD_ANALOG_TOP_1,	
	PAD_ANALOG_TOP_2,	
	PAD_ANALOG_TOP_3,	
	PAD_ANALOG_TOP_4,	

	PAD_ANALOG_BUTTON_COUNT
};

// PS3 pad motion sensor enums
enum	PAD_SENSOR_ID
{
	// Map values to the FpPad values directly
	PAD_SENSOR_ACCEL_X = 0,
	PAD_SENSOR_ACCEL_Y,
	PAD_SENSOR_ACCEL_Z,
	PAD_SENSOR_GYRO_Y,

	PAD_SENSOR_COUNT,
};

enum	PAD_SENSOR_FILTER
{
	PAD_SENSOR_FILTER_NONE = 0,			// Just use the most recent sample
	PAD_SENSOR_FILTER_AVERAGE_5,		// Average of last x samples
	PAD_SENSOR_FILTER_AVERAGE_10,
	PAD_SENSOR_FILTER_EXPONENTIAL_5,	// Current sample has double the weight of the previous sample
	PAD_SENSOR_FILTER_EXPONENTIAL_10,

	PAD_SENSOR_FILTER_COUNT,
};

// Handy colour-based equivalents of A,B,X and Y buttons

#define	PAD_GREEN			PAD_A
#define	PAD_RED				PAD_B
#define	PAD_BLUE			PAD_X
#define	PAD_YELLOW			PAD_Y

#define	PAD_ANALOG_GREEN	PAD_ANALOG_A
#define	PAD_ANALOG_RED		PAD_ANALOG_B
#define	PAD_ANALOG_BLUE		PAD_ANALOG_X
#define	PAD_ANALOG_YELLOW	PAD_ANALOG_Y

//--------------------------------------------------------------------------------------------------


#ifdef	DEBUG_KEYBOARD
#define	DEF_KEY( type, pcvalue ) type = pcvalue
#else
#define	DEF_KEY( type, pcvalue ) type = 0
#endif	// DEBUG_KEYBOARD

// If you edit this list, please also edit CInputKeyboard::Initialise()
enum	KEY_CODE
{
	DEF_KEY( KEYC_ESCAPE,			DIK_ESCAPE ),
	DEF_KEY( KEYC_F1,				DIK_F1 ),
	DEF_KEY( KEYC_F2,				DIK_F2 ),
	DEF_KEY( KEYC_F3,				DIK_F3 ),
	DEF_KEY( KEYC_F4,				DIK_F4 ),
	DEF_KEY( KEYC_F5,				DIK_F5 ),
	DEF_KEY( KEYC_F6,				DIK_F6 ),
	DEF_KEY( KEYC_F7,				DIK_F7 ),
	DEF_KEY( KEYC_F8,				DIK_F8 ),
	DEF_KEY( KEYC_F9,				DIK_F9 ),
	DEF_KEY( KEYC_F10,				DIK_F10 ),
	DEF_KEY( KEYC_F11,				DIK_F11 ),
	DEF_KEY( KEYC_F12,				DIK_F12 ),
	DEF_KEY( KEYC_PRINTSCREEN,		DIK_SYSRQ ),
	DEF_KEY( KEYC_PAUSE,			DIK_PAUSE ),
	DEF_KEY( KEYC_INSERT,			DIK_INSERT ),
	DEF_KEY( KEYC_DELETE,			DIK_DELETE ),
	DEF_KEY( KEYC_HOME,				DIK_HOME ),
	DEF_KEY( KEYC_END,				DIK_END ),
	DEF_KEY( KEYC_PAGE_UP,			DIK_PRIOR ),
	DEF_KEY( KEYC_PAGE_DOWN,		DIK_NEXT ),
	DEF_KEY( KEYC_RIGHT_ARROW,		DIK_RIGHT ),
	DEF_KEY( KEYC_LEFT_ARROW,		DIK_LEFT ),
	DEF_KEY( KEYC_DOWN_ARROW,		DIK_DOWN ),
	DEF_KEY( KEYC_UP_ARROW,			DIK_UP ),
	DEF_KEY( KEYC_A,				DIK_A ),
	DEF_KEY( KEYC_B,				DIK_B ),
	DEF_KEY( KEYC_C,				DIK_C ),
	DEF_KEY( KEYC_D,				DIK_D ),
	DEF_KEY( KEYC_E,				DIK_E ),
	DEF_KEY( KEYC_F,				DIK_F ),
	DEF_KEY( KEYC_G,				DIK_G ),
	DEF_KEY( KEYC_H,				DIK_H ),
	DEF_KEY( KEYC_I,				DIK_I ),
	DEF_KEY( KEYC_J,				DIK_J ),
	DEF_KEY( KEYC_K,				DIK_K ),
	DEF_KEY( KEYC_L,				DIK_L ),
	DEF_KEY( KEYC_M,				DIK_M ),
	DEF_KEY( KEYC_N,				DIK_N ),
	DEF_KEY( KEYC_O,				DIK_O ),
	DEF_KEY( KEYC_P,				DIK_P ),
	DEF_KEY( KEYC_Q,				DIK_Q ),
	DEF_KEY( KEYC_R,				DIK_R ),
	DEF_KEY( KEYC_S,				DIK_S ),
	DEF_KEY( KEYC_T,				DIK_T ),
	DEF_KEY( KEYC_U,				DIK_U ),
	DEF_KEY( KEYC_V,				DIK_V ),
	DEF_KEY( KEYC_W,				DIK_W ),
	DEF_KEY( KEYC_X,				DIK_X ),
	DEF_KEY( KEYC_Y,				DIK_Y ),
	DEF_KEY( KEYC_Z,				DIK_Z ),
	DEF_KEY( KEYC_1,				DIK_1 ),
	DEF_KEY( KEYC_2,				DIK_2 ),
	DEF_KEY( KEYC_3,				DIK_3 ),
	DEF_KEY( KEYC_4,				DIK_4 ),
	DEF_KEY( KEYC_5,				DIK_5 ),
	DEF_KEY( KEYC_6,				DIK_6 ),
	DEF_KEY( KEYC_7,				DIK_7 ),
	DEF_KEY( KEYC_8,				DIK_8 ),
	DEF_KEY( KEYC_9,				DIK_9 ),
	DEF_KEY( KEYC_0,				DIK_0 ),
	DEF_KEY( KEYC_ENTER,			DIK_RETURN ),
	DEF_KEY( KEYC_EQUAL,			DIK_EQUALS ),
	DEF_KEY( KEYC_TAB,				DIK_TAB ),
	DEF_KEY( KEYC_SPACE,			DIK_SPACE ),
	DEF_KEY( KEYC_MINUS,			DIK_MINUS ),
	DEF_KEY( KEYC_SEMICOLON,		DIK_SEMICOLON ),
	DEF_KEY( KEYC_COMMA,			DIK_COMMA ),
	DEF_KEY( KEYC_PERIOD,			DIK_PERIOD),
	DEF_KEY( KEYC_SLASH,			DIK_SLASH ),
	DEF_KEY( KEYC_KPAD_DIVIDE,		DIK_NUMPADSLASH ),
	DEF_KEY( KEYC_KPAD_MULTIPLY,	DIK_NUMPADSTAR ),
	DEF_KEY( KEYC_KPAD_MINUS,		DIK_NUMPADMINUS ),
	DEF_KEY( KEYC_KPAD_PLUS,		DIK_NUMPADPLUS ),
	DEF_KEY( KEYC_KPAD_ENTER,		DIK_NUMPADENTER ),
	DEF_KEY( KEYC_KPAD_1,			DIK_NUMPAD1 ),
	DEF_KEY( KEYC_KPAD_2,			DIK_NUMPAD2 ),
	DEF_KEY( KEYC_KPAD_3,			DIK_NUMPAD3 ),
	DEF_KEY( KEYC_KPAD_4,			DIK_NUMPAD4 ),
	DEF_KEY( KEYC_KPAD_5,			DIK_NUMPAD5 ),
	DEF_KEY( KEYC_KPAD_6,			DIK_NUMPAD6 ),
	DEF_KEY( KEYC_KPAD_7,			DIK_NUMPAD7 ),
	DEF_KEY( KEYC_KPAD_8,			DIK_NUMPAD8 ),
	DEF_KEY( KEYC_KPAD_9,			DIK_NUMPAD9 ),
	DEF_KEY( KEYC_KPAD_0,			DIK_NUMPAD0 ),
	DEF_KEY( KEYC_KPAD_PERIOD,		DIK_NUMPADPERIOD ),



	DEF_KEY( KEYC_JOYPAD_LEFT,		DIK_PREVTRACK ),
	DEF_KEY( KEYC_JOYPAD_RIGHT,		DIK_NEXTTRACK ),
	DEF_KEY( KEYC_JOYPAD_UP,		DIK_MUTE ),
	DEF_KEY( KEYC_JOYPAD_DOWN,		DIK_PLAYPAUSE ),
	DEF_KEY( KEYC_JOYPAD_SELECT,	DIK_MEDIASTOP ),
	DEF_KEY( KEYC_JOYPAD_START,		DIK_VOLUMEDOWN ),
	DEF_KEY( KEYC_JOYPAD_TRIANGLE,	DIK_VOLUMEUP ),
	DEF_KEY( KEYC_JOYPAD_CROSS,		DIK_WEBHOME ),
	DEF_KEY( KEYC_JOYPAD_SQUARE,	DIK_WEBSEARCH ),
	DEF_KEY( KEYC_JOYPAD_CIRCLE,	DIK_WEBFAVORITES ),
	DEF_KEY( KEYC_JOYPAD_L1,		DIK_WEBREFRESH ),
	DEF_KEY( KEYC_JOYPAD_L2,		DIK_WEBSTOP ),
	DEF_KEY( KEYC_JOYPAD_R1,		DIK_WEBFORWARD ),
	DEF_KEY( KEYC_JOYPAD_R2,		DIK_WEBBACK )
};
#undef DEF_KEY


#define	KEY_QUEUE_SIZE	( 8 )

enum	KEY_MODIFIER
{
	KEYM_NONE			=			0x00,
	KEYM_CTRL			=			0x01,
	KEYM_SHIFT			=			0x02,
	KEYM_ALT			=			0x04,
	KEYM_MODIFIER_MASK	=			0x0f,
};

enum	KEY_STATE
{	
	KEYS_NONE			=			0x00,
	KEYS_PRESSED		=			0x10,
	KEYS_HELD			=			0x20,
	KEYS_RELEASED		=			0x40,
	KEYS_STATE_MASK		=			0xf0
};


//--------------------------------------------------------------------------------------------------

// Forward declaration
class	CInputHardware;

/***************************************************************************************************
*	
*	CLASS			CAnalogState
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CAnalogState
{
public:

	// Construction
	CAnalogState();

	float	m_fX;
	float	m_fY;
	float	m_fAngle;
	float	m_fMagnitude;

	float	m_fMinX;
	float	m_fMinY;
	float	m_fMaxX;
	float	m_fMaxY;

	void Update( short sX, short sY );

	// The input values change for controllers and drivers - separate the functionality
	virtual void GenerateFloats( short sX, short sY, float& fX, float& fY ) = 0;
};

/***************************************************************************************************
*	
*	CLASS			CInputPadCapture
*
*	DESCRIPTION		A class that encapsulates the data needed for input capture
*
***************************************************************************************************/

class CInputPadCapture
{
public:
	int m_uiButtonData;
	int m_iHardwareX;
	int m_iHardwareY;
	int m_iHardwareZ;
	int m_iHardwareR;
};

/***************************************************************************************************
*	
*	CLASS			CInputPad
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CInputPad
{
public:

	// Construction destruction
	CInputPad();
	virtual ~CInputPad();

	// Update. Call once per frame...
	virtual void Update( float fTimeDelta ) = 0;

	// Update from capture
	virtual void Update(const CInputPadCapture* pobInputCapture) = 0;

	// Write the input values to a text buffer
	virtual const CInputPadCapture* GetInputCapture() const = 0;

	// Is the pad valid?
	bool IsValid( void ) const { return ( m_hDevice != 0 ); }

	// Get pad type
	PAD_TYPE GetType( void ) const	{ return m_ePadType; }

	// Get states...
	uint32_t	GetPressed( void )	const	{ return( m_uiPressed ); }
	uint32_t	GetHeld( void )		const	{ return( m_uiHeld ); }
	uint32_t	GetReleased( void ) const	{ return( m_uiReleased ); }

	// Clear button pressed flag for debounce...
	void ClearPressed( PAD_BUTTON eButton ) { m_uiPressed &= ~eButton; }

	// Clear buttons (everything: pressed, held, released)
	void Initialise();

	void ClearButtons( void );

	// Get analog button pressure values as 0.0f->1.0f

	float		GetButtonFrac( PAD_ANALOG_BUTTON eButton ) const { return m_afButtonPressure[ eButton ]; }

	// Get analog button pressure velocity values as 0.0f->1.0f

	float		GetButtonVelFrac( PAD_ANALOG_BUTTON eButton ) const { return m_afButtonVelocity[ eButton ]; }

	// Get angle represented by analog stick push...

	float		GetAnalogLAngle( void ) const { ntAssert( m_pobAnalogLeft ); return m_pobAnalogLeft->m_fAngle; }
	float		GetAnalogRAngle( void ) const { ntAssert( m_pobAnalogRight ); return m_pobAnalogRight->m_fAngle; }

	// Get magnitude of analog stick push (0.0f -> 1.0f)...

	float		GetAnalogLMag( void )	const { ntAssert( m_pobAnalogLeft ); return m_pobAnalogLeft->m_fMagnitude; }
	float		GetAnalogRMag( void )	const { ntAssert( m_pobAnalogRight ); return m_pobAnalogRight->m_fMagnitude; }
				
	float		GetAnalogLFrac( void )	const { ntAssert( m_pobAnalogLeft ); return m_pobAnalogLeft->m_fMagnitude; }
	float		GetAnalogRFrac( void )	const { ntAssert( m_pobAnalogRight ); return m_pobAnalogRight->m_fMagnitude; }
				
	float		GetAnalogLXFrac( void ) const { ntAssert( m_pobAnalogLeft ); return m_pobAnalogLeft->m_fX; };
	float		GetAnalogLYFrac( void ) const { ntAssert( m_pobAnalogLeft ); return m_pobAnalogLeft->m_fY; };
	float		GetAnalogRXFrac( void ) const { ntAssert( m_pobAnalogRight ); return m_pobAnalogRight->m_fX; };
	float		GetAnalogRYFrac( void ) const { ntAssert( m_pobAnalogRight ); return m_pobAnalogRight->m_fY; };

	// Motion Sensing - not used on PC builds at present
	bool		IsMotionSensor( void ) const  { return false; };
	
	float		GetSensorRawMag( PAD_SENSOR_ID eSensor ) const { UNUSED(eSensor); return 0.0f; };
	float		GetSensorFilteredMag( PAD_SENSOR_ID eSensor, PAD_SENSOR_FILTER eFilter ) const { UNUSED(eSensor); UNUSED(eFilter); return 0.0f; };

	void		SetRumble( bool bRumble ) { m_bRumble = bRumble; }

	virtual bool GetDebugStateString( char* ) { return false; }

protected:

	friend	CInputHardware;

	void		Removed( void );
	void		Inserted( int iPort );
	void		InitialiseRumble( void );

	LPDIRECTINPUTDEVICE8	m_hDevice;										// Handle to GamePad object
	LPDIRECTINPUTEFFECT		m_hFeedBack;									// Handle to a feedback effect for the controler

	PAD_TYPE				m_ePadType;										// What kind of controller is it?

	uint32_t				m_uiHeld;										// Buttons held at last update
	uint32_t				m_uiPressed;									// Buttons pressed (first frame) at last update
	uint32_t				m_uiReleased;									// Buttons released (previously held) at last update

	float					m_afButtonPressure[ PAD_ANALOG_BUTTON_COUNT ];	// Floating point representations of analog button pressures
	float					m_afButtonVelocity[ PAD_ANALOG_BUTTON_COUNT ];	// Floating point representations of analog button velocities

	CAnalogState*			m_pobAnalogLeft;
	CAnalogState*			m_pobAnalogRight;

	// Is the force feedback effect initialised?
	bool					m_bRumbleInitialised;

	// Do we want to rumble this frame?
	bool					m_bRumble;				
};

/***************************************************************************************************
*	
*	CLASS			CInputPad_Xbox
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CInputPad_Xbox : public CInputPad
{
public:

	// Construction destruction
	CInputPad_Xbox();
	virtual ~CInputPad_Xbox();

	// Update. Call once per frame...
	virtual void Update( float fTimeDelta );

	// Update from capture
	virtual void Update(const CInputPadCapture* pobInputCapture);

	// Write the input values to a text buffer
	virtual const CInputPadCapture* GetInputCapture() const;


protected:

	// Create our own version of the analogue state
	class CAnalogState_Xbox : public CAnalogState
	{
	public:
		CAnalogState_Xbox();
		virtual void GenerateFloats( short sX, short sY, float& fX, float& fY );
	private:
		bool m_bIsCalibrated;
	};

	// We need to redefine this friendship
	friend	CInputHardware;

	// Some internal translations for buttons we use here
	enum PAD_BUTTON_XBOX
	{
		PAD_BACK			=	PAD_BACK_SELECT,

		PAD_A				=	PAD_FACE_1,
		PAD_B				=	PAD_FACE_2,
		PAD_X				=	PAD_FACE_3,
		PAD_Y				=	PAD_FACE_4,
		PAD_BLACK			=	PAD_TOP_1,
		PAD_WHITE			=	PAD_TOP_2,
		PAD_LEFT_TRIGGER	=	PAD_TOP_3,
		PAD_RIGHT_TRIGGER	=	PAD_TOP_4,
	};	

	// Some internal translations for analogue buttons we use here
	enum PAD_ANALOG_BUTTON_XBOX
	{	
		PAD_ANALOG_A				= PAD_ANALOG_FACE_1,
		PAD_ANALOG_B				= PAD_ANALOG_FACE_2,
		PAD_ANALOG_X				= PAD_ANALOG_FACE_3,	
		PAD_ANALOG_Y				= PAD_ANALOG_FACE_4,
		PAD_ANALOG_BLACK			= PAD_ANALOG_TOP_1,
		PAD_ANALOG_WHITE			= PAD_ANALOG_TOP_2,
		PAD_ANALOG_LEFT_TRIGGER		= PAD_ANALOG_TOP_3,
		PAD_ANALOG_RIGHT_TRIGGER	= PAD_ANALOG_TOP_4,
	};
};




/***************************************************************************************************
*	
*	CLASS			CInputPad_PS2
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CInputPad_PS2 : public CInputPad
{
public:

	// Construction destruction
	CInputPad_PS2();
	virtual ~CInputPad_PS2();

	// Update. Call once per frame...
	virtual void Update( float fTimeDelta );

	// Update from capture
	virtual void Update(const CInputPadCapture* pobInputCapture);

	// Write the input values to a text buffer
	virtual const CInputPadCapture* GetInputCapture() const;

	// for debug pad logging
	virtual bool GetDebugStateString( char* pBuffer );

protected:

	// Common update functionality for the two update functions
	void UpdateShared();

	// Create our own version of the analogue state for the left stick
	class CAnalogState_PS2Left : public CAnalogState
	{
	public:
		virtual void GenerateFloats( short sX, short sY, float& fX, float& fY );
	};

	// Create our own version of the analogue state for the right stick
	class CAnalogState_PS2Right : public CAnalogState
	{
	public:
		virtual void GenerateFloats( short sX, short sY, float& fX, float& fY );
	};

	// We need to redefine this friendship
	friend	CInputHardware;

	// Some internal translations for buttons we use here
	enum PAD_BUTTON_PS2
	{
		PAD_SELECT		=	PAD_BACK_SELECT,

		PAD_TRIANGLE	=	PAD_FACE_1,
		PAD_CIRCLE		=	PAD_FACE_2,
		PAD_CROSS		=	PAD_FACE_3,
		PAD_SQUARE		=	PAD_FACE_4,
		PAD_L1			=	PAD_TOP_1,
		PAD_R1			=	PAD_TOP_2,
		PAD_L2			=	PAD_TOP_3,
		PAD_R2			=	PAD_TOP_4,
	};	

	// Some internal translations for analogue buttons we use here
	enum PAD_ANALOG_BUTTON_PS2
	{	
		PAD_ANALOG_TRIANGLE		= PAD_ANALOG_FACE_1,
		PAD_ANALOG_CIRCLE		= PAD_ANALOG_FACE_2,
		PAD_ANALOG_CROSS		= PAD_ANALOG_FACE_3,	
		PAD_ANALOG_SQUARE		= PAD_ANALOG_FACE_4,
		PAD_ANALOG_L1			= PAD_ANALOG_TOP_1,
		PAD_ANALOG_R1			= PAD_ANALOG_TOP_2,
		PAD_ANALOG_L2			= PAD_ANALOG_TOP_3,
		PAD_ANALOG_R2			= PAD_ANALOG_TOP_4,
	};

	// Storage for hardware values (nneded for input capture)
	CInputPadCapture m_obCapture;

};


/***************************************************************************************************
*	
*	CLASS			CInputKeyboard
*
*	DESCRIPTION		
*
***************************************************************************************************/

class	CInputKeyboard
{
public:
	CInputKeyboard()	
	{
		Initialise();
	}
	~CInputKeyboard()	{}


	// Initialise the keyboard
	void Initialise();

#ifdef	DEBUG_KEYBOARD
	// Convert a keycode to a textual representation of the code
	static const char* KeyToText(KEY_CODE eKey ) {return m_apcKeyToText[eKey];}

	// Update. Call once per frame...
	void		Update( void );

	// Is a keyboard present?
	bool		IsValid( void )
	{
		return ( m_hDevice != NULL );
	}

	// Has a key been pressed?
	bool		IsKeyPressed( KEY_CODE eKey, int iModifierMask = KEYM_NONE )
	{	
		return ( ( iModifierMask == m_iModifiers ) && ( m_aucKeys[ eKey ] == KEYS_PRESSED ) );
	}

	// Is a key being held down?
	bool		IsKeyHeld( KEY_CODE eKey, int iModifierMask = KEYM_NONE )
	{	
		return ( ( iModifierMask == m_iModifiers ) && ( m_aucKeys[ eKey ] == KEYS_HELD ) );
	}

	// Has a key been released?
	bool		IsKeyReleased( KEY_CODE eKey, int iModifierMask = KEYM_NONE )
	{
		return ( ( iModifierMask == m_iModifiers ) && ( m_aucKeys[ eKey ] == KEYS_RELEASED ) );
	}

	char		KeyCodeToAscii( KEY_CODE eKey, int iModifierMask = KEYM_NONE );

	// Access the current state of a particular key
	KEY_STATE GetKeyState( KEY_CODE eKey)
	{
		return (KEY_STATE)m_aucKeys[eKey];
	}

	// Access the current state of the modifiers
	KEY_MODIFIER GetModifierState()
	{
		return (KEY_MODIFIER)m_iModifiers;
	}

#endif // DEBUG_KEYBOARD

#ifndef	DEBUG_KEYBOARD
	void	Update( void ) {};
	bool	IsKeyPressed( KEY_CODE, int = KEYM_NONE )	{ return false; }
	bool	IsKeyHeld( KEY_CODE, int = KEYM_NONE )		{ return false; }
	bool	IsKeyReleased( KEY_CODE, int = KEYM_NONE )	{ return false; }
#endif	// DEBUG_KEYBOARD

	void EncodeKeyPressToString(char *pcBuffer, int iSize);
	void DecodeStringToKeypress(char* pcBuffer);

protected:

#ifdef	DEBUG_KEYBOARD
	friend	CInputHardware;

	void		Removed( void );
	void		Inserted( int iPort );

	// Is modifier being held
	bool		IsModifierHeld( int iModifierMask )
	{
		return ( ( iModifierMask & m_iModifiers ) != NULL );
	}

	// Is a key being pressed (ignoring the influence of any modifiers)
	bool		IsKeyPressedNoMod( KEY_CODE eKey )
	{	
		return	( m_aucKeys[ eKey ] == KEYS_PRESSED );
	}

	LPDIRECTINPUTDEVICE8	m_hDevice;
	int						m_iModifiers;
	uint8_t					m_aucKeys[ 256 ];
	uint8_t					m_aucLastKeyMatrix[ 256 ];
	uint8_t					aucKeyMatrix[ 256 ];
	static const char*		m_apcKeyToText[ 256 ];

#endif	// DEBUG_KEYBOARD
};


/***************************************************************************************************
*	
*	CLASS			CInputHardware
*
*	DESCRIPTION		
*
***************************************************************************************************/

class	CInputHardware : public	Singleton<CInputHardware>
{
public:
	CInputHardware();
	~CInputHardware();

	void	Update( float fTimeDelta);
	void	ClearRumble( void );

	// Context setting/retrieval
	void			SetContext( INPUT_CONTEXT eContext )	{ m_eContext = eContext; };
	INPUT_CONTEXT	GetContext( void )						{ return m_eContext; };

	void			SetPadContext( PAD_CONTEXT eContext )	{ m_ePadContext = eContext; };
	PAD_CONTEXT		GetPadContext( void )					{ return m_ePadContext; };

	// Access to joypad hardware
	
	CInputPad&		GetPad( PAD_NUMBER ePad )	{ ntAssert( m_apobPad[ ePad ] ); return *m_apobPad[ ePad ]; };
	CInputPad*		GetPadP( PAD_NUMBER ePad )	{ ntAssert( m_apobPad[ ePad ] ); return m_apobPad[ ePad ]; }; 

	// Keyboard access

	CInputKeyboard&	GetKeyboard( void )			{ return m_obKeyboard; };
	CInputKeyboard*	GetKeyboardP( void ) 		{ return &m_obKeyboard; };

	// Cheeky thing for auto re-assignment of player inputs

	int				GetPadCount( void ) const	{ return m_iPadCount; }

	// Accessors for the dead zone mode. 
	void				DeadZoneMode( PAD_DEADZONE_MODE eMode )			{ m_eDeadZone = eMode; }
	PAD_DEADZONE_MODE	DeadZoneMode( )	const							{ return m_eDeadZone; }

private:

	static	BOOL CALLBACK	EnumJoysticksCallback( LPCDIDEVICEINSTANCE pInst, LPVOID lpvContext );
	static	BOOL CALLBACK   EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );

	IDirectInput8*		m_pobDirectInput;				// Direct Input interfaces
	
	int					m_iPadCount;
	INPUT_CONTEXT		m_eContext;
	PAD_CONTEXT			m_ePadContext;
	CInputPad*			m_apobPad[ PAD_NUM ];
	CInputKeyboard		m_obKeyboard;
	PAD_DEADZONE_MODE	m_eDeadZone;
};


#endif	//_INPUTHARDWARE_PC_H
