//---------------------------------------------------------------
//!
//! \file ntlib\basetypes_spu.h
//!  The basic type information header for platform indepedence.
//!  Setups most of the C99 types for sized int, restrict etc.
//!  Types here are intentionally placed into global namespace
//!  This is the SPU version of these types
//!
//---------------------------------------------------------------

#ifndef CORE_BASETYPES_PS3_H_
#define CORE_BASETYPES_PS3_H_

#define __STD_USING
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

/// For WCHAR_T and size_t among others
#include <stddef.h>
/// PS3 has a full C99 stdint which is nice...
#include <stdint.h>

#define restrict __restrict

// add uint128_t, just to keep the code pretty
typedef vector unsigned int uint128_t;

#endif // CORE_BASETYPES_H_
