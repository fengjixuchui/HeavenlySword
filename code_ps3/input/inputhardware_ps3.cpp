/***************************************************************************************************
*
*	PS3 wrapper around fp Input functionality
*
*
*
***************************************************************************************************/

#include <Fp/FpInput/FpInput.h>
#include <Fp/FpInput/FpPad.inl>
#include "input/inputhardware.h"

#define	MAX_RAW_SAMPLES 128	// Should be the same as ATG's buffer size

/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::CInputHardware
*
*	DESCRIPTION		Constructor for CInputHardware singleton object, performing one-off
*					initialisation of elements within the single CInputHardware object.
*
***************************************************************************************************/

CInputHardware::CInputHardware()
{
#if defined (PLATFORM_PS3)
	// both mouse and inputhardware require the fpinput singleton, so this checks that the other
	// one has initalised it
	if( !FpInput::Exists() )
	{
		// Initialise the input toolkit.
		FpInput::Initialise();
	}
#endif
	
	// Create all the pads we need - at the moment based on startup parameters
	for ( int iLoop = 0; iLoop < MAX_PADS; iLoop++ )
	{
		m_apobPad[ iLoop ] = NT_NEW CInputPad( FpInput::Get().GetPad(iLoop) );
	}

	// Set our initial context
	SetContext( INPUT_CONTEXT_GAME );
	SetPadContext( PAD_CONTEXT_GAME );

	// Assign the default dead zone handling. 
	m_eDeadZone = DZ_STD;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::~CInputHardware
*
*	DESCRIPTION		Destructor, responsible for releasing all DirectInput objects prior to exit.
*
***************************************************************************************************/

CInputHardware::~CInputHardware()
{
#if defined (PLATFORM_PS3)
	// Get rid of the input toolkit.
	FpInput::Shutdown();
#endif

	// Clear out our array of controllers
	for ( int iLoop = 0; iLoop < MAX_PADS; iLoop++ )
	{
		NT_DELETE( m_apobPad[ iLoop ] );
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::Update
*
*	DESCRIPTION		Updates values & states associated with input devices
*
***************************************************************************************************/

void	CInputHardware::Update( float fTimeDelta )
{
	UNUSED( fTimeDelta );

	// Update all input devices.
	FpInput::Get().Update();

	// Poll each of our pads..
	for ( int iLoop = 0; iLoop < MAX_PADS; iLoop++ )
	{
		m_apobPad[ iLoop ]->Update( iLoop );		
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::ClearRumble
*
*	DESCRIPTION		Terminate rumble on all pads
*
***************************************************************************************************/

void	CInputHardware::ClearRumble( void )
{
}


/***************************************************************************************************
*	
*	FUNCTION		CAnalogState::CAnalogState
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CAnalogState::CAnalogState()
:	m_fX( 0.0f ),
	m_fY( 0.0f ),
	m_fAngle( 0.0f ),
	m_fMagnitude( 0.0f ),
	m_fMinX( 0.0f ),
	m_fMinY( 0.0f ),
	m_fMaxX( 0.0f ),
	m_fMaxY( 0.0f ),
	m_bIsCalibrated( false )
{
}

/***************************************************************************************************
*	
*	FUNCTION		CAnalogState::Update
*
*	DESCRIPTION		Given a hardware-format stick position, this function calculates an angle and
*					magnitude that's correctly clamped and which also takes into account the gamepad
*					dead-zone.
*
***************************************************************************************************/

void CAnalogState::Update( float fRawX, float fRawY )
{
	// Get float values between -1 and 1
	float fX, fY; 
	GenerateFloats( fRawX, fRawY, fX, fY );

	// ------------------------------------------------------------------------------
	// Calculate angle and magnitude from adjusted values, and clamp magnitude to 1.0f

	m_fMagnitude = fsqrtf( ( fX * fX ) + ( fY * fY ) );

	if ( m_fMagnitude > 0.0f )
	{
		m_fAngle = HALF_PI - facosf( clamp( fabsf( fY / m_fMagnitude ), 0.0f, 1.0f ) );
/*
		//
		//	Performs the same operations as the above line, but checks for SNANs at each step.
		//	The reason for the clamp in the above line is that due to numerical "issues",
		//	the quotient fY / m_fMagnitude sometimes (vary rarely) comes out infinitesimally
		//	larger than 1.0f (I think) and so facosf returns an SNAN. :(
		//

		float quotient = fY / m_fMagnitude;
		ntError( !isnan( quotient ) );

		float abs_quotient = fabsf( quotient );
		ntError( !isnan( abs_quotient ) );

		float clamp_abs_quotient = clamp( abs_quotient, 0.0f, 1.0f );
		ntError( !isnan( clamp_abs_quotient ) );

		float acos_abs_quotient = facosf( clamp_abs_quotient );
		ntError( !isnan( acos_abs_quotient ) );

		m_fAngle = HALF_PI - acos_abs_quotient;
		ntError( !isnan( m_fAngle ) );
*/
		if ( fX < 0.0f )
		{
			if ( fY < 0.0f )
				m_fAngle = m_fAngle + ( HALF_PI * 3.0f );
			else
				m_fAngle = ( HALF_PI * 3.0f ) - m_fAngle;
		}

		else
		{
			if ( fY < 0.0f )
				m_fAngle = HALF_PI - m_fAngle;
			else
				m_fAngle = m_fAngle + HALF_PI;
		}
    }
	else
	{
		m_fAngle = 0.0f;
	}

	if ( m_fMagnitude > 1.0f )
		m_fMagnitude = 1.0f;

	// ------------------------------------------------------------------------------
	// We need X/Y values generated from the adjusted angle and magnitude 
	
	m_fX = fsinf( m_fAngle ) * m_fMagnitude;
	m_fY = -fcosf( m_fAngle ) * m_fMagnitude;
}

/***************************************************************************************************
*	
*	FUNCTION		CAnalogState::GenerateFloats
*
*	DESCRIPTION		Generates floats from -1 to +1 from a pad/driver specific short input
*
***************************************************************************************************/

void CAnalogState::GenerateFloats( float fRawX, float fRawY, float& fX, float& fY )
{
	fX = fRawX;
	fY = fRawY;

	// Decide what our dead zone is based on the pad calibration state.
	float	fInputDeadZone;

	if ( !m_bIsCalibrated )
		fInputDeadZone = INITIAL_DEADZONE;
	else
		fInputDeadZone = FINAL_DEADZONE;

	// Handle the dead zone 
	switch( CInputHardware::Get().DeadZoneMode() )
	{
		case DZ_NONE:
			break;

		// Now scale the input based on the appropriate dead zone
		case DZ_STD:
			fX = ( fX >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fX ) - fInputDeadZone ) / ( 1.0f - fInputDeadZone ) );
			fY = ( fY >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fY ) - fInputDeadZone ) / ( 1.0f - fInputDeadZone ) );
			break;
	}

	// Initially our sticks are in a state where calibration hasn't happened. As such we need a much 
	// larger dead zone until we've moved in all directions. This behavior was inspired (if that's
	// the right word) from the TechCertGame demonstration project that comes with the XDK

	if ( !m_bIsCalibrated )
	{
		if ( m_fX > m_fMaxX ) m_fMaxX = m_fX;
		if ( m_fX < m_fMinX ) m_fMinX = m_fX;
		if ( m_fY > m_fMaxY ) m_fMaxY = m_fY;
		if ( m_fY < m_fMinY ) m_fMinY = m_fY;

		if ( ( m_fMaxX >= 0.99f ) && ( m_fMinX <= -0.99f ) && ( m_fMaxY >= 0.99f ) && ( m_fMinY <= -0.99f ) )
		{
			m_bIsCalibrated = true;	
		}
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad::CInputPad
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CInputPad::CInputPad( FpPad* pPad )
:	m_pobAnalogLeft( 0 ),
	m_pobAnalogRight( 0 ),
	m_pPad( pPad )
{
	Initialise();

	m_pobAnalogLeft = NT_NEW CAnalogState();
	m_pobAnalogRight = NT_NEW CAnalogState();

	// Init pad correction modes.
	if (m_pPad)
	{
		FpPad::StickMode correctMode = FpPad::kStickModeRaw;
//		FpPad::StickMode correctMode = FpPad::kStickModeSquare;
//		FpPad::StickMode correctMode = FpPad::kStickModeCircle;

		m_pPad->SetStickCorrectionMode( FpPad::kStickLeft,	correctMode	);
		m_pPad->SetStickCorrectionMode( FpPad::kStickRight,	correctMode	);
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad::~CInputPad
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CInputPad::~CInputPad()
{
	NT_DELETE( m_pobAnalogLeft );
	NT_DELETE( m_pobAnalogRight );
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad::Initialise
*
*	DESCRIPTION		Clears all the pad information
*
***************************************************************************************************/

void CInputPad::Initialise()
{
	// No pad linked to as yet
	m_ePadType = PAD_TYPE_NONE;							

	// Clear the basic button information
	ClearButtons();				

	// Run through all the analogue pads and clear their details
	for ( int i = 0; i < PAD_ANALOG_BUTTON_COUNT; i++ )
	{
		m_afButtonPressure[i] = 0.0f;
		m_afButtonVelocity[i] = 0.0f;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::ClearButtons
*
*	DESCRIPTION		Clears the basic button press information
*
***************************************************************************************************/

void CInputPad::ClearButtons()
{
	m_uiPressed		= 0;
	m_uiHeld		= 0;
	m_uiReleased	= 0;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::GetSensorRawMag
*
*	DESCRIPTION		Gets the most recent, unfiltered, value for a sensor
*
***************************************************************************************************/

float CInputPad::GetSensorRawMag( PAD_SENSOR_ID eSensor ) const
{
	ntAssert( m_pPad );

	// Get the raw sample from the pad (in range 0 -> 1023)
	uint16_t val;
	int iSampleCount = m_pPad->GetSensorRawSamples( &val, (FpPad::SensorId)eSensor, 1 );

	if ( iSampleCount )
	{
		// Bring the value into -1.0 to 1.0 range before it is returned
		return RemapSensor(val);
	}

	return 0.0f;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::GetSensorFilteredMag
*
*	DESCRIPTION		Gets a filtered value for a sensor
*
***************************************************************************************************/

float CInputPad::GetSensorFilteredMag( PAD_SENSOR_ID eSensor, PAD_SENSOR_FILTER eFilter ) const
{
	ntAssert( m_pPad );

	float fResult = 0.0f;

	// Call appropriate function with parameters as necessary
	switch( eFilter )
	{
		case PAD_SENSOR_FILTER_NONE:
			// Simply return this value, as it is normalised already
			fResult = GetSensorRawMag( eSensor );
			break;

		case PAD_SENSOR_FILTER_AVERAGE_5:
			fResult = GetSensorAverageFilterMag( eSensor, 5 );
			break;

		case PAD_SENSOR_FILTER_AVERAGE_10:
			fResult = GetSensorAverageFilterMag( eSensor, 10 );
			break;

		case PAD_SENSOR_FILTER_EXPONENTIAL_5:
			fResult = GetSensorExponentialFilterMag( eSensor, 5 );
			break;

		case PAD_SENSOR_FILTER_EXPONENTIAL_10:
			fResult = GetSensorExponentialFilterMag( eSensor, 10 );
			break;

		default:
			ntAssert_p( 0, ( "Unknown sensor ID" ) );
	}

	return fResult;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::GetSensorAverageFilterMag
*
*	DESCRIPTION		Uses a simple average filter over x number of samples for a sensor
*
***************************************************************************************************/

float CInputPad::GetSensorAverageFilterMag( PAD_SENSOR_ID eSensor, int iNumSamples ) const
{
	ntAssert( iNumSamples > 0 && iNumSamples <= MAX_RAW_SAMPLES );

	uint16_t rawSamples[MAX_RAW_SAMPLES];
	float fResult = 0.0f;

	memset( rawSamples, 0, sizeof(uint16_t) * MAX_RAW_SAMPLES );
	
	// Get the raw samples from the pad
	int iSampleCount = m_pPad->GetSensorRawSamples( rawSamples, (FpPad::SensorId)eSensor, iNumSamples );

	if ( iSampleCount )
	{
		// Average the values
		for ( int i = 0; i < iSampleCount; i++ )
		{
			fResult += RemapSensor(rawSamples[i]);
		}

		fResult /= (float)iSampleCount;
	}

	return fResult;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::GetSensorExponentialFilterMag
*
*	DESCRIPTION		Uses an exponential filter whereby the current value has double the weight of
*					the previous value.  So the recent samples are more important than older ones,
*					so this should give precise, instantaneous input, yet have an element of
*					smoothness in the results.
*
***************************************************************************************************/

float CInputPad::GetSensorExponentialFilterMag( PAD_SENSOR_ID eSensor, int iNumSamples ) const
{
	ntAssert( iNumSamples > 0 && iNumSamples <= MAX_RAW_SAMPLES );

	// We use bit shifting for this, so there is a limit to the number of times we can do that
	ntAssert( iNumSamples <= 30 );

	uint16_t rawSamples[MAX_RAW_SAMPLES];
	float fResult = 0.0f;

	memset( rawSamples, 0, sizeof(uint16_t) * MAX_RAW_SAMPLES );

	// Get the raw samples from the pad
	int iSampleCount = m_pPad->GetSensorRawSamples( rawSamples, (FpPad::SensorId)eSensor, iNumSamples );

	if ( iSampleCount )
	{
		// Calculate the fraction part
		float fFrac = 1.0f / (float)( 2 << iSampleCount );

		for ( int i = 0; i < iSampleCount; i++ )
		{
			int iExpCount = 2 << i;

			// Multiply the raw sample value by the weight we want to give the value
			fResult += RemapSensor(rawSamples[i]) * ( fFrac * (float)iExpCount );
		}
	}

	return fResult;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::Removed
*
*	DESCRIPTION		Handles removal of the gamepad devices
*
***************************************************************************************************/

void	CInputPad::Removed( void )
{
	ntAssert( false );
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::Inserted
*
*	DESCRIPTION		Handles insertion of gamepad devices
*
***************************************************************************************************/

void	CInputPad::Inserted( int )
{
	ntAssert( false );
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad::Update
*
*	DESCRIPTION		Updates values & states associated with input devices
*
***************************************************************************************************/

void CInputPad::Update( int iIndex )
{
	// Do we have a valid device to read?
	if( m_pPad && m_pPad->IsConnected() )
	{
		m_ePadType = PAD_TYPE_PRESENT;

		// convert FpPad info into our button presses
		u_int	uiButtonData = 0;
		
		static const FpPad::ButtonId analog[PAD_ANALOG_BUTTON_COUNT] =
		{
			FpPad::kButtonTriangle,		//	PAD_ANALOG_TRIANGLE		= PAD_ANALOG_FACE_1,
			FpPad::kButtonCircle,		//	PAD_ANALOG_CIRCLE		= PAD_ANALOG_FACE_2,
			FpPad::kButtonCross,		//	PAD_ANALOG_CROSS		= PAD_ANALOG_FACE_3,	
			FpPad::kButtonSquare,		//	PAD_ANALOG_SQUARE		= PAD_ANALOG_FACE_4,
			FpPad::kButtonL1,			//	PAD_ANALOG_L1			= PAD_ANALOG_TOP_1,
			FpPad::kButtonR1,			//	PAD_ANALOG_R1			= PAD_ANALOG_TOP_2,
			FpPad::kButtonL2,			//	PAD_ANALOG_L2			= PAD_ANALOG_TOP_3,
			FpPad::kButtonR2,			//	PAD_ANALOG_R2			= PAD_ANALOG_TOP_4,
		};

		static const PAD_BUTTON flag[PAD_ANALOG_BUTTON_COUNT] =
		{
			PAD_TRIANGLE,
			PAD_CIRCLE,
			PAD_CROSS,
			PAD_SQUARE,
			PAD_L1,
			PAD_R1,
			PAD_L2,
			PAD_R2,
		};

		// analogue buttons first
		for (int i = 0; i < PAD_ANALOG_BUTTON_COUNT; i++)
		{
			float fAnalogButton = m_pPad->GetButtonAnalog( analog[i] );

			m_afButtonVelocity[i] = fAnalogButton - m_afButtonPressure[i];
			m_afButtonPressure[i] = fAnalogButton;

			static const float fAnalogThreshold = 0.3f;

			if ( fAnalogButton >= fAnalogThreshold )
				uiButtonData |= flag[i];
			
			if ( m_pPad->GetButtonDigital( analog[i] ) )
				uiButtonData |= flag[i];
		}
		
		// now digital buttons
		if ( m_pPad->GetButtonDigital( FpPad::kButtonUp ) )			uiButtonData |= PAD_UP;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonDown ) )		uiButtonData |= PAD_DOWN;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonLeft ) )		uiButtonData |= PAD_LEFT;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonRight ) )		uiButtonData |= PAD_RIGHT;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonStart ) )		uiButtonData |= PAD_START;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonSelect ) )		uiButtonData |= PAD_BACK_SELECT;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonL3 ) )			uiButtonData |= PAD_LEFT_THUMB;
		if ( m_pPad->GetButtonDigital( FpPad::kButtonR3 ) )			uiButtonData |= PAD_RIGHT_THUMB;

		// Update pressed/released/held button states (these take into account analog->digital conversion too)	
		u_int uiDelta	= uiButtonData ^ m_uiHeld;
		m_uiReleased	= m_uiHeld & uiDelta;
		m_uiPressed		= uiButtonData & uiDelta;
		m_uiHeld		= uiButtonData;

		// Update analog stick values (angle, magnitude, X & Y)
		m_pobAnalogLeft->Update(	m_pPad->GetStickRawX( FpPad::kStickLeft ),
									m_pPad->GetStickRawY( FpPad::kStickLeft )	);

		m_pobAnalogRight->Update(	m_pPad->GetStickRawX( FpPad::kStickRight ),
									m_pPad->GetStickRawY( FpPad::kStickRight )	);

		// dont forget to add rumble functionality at some point
	}
	else
	{
		m_ePadType = PAD_TYPE_NONE;
	}
}	



// -------------------------------------------------------------------------------------------------

#ifdef	DEBUG_KEYBOARD

// The table that maps a keycode to a textversion of the keycode
const char*		CInputKeyboard::m_apcKeyToText[ 256];

//--------------------------------------------------
//!
//!	Initialise the keyboard
//!
//--------------------------------------------------
void CInputKeyboard::Initialise()
{
	// Macro to declare text version of key
#define KEY_TEXT(k) m_apcKeyToText[(k)] = #k; 

	// Define all the key's text
	KEY_TEXT( KEYC_ESCAPE )
	KEY_TEXT( KEYC_F1 )
	KEY_TEXT( KEYC_F2 )
	KEY_TEXT( KEYC_F3 )
	KEY_TEXT( KEYC_F4 )
	KEY_TEXT( KEYC_F5 )
	KEY_TEXT( KEYC_F6 )
	KEY_TEXT( KEYC_F7 )
	KEY_TEXT( KEYC_F8 )
	KEY_TEXT( KEYC_F9 )
	KEY_TEXT( KEYC_F10 )
	KEY_TEXT( KEYC_F11 )
	KEY_TEXT( KEYC_F12 )
	KEY_TEXT( KEYC_PRINTSCREEN )
	KEY_TEXT( KEYC_PAUSE )
	KEY_TEXT( KEYC_INSERT )
	KEY_TEXT( KEYC_DELETE )
	KEY_TEXT( KEYC_HOME )
	KEY_TEXT( KEYC_END )
	KEY_TEXT( KEYC_PAGE_UP )
	KEY_TEXT( KEYC_PAGE_DOWN )
	KEY_TEXT( KEYC_RIGHT_ARROW )
	KEY_TEXT( KEYC_LEFT_ARROW )
	KEY_TEXT( KEYC_DOWN_ARROW )
	KEY_TEXT( KEYC_UP_ARROW )
	KEY_TEXT( KEYC_A )
	KEY_TEXT( KEYC_B )
	KEY_TEXT( KEYC_C )
	KEY_TEXT( KEYC_D )
	KEY_TEXT( KEYC_E )
	KEY_TEXT( KEYC_F )
	KEY_TEXT( KEYC_G )
	KEY_TEXT( KEYC_H )
	KEY_TEXT( KEYC_I )
	KEY_TEXT( KEYC_J )
	KEY_TEXT( KEYC_K )
	KEY_TEXT( KEYC_L )
	KEY_TEXT( KEYC_M )
	KEY_TEXT( KEYC_N )
	KEY_TEXT( KEYC_O )
	KEY_TEXT( KEYC_P )
	KEY_TEXT( KEYC_Q )
	KEY_TEXT( KEYC_R )
	KEY_TEXT( KEYC_S )
	KEY_TEXT( KEYC_T )
	KEY_TEXT( KEYC_U )
	KEY_TEXT( KEYC_V )
	KEY_TEXT( KEYC_W )
	KEY_TEXT( KEYC_X )
	KEY_TEXT( KEYC_Y )
	KEY_TEXT( KEYC_Z )
	KEY_TEXT( KEYC_1 )
	KEY_TEXT( KEYC_2 )
	KEY_TEXT( KEYC_3 )
	KEY_TEXT( KEYC_4 )
	KEY_TEXT( KEYC_5 )
	KEY_TEXT( KEYC_6 )
	KEY_TEXT( KEYC_7 )
	KEY_TEXT( KEYC_8 )
	KEY_TEXT( KEYC_9 )
	KEY_TEXT( KEYC_0 )
	KEY_TEXT( KEYC_ENTER )
	KEY_TEXT( KEYC_EQUAL )
	KEY_TEXT( KEYC_TAB )
	KEY_TEXT( KEYC_SPACE )
	KEY_TEXT( KEYC_MINUS )
	KEY_TEXT( KEYC_SEMICOLON )
	KEY_TEXT( KEYC_COMMA )
	KEY_TEXT( KEYC_PERIOD )
	KEY_TEXT( KEYC_SLASH )
	KEY_TEXT( KEYC_KPAD_DIVIDE )
	KEY_TEXT( KEYC_KPAD_MULTIPLY )
	KEY_TEXT( KEYC_KPAD_MINUS )
	KEY_TEXT( KEYC_KPAD_PLUS )
	KEY_TEXT( KEYC_KPAD_ENTER )
	KEY_TEXT( KEYC_KPAD_1 )
	KEY_TEXT( KEYC_KPAD_2 )
	KEY_TEXT( KEYC_KPAD_3 )
	KEY_TEXT( KEYC_KPAD_4 )
	KEY_TEXT( KEYC_KPAD_5 )
	KEY_TEXT( KEYC_KPAD_6 )
	KEY_TEXT( KEYC_KPAD_7 )
	KEY_TEXT( KEYC_KPAD_8 )
	KEY_TEXT( KEYC_KPAD_9 )
	KEY_TEXT( KEYC_KPAD_0 )
	KEY_TEXT( KEYC_KPAD_PERIOD )



#undef KEY_TEXT
}



//--------------------------------------------------
//!
//!	Has a key been pressed?
//!	\return true if true
//!	\param eKey: key pressed
//!	\param iModifierMask: modifier used
//!
//--------------------------------------------------
bool	CInputKeyboard::IsKeyPressed( KEY_CODE eKey, int iModifierMask )
{
	FpKeyboard*	pKeyboard = FpInput::Get().GetKeyboard();
	
	bool bYup = pKeyboard->IsKeyPressed( (FpKeyboard::KeyId)eKey );

	if	(
		( iModifierMask & KEYM_SHIFT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftShift) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightShift) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_CTRL ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftControl) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightControl) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_ALT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftAlt) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightAlt) )
		)
		bYup = false;

	return bYup;
}

//--------------------------------------------------
//!
//!	Is a key held down?
//!	\return true if true
//!	\param eKey: key pressed
//!	\param iModifierMask: modifier used
//!
//--------------------------------------------------
bool	CInputKeyboard::IsKeyHeld( KEY_CODE eKey, int iModifierMask )
{
	FpKeyboard*	pKeyboard = FpInput::Get().GetKeyboard();
	
	bool bYup = pKeyboard->IsKeyHeld( (FpKeyboard::KeyId)eKey );

	if	(
		( iModifierMask & KEYM_SHIFT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftShift) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightShift) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_CTRL ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftControl) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightControl) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_ALT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftAlt) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightAlt) )
		)
		bYup = false;

	return bYup;
}

//--------------------------------------------------
//!
//!	Has a key been released?
//!	\return true if true
//!	\param eKey: key pressed
//!	\param iModifierMask: modifier used
//!
//--------------------------------------------------
bool	CInputKeyboard::IsKeyReleased( KEY_CODE eKey, int iModifierMask )
{
	FpKeyboard*	pKeyboard = FpInput::Get().GetKeyboard();
	
	bool bYup = pKeyboard->IsKeyReleased( (FpKeyboard::KeyId)eKey );

	if	(
		( iModifierMask & KEYM_SHIFT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftShift) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightShift) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_CTRL ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftControl) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightControl) )
		)
		bYup = false;
	
	if	(
		( iModifierMask & KEYM_ALT ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftAlt) ) &&
		( !pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightAlt) )
		)
		bYup = false;

	return bYup;
}

//--------------------------------------------------
//!
//!	Access the current state of a key
//!	\return Current key state
//!	\param eKey: key pressed
//!
//--------------------------------------------------
KEY_STATE CInputKeyboard::GetKeyState( KEY_CODE eKey)
{
	// Pressed?
	if (IsKeyPressed(eKey))
		return KEYS_PRESSED;

	// Held?
	if (IsKeyHeld(eKey))
		return KEYS_HELD;

	// Released?
	if (IsKeyReleased(eKey))
		return KEYS_RELEASED;

	// Not active in any way
	return KEYS_NONE;
}

//--------------------------------------------------
//!
//!	Access the current state of the modifiers
//!	\return Current modifiers state
//!
//--------------------------------------------------
KEY_MODIFIER CInputKeyboard::GetModifierState()
{
	FpKeyboard*	pKeyboard = FpInput::Get().GetKeyboard();

	int iRet = 0;

	if	(
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftShift) ) ||
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightShift) )
		)
		iRet |= KEYM_SHIFT;
	
	if	(
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftControl) ) ||
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightControl) )
		)
		iRet |= KEYM_CTRL;
	
	if	(
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyLeftAlt) ) ||
		( pKeyboard->IsKeyHeld(FpKeyboard::kKeyRightAlt) )
		)
		iRet |= KEYM_ALT;

	return (KEY_MODIFIER)iRet;
}



#endif	// DEBUG_KEYBOARD
