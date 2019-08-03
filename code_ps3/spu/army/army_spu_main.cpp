/***************************************************************************************************
*
*	DESCRIPTION		The entry point for the army update system on SPUs.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"

#include "army/army_ppu_spu.h"
#include "army/battlefield.h"
#include "army/battalion.h"
#include "army/grunt.h"
#include "army/unit.h"

#include <stdlib.h> // for qsort

#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP()

// prototypes
void GruntChunkifyAndSort( SPUArgumentList &params );
void BattalionUpdate( SPUArgumentList &params );

//**************************************************************************************
//	Entry point for the code on this SPU.
//**************************************************************************************
extern "C" void SpuMain( SPUArgumentList &params )
{
	INSERT_PA_BOOKMARK ( PABM_SPU_ARMY )

	GetU32Input( runType, SRTA_RUN_TYPE );

	switch( runType )
	{
	case BATTALION_UPDATE:
		BattalionUpdate( params );			break;
	case GRUNT_CHUNKIFY_AND_SORT:
		GruntChunkifyAndSort( params );		break;
	default:
		ntAssert( false );
	};
}
