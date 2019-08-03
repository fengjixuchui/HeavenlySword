//---------------------------------------------------------------
//!
//! \file ntlib\basetypes_spu.h
//!  The basic type information header for platform indepedence.
//!  Setups most of the C99 types for sized int, restrict etc.
//!  Types here are intentionally placed into global namespace
//!  This is the SPU version of these types
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
#if !defined( NTLIB_BASETYPES_SPU_H )
#define NTLIB_BASETYPES_SPU_H


/// For WCHAR_T and size_t among others
#include <stddef.h>
/// PS3 has a full C99 stdint which is nice...
#include <stdint.h>

#include <inttypes.h>
#include <spu_intrinsics.h>
								
#define restrict			__restrict				//!< restrict keyword

typedef qword uint128_t;

#if !defined(UNUSED)
# define UNUSED(X)				(void) X
#endif

static const size_t POINTER_SIZE_IN_BYTES = sizeof( intptr_t );

#endif // NTLIB_BASETYPES_SPU_H
