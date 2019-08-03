#if !defined(GFX_DXERROR_PC_H)
#define GFX_DXERROR_PC_H
//-----------------------------------------------------
//!
//!	\file gfx\ntError.h
//! Bunch of macros to spit out sensible ntError messages
//! Totally platform specific of course.
//!
//-----------------------------------------------------

#if !defined( _RELEASE )

#if !defined( STD_DXERR9_H )
#include <dxerr9.h>
#define STD_DXERR9_H
#endif

//! Asserts hr passed o.k. and display user message even on Release builds
#define dxerror_p( hr, condition_msg )															\
	{ HRESULT res = (hr);																		\
		if ( FAILED(res) )																		\
		{																						\
			Debug::Printf( Debug::DCU_GRAPHICS, "%s(%d) : D3D Error :", __FILE__, __LINE__);			\
			Debug::Printf( Debug::DCU_GRAPHICS, "%s : %s : ", ::DXGetErrorString9(res), ::DXGetErrorDescription9(res) );	\
			Debug::Printf condition_msg;																\
			Debug::Printf( Debug::DCU_GRAPHICS, "\n" );												\
			Debug::FlushLog();																	\
			DebugBreakNow();																\
		}																						\
	}

//! Asserts hr passed o.k even on Release builds
#define dxerror( hr )																			\
	{ HRESULT res = (hr);																		\
		if ( FAILED(res) )																		\
		{																						\
			Debug::Printf( Debug::DCU_GRAPHICS, "%s(%d) : D3D Error :", __FILE__, __LINE__);			\
			Debug::Printf( Debug::DCU_GRAPHICS, "%s : %s : ", ::DXGetErrorString9(res), ::DXGetErrorDescription9(res) );		\
			Debug::Printf( Debug::DCU_GRAPHICS, "\n" );												\
			Debug::FlushLog();																	\
			DebugBreakNow();																\
		}																						\
	}

#else

//! Asserts hr passed o.k. and display user message even on Release builds
#define dxerror_p( hr, condition_msg )	(hr)

//! Asserts hr passed o.k. and display user message even on Release builds
#define dxerror( hr )	(hr)

#endif // end _RELEASE


#endif // end GFX_DXERROR_PC_H
