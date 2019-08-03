//------------------------------------------------------------------------------------------
//!
//!	\file KeyBindManager.h
//!
//------------------------------------------------------------------------------------------

// NOTE: This class is currently implemented as a halfway house between the current context 
// system that uses hard defined contexts and a system that allows the contexts to be extended. 


#ifndef _KEY_BINDER_H
#define _KEY_BINDER_H

//------------------------
// Includes
//------------------------

#include <map>
#include <list>
#include "core/singleton.h"
#include "input/inputcontexts.h"
#include "input/inputhardware.h"
#include "game/command.h"
#include "game/commandresult.h"
#include "tbd/functor.h"

// Define to select which context is the default context.
#define DEFAULT_CONTEXT INPUT_CONTEXT_GAME

//------------------------------------------------------------------------------------------
//!
//!	KeyBindManager
//!
//! Manager class to handle the binding of keys to classes and functions 
//!	
//------------------------------------------------------------------------------------------

class KeyBindManager : public Singleton<KeyBindManager>
{
public:



//private:

	//------------------------------------------------------------------------------------------
	//!
	//!	KeyMapping
	//!
	//! 
	//!	
	//------------------------------------------------------------------------------------------
	class KeyMappingBase
	{
	public:
		// Destructor
		virtual ~KeyMappingBase(){}

		// Execute the mapping
		virtual int Execute() = 0;

		// Description accessor
		const char* GetDescription() const { return m_obDescription.c_str();}

		// Create a clone of this keymapping
		virtual KeyMappingBase* Clone() = 0;

		// Event accessor
		KEY_STATE GetEvent() {return m_eEvent;}

		// Accessor for command base
		virtual CommandBase* GetCommandBase() = 0;

		// Was this created in global scope
		bool IsGlobal() {return m_bGlobal;}

	protected:

		// Constructor
		KeyMappingBase(const char* pcDescription, KEY_STATE eEvent, bool bGlobal) : 
			m_obDescription(pcDescription),
			m_eEvent(eEvent),
			m_bGlobal(bGlobal)
		{
		}

		// Description of mapping
		ntstd::String m_obDescription;
		
		// The event that triggers it
		KEY_STATE m_eEvent;

		// Is the mapping in global state
		bool m_bGlobal;
	};


	class KeyMappingNoInput : public KeyMappingBase
	{
	public:
		// Constructor
		KeyMappingNoInput(CommandBaseNoInput* pobCommand, const char* pcDescription, KEY_STATE eEvent, bool bGlobal) :
			KeyMappingBase(pcDescription, eEvent, bGlobal),
			m_pobCommand(pobCommand)
		{
		}

		// Copy Constructor
		KeyMappingNoInput(const KeyMappingNoInput& obOther) :
			KeyMappingBase(obOther),
			m_pobCommand(obOther.m_pobCommand)
		{
		}

		// Destructor
		virtual ~KeyMappingNoInput()
		{
		}

		// Execute the mapping
		virtual int Execute()
		{
			return m_pobCommand->Call();
		}

		// Create a clone of this keymapping
		virtual KeyMappingBase* Clone()
		{
			// Create new mapping that's a copy of this one
			KeyMappingBase* pobNew = NT_NEW KeyMappingNoInput(*this);
			// Return new mapping
			return pobNew;
		}

		// Accessor for command base
		virtual CommandBase* GetCommandBase() { return m_pobCommand;}

	private:
		// The command
		CommandBaseNoInput* m_pobCommand;
	};


	template <class IClass>
	class KeyMapping : public KeyMappingBase
	{
	public:
		// Constructor
		KeyMapping(CommandBaseInput<const IClass&>* pobCommand, const char* pcDescription, IClass obParams, KEY_STATE eEvent, bool m_bGlobal) :
			KeyMappingBase(pcDescription, eEvent, m_bGlobal),
			m_pobCommand(pobCommand),
			m_obParams(obParams)
		{
		}

		// Copy Constructor
		KeyMapping(const KeyMapping& obOther) :
			KeyMappingBase(obOther),
			m_pobCommand(obOther.m_pobCommand),
			m_obParams(obOther.m_obParams)
		{
		}

		// Destructor
		virtual ~KeyMapping()
		{
		}

		// Execute the mapping
		virtual int Execute()
		{
			return m_pobCommand->Call(m_obParams);
		}

		// Create a clone of this keymapping
		virtual KeyMappingBase* Clone()
		{
			// Create new mapping that's a copy of this one
			KeyMappingBase* pobNew = NT_NEW KeyMapping(*this);
			// Return new mapping
			return pobNew;
		}

		// Accessor for command base
		virtual CommandBase* GetCommandBase() { return m_pobCommand;}

	private:
		// The command
		CommandBaseInput<const IClass&>* m_pobCommand;

		// Storage for the parameters
		IClass m_obParams;
	};

	// specialization of the above specifically to support passing in non-const pointers!
	template <class IClass>
	class KeyMapping<IClass*> : public KeyMappingBase
	{
	public:
		// Constructor
		KeyMapping(CommandBaseInput<IClass*>* pobCommand, const char* pcDescription, IClass* obParams, KEY_STATE eEvent, bool m_bGlobal) :
			KeyMappingBase(pcDescription, eEvent, m_bGlobal),
			m_pobCommand(pobCommand),
			m_obParams(obParams)
		{
		}

		// Copy Constructor
		KeyMapping(const KeyMapping& obOther) :
			KeyMappingBase(obOther),
			m_pobCommand(obOther.m_pobCommand),
			m_obParams(obOther.m_obParams)
		{
		}

		// Destructor
		virtual ~KeyMapping()
		{
		}

		// Execute the mapping
		virtual int Execute()
		{
			return m_pobCommand->Call(m_obParams);
		}

		// Create a clone of this keymapping
		virtual KeyMappingBase* Clone()
		{
			// Create new mapping that's a copy of this one
			KeyMappingBase* pobNew = NT_NEW KeyMapping(*this);
			// Return new mapping
			return pobNew;
		}

		// Accessor for command base
		virtual CommandBase* GetCommandBase() { return m_pobCommand;}

	private:
		// The command
		CommandBaseInput<IClass*>* m_pobCommand;

		// Storage for the parameters
		IClass* m_obParams;
	};




	//------------------------------------------------------------------------------------------
	//!
	//!	KeyContext
	//!
	//! Class that encapsulates all keys in a context. The context is a usage state for a set of
	//! keys. E.g. AI, debug
	//!	
	//------------------------------------------------------------------------------------------
	class KeyContext
	{
	public:

		// Constructor
		KeyContext(const char* pcName, const char* pcDescription);

		// Destructor - default
		~KeyContext();

		// Register a key with the context
		bool AddKey(KeyMappingBase* pobMapping, int iKey);

		// Get the name of this context
		const char* GetName() const {return m_obName.c_str();}

		// Get the description of ths context
		const char* GetDescription() const {return m_obDescription.c_str();}

		// Find a Key in the context
		KeyMappingBase* Find(int iKey);

		// Process all keys in the context
		bool ProcessKeys();

		// See if a key is mapped
		bool IsKeyMapped(int iKey, int iModifier, int iEvent)
		{
			KeyMappingBase* pobMapping = Find(GenerateCombinedKey(iKey, iModifier, iEvent));
			return (pobMapping != 0);
		}

		// Combine the key and modifier into one single value
		static int GenerateCombinedKey(int iKey, int iModifier, int iEvent)
		{
			ntAssert((iKey & 0xff) < 256)
			ntAssert((iModifier & 0x0f) < 16 )
			ntAssert((iEvent & 0x0f) < 16)

			// Combine the key and modifier into one key. Doing it this way should mean that things get stored in the order K, Ctrl-K, etc..
			return iKey | (iModifier << 8) | (iEvent << 12) ;
		}

		// Do the reverse of GenerateCombinedKey(...)
		static void UnrollCombinedKey(int iCombined, int& iKey, int& iModifier, int& iEvent)
		{
			iKey = iCombined & 0xff;
			iModifier = (iCombined >> 8) & 0x0f;
			iEvent = (iCombined >> 12) & 0x0f;
		}

		// Get only the key from a combined keypress
		static int GetKeyFromCombinedKey(int iCombined)
		{
			return iCombined & 0xff;
		}

		// Get starting iterator
		ntstd::Map<int, KeyMappingBase*>::iterator begin() {return m_obRegisteredKeys.begin();}

		// Get ending iterator
		ntstd::Map<int, KeyMappingBase*>::iterator end() {return m_obRegisteredKeys.end();}

		// Destroy all the commands that were created in a non global context
		void DestroyNonGlobals();

	protected:

		// Name of the context
		ntstd::String m_obName;

		// Description of the context
		ntstd::String m_obDescription;

		// Map the key code to the functor
		ntstd::Map<int, KeyMappingBase*> m_obRegisteredKeys;
	};

public:

	// Constructor - default
	KeyBindManager();

	// Destructor - default
	~KeyBindManager();

	// Create a new KeyContext
	bool CreateContext(const char* pcName, const char* pcDescription);

	// Register a key to be handled by the binder
	template<class IClass>
	void RegisterKey(const char* pcContext, CommandBaseInput<const IClass&>* pobCommand, const char* pcDescription, IClass obParams, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		KeyMappingBase* pobMapping = NT_NEW KeyMapping<IClass>(pobCommand, pcDescription, obParams, eEvent, m_bInGlobalState);
		int iKey = KeyContext::GenerateCombinedKey(iKeyCode, iKeyModifier, eEvent);
		AddKey(pcContext, pobMapping, iKey);
	}
	
	// Register a key to be handled by the binder, but with a pointer parameter
	template<class IClass>
	void RegisterKey(const char* pcContext, CommandBaseInput<IClass*>* pobCommand, const char* pcDescription, IClass* obParams, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		KeyMappingBase* pobMapping = NT_NEW KeyMapping<IClass*>(pobCommand, pcDescription, obParams, eEvent, m_bInGlobalState);
		int iKey = KeyContext::GenerateCombinedKey(iKeyCode, iKeyModifier, eEvent);
		AddKey(pcContext, pobMapping, iKey);
	}

	// Register a key with no inputs to be handled by the binder
	void RegisterKeyNoInput(const char* pcContext, CommandBaseNoInput* pobCommand, const char* pcDescription, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		KeyMappingBase* pobMapping = NT_NEW KeyMappingNoInput(pobCommand, pcDescription, eEvent, m_bInGlobalState);
		int iKey = KeyContext::GenerateCombinedKey(iKeyCode, iKeyModifier, eEvent);
		AddKey(pcContext, pobMapping, iKey);
	}

	// Register a key to be handled by the binder using a name for the command
	template<class IClass>
	void RegisterKey(const char* pcContext, const char* pcCommandName, const char* pcDescription, IClass obParams, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		CommandBaseInput<IClass>* pobCommand = (CommandBaseInput<IClass>*)CommandManager::Get().Find(pcCommandName);
		ntError_p(pobCommand, ("Could not find command - %s", pcCommandName));
		RegisterKey(pcContext, pobCommand, pcDescription, obParams, eEvent, iKeyCode, iKeyModifier);
	}
	
	// Register a key to be handled by the binder using a name for the command
	template<class CommandInputType, class IClass>
	void RegisterKey(const char* pcContext, const char* pcCommandName, const char* pcDescription, IClass* obParams, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		CommandInputType* pobCommand = (CommandInputType*)CommandManager::Get().Find(pcCommandName);
		ntError_p(pobCommand, ("Could not find command - %s", pcCommandName));
		RegisterKey(pcContext, pobCommand, pcDescription, obParams, eEvent, iKeyCode, iKeyModifier);
	}

	// Register a key with no inputs to be handled by the binder using a name for the command
	void RegisterKeyNoInput(const char* pcContext, const char* pcCommandName, const char* pcDescription, KEY_STATE eEvent, int iKeyCode, int iKeyModifier = 0)
	{
		CommandBaseNoInput* pobCommand = (CommandBaseNoInput*)CommandManager::Get().Find(pcCommandName);
		ntError_p(pobCommand, ("Could not find command - %s", pcCommandName));
		RegisterKeyNoInput(pcContext, pobCommand, pcDescription, eEvent, iKeyCode, iKeyModifier);
	}

	// Process all of the registered keys
	void ProcessKeys();

	// Get a string version of the current context
	const char* GetContextAsString() const
	{
		return g_apcContextTitleStrings[m_eCurrentContextIdentifier];
	}

	// Set the current context
	void SetContext( INPUT_CONTEXT eContext )	{ m_eCurrentContextIdentifier = eContext; };

	// Get hold of the current context identifier. This is temporary until all bind functions are in place.
	INPUT_CONTEXT GetContext() const { return m_eCurrentContextIdentifier; }

	// Generate a report of all the registered keys
	COMMAND_RESULT DumpMappedKeys();

	// Generate a report of all the registered keys in confluence markup format
	COMMAND_RESULT DumpMappedKeysInConfluenceFormat();

	// Generate a report of all the registered keys
	COMMAND_RESULT DumpAvailableKeys();

	// Destroy all the commands that were created in a non global context
	void DestroyNonGlobals();

	// Set to global state ( all commands create will be global )
	void EnableGlobalState() {m_bInGlobalState = true;}

	// Set to non global state ( all commands will be non global )
	void DisableGlobalState() {m_bInGlobalState = false;}

protected:

	// The current context identifier
	INPUT_CONTEXT m_eCurrentContextIdentifier;

	// Find the context by name
	KeyContext* FindContextByName(const char* pcName);

	// Add a new key to the context
	bool AddKey(const char* pcContext, KeyMappingBase* pobMapping, int iKey);

	// A list of the contexts in the mapping
	ntstd::List<KeyContext*> m_obContexts;

	// A cache of the last found context
	KeyContext* m_pobLastFoundContext;

	// Global context name
	static const char* m_gGlobalContextName;

	// Global context description
	static const char* m_gGlobalContextDecription;

	// Pointer to global context
	KeyContext* m_pobGlobalContext;
	
	// Bool to indicate that the manager is in global state
	bool m_bInGlobalState;
};

#endif //_KEY_BINDER_H
