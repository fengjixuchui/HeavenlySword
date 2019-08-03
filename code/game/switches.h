/***************************************************************************************************
*
*   $Header:: /game/switches.h 22    14/08/03 10:48 Dean                                           $
*
*	Compilation switches. Don't add anything other than *switches* to this file.
*
*	CHANGES		
*
*	26/03/2001	dean	Created
*
***************************************************************************************************/

#ifndef	_SWITCHES_H

//#define _MASTER

#if defined( _MASTER ) && defined( _RELEASE )
#define _GOLD_MASTER
#endif

//--------------------------------------------------------------------------------------------------
// Build Identifier
//--------------------------------------------------------------------------------------------------

#ifndef	_BUILD_ID
#define	_BUILD_ID			"Internal"
#define	_BUILD_IS_INTERNAL
#endif	//_BUILD_ID

// We're using Lua Plus as a library.. 
#define	LUAPLUS_LIB

// Master builds should run on test-kits (192MB XDDR etc...).
#ifndef _MASTER
#	define _HAVE_DEBUG_MEMORY
#endif

// for getting rid of debug code that uses unavailable memory
#if !defined( _HAVE_DEBUG_MEMORY ) || defined( _RELEASE )
#define _NO_DBGMEM_OR_RELEASE
#endif

// for now we put profiling in all builds including release, but not master, as we require chuncks of debug memory
#ifndef _MASTER
#	define _PROFILING
#endif

#ifndef _GOLD_MASTER
#	define TRACK_GFX_MEM
#endif

//--------------------------------------------------------------------------------------------------
// Global debug states
//--------------------------------------------------------------------------------------------------

#ifdef	_BUILD_IS_INTERNAL
#define _BOUND_CHECK					// Enables asserts on dynamics values out of range
#endif

#ifdef	_DEBUG
//#define	_ENABLE_FP_EXCEPTIONS
//#define	LUA_DEBUG_MEMORY
#endif	//_DEBUG

#ifndef _GOLD_MASTER
#	define	DEBUG_KEYBOARD					// Used to enable debug keyboard support
#endif

//--------------------------------------------------------------------------------------------------
// Gatso Options
//--------------------------------------------------------------------------------------------------

#ifdef _PROFILING
#define	_GATSO							// Enable to allow gatso timing routines
#define	_GATSO_NO_SAFETY_CHECKS			// Enable to prevent gatso validation asserts
#endif

#if defined( PLATFORM_PC )
//--------------------------------------------------------------------------------------------------
// PC specific switches
//--------------------------------------------------------------------------------------------------
	#define D3DCOMPILE_PUREDEVICE 1			// Use PC D3D fast path
	
	#ifdef _WIN32
		#define WIN32_LEAN_AND_MEAN
	#endif // _WIN32

	#define	_USE_SSE						// Enables use of real Pentium/Athlon XP SSE instructions

#elif defined( PLATFORM_PS3 )
//--------------------------------------------------------------------------------------------------
// PS3 specific switches
//--------------------------------------------------------------------------------------------------
	#define _NO_DSOUND						// Disable direct sound
	
	// current ntPrintf bad mmmokay
	#undef _PRINTF
	
	// A PCism used ALOT
	#define MAX_PATH 512
	#define __FUNCTION__ "Unknown Function"

	#ifndef _SPEEDTREE
	#define _SPEEDTREE
	#endif

#else
	#error No PLATFORM_XXXX macro defined
#endif


//--------------------------------------------------------------------------------------------------
// Physics engine options
//--------------------------------------------------------------------------------------------------
//#define _HAVOK

//--------------------------------------------------------------------------------------------------
// Movement options
//--------------------------------------------------------------------------------------------------

#ifndef _MASTER

//#	define _LOGMOVEMENT

#	if	defined(_DEBUG) || defined(USER_Mike)
#	define ATTACK_PRINTF
#	endif

#	if	defined(_DEBUG) || defined(USER_Mike)
#	define INTERACTIVEMUSIC_PRINTF
#	endif

#	if	defined(_DEBUG) || defined(USER_JohnL)
//#	define CAMERA_PRINTF
#	endif

#	if defined(USER_JohnL) || defined(USER_gavin)
#	define MSVS_CLICK_FRIENDLY
#	endif

#endif // !_MASTER

//--------------------------------------------------------------------------------------------------
// Audio options
//--------------------------------------------------------------------------------------------------
#define _AUDIO_SYSTEM_ENABLE


#endif	//_SWITCHES_H
