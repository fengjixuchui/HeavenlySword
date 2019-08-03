/***************************************************************************************************
*
*	DESCRIPTION		Describes the 'pages' of the GUI
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiscreen.h"
#include "guiunit.h"
#include "guimanager.h"
#include "guisound.h"
#include "guiflow.h"
#include "core\timer.h"

#include "gui/guisettings.h"

#include "game/shellconfig.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiScreen(); }

// Register this class under it's XML tag
bool g_bSCREEN = CGuiManager::Register( "SCREEN", &ConstructWrapper );

CScreenHeader* CGuiScreen::ms_pobCurrentScreenHeader = NULL;
CGuiScreen* CGuiScreen::ms_pobCurrentScreen = NULL;

LUA_EXPOSED_START(CGuiScreen)
LUA_EXPOSED_END(CGuiScreen)

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::CGuiScreen
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/


bool CGuiScreen::ProcessEnd( void )
{
	CGuiUnit::ProcessEnd();
	ms_pobCurrentScreen = NULL;
	return true;
}

CGuiScreen::CGuiScreen( void )
{
	ATTACH_LUA_INTERFACE(CGuiScreen);

	m_iScreenFlags = 0;
	m_iAcceptsInput = CGuiInput::ACCEPTS_UNKNOWN;
	m_pobScreenHeader = ms_pobCurrentScreenHeader;

	ms_pobCurrentScreen = this;

	m_bUpdateGame = true;

	m_fScreenFade = 0.0f;
	m_bFadeRunning = false;
	m_fScreenFadeDestination = 1.0f;

	//override
	m_fScreenFadeScale = 1.0f / CGuiManager::Get().GuiSettings()->ScreenFadeTime();

	m_bShowFilter = false;

	m_bHoldFadeIn = false;

//	m_bCrossFadeEntering = true;

	if (g_ShellOptions->m_eFrontendMode == FRONTEND_TGS)
		m_bCrossFadeExiting = true;
	else
		m_bCrossFadeExiting = false;

}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::~CGuiScreen
*
*	DESCRIPTION		Desruction
*
*					There is no need to kill the units here since this structure is built up at the
*					CXMLElement level - and will be destroyed by it too.
*
***************************************************************************************************/

CGuiScreen::~CGuiScreen( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::Render
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiScreen::Render( void )
{
	if (!m_bAllowRender)
		return false;

	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		if (( *obIt )->AllowRender())
			( *obIt )->Render();
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateFadeIn
*
*	DESCRIPTION		Interpolate our fade up to 1.0
*
***************************************************************************************************/

void CGuiScreen::UpdateFadeIn( void )
{
	m_fScreenFade += CTimer::Get().GetSystemTimeChange() * m_fScreenFadeScale;
	if (m_fScreenFade >= 1.0f ) //|| !m_bAutoFade)
	{
		m_fScreenFade = 1.0f;
		m_bFadeRunning = false;
	}

	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	UpdateAnimations();

	// If we are done, try to focus on the selected child element
	if ( !IsAnimating() )
	{
		// Check all the child elements are idle
		bool bAreEntering = false;

		for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
		{
			bAreEntering |= !( *obIt )->IsInteractive();
		}

		// If they are all done then we are too
		if ( !bAreEntering )
		{
			SetStateIdle();
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateFadeOut
*
*	DESCRIPTION		Interpolate our fade down to 0.0
*
***************************************************************************************************/

void CGuiScreen::UpdateFadeOut( void )
{
	m_fScreenFade -= CTimer::Get().GetSystemTimeChange() * m_fScreenFadeScale;
	if (m_fScreenFade <= 0.0f ) //|| !m_bAutoFade)
	{
		m_fScreenFade = 0.0f;
		m_bFadeRunning = false;
	}

	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	UpdateAnimations();

	// If we are done, try to focus on the selected child element
	if ( !IsAnimating() )
	{
		// Check all the child elements are done fading
		bool bAreFading = false;

		for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
		{
			bAreFading |= (( *obIt )->GetState() == CGuiUnit::STATE_FADEOUT);
		}

		// If they are all done then we are too
		if ( !bAreFading )
		{
			SetStateExit();
		}
	}
}

void CGuiScreen::SetStateFadeIn( void )
{
	m_bFadeRunning = true;

	if (m_bHoldFadeIn)
		return;

	CGuiUnit::SetStateFadeIn();
}

void CGuiScreen::SetStateFadeOut( void )
{
	m_bFadeRunning = true;
	CGuiUnit::SetStateFadeOut();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateEnter
*
*	DESCRIPTION		We have to do some extra work before we come out of the entrance state.
*
***************************************************************************************************/

void CGuiScreen::UpdateEnter( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	// If we are done, try to focus on the selected child element
	if ( !IsAnimating() )
	{
		// Check all the child elements are fading
		bool bAreEntering = false;

		for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
		{
			bool bEntering = m_bAutoFade ? (( *obIt )->GetState() == CGuiUnit::STATE_ENTER) : !( *obIt )->IsInteractive();

			bAreEntering |= bEntering;
		}

		// If they are all done then we are too
		if ( !bAreEntering )
		{
//			SetStateIdle();
			SetStateFadeIn();
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateIdle
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiScreen::UpdateIdle( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateIdle();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateFocus
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiScreen::UpdateFocus( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocus();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiScreen::UpdateFocusIn( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocusIn();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateFocusOut
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiScreen::UpdateFocusOut( void )
{
	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		( *obIt )->Update();
	}

	// Update ourself
	CGuiUnit::UpdateFocusOut();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiScreen::UpdateExit( void )
{
	// Monitor of the children are alive
	bool bLiveChildren = false;

	// Update all the children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		bLiveChildren |= ( *obIt )->Update();
	}

	// Update the animation if there is one
	UpdateAnimations();

	// Are the children dead dear?
	if ( !bLiveChildren )
	{
		// If we are done too
		if ( !IsAnimating() )
		{
			SetStateDead();
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::BeginExit
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiScreen::BeginExit( bool bForce )
{
	// Call the base update first for this element
	bool bSuccess = CGuiUnit::BeginExit( bForce );

	// Now call it for all our children
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt )
	{
		bSuccess &= ( *obIt )->BeginExit( bForce );
	}

	// Returns true if all elements changed state 
	return bSuccess;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiScreen::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "acceptsinput" ) == 0 )
		{
			return GuiUtil::SetFlags(pcValue, &CGuiInput::ms_astrAcceptsInputFlags[0], &m_iAcceptsInput);
		}
		else if ( strcmp( pcTitle, "updategame" ) == 0 )
		{
			return GuiUtil::SetBool(pcValue, &m_bUpdateGame);
		}
		else if ( strcmp( pcTitle, "filter" ) == 0 )
		{
			return GuiUtil::SetBool(pcValue, &m_bShowFilter);
		}
		else if ( strcmp( pcTitle, "holdfadein" ) == 0 )
		{
			return GuiUtil::SetBool(pcValue, &m_bHoldFadeIn);
		}
		/*
		if ( strcmp( pcTitle, "something" ) == 0 )
		{
			return SetSomethingValue( pcValue );
		}

		else if ( strcmp( pcTitle, "somethingelse" ) == 0 )
		{
			return SetSomethingElseValue( pcValue );
		}
		*/

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::ProcessChild
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiScreen::ProcessChild( CXMLElement* pobChild )
{
	// Drop out if the data is shonky
	ntAssert ( pobChild );

	// The screen only knows that it holds a list of screen units
	m_obScreenUnits.push_back( ( CGuiUnit* )pobChild );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::AddGuiElement
*
*	DESCRIPTION		Add a newly created Gui element to the current screen.
*
***************************************************************************************************/
bool CGuiScreen::AddGuiElement( CGuiUnit* pobChild )
{
	// Drop out if the data is shonky
	ntAssert ( pobChild );

	// The screen only knows that it holds a list of screen units
	m_obScreenUnits.push_back( pobChild );
	
	// As the added element did not come from xml, we should add that too
	m_obChildren.push_back( pobChild);
	pobChild->SetParent( this );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::RemoveGuiElement
*
*	DESCRIPTION		Remove specified gui element if still available.
*
***************************************************************************************************/
bool CGuiScreen::RemoveGuiElement( CGuiUnit* pobChild )
{
	// Drop out if the data is shonky
	ntAssert ( pobChild );

	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		if ( ( *obIt ) == pobChild )
			pobChild->BeginExit( true );
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::MoveLeftAction
*
*	DESCRIPTION		If there is a move left command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::MoveLeftAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Move left", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->MoveLeftAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::MoveRightAction
*
*	DESCRIPTION		If there is a move right command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::MoveRightAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Move right", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->MoveRightAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::MoveDownAction
*
*	DESCRIPTION		If there is a move down command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::MoveDownAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Move down", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->MoveDownAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::MoveUpAction
*
*	DESCRIPTION		If there is a move up command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::MoveUpAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Move up", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->MoveUpAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::StartAction
*
*	DESCRIPTION		If there is a start command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::StartAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Start", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->StartAction( iPads );
	}

	// If nothing acted on this message then we need to move on alone
	if ( !bActedOn )
	{
		// If this is a HUD screen we should only go on to a Pause screen
		if ( m_iScreenFlags & CScreenHeader::SCREEN_HUD )
		{
			if (CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

			bActedOn = CGuiManager::Get().MoveOnScreenType(CScreenHeader::SCREEN_PAUSE);

			if (! CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);


		}
		else
		{
			if (CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

			bActedOn = CGuiManager::Get().MoveOnScreen();
			
			if (! CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
		}
	}

	// Did we act on this message?
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::SelectAction
*
*	DESCRIPTION		If there is a select command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::SelectAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Select", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->SelectAction( iPads );
	}

	// If nothing acted on this message then we need to move on alone
	if ( !bActedOn )
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		if ( CGuiManager::Get().MoveOnScreen() )
		{
			if (! CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
		}
	}

	// Did we act on this message?
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::BackAction
*
*	DESCRIPTION		If there is a back command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::BackAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;
	
	// A little debugging action
	DebugPadAction( "Back", iPads );

	// Set up our return value
	bool bActedOn = false;


	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->BackAction( iPads );
	}

	// If nothing acted on this message then we need to move back alone
	if ( !bActedOn )
	{
		if (! (m_iScreenFlags & CScreenHeader::SCREEN_NO_BACK ) )
		{
			if (CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_BACK);

			CGuiManager::Get().MoveBackScreen();
			
			if (! CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_BACK);
		}
	}

	// Did we act on this message?
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::DeleteAction
*
*	DESCRIPTION		If there is a 'delete' command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::DeleteAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Delete", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->DeleteAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::AnyAction
*
*	DESCRIPTION		If there is a 'delete' command from a controller it will be indicated in a
*					bitwise manner in iPads.  For example, if the is an action on PAD_2 only then
*					iPads = ( 0x01 << PAD_2 ).
*
***************************************************************************************************/

bool CGuiScreen::AnyAction( int iPads )
{
	// No point if there is no message
	if ( iPads == 0 ) return false;

	// Do not pass on if the screen is not in an interactive state
	if ( !IsInteractive() ) return false;

	// A little debugging action
	DebugPadAction( "Any", iPads );

	// Set up our return value
	bool bActedOn = false;

	// Pass this message on to any interctive units
	for( ntstd::List< CGuiUnit* >::iterator obIt = m_obScreenUnits.begin(); obIt != m_obScreenUnits.end(); ++obIt)
	{
		bActedOn |= ( *obIt )->AnyAction( iPads );
	}

	// Did we act on this message?
	return bActedOn;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiScreen::DebugPadAction
*
*	DESCRIPTION		Takes a string to say what has been done and a bitwise flag to show what pad(s)
*					that action occurred on.  It builds a nice string to tell us all about it.
*
***************************************************************************************************/

void CGuiScreen::DebugPadAction( const char* pcAction, int iPads )
{
#ifdef DEBUG_GUI_INPUT

	// We shouldn't be calling this if nothing was pressed
	ntAssert( iPads );

	// Count the number of bits set
	int iNumberOfPads = 0;
	int iPadsPressed[ PAD_NUM ] = { 0 };

	// Save the pads that have been pressed
	for ( int iPad = 0; iPad < PAD_NUM; ++iPad )
	{
		if ( ( 0x01 << iPad ) & iPads ) 
		{
			iPadsPressed[iNumberOfPads] = iPad + 1;
			++iNumberOfPads;
		}
	}

	// If we have one pad then the string its easy
	if ( iNumberOfPads == 1 )
	{
		ntPrintf( "%s was pressed on pad %d.\n", pcAction, iPadsPressed[0] );
	}

	// Otherwise it's not
	else
	{
		// Build a string, we can't do this in one go
		char	pcDebugString[MAX_PATH] = { 0 };
		int		iStringPosition = 0;

		// Start with the action and stuff
		iStringPosition += sprintf( pcDebugString,  "%s was pressed on pads ", pcAction );

		for ( int iPad = 0; iPad < iNumberOfPads; ++iPad )
		{
			// Continue with the pads used
			iStringPosition += sprintf( pcDebugString + iStringPosition,  "%d", iPadsPressed[iPad] );

			// Format the string so it's all nice
			if ( iPad == ( iNumberOfPads - 1 ) )
				iStringPosition += sprintf( pcDebugString + iStringPosition,  ".\n" );
			else if ( iPad == ( iNumberOfPads - 2 ) )
				iStringPosition += sprintf( pcDebugString + iStringPosition,  " and " );
			else
				iStringPosition += sprintf( pcDebugString + iStringPosition,  ", " );
		}

		// Make sure we haven't blatted over anything
		ntAssert( iStringPosition < MAX_PATH );

		// Print the string dag nammit
		ntPrintf( pcDebugString );
	}

#else

	// Unused formal parameters
	UNUSED( iPads );
	UNUSED( pcAction );

#endif // DEBUG_GUI_INPUT
}

void CGuiScreen::ProcessInput()
{
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_ANY))
	{
		AnyAction(CGuiInput::Get().AnyAction(true));
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_LEFT))
	{
		MoveLeftAction(CGuiInput::Get().MoveLeft());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_RIGHT))
	{
		MoveRightAction(CGuiInput::Get().MoveRight());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_UP))
	{
		MoveUpAction(CGuiInput::Get().MoveUp());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_DOWN))
	{
		MoveDownAction(CGuiInput::Get().MoveDown());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_START))
	{
		StartAction(CGuiInput::Get().StartPress());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_SELECT))
	{
		SelectAction(CGuiInput::Get().SelectPress());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_BACK))
	{
		BackAction(CGuiInput::Get().Back());
	}
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_DELETE))
	{
		DeleteAction(CGuiInput::Get().Delete());
	}
}
