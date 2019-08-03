/***************************************************************************************************
*
*	DESCRIPTION		Test environment for LuaPlus
*
*	NOTES
*
***************************************************************************************************/

#include "timer.h"

#include "luatest.h"
#include "luamem.h"
#include "luahelp.h"

#include "luaglobal.h"
#include "luaplus/luaplus.h"
using namespace LuaPlus;


/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::Constructor
*
*	DESCRIPTION		Create a LuaState, overide the basic functions, memory tag and test functions.
*
***************************************************************************************************/
CLuaTestRig::CLuaTestRig( void ) :
	m_pobState( NULL )
{
#ifdef LUA_DEBUG_MEMORY
	CLuaMemoryMan::Get().StopTagging();
#endif

	m_pobState = LuaState::Create( false );
	ntAssert(GetState());

	CLuaHelper::LoadLuaLibs( GetState() );
	
#ifdef LUA_DEBUG_MEMORY
	CLuaMemoryMan::Get().StartTagging();
#endif

//	CLuaRDebuggerMan::DebugScript( GetState(), TEST_SCRIPT_PATH"ScriptVectorDump.lua" );

//	GetState()->DoString( "print('Hello from Lua')");

//	TestDump();
//	TestDumpGlobals();
//	TestGarbageCollect();
//	TestNewCall();
//	TestClone();
//	TestPointer();
//	TestTableIterator();

//	TestScriptFormat();
//	TestScriptCallback();
//	TestScriptSave();
//	TestScriptArray();
//	TestThread();

#ifdef LUA_DEBUG_MEMORY
	CMemoryDebug::Dump( CMemoryDebug::ALPHABETICAL );
	CLuaMemoryMan::Get().StopTagging();
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::~Destructor
*
*	DESCRIPTION		cleanup state
*
***************************************************************************************************/
CLuaTestRig::~CLuaTestRig( void )
{
	LuaState::Destroy( m_pobState );
}





/***************************************************************************************************
*
*	FUNCTION		LS_PrintNumber
*
*	DESCRIPTION		LuaScript registered print function
*
***************************************************************************************************/
static int LS_PrintNumber( LuaState* pobState, LuaStackObject* pobArgs )
{
#ifdef LUA_DEBUG_FUNC_REG
	if (pobArgs[1].IsNumber())
		printf("LS_PrintNumber(%f)\n", pobArgs[1].GetNumber());
	else
		printf("LS_PrintNumber(INVALID_ARG)\n");
#else

	// Verify it is a number and print it.
	if (pobArgs[1].IsNumber())
		printf("%f\n", pobArgs[1].GetNumber());

#endif

	// No return values.
	UNUSED(pobState);

	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		LS_Add
*
*	DESCRIPTION		LuaScript registered add function
*
***************************************************************************************************/
static int LS_Add( LuaState* pobState, LuaStackObject* pobArgs )
{
#ifdef LUA_DEBUG_FUNC_REG
	if (pobArgs[1].IsNumber() && pobArgs[2].IsNumber())
		printf("LS_Add(%f,%f)\n",pobArgs[1].GetNumber(), pobArgs[2].GetNumber());
	else if (pobArgs[2].IsNumber())
		printf("LS_Add(INVALID_ARG,%f)\n",pobArgs[2].GetNumber());
	else if (pobArgs[1].IsNumber())
		printf("LS_Add(%f,INVALID_ARG)\n",pobArgs[1].GetNumber());
	else
		printf("LS_Add(INVALID_ARG,INVALID_ARG)\n");
#endif

	if (pobArgs[1].IsNumber()  &&  pobArgs[2].IsNumber())
	{
		pobState->PushNumber( pobArgs[1].GetNumber() + pobArgs[2].GetNumber() );
	}
	else
	{
		pobState->PushNumber( 0.0 );
	}

	// One return value.
	return 1;
}

/***************************************************************************************************
*
*	FUNCTION		LS_LightUserDataCall
*
*	DESCRIPTION		LuaScript registered pointer handler
*
***************************************************************************************************/
static int LS_LightUserDataCall( LuaState* pobState, LuaStackObject* pobArgs )
{
	bool bIsLightUserData = pobArgs[ 1 ].IsLightUserData();
	const void* pvPointer = pobArgs[ 1 ].GetUserData();

	if (bIsLightUserData)
		printf("In LS_LightUserDataCall (ptr:0x%x)", pvPointer );
	else
		printf("In LS_LightUserDataCall with invalid pointer");

	UNUSED( pobState );

	// No return values
	return 0;
}





/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestThread
*
*	DESCRIPTION		Test creating our own thread (these are currently unavailable)
*
***************************************************************************************************/
void	CLuaTestRig::TestThread( void )
{
	CLuaHelper::LoadLuaLibs( GetState(), CLuaHelper::LUA_LIB_BASE_F );

	// this file barfs because of a bad closure read on the third resume.
//	const char* pcName =  TEST_SCRIPT_PATH"testthread.lua";
//	printf("Running File %s\n", pcName);
//	GetState()->DoFile( pcName );

	// this file barfs because luaD_callEx uses the stateUserData state field which is not
	// initialised correctly for threads.
	const char* pcName =  TEST_SCRIPT_PATH"testthread2.lua";
	printf("Running File %s\n", pcName);
	GetState()->DoFile( pcName );
	
	// this block barfs because of an unititialised alloc name field in the new state
	// (only shows up with memory debugging turned on)
//	LuaStateOwner obNewState = LuaState::CreateThread( GetState() );
//	obNewState->DoString( "mike = 'hello'; print(mike)" );

	// this block barfs because of the unitinitalised alloc name
	// (only shows up with memory debugging turned on)
//	LuaStateOwner obNewState = LuaState::CreateThread( GetState() );
//	CLuaHelper::LoadLuaLibs( obNewState, CLuaHelper::LUA_LIB_TABLE_F );
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestDump
*
*	DESCRIPTION		Test dumping a table to a lua file
*
***************************************************************************************************/
void	CLuaTestRig::TestDump( void )
{	
	// scoping allows collect garbage to properly clean up after the LuaObject
	{
		LuaObject	obComplexObj = GetState()->GetGlobals().CreateTable("Complex");
		obComplexObj.SetString( "d:\\Test\\Stuff\\\xff\xfe", "An entry" );
		GetState()->DumpObject( TEST_SCRIPT_PATH"TestDump.lua", "Complex", obComplexObj );
	}

	GetState()->GetGlobals().SetNil("Complex");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestDumpGlobals
*
*	DESCRIPTION		Test dumping all the global objects to a file
*
***************************************************************************************************/
void	CLuaTestRig::TestDumpGlobals( void )
{
	GetState()->DoString("GlobalTable = { 1, 2, 3, 4 }");
	GetState()->DoString("GlobalValue = 5");
	GetState()->DumpGlobals(TEST_SCRIPT_PATH"TestDump.lua");
	GetState()->GetGlobals().SetNil("GlobalTable");
	GetState()->GetGlobals().SetNil("GlobalValue");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestGarbageCollect
*
*	DESCRIPTION		Test Garbage Collection
*
***************************************************************************************************/
void	CLuaTestRig::TestGarbageCollect( void )
{
	GetState()->DoString("a = 5");
	GetState()->GetGlobals().SetNil("a");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestNewCall
*
*	DESCRIPTION		Test calling a lua function explicitly in two different ways.
*
***************************************************************************************************/
void	CLuaTestRig::TestNewCall( void )
{
	// scoping allows collect garbage to properly clean up after the LuaObjects and LuaFunction
	{
		GetState()->DoString("function Add(x, y) return x + y end");
		
		LuaObject obAddFunc = GetState()->GetGlobal( "Add" );

		LuaObject obResult = obAddFunc() << 2 << 7 << LuaRun();
		printf( "Lua Add 2 + 7 = %f\n", obResult.GetNumber() );
		
		LuaFunction<float> Add = GetState()->GetGlobal("Add");
		printf("Lua Add 3 + 9 %f\n", Add(3, 9) );
	}

	GetState()->GetGlobals().SetNil("Add");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestClone
*
*	DESCRIPTION		Compare cloning to raw table creation
*
***************************************************************************************************/
void	CLuaTestRig::TestClone( void )
{
	// scoping allows collect garbage to properly clean up after the LuaObjects
	{
		CMicroTimer obTimer;

		// Time a table create
		obTimer.Start();
		GetState()->DoString("Table = { 0, 1, 2, 'Hello', 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
		obTimer.Stop();

		printf( "Table Create: %fmus (%f%%)", obTimer.GetMicroSecs(), obTimer.GetFramePercent() );

		// Get the table object
		LuaObject obTableObj = GetState()->GetGlobal("Table");
		

		// Time a table clone
		obTimer.Start();
		LuaObject obCloneTableObj = obTableObj.Clone();
		obTimer.Stop();

		printf( "Table Clone: %fmus (%f%%)", obTimer.GetMicroSecs(), obTimer.GetFramePercent() );

		// this should not affect the original table
		obCloneTableObj.SetNil("Yo");

		// this line will register our cloned table in the global namespace
//		GetState()->GetGlobals().SetObject( "CloneTable", obCloneTableObj );

		// dump created tables		
		GetState()->DumpObject( TEST_SCRIPT_PATH"test1.lua", "Table", obTableObj, false);
		GetState()->DumpObject( TEST_SCRIPT_PATH"test2.lua", "CloneTable", obCloneTableObj, false);
	}

	GetState()->GetGlobals().SetNil("Table");
//	GetState()->GetGlobals().SetNil("CloneTable");	// only nessecary for cleanup if registered globally
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestPointer
*
*	DESCRIPTION		Test registering a pointer callback func and calling it.
*
***************************************************************************************************/
void	CLuaTestRig::TestPointer( void )
{
	// scoping allows collect garbage to properly clean up after the LuaObjects
	{
		GetState()->GetGlobals().Register("LightUserDataCall", LS_LightUserDataCall );

		LuaObject obLUDFunc = GetState()->GetGlobal("LightUserDataCall");

		obLUDFunc.PCall( "u", (void*)0xfedcba98);
	
		GetState()->DumpObject( TEST_SCRIPT_PATH"TestDump.lua", "LightUserDataCall", obLUDFunc, LuaState::DUMP_WRITEALL);
	}

	GetState()->GetGlobals().SetNil("LightUserDataCall");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestTableIterator
*
*	DESCRIPTION		Test table iterators
*
***************************************************************************************************/
void	CLuaTestRig::TestTableIterator( void )
{	
	// scoping allows collect garbage to properly clean up after the LuaObjects
	{
		GetState()->DoString( "Table = { Hi = 5, Hello = 10, Yo = 6 }" );

		LuaObject obTableObj = GetState()->GetGlobal("Table");

		GetState()->DumpObject( TEST_SCRIPT_PATH"test1.lua", "Table", obTableObj, false);

		for ( LuaTableIterator obIt( obTableObj ); obIt; obIt.Next() )
		{
			printf( "Key:%s Value: %d", obIt.GetKey().GetString(), obIt.GetValue().GetInteger() );
		}
	}

	GetState()->GetGlobals().SetNil("Table");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestScriptFormat
*
*	DESCRIPTION		Test spitting out a vector file via metatable formatted write functions.
*
***************************************************************************************************/
void	CLuaTestRig::TestScriptFormat( void )
{
	const char* pcName =  TEST_SCRIPT_PATH"ScriptVectorDump.lua";
	printf("Running File %s\n", pcName);
	GetState()->DoFile( pcName );
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestScriptCallback
*
*	DESCRIPTION		Test registering funcitons and calling them in a script.
*
***************************************************************************************************/
void	CLuaTestRig::TestScriptCallback( void )
{
	GetState()->GetGlobals().Register("PrintNumber", LS_PrintNumber);
	GetState()->GetGlobals().Register("Add", LS_Add);
	GetState()->DoFile( TEST_SCRIPT_PATH"ScriptCallbackTest.lua" );
	GetState()->GetGlobals().SetNil("PrintNumber");
	GetState()->GetGlobals().SetNil("Add");
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestScriptSave
*
*	DESCRIPTION		Test dumping a complex lua state set.
*
***************************************************************************************************/
void	CLuaTestRig::TestScriptSave( void )
{
	const char* pcName =  TEST_SCRIPT_PATH"ScriptSaveTest.lua";
	printf("Running File %s\n", pcName);

	GetState()->DoFile( pcName );

	const char* pcName2 =  TEST_SCRIPT_PATH"ScriptSaveTest.out";
	printf("Dumping to File %s\n", pcName2);

	GetState()->DumpGlobals( pcName2 );

	GetState()->GetGlobals().SetNil("TestNumber");
	GetState()->GetGlobals().SetNil("TestString");
	GetState()->GetGlobals().SetNil("TestTable");
	GetState()->GetGlobals().SetNil("NestedTableLevel1");
	
	GetState()->CollectGarbage();
}

/***************************************************************************************************
*
*	FUNCTION		CLuaTestRig::TestScriptArray
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CLuaTestRig::TestScriptArray( void )
{
	GetState()->DoFile( TEST_SCRIPT_PATH"ScriptArrayTest.lua" );

	// scoping allows collect garbage to properly clean up after the LuaObjects
	{
		LuaObject obTableObj = GetState()->GetGlobals()[ "TestArray" ];

		for (int i = 1; ; ++i)
		{
			LuaObject obEntryObj = obTableObj[ i ];
			
			if (obEntryObj.IsNil())
				break;

			if (obEntryObj.IsNumber())
				printf("%f\n", obEntryObj.GetNumber());

			else if (obEntryObj.IsString())
				printf("%s\n", obEntryObj.GetString());
		}
	}

	GetState()->GetGlobals().SetNil("TestArray");
	GetState()->CollectGarbage();
}