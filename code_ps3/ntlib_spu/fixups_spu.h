//----------------------------------------------------------------------------------------
//! 
//! \filename ntlib_spu\fixups.h
//! 
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//----------------------------------------------------------------------------------------
#ifndef FIXUPS_SPU_H_
#define FIXUPS_SPU_H_

#ifndef __SPU__
#	error This file is for inclusion in SPU projects ONLY!
#endif // !__SPU__

#include "jobapi/jobapi.h"
#include "ntlib_spu\util_spu.h"
#include "ntlib_spu\debug_spu.h"
#include "ntlib_spu\ntDma.h"
#include "exec\ppu\spuargument_ps3.h"

#define SPU_MODULE_FIXUP()\
	extern "C" void JobMain() { SPUFixups::RunJob(); }

//
//	Declare and initialise an SPU program input/output equivalent to:
//
//		type var = /* initialise code */;
//
#define GetArrayOutput( type, var, slot )	type var = static_cast< type >( params.Get( slot )->GetBuffer()->GetLS() ); UNUSED( var );
#define GetArrayInput( type, var, slot )	type var = static_cast< type >( params.Get( slot )->GetBuffer()->GetLS() ); UNUSED( var );
#define GetU64Input( var, slot ) 			uint64_t var = params.Get( slot )->GetU64(); UNUSED( var );
#define GetU32Input( var, slot ) 			uint32_t var = params.Get( slot )->GetU32(); UNUSED( var );
#define GetFloatInput( var, slot )			float var = params.Get( slot )->GetFloat(); UNUSED( var );

namespace SPUFixups
{
	void	RunJob	();
}

// Allocate some memory.void *Allocate( uint32_t length_in_bytes );


// if you need the EA of input DMA parameters, you can retrieve them from here
extern uint32_t g_DMAEffectiveAddresses[ SPUArgumentList::MaxNumArguments ];

#endif // !FIXUPS_SPU_H_
