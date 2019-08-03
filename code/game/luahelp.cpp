/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "game/luahelp.h"
#include "game/luaglobal.h"
#include "game/luardebug.h"
#include "game/messagehandler.h"
#include "input/inputhardware.h"
#include "gfx/levelofdetail.h"
#include "gfx/shadowsystem.h"
#include "gfx/levellighting.h"
#include "game/randmanager.h"
#include "core/io.h"
#include "objectdatabase/dataobject.h"


// BINDFUNC:		dofile( string levelname )
// DESCRIPTION:		Patch up absolute paths 
static int DoFileOveride(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);
	user_error_p( args[1].IsString(), ("BindFunction - DoFileOveride: Invalid argument.") );

	char acFileName[MAX_PATH];
	Util::GetFiosFilePath( args[1].GetString(), acFileName );

	NinjaLua::InstallFile(*pobState, acFileName);

	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		DumpCallStack
*
*	DESCRIPTION		Basic call stack dump for Lua
*
***************************************************************************************************/
void CLuaHelper::DumpCallStack(NinjaLua::LuaState& state)
{
	lua_State* L = &(*state);

	lua_Debug	stDebugArg;
	int			iLevel = 0;

	// Find the end of the depth
	for( ; lua_getstack (L, iLevel, &stDebugArg); ++iLevel ); 

	// No levels to dump out, then return now. 
	if( !iLevel ) return; 

	ntPrintf("Lua call stack dump:\n" );

	// Print the stack in a reverse order
	for( --iLevel; iLevel >= 0; --iLevel )
	{
		if( !lua_getinfo (L, "Sln", &stDebugArg) )
			continue;

		ntPrintf("level %d, %s:%d, (%s)\n", iLevel, stDebugArg.source, stDebugArg.currentline, stDebugArg.what );
	}

}

/***************************************************************************************************
*
*	FUNCTION		CLuaHelper::RetrieveLuaString
*
*	DESCRIPTION		retrive the lua string at the top of the stack
*
***************************************************************************************************/
const char* CLuaHelper::RetrieveLuaString( NinjaLua::LuaState* pobState, NinjaLua::LuaStack& args )
{
	ntAssert(pobState);

	// stack top holds number of arguments when called
	int iNumArgs = pobState->GetTop();

	static char acBuffer[512];
	acBuffer[0] = 0;

	// retrieve 'tostring' method for awkward print arguments
	NinjaLua::LuaObject obToStringFunc = pobState->GetGlobals()["tostring"];
	ntAssert_p( obToStringFunc.IsFunction(), ("Need a tostring method for this to work") );

	for (int i = 1; i <= iNumArgs; i++)
	{
		const char *pcString = NULL;

		if (args[i].IsString())  // normal
		{
			pcString = args[i].GetString();
		}
		else // table, function or some such
		{
			NinjaLua::LuaFunctionRet<const char*> to_string(obToStringFunc);
			
			pcString = to_string(NinjaLua::LuaObject(args[i]));

			if(pcString == NULL)
				return NULL;							// caller must trap this ntError
		}
		strcat(acBuffer, pcString);
	}
	return acBuffer;
}

/***************************************************************************************************
*
*	FUNCTION		CLuaHelper::FormatLuaString
*
*	DESCRIPTION		Mark each new line with the appriate marker strings.
*
***************************************************************************************************/
const char* CLuaHelper::FormatLuaString( const char* pcString, const char* pcStartStr, const char* pcEndStr )
{
	ntAssert(pcString);
	ntAssert(pcStartStr);
	ntAssert(pcEndStr);

	static char acFinalBuffer[1024];
	
	acFinalBuffer[0] = 0;

	const char* pcStart = pcString;
	char* pcNext = NULL;

	do
	{
		strcat( acFinalBuffer, pcStartStr );// insert start
		pcNext = strstr( (char*)pcStart, "\n" );	// seek to next newline

		if(pcNext != NULL)					// found one, plonk on end, repeat
		{
			strncat( acFinalBuffer, pcStart,  (int)(pcNext - pcStart));
			strcat( acFinalBuffer, pcEndStr );
			pcStart = pcNext + 1;
		}
		else								// finished, plonk on end, return
		{
			strcat( acFinalBuffer, pcStart );
			strcat( acFinalBuffer, pcEndStr );
			break;
		}
	}
	while( *pcStart != 0 );

	return acFinalBuffer;
}


// BINDFUNC:		LoadXMLFile( string filename )
// DESCRIPTION:		load an XML file into the game
static void LoadXMLFile( const char* pcFileName )
{
	static char acFileName[512];
	Util::GetFiosFilePath( pcFileName, acFileName );

	// Tell people what we're up to
	ntPrintf("XML loading \'%s\'\n", acFileName);

	// Open the XML file in memory
	FileBuffer obFile( acFileName, true );

	if ( !ObjectDatabase::Get().LoadDataObject( &obFile, pcFileName ) )
	{
		ntError_p( false, ( "Failed to parse XML file '%s'", pcFileName ) );
	}
}

// BINDFUNC:		SetLODBudget( float fLOD )
// DESCRIPTION:		Set the lod buget 
static void SetLODBudget( float fLOD )
{
	CLODManager::Get().SetLODBudget( fLOD );
}

// BINDFUNC:		LoadNewLightingDef( string filename )
// DESCRIPTION:		destroy the previous lighting definition and load a new one 
static int SwitchLightingFile(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);
	user_error_p( args[1].IsString(), ("BindFunction - SwitchLightingFile: Invalid argument.") );
	
	LevelLightingCtrl::SwitchLightingFile( args[1].GetString() );
	return 0;
}

// BINDFUNC:		SetTOD( int hours, int minutes )
// DESCRIPTION:		set a new time of day
static int SetTOD(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);

	user_error_p( args[1].IsInteger(), ("BindFunction - SetTOD: First argument is invalid") );
	user_error_p( args[2].IsInteger(), ("BindFunction - SetTOD: Second argument is invalid") );

	float fNewTOD = static_cast<float>( args[1].GetInteger() );
	fNewTOD += static_cast<float>( args[2].GetInteger() ) / 60.0f;

	LevelLighting::Get().SetTimeOfDay( fNewTOD );

	return 0;
}


// BINDFUNC:  Random(int min, int max)
// DESCRIPTION: Return a random integer in the specified range
static int Script_Random(int iMin, int iMax)
{
	return int( floor( ( grandf( 1.0f ) * float( iMax - iMin ) ) + 0.5f ) ) + iMin;
}

// BINDFUNC:  RandomPercentage()
// DESCRIPTION: Return a random percentage
static float Script_RandomPercentage()
{
	return grandf(100.0f);
}


// BINDFUNC:  IsKeyPressed(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug keyboard info
static int IsKeyPressed( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obKey = args[1];
	NinjaLua::LuaStackObject obModifier = args[2];
	
	if (obModifier.IsNone())
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyPressed((KEY_CODE)obKey.GetInteger()) );
	else
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyPressed((KEY_CODE)obKey.GetInteger(),obModifier.GetInteger()));

	return 1;
}

// BINDFUNC:  IsKeyHeld(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug keyboard info
static int IsKeyHeld( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obKey = args[1];
	NinjaLua::LuaStackObject obModifier = args[2];

	if (obModifier.IsNone())
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyHeld((KEY_CODE)obKey.GetInteger()) );
	else
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyHeld((KEY_CODE)obKey.GetInteger(),obModifier.GetInteger()));

	return 1;
}

// BINDFUNC:  IsKeyReleased(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug keyboard info
static int IsKeyReleased(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obKey = args[1];
	NinjaLua::LuaStackObject obModifier = args[2];

	if (obModifier.IsNone())
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyReleased((KEY_CODE)obKey.GetInteger()) );
	else
		pobState->Push( CInputHardware::Get().GetKeyboard().IsKeyReleased((KEY_CODE)obKey.GetInteger(),obModifier.GetInteger()));

	return 1;
}

// BINDFUNC:  IsPadPressed(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug pad info
static int IsPadPressed( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obPad = args[1];
	NinjaLua::LuaStackObject obButton = args[2];

	ntAssert( !obPad.IsNone() );
	ntAssert( !obButton.IsNone() );

	pobState->Push(!!(CInputHardware::Get().GetPad( (PAD_NUMBER)obPad.GetInteger() ).GetPressed() & (PAD_BUTTON)obButton.GetInteger() ));
	return 1;
}

// BINDFUNC:  IsPadHeld(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug pad info
static int IsPadHeld( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obPad = args[1];
	NinjaLua::LuaStackObject obButton = args[2];

	ntAssert( !obPad.IsNone() );
	ntAssert( !obButton.IsNone() );

	pobState->Push(!!(CInputHardware::Get().GetPad( (PAD_NUMBER)obPad.GetInteger() ).GetHeld() & (PAD_BUTTON)obButton.GetInteger() ));
	return 1;
}

// BINDFUNC:  IsPadReleased(int iKey, [int iModifier] )
// DESCRIPTION: Wrapper for debug pad info
static int IsPadReleased( NinjaLua::LuaState* pobState )
{
	NinjaLua::LuaStack args(pobState);
	NinjaLua::LuaStackObject obPad = args[1];
	NinjaLua::LuaStackObject obButton = args[2];

	ntAssert( !obPad.IsNone() );
	ntAssert( !obButton.IsNone() );

	pobState->Push(!!(CInputHardware::Get().GetPad( (PAD_NUMBER)obPad.GetInteger() ).GetReleased() & (PAD_BUTTON)obButton.GetInteger() ));
	return 1;
}

// BINDFUNC:  luaB_foreach(lua_State *L)
// DESCRIPTION: if we used the lua 5 libs, i wouldnt have to do this...
static int luaB_foreach (NinjaLua::LuaState* pState) {

	lua_State *L = &(**pState);
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	lua_pushnil(L);  /* first key */
	for (;;) {
	if (lua_next(L, 1) == 0)
		return 0;
	lua_pushvalue(L, 2);  /* function */
	lua_pushvalue(L, -3);  /* key */
	lua_pushvalue(L, -3);  /* value */
	lua_call(L, 2, 1);
	if (!lua_isnil(L, -1))
		return 1;
	lua_pop(L, 2);  /* remove value and result */
	}
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CLuaHelper::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.RegisterRaw( "dofile",		DoFileOveride );

	obGlobals.Register( "SetLODBudget", SetLODBudget );
	obGlobals.Register( "LoadXMLFile", LoadXMLFile );
	
	obGlobals.Register( "Random", Script_Random );
	obGlobals.Register( "RandomPercentage", Script_RandomPercentage );

	obGlobals.RegisterRaw( "IsKeyPressed",		IsKeyPressed );
	obGlobals.RegisterRaw( "IsKeyHeld",		IsKeyHeld );
	obGlobals.RegisterRaw( "IsKeyReleased",	IsKeyReleased );

	obGlobals.RegisterRaw( "IsPadPressed",		IsPadPressed );
	obGlobals.RegisterRaw( "IsPadHeld",		IsPadHeld );
	obGlobals.RegisterRaw( "IsPadReleased",	IsPadReleased );

	obGlobals.RegisterRaw( "foreach",			luaB_foreach );

	obGlobals.RegisterRaw( "SwitchLightingFile",	SwitchLightingFile ); 
	obGlobals.RegisterRaw( "SetTOD",	SetTOD ); 

}


