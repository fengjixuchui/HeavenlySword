//---------------------------------------------------------------
//!
//! \file ntlib\debug_spu.h
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//---------------------------------------------------------------
#if !defined( NTLIB_DEBUG_SPU_H )
#define NTLIB_DEBUG_SPU_H

#include "jobapi/jobprintf.h"
#include "jobsystem/helpers.h"

#define ntBreakpoint()	WWSJOB_BREAKPOINT()


#if !defined( _RELEASE )

#	define ntAssert( condition )											\
		do																	\
		{																	\
			if ( !(condition) )												\
			{																\
				ntPrintf( "%s(%d): Assertion failed: %s\n", __FILE__, __LINE__, #condition );			\
				ntBreakpoint();												\
			}																\
		}																	\
		while ( false )

#	define ntAssert_p( condition, msg )										\
		do																	\
		{																	\
			if ( !(condition) )												\
			{																\
				ntPrintf( "%s(%d): Assertion failed: %s\nMessage:", __FILE__, __LINE__, #condition );	\
				ntPrintf msg ;												\
				ntPrintf( "\n" );											\
				ntBreakpoint();												\
			}																\
		}																	\
		while ( false )
#else

#	define ntAssert( condition ) (void) 0
#	define ntAssert_p( condition, msg ) (void)0

#endif

#define ntPrintf JobPrintf
#define ntError( condition )			ntAssert( condition )
#define ntError_p( condition, msg )		ntAssert_p( condition, msg )

#define static_assert_in_class( expr, msg ) typedef char ERROR_##msg[1][(expr) == 0 ? -1 : 1 ]

#endif // NTLIB_DEBUG_SPU_H

