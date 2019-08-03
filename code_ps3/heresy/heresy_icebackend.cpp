//--------------------------------------------------
//!
//!	\file heresy_icebackend.cpp
//! these 
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
#include <cell/gcm.h>

#include <core/memman.h>
#include <gfx/renderer.h>

#include "heresy_hardware.h"
#include "heresy_capi.h"

//--------------------------------------------------
//!
//! currently just thunk through to ice to let it 
//! do the work
//!
//--------------------------------------------------
void Heresy_SetVertexShader( Heresy_PushBuffer* pPB, const Heresy_VertexShader* pProgram )
{
	Ice::Render::Inline::CommandContext* pIcePB = (Ice::Render::Inline::CommandContext*) pPB;
	const Ice::Render::VertexProgram* pIceProgram = (const Ice::Render::VertexProgram*) pProgram;
	pIcePB->SetVertexProgram( pIceProgram );
}

//--------------------------------------------------
//!
//! currently just thunk through to ice to let it 
//! do the work
//!
//--------------------------------------------------
void Heresy_SetPixelShader( Heresy_PushBuffer* pPB, const Heresy_PixelShader* pProgram )
{
	Ice::Render::Inline::CommandContext* pIcePB = (Ice::Render::Inline::CommandContext*) pPB;
	const Ice::Render::FragmentProgram* pIceProgram = (const Ice::Render::FragmentProgram*) pProgram;
	pIcePB->SetFragmentProgram( pIceProgram );
}

//--------------------------------------------------
//!
//! this needs an SPU version fairly quickly but 
//! until I leave it for a later date when I've 
//! had a think about the whole mess that is fragment
//! shaders on RSX...
//!
//--------------------------------------------------
void Heresy_SetPixelShaderConstant4F( restrict Heresy_PushBuffer* pPB, Heresy_PixelShader* pProgram, uint32_t constantnum, const float* restrict constant )
{
	Ice::Render::Inline::CommandContext* pIcePB = (Ice::Render::Inline::CommandContext*) pPB;
	Ice::Render::FragmentProgram* pIceProgram = (Ice::Render::FragmentProgram*) pProgram;
	pIcePB->SetFragmentProgramConstant( pIceProgram, constantnum, constant );
}


//--------------------------------------------------
//!
//! set up our
//!
//--------------------------------------------------
void Heresy_InitGlobalData( Heresy_GlobalData* pGlobal )
{
	uint32_t iBase = Mem::GetBaseAddress( Mem::MC_RSX_MAIN_INTERNAL );
	uint32_t offset = Ice::Render::TranslateAddressToIoOffset( (void*)iBase );
	pGlobal->m_RSXMainBaseAdjust = iBase - offset;

	// Aquire the GPU's configuration.
	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	pGlobal->m_IceVramOffset = (uint32_t) config.localAddress;

}

//--------------------------------------------------
//!
//! Another one where we need to roll our own homebrew version 
//! pretty damn sharpish
//!
//--------------------------------------------------
uint32_t Heresy_MainRamAddressToRSX( void* pPtr )
{
	// we need to offset from Ice still... to get this at offset 0 (which we want pass the base addr of MC_RSX_MAIN_INTERNAL
	uint32_t iBase = Mem::GetBaseAddress( Mem::MC_RSX_MAIN_INTERNAL );
	uint32_t offset = Ice::Render::TranslateAddressToIoOffset( (void*)iBase );
	return ((((uint32_t)pPtr) - iBase) - offset);
}

uint32_t Heresy_VramRamAddressToRSX( void* pPtr )
{
	return Ice::Render::TranslateAddressToOffset( pPtr );
}

// TODO our version
void* Heresy_AllocatePixelShaderSpaceInGDDR( uint16_t space )
{
	return (void*) Ice::Render::AllocateLinearVideoMemory( space );
}

uint32_t Heresy_DummyCommandFillCallback( void*, uint32_t )
{
	return 0;
}

// Patch for performance on 083
extern "C"
{
	void nvgleFifoWaitForFreeSpaceDefaultArgs()
	{
	}
}
