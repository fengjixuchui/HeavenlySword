#include <basetypes_spu.h>
#include <debug_spu.h>


extern "C" void SpuMain( const uint128_t* pParamsArray )
{
	const float*	pInputArray		= (const float*) si_to_uint( pParamsArray[0] );
	float*			pOutputArray	= (float*) si_to_uint( pParamsArray[1] );

	ntPrintf( "Hello, World\nInputArray %x Output Array %x\n", pInputArray, pOutputArray );
}
