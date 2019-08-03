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
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"

#include "anim/animation.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "core/exportstruct_keyframe.h"

#include "animation_spu.h"

SPU_MODULE_FIXUP()

//**************************************************************************************
//	Entry point for the code on this SPU.
//**************************************************************************************
extern "C" void SpuMain( SPUArgumentList &params )
{
	GetArrayInput( CAnimator_DMASection *, animator, 0 );
	GetArrayInput( const int8_t *, char_bone_to_index_array, 1 );
	GetU32Input( largest_anim_size, 2 );
	GetU32Input( largest_header_size, 3 );
	GetU32Input( num_transforms, 4 );
	GetArrayInput( v128 *, bind_pose_array, 5 );
	GetU32Input( transform_array_ea, 6 );
	GetU32Input( transform_array_stride, 7 );

	ntError( (intptr_t)animator > 0 && (intptr_t)animator < 0x30000 );
	ntError( (intptr_t)char_bone_to_index_array > 0 && (intptr_t)char_bone_to_index_array < 0x30000 );
	ntError( largest_anim_size > 0 );
	ntError( largest_header_size > 0 );
	ntError( num_transforms > 0 );
	ntError( (intptr_t)bind_pose_array > 0 && (intptr_t)bind_pose_array < 0x30000 );
	ntError( transform_array_ea > 0 );
	ntError( transform_array_stride > 0 );

	largest_anim_size = ( largest_anim_size & 0xfffffff0 ) + 0x10;
	largest_header_size = ( largest_header_size & 0xfffffff0 ) + 0x10;

	SPUAnim::CAnimator_Update(	animator, num_transforms, transform_array_ea, transform_array_stride,
								bind_pose_array, char_bone_to_index_array, largest_anim_size, largest_header_size );
}





