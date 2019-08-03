/***************************************************************************************************
*
*	DESCRIPTION		Header file for PS3 CInputHardware class
*
*
***************************************************************************************************/

#ifndef	_INPUTHARDWARE_PS3_H
#define	_INPUTHARDWARE_PS3_H

// Include definition of input contexts
#include "input/inputcontexts.h"
#include <Fp/FpInput/FpKeyboard.h>
#include <Fp/FpInput/FpPad.h>

//--------------------------------------------------------------------------------------------------

#define	INITIAL_DEADZONE		0.35f
#define	FINAL_DEADZONE			0.25f

#define	MAX_PADS	( 4 )

enum PAD_NUMBER
{
	PAD_0	=	0,
	PAD_1	=	1,
	PAD_2	=	2,
	PAD_3	=	3,

	PAD_NUM	=	4,

	PAD_AI = 0x7fffffff,
};

// These are currently simplified - may need to be used in future
enum PAD_TYPE
{ 
	PAD_TYPE_NONE,
	PAD_TYPE_PRESENT,
};

#define	PAD_ATOD_SHIFT		(8)
#define PAD_BUTTON_COUNT	(16)

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
	PAD_TOP_4			=	0x80 << PAD_ATOD_SHIFT,

	PAD_BACK			=	PAD_BACK_SELECT,

	PAD_TRIANGLE		=	PAD_FACE_1,
	PAD_CIRCLE			=	PAD_FACE_2,
	PAD_CROSS			=	PAD_FACE_3,
	PAD_SQUARE			=	PAD_FACE_4,
	PAD_L1				=	PAD_TOP_1,
	PAD_R1				=	PAD_TOP_2,
	PAD_L2				=	PAD_TOP_3,
	PAD_R2				=	PAD_TOP_4,
};	

enum	PAD_ANALOG_BUTTON
{	
	PAD_ANALOG_FACE_1 = 0,				// These must be in the same order as the digital
	PAD_ANALOG_FACE_2,					// representations defined above.
	PAD_ANALOG_FACE_3,	
	PAD_ANALOG_FACE_4,	
	PAD_ANALOG_TOP_1,	
	PAD_ANALOG_TOP_2,	
	PAD_ANALOG_TOP_3,	
	PAD_ANALOG_TOP_4,	

	PAD_ANALOG_BUTTON_COUNT,

	PAD_ANALOG_TRIANGLE		= PAD_ANALOG_FACE_1,
	PAD_ANALOG_CIRCLE		= PAD_ANALOG_FACE_2,
	PAD_ANALOG_CROSS		= PAD_ANALOG_FACE_3,	
	PAD_ANALOG_SQUARE		= PAD_ANALOG_FACE_4,
	PAD_ANALOG_L1			= PAD_ANALOG_TOP_1,
	PAD_ANALOG_R1			= PAD_ANALOG_TOP_2,
	PAD_ANALOG_L2			= PAD_ANALOG_TOP_3,
	PAD_ANALOG_R2			= PAD_ANALOG_TOP_4,
};

enum	PAD_SENSOR_ID
{
	// Map values to the FpPad values directly
	PAD_SENSOR_ACCEL_X = FpPad::kSensorAccelX,
	PAD_SENSOR_ACCEL_Y = FpPad::kSensorAccelY,
	PAD_SENSOR_ACCEL_Z = FpPad::kSensorAccelZ,
	PAD_SENSOR_GYRO_Y = FpPad::kSensorGyroY,

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

//--------------------------------------------------------------------------------------------------


#ifdef	DEBUG_KEYBOARD
#define	DEF_KEY( type, value ) type = value
#else
#define	DEF_KEY( type, value ) type = 0
#endif	// DEBUG_KEYBOARD

// these mirror FpKeyboard almost exactly....
enum	KEY_CODE
{
	DEF_KEY( KEYC_ESCAPE,			FpKeyboard::kKeyEscape ),
	DEF_KEY( KEYC_F1,				FpKeyboard::kKeyF1 ),
	DEF_KEY( KEYC_F2,				FpKeyboard::kKeyF2 ),
	DEF_KEY( KEYC_F3,				FpKeyboard::kKeyF3 ),
	DEF_KEY( KEYC_F4,				FpKeyboard::kKeyF4 ),
	DEF_KEY( KEYC_F5,				FpKeyboard::kKeyF5 ),
	DEF_KEY( KEYC_F6,				FpKeyboard::kKeyF6 ),
	DEF_KEY( KEYC_F7,				FpKeyboard::kKeyF7 ),
	DEF_KEY( KEYC_F8,				FpKeyboard::kKeyF8 ),
	DEF_KEY( KEYC_F9,				FpKeyboard::kKeyF9 ),
	DEF_KEY( KEYC_F10,				FpKeyboard::kKeyF10 ),
	DEF_KEY( KEYC_F11,				FpKeyboard::kKeyF11 ),
	DEF_KEY( KEYC_F12,				FpKeyboard::kKeyF12 ),

	DEF_KEY( KEYC_PRINTSCREEN,		FpKeyboard::kKeyPrintScreen ),
	DEF_KEY( KEYC_PAUSE,			FpKeyboard::kKeyPause ),
	DEF_KEY( KEYC_INSERT,			FpKeyboard::kKeyInsert ),
	DEF_KEY( KEYC_DELETE,			FpKeyboard::kKeyDelete ),
	DEF_KEY( KEYC_HOME,				FpKeyboard::kKeyHome ),
	DEF_KEY( KEYC_END,				FpKeyboard::kKeyEnd ),
	DEF_KEY( KEYC_PAGE_UP,			FpKeyboard::kKeyPageUp ),
	DEF_KEY( KEYC_PAGE_DOWN,		FpKeyboard::kKeyPageDown ),
	DEF_KEY( KEYC_RIGHT_ARROW,		FpKeyboard::kKeyRight ),
	DEF_KEY( KEYC_LEFT_ARROW,		FpKeyboard::kKeyLeft ),
	DEF_KEY( KEYC_DOWN_ARROW,		FpKeyboard::kKeyDown ),
	DEF_KEY( KEYC_UP_ARROW,			FpKeyboard::kKeyUp ),

	DEF_KEY( KEYC_A,				FpKeyboard::kKeyA ),
	DEF_KEY( KEYC_B,				FpKeyboard::kKeyB ),
	DEF_KEY( KEYC_C,				FpKeyboard::kKeyC ),
	DEF_KEY( KEYC_D,				FpKeyboard::kKeyD ),
	DEF_KEY( KEYC_E,				FpKeyboard::kKeyE ),
	DEF_KEY( KEYC_F,				FpKeyboard::kKeyF ),
	DEF_KEY( KEYC_G,				FpKeyboard::kKeyG ),
	DEF_KEY( KEYC_H,				FpKeyboard::kKeyH ),
	DEF_KEY( KEYC_I,				FpKeyboard::kKeyI ),
	DEF_KEY( KEYC_J,				FpKeyboard::kKeyJ ),
	DEF_KEY( KEYC_K,				FpKeyboard::kKeyK ),
	DEF_KEY( KEYC_L,				FpKeyboard::kKeyL ),
	DEF_KEY( KEYC_M,				FpKeyboard::kKeyM ),
	DEF_KEY( KEYC_N,				FpKeyboard::kKeyN ),
	DEF_KEY( KEYC_O,				FpKeyboard::kKeyO ),
	DEF_KEY( KEYC_P,				FpKeyboard::kKeyP ),
	DEF_KEY( KEYC_Q,				FpKeyboard::kKeyQ ),
	DEF_KEY( KEYC_R,				FpKeyboard::kKeyR ),
	DEF_KEY( KEYC_S,				FpKeyboard::kKeyS ),
	DEF_KEY( KEYC_T,				FpKeyboard::kKeyT ),
	DEF_KEY( KEYC_U,				FpKeyboard::kKeyU ),
	DEF_KEY( KEYC_V,				FpKeyboard::kKeyV ),
	DEF_KEY( KEYC_W,				FpKeyboard::kKeyW ),
	DEF_KEY( KEYC_X,				FpKeyboard::kKeyX ),
	DEF_KEY( KEYC_Y,				FpKeyboard::kKeyY ),
	DEF_KEY( KEYC_Z,				FpKeyboard::kKeyZ ),
	DEF_KEY( KEYC_1,				FpKeyboard::kKey1 ),
	DEF_KEY( KEYC_2,				FpKeyboard::kKey2 ),
	DEF_KEY( KEYC_3,				FpKeyboard::kKey3 ),
	DEF_KEY( KEYC_4,				FpKeyboard::kKey4 ),
	DEF_KEY( KEYC_5,				FpKeyboard::kKey5 ),
	DEF_KEY( KEYC_6,				FpKeyboard::kKey6 ),
	DEF_KEY( KEYC_7,				FpKeyboard::kKey7 ),
	DEF_KEY( KEYC_8,				FpKeyboard::kKey8 ),
	DEF_KEY( KEYC_9,				FpKeyboard::kKey9 ),
	DEF_KEY( KEYC_0,				FpKeyboard::kKey0 ),

	DEF_KEY( KEYC_ENTER,			FpKeyboard::kKeyReturn ),
	DEF_KEY( KEYC_EQUAL,			FpKeyboard::kKeyEquals ),
	DEF_KEY( KEYC_TAB,				FpKeyboard::kKeyTab ),
	DEF_KEY( KEYC_SPACE,			FpKeyboard::kKeySpace ),
	DEF_KEY( KEYC_MINUS,			FpKeyboard::kKeyMinus ),
	DEF_KEY( KEYC_SEMICOLON,		FpKeyboard::kKeySemiColon ),
	DEF_KEY( KEYC_COMMA,			FpKeyboard::kKeyComma ),
	DEF_KEY( KEYC_PERIOD,			FpKeyboard::kKeyPeriod),
	DEF_KEY( KEYC_SLASH,			FpKeyboard::kKeyForwardSlash ),
	DEF_KEY( KEYC_KPAD_DIVIDE,		FpKeyboard::kKeyNumDiv ),
	DEF_KEY( KEYC_KPAD_MULTIPLY,	FpKeyboard::kKeyNumMul ),
	DEF_KEY( KEYC_KPAD_MINUS,		FpKeyboard::kKeyNumSub ),
	DEF_KEY( KEYC_KPAD_PLUS,		FpKeyboard::kKeyNumAdd ),
	DEF_KEY( KEYC_KPAD_ENTER,		FpKeyboard::kKeyNumEnter ),
	DEF_KEY( KEYC_KPAD_1,			FpKeyboard::kKeyNum1 ),
	DEF_KEY( KEYC_KPAD_2,			FpKeyboard::kKeyNum2 ),
	DEF_KEY( KEYC_KPAD_3,			FpKeyboard::kKeyNum3 ),
	DEF_KEY( KEYC_KPAD_4,			FpKeyboard::kKeyNum4 ),
	DEF_KEY( KEYC_KPAD_5,			FpKeyboard::kKeyNum5 ),
	DEF_KEY( KEYC_KPAD_6,			FpKeyboard::kKeyNum6 ),
	DEF_KEY( KEYC_KPAD_7,			FpKeyboard::kKeyNum7 ),
	DEF_KEY( KEYC_KPAD_8,			FpKeyboard::kKeyNum8 ),
	DEF_KEY( KEYC_KPAD_9,			FpKeyboard::kKeyNum9 ),
	DEF_KEY( KEYC_KPAD_0,			FpKeyboard::kKeyNum0 ),
	DEF_KEY( KEYC_KPAD_PERIOD,		FpKeyboard::kKeyNumDel ),

	// Map joypad inputs to unlikely key usages
	DEF_KEY( KEYC_JOYPAD_LEFT,		FpKeyboard::kKeyF13 ),
	DEF_KEY( KEYC_JOYPAD_RIGHT,		FpKeyboard::kKeyF14 ),
	DEF_KEY( KEYC_JOYPAD_UP,		FpKeyboard::kKeyF15 ),
	DEF_KEY( KEYC_JOYPAD_DOWN,		FpKeyboard::kKeyF16 ),
	DEF_KEY( KEYC_JOYPAD_SELECT,	FpKeyboard::kKeyF17 ),
	DEF_KEY( KEYC_JOYPAD_START,		FpKeyboard::kKeyF18 ),
	DEF_KEY( KEYC_JOYPAD_TRIANGLE,	FpKeyboard::kKeyF19 ),
	DEF_KEY( KEYC_JOYPAD_CROSS,		FpKeyboard::kKeyF20 ),
	DEF_KEY( KEYC_JOYPAD_SQUARE,	FpKeyboard::kKeyF21 ),
	DEF_KEY( KEYC_JOYPAD_CIRCLE,	FpKeyboard::kKeyF22 ),
	DEF_KEY( KEYC_JOYPAD_L1,		FpKeyboard::kKeyF23 ),
	DEF_KEY( KEYC_JOYPAD_L2,		FpKeyboard::kKeyF24 ),
	DEF_KEY( KEYC_JOYPAD_R1,		FpKeyboard::kKeyStop1 ),
	DEF_KEY( KEYC_JOYPAD_R2,		FpKeyboard::kKeyStop2 ),

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

	void Update( float fRawX, float fRawY );

	// The input values change for controllers and drivers - separate the functionality
	void GenerateFloats( float fRawX, float fRawY, float& fX, float& fY );

private:
	bool m_bIsCalibrated;
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
	CInputPad( FpPad* pPad );
	~CInputPad();

	// Update. Call once per frame...
	void Update( int iIndex );

	// Is the pad valid?
	bool IsValid( void ) const { return ( GetType() == PAD_TYPE_PRESENT ); }

	// Get pad type
	PAD_TYPE GetType( void ) const	{ return m_ePadType; }

	// Get states...
	u_int	GetPressed( void )	const	{ return( m_uiPressed ); }
	u_int	GetHeld( void )		const	{ return( m_uiHeld ); }
	u_int	GetReleased( void ) const	{ return( m_uiReleased ); }

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

	// Motion Sensing

	bool		IsMotionSensor( void ) const  { ntAssert( m_pPad ); return m_pPad->HasSensors(); };

	// Get motion sensor values, use no filtering
	
	float		GetSensorRawMag( PAD_SENSOR_ID eSensor ) const;
	float		GetSensorFilteredMag( PAD_SENSOR_ID eSensor, PAD_SENSOR_FILTER eFilter ) const;
	
protected:

	// Input is in the range 0 -> 1023
	inline float RemapSensor( uint16_t val ) const
	{
		// output is in the range -1.f -> 1.f
		static const float remap = 1.f / 511.5f;
		return (_R(val) * remap) - 1.0f;
	}

	friend class CInputHardware;

	void		Removed( void );
	void		Inserted( int iPort );

	// Motion sensor filter functions
	float		GetSensorAverageFilterMag( PAD_SENSOR_ID eSensor, int iNumSamples ) const;
	float		GetSensorExponentialFilterMag( PAD_SENSOR_ID eSensor, int iNumSamples ) const;

	PAD_TYPE				m_ePadType;										// What kind of controller is it?

	u_int					m_uiHeld;										// Buttons held at last update
	u_int					m_uiPressed;									// Buttons pressed (first frame) at last update
	u_int					m_uiReleased;									// Buttons released (previously held) at last update

	float					m_afButtonPressure[ PAD_ANALOG_BUTTON_COUNT ];	// Floating point representations of analog button pressures
	float					m_afButtonVelocity[ PAD_ANALOG_BUTTON_COUNT ];	// Floating point representations of analog button velocities

	CAnalogState*			m_pobAnalogLeft;
	CAnalogState*			m_pobAnalogRight;

	FpPad*					m_pPad;
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

	// Update. Call once per frame... Does sweet FA on Xenon
	void		Update( void ) {};

#ifdef	DEBUG_KEYBOARD

protected:
	static const char*		m_apcKeyToText[ 256 ];

public:
	// Convert a keycode to a textual representation of the code
	static const char* KeyToText(KEY_CODE eKey ) {return m_apcKeyToText[eKey];}

	// Initialise the keyboard
	void Initialise();

	bool	IsKeyPressed( KEY_CODE eKey, int iModifierMask = KEYM_NONE );
	bool	IsKeyHeld( KEY_CODE eKey, int iModifierMask = KEYM_NONE );
	bool	IsKeyReleased( KEY_CODE eKey, int iModifierMask = KEYM_NONE );
	// Access the current state of a particular key
	KEY_STATE GetKeyState( KEY_CODE eKey);

	// Access the current state of the modifiers
	KEY_MODIFIER GetModifierState();

#else // DEBUG_KEYBOARD

	// Initialise the keyboard
	void Initialise() {};

	bool	IsKeyPressed( KEY_CODE, int = KEYM_NONE )	{ return false; }
	bool	IsKeyHeld( KEY_CODE, int = KEYM_NONE )		{ return false; }
	bool	IsKeyReleased( KEY_CODE, int = KEYM_NONE )	{ return false; }

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

	void	Update( float fTimeDelta );
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
	int				GetPadCount( void ) const	{ return 0; } // this isnt supported yet...

	// Accessors for the dead zone mode. 
	void				DeadZoneMode( PAD_DEADZONE_MODE eMode )			{ m_eDeadZone = eMode; }
	PAD_DEADZONE_MODE	DeadZoneMode( )	const							{ return m_eDeadZone; }


private:
	INPUT_CONTEXT		m_eContext;
	PAD_CONTEXT			m_ePadContext;
	CInputPad*			m_apobPad[ MAX_PADS ];
	CInputKeyboard		m_obKeyboard;
	PAD_DEADZONE_MODE	m_eDeadZone;
};


#endif	//_INPUTHARDWARE_PS3_H
