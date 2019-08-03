/***************************************************************************************************
*
*	DESCRIPTION		The entry point for the flag update system on SPUs.
*
*	NOTES
*
***************************************************************************************************/

#include "hair_animator_spu.h"
#include "ntlib_spu/fixups_spu.h"
#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP();

//**************************************************************************************
//	Entry point for the code on this SPU.
//**************************************************************************************
extern "C" void SpuMain( SPUArgumentList &params )
{
	INSERT_PA_BOOKMARK ( PABM_SPU_HAIR );

	// binding, mind the order
	GetArrayInput( ChainSPU::OneChain *, pOneChain, 0 );
	GetArrayInput( ChainSPU::MetaBallGroup *, pMetaBallGroup, 1 );
	GetArrayInput( ChainSPU::Global *, pGlobal, 2 );
	
	// print begin
	if(pGlobal->m_flags[ChainSPU::Global::DEBUG_BEGINEND]) ntPrintf("HAIR:SpuMain BEGIN\n");

	// breakpoint
	if(pGlobal->m_flags[ChainSPU::Global::DEBUG_BREAKPOINT]) ntBreakpoint();
	
	// do the job
	ChainSPU::ProcessOneChain(pOneChain, *pMetaBallGroup, *pGlobal);

	// print end
	if(pGlobal->m_flags[ChainSPU::Global::DEBUG_BEGINEND]) ntPrintf("HAIR:SpuMain END\n");
}





