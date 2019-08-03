/***************************************************************************************************
*
*	DESCRIPTION		Filters user input into a form that is meaningful to the user interface
*
*	NOTES			Basically this code further abstracts any input from the pad code into UI
*					relevant commands - e.g. Up, Down, Select, Back, Start...
*
***************************************************************************************************/

// Includes
#include "guiinput.h"
#include "game/shellconfig.h"

// Input timing constants in seconds
#define PRESS_PAUSE	( 0.1f )

// Input details for analogue stick acceptance
#define ANALOGUE_ANGLE_TOLERANCE		( QUARTER_PI / 4.0f )
#define ANALOGUE_MAGNITUDE_ACCEPTANCE	( 0.6f )

const CStringUtil::STRING_FLAG CGuiInput::ms_astrAcceptsInputFlags[] = 
{
	{ CGuiInput::ACCEPTS_NONE,		"NONE"		},
	{ CGuiInput::ACCEPTS_ANY,		"ANY"		},
	{ CGuiInput::ACCEPTS_LEFT,		"LEFT"		},
	{ CGuiInput::ACCEPTS_RIGHT,		"RIGHT"		},
	{ CGuiInput::ACCEPTS_UP,		"UP"		},
	{ CGuiInput::ACCEPTS_DOWN,		"DOWN"		},
	{ CGuiInput::ACCEPTS_START,		"START"		},
	{ CGuiInput::ACCEPTS_SELECT,	"SELECT"	},
	{ CGuiInput::ACCEPTS_BACK,		"BACK"		},
	{ CGuiInput::ACCEPTS_DELETE,	"DELETE"	},
	{ CGuiInput::ACCEPTS_ALL,		"ALL"		},
	{ 0,							0},
};

/***************************************************************************************************
*
*	FUNCTION		CGuiInput::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiInput::Initialise( void )
{
	// Do we need to update pad localisation settings
	// can move to update if this is to change via an ingame option
	if (  g_ShellOptions->m_bGuiXOSwap != m_bLocalisation )
	{
		m_bLocalisation = g_ShellOptions->m_bGuiXOSwap;
		if ( m_bLocalisation ) // JAP
		{
			m_eSelectPress = PAD_FACE_2;
			m_eCancelPress = PAD_FACE_3;
		}
		else	// US/UK
		{
			m_eSelectPress = PAD_FACE_3;
			m_eCancelPress = PAD_FACE_1;
		}
	}

	// Zero the timers
	InitialiseTimers();

	// Initialise the pad status
	UpdatePadStatus();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::Update
*
*	DESCRIPTION		Delays with the passing of button response delays.
*
***************************************************************************************************/

void CGuiInput::Update( void )
{
	for ( int iCount = 0; iCount < PAD_NUM; iCount++ )
	{
		m_aobPadTimers[ iCount ].Update();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::InitialiseTimers
*
*	DESCRIPTION		Sets the delay timers to their pause values 
*
***************************************************************************************************/

void CGuiInput::InitialiseTimers( void )
{
	for ( int iCount = 0; iCount < PAD_NUM; iCount++ )
	{
		m_aobPadTimers[ iCount ].Set( PRESS_PAUSE );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::UpdatePadStatus
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiInput::UpdatePadStatus( void )
{
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		m_abPadStatus[iPad] = CInputHardware::Get().GetPad( ( PAD_NUMBER )iPad ).IsValid();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::PadStatusChange
*
*	DESCRIPTION		Returns a mask displaying which pads have changed status since the last request.
*					If further information is required then you'll need to ask what the current
*					status is afterwards.
*
***************************************************************************************************/

int CGuiInput::PadStatusChange( void )
{
	if (m_bInputBlocked)
		return 0;

	// Initialise the return value
	int iWhichPads = 0;

	// Check for a change of any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		// Is the current status different from the saved status?
		if ( m_abPadStatus[iPad] != CInputHardware::Get().GetPad( ( PAD_NUMBER )iPad ).IsValid() ) 
		{
			// Set the return mask
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	// Save the current status
	UpdatePadStatus();

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::GetCurrentStatus
*
*	DESCRIPTION		Tells you the current status of the input pads
*
***************************************************************************************************/

int CGuiInput::GetCurrentStatus( void )
{
	if (m_bInputBlocked)
		return 0;

	// Initialise the return value
	int iWhichPads = 0;

	// Set the mask from the current status of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( m_abPadStatus[iPad] ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::GetPadCurrentStatus
*
*	DESCRIPTION		Tells you the current status of an input pad - as held by the filter class
*
***************************************************************************************************/

bool CGuiInput::GetPadCurrentStatus( PAD_NUMBER ePad )
{
	return m_bInputBlocked ? false : m_abPadStatus[ePad];
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::Movedown
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'move down' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::MoveDown( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadMovement( ( PAD_NUMBER )iPad, PAD_DOWN, PI, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::MoveUp
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'move up' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::MoveUp( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadMovement( ( PAD_NUMBER )iPad, PAD_UP, 0.0f, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::MoveLeft
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'move left' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::MoveLeft( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadMovement( ( PAD_NUMBER )iPad, PAD_LEFT, PI + HALF_PI, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::MoveRight
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'move right' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::MoveRight( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadMovement( ( PAD_NUMBER )iPad, PAD_RIGHT, HALF_PI, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::StartPress
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'start' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::StartPress( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, PAD_START, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::SelectPress
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'select' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::SelectPress( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, m_eSelectPress, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::Delete
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'delete' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::Delete( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, m_eCancelPress, bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::Back
*
*	DESCRIPTION		Returns bitwise flags to indicate if a 'back' command has been issued to
*					the gui on any of the input pads.
*
***************************************************************************************************/

int CGuiInput::Back( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for action on any of the pads
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, ( m_eCancelPress | PAD_BACK_SELECT ), bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiInput::AnyAction
*
*	DESCRIPTION		Returns true if a command suitable for interupting the attract mode has been
*					issued by any controllers.
*
***************************************************************************************************/

int CGuiInput::AnyAction( bool bGetRaw )
{
	// Initialise the return value
	int iWhichPads = 0;

	// Check for any presses on the face buttons
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, ( PAD_FACE_1 | PAD_FACE_2 | PAD_FACE_3 | PAD_FACE_4 | PAD_BACK_SELECT | PAD_START ), bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	// Check for the triggers
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, ( PAD_TOP_1 | PAD_TOP_2 | PAD_TOP_3 | PAD_TOP_4 ), bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	// Check the directional pad
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, ( PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT ), bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	// Check the analogue sticks for presses
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAction( ( PAD_NUMBER )iPad, ( PAD_LEFT_THUMB | PAD_RIGHT_THUMB ), bGetRaw ) ) 
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	// Check for any movement on the analogue sticks
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( PadAnyAnalogueMovement( ( PAD_NUMBER )iPad, bGetRaw ) )
		{
			iWhichPads |= ( 0x01 << iPad );
		}
	}

	return iWhichPads;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::PadMovement
*
*	DESCRIPTION		Returns true if the specified pad is moving in the specified diection.  This
*					deals with movement timing issues on a per pad basis.
*
***************************************************************************************************/

bool CGuiInput::PadMovement( PAD_NUMBER ePadNumber, PAD_BUTTON eDigitalDirection, float fAnalogueAngle, bool bGetRaw )
{
	if (m_bInputBlocked)
		return false;

	// Check the left analogue stick
	if ( ( ( CInputHardware::Get().GetPad( ePadNumber ).GetAnalogLAngle() > ( fAnalogueAngle - ANALOGUE_ANGLE_TOLERANCE ) )
		   &&
		   ( CInputHardware::Get().GetPad( ePadNumber ).GetAnalogLAngle() < ( fAnalogueAngle + ANALOGUE_ANGLE_TOLERANCE ) ) 
		   &&
		   ( CInputHardware::Get().GetPad( ePadNumber ).GetAnalogLMag() > ANALOGUE_MAGNITUDE_ACCEPTANCE ) )
		 ||
		// And also look at the digital pad
		( CInputHardware::Get().GetPad( ePadNumber ).GetHeld() & ( eDigitalDirection ) ) )
	{
		if (bGetRaw)
			return true;
		if ( m_aobPadTimers[ ePadNumber ].Passed() )
		{
			// Reset the timer to stop immediate presses
			m_aobPadTimers[ ePadNumber ].Set( PRESS_PAUSE );

			return true;
		}
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::PadAnyAnalogueMovement
*
*	DESCRIPTION		Returns true if the specified pad has any diectional analogue commands above the
*                   accepted magnitude.  This deals with movement timing issues on a per pad basis.
*
***************************************************************************************************/

bool CGuiInput::PadAnyAnalogueMovement( PAD_NUMBER ePadNumber, bool bGetRaw  )
{
	if (m_bInputBlocked)
		return false;

	// Check the left analogue stick
	if ( CInputHardware::Get().GetPad( ePadNumber ).GetAnalogLMag() > ANALOGUE_MAGNITUDE_ACCEPTANCE )
	{
		if (bGetRaw)
			return true;
		if ( m_aobPadTimers[ ePadNumber ].Passed() )
		{
			// Reset the timer to stop immediate presses
			m_aobPadTimers[ ePadNumber ].Set( PRESS_PAUSE );

			return true;
		}
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiInput::PadAction
*
*	DESCRIPTION		Returns true if the selected buttons are pressed on the selected pad - i.e.
*					uiPadButtons = ( PAD_B | PAD_BACK_SELECT ) would constitute a back action.
*
***************************************************************************************************/

bool CGuiInput::PadAction( PAD_NUMBER ePadNumber, u_int uiPadButtons, bool bGetRaw )
{
	if (m_bInputBlocked)
		return false;

	if ( CInputHardware::Get().GetPad( ePadNumber ).GetPressed() & uiPadButtons )
	{
		if (bGetRaw)
			return true;
		if ( m_aobPadTimers[ ePadNumber ].Passed() )
		{
			// Reset the timer to stop immediate presses
			m_aobPadTimers[ ePadNumber ].Set( PRESS_PAUSE );

			return true;
		}
	}

	return false;
}



