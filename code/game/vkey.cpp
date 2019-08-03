/***************************************************************************************************
*
*	Virtual key mapping classes
*
*	CHANGES
*
*	19/02/2004	Mike	Created
*
***************************************************************************************************/

// Necessary includes
#include "game/vkey.h"
#include "input/inputhardware.h"
#include "editable/enumlist.h"
#include "game/shellconfig.h"

/***************************************************************************************************
*
*	FUNCTION		CVKeyLayout::CVKeyLayout
*
*	DESCRIPTION		Constructor. This (for the moment) is where a pad layout is defined.	
*
***************************************************************************************************/

CVKeyLayout::CVKeyLayout()
{
	// Set the pads up to best duit the controller types
	bool bUsePS2Pad = true;
	if ( bUsePS2Pad )
	{
		// ---- Build the VKey mappings ----
		//		CVKeyButton(ResultingVKey, PadInput)
		AddKey(NT_NEW CVKeyButton(AB_ATTACK_MEDIUM,PAD_FACE_4));
		AddKey(NT_NEW CVKeyButton(AB_ACTION,		PAD_FACE_3));
		AddKey(NT_NEW CVKeyButton(AB_ATTACK_FAST,	PAD_FACE_1));
		AddKey(NT_NEW CVKeyButton(AB_GRAB,			PAD_FACE_2));
		AddKey(NT_NEW CVKeyButton(AB_PSTANCE,		PAD_TOP_2));
		AddKey(NT_NEW CVKeyButton(AB_RSTANCE,		PAD_TOP_1));
		AddKey(NT_NEW CVKeyButton(AB_SPECIAL_A,	PAD_TOP_3));
		AddKey(NT_NEW CVKeyButton(AB_SPECIAL_B,	PAD_TOP_4));
		AddKey(NT_NEW CVKeyButton(AB_AIM,			PAD_LEFT_THUMB));

		// ---- Build the combiners ----
		// Note a time of zero will in fact delay both combining inputs by a frame
		// CVKeyCombiner(Resulting VKey, VKey1, VKey2, Combination time);
		AddCombiner(NT_NEW CVKeyCombiner(AB_SPECIAL, AB_SPECIAL_A, AB_SPECIAL_B, 0.05f));
	}
	else
	{
		// ---- Build the VKey mappings ----
		//		CVKeyButton(ResultingVKey, PadInput)
		AddKey(NT_NEW CVKeyButton(AB_ATTACK_MEDIUM,PAD_FACE_3));
		AddKey(NT_NEW CVKeyButton(AB_ACTION,		PAD_FACE_1));
		AddKey(NT_NEW CVKeyButton(AB_ATTACK_FAST,	PAD_FACE_4));
		AddKey(NT_NEW CVKeyButton(AB_GRAB,			PAD_FACE_2));
		AddKey(NT_NEW CVKeyButton(AB_PSTANCE,		PAD_TOP_3));
		AddKey(NT_NEW CVKeyButton(AB_RSTANCE,		PAD_TOP_4));
		AddKey(NT_NEW CVKeyButton(AB_SPECIAL,		PAD_TOP_1));
		AddKey(NT_NEW CVKeyButton(AB_AIM,			PAD_LEFT_THUMB));
	}

	if ( g_ShellOptions->m_bEvades )
	{
		//	CVKeyStick(ResultingVVKey, Stick number(0=left, 1=right), Magnitude, Minimum angle, Maximum angle)
		AddKey(NT_NEW CVKeyStick(AB_DODGE_RIGHT, 1, 0.5f, (TWO_PI/8.0f)*1, (TWO_PI/8.0f)*3));
		AddKey(NT_NEW CVKeyStick(AB_DODGE_LEFT, 1, 0.5f, (TWO_PI/8.0f)*5, (TWO_PI/8.0f)*7));
		AddKey(NT_NEW CVKeyStick(AB_DODGE_FORWARD, 1, 0.5f, (TWO_PI/8.0f)*0, (TWO_PI/8.0f)*1));
		AddKey(NT_NEW CVKeyStick(AB_DODGE_FORWARD, 1, 0.5f, (TWO_PI/8.0f)*7, (TWO_PI/8.0f)*8));
		AddKey(NT_NEW CVKeyStick(AB_DODGE_BACK, 1, 0.5f, (TWO_PI/8.0f)*3, (TWO_PI/8.0f)*5));
	}
}

/***************************************************************************************************
*
*	FUNCTION		CVKeyLayout::~CVKeyLayout
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CVKeyLayout::~CVKeyLayout()
{
	// Remove the keys
	while (!m_obVKeyList.empty())
	{
		NT_DELETE( m_obVKeyList.back() );
		m_obVKeyList.pop_back();
	}

	// Remove the combiners
	while (!m_obCombinerList.empty())
	{
		NT_DELETE( m_obCombinerList.back() );
		m_obCombinerList.pop_back();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CVKeyLayout::AddKey
*
*	DESCRIPTION		Add a virtual key mapping to the layout
*
***************************************************************************************************/

void CVKeyLayout::AddKey(CVKey* pobKey)
{
	ntAssert(pobKey);
	// Add it to the list
	m_obVKeyList.push_back(pobKey);
}


/***************************************************************************************************
*
*	FUNCTION		CVKeyLayout::AddCombiner
*
*	DESCRIPTION		Add a virtual key combiner to the layout
*
***************************************************************************************************/

void CVKeyLayout::AddCombiner(CVKeyCombiner* pobCombiner)
{
	ntAssert(pobCombiner);
	// Add it to the list
	m_obCombinerList.push_back(pobCombiner);
}



/***************************************************************************************************
*
*	FUNCTION		CVKeyButton::IsPressed
*
*	DESCRIPTION		Is the VKey pressed?
*
***************************************************************************************************/
bool CVKeyButton::IsPressed(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative)
{
	UNUSED(fStick1AngleCameraRelative);
	UNUSED(fStick2AngleCameraRelative);
	return 0 != (pobPad->GetPressed() & m_uiButtonValue);
}


/***************************************************************************************************
*
*	FUNCTION		CVKeyButton::IsPressed
*
*	DESCRIPTION		Is the VKey held? - JML Added
*
***************************************************************************************************/
bool CVKeyButton::IsHeld(CInputPad* pobPad, float, float)
{
	return 0 != (pobPad->GetHeld() & m_uiButtonValue);
}


/***************************************************************************************************
*
*	FUNCTION		CVKeyStick::IsHeld
*
*	DESCRIPTION		Is the VKey pressed? - JML Added
*
***************************************************************************************************/
bool CVKeyStick::IsHeld(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative)
{
	if (m_uiWhichStick == 0)
	{
		if ((pobPad->GetAnalogLMag() > m_fMagnitude) && (fStick1AngleCameraRelative >= m_fMinAngle) && (fStick1AngleCameraRelative < m_fMaxAngle)) 
		{
			return true;
		}
		return false;
	}

	if (m_uiWhichStick == 1)
	{
		if ((pobPad->GetAnalogRMag() > m_fMagnitude) && (fStick2AngleCameraRelative >= m_fMinAngle) && (fStick2AngleCameraRelative < m_fMaxAngle)) 
		{
			return true;
		}
		return false;
	}


	// Bad Stick number
	ntAssert(0);
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CVKeyStick::IsPressed
*
*	DESCRIPTION		Is the VKey pressed?
*
***************************************************************************************************/
bool CVKeyStick::IsPressed(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative)
{
	if(m_bHeld)
	{
		m_bHeld = IsHeld(pobPad, fStick1AngleCameraRelative, fStick2AngleCameraRelative);
		return false;
	}
	else
	{
		m_bHeld = IsHeld(pobPad, fStick1AngleCameraRelative, fStick2AngleCameraRelative);
		return m_bHeld;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CVKeyCombiner::Update
*
*	DESCRIPTION		Update a key combiner
*
***************************************************************************************************/

void CVKeyCombiner::Update(float fTimeDelta, u_int& uiVKeysHeld)
{
	// Update time
	m_fLastSingle += fTimeDelta;

	bool bEitherPressed = ( uiVKeysHeld & ( 1 << m_eKey1 ) ) || ( uiVKeysHeld & ( 1<< m_eKey2 ) );
	bool bBothPressed = ( uiVKeysHeld & ( 1 << m_eKey1 ) ) && ( uiVKeysHeld & ( 1 << m_eKey2 ) );

	// Update depending on state
	switch(m_eState)
	{
	case OPEN:
		// See if any keys are pressed
		if (!bEitherPressed)
		{
			// Neither pressed
			break;
		}
		// At least one is pressed
		m_fLastSingle = 0.0f;
		m_eState = WAITING_FOR_CLOSE;

		// Fall through to WAITING_FOR_CLOSE
	case WAITING_FOR_CLOSE:

		// See if we haven't pressed the other in time
		if (m_fLastSingle > m_fCombineTime)
		{
			m_eState = FAILED;
			break;
		}

		// Remove the constituent VKeys
		uiVKeysHeld &= ~( ( 1 << m_eKey1 ) | ( 1 << m_eKey2 ) );

		// When both are pressed, move through to closed
		if (bBothPressed)
		{
			// Both pressed
			m_eState = CLOSED;
			break;
		}
		break;

	case CLOSED:

		// Remove the constituent VKeys
		uiVKeysHeld &= ~( ( 1 << m_eKey1 ) | ( 1 << m_eKey2 ) );

		// Set the VKey !!!!!
		uiVKeysHeld |= ( 1 << m_eResult );
		
		// Check for a release
		if (!bBothPressed)
		{
			m_eState = WAITING_FOR_OPEN;
		}

		// Fall through to waiting for open
	case WAITING_FOR_OPEN:

		// Remove the constituent VKeys
		uiVKeysHeld &= ~( ( 1 << m_eKey1 ) | ( 1 << m_eKey2 ) );

		// Fall through to failed
	case FAILED:
		// Wait for both to be released
		if (!bEitherPressed)
		{
			m_eState = OPEN;
		}
		break;
    };
}



/***************************************************************************************************
*
*	FUNCTION		CVKeyManager::CVKeyManager
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CVKeyManager::CVKeyManager(CInputPad* pobPad)
:	m_pobPad( pobPad ),
	m_uiVKeysHeld( 0 ),
	m_uiVKeysPressed( 0 ),
	m_uiVKeysReleased( 0 )
{
	// Get pointer to our (currently) one static layout
	m_pobVKeyLayout = NT_NEW CVKeyLayout();
	ntAssert(m_pobVKeyLayout);
}

/***************************************************************************************************
*
*	FUNCTION		CVKeyManager::~CVKeyManager
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CVKeyManager::~CVKeyManager()
{
	NT_DELETE(	m_pobVKeyLayout );
}


/***************************************************************************************************
*
*	FUNCTION		CVKeyManager::Reset
*
*	DESCRIPTION		Return the key manager to a known state
*
***************************************************************************************************/

void CVKeyManager::Reset()
{
	// Reset the combiners
	for (ntstd::List<CVKeyCombiner*>::iterator obIt = m_pobVKeyLayout->m_obCombinerList.begin(); obIt != m_pobVKeyLayout->m_obCombinerList.end(); ++obIt)
	{
		(*obIt)->Reset();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CVKeyManager::Update
*
*	DESCRIPTION		Update the VKey manager by mapping all VKey inputs and then applying combiners.
*
***************************************************************************************************/
void CVKeyManager::Update(float fTimeDelta, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative)
{
	// Store the old values
	const u_int uiOldVKeysHeld = m_uiVKeysHeld;

	// Collect held results
	m_uiVKeysHeld = 0;
	m_uiVKeysPressed = 0;
	m_uiVKeysReleased = 0;

	// Update the VKey list
	for (ntstd::List<CVKey*>::iterator obIt = m_pobVKeyLayout->m_obVKeyList.begin(); obIt != m_pobVKeyLayout->m_obVKeyList.end(); ++obIt)
	{
		if ((*obIt)->IsPressed(m_pobPad, fStick1AngleCameraRelative, fStick2AngleCameraRelative))
		{
			// Set VKey
			m_uiVKeysPressed |= ( 1<< (*obIt)->GetResult() );
		}

		if ((*obIt)->IsHeld(m_pobPad, fStick1AngleCameraRelative, fStick2AngleCameraRelative))
		{
			// Set VKey
			m_uiVKeysHeld |= ( 1<< (*obIt)->GetResult() );
		}
	}

	// Update the combiner list
	for (ntstd::List<CVKeyCombiner*>::iterator obIt = m_pobVKeyLayout->m_obCombinerList.begin(); obIt != m_pobVKeyLayout->m_obCombinerList.end(); ++obIt)
	{
		(*obIt)->Update(fTimeDelta, m_uiVKeysHeld);
	}

	// Build the released VKeys
	m_uiVKeysReleased = uiOldVKeysHeld & ~m_uiVKeysHeld;
}

