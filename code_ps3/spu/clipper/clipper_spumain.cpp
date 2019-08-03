/***************************************************************************************************
*
*	DESCRIPTION		The entry point for the animation update system on SPUs.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <basetypes_spu.h>
#include <debug_spu.h>

#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/boundingvolumes_spu.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/fixups_spu.h"

#include "clipper_dmadata.h"

#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP();

using namespace ClipperData;

namespace ClipperSPU
{
	extern void Process(DMA_In*, DMA_Out*);
}

//**************************************************************************************
//	Entry point for the code on this SPU.
//**************************************************************************************
extern "C" void SpuMain( SPUArgumentList &params )
{
	INSERT_PA_BOOKMARK ( PABM_SPU_CLIPPER );

	GetArrayInput( DMA_In*, Input, INPUT_SLOT );
	GetArrayOutput( DMA_Out*, Output, OUTPUT_SLOT );

	ClipperSPU::Process(Input, Output);
}
