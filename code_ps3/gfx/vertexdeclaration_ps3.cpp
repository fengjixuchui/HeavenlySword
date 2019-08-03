#include "gfx/vertexdeclaration.h"

//--------------------------------------------------
//!
//!	convert whats the count of fields used in this type
//! this function was moved here cause our compiler sucks
//--------------------------------------------------

int GetStreamElements( VERTEX_DECL_STREAM_TYPE ntType )
{
	switch (ntType)
	{
		case VD_STREAM_TYPE_FLOAT1:
		case VD_STREAM_TYPE_PACKED:
			return 1;
		
		case VD_STREAM_TYPE_FLOAT2:
		case VD_STREAM_TYPE_SHORT2:
		case VD_STREAM_TYPE_SHORT2N:
		case VD_STREAM_TYPE_USHORT2N:
		case VD_STREAM_TYPE_HALF2:
			return 2;
		
		case VD_STREAM_TYPE_FLOAT3:
			return 3;
		
		case VD_STREAM_TYPE_FLOAT4:
		case VD_STREAM_TYPE_UBYTE4:
		case VD_STREAM_TYPE_SHORT4:
		case VD_STREAM_TYPE_UBYTE4N:
		case VD_STREAM_TYPE_SHORT4N:
		case VD_STREAM_TYPE_USHORT4N:
		case VD_STREAM_TYPE_HALF4:
			return 4;

		default:
			ntAssert_p(0,("Unrecognised stream type: %s\n", ntType));
		break;
	}

	return 0;
}


//--------------------------------------------------
//!
//!	convert VERTEX_DECL_STREAM_TYPE to StreamType
//!
//--------------------------------------------------
Gc::StreamType GetStreamType( VERTEX_DECL_STREAM_TYPE ntType )
{
	switch (ntType)
	{
	case VD_STREAM_TYPE_FLOAT1:
	case VD_STREAM_TYPE_FLOAT2:
	case VD_STREAM_TYPE_FLOAT3:
	case VD_STREAM_TYPE_FLOAT4:
		return Gc::kFloat;

	case VD_STREAM_TYPE_PACKED:
		return Gc::kPacked;

	case VD_STREAM_TYPE_UBYTE4:
	case VD_STREAM_TYPE_UBYTE4N:
		return Gc::kUByte;

	case VD_STREAM_TYPE_SHORT2:
	case VD_STREAM_TYPE_SHORT4:
	case VD_STREAM_TYPE_SHORT2N:
	case VD_STREAM_TYPE_SHORT4N:
		return Gc::kShort;

	case VD_STREAM_TYPE_USHORT2N:
	case VD_STREAM_TYPE_USHORT4N:
		return Gc::kUShort;

	case VD_STREAM_TYPE_HALF2:
	case VD_STREAM_TYPE_HALF4:
		return Gc::kHalf;
	}

	ntAssert_p(0,("Unrecognised stream type: %s\n", ntType));
	return Gc::kUInt;
};


//--------------------------------------------------
//!
//! is VERTEX_DECL_STREAM_TYPE normalised?
//!
//--------------------------------------------------
bool IsTypeNormalised( VERTEX_DECL_STREAM_TYPE ntType )
{
	switch(ntType)
	{
	case VD_STREAM_TYPE_FLOAT1:
	case VD_STREAM_TYPE_FLOAT2:
	case VD_STREAM_TYPE_FLOAT3:
	case VD_STREAM_TYPE_FLOAT4:
	case VD_STREAM_TYPE_PACKED:
	case VD_STREAM_TYPE_UBYTE4:
	case VD_STREAM_TYPE_SHORT2:
	case VD_STREAM_TYPE_SHORT4:
	case VD_STREAM_TYPE_HALF2:
	case VD_STREAM_TYPE_HALF4:
		return false;

	case VD_STREAM_TYPE_UBYTE4N:
	case VD_STREAM_TYPE_SHORT2N:
	case VD_STREAM_TYPE_SHORT4N:
	case VD_STREAM_TYPE_USHORT2N:
	case VD_STREAM_TYPE_USHORT4N:
		return true;
	}
	ntAssert_p(0,("Unrecognised stream type: %s\n", ntType));
	return false;
};


// this probably doesn't belong here but we need to convert from the exported
// STREAM_SEMANTIC_TYPE to a hashed name that Gc uses, this does it 
FwHashedString GetSemanticName( STREAM_SEMANTIC_TYPE ntType )
{
	switch( ntType )
	{
	case STREAM_BINORMAL:				return FwHashedString( "binormal" );
	case STREAM_BLENDINDICES:			return FwHashedString( "indices" );
	case STREAM_BLENDWEIGHTS:			return FwHashedString( "weights" );
	case STREAM_DIFFUSE_TEXCOORD0:		return FwHashedString( "diffuse_texcoord0" );
	case STREAM_DIFFUSE_TEXCOORD1:		return FwHashedString( "diffuse_texcoord1" );
	case STREAM_DIFFUSE_TEXCOORD2:		return FwHashedString( "diffuse_texcoord2" );

	case STREAM_GENERIC_TEXCOORD0:		return FwHashedString( "generic_texcoord0" );
	case STREAM_GENERIC_TEXCOORD1:		return FwHashedString( "generic_texcoord1" );
	case STREAM_GENERIC_TEXCOORD2:		return FwHashedString( "generic_texcoord2" );
	case STREAM_GENERIC_TEXCOORD3:		return FwHashedString( "generic_texcoord3" );
	case STREAM_GENERIC_TEXCOORD4:		return FwHashedString( "generic_texcoord4" );
	case STREAM_GENERIC_TEXCOORD5:		return FwHashedString( "generic_texcoord5" );
	case STREAM_GENERIC_TEXCOORD6:		return FwHashedString( "generic_texcoord6" );
	case STREAM_GENERIC_TEXCOORD7:		return FwHashedString( "generic_texcoord7" );
	case STREAM_GENERIC_TEXCOORD8:		return FwHashedString( "generic_texcoord8" );
	case STREAM_GENERIC_TEXCOORD9:		return FwHashedString( "generic_texcoord9" );
	case STREAM_GENERIC_TEXCOORD10:		return FwHashedString( "generic_texcoord10" );
	case STREAM_GENERIC_TEXCOORD11:		return FwHashedString( "generic_texcoord11" );
	case STREAM_GENERIC_TEXCOORD12:		return FwHashedString( "generic_texcoord12" );
	case STREAM_GENERIC_TEXCOORD13:		return FwHashedString( "generic_texcoord13" );
	case STREAM_GENERIC_TEXCOORD14:		return FwHashedString( "generic_texcoord14" );
	case STREAM_GENERIC_TEXCOORD15:		return FwHashedString( "generic_texcoord15" );

	case STREAM_NORMAL_MAP_COORD_SYSTEM: return FwHashedString( "tangent_space_basis" );
	case STREAM_NORMAL:					return FwHashedString( "normal" );
	case STREAM_NORMAL_MAP_TEXCOORD:	return FwHashedString( "normal_map_texcoord" );
	case STREAM_POSITION:				return FwHashedString( "position" );
	case STREAM_TANGENT:				return FwHashedString( "tangent" );
	case STREAM_SPECULAR_COLOUR_METALLICITY_TEXCOORD: return FwHashedString( "specular_colour_map_texcoord" );

	case STREAM_SPEEDTREE_COLOUR:		return FwHashedString( "speedtree_colour" );
	case STREAM_SPEEDTREE_WINDCOEF:		return FwHashedString( "speedtree_windcoef" );
	case STREAM_SPEEDTREE_LEAFCOEF:		return FwHashedString( "speedtree_leafcoef" );
	case STREAM_SPEEDTREE_CLIPINDEX:	return FwHashedString( "speedtree_alphaIndex" );
	case STREAM_SPEEDTREE_CLIPTRESHOLD:	return FwHashedString( "speedtree_alphaTreshold" );

	default:
		ntAssert_p( false, ("%d is not a valid stream semantic\n", ntType) );
		return FwHashedString("");
	}
}
