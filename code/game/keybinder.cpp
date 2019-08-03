//------------------------------------------------------------------------------------------
//!
//!	\file KeyBindManager.cpp
//!
//------------------------------------------------------------------------------------------

//------------------------
// Includes
//------------------------
#include "keybinder.h"

#if defined( PLATFORM_PC )
#	include "input/inputhardware_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "input/inputhardware_ps3.h"
#endif

// ---- globals ----
const char* KeyBindManager::m_gGlobalContextName = "global";
const char* KeyBindManager::m_gGlobalContextDecription = "Global keymappings";


//------------------------------------------------------------------------------------------
//!
//!	KeyContext - constructor
//!
//! Class that encapsulates all keys in a context
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::KeyContext::KeyContext(const char* pcName, const char* pcDescription) :
	m_obName(pcName),
	m_obDescription(pcDescription)
{
}

//------------------------------------------------------------------------------------------
//!
//!	KeyContext - destructor
//!
//! Class that encapsulates all keys in a context
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::KeyContext::~KeyContext()
{
	// Delete all commands in the context
	ntstd::Map<int, KeyMappingBase*>::iterator obIt = begin();
	while (obIt != end())
	{
		NT_DELETE( obIt->second );
		obIt++;
	}

	// Empty the map
	m_obRegisteredKeys.clear();
}

//------------------------------------------------------------------------------------------
//!
//!	KeyContext - Find
//!
//! Find a specific key within a key context
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::KeyMappingBase* KeyBindManager::KeyContext::Find(int iKey)
{
	// See if we can find the key in our map
	ntstd::Map<int, KeyMappingBase*>::iterator obIt = m_obRegisteredKeys.find(iKey);
	if (obIt != m_obRegisteredKeys.end())
	{
		// Found the item, so return the functor
		return obIt->second;
	}
	// Failed to find
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyContext - Find
//!
//! Add a key to a specific key context
//!	
//------------------------------------------------------------------------------------------

bool KeyBindManager::KeyContext::AddKey(KeyMappingBase* pobMapping, int iKey)
{
	// See if the key already exists in this context
	KeyMappingBase* pobPreviousMapping = Find(iKey);

	if (!pobPreviousMapping)
	{
		// Store the key in our map
		m_obRegisteredKeys.insert(ntstd::Map<int, KeyMappingBase*>::value_type(iKey, pobMapping));
		// Successful
		return true;
	}

	// Could not register key in context
	ntError_p( false, ("Could not register key \'%s\' in context \'%s\'\nKey already exists and is mapped as \'%s\' to command %s",
		CInputKeyboard::KeyToText((KEY_CODE)KeyContext::GetKeyFromCombinedKey(iKey)),
		GetName(), pobPreviousMapping->GetDescription(), 
		ntStr::GetString(pobPreviousMapping->GetCommandBase()->GetName())));

	// Could not add the key
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyContext - ProcessKeys
//!
//! Process all keys in the context
//!	
//------------------------------------------------------------------------------------------

bool KeyBindManager::KeyContext::ProcessKeys()
{
#ifdef	DEBUG_KEYBOARD

	// Get hold of the keyboard
	CInputKeyboard& obKeyb = CInputHardware::Get().GetKeyboard();
	int iModifier = 0;
	int iKey = 0;
	int iEvent;

	// Run through all of the keys that are registered
	ntstd::Map<int, KeyMappingBase*>::iterator obIt = begin();
	while (obIt != end())
	{
		// Retrieve the key and modifier
		UnrollCombinedKey(obIt->first, iKey, iModifier, iEvent);
		KeyMappingBase* pobKeyMap = obIt->second;


#define HANDLE_PAD(key, button)												\
		if ((KEY_CODE)iKey == key)											\
		{																	\
			if (CInputHardware::Get().GetPad(PAD_0).GetPressed() & button)		\
			{																\
				ntPrintf("Key Command: %s", pobKeyMap->GetDescription());	\
				pobKeyMap->Execute();										\
				ntPrintf("\n", pobKeyMap->GetDescription());				\
				return true;												\
			}																\
		}

		HANDLE_PAD (KEYC_JOYPAD_LEFT,		PAD_LEFT )
		HANDLE_PAD( KEYC_JOYPAD_RIGHT,		PAD_RIGHT )
		HANDLE_PAD( KEYC_JOYPAD_UP,			PAD_UP )
		HANDLE_PAD( KEYC_JOYPAD_DOWN,		PAD_DOWN )
		HANDLE_PAD( KEYC_JOYPAD_SELECT,		PAD_BACK_SELECT )
		HANDLE_PAD( KEYC_JOYPAD_START,		PAD_START )
		HANDLE_PAD( KEYC_JOYPAD_TRIANGLE,	PAD_FACE_1 )
		HANDLE_PAD( KEYC_JOYPAD_CROSS,		PAD_FACE_2 )
		HANDLE_PAD( KEYC_JOYPAD_SQUARE,		PAD_FACE_3 )
		HANDLE_PAD( KEYC_JOYPAD_CIRCLE,		PAD_FACE_4 )
		HANDLE_PAD( KEYC_JOYPAD_L1,			PAD_TOP_1 )
		HANDLE_PAD( KEYC_JOYPAD_L2,			PAD_TOP_2 )
		HANDLE_PAD( KEYC_JOYPAD_R1,			PAD_TOP_3 )
		HANDLE_PAD( KEYC_JOYPAD_R2,			PAD_TOP_4 )

		// Check to see if the key state is correct and if the modifier is correct
		if ( (obKeyb.GetKeyState((KEY_CODE)iKey) == pobKeyMap->GetEvent()) && (obKeyb.GetModifierState() == iModifier))
		{
			// Print the key binding description - no newline so that the command itself may print extra info
			ntPrintf("Key Command: %s", pobKeyMap->GetDescription());

			// Invoke the command
			pobKeyMap->Execute();

			// Print the endline for the command
			ntPrintf("\n", pobKeyMap->GetDescription());
			return true;
		}

		// Neeeext
		obIt++;
	}
#endif

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - Constructor
//!
//! Default constructor
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::KeyBindManager()
{
	// Create a global context. This is always the first context in the list
	m_pobGlobalContext = NT_NEW KeyContext(m_gGlobalContextName, m_gGlobalContextDecription );

	// Add it to the list
	m_obContexts.push_back(m_pobGlobalContext);

	// Clear cached find
	m_pobLastFoundContext = 0;

	// Set global state
	m_bInGlobalState = true;

	// Set the input context to GAME
	m_eCurrentContextIdentifier = INPUT_CONTEXT_GAME;

	// populate the context's with those defined in g_apcContextStrings. At a later stage these could be populated from text file.
	for (int i = 0; i < INPUT_CONTEXT_MAX; i++)
	{ 
		// Create a new context. Ultimately all contexts will be generated programatically. 
		CreateContext(g_apcContextTitleStrings[i], g_apcContextDescriptionStrings[i]);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - Destructor
//!
//! Default destructor
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::~KeyBindManager()
{
	// Delete all of the contexts
	while(!m_obContexts.empty())
	{
		// Delete away
		NT_DELETE( m_obContexts.back() );
		m_obContexts.pop_back();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - FindContextByName
//!
//! Find a context via it's text name
//!	
//------------------------------------------------------------------------------------------

KeyBindManager::KeyContext* KeyBindManager::FindContextByName(const char* pcName)
{
	// See if it was the last thing we searched for
	if (m_pobLastFoundContext)
	{
		if (0 == strcmp (m_pobLastFoundContext->GetName(), pcName))
		{
			return m_pobLastFoundContext;
		}
	}

	// Search through the list of KeyContexts
	for (ntstd::List<KeyContext*>::iterator obIt = m_obContexts.begin(); obIt != m_obContexts.end(); obIt++)
	{
		// Compare names
		if (0 == strcmp ((*obIt)->GetName(), pcName))
		{
			// Store the last found context
			m_pobLastFoundContext = *obIt;
			// Got it.. return the context
			return *obIt;
		}
	}
	// Failed
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - CreateContext
//!
//! Create a key context with a specific name
//!	
//------------------------------------------------------------------------------------------

bool KeyBindManager::CreateContext(const char* pcName, const char* pcDescription)
{
	// Check to see if the context already exists
	if (!FindContextByName(pcName))
	{
		// Create the new context
		KeyContext* pobContext = NT_NEW KeyContext(pcName, pcDescription);

		// Add the context to the end of our list
		m_obContexts.push_back(pobContext);

		// Successful
		return true;
	}
	// Failed
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - AddKey
//!
//! Add a new key to the context
//!	
//------------------------------------------------------------------------------------------
bool KeyBindManager::AddKey(const char* pcContext, KeyMappingBase* pobMapping, int iKey)
{
	// Add the mapping to a single context

	// See if we have the context
	KeyContext* pobContext = FindContextByName(pcContext);

	// Validate that we found the context
	ntError_p( pobContext, ("Invalid context specified - \'%s\'", pcContext ));

	// See if the key is in the global context (if it was not selected to be in the global context)
	if ((pobContext != m_pobGlobalContext) && (m_pobGlobalContext->Find(iKey)))
	{
		return false;
	}

	// If we have a context, add the key
	if (pobContext)
	{
		// Set the key in the context
		return pobContext->AddKey(pobMapping, iKey);
	}

	// Did not find the context
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - ProcessKeys
//!
//! Process all of the keys
//!	
//------------------------------------------------------------------------------------------
void KeyBindManager::ProcessKeys()
{
	// Check the global context
	m_pobGlobalContext->ProcessKeys();
	
	// Check the current context
	INPUT_CONTEXT eContext = CInputHardware::Get().GetContext();
	const char* pcContextName = g_apcContextTitleStrings[eContext];
	KeyContext* pobContext = FindContextByName(pcContextName);
	ntAssert_p(pobContext, ("Could not find context"));
	pobContext->ProcessKeys();
}


//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - DumpMappedKeys
//!
//! Generate a report of all registered keys
//!	
//------------------------------------------------------------------------------------------
COMMAND_RESULT KeyBindManager::DumpMappedKeys()
{
#ifdef	DEBUG_KEYBOARD
	ntPrintf("\nHEAVENLY SWORD KEYMAP\n");
	ntPrintf("Here follows a list of all keys used in Heavenly Sword\n");

	// Run through all the contexts and 
	for (ntstd::List<KeyContext*>::iterator obIt = m_obContexts.begin(); obIt != m_obContexts.end(); ++obIt)
	{
		ntPrintf("\n--== %s ==--\n", (*obIt)->GetName());
		
		// Run through all the registered keys in the context
		for (ntstd::Map<int, KeyMappingBase*>::iterator obKeyIt = (*obIt)->begin(); obKeyIt != (*obIt)->end(); ++obKeyIt)
		{
			int iFirst = (*obKeyIt).first;
			int iKey = KeyContext::GetKeyFromCombinedKey(iFirst);

			KeyMappingBase* pobD = (*obKeyIt).second;

			// Write out the registered key
			ntPrintf("%s: %s\n", CInputKeyboard::KeyToText((KEY_CODE)iKey), pobD->GetDescription());
			UNUSED( iKey );
			UNUSED( pobD );
		}
	}
#endif
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - DumpMappedKeys
//!
//! Generate a report of all registered keys
//!	
//------------------------------------------------------------------------------------------
COMMAND_RESULT KeyBindManager::DumpMappedKeysInConfluenceFormat()
{
#ifdef	DEBUG_KEYBOARD
	ntPrintf("H1.HEAVENLY SWORD KEYMAP\n");
	ntPrintf("Here follows a list of all keys used in Heavenly Sword\n");

	// Run through all the contexts and 
	for (ntstd::List<KeyContext*>::iterator obIt = m_obContexts.begin(); obIt != m_obContexts.end(); ++obIt)
	{
		ntPrintf("h2.%s\n", (*obIt)->GetName());
		ntPrintf("||Key||Description||");
		
		// Run through all the registered keys in the context
		for (ntstd::Map<int, KeyMappingBase*>::iterator obKeyIt = (*obIt)->begin(); obKeyIt != (*obIt)->end(); ++obKeyIt)
		{
			int iFirst = (*obKeyIt).first;
			int iKey = KeyContext::GetKeyFromCombinedKey(iFirst);

			KeyMappingBase* pobD = (*obKeyIt).second;

			// Write out the registered key
			ntPrintf("|%s|%s|\n", CInputKeyboard::KeyToText((KEY_CODE)iKey), pobD->GetDescription());
			UNUSED( iKey );
			UNUSED( pobD );
		}
	}
#endif
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - DumpAvailableKeys
//!
//! Generate a report of all the available keys
//!	
//------------------------------------------------------------------------------------------
COMMAND_RESULT KeyBindManager::DumpAvailableKeys()
{
#ifdef	DEBUG_KEYBOARD
	ntPrintf("\nHEAVENLY SWORD FREE KEYS\n");

	// Run through all the contexts and 
	for (ntstd::List<KeyContext*>::iterator obIt = m_obContexts.begin(); obIt != m_obContexts.end(); ++obIt)
	{
		// Print the context descriptor
		ntPrintf("\n--== %s ==--\n", (*obIt)->GetName());
		//ntPrintf("%s\n", (*obIt)->GetDescription());
		
		// Run through all keys
		for (int i = 0; i < 256; i++)
		{
			// See if the key is a valid key
			if (CInputKeyboard::KeyToText((KEY_CODE)i))
			{
				// See if the key is not mapped in this context
				if (!(*obIt)->IsKeyMapped((KEY_CODE)i, KEYM_NONE, KEYS_PRESSED))
				{
					// Print out the name of the key
					ntPrintf("%s, ", CInputKeyboard::KeyToText((KEY_CODE)i));
				}
			}
		}
	}
#endif
	return CR_SUCCESS;
}




//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - DestroyNonGlobals
//!
//! Remove any binding that is in a non global context
//!	
//------------------------------------------------------------------------------------------
void KeyBindManager::DestroyNonGlobals()
{
	// Search through the list of KeyContexts
	for (ntstd::List<KeyContext*>::iterator obIt = m_obContexts.begin(); obIt != m_obContexts.end(); obIt++)
	{
		(*obIt)->DestroyNonGlobals();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager - DestroyNonGlobals
//!
//! Remove any binding that is in a non global context
//!	
//------------------------------------------------------------------------------------------
void KeyBindManager::KeyContext::DestroyNonGlobals()
{
	ntstd::Map<int, KeyMappingBase*>::iterator obIt = m_obRegisteredKeys.begin();
	while (obIt != m_obRegisteredKeys.end())
	{
		// If the command is global, move to next
		if (obIt->second->IsGlobal())
		{
			obIt++;
			continue;
		}

		// Make a copy of the current iterator to erase the item
		ntstd::Map<int, KeyMappingBase*>::iterator obDelete = obIt;

		//Move to next
		obIt++;

		// Destroy the mapping
		NT_DELETE( obDelete->second );

		// Remove pointer from the list
		m_obRegisteredKeys.erase(obDelete);
	}
}

