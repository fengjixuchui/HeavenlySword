//--------------------------------------------------------------------------------------------------
/**
	@file		FwSwitches.h
	
	@brief		Framework switches, associated with build types

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_SWITCHES_H
#define	FW_SWITCHES_H

//--------------------------------------------------------------------------------------------------
// If we don't have any build type defined, then it's likely we're being included from
// tools, so in this case we define our build type based on the VS.NET standard defines.
// The nice part of this scheme is that if a tool decides that they want, for example, a 
// profile build, they can just define ATG_PROFILE_BUILD in their VS.NET settings and this
// catch-all processing gets silently skipped.

#if	!defined(ATG_DEBUG_BUILD) && !defined(ATG_DEVELOPMENT_BUILD) && !defined(ATG_PROFILE_BUILD) && !defined(ATG_RELEASE_BUILD)
	#ifdef	_DEBUG
		#define	ATG_DEBUG_BUILD
	#else
		#define	ATG_RELEASE_BUILD
	#endif	//_DEBUG
#endif

//--------------------------------------------------------------------------------------------------
// Default to PC platform if none defined

#if !defined(ATG_PC_PLATFORM) && !defined(ATG_PS3_PLATFORM)
	#define ATG_PC_PLATFORM
#endif

//--------------------------------------------------------------------------------------------------

#ifdef	ATG_DEBUG_BUILD
	#define	ATG_DEBUG_MODE
	#define	ATG_ASSERTS_ENABLED
	#define	ATG_PRINTF_ENABLED
	#define	ATG_PROFILE_ENABLED
	#define	ENABLE_VECTOR_QUADWORD_VALIDATION
#endif	//ATG_DEBUG_BUILD

#ifdef	ATG_DEVELOPMENT_BUILD
	#define	ATG_DEBUG_MODE
	#define	ATG_ASSERTS_ENABLED
	#define	ATG_PRINTF_ENABLED
	#define	ATG_PROFILE_ENABLED
#endif	//ATG_DEVELOPMENT_BUILD

#ifdef	ATG_PROFILE_BUILD
	#define	ATG_PROFILE_ENABLED
#endif	//ATG_PROFILE_BUILD

#ifdef	ATG_RELEASE_BUILD

#endif	//ATG_RELEASE_BUILD

//--------------------------------------------------------------------------------------------------
// Platform Specifiers. We define ATG_PLATFORM with a quoted string, so that it can be used as a
// constant value within normal C/C++ string operations. Neither GCC or Visual Studio support the
// passing of quoted strings into macros defined on the compiler command line. Hence this being here.

#ifdef	ATG_PC_PLATFORM
#define	ATG_PLATFORM	"PC"
#endif	//ATG_PC_PLATFORM

#ifdef	ATG_PS3_PLATFORM
#define	ATG_PLATFORM	"PS3"
#endif	//ATG_PS3_PLATFORM


#endif	//FW_SWITCHES_H
