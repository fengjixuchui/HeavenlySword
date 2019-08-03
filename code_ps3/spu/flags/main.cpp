/***************************************************************************************************
*
*	DESCRIPTION		The entry point for the flag update system on SPUs.
*
*	NOTES
*
***************************************************************************************************/

#include "flags.h"

#include "ntlib_spu/fixups_spu.h"
#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP();

//**************************************************************************************
//	Entry point for the code on this SPU.
//**************************************************************************************
extern "C" void SpuMain( SPUArgumentList &params )
{
	INSERT_PA_BOOKMARK ( PABM_SPU_FLAGS );

	FlagDynamicArray dynamicArray;

	GetArrayInput( const FlagIn *, pFlagIn, 0 );
	dynamicArray.m_pGridDynamicCurrent = (PointDynamic *)( params.Get( 1 )->GetBuffer()->GetLS() );
	dynamicArray.m_pGridDynamicBefore = (const PointDynamic*)( params.Get( 2 )->GetBuffer()->GetLS() );
	dynamicArray.m_pGridDynamicEvenBefore = (const PointDynamic*)( params.Get( 3 )->GetBuffer()->GetLS() );
	GetArrayOutput( FlagOut *, pFlagOut, 4 ); 
	GetArrayOutput( VertexDynamic *, pMesh, 5 ); 
	
	// breakpoint
	if(pFlagIn->m_flags[BREAKPOINT])
	{
		ntBreakpoint();
	}
	
	if (pFlagIn->m_global.m_fDeltaTime <= 0.00001f)
	{
		Code::CopyPoints(*pFlagIn, dynamicArray.m_pGridDynamicCurrent, dynamicArray.m_pGridDynamicBefore);
	}
	else
	{
		// computation
		Code::ApplyForces(*pFlagIn, dynamicArray.m_pGridDynamicCurrent, dynamicArray.m_pGridDynamicBefore, dynamicArray.m_pGridDynamicEvenBefore, pMesh);
		for(uint16_t step = 0 ; step < pFlagIn->m_uiNbStep ; ++step )
		{
			Code::SatisfyConstraint(*pFlagIn, dynamicArray.m_pGridDynamicCurrent);
		}
	}
	Code::GenerateMesh(*pFlagIn, dynamicArray.m_pGridDynamicCurrent, pMesh, pFlagOut);
}





