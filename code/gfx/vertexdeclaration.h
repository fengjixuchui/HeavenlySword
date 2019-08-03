//--------------------------------------------------
//!
//!	\file vertexdeclaration.h
//!	Describes what a vertex is made up of
//!
//--------------------------------------------------

#ifndef GFX_VERTEXDECLARATION_H
#define GFX_VERTEXDECLARATION_H

enum VERTEX_DECL_STREAM_TYPE
{
	VD_STREAM_TYPE_FLOAT1,
	VD_STREAM_TYPE_FLOAT2,
	VD_STREAM_TYPE_FLOAT3,
	VD_STREAM_TYPE_FLOAT4,
	VD_STREAM_TYPE_PACKED,

	VD_STREAM_TYPE_UBYTE4,
	VD_STREAM_TYPE_SHORT2,
	VD_STREAM_TYPE_SHORT4,

	VD_STREAM_TYPE_UBYTE4N,
	VD_STREAM_TYPE_SHORT2N,
	VD_STREAM_TYPE_SHORT4N,

	VD_STREAM_TYPE_USHORT2N,
	VD_STREAM_TYPE_USHORT4N,

	VD_STREAM_TYPE_HALF2,
	VD_STREAM_TYPE_HALF4,
};

#if defined( PLATFORM_PC )
#	include "gfx/vertexdeclaration_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/vertexdecleration_ps3.h"
#endif

#endif // end GFX_TEXTURE_H
