//--------------------------------------------------
//!
//!	\file heresy_verify.cpp
//! verify and checks our assumption (of which there
//! are lots due to the nature of the heresy project)
//!
//--------------------------------------------------
#include "core/debug.h"

/*
#include <system/icebase.h>
#include <system/icetypes.h>
#include <render/icerender.h>
#include <render/icecommandcontext.h>
*/
#include <Gc/Gc.h>

#include "heresy_hardware.h"
#include "heresy_capi.h"

void Heresy_Verify()
{
	// Ice have a single 4 byte constant to mark the array I don't
	static_assert( sizeof(Heresy_VertexShader)+sizeof(uint32_t) == sizeof( Ice::Render::VertexProgram ) , HeresyVP_Ice_Mismatch);
	static_assert( sizeof(Heresy_PixelShader) == sizeof( Ice::Render::FragmentProgram ) , HeresyFP_Ice_Mismatch);
	static_assert( sizeof(Heresy_Texture) == sizeof( Ice::Render::Texture ) , HeresyTex_Ice_Mismatch);

}
