/***************************************************************************************************
*
*   $Header:: /game/inputhardware_pc.cpp 9     14/08/03 10:48 Dean                                 $
*
*	Implementation for PC CInputHardware class.
*
*	CHANGES		
*
*	10/01/2001	Dean	Created
*
***************************************************************************************************/

#include "input/inputhardware.h"
#include "editable/enumlist.h"
#include "gfx/graphicsdevice.h" // attn! deano! do you want shell to provide a HWND?

// Sorry - quick solution for PS2 hardware - GH
#include "core/timer.h"
#include <stdio.h>
#include <stdlib.h>


// Here's a handy array of strings that we can use to give a readable key press

#ifdef	DEBUG_KEYBOARD

// #define RUMBLE_ENABLED


//! NVPerfHud requires the use of buffered keyboard dinput this causes it to happen
#define USE_BUFFERED_KEYBOARD_INPUT
#define SAMPLE_BUFFER_SIZE 8

#endif	//DEBUG_KEYBOARD


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
	// Create all the pads we need - at the moment based on startup parameters
	for ( int iLoop = 0; iLoop < PAD_NUM; iLoop++ )
	{
		m_apobPad[ iLoop ] = NT_NEW CInputPad_PS2();
	}

	{
		HRESULT	hRes;

		// Create our DirectInput device
		hRes = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( void** )&m_pobDirectInput, NULL );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed to create DirectInput8 device" ) );

		// Now enumerate all the pads that we have.
		m_iPadCount = 0;
		hRes = m_pobDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL, CInputHardware::EnumJoysticksCallback, NULL, DIEDFL_FORCEFEEDBACK | DIEDFL_ATTACHEDONLY );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed to enumerate game controllers" ) );

		// If we have no registered pads - try again without the requirement for force feedback (may not have driver support )
		if ( m_iPadCount == 0 )
		{
			hRes = m_pobDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL, CInputHardware::EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY );
			ntAssert_p( SUCCEEDED( hRes ), ( "Failed to enumerate game controllers" ) );
		}

	}

	// Initialise the keyboard object
#ifdef	DEBUG_KEYBOARD
	{
		HRESULT	hRes;

		m_pobDirectInput->CreateDevice( GUID_SysKeyboard, &m_obKeyboard.m_hDevice, NULL );
		if ( m_obKeyboard.m_hDevice )
		{
			hRes = m_obKeyboard.m_hDevice->SetDataFormat( &c_dfDIKeyboard );
			ntAssert_p( SUCCEEDED( hRes ), ( "Failed to set keyboard data format" ) );
			hRes = m_obKeyboard.m_hDevice->SetCooperativeLevel( (HWND)GraphicsDevice::Get().m_Platform.GetHwnd(), ( DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ) );
			ntAssert_p( SUCCEEDED( hRes ), ( "Failed to set keyboard cooperative level" ) );
#if defined( USE_BUFFERED_KEYBOARD_INPUT )
			DIPROPDWORD dipdw;
			dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
			dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dipdw.diph.dwObj        = 0;
			dipdw.diph.dwHow        = DIPH_DEVICE;
			dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size
			hRes = m_obKeyboard.m_hDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
			ntAssert_p( SUCCEEDED( hRes ), ( "Failed to set keyboard buffer size" ) );
#endif
			m_obKeyboard.m_hDevice->Acquire();

		}
	}
#endif	//DEBUG_KEYBOARD

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
	// Release all of the pads we found..
	for ( int iLoop = 0; iLoop < m_iPadCount; iLoop++ )
	{
		m_apobPad[ iLoop ]->m_hDevice->Unacquire();
		m_apobPad[ iLoop ]->m_hDevice->Release();
	}

	// Clear out our array of controllers
	for ( int iLoop = 0; iLoop < PAD_NUM; iLoop++ )
	{
		NT_DELETE( m_apobPad[ iLoop ] );
	}

	// Now release the keyboard too..
#ifdef	DEBUG_KEYBOARD
	if ( m_obKeyboard.m_hDevice )	
	{
		m_obKeyboard.m_hDevice->Unacquire();
		m_obKeyboard.m_hDevice->Release();
	}
#endif	//DEBUG_KEYBOARD

	// And finally release the DirectInput device
	m_pobDirectInput->Release();
}

/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::EnumJoysticksCallback
*
*	DESCRIPTION		Callback used to enumerate all controllers connected to the PC when the project
*					is launched. All controllers that are identified have DirectInput devices created
*					for them, along with correct configuration of their cooperative levels and data
*					formats. If we can't configure the controller, we ntAssert.
*
***************************************************************************************************/
BOOL CALLBACK	CInputHardware::EnumJoysticksCallback( LPCDIDEVICEINSTANCE pInst, LPVOID )
{
	// If we've already got a full complement of controller, then stop.
	if ( CInputHardware::Get().m_iPadCount >= PAD_NUM )
		return DIENUM_STOP;

	CInputHardware* pobInputHardware	= CInputHardware::GetP();
	CInputPad*		pobInputPad			= pobInputHardware->m_apobPad[ pobInputHardware->m_iPadCount ];

	// Create an interface to the controller
	HRESULT	hRes;

	hRes = pobInputHardware->m_pobDirectInput->CreateDevice( pInst->guidInstance, &pobInputPad->m_hDevice, NULL );
	if ( SUCCEEDED( hRes ) )
	{
		hRes = pobInputPad->m_hDevice->SetDataFormat( &c_dfDIJoystick2 );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed during call to SetDataFormat for controller" ) );
		if ( FAILED( hRes ) )
			return DIENUM_STOP;

		hRes = pobInputPad->m_hDevice->SetCooperativeLevel( (HWND)GraphicsDevice::Get().m_Platform.GetHwnd(), ( DISCL_EXCLUSIVE | DISCL_BACKGROUND ) );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed during call to SetCooperativeLevel for controller" ) );
		if ( FAILED( hRes ) )
			return DIENUM_STOP;

		// Enumerate controller objects. We need to do this to set axis parameters correctly.
		hRes = pobInputPad->m_hDevice->EnumObjects( CInputHardware::EnumObjectsCallback, NULL, DIDFT_ALL );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed during call to EnumObjects for controller" ) ) ;
    
		// Now everything is sorted, acquire the pad, and then increase our pad count.
		pobInputPad->m_hDevice->Acquire();
		pobInputHardware->m_iPadCount++;

#ifdef RUMBLE_ENABLED
		// Create a simple feedback effect for the pad
		hRes = pobInputPad->m_hDevice->CreateEffect( GUID_Square, 0, &pobInputPad->m_hFeedBack, 0 );
		if ( !SUCCEEDED( hRes ) )
			ntPrintf( "Failed during call to EnumObjects for controller - couldn't create feedback effect" );
#endif
	}	

	// We're still looking for working pads..
	return DIENUM_CONTINUE;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::EnumObjectsCallback
*
*	DESCRIPTION		Callback invoked during controller enumeration. This allows us to set an 
*					appropriate range for the analog sticks that matches the values returned by the
*					Xbox libraries. If we fail to set properties correctly, we ntAssert.
*
***************************************************************************************************/

BOOL CALLBACK   CInputHardware::EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pobDevObjInst, VOID* )
{
	// Get some pointers..
	CInputHardware* pobInputHardware	= CInputHardware::GetP();
	CInputPad*		pobInputPad			= pobInputHardware->m_apobPad[ pobInputHardware->m_iPadCount ];

	// For axes that are returned, set the DIPROP_RANGE property for the enumerated axis in order to scale min/max values.
	if	(	( pobDevObjInst->guidType == GUID_XAxis ) ||
			( pobDevObjInst->guidType == GUID_YAxis ) ||
			( pobDevObjInst->guidType == GUID_RxAxis ) ||
			( pobDevObjInst->guidType == GUID_RyAxis )
		)
	{
		DIPROPRANGE	diprg; 
		diprg.diph.dwSize       = sizeof( DIPROPRANGE ); 
		diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER ); 
		diprg.diph.dwHow        = DIPH_BYID;
		diprg.diph.dwObj        = pobDevObjInst->dwType;	// Specify the enumerated axis
		diprg.lMin              = -32767;
		diprg.lMax              = 32767; 

		// Set the range for the axis
		HRESULT	hRes;
		hRes = pobInputPad->m_hDevice->SetProperty( DIPROP_RANGE, &diprg.diph );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed during call to SetProperty on controller" ) );
		if ( FAILED( hRes ) )
			return DIENUM_STOP;
	}	
	else
	if	(	( pobDevObjInst->guidType == GUID_ZAxis ) ||
			( pobDevObjInst->guidType == GUID_RzAxis ) ||
			( pobDevObjInst->guidType == GUID_RxAxis ) ||
			( pobDevObjInst->guidType == GUID_RyAxis )
		)
	{
		DIPROPRANGE	diprg; 
		diprg.diph.dwSize       = sizeof( DIPROPRANGE ); 
		diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER ); 
		diprg.diph.dwHow        = DIPH_BYID;
		diprg.diph.dwObj        = pobDevObjInst->dwType;	// Specify the enumerated axis
		diprg.lMin              = 0;
		diprg.lMax              = 255; 

		// Set the range for the axis
		HRESULT	hRes;
		hRes = pobInputPad->m_hDevice->SetProperty( DIPROP_RANGE, &diprg.diph );
		ntAssert_p( SUCCEEDED( hRes ), ( "Failed during call to SetProperty on controller" ) );
		if ( FAILED( hRes ) )
			return DIENUM_STOP;
	}
	else
	if	( pobDevObjInst->guidType == GUID_POV )
	{		

	}

	return DIENUM_CONTINUE;
}

/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::Update
*
*	DESCRIPTION		Updates values & states associated with input devices
*					NOTE: pcTextUpdate allows the input to be driven from a text string
*
***************************************************************************************************/

void	CInputHardware::Update( float fTimeDelta)
{
	// Poll each of our pads..
	for ( int iLoop = 0; iLoop < m_iPadCount; iLoop++ )
	{
		m_apobPad[ iLoop ]->Update(fTimeDelta);
	}

#ifdef	DEBUG_KEYBOARD
	m_obKeyboard.Update();
#endif	//DEBUG_KEYBOARD
}

/***************************************************************************************************
*	
*	FUNCTION		CInputHardware::ClearRumble
*
*	DESCRIPTION		Terminate rumble on all pads
*
***************************************************************************************************/

void CInputHardware::ClearRumble( void )
{
	for ( int iLoop = 0; iLoop < PAD_NUM; iLoop++ )
	{
		m_apobPad[ iLoop ]->SetRumble(false);
		m_apobPad[ iLoop ]->Update(0.0f);
	}	
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
	m_fMaxY( 0.0f )
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

void CAnalogState::Update( short sX, short sY  )
{
	// Get float values between -1 and 1
	float fX, fY; 
	GenerateFloats( sX, sY, fX, fY );

	// ------------------------------------------------------------------------------
	// Calculate angle and magnitude from adjusted values, and clamp magnitude to 1.0f

	m_fMagnitude = fsqrtf( ( fX * fX ) + ( fY * fY ) );

	if ( m_fMagnitude > 0.0f )
	{
		m_fAngle = HALF_PI - facosf( fabsf( fY / m_fMagnitude ) );

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
*	FUNCTION		CInputPad::CInputPad
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CInputPad::CInputPad()
:	m_pobAnalogLeft( 0 ),
	m_pobAnalogRight( 0 )
{
	Initialise();
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
	m_hDevice = 0;	
	m_hFeedBack = 0;
	m_ePadType = PAD_TYPE_NONE;							

	// Clear the basic button information
	ClearButtons();				

	// Clear the rumble details
	// Is the force feedback effect initialised?
	m_bRumbleInitialised = false;
	m_bRumble = false;								

	// Run through all the analogue pads and clear their details
	for ( int i = 0; i < PAD_ANALOG_BUTTON_COUNT; i++ )
	{
		m_afButtonPressure[i] = 0.0f;
		m_afButtonVelocity[i] = 0.0f;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad::InitialiseRumble
*
*	DESCRIPTION		Sets up a simple effect for some basic force feedback.  This is a very breif
*					implementation - a full one can be left until proper hardware.
*
***************************************************************************************************/
void CInputPad::InitialiseRumble( void )
{
	// Check that we are set to make the initialisation
	ntAssert( m_hFeedBack );
	ntAssert( !m_bRumbleInitialised );

	// A structure containing feedback parameters particular to a periodic effect
	// such as the simple sine wave we are using
	DIPERIODIC obSpecificDef;
	obSpecificDef.dwMagnitude = DI_FFNOMINALMAX;
	obSpecificDef.dwPeriod = 100;
	obSpecificDef.dwPhase = 100;
	obSpecificDef.lOffset = 0;

	// A current empty envelope structure
	DIENVELOPE obEnvelope; 
	obEnvelope.dwSize = sizeof(DIENVELOPE);
	obEnvelope.dwAttackLevel = 0; 
	obEnvelope.dwAttackTime = 0; 
	obEnvelope.dwFadeLevel = 0; 
	obEnvelope.dwFadeTime = 0; 

	// The standard definition for a DI feedback effect
	DWORD dwAxes[2] = {DIJOFS_X, DIJOFS_Y};
	LONG lDirection[2] = {0, 0};

	DIEFFECT obDef;
	obDef.dwSize = sizeof( DIEFFECT );
	obDef.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS; 
	obDef.dwDuration = INFINITE;
	obDef.dwSamplePeriod = 0;
	obDef.dwGain = DI_FFNOMINALMAX;
	obDef.dwTriggerButton = DIEB_NOTRIGGER;
	obDef.dwTriggerRepeatInterval = 0;
	obDef.cAxes = 2;
	obDef.rgdwAxes = dwAxes;
	obDef.rglDirection = &lDirection[0];
	obDef.cbTypeSpecificParams = sizeof( DIPERIODIC );
	obDef.lpvTypeSpecificParams = &obSpecificDef;
	obDef.lpEnvelope = &obEnvelope; 
	obDef.dwStartDelay = 0;

	// Set the parameters to our feedback device
	HRESULT hres = m_hFeedBack->SetParameters( &obDef, DIEP_AXES | DIEP_DIRECTION | DIEP_DURATION | DIEP_ENVELOPE | DIEP_GAIN | DIEP_SAMPLEPERIOD | DIEP_STARTDELAY | DIEP_TYPESPECIFICPARAMS );
	if ( ( hres != DI_OK ) && ( hres != DI_TRUNCATED ) )
		{ ntAssert( 0 ); }

	// Set the flage to say the job has been done
	m_bRumbleInitialised = true;
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
*	FUNCTION		CInputPad_Xbox::CInputPad_Xbox
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CInputPad_Xbox::CInputPad_Xbox()
{
	// Construct our overriden analogue container class
	m_pobAnalogLeft = NT_NEW CAnalogState_Xbox();
	m_pobAnalogRight = NT_NEW CAnalogState_Xbox();
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::~CInputPad_Xbox
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CInputPad_Xbox::~CInputPad_Xbox()
{
	// Clear our overriden analogue container class
	NT_DELETE( m_pobAnalogLeft );
	NT_DELETE( m_pobAnalogRight );
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::Update
*
*	DESCRIPTION		Updates values & states associated with input devices
*
***************************************************************************************************/

void CInputPad_Xbox::Update( float fTimeDelta )
{
	UNUSED(fTimeDelta);
	
	// Do we have a valid device to read?
	if ( m_hDevice )
	{
		HRESULT	hRes;

		// Read hardware state
		DIJOYSTATE2	obJoyState;
		memset( &obJoyState, 0, sizeof( obJoyState ) );

		hRes = m_hDevice->Poll();

		if ( SUCCEEDED( hRes ) )
		{
			hRes = m_hDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &obJoyState );
		}

		if ( FAILED( hRes ) )
		{
			if ( ( hRes == DIERR_INPUTLOST ) || ( hRes == DIERR_NOTACQUIRED ) )
				m_hDevice->Acquire();

			LPDIRECTINPUTDEVICE8 hTempDevice = m_hDevice;
			Initialise();
			m_hDevice = hTempDevice;
			m_ePadType = PAD_TYPE_NONE;
			return;
		}
		else
		{
			m_ePadType = PAD_TYPE_PRESENT;
		}

		// Turn DIJOYSTATE2 information into a nice bitmask. If you can think of a nicer way of
		// doing this, then please let me know. 

		u_int	uiButtonData = 0;

		u_char	aucAnalogButtonData[ PAD_ANALOG_BUTTON_COUNT ];
		memset( aucAnalogButtonData, 0, sizeof( aucAnalogButtonData ) );

		// First, sort out buttons that are actually analog on the Xbox controller..		
		if ( obJoyState.rgbButtons[0] & 0x80 )	{ uiButtonData |= PAD_A; aucAnalogButtonData[ PAD_ANALOG_A ] = 255; };
		if ( obJoyState.rgbButtons[1] & 0x80 )	{ uiButtonData |= PAD_B; aucAnalogButtonData[ PAD_ANALOG_B ] = 255; };
		if ( obJoyState.rgbButtons[2] & 0x80 )	{ uiButtonData |= PAD_X; aucAnalogButtonData[ PAD_ANALOG_X ] = 255; };
		if ( obJoyState.rgbButtons[3] & 0x80 )	{ uiButtonData |= PAD_Y; aucAnalogButtonData[ PAD_ANALOG_Y ] = 255; };
		if ( obJoyState.rgbButtons[4] & 0x80 )	{ uiButtonData |= PAD_BLACK; aucAnalogButtonData[ PAD_ANALOG_BLACK ] = 255; };
		if ( obJoyState.rgbButtons[5] & 0x80 )	{ uiButtonData |= PAD_WHITE; aucAnalogButtonData[ PAD_ANALOG_WHITE ] = 255; };

		// Next, do the standard digital buttons
		if ( obJoyState.rgbButtons[6] & 0x80 )	{ uiButtonData |= PAD_START; };
		if ( obJoyState.rgbButtons[7] & 0x80 )	{ uiButtonData |= PAD_BACK; };
		if ( obJoyState.rgbButtons[8] & 0x80 )	{ uiButtonData |= PAD_LEFT_THUMB; };
		if ( obJoyState.rgbButtons[9] & 0x80 )	{ uiButtonData |= PAD_RIGHT_THUMB; };

		// Then interpret POV data as directional component
		switch( obJoyState.rgdwPOV[0] )
		{
		case ( 0 ):			uiButtonData |= ( PAD_UP );					break;		// 0x00000000
		case ( 4500 ):		uiButtonData |= ( PAD_UP | PAD_RIGHT );		break;		// 0x00001194
		case ( 9000 ):		uiButtonData |= ( PAD_RIGHT );				break;		// 0x00002328
		case ( 13500 ):		uiButtonData |= ( PAD_RIGHT | PAD_DOWN );	break;		// 0x000034bc
		case ( 18000 ):		uiButtonData |= ( PAD_DOWN );				break;		// 0x00004650
		case ( 22500 ):		uiButtonData |= ( PAD_LEFT | PAD_DOWN );	break;		// 0x000057e4
		case ( 27000 ):		uiButtonData |= ( PAD_LEFT );				break;		// 0x00006978
		case ( 31500 ):		uiButtonData |= ( PAD_LEFT | PAD_UP );		break;		// 0x00007b0c
		default:														break;		// 0xffffffff
		}

		// And finally, make sure analog components make it across
		aucAnalogButtonData[ PAD_ANALOG_LEFT_TRIGGER ]	= ( u_char )obJoyState.lZ;
		aucAnalogButtonData[ PAD_ANALOG_RIGHT_TRIGGER ]	= ( u_char )obJoyState.lRz;

		// Process analog button pressures, taking into account any crosstalk and building our digital
		// bit pattern too..

		for ( int iLoop = 0; iLoop < PAD_ANALOG_BUTTON_COUNT; iLoop++ )
		{
			// According to SDK documentation, if the analog buttons value is < GAMEPAD_MAX_CROSSTALK
			// then the button should be ignored, and is not to be considered a button press.

			// Allow the last two buttons (the trigger buttons) to have a different threshold
			u_char	ucThreshold = (iLoop < ( PAD_ANALOG_BUTTON_COUNT-2 )) ? GAMEPAD_MAX_CROSSTALK : 200;

			if ( aucAnalogButtonData[ iLoop ] < ucThreshold )
			{
				// Allow the analog values of the trigger buttons to pass through
				if (iLoop < (PAD_ANALOG_BUTTON_COUNT-2))
				{
					aucAnalogButtonData[ iLoop ] = 0;
				}
			}
			else
				uiButtonData |= ( ( 1 << iLoop ) << PAD_ATOD_SHIFT );

			// We make sure button pressures are in the range 0.0f to 1.0f, taking into account crosstalk values. 
			float	fAnalogButton = ( float )( ntstd::Max( 0, ( aucAnalogButtonData[ iLoop ] - GAMEPAD_MAX_CROSSTALK ) ) ) / ( float )( 255.0f - ( float )GAMEPAD_MAX_CROSSTALK );

			m_afButtonVelocity[ iLoop ] = fAnalogButton - m_afButtonPressure[ iLoop ];
			m_afButtonPressure[ iLoop ] = fAnalogButton;
		}

		// Update pressed/released/held button states (these take into account analog->digital conversion too)
	
		u_int uiDelta	= uiButtonData ^ m_uiHeld;
		m_uiReleased	= m_uiHeld & uiDelta;
		m_uiPressed		= uiButtonData & uiDelta;
		m_uiHeld		= uiButtonData;

		// Update analog stick values (angle, magnitude, X & Y)

		m_pobAnalogLeft->Update( ( short )obJoyState.lX, ( short )obJoyState.lY );
		m_pobAnalogRight->Update( ( short )obJoyState.lRx, ( short )obJoyState.lRy );

#ifdef RUMBLE_ENABLED
		// If we have a feedback device
		if ( m_hFeedBack )
		{
			// Make sure the effect is created
			if ( !m_bRumbleInitialised )
				InitialiseRumble();

			// If a rumble request has been made this frame then make
			// sure that the effect is currently running
			if ( m_bRumble )
			{
				m_hFeedBack->Start( 1, 0 );
				m_bRumble = false;
			}
	
			// ...otherwise if no rumble is requested then make 
			// sure the effect is not playing
			else
			{
				m_hFeedBack->Stop();
			}
		}
#endif
	}
}	

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::Update(CInputPadCapture* pobInputCapture)
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CInputPad_Xbox::Update(const CInputPadCapture* pobInputCapture)
{
	UNUSED(pobInputCapture);
	ntError_p(0, ("Not implemented\n"));
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::GetInputCapture
*
*	DESCRIPTION		Get the captured values
*
***************************************************************************************************/

const CInputPadCapture* CInputPad_Xbox::GetInputCapture() const
{
	ntError_p(0, ("Not implemented\n"));
	return 0;
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::CAnalogState_Xbox::CAnalogState_Xbox
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CInputPad_Xbox::CAnalogState_Xbox::CAnalogState_Xbox()
:	m_bIsCalibrated( false )
{
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_Xbox::CAnalogState_Xbox::GenerateFloats
*
*	DESCRIPTION		Generates floats from -1 to +1 from a pad/driver specific short input
*
***************************************************************************************************/

void CInputPad_Xbox::CAnalogState_Xbox::GenerateFloats( short sX, short sY, float& fX, float& fY )
{

	fX = ( ( float )sX + 0.5f ) / 32767.5f;
	fY = ( ( float )sY + 0.5f ) / 32767.5f;

	// Decide what our dead zone is based on the pad calibration state.
	float	fInputDeadZone;

	if ( !m_bIsCalibrated )
		fInputDeadZone = INITIAL_DEADZONE;
	else
		fInputDeadZone = FINAL_DEADZONE;

	// Now scale the input based on the appropriate dead zone

	fX = ( fX >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fX ) - fInputDeadZone ) / ( 1.0f - fInputDeadZone ) );
	fY = ( fY >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fY ) - fInputDeadZone ) / ( 1.0f - fInputDeadZone ) );

	// Initially our sticks are in a state where calibration hasn't happened. As such we need a much 
	// larger dead zone until we've moved in all directions. This behaviour was inspired (if that's
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
*	FUNCTION		CInputPad_PS2::CInputPad_PS2
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CInputPad_PS2::CInputPad_PS2()
{
	// Construct our overriden analogue container class
	m_pobAnalogLeft = NT_NEW CAnalogState_PS2Left();
	m_pobAnalogRight = NT_NEW CAnalogState_PS2Right();
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::~CInputPad_PS2
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CInputPad_PS2::~CInputPad_PS2()
{
	// Clear our overriden analogue container class
	NT_DELETE( m_pobAnalogLeft );
	NT_DELETE( m_pobAnalogRight );
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::Update
*
*	DESCRIPTION		Updates values & states associated with input devices
*
***************************************************************************************************/

void CInputPad_PS2::Update( float fTimeDelta)
{
	UNUSED(fTimeDelta);

	// Flag no pad ubntil we find one
	m_ePadType = PAD_TYPE_NONE;

	// Do we have a valid device to read?
	if ( !m_hDevice )
	{
		return;
	}

	// Read hardware state
	DIJOYSTATE2	obJoyState;
	memset( &obJoyState, 0, sizeof( obJoyState ) );

	// See if we are in contact with our device
	HRESULT hRes = m_hDevice->Poll();

	// If we could poll get the device state information
	if ( SUCCEEDED( hRes ) )
	{
		hRes = m_hDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &obJoyState );
	}

	// If we failed clean up and drop out
	if ( FAILED( hRes ) )
	{
		if ( ( hRes == DIERR_INPUTLOST ) || ( hRes == DIERR_NOTACQUIRED ) )
			m_hDevice->Acquire();

		LPDIRECTINPUTDEVICE8 hTempDevice = m_hDevice;
		Initialise();
		m_hDevice = hTempDevice;
		return;
	}

	//flag that we are still present
	m_ePadType = PAD_TYPE_PRESENT;

	// Turn DIJOYSTATE2 information into a nice bitmask. If you can think of a nicer way of
	// doing this, then please let me know. 

	// Clear the values
	m_obCapture.m_uiButtonData = 0;
	u_char	aucAnalogButtonData[ PAD_ANALOG_BUTTON_COUNT ];
	memset( aucAnalogButtonData, 0, sizeof( aucAnalogButtonData ) );

	// First, sort out buttons that are actually analog on the Xbox controller..		
	if ( obJoyState.rgbButtons[0] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_TRIANGLE;
	if ( obJoyState.rgbButtons[1] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_CIRCLE;
	if ( obJoyState.rgbButtons[2] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_CROSS; 
	if ( obJoyState.rgbButtons[3] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_SQUARE;
	if ( obJoyState.rgbButtons[4] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_L2;
	if ( obJoyState.rgbButtons[5] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_R2;
	if ( obJoyState.rgbButtons[6] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_L1;
	if ( obJoyState.rgbButtons[7] & 0x80 )	 m_obCapture.m_uiButtonData |= PAD_R1;

	// Next, do the standard digital buttons
	if ( obJoyState.rgbButtons[8] & 0x80 )	{ m_obCapture.m_uiButtonData |= PAD_START; };
	if ( obJoyState.rgbButtons[9] & 0x80 )	{ m_obCapture.m_uiButtonData |= PAD_SELECT; };
	if ( obJoyState.rgbButtons[10] & 0x80 )	{ m_obCapture.m_uiButtonData |= PAD_LEFT_THUMB; };
	if ( obJoyState.rgbButtons[11] & 0x80 )	{ m_obCapture.m_uiButtonData |= PAD_RIGHT_THUMB; };

	// Then interpret POV data as directional component
	switch( obJoyState.rgdwPOV[0] )
	{
	case ( 0 ):			m_obCapture.m_uiButtonData |= ( PAD_UP );				break;		// 0x00000000
	case ( 4500 ):		m_obCapture.m_uiButtonData |= ( PAD_UP | PAD_RIGHT );	break;		// 0x00001194
	case ( 9000 ):		m_obCapture.m_uiButtonData |= ( PAD_RIGHT );			break;		// 0x00002328
	case ( 13500 ):		m_obCapture.m_uiButtonData |= ( PAD_RIGHT | PAD_DOWN );	break;		// 0x000034bc
	case ( 18000 ):		m_obCapture.m_uiButtonData |= ( PAD_DOWN );				break;		// 0x00004650
	case ( 22500 ):		m_obCapture.m_uiButtonData |= ( PAD_LEFT | PAD_DOWN );	break;		// 0x000057e4
	case ( 27000 ):		m_obCapture.m_uiButtonData |= ( PAD_LEFT );				break;		// 0x00006978
	case ( 31500 ):		m_obCapture.m_uiButtonData |= ( PAD_LEFT | PAD_UP );	break;		// 0x00007b0c
	default:																	break;		// 0xffffffff
	}
	
	// Store the hardware values for the two analog sticks
	m_obCapture.m_iHardwareX = obJoyState.lX;
	m_obCapture.m_iHardwareY = obJoyState.lY;
	m_obCapture.m_iHardwareZ = obJoyState.lZ;
	m_obCapture.m_iHardwareR = obJoyState.lRz;

	// Call the shared update function
	UpdateShared();
}


	

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::Update
*
*	DESCRIPTION		Updates values & states associated with input devices from a text source
*
***************************************************************************************************/

void CInputPad_PS2::Update(const CInputPadCapture* pobInputCapture)
{
	// Copy contents of the capture
	m_obCapture = *pobInputCapture;

	//flag that we are still present
	m_ePadType = PAD_TYPE_PRESENT;

	// Call the shared update function
	UpdateShared();
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::UpdateShared
*
*	DESCRIPTION		Common update functionality for the two update functions
*
***************************************************************************************************/

void CInputPad_PS2::UpdateShared()
{
	// Update analog stick values (X & Y)
	m_pobAnalogLeft->Update((short)m_obCapture.m_iHardwareX, (short)m_obCapture.m_iHardwareY );
	m_pobAnalogRight->Update((short)m_obCapture.m_iHardwareZ, (short)m_obCapture.m_iHardwareR );

	// Process analog button pressures
	int iMask = 256;
	for (int iLoop = 0; iLoop < 8; iLoop++)
	{
		// Generate a float value from the input bits
		float fAnalogButton = (m_obCapture.m_uiButtonData & iMask) ? 1.0f : 0.0f;

		// Generate velocity and pressure
		m_afButtonVelocity[ iLoop ] = fAnalogButton - m_afButtonPressure[ iLoop ];
		m_afButtonPressure[ iLoop ] = fAnalogButton;

		// Move the mask up
		iMask <<= 1;
	}

	// Update pressed/released/held button states (these take into account analog->digital conversion too)
	u_int uiDelta	= m_obCapture.m_uiButtonData ^ m_uiHeld;
	m_uiReleased	= m_uiHeld & uiDelta;
	m_uiPressed		= m_obCapture.m_uiButtonData & uiDelta;
	m_uiHeld		= m_obCapture.m_uiButtonData;

#ifdef RUMBLE_ENABLED
	// If we have a feedback device
	if ( m_hFeedBack )
	{
		// Make sure the effect is created
		if ( !m_bRumbleInitialised )
			InitialiseRumble();

		// If a rumble request has been made this frame then make
		// sure that the effect is currently running
		if ( m_bRumble )
		{
			m_hFeedBack->Start( 1, 0 );
			m_bRumble = false;
		}

		// ...otherwise if no rumble is requested then make 
		// sure the effect is not playing
		else
		{
			m_hFeedBack->Stop();
		}
	}
#endif
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::GetInputCapture
*
*	DESCRIPTION		Get the captured values
*
***************************************************************************************************/

const CInputPadCapture* CInputPad_PS2::GetInputCapture() const
{
	return &m_obCapture;
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::CAnalogState_PS2Right::GenerateFloats
*
*	DESCRIPTION		The right analogue stick on the PS2 pad with the current drivers gives us data
*					on the right and left z values - strangely.
*
*	INPUTS			sX - 0 left -> 256 right
*					sY - 0 top --> 256 down
*
***************************************************************************************************/

void CInputPad_PS2::CAnalogState_PS2Right::GenerateFloats( short sX, short sY, float& fX, float& fY )
{
	// Generate our initial float estimate
	fX = ( ( float )sX - 128.0f ) / 128.0f;
	fY = ( ( float )sY - 128.0f ) / 128.0f;

	// Handle the dead zone 
	switch( CInputHardware::Get().DeadZoneMode() )
	{
		case DZ_NONE:
			break;

		// We have no hardware calibration so we use the FINAL_DEADZONE and rely on external calibration
		case DZ_STD:
			fX = ( fX >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fX ) - FINAL_DEADZONE ) / ( 1.0f - FINAL_DEADZONE ) );
			fY = ( fY >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fY ) - FINAL_DEADZONE ) / ( 1.0f - FINAL_DEADZONE ) );
			break;
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::CAnalogState_PS2Left::GenerateFloats
*
*	DESCRIPTION		Generates float values ( -1 - 1 ) from the left stick short input.
*
***************************************************************************************************/

void CInputPad_PS2::CAnalogState_PS2Left::GenerateFloats( short sX, short sY, float& fX, float& fY )
{
	// Generate our initial float estimate
	fX = ( ( float )sX + 0.5f ) / 32767.5f;
	fY = ( ( float )sY + 0.5f ) / 32767.5f;

	// Handle the dead zone 
	switch( CInputHardware::Get().DeadZoneMode() )
	{
		case DZ_NONE:
			break;

		// We have no hardware calibration so we use the FINAL_DEADZONE and rely on external calibration
		case DZ_STD:
			fX = ( fX >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fX ) - FINAL_DEADZONE ) / ( 1.0f - FINAL_DEADZONE ) );
			fY = ( fY >= 0.0f ? 1.0f : -1.0f ) * ntstd::Max( 0.0f, ( fabsf( fY ) - FINAL_DEADZONE ) / ( 1.0f - FINAL_DEADZONE ) );
			break;
	}
}

/***************************************************************************************************
*	
*	FUNCTION		CInputPad_PS2::GetDebugStateString
*
*	DESCRIPTION		Dump our state if anything is pressed or released
*
***************************************************************************************************/

bool CInputPad_PS2::GetDebugStateString( char* pBuffer )
{
	bool bAnything = false;
	char* pCurrent = pBuffer;

	if (GetPressed() & PAD_UP)			{	sprintf( pCurrent, "PRESSED_UP " );			pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_DOWN)		{	sprintf( pCurrent, "PRESSED_DOWN " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_LEFT)		{	sprintf( pCurrent, "PRESSED_LEFT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_RIGHT)		{	sprintf( pCurrent, "PRESSED_RIGHT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_START)		{	sprintf( pCurrent, "PRESSED_START " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_BACK_SELECT) {	sprintf( pCurrent, "PRESSED_SELECT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_LEFT_THUMB)	{	sprintf( pCurrent, "PRESSED_LTHUMB " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_RIGHT_THUMB) {	sprintf( pCurrent, "PRESSED_RTHUMB " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

	if (GetPressed() & PAD_TRIANGLE)	{	sprintf( pCurrent, "PRESSED_TRIANGLE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_CIRCLE)		{	sprintf( pCurrent, "PRESSED_CIRCLE " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_CROSS)		{	sprintf( pCurrent, "PRESSED_CROSS " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_SQUARE)		{	sprintf( pCurrent, "PRESSED_SQUARE " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_L1)			{	sprintf( pCurrent, "PRESSED_L1 " );			pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_R1)			{	sprintf( pCurrent, "PRESSED_R1 " );			pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_L2)			{	sprintf( pCurrent, "PRESSED_L2 " );			pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetPressed() & PAD_R2)			{	sprintf( pCurrent, "PRESSED_R2 " );			pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

	if (GetReleased() & PAD_UP)			{	sprintf( pCurrent, "RELEASED_UP " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_DOWN)		{	sprintf( pCurrent, "RELEASED_DOWN " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_LEFT)		{	sprintf( pCurrent, "RELEASED_LEFT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_RIGHT)		{	sprintf( pCurrent, "RELEASED_RIGHT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_START)		{	sprintf( pCurrent, "RELEASED_START " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_BACK_SELECT) {	sprintf( pCurrent, "RELEASED_SELECT " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_LEFT_THUMB)	{	sprintf( pCurrent, "RELEASED_LTHUMB " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_RIGHT_THUMB) {	sprintf( pCurrent, "RELEASED_RTHUMB " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

	if (GetReleased() & PAD_TRIANGLE)	{	sprintf( pCurrent, "RELEASED_TRIANGLE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_CIRCLE)		{	sprintf( pCurrent, "RELEASED_CIRCLE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_CROSS)		{	sprintf( pCurrent, "RELEASED_CROSS " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_SQUARE)		{	sprintf( pCurrent, "RELEASED_SQUARE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_L1)			{	sprintf( pCurrent, "RELEASED_L1 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_R1)			{	sprintf( pCurrent, "RELEASED_R1 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_L2)			{	sprintf( pCurrent, "RELEASED_L2 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
	if (GetReleased() & PAD_R2)			{	sprintf( pCurrent, "RELEASED_R2 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

	if (bAnything)
	{
		if (GetHeld() & PAD_UP)			{	sprintf( pCurrent, "HELD_UP " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_DOWN)		{	sprintf( pCurrent, "HELD_DOWN " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_LEFT)		{	sprintf( pCurrent, "HELD_LEFT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_RIGHT)		{	sprintf( pCurrent, "HELD_RIGHT " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_START)		{	sprintf( pCurrent, "HELD_START " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_BACK_SELECT) {	sprintf( pCurrent, "HELD_SELECT " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_LEFT_THUMB)	{	sprintf( pCurrent, "HELD_LTHUMB " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_RIGHT_THUMB) {	sprintf( pCurrent, "HELD_RTHUMB " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

		if (GetHeld() & PAD_TRIANGLE)	{	sprintf( pCurrent, "HELD_TRIANGLE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_CIRCLE)		{	sprintf( pCurrent, "HELD_CIRCLE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_CROSS)		{	sprintf( pCurrent, "HELD_CROSS " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_SQUARE)		{	sprintf( pCurrent, "HELD_SQUARE " );	pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_L1)			{	sprintf( pCurrent, "HELD_L1 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_R1)			{	sprintf( pCurrent, "HELD_R1 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_L2)			{	sprintf( pCurrent, "HELD_L2 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }
		if (GetHeld() & PAD_R2)			{	sprintf( pCurrent, "HELD_R2 " );		pCurrent = pBuffer + strlen(pBuffer); bAnything = true; }

		// dump analogue state too
		sprintf( pCurrent, "\nLMAG(%.2f) LANG(%.0f) RMAG(%.2f) RANG(%.0f)\n", GetAnalogLMag(), GetAnalogLAngle() * RAD_TO_DEG_VALUE, GetAnalogRMag(), GetAnalogRAngle() * RAD_TO_DEG_VALUE );
	}

	return bAnything;
}

// -------------------------------------------------------------------------------------------------

#ifdef	DEBUG_KEYBOARD

// The table that maps a keycode to a textversion of the keycode
const char*		CInputKeyboard::m_apcKeyToText[ 256];


/***************************************************************************************************
*	
*	FUNCTION		CInputKeyboard::Initialise
*
*	DESCRIPTION		Handles initialisation of the debug keyboard
*
***************************************************************************************************/

void CInputKeyboard::Initialise()
{
	m_hDevice = 0;
	m_iModifiers = 0;
	memset(m_aucKeys, 0, sizeof(m_aucKeys));
	memset(m_aucLastKeyMatrix, 0, sizeof(m_aucLastKeyMatrix));

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



	KEY_TEXT( KEYC_JOYPAD_LEFT )
	KEY_TEXT( KEYC_JOYPAD_RIGHT )		
	KEY_TEXT( KEYC_JOYPAD_UP )	
	KEY_TEXT( KEYC_JOYPAD_DOWN )		
	KEY_TEXT( KEYC_JOYPAD_SELECT )	
	KEY_TEXT( KEYC_JOYPAD_START )		
	KEY_TEXT( KEYC_JOYPAD_TRIANGLE )	
	KEY_TEXT( KEYC_JOYPAD_CROSS )	
	KEY_TEXT( KEYC_JOYPAD_SQUARE )	
	KEY_TEXT( KEYC_JOYPAD_CIRCLE )	
	KEY_TEXT( KEYC_JOYPAD_L1 )	
	KEY_TEXT( KEYC_JOYPAD_L2 )		
	KEY_TEXT( KEYC_JOYPAD_R1 )		
	KEY_TEXT( KEYC_JOYPAD_R2 )		


#undef KEY_TEXT
}


/***************************************************************************************************
*	
*	FUNCTION		CInputKeyboard::Removed
*
*	DESCRIPTION		Handles removal of the debug keyboard device
*
***************************************************************************************************/

void	CInputKeyboard::Removed( void )
{
	ntAssert( false );
}

/***************************************************************************************************
*	
*	FUNCTION		CInputKeyboard::Inserted
*
*	DESCRIPTION		Handles insertion of the debug keyboard device
*
***************************************************************************************************/

void	CInputKeyboard::Inserted( int )
{
	ntAssert( false );
}

/***************************************************************************************************
*	
*	FUNCTION		CInputKeyboard::Update
*
*	DESCRIPTION		Updates the state of a single USB keyboard..
*
***************************************************************************************************/

void	CInputKeyboard::Update( void )
{
	HRESULT			hRes;

#if !defined( USE_BUFFERED_KEYBOARD_INPUT )
	// Clear our processed key array..
	memset( m_aucKeys, 0, sizeof( m_aucKeys ) );	
	m_iModifiers = KEYM_NONE;
#else
	// move any keys that were pressed down last frame to held
	for ( int iLoop = 0; iLoop < 256; iLoop++ )
	{
		if( m_aucKeys[iLoop] == KEYS_PRESSED )
			m_aucKeys[iLoop] = KEYS_HELD;

		if ( m_aucKeys[ iLoop ] == KEYS_RELEASED )
			m_aucKeys[ iLoop ] = KEYS_NONE;
	}
#endif

	if ( m_hDevice )
	{

		// If we are in playback then we shall get this structure from the playback
		memset(aucKeyMatrix, 0, sizeof(aucKeyMatrix));


#if defined( USE_BUFFERED_KEYBOARD_INPUT )

	    DIDEVICEOBJECTDATA didod[ SAMPLE_BUFFER_SIZE ];  // Receives buffered data 
		DWORD              dwElements = 0;
		dwElements = SAMPLE_BUFFER_SIZE;
		hRes = m_hDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
#else
		hRes = m_hDevice->Poll();
		if ( SUCCEEDED( hRes ) )
		{
			hRes = m_hDevice->GetDeviceState( sizeof( aucKeyMatrix ), &aucKeyMatrix ); 
		}

#endif

		if ( FAILED( hRes ) )
		{
			if ( ( hRes == DIERR_INPUTLOST ) || ( hRes == DIERR_NOTACQUIRED ) )
				m_hDevice->Acquire();

			LPDIRECTINPUTDEVICE8 hTempDevice = m_hDevice;
			Initialise();
			m_hDevice = hTempDevice;
			return;
			// fall through to allow the matrix to be updated.
			// This is so that captured keyboard input can still be processed even when the game does not have focus.
		}

		// Now process it into something chunky..
#if defined( USE_BUFFERED_KEYBOARD_INPUT )


		for( uint32_t i = 0; i < dwElements; i++ ) 
		{
			// do modifiers
			if( didod[ i ].dwOfs == DIK_LCONTROL || didod[ i ].dwOfs == DIK_RCONTROL )
			{
				if(didod[ i ].dwData & 0x80)
					m_iModifiers |= KEYM_CTRL;
				else 
					m_iModifiers &= ~KEYM_CTRL;
			} else if( didod[ i ].dwOfs == DIK_LMENU || didod[ i ].dwOfs == DIK_RMENU)
			{
				if(didod[ i ].dwData & 0x80)
					m_iModifiers |= KEYM_ALT;
				else 
					m_iModifiers &= ~KEYM_ALT;
			} else if( didod[ i ].dwOfs == DIK_LSHIFT || didod[ i ].dwOfs == DIK_RSHIFT)
			{
				if(didod[ i ].dwData & 0x80)
					m_iModifiers |= KEYM_SHIFT;
				else 
					m_iModifiers &= ~KEYM_SHIFT;
			} else
			{
				if( didod[ i ].dwData & 0x80 )
					m_aucKeys[ didod[ i ].dwOfs ] = KEYS_PRESSED;
				else
					m_aucKeys[ didod[ i ].dwOfs ] = KEYS_RELEASED;
			}
		}
		
#else

		if ( aucKeyMatrix[ DIK_LCONTROL ] || aucKeyMatrix[ DIK_RCONTROL ] )	m_iModifiers |= KEYM_CTRL;
		if ( aucKeyMatrix[ DIK_LMENU ] || aucKeyMatrix[ DIK_RMENU ] )		m_iModifiers |= KEYM_ALT;
		if ( aucKeyMatrix[ DIK_LSHIFT ] || aucKeyMatrix[ DIK_RSHIFT ] )		m_iModifiers |= KEYM_SHIFT;

		for ( int iLoop = 0; iLoop < 256; iLoop++ ) 
		{
			if ( ( aucKeyMatrix[ iLoop ] == 0x80 ) && ( m_aucLastKeyMatrix[ iLoop ] == 0x00 ) )
				m_aucKeys[ iLoop ] = KEYS_PRESSED;
			else
			if ( ( aucKeyMatrix[ iLoop ] == 0x00 ) && ( m_aucLastKeyMatrix[ iLoop ] == 0x80 ) )
				m_aucKeys[ iLoop ] = KEYS_RELEASED;
			else
			if ( ( aucKeyMatrix[ iLoop ] == 0x80 ) && ( m_aucLastKeyMatrix[ iLoop ] == 0x80 ) )
				m_aucKeys[ iLoop ] = KEYS_HELD;

		}

		// We need to keep this current key matrix set for processing during next update.
		NT_MEMCPY( m_aucLastKeyMatrix, aucKeyMatrix, sizeof( aucKeyMatrix ) ) ;

#endif
	}


}


void CInputKeyboard::EncodeKeyPressToString(char* pcBuffer, int iLength)
{
	UNUSED(iLength);
	char* pcIt = pcBuffer;
	
	// Clear the buffer
	*pcBuffer = 0;

	// Go through all the keys and see what is pressed. 
	for (int i = 0; i <=256; i++)
	{
		// Key occlusion list, place any keys you DON'T want recording into this list.

		switch(i)
		{
		// Keys to be excluded
		case KEYC_Q:
		case KEYC_ENTER:
		case KEYC_JOYPAD_LEFT:
		case KEYC_JOYPAD_RIGHT:
		case KEYC_JOYPAD_UP:
		case KEYC_JOYPAD_DOWN:
		case KEYC_JOYPAD_START:
		case KEYC_JOYPAD_SELECT:
			continue;
			break;

		// Any other key
		default:
			break;
		};

		if (m_aucKeys[i] == KEYS_PRESSED)
		{
			int iMove =	sprintf(pcIt, "%3d,", i);
			pcIt += iMove;
		}

		if (m_aucKeys[i] == KEYS_RELEASED)
		{
			int iMove =	sprintf(pcIt, "%3d,", i | 0x100);	// 0x100 indicates it was a key release
			pcIt += iMove;
		}
	}
}


void CInputKeyboard::DecodeStringToKeypress(char* pcBuffer)
{
	char* pcTok = strtok(pcBuffer, ",");
	while (pcTok != 0)
	{
		int iValue = atoi(pcTok);
		m_aucKeys[iValue & 0xff] = (iValue & 0x100) ? (u_char)KEYS_RELEASED : (u_char)KEYS_PRESSED;
		pcTok = strtok(0, ",");
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CInputKeyboard::KeyCodeToAscii
*
*	DESCRIPTION		Translates a keycode to an ascii char
*
***************************************************************************************************/

char CInputKeyboard::KeyCodeToAscii( KEY_CODE eKey, int iModifierMask )
{
	iModifierMask;
	HKL hLayout = GetKeyboardLayout(0);
	uint8_t aucState[256];
	WORD usResult;

	if (GetKeyboardState((PBYTE)aucState)==FALSE)
		return 0;

	unsigned int uiVk = MapVirtualKeyEx((uint32_t)eKey, 1, hLayout);
	if( ToAsciiEx(uiVk, (uint32_t)eKey, aucState, &usResult, 0, hLayout) == 0 )
		return 0;

	return (char)usResult;
}

#endif	// DEBUG_KEYBOARD
