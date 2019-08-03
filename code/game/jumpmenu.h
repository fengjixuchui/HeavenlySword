//------------------------------------------------------------------------------------------
//!
//!	\file jumpmenu.h
//!
//------------------------------------------------------------------------------------------
#if !defined( HS_JUMPMENU_H )
#define HS_JUMPMENU_H

#include "game/commandresult.h"

// forward decl
struct DebugHudItem;

//------------------------------------------------------------------------------------------
//!
//! a system to allow us to jump around a level in a fairly user friendly manner
//!
//------------------------------------------------------------------------------------------
class JumpMenu : public Singleton<JumpMenu>
{
public:

	JumpMenu();
	~JumpMenu();

	//! loads the jump points lua file
	void Load( const char* pLevelName );

	// toggle wether the menu should be loaded
	COMMAND_RESULT ToggleMenu();

private:
	static const int STRING_BUFFER_SIZE = 2048;		// 2K

	DebugHudItem*	m_pDebugHudArray;

	char m_StringBuffer[STRING_BUFFER_SIZE];	//!< we need some temp space for strings
	int m_iUsedString;							//!< how much of our string buffer have we used

	bool m_bShowMenu;							//!< Whether the menu is visible

	// allocates and copy the string onto the internal string buffer
	const char* AllocString( const char* pToCopy );
};

#endif // end HS_JUMPMENU_H

