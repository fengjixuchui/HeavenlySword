//------------------------------------------------------------------------------------------
//!
//!	\file KeyBindManager.cpp
//!
//------------------------------------------------------------------------------------------

//------------------------
// Includes
//------------------------
#include "game/command.h"
#include "game/keybinder.h"


//------------------------------------------------------------------------------------------
//!
//!	KeyContext - DestroyNonGlobals
//!
//! Class that encapsulates all keys in a context
//!	
//------------------------------------------------------------------------------------------

	// Destroy all the commands that were created in a non global context
void CommandManager::DestroyNonGlobals()
{
	ntstd::List<CommandBase*>::iterator obIt = m_obCommands.begin();
	while (obIt != m_obCommands.end())
	{
		// If the command is global, move to next
		if ((*obIt)->IsGlobal())
		{
			obIt++;
			continue;
		}

		// Make a copy of the current iterator to erase the item
		ntstd::List<CommandBase*>::iterator obDelete = obIt;
		//Move to next
		obIt++;

		// Destroy the command
		NT_DELETE( *obDelete );

		// Remove pointer from the list
		m_obCommands.erase(obDelete);
	}
}

