//------------------------------------------------------------------------------------------
//!
//!	\file	sysmsgdialog.h
//!
//!	Definition of the sysutils msg dialog wrapper.		
//!
//------------------------------------------------------------------------------------------

#ifndef _SYSMSGDIALOG_H_
#define _SYSMSGDIALOG_H_

#ifdef PLATFORM_PS3

#include <sysutil/sysutil_msgdialog.h>

// Callback type for just the button pressed.
typedef void (*ButtonPressedCallback)( const CellMsgDialogButtonType Button );

//------------------------------------------------------------------------------------------
//!
//!	Class CSysMsgDialog
//!
//!	Sysutils msg dialog wrapper.		
//!
//------------------------------------------------------------------------------------------
class CSysMsgDialog
{
public:

	friend void SysMsgDialogCallback( int button_type, void *userdata );

	// Shorter versions of the button types.
	static CellMsgDialogButtonType YES;
	static CellMsgDialogButtonType NO;
	static CellMsgDialogButtonType ESCAPE;
	static CellMsgDialogButtonType INVALID;

	// Dialog configurations.
	typedef enum eDIALOG_CONFIG
	{
		YESNO_NORMAL_YES =	(
								CELL_MSGDIALOG_DIALOG_TYPE_NORMAL |
								CELL_MSGDIALOG_BUTTON_TYPE_YESNO |
								CELL_MSGDIALOG_DEFAULT_CURSOR_YES
							),


		YESNO_NORMAL_NO =	(
								CELL_MSGDIALOG_DIALOG_TYPE_NORMAL |
								CELL_MSGDIALOG_BUTTON_TYPE_YESNO |
								CELL_MSGDIALOG_DEFAULT_CURSOR_NO
							)
	} DialogConfig;

	static void CreateDialog( const DialogConfig Config, const char* pcStringID, ButtonPressedCallback Callback );
	
	// Check for user interaction.
	static void Update( void );

private:

	// Set flag to enable sysutils callback polling.
	//static void SetCheckCallback( const bool bCheck );
	
	// Callback management.
	static ButtonPressedCallback GetButtonPressedCallback( void );
	static void ClearButtonPressedCallback( void );
	static ButtonPressedCallback m_ButtonPressedCallback;
	//static bool m_bCheckCallback;
};

#endif // PLATFORM_PS3

#endif // _SYSMSGDIALOG_H_
