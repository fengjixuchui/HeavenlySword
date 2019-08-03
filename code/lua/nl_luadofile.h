//--------------------------------------------------
//!
//!	\file nl_luadofile.h
//!	seperate file loading from game lua implementation	
//!
//--------------------------------------------------

#ifndef _LUADOFILE_H
#define _LUADOFILE_H

namespace NinjaLua
{
	// Load a buffer and install it.
	bool DoBuffer( LuaState& state, const char* pcBuff, size_t size, const char* pcName );

	// Load a file and install it.
	void InstallFile( LuaState& state, const char* pcFileName );
};

#endif //_LUADOFILE_H
