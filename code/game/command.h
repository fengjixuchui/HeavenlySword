//------------------------------------------------------------------------------------------
//!
//!	\file Command.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COMMAND_H
#define _COMMAND_H

//------------------------
// Includes
//------------------------

#include "core/singleton.h"
#include "core/keystring.h"
#include "tbd/TypeSelect.h"
#include "game/commandresult.h"
#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
//!
//!	CommandBase
//!
//! Base command class
//!	
//------------------------------------------------------------------------------------------

class CommandBase
{
public:
	// Destructor
	virtual ~CommandBase() {};

//protected:
	// Constructor 
	CommandBase(const char* pcName, bool bGlobal) :
		 m_obName(pcName), m_bGlobal(bGlobal)
	{
	}

	// Accessor for name
	const CHashedString& GetName() const {return m_obName;}

	// Accessor for global status
	bool IsGlobal() {return m_bGlobal;}

protected:
	// Name of the command
	CHashedString m_obName;

	// Was this command created referring to a global object if not it may have to be
	// removed when the level is shut down.
	bool m_bGlobal;
};



//------------------------------------------------------------------------------------------
//!
//!	CommandBaseNoInput
//!
//! Command with no templated input parameter
//!	
//------------------------------------------------------------------------------------------
class CommandBaseNoInput : public CommandBase
{
public:
	// override function "Call"
	virtual COMMAND_RESULT Call() = 0;

	// Destructor
	virtual ~CommandBaseNoInput() {};

protected:
	// Constructor
	CommandBaseNoInput(const char* pcName, bool bGlobal) :
		 CommandBase(pcName, bGlobal)
	{
	}
};


//------------------------------------------------------------------------------------------
//!
//!	CommandBaseInput
//!
//! Command with templated input parameter
//!	
//------------------------------------------------------------------------------------------
template
<
	class		IClass
>
class CommandBaseInput : public CommandBase
{
public:
	// override function "Call"
	virtual COMMAND_RESULT Call(IClass obParams) = 0;

	// Destructor
	virtual ~CommandBaseInput() {};

protected:
	// Constructor
	CommandBaseInput(const char* pcName, bool bGlobal) :
		 CommandBase(pcName, bGlobal)
	{
	}
};


//------------------------------------------------------------------------------------------
//!
//!	Command
//!
//! Command with templated input parameter, and templated functor
//!	
//------------------------------------------------------------------------------------------
template
<
	class		TClass,
	bool		IsConstMember
>
class CommandNoInput : public CommandBaseNoInput
{
private:
	typedef COMMAND_RESULT ( TClass::*NonConstFuncType )();
	typedef COMMAND_RESULT ( TClass::*ConstFuncType )() const;

	typedef typename TypeSelect< IsConstMember, ConstFuncType, NonConstFuncType >::ResultType FuncType;

	// pointer to object
	TClass*		m_pt2Object;	

	// pointer to member function
	FuncType	m_fpt;			

	// Description of the command
	CHashedString m_obDescription;

public:
	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	CommandNoInput(const char* pcName, TClass *pt2Object, FuncType fpt, const char* pcDescription, bool bGlobal ) :
		CommandBaseNoInput(pcName, bGlobal),
		m_pt2Object(pt2Object),
		m_fpt(fpt),
		m_obDescription(pcDescription)
	{
	}

	// Destructor
	virtual ~CommandNoInput() {};

	// override function "Call"
	virtual COMMAND_RESULT Call()
	{
		return ( *m_pt2Object.*m_fpt )();						// execute member function
	}
};


//------------------------------------------------------------------------------------------
//!
//!	Command
//!
//! Command with templated input parameter, and templated functor
//!	
//------------------------------------------------------------------------------------------
template
<
	class		TClass,
	class		IClass,
	bool		IsConstMember
>
class Command : public CommandBaseInput<IClass>
{
private:
	typedef COMMAND_RESULT ( TClass::*NonConstFuncType )(IClass);
	typedef COMMAND_RESULT ( TClass::*ConstFuncType )(IClass) const;

	typedef typename TypeSelect< IsConstMember, ConstFuncType, NonConstFuncType >::ResultType FuncType;

	// pointer to object
	TClass*		m_pt2Object;	

	// pointer to member function
	FuncType	m_fpt;			

	// Description of the command
	CHashedString m_obDescription;

public:
	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	Command(const char* pcName, TClass *pt2Object, FuncType fpt, const char* pcDescription, bool bGlobal ) :
		CommandBaseInput<IClass>(pcName, bGlobal),
		m_pt2Object(pt2Object),
		m_fpt(fpt),
		m_obDescription(pcDescription)
	{
	}

	// Destructor
	virtual ~Command() {};

	// override function "Call"
	virtual COMMAND_RESULT Call(IClass obParams)
	{
		return ( *m_pt2Object.*m_fpt )(obParams);						// execute member function
	}
};


//------------------------------------------------------------------------------------------
//!
//!	CommandManager
//!
//! Singleton to handle all commands created
//!	
//------------------------------------------------------------------------------------------

// The class below is more or less a "dummy" for the reflected interface to hold.
// The CommandManager will populate the ODB interface of CommandManagerNet with
// commands that can be called through the net interface. It will also make a single
// instance of the CommandManagerNet available in the ODB at a global point.
class CommandManagerNet
{
public:
	// Global variable to store results in, for net commands.
	CHashedString m_sResultData;
};
START_STD_INTERFACE(CommandManagerNet)
	PUBLISH_VAR_AS(m_sResultData, ResultData);
END_STD_INTERFACE


class CommandManager : public Singleton<CommandManager>
{
public:
	// Constructor
	CommandManager()
	{
		// Start in global state
		m_bInGlobalState = true;
		m_pCommandManagerNet = ObjectDatabase::Get().Construct<CommandManagerNet>("CommandManagerNet", "CommandManager");
	}

	// Destructor
	~CommandManager()
	{
		// Delete all held commands
		while (!m_obCommands.empty())
		{
			CommandBase* pobLast = m_obCommands.back();
			m_obCommands.pop_back();
			NT_DELETE(pobLast);
		}
	}

	// Create a new command with a pointer parameter
	template <class	TClass, class IClass>
	CommandBaseInput<IClass*>* CreateCommand(const char* pcName, TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)(IClass*), const char* pcDescription)
	{
		// Create a new command 
		CommandBaseInput<IClass*>* pobComm = NT_NEW Command<TClass, IClass*, false>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		return pobComm;
	}

	// Create a new const command with a pointer parameter*
	template <class	TClass, class IClass>
	CommandBaseInput<IClass*>* CreateConstCommand(const char* pcName, const TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)(IClass*) const, const char* pcDescription)
	{
		// Create a new command 
		CommandBaseInput<IClass*>* pobComm = NT_NEW Command<TClass, IClass*, false>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		return pobComm;
	}

	// Create a new command
	template <class	TClass, class IClass>
	CommandBaseInput<const IClass&>* CreateCommand(const char* pcName, TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)(const IClass&), const char* pcDescription)
	{
		// Create a new command 
		CommandBaseInput<const IClass&>* pobComm = NT_NEW Command<TClass, const IClass&, false>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		// Register the command to be available for the net component
		StdDataInterface* pNetInterface = ObjectDatabase::Get().GetInterface("CommandManagerNet");
		pNetInterface->AddField( NT_NEW SingleItemDIF<IClass>( pcName, NT_NEW CommandAccessorField< IClass, TClass, false, true >( ft, pt2Object ), DataInterfaceField::MACRO_DEFAULT_MARKER  ));
		return pobComm;
	}

	// Create a new const command
	template <class	TClass, class IClass>
	CommandBaseInput<const IClass&>* CreateCommand(const char* pcName, TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)(const IClass&) const, const char* pcDescription)
	{
		// Create a new command 
		CommandBaseInput<const IClass&>* pobComm = NT_NEW Command<TClass, const IClass&, true>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		// Register the command to be available for the net component
		StdDataInterface* pNetInterface = ObjectDatabase::Get().GetInterface("CommandManagerNet");
		pNetInterface->AddField( NT_NEW SingleItemDIF<IClass>( pcName, NT_NEW CommandAccessorField< IClass, TClass, true, true >( ft, pt2Object ), DataInterfaceField::MACRO_DEFAULT_MARKER  ));
		return pobComm;
	}

	// Create a new command with no inputs
	template <class	TClass>
	CommandBaseNoInput* CreateCommandNoInput(const char* pcName, TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)(), const char* pcDescription)
	{
		// Create a new command 
		CommandBaseNoInput* pobComm =  NT_NEW CommandNoInput<TClass, false>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		// Register the command to be available for the net component, with a dummy bool argument (that will be discarded automatically)
		StdDataInterface* pNetInterface = ObjectDatabase::Get().GetInterface("CommandManagerNet");
		pNetInterface->AddField( NT_NEW SingleItemDIF<bool>( pcName, NT_NEW CommandAccessorField< bool, TClass, false, false >( ft, pt2Object ), DataInterfaceField::MACRO_DEFAULT_MARKER  ));
		return pobComm;
	}

	// Create a new const command with no inputs
	template <class	TClass>
	CommandBaseNoInput* CreateCommandNoInput(const char* pcName, TClass* pt2Object, COMMAND_RESULT ( TClass::*ft)() const, const char* pcDescription)
	{
		// Create a new command 
		CommandBaseNoInput* pobComm =  NT_NEW CommandNoInput<TClass, true>(pcName, pt2Object, ft, pcDescription, m_bInGlobalState);
		m_obCommands.push_back(pobComm);
		// Register the command to be available for the net component, with a dummy bool argument (that will be discarded automatically)
		StdDataInterface* pNetInterface = ObjectDatabase::Get().GetInterface("CommandManagerNet");
		pNetInterface->AddField( NT_NEW SingleItemDIF<bool>( pcName, NT_NEW CommandAccessorField< bool, TClass, true, false >( ft, pt2Object ), DataInterfaceField::MACRO_DEFAULT_MARKER  ));
		return pobComm;
	}

	// Find a command by name
	CommandBase* Find(const char* pcFind)
	{
		CHashedString StringToFind(pcFind);

		// Iterate through all commands
		ntstd::List<CommandBase*>::iterator obIt = m_obCommands.begin();
		while (obIt != m_obCommands.end())
		{
			// Check the address of the command
			if ( (*obIt)->GetName() == pcFind)
			{
				// Found it
				return *obIt;
			}
			obIt++;
		}
		// Failed to find
		return 0;
	}

	// Destroy all the commands that were created in a non global context
	void DestroyNonGlobals();

	// Set to global state ( all commands create will be global )
	void EnableGlobalState() {m_bInGlobalState = true;}

	// Set to non global state ( all commands will be non global )
	void DisableGlobalState() {m_bInGlobalState = false;}

	// Store the result of some command, for later retrieval.
	void StoreCommandResult(const CHashedString& data)
	{
		m_pCommandManagerNet->m_sResultData = data;
	}

private:

	// List of all commands that have been created
	ntstd::List<CommandBase*> m_obCommands;

	// Bool to indicate that the manager is in global state
	bool m_bInGlobalState;

	// Keep a pointer to the exposed net command manager, so we can easily store data in it.
	CommandManagerNet *m_pCommandManagerNet;
};


#endif //_COMMAND_H

