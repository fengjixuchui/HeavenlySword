/***************************************************************************************************
*
*	$Header:: /game/pch.h 27    19/08/03 13:06 Dean                                                $
*
*	KFM Prototype Precompiled Header
*
*	CHANGES
*
*	5/3/2003	Simon Created
*
***************************************************************************************************/

#ifndef _PCH_H
#define _PCH_H

// We can't have asserts defined as the default is to call exit().
#ifdef assert
#	undef assert
#endif
#define assert ntAssert

// Project switches
#include "switches.h"

// we need these for lua
#if defined( PLATFORM_PS3 )
	#include <wchar.h>
	#include <wctype.h>

	#define	stricmp		std::strcasecmp
	#define strnicmp	std::strncasecmp

	// be aware that on PS3, wchar_t is a signed 32bit quantity.
	typedef	wchar_t	WCHAR_T;
	
#elif defined( PLATFORM_PC )
	#ifdef __cplusplus
		#if !defined( _HAS_EXCEPTIONS )
			#define _HAS_EXCEPTIONS 0
		#endif // end !HAS_EXCEPTIONS
		// dummy exception to keep visual studio broken headers happy
		class exception{};
	#endif // end __cplusplus

	#define _WIN32_WINNT 0x0500
	#define WINVER 0x0500
	#define WIN32_LEAN_AND_MEAN

	// Windows headers
	#include <windows.h>
	// Common macros
	#undef min
	#undef max
	// for LuaPlus
	#undef LoadString

	#undef GetFreeSpace

	// on PC wchar_t is an unsigned 16bit quantity.
	typedef	wchar_t		WCHAR_T;			

#endif // end PLATFORM_PC

// include our base types
#include "core/basetypes.h"
#include "core/debug.h"

#ifdef __cplusplus

// include ATG framework header
#include <Fw/Fw.h>
#include "core/mem.h"
#include "core/nt_std.h"

#if defined( PLATFORM_PC )

	// DirectX headers
	// DX9 Summer 2004 has extra runtime debug info if this is defined
	#ifndef _RELEASE
		#define D3D_DEBUG_INFO
	#endif
	#include <d3d9.h>
	#include <d3dx9shader.h>

	// Common pragmas
	#pragma warning( default : 4100 )		// 'unreferenced formal parameter'
	#pragma warning( disable : 4127 )		// 'conditional expression is constant'
	#pragma warning( disable : 4239 )		// 'nonstandard extension used : <blah>'
	#pragma warning( disable : 4324 )		// 'structure was padded due to __declspec(align())'
	#pragma warning( disable : 4511 4512 )	// 'copy constructor/assignment operator could not be generated'
	#pragma warning( disable : 4514 )		// 'unreferenced inline removed'
	#pragma warning( disable : 4702 )		// 'unreachable code'
	#pragma warning( disable : 4714 )		// 'function marked as __forceinline not inlined'
	#pragma warning( disable : 4505 )		// 'unreferenced local function removed'
	#pragma warning( disable : 4291 )		// 'no matching operator delete found' placement new warning

#elif defined( PLATFORM_PS3 )

#endif


// System Limits
#define	VRAM_LIMIT	( 224 * 1024 * 1024 )
#define	RAM_LIMIT	( 192 * 1024 * 1024 )

// Common types
typedef unsigned long   u_long;			// 4-byte unsigned integer
typedef unsigned int	u_int;			// 4-byte unsigned integer
typedef unsigned short	u_short;		// 2-byte unsigned integer
typedef unsigned char	u_char;			// a single unsigned byte


// System code: asserts, singleton, smart pointers, pool, list
#include "core/util.h"

#include "core/singleton.h"
#include "core/smartptr.h"
#include "core/hash.h"
#include "core/stringutil.h"

// Maths classes
#include "core/maths.h"
#include "core/vecmath.h"
#include "core/keystring.h"

#endif // __cplusplus

#endif // _PCH_H
