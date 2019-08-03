//------------------------------------------------------------------------------------------
//!
//!	\file	sysmsgdialog.cpp
//!
//!	Implementation of the sysutils msg dialog wrapper.		
//!
//------------------------------------------------------------------------------------------

#ifdef PLATFORM_PS3

#include "game/sysmsgdialog.h"
#include "gui/guitext.h"
#include "gui/guiinput.h"

//------------------------------------------------------------------------------------------
//!
//! Static initialisation.
//!
//------------------------------------------------------------------------------------------
//bool CSysMsgDialog::m_bCheckCallback = false;
ButtonPressedCallback CSysMsgDialog::m_ButtonPressedCallback = NULL;

// Shorter, easy to remember button names.
CellMsgDialogButtonType CSysMsgDialog::YES = CELL_MSGDIALOG_BUTTON_YES;
CellMsgDialogButtonType CSysMsgDialog::NO = CELL_MSGDIALOG_BUTTON_NO;
CellMsgDialogButtonType CSysMsgDialog::ESCAPE = CELL_MSGDIALOG_BUTTON_ESCAPE;
CellMsgDialogButtonType CSysMsgDialog::INVALID = CELL_MSGDIALOG_BUTTON_INVALID;

//------------------------------------------------------------------------------------------
//!
//! Callback executed when the user interacts with the dialog.
//!
//------------------------------------------------------------------------------------------
void SysMsgDialogCallback( int button_type, void *userdata )
{
	// Call the user registered callback with button pressed.
	if	( CSysMsgDialog::GetButtonPressedCallback() )
	{
		CSysMsgDialog::GetButtonPressedCallback()( ( CellMsgDialogButtonType ) button_type );
	}

	// Callback has now been called to disable callback checking and clear callback pointer.
	//CSysMsgDialog::SetCheckCallback( false );
	CSysMsgDialog::ClearButtonPressedCallback();

	// Allow input for the gui.
	CGuiInput::Get().BlockInput( false );
}

//------------------------------------------------------------------------------------------
//!
//! CSysMsgDialog::Create	
//!
//------------------------------------------------------------------------------------------
void CSysMsgDialog::CreateDialog( const DialogConfig Config, const char* pcStringID, ButtonPressedCallback Callback )
{
	// Get the string.
	const WCHAR_T* pwcResourceString = CStringManager::Get().GetResourceString( pcStringID );

	// Convert to multi byte for cellMsgDialogOpen.
	char acMessage[ CELL_MSGDIALOG_STRING_SIZE ];
	wcstombs( acMessage, pwcResourceString, sizeof( acMessage )-1 );

	int iRetVal = cellMsgDialogOpen( Config, acMessage, SysMsgDialogCallback, NULL, NULL );

	// iRetVal will be 0 if cellMsgDialogOpen terminates normally.
	if	( 0 == iRetVal )
	{
		m_ButtonPressedCallback = Callback;

		// Stop input getting back to the gui.
		CGuiInput::Get().BlockInput( true );
	}
	else
	{
		ntPrintf( "CSysMsgDialog::CreateDialog( %s ) had errors.\n", pcStringID );
	}
}

//------------------------------------------------------------------------------------------
//!
//! CSysMsgDialog::Update	
//!
//!	Wait for the user to respond to the dialog.
//!
//! No longer needed, cellSysutilCheckCallback is called every update in ShellGlobal::Update.
//------------------------------------------------------------------------------------------
/*
void CSysMsgDialog::Update( void )
{
	if	( m_bCheckCallback )
	{
		cellSysutilCheckCallback();	
	}
}
*/

//------------------------------------------------------------------------------------------
//!
//! CSysMsgDialog::SetCheckCallback
//!
//!	Returns which button was pressed.
//!
//! No longer needed, cellSysutilCheckCallback is called every update in ShellGlobal::Update.
//------------------------------------------------------------------------------------------
/*
void CSysMsgDialog::SetCheckCallback( const bool bCheck )
{
	m_bCheckCallback = bCheck;
}
*/

//------------------------------------------------------------------------------------------
//!
//! CSysMsgDialog::GetButtonPressedCallback
//!
//!	Returns the registered button pressed callback.
//!
//------------------------------------------------------------------------------------------
ButtonPressedCallback CSysMsgDialog::GetButtonPressedCallback( void )
{
	return m_ButtonPressedCallback;
}

//------------------------------------------------------------------------------------------
//!
//! CSysMsgDialog::ClearButtonPressedCallback
//!
//!	NULL button pressed callback.
//!
//------------------------------------------------------------------------------------------
void CSysMsgDialog::ClearButtonPressedCallback( void )
{
	m_ButtonPressedCallback = NULL;
}

#endif // PLATFORM_PS3
