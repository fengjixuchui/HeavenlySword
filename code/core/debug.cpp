//------------------------------------------------------
//!
//!	\file core\debug.cpp
//!	Implements the debug library code and statics.
//!
//------------------------------------------------------

#include "core/io.h"
#include "core/osddisplay.h"

#include "game/luahelp.h"	// ATTN! deano. dont want game includes here...
#include "game/luardebug.h"
#include "game/shellconfig.h"

#include "gfx/display.h"

#if defined(PLATFORM_PS3)
#include <sys/console.h>
#endif


//! all channels initialise to no debug output before being setup
uint32_t Debug::s_uiDestFlags[MAX_DEBUG_CHANNELS] = {0};
//! no file handles (obviously)
LogFile* Debug::s_pLogFile[MAX_DEBUG_CHANNELS] = {0};
//! Callbacks for each debug channel
Debug::DebugNetCallback Debug::s_pLogDebugNet[MAX_DEBUG_CHANNELS];

//! current handle in use (default to global)
uint8_t Debug::s_uiCurrentChannel = 0;
//! inibit DebugBreak from actually breaking into the debugger
bool Debug::s_bInhibitDebugBreak = false;

//! a global used to force debug output when the Debug object has been killed to the console
//! so we can at least see something
bool f_bDebugUp = false;

//! critical section to serialises all debug output..
CriticalSection Debug::s_DebugCritSec;

// size of the maximum ntPrintf message
static const int MAX_PRINTF_BUFFER_SIZE = 1024;

namespace
{
	bool ATGAssertHook(  const char* pCond, const char* pCondMsg, const char* pFile, int line )
	{
		ntPrintf( Debug::DCU_CORE, "%s(%d) : Assertion failed: %s \n", pFile, line, pCond );
		if (pCondMsg)
			ntPrintf( Debug::DCU_CORE, "%s\n", pCondMsg );
		if( Debug::s_bInhibitDebugBreak == false )
			return true;
		else
			return false;
	}
};

//-------------------------------------------------------------
//!
//! Update wether we send this channel console output or not,
//! Doesn't effect other types in any way.
//! \param uiChannel Which channel to we want to set
//! \param bEnable Do we want to send this channel or not?
//!
//-------------------------------------------------------------
void Debug::SetLogToConsole( const uint8_t uiChannel, bool bEnable)
{
	ScopedCritical sc( s_DebugCritSec );
	ntAssert_p( uiChannel >= DCU_START_ID, ("Invalid Debug Channel ID") );
	if( bEnable )
	{
		// we want console output
		s_uiDestFlags[uiChannel-DCU_START_ID] |= Debug::DD_CONSOLE;
	} else
	{
		// we don't want console output
		s_uiDestFlags[uiChannel-DCU_START_ID] &= ~Debug::DD_CONSOLE;
	}
}

//-------------------------------------------------------------
//!
//! Update wether we send this channel output to a file,
//! Doesn't effect other types in any way.
//! \param uiChannel Which channel to we want to set
//! \param pFileHandle file handle to direct to or 0 to turn off
//!
//-------------------------------------------------------------
void Debug::SetLogToFile( const uint8_t uiChannel, LogFile* pFileHandle)
{
	ScopedCritical sc( s_DebugCritSec );
	ntAssert_p( uiChannel >= DCU_START_ID, ("Invalid Debug Channel ID") );

	// we want file output 
	// if we are changing global log file we may have to close our version
	if( uiChannel == DCU_GLOBAL && s_uiDestFlags[0] & DD_INT_GLOBAL_FLAG)
	{
		CloseGlobalLogFile();
	}

	if( pFileHandle )
	{
		s_pLogFile[uiChannel-DCU_START_ID] = pFileHandle;
		s_uiDestFlags[uiChannel-DCU_START_ID] |= Debug::DD_FILE;
	} else
	{
		s_pLogFile[uiChannel-DCU_START_ID] = 0;
		s_uiDestFlags[uiChannel-DCU_START_ID] &= ~Debug::DD_FILE;
	}
}

//-------------------------------------------------------------
//!
//! Update wether we send this channel output to the debug network,
//! callback. On console the debug network callback usually send its
//! off to the remote debug console. In debug console its usually
//! sends it to a text processor for colouring etc.
//! \param uiChannel Which channel to we want to set
//! \param pCallback call back to call or 0 to turn off
//!
//-------------------------------------------------------------
void Debug::SetLogToDebugNet( const uint8_t uiChannel, DebugNetCallback pCallback )
{
	ScopedCritical sc( s_DebugCritSec );
	ntAssert_p( uiChannel >= DCU_START_ID, ("Invalid Debug Channel ID") );

	if( pCallback )
	{
		s_pLogDebugNet[uiChannel-DCU_START_ID] = pCallback;
		s_uiDestFlags[uiChannel-DCU_START_ID] |= Debug::DD_DEBUGNET;
	} else
	{
		s_pLogDebugNet[uiChannel-DCU_START_ID] = 0;
		s_uiDestFlags[uiChannel-DCU_START_ID] &= ~Debug::DD_DEBUGNET;
	}
}


//-------------------------------------------------------------------
//!
//! Ensures all messages have completed and are at there destinations.
//!
//-------------------------------------------------------------------
void Debug::FlushLog()
{
	ScopedCritical sc( s_DebugCritSec );

	for (u_int i = 0; i < MAX_DEBUG_CHANNELS; i++)
	{
		if	(
			(s_pLogFile[i]) &&
			(s_uiDestFlags[i] & Debug::DD_FILE)
			)
			s_pLogFile[i]->Flush();
	}
}

//-------------------------------------------------------------------
//!
//! function to spin until a lua debugger is connected
//!
//-------------------------------------------------------------------
void Debug::WaitForLuaDebugger(bool bFatal)
{
#ifndef REMOVE_LUA
#ifndef _GOLD_MASTER
	if(LuaDebugServer::AutoLaunchEnabled() && g_message_box_override == false )
	{
		MBPrintf( "Start your Lua Scite Debugger" );
		CLuaHelper::DumpCallStack(CLuaGlobal::Get().State());
		LuaDebugServer::Assert();
	} else if(bFatal)
	{
		ntPrintf( "WaitForLuaDebugger: The user doesn't want to use the Lua Debugger (or is fullscreen) so causing a crash dump" );
		ntError(0);
	}
	else
		ntPrintf("WaitForLuaDebugger: Non Fatal Breakpoint ignored as debugging disabled.\n");
#endif
#endif
}

//------------------------------------------------------
//!
//! replace newline characters in string with windows ones
//!
//------------------------------------------------------
void FixFileNewLines( const char* restrict str, char* result )
{
	const char* restrict pCurr = str;
	const char* restrict pNext = strstr( pCurr, "\n" );
	
	result[0] = 0;

	while (pNext != NULL)
	{
		// copy up to this new line
		strncat( result, pCurr, (pNext - pCurr) );

		// insert CR+LF (\r\n)
		strcat( result, "\r\n" );

		// search for next newline
		pCurr = pNext+1;
		pNext = strstr( pCurr, "\n" );
	}

	// copy up to end of array
	strcat( result, pCurr );
}

//------------------------------------------------------
//!
//! Outputs the debug string to the outputs
//!
//------------------------------------------------------
void Debug::OutputString( const char* restrict str )
{
#	ifndef _GOLD_MASTER
	{
		ScopedCritical sc( s_DebugCritSec );

		// output to the debug console if the current or the global channel
		// asked for 
		if( s_uiDestFlags[s_uiCurrentChannel] & Debug::DD_CONSOLE || ( f_bDebugUp == false) )
		{
#			if defined( PLATFORM_PC )
				::OutputDebugString( str );
#			elif defined( PLATFORM_PS3 )
				FwPrintf( str );
#			endif
		}

		// output to a file if the current channel wants it
		if( s_uiDestFlags[s_uiCurrentChannel] & Debug::DD_FILE )
		{
			ntAssert( s_pLogFile[s_uiCurrentChannel] != 0 && s_pLogFile[s_uiCurrentChannel]->IsValid() );
			
#			ifdef PLATFORM_PS3
				char newBuffer[ MAX_PRINTF_BUFFER_SIZE ];
				FixFileNewLines( str, newBuffer );
				s_pLogFile[s_uiCurrentChannel]->Write( newBuffer, strlen(newBuffer) );
#			else		
				s_pLogFile[s_uiCurrentChannel]->Write( str, strlen(str) );
#			endif
		}

		// output to the debug net system if the current channel wants it
		if( s_uiDestFlags[s_uiCurrentChannel] & Debug::DD_DEBUGNET )
		{
			ntAssert( s_pLogDebugNet[s_uiCurrentChannel] != 0 );
			s_pLogDebugNet[s_uiCurrentChannel]( str ); // call callback 
		}

		// do global output if we want it and the current channel is 0
		if( s_uiDestFlags[0] != 0 && s_uiCurrentChannel != 0)
		{
			static const int TEMP_STRING_LEN = 2048;
			const char aChanString[] = "Channel 0x%0X : %s";
			char aTempString[TEMP_STRING_LEN];

			// make sure we don't overflow just drop the log in this cause
			if( strlen(str) > TEMP_STRING_LEN - strlen(aChanString) -2 )
			{	
				return;
			}
			sprintf( aTempString, aChanString, s_uiCurrentChannel, str );

			if(	s_uiDestFlags[0] & Debug::DD_CONSOLE )
			{
#				if defined( PLATFORM_PC )
					::OutputDebugString( aTempString );
#				elif defined( PLATFORM_PS3 )
					FwPrintf( aTempString );
#				endif
			}

			if( s_uiDestFlags[0] & Debug::DD_FILE )
			{
				ntAssert( s_pLogFile[0] != 0 );

#				ifdef PLATFORM_PS3
					char newBuffer[ MAX_PRINTF_BUFFER_SIZE ];
					FixFileNewLines( aTempString, newBuffer );
					s_pLogFile[0]->Write( newBuffer, strlen(newBuffer) );
#				else		
					s_pLogFile[0]->Write( aTempString, strlen(aTempString) );
#				endif
			}
		}
	}
#	endif // !_GOLD_MASTER
}

//! Always Outputs the string to the debug console regardless of build setting
void Debug::AlwaysOutputString( const char* restrict str )
{
#	ifndef _GOLD_MASTER
	{
#		if defined( PLATFORM_PC )
			::OutputDebugString( str );
#		elif defined( PLATFORM_PS3 )
			// this is marked as temporary... so this probably will stop compiling one upgrade of the OS...
			// and yes the OS does take a char* not a const char*...
			console_write( const_cast<char*>(str), strlen(str) );
#		endif
	}
#	endif // !_GOLD_MASTER
}


//--------------------------------------------------------------------
//!
//! Initalises the debug system. this potentially can occur very early
//! so shouldn't rely on memory, filesystems or network being available
//! these should be set later on if needed.
//!
//--------------------------------------------------------------------
void Debug::Init()
{
	ScopedCritical sc( s_DebugCritSec );

	f_bDebugUp = true;

	// default global log to file output called debug.log
#ifdef PLATFORM_PS3
	if (g_ShellOptions->m_bUsingHDD == false)
#endif
		InitGlobalLog();

	// hook ATW base ntAssert system into our logging system
	FwBaseAssertCtrl::SetHandler( &ATGAssertHook );
}

void Debug::InitGlobalLog()
{
	OpenGlobalLogFile();

	SetLogToFile( DCU_GLOBAL, s_pLogFile[0] );

	// so any change to the global file will cause us to close the file
	s_uiDestFlags[0] |= DD_INT_GLOBAL_FLAG;
}

//-------------------------------------------------------------------
//!
//! Kills the debug systems. should be last thing to happen
//!
//-------------------------------------------------------------------
void Debug::Kill()
{
	ScopedCritical sc( s_DebugCritSec );

	f_bDebugUp = false;

	// do we need to close the file
	for( unsigned int i=0;i < MAX_DEBUG_CHANNELS;i++)
	{
		SetLogToConsole( (uint8_t) (i + DCU_START_ID), false );
		SetLogToFile( (uint8_t) (i + DCU_START_ID), 0 );
		SetLogToDebugNet( (uint8_t) (i + DCU_START_ID), 0 );
	}
}




//---------------------------------------
//!
//! Open's the log file for writing
//!
//---------------------------------------
void Debug::OpenGlobalLogFile()
{
	ScopedCritical sc( s_DebugCritSec );

	s_pLogFile[0] = CreateNewLogFile( "debug.log" );
	s_uiDestFlags[0] |= Debug::DD_FILE;
	ntAssert_p( s_pLogFile[0] != 0 && s_pLogFile[0]->IsValid(), ("Unable to open log file") );

	// write a version string
	// TODO proper version IDs here
	char aVersionString[1024];
//	char aVersionString[Version::MAX_OUT_STRING_LENGTH];
//	g_CoreVersion.GetVersionString( aVersionString );
#if defined(PLATFORM_PC)
	strcpy( aVersionString, "PC Build\n" );
#elif defined(PLATFORM_PS3)
	strcpy( aVersionString, "PS3 Build\n" );
#endif
	s_pLogFile[0]->Write( aVersionString, strlen( aVersionString ) );
}
//---------------------------------------
//!
//! Close the log file
//!
//----------------------------------------
void Debug::CloseGlobalLogFile()
{
	ScopedCritical sc( s_DebugCritSec );
	s_uiDestFlags[0] &= ~DD_INT_GLOBAL_FLAG;
	s_uiDestFlags[0] &= ~Debug::DD_FILE;

	NT_DELETE( s_pLogFile[0] );
	s_pLogFile[0] = 0;
}

//---------------------------------------
//!
//! Exits the game right now!
//!
//----------------------------------------
void Debug::ExitGame()
{
#	ifdef PLATFORM_PC
		// TODO Clean exit
		ntPrintf("Exiting Game\n");
		exit(1);
#	elif defined (PLATFORM_PS3)
		DebugBreakNow();
#	endif
}

//--------------------------------------------------------------------
//!
//! replacement ntPrintf
//!
//--------------------------------------------------------------------
int Debug::Printf( const char* restrict pcFormat, ... )
{
#	ifndef _RELEASE
	{
		ScopedCritical sc( Debug::s_DebugCritSec );

		// process the ntPrintf style stuff onto a big bit of stack memory
		char acBuffer[ MAX_PRINTF_BUFFER_SIZE ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		// output the formated string
		Debug::OutputString( acBuffer );

		return iResult;
	}
#	else
	{
		UNUSED( pcFormat );
		return 0;	// Return number of characters written.
	}
#	endif
}

//--------------------------------------------------------------------
//!
//! replacement ntPrintf with custom channel stuff
//!
//--------------------------------------------------------------------
int Debug::Printf( DEBUG_CHANNEL_USAGE uiChannel, const char* restrict pcFormat, ... )
{
#	ifndef _RELEASE
	{
		ScopedCritical sc( Debug::s_DebugCritSec );

		// process the ntPrintf style stuff onto a big bit of stack memory
		char acBuffer[ MAX_PRINTF_BUFFER_SIZE ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		// back up the current channel
		uint8_t iCurChannel = Debug::GetCurrentChannel();
		// set out one and output the message
		Debug::SetCurrentChannel( (uint8_t) uiChannel );
		Debug::OutputString( acBuffer );
		// restore the old channel
		Debug::SetCurrentChannel( iCurChannel );

		return iResult;
	}
#	else
	{
		UNUSED( uiChannel );
		UNUSED( pcFormat );
		return 0;	// Return number of characters written.
	}
#	endif
}

//--------------------------------------------------------------------
//!
//! ntPrintf that sticks up a message box on PC
//!
//--------------------------------------------------------------------
int Debug::MBPrintf( const char* restrict pcFormat, ... )
{
#	ifndef _RELEASE
	{
		ScopedCritical sc( Debug::s_DebugCritSec );

		// process the ntPrintf style stuff onto a big bit of stack memory
		char acBuffer[ MAX_PRINTF_BUFFER_SIZE ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		Debug::OutputString( "\n--------------- MBPrintf ---------------\n" );
		Debug::OutputString( acBuffer );

#		if defined( PLATFORM_PC )
		{
			if ( DisplayManager::Exists() && !DisplayManager::Get().IsFullScreen() )
			{
				MessageBox( 0, acBuffer, "Game Message", MB_OK );
			}
		}
#		else
		{
			//TODO Use OS message box but for now just cause a flush of the output
			Debug::OutputString( "\n--------------- MBPrintf ---------------\n" );
		}
#		endif

		return iResult;
	}
#	else
	{
		UNUSED( pcFormat );
		return 0;	// Return number of characters written.
	}
#	endif
}

//--------------------------------------------------------------------
//!
//! LuaWarning that sticks up a console message
//! and logs the warning to SciTE if it's connected
//!
//--------------------------------------------------------------------
void Debug::LuaWarning(const char* restrict pcFormat, ...)
{
#ifndef REMOVE_LUA
#ifndef _GOLD_MASTER
	// process the ntPrintf style stuff onto a big bit of stack memory
	char acBuffer[ MAX_PRINTF_BUFFER_SIZE ];
	acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
	int iResult;
	va_list	stArgList;
	va_start( stArgList, pcFormat );
	iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
	va_end( stArgList );

	if(OSD::IsChannelEnabled(OSD::SCRIPT))
		OSD::Add(OSD::SCRIPT, DC_RED, acBuffer);
	else
		ntPrintf(acBuffer);

	LuaDebugServer::Trace(acBuffer);
#endif
#endif
}

LogFile* Debug::CreateNewLogFile( const char* pName )
{
	char path[MAX_PATH];

#ifdef PLATFORM_PS3
	Util::GetFullGameDataFilePath( pName, path );
#else
	Util::GetFiosFilePath( pName, path );
#endif

	return NT_NEW LogFile(path, DEBUG_LOG_FLAGS);
}

// these functions are global namespace function callable from C
// C_DBG_Printf lives with the other ntPrintf in ntPrintf.cpp
extern "C"
{
	//! C Flush nt printf
	int C_DBG_ntPrintf( const char* restrict pcFormat, ... )
	{
		// process the ntPrintf style stuff onto a big bit of stack memory
		char acBuffer[ MAX_PRINTF_BUFFER_SIZE ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		Debug::OutputString( acBuffer );
		return iResult;
	}

	//! C Flush Log
	void C_DBG_FlushLog()
	{
		Debug::FlushLog();
	}

	//! C Debug Break
	void C_DBG_DebugBreak()
	{
		DebugBreakNow();
	}
}


#undef DebugBreakNow
//! to keep GCC happy we need a function version (not entirely sure why
void DebugBreakNow()
{
#	ifndef _GOLD_MASTER
	{
		ScopedCritical sc( Debug::s_DebugCritSec );				
		if( Debug::s_bInhibitDebugBreak == false )				
		{														
#			if defined( PLATFORM_PC )									
				::DebugBreak();
#			elif defined( PLATFORM_PS3 )
				asm( "trap" );
#			endif
		}														
	}
#	endif // !GOLD_MASTER
}
