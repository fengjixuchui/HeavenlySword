//--------------------------------------------------
//!
//!	\file nl_luadofile.h
//!	seperate file loading from game lua implementation	
//!
//--------------------------------------------------

#include "ninjalua.h"

namespace NinjaLua
{

//------------------------------------------------------------------------------------------
//!
//!	LoadError
//! A little function to log a warning when loading a .lua file
//!
//------------------------------------------------------------------------------------------
static int LoadError(lua_State *pobState)
{
	const char* file = lua_tostring(pobState,1);
	const char* error = lua_tostring(pobState,2);
	
	ntPrintf(error);
	UNUSED(file);
	UNUSED( error );
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	SVerySimpleLuaReaderData
//!	Data for the VerySimpleLuaReader Callback function
//!
//------------------------------------------------------------------------------------------
struct SVerySimpleLuaReaderData
{
	SVerySimpleLuaReaderData(const char* _pcBuffer, size_t _size) {pcBuffer = _pcBuffer; size = _size;}

	size_t      size;
	const char* pcBuffer;
};

//------------------------------------------------------------------------------------------
//!
//!	VerySimpleLuaReader
//!	Callback function used by lua to read in lua scripts
//!
//------------------------------------------------------------------------------------------
static const char* VerySimpleLuaReader(lua_State *, void *ud, size_t *size)
{
	SVerySimpleLuaReaderData* data = (SVerySimpleLuaReaderData*)ud;

	if(data->size == 0)
		return 0;

	*size = data->size;
	data->size = 0;
	return data->pcBuffer;
}

//------------------------------------------------------------------------------------------
//!
//!	DoBuffer
//!	Load a buffer and install it.
//!
//------------------------------------------------------------------------------------------
bool DoBuffer(LuaState& state, const char* pcBuff, size_t size, const char* pcName)
{
	SVerySimpleLuaReaderData data(pcBuff, size);

	int iStatus = lua_load(&(*state), VerySimpleLuaReader, &data, pcName);
	if(iStatus != 0)
	{
		switch(iStatus)
		{
		case LUA_ERRSYNTAX:
			LoadError(state);
			user_warn_p(false, ("LUA Syntax Error Loading Buffer: %s\n", pcName));
			break;
		case LUA_ERRMEM:
			LoadError(state);
			user_warn_p(false, ("LUA Memory Error Loading Buffer: %s\n", pcName));
			break;
		default:
			LoadError(state);
			user_warn_p(false, ("LUA Undefined Error Loading Buffer: %s\n", pcName));
		}
		return false;
	}
	
	iStatus = lua_pcall(&(*state), 0, LUA_MULTRET, 0);
	if(iStatus != 0)
	{
		switch(iStatus)
		{
		case LUA_ERRRUN:
			LoadError(state);
			user_warn_p(false, ("LUA Runtime Error Calling Buffer:: %s\n", pcName));
			break;
		case LUA_ERRMEM:
			LoadError(state);
			user_warn_p(false, ("LUA Memory Error Calling Buffer: %s\n", pcName));
			break;
		case LUA_ERRERR:
			LoadError(state);
			user_warn_p(false, ("LUA Error Error Calling Buffer: %s\n", pcName));
			break;
		default:
			LoadError(state);
			user_warn_p(false, ("LUA Undefined Error Calling Buffer: %s\n", pcName));

		}
		return false;
	}

	// All Good
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	InstallFile
//!	Load a file and install it.
//!
//------------------------------------------------------------------------------------------
void InstallFile(LuaState& state, const char* pcFileName)
{
	File file(pcFileName, File::FT_READ | File::FT_BINARY);
	ntAssert_p(file.IsValid(), ("%s(%d): CLuaGlobal::InstallFile: %s. Is it checked in?\n", __FILE__, __LINE__, pcFileName));

	if(!file.IsValid())
		return;

	size_t fileSize = file.GetFileSize();

	if(fileSize<=0)
	{
		ntPrintf("%s(%d): Reading from a zero sized file: %s\n", __FILE__, __LINE__, pcFileName);
		return;
	}

	char* pcContents = NT_NEW char[fileSize];
	size_t readSize = file.Read(pcContents, fileSize);

	DoBuffer(state, pcContents, readSize, pcFileName);
	NT_DELETE( pcContents );
}

}
