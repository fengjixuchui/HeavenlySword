#if !defined(CORE_DEBUG_H)
#define CORE_DEBUG_H
//---------------------------------------
//!
//!	\file core\debug.h
//!	Asserts and other debug functions.
//!
//---------------------------------------

#if defined( __cplusplus )

#undef	ntAssert


#if !defined(CORE_SYNCPRIMS_H)
#include "core/syncprims.h"
#endif


#ifdef PLATFORM_PS3

#	if !defined(CORE_CELLFSFILE_H)
#	include "core/cellfsfile_ps3.h"
#	endif

	typedef CellFsFile LogFile;

#else	// PLATFORM_PS3

#	if !defined(CORE_FILE_H)
#	include "core/file.h"
#	endif

	typedef File LogFile;

#endif	// PLATFORM_PS3

//---------------------------------------------------------
//! Forces a break into the debugger.
//! Function version to keep GCC happy, though in general
//! the macro version will be used. Gives you much better
//! break position in the debugger.
//---------------------------------------------------------
void DebugBreakNow();

#if defined( PLATFORM_PC )									
#	define DebugBreakNow()										\
	{															\
		ScopedCritical sc( Debug::s_DebugCritSec );				\
		if( Debug::s_bInhibitDebugBreak == false )				\
		{														\
			::DebugBreak();										\
		}														\
	}															
#elif defined( PLATFORM_PS3 )
#	define DebugBreakNow()										\
	{															\
		ScopedCritical sc( Debug::s_DebugCritSec );				\
		if( Debug::s_bInhibitDebugBreak == false )				\
		{														\
			asm( "trap" );										\
		}														\
	}
#endif

#ifdef PLATFORM_PC
#	define ntBreakpoint() asm( "int 3" )
#elif PLATFORM_PS3
#	define ntBreakpoint() __asm__ volatile ( "tw 31,1,1" )
#else
#	error Unknown platform.
#endif


// this will force all message boxes to not appear
extern bool g_message_box_override;

//! Compile-time ntAssert helper template class.
template < bool > class CCompileTimeError;

//! Compile-time ntAssert helper template class specialisation for true.
template <> class CCompileTimeError< true > {};

//! Compile-time assertion.
//! \note The message cannot contain spaces. Use messages like Thing_Size_Invalid.
#define static_assert(expr, msg) typedef CCompileTimeError<((expr) != 0)> ERROR_##msg
#define static_assert_in_class( expr, msg ) typedef char ERROR_##msg[1][(expr) == 0 ? -1 : 1 ]

//*******************************************
// Asserts.
//*******************************************
#ifdef	_DEBUG

	//! Asserts condition is not false
	//! \param condition log and break if not true
#	define ntAssert( condition )																					\
		if( ((condition) ? 0 : Debug::Printf("%s(%d): Assertion failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																											\
			Debug::FlushLog();																						\
			DebugBreakNow();																						\
		}

	//! Asserts condition is not false and display user message
	//! \param condition log and break if not true
#	define ntAssert_p( condition, condition_msg )																	\
		if( ((condition) ? 0 : Debug::Printf("%s(%d): Assertion failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																											\
			Debug::Printf condition_msg;																			\
			Debug::Printf("\n");																					\
			Debug::FlushLog();																						\
			DebugBreakNow();																						\
		}
#else 

#	define ntAssert( condition )	
#	define ntAssert_p( condition, condition_msg )	

#endif // _DEBUG
//*******************************************
//*******************************************

//*******************************************
//	ntError
//*******************************************
#ifndef	_RELEASE
	//! Asserts condition is not false on all builds except Release builds
	//! \param condition log and break if not true
#	define ntError( condition )																						\
		if( ((condition) ? 0 : Debug::Printf("%s(%d): Assertion failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																											\
			Debug::FlushLog();																						\
			DebugBreakNow();																						\
		}

	//! Asserts condition is not false and display user message even on Release builds
	//! \param condition log and break if not true
#	define ntError_p( condition, condition_msg )																	\
		if( ((condition) ? 0 : Debug::Printf("%s(%d): Assertion failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																											\
			Debug::Printf condition_msg;																			\
			Debug::Printf("\n");																					\
			Debug::FlushLog();																						\
			DebugBreakNow();																						\
		}

#	define lua_debug_break           Debug::WaitForLuaDebugger();
#	define lua_debug_break_non_fatal Debug::WaitForLuaDebugger(false);

#	define lua_bind_warn_msg(condition_msg) Debug::LuaWarning condition_msg;


#	define assert_lua_p( condition, condition_msg )																		\
		if( ((condition) ? 0 : Debug::Printf("%s(%d): Lua Assertion failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																												\
			Debug::MBPrintf condition_msg;																				\
			Debug::WaitForLuaDebugger();																				\
		}

	template< int T >
	struct one_time_assert_internal
	{
		bool Warn()
		{
			if( s_bShown == false )
			{
				s_bShown = true;
				return true;
			} else
			{
				return false;
			}
		}
		static bool s_bShown;
	};
	template<int T > bool one_time_assert_internal<T>::s_bShown = false;

	// one time warns are currently always compiled in, in future the final master build should remove them
	// tag is an integer that allows you to group one time warns (you'll have to manage to namespace your self)
#	define one_time_assert_p( tag, condition, condition_msg )												\
		if ( !( condition) )																				\
		{																									\
			one_time_assert_internal<tag> warner;															\
			if( warner.Warn() )																				\
			{																								\
				Debug::Printf( "%s(%d): one_time_assert failed: %s\n", __FILE__, __LINE__, #condition);		\
				Debug::MBPrintf condition_msg;																\
			}																								\
		}

#	define user_warn( condition )																						\
		if( ((condition) ? 0 : Debug::MBPrintf( "%s(%d): user_warn failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																												\
		}

#	define user_warn_p( condition, condition_msg )																		\
		if( ((condition) ? 0 : Debug::Printf( "%s(%d): user_warn_p failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																												\
			Debug::MBPrintf condition_msg;																				\
		}

#	define user_warn_msg( condition_msg )	if( true ) { Debug::MBPrintf condition_msg; }

#	define user_error( condition )																						\
		if( ((condition) ? 0 : Debug::MBPrintf( "%s(%d): user_error failed: %s\n", __FILE__, __LINE__, #condition)) )	\
		{																												\
			Debug::ExitGame();																							\
		}

	// user ntError is an ntError that display a dialog box to tell the user and then exits without a crash dump
#	define user_error_p( condition, condition_msg )																		\
		if( ((condition) ? 0 : Debug::Printf( "%s(%d): user_error failed: %s\n", __FILE__, __LINE__, #condition)) )		\
		{																												\
			Debug::Printf condition_msg;																				\
			Debug::MBPrintf condition_msg;																				\
			Debug::ExitGame();																							\
		}


#else	//_RELEASE

#	define ntError( condition )
#	define ntError_p( condition, condition_msg )
#	define lua_debug_break
#	define lua_debug_break_non_fatal
#	define lua_bind_warn_msg(condition_msg)
#	define assert_lua_p( condition, condition_msg )
#	define one_time_assert_p( tag, condition, condition_msg )
#	define user_warn( condition )
#	define user_warn_p( condition, condition_msg )
#	define user_warn_msg( condition_msg )
#	define user_error( condition )
#	define user_error_p( condition, condition_msg )

#endif	//_RELEASE
//*******************************************
//*******************************************


//--------------------------------------------------------------
//!
//!	The main static debug class all debug function route through here.
//!	Contains the real meat of the debug system, routes and writes 
//! all debug info to the correct place, this is a static due to Singleton 
//! actually using it for its assertation, manually call Init(), Kill() ASAP 
//! after creation. 
//! DO NOT USE MEMORY STUFF IN static classes IT will lock you 
//! machine hard.
//! There are lots of channels (enough for both systems and user channel)
//! Each channel can be directed to multiple places at the same time
//! (E.g. both console and file can be output simulatously)
//! DCU_GLOBAL is a global channels to gets all other channels, best
//! to spit it to a file for a total debug log
//!	\note Init should be as light as possible, as file and memory system 
//! may not be in place yet.
//!
//-----------------------------------------------------------------------------------------------
class Debug
{
public:
	//! maximum number of debug channels. The memory used is MAX_DEBUG_CHANNELS * 16 bytes currently
	static const unsigned int MAX_DEBUG_CHANNELS = 128;

	//! What each channel number is actually used for
	enum DEBUG_CHANNEL_USAGE
	{
		DCU_START_ID = 128, //!< begining of channel namespace
		DCU_GLOBAL   = 128, //!< all output is mirrored to here its only valid input to SetLogDest
		DCU_CORE	 = 129,	//!< core systems logs (including memory)
		DCU_GRAPHICS = 130,	//!< graphics logs
		DCU_SOUND	 = 131,	//!< sound logs
		DCU_NETWORK	 = 132,	//!< network logs
		DCU_LUA		 = 133, //!< Lua logs
		DCU_GAME	 = 134, //!< Logs from the game
		
		DCU_EXEC	 = 135, //!< Logs from exec
		DCU_MATH	 = 136, //!< Logs from math library/unit test

		DCU_TEXTURE	 = 137, //!< Record problems with textures and texture info
		DCU_CLUMP	 = 138, //!< Record problems with clumps and clump info
		DCU_ANIM	 = 139, //!< Record problems with animations and animation info

		DCU_RESOURCES = 140,	//!< 
		DCU_AREA_LOAD = 141,	//!< 
		DCU_AI		  = 142,	//!< AI logs
		DCU_AI_CHBOX  = 143,	//!< AI logs
		DCU_AI_BEHAVE = 144,

		DCU_USER_START = 200, //!< room for 55 user channels
		DCU_USER_DEANO = 200, 
		DCU_USER_MIKEY,
		DCU_USER_WIL,
		DCU_USER_HARVEY,
		DCU_USER_JOHN,
		DCU_USER_GILES,
		DCU_USER_FRANK,
		DCU_USER_MARCO,
		DCU_USER_PAUL,
		DCU_USER_OSIRIS,
		DCU_USER_ANDREW,
		DCU_USER_MUSTAPHA,
		DCU_USER_DUNCAN,
		DCU_USER_GAV,
		DCU_USER_PAULC,
		DCU_USER_GUYM

	};
	
	//! our replacement version of ntPrintf
	static int Printf( const char* restrict pcFormat, ... );

	//! a version of ntPrintf that takes a channel as the first parameter
	static int Printf( DEBUG_CHANNEL_USAGE uiChannel, const char* restrict pcFormat, ... );

	//! a version of ntPrintf that on PC raises a message box (PS3 TODO)
	static int MBPrintf( const char* restrict pcFormat, ... );

	//! a version of ntPrintf that sends the ntError to OSD for Lua
	static void LuaWarning( const char* restrict pcFormat, ... );

	//! Sends debug log message to the console.
	static void SetLogToConsole( const uint8_t uiChannel, bool bEnable);

	//! Sends debug log message to the OSD.
	static void SetLogToOSD( const uint8_t uiChannel, bool bEnable);

	//! Sends debug log message to a file.
	static void SetLogToFile( const uint8_t uiChannel, LogFile* pFileHandle );

	//! function to spin until a lua debugger is connected
	static void WaitForLuaDebugger(bool bFatal = true);

	//! a fast emergenacy exit
	static void ExitGame();

	//! function to get called in the debug network code 
	//! Debug system is NOT Re-entrant DO NOT Use any debug functions inside the callback
	typedef void (*DebugNetCallback)( const char* pText);

	//! Sends debug log message to a debug network callback.
	static void SetLogToDebugNet( const uint8_t uiChannel, DebugNetCallback pCallback );

	//! Flush logs causes any buffers to sent to where there going NOW!.
	static void FlushLog();

	//! Set the current channel to the specified 
	static void SetCurrentChannel( const uint8_t uiChannel )
	{
		s_uiCurrentChannel = (uint8_t) (uiChannel - DCU_START_ID);
	}

	//! Gets the current channel
	static uint8_t GetCurrentChannel() 
	{ 
		return (uint8_t) (s_uiCurrentChannel + DCU_START_ID); 
	}

	//! Outputs the string to the debug output
	static void OutputString( const char* restrict str );

	//! Always Outputs the string to the debug console regardless of build setting. 
	//! Note bypassing most debug stuff (logs etc.) and should only be used in special circumstance (release mode profiling and tools)
	static void AlwaysOutputString( const char* restrict str );

	//! sets up the debug system
	static void Init();

	//! tears down the debug system
	static void Kill();
	//! Global flag that inhibits asserts/ntError's breaking into the debugger.
	//! To use set this manually in the debugger to true
	static bool s_bInhibitDebugBreak;

	//! critical section to allow multiple thread to use debug safetly
	static CriticalSection s_DebugCritSec;

	//! helper function to abstract log file creation
	static LogFile* CreateNewLogFile( const char* pName );

	//! startup the global log
	static void InitGlobalLog();

private:
	//! bit field where to send debug messages
	enum DEBUG_DEST
	{
		DD_CONSOLE 	= 1 << 0,	//!< Send logs to debug console
		DD_FILE 	= 1 << 1,	//!< Send logs to a file
		DD_DEBUGNET	= 1 << 2,	//!< Send logs through the debug net system

		DD_INT_GLOBAL_FLAG = 1 << 31, //!< Used to know if we created the file or not
	};

	//! where the debug messages are going
	static uint32_t s_uiDestFlags[MAX_DEBUG_CHANNELS];

	//! file handles for each debug channel
	static LogFile* s_pLogFile[MAX_DEBUG_CHANNELS];

	//! Callbacks for each debug channel
	static DebugNetCallback s_pLogDebugNet[MAX_DEBUG_CHANNELS];

	//! Current channel debugging output is going to 
	static uint8_t s_uiCurrentChannel;

	//! open the global log file
	static void OpenGlobalLogFile();
	//! close the global log file
	static void CloseGlobalLogFile();			
};

#ifndef _RELEASE
#	define ntPrintf Debug::Printf
#else
#	ifdef PLATFORM_PS3
#		define ntPrintf( ... ) do {} while( 0 )
#	elif PLATFORM_PC
		// VS.Net 7.1 doesn't have VARARG Macros.
#		define ntPrintf Debug::Printf
#	endif
#endif

#else // ^^^ C++ version

#ifdef	_DEBUG

#define ntAssert( condition )																									\
	{	int aXTYZYYD = ((condition) ? 0 : C_DBG_ntPrintf( "%s(%d) : Assertion failed: %s\n", __FILE__, __LINE__, #condition )); \
		if( aXTYZYYD )																											\
		{																														\
			C_DBG_FlushLog();																									\
			C_DBG_DebugBreak();																									\
		}																														\
	}

//! Asserts condition is not false and display user message
//! \param condition log and break if not true
#define ntAssert_p( condition, condition_msg )																					\
	{	int aXTYZYYD = ((condition) ? 0 : C_DBG_ntPrintf( "%s(%d) : Assertion failed: %s\n", __FILE__, __LINE__, #condition )); \
		if( aXTYZYYD )																											\
		{																														\
			C_DBG_ntPrintf condition_msg;																						\
			C_DBG_ntPrintf( "\n" );																								\
			C_DBG_FlushLog();																									\
			C_DBG_DebugBreak();																									\
		}																														\
	}

#else 

#define ntAssert( condition )	
#define ntAssert_p( condition, condition_msg )	

#endif // _DEBUG

#ifndef	_RELEASE
//! Asserts condition is not false even on Release builds
//! \param condition log and break if not true
#define ntError( condition )																						\
	if( ((condition) ? 0 : C_DBG_ntPrintf( "%s(%d) : Assertion failed: %s\n", __FILE__, __LINE__, #condition )) )	\
	{																												\
		C_DBG_FlushLog();																							\
		C_DBG_DebugBreak();																							\
	}

//! Asserts condition is not false and display user message even on Release builds
//! \param condition log and break if not true
#define ntError_p( condition, condition_msg )																		\
	if( ((condition) ? 0 : C_DBG_ntPrintf( "%s(%d) : Assertion failed: %s\n", __FILE__, __LINE__, #condition )) )	\
	{																												\
		C_DBG_ntPrintf condition_msg;																				\
		C_DBG_ntPrintf( "\n" );																						\
		C_DBG_FlushLog();																							\
		C_DBG_DebugBreak();																							\
	}
#else 

#define ntError( condition )	
#define ntError_p( condition, condition_msg )	

#endif // _DEBUG
extern int C_DBG_ntPrintf( const char* restrict pcFormat, ... );
extern int C_DBG_ntAssertPrintf( const char* restrict pcFormat, ... );

extern void C_DBG_FlushLog();
extern void C_DBG_DebugBreak();

#endif // ^^^ simple C version to route external C code through out stuff

// \todo We need a 'blue screen' type screen for this	
//! fatal ntError is the worse kind of ntError, uncontiueable in a MASTER build. This is the worst kind of bad 
#define MASTER_FATAL_MSG( msg ) ntError_p( false, (msg) )
// \todo We need a 'blue screen' type screen for this
#define MASTER_FATAL_ERROR( cnd, condition_msg ) ntError_p( (cnd), (condition_msg) );

// this define makes the loggin faster, but wont cope with a hard reset crash bug.
// If we start getting one, remove this define
#define DISABLE_LOG_CACHES

#ifdef DISABLE_LOG_CACHES
#define DEBUG_LOG_FLAGS	(File::FT_TEXT | File::FT_WRITE)
#else
#define DEBUG_LOG_FLAGS	(File::FT_TEXT | File::FT_WRITE | File::FT_LOG)
#endif

#endif // end include guard
