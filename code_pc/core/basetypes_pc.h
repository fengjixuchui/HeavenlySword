#if !defined( CORE_BASETYPES_PC_H )
#define CORE_BASETYPES_PC_H

//---------------------------------------------------------------
//!
//! \file core\basetypes.h
//!  The basic type information header for platform indepedence.
//!  Setups most of the C99 types for sized int, restrict etc.
//!  Types here are intentionally placed into global namespace
//!  This is the PC version of these types
//!
//---------------------------------------------------------------

// VC 7.1 doesn't yet define all the C99 types so we manually fix them up here
// Win32 is 32 bit archtecture so MAXINT = 32bits there is a 64 bit type
// but some math operations aren't supported natively
typedef __int8 				int8_t;			//!< exact 8 bit int
typedef __int16				int16_t;		//!< exact 16 bit int
typedef __int32				int32_t;		//!< exact 32 bit int
typedef __int64 			int64_t;		//!< exact 64 bit int
typedef unsigned __int8 	uint8_t;		//!< exact 8 bit unsigned int
typedef unsigned __int16	uint16_t;		//!< exact 16 bit unsigned int
typedef unsigned __int32	uint32_t;		//!< exact 32 bit unsigned int
typedef unsigned __int64 	uint64_t;		//!< exact 64 bit unsigned int

// least are exact
typedef int8_t 				int_least8_t;	//!< at least 8 bit int
typedef int16_t 			int_least16_t;	//!< at least 16 bit int
typedef int32_t 			int_least32_t;	//!< at least 32 bit int
typedef int64_t 			int_least64_t;	//!< at least 64 bit int
typedef uint8_t 			uint_least8_t;	//!< at least 8 bit unsigned int
typedef uint16_t 			uint_least16_t;	//!< at least 16 bit unsigned int
typedef uint32_t 			uint_least32_t;	//!< at least 32 bit unsigned int
typedef uint64_t 			uint_least64_t;	//!< at least 64 bit unsigned int
	
// fast are min 32 bit on PC
typedef int32_t 			int_fast8_t;	//!< fast 8 bit int
typedef int32_t 			int_fast16_t;	//!< fast 16 bit int
typedef int32_t 			int_fast32_t;	//!< fast 32 bit int
typedef int64_t 			int_fast64_t;	//!< fast 64 bit int
typedef uint32_t 			uint_fast8_t;	//!< fast 8 bit unsigned int
typedef uint32_t 			uint_fast16_t;	//!< fast 16 bit unsigned int
typedef uint32_t 			uint_fast32_t;	//!< fast 32 bit unsigned int
typedef uint64_t 			uint_fast64_t;	//!< fast 64 bit unsigned int

/* already defined in stddef.h -	maybe so, but stddef.h isn't included anywhere apart from within Fw.h
									which is too late in related to some other stuff. I've included
									stddef.h at the bottom of this file - see note there. [ARV]
// 32 bit pointer and is maximum native type
typedef int32_t 			intptr_t;		//!< integer that can holder a pointer
typedef uint32_t 			uintptr_t;		//!< unsigned integer that can holder a pointer
typedef intptr_t			ptrdiff_t;			//!< difference in pointer is the same as the pointer itself
*/

typedef int32_t				intmax_t;		//!< integer of maximum size
typedef uint32_t			uintmax_t;		//!< unsigned integer of maximum size

// integer constant macros (VC gets i8 and ui16 min wrong hence to cast!) 
#define INT8_C(x)			int8_t(x)		//!< user defined constant of type int8_t
#define UINT8_C(x)			uint8_t(x)		//!< user defined constant of type uint8_t
#define INT16_C(x)			int16_t(x)		//!< user defined constant of type int8_t
#define UINT16_C(x)			uint16_t(x)		//!< user defined constant of type uint16_t
#define INT32_C(x)			x##i32			//!< user defined constant of type int32_t
#define UINT32_C(x)			x##Ui32			//!< user defined constant of type uint32_t
#define INT64_C(x)			x##i64			//!< user defined constant of type int64_t
#define UINT64_C(x)			x##Ui64			//!< user defined constant of type uint64_t
#define INTMAX_C(x) 		x##i64			//!< user defined constant of type intmax_t
#define UINTMAX_C(x) 		x##Ui64			//!< user defined constant of type uintmax_t

// limit of specified width integer types
#define INT8_MIN			INT8_C( -128 )						//!< minimum int8_t number
#define INT8_MAX			INT8_C( 127 )						//!< maximum int8_t number
#define UINT8_MAX			UINT8_C( 255 )						//!< maximum uint8_t number
#define INT16_MIN			INT16_C( -32768 )					//!< minimum int16_t number
#define INT16_MAX			INT16_C( 32767 )					//!< maximum int16_t number
#define UINT16_MAX			UINT16_C( 65535 )					//!< maximum uint16_t number
#define INT32_MIN			INT32_C( -2147483648 )				//!< minimum int32_t number
#define INT32_MAX			INT32_C( 2147483647 )				//!< maximum int32_t number
#define UINT32_MAX			UINT32_C( 4294967295 )				//!< maximum uint32_t number
#define INT64_MIN			INT64_C( -9223372036854775808 )	 	//!< minimum int64_t number
#define INT64_MAX			INT64_C( 9223372036854775807 )	 	//!< maximum int64_t number
#define UINT64_MAX			UINT64_C( 18446744073709551615 ) 	//!< maximum uint64_t number

#define INT_LEAST8_MIN		INT8_MIN			//!< minimum int_least8_t number
#define INT_LEAST8_MAX		INT8_MAX			//!< maximum int_least8_t number
#define UINT_LEAST8_MAX		UINT8_MAX			//!< maximum uint_least8_t number
#define INT_LEAST16_MIN		INT16_MIN			//!< minimum int_least16_t number
#define INT_LEAST16_MAX		INT16_MAX			//!< maximum int_least16_t number
#define UINT_LEAST16_MAX	UINT16_MAX			//!< maximum uint_least16_t number
#define INT_LEAST32_MIN		INT32_MIN			//!< minimum int_least32_t number
#define INT_LEAST32_MAX		INT32_MAX			//!< maximum int_least32_t number
#define UINT_LEAST32_MAX	UINT32_MAX			//!< maximum uint_least32_t number
#define INT_LEAST64_MIN		INT64_MIN			//!< minimum int_least64_t number
#define INT_LEAST64_MAX		INT64_MAX			//!< maximum int_least64_t number
#define UINT_LEAST64_MAX	UINT64_MAX			//!< maximum uint_least64_t number
												
#define INT_FAST8_MIN		INT32_MIN			//!< minimum int_fast8_t number
#define INT_FAST8_MAX		INT32_MAX			//!< maximum int_fast8_t number
#define UINT_FAST8_MAX		UINT32_MAX			//!< maximum uint_fast8_t number
#define INT_FAST16_MIN		INT32_MIN			//!< minimum int_fast16_t number
#define INT_FAST16_MAX		INT32_MAX			//!< maximum int_fast16_t number
#define UINT_FAST16_MAX		UINT32_MAX			//!< maximum uint_fast16_t number
#define INT_FAST32_MIN		INT32_MIN			//!< minimum int_fast32_t number
#define INT_FAST32_MAX		INT32_MAX			//!< maximum int_fast32_t number
#define UINT_FAST32_MAX		UINT32_MAX			//!< maximum uint_fast32_t number
#define INT_FAST64_MIN		INT64_MIN			//!< minimum int_fast64_t number
#define INT_FAST64_MAX		INT64_MAX			//!< maximum int_fast64_t number
#define UINT_FAST64_MAX		UINT64_MAX			//!< maximum uint_fast64_t number

// 32 bit pointer, 32 bit max native integer
#define INTPTR_MIN			INT32_MIN			//!< minimum intptr_t number
#define INTPTR_MAX			INT32_MAX			//!< maximum intptr_t number
#define	UINTPTR_MAX			UINT32_MAX			//!< maximum uintptr_t number
#define INTMAX_MIN			INT32_MIN			//!< minimum intmax_t number
#define INTMAX_MAX			INT32_MAX			//!< maximum intmax_t number
#define UINTMAX_MAX			UINT32_MAX			//!< maximum uintmax_t number
												
// PC don't stupport restrict keyword yet
#define restrict								//!< restrict keyword

// I've included this here so we have intptr_t defined at about the same time as
// the other std typedefs. Otherwise the types intptr_t, uintptr_t and ptrdiff_t
// aren't defined until Fw.h is included in pch.h.
// If we don't want to include this here we could define our own typedefs for these
// three types and #define _INTPTR_T_DEFINED, _UINTPTR_T_DEFINED and _PTRDIFF_T_DEFINED
// so that stddef.h doesn't get confused when Fw.h includes it later on. [ARV].
#include <stddef.h>

#endif
