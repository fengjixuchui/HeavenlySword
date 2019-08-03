//--------------------------------------------------
//!
//!	\file bsskin_parameters.h
//!	This are the standarised names used internally by BSSkin
//!
//--------------------------------------------------


#ifndef _BSSKIN_SEMANTIC_NAMES_H_
#define _BSSKIN_SEMANTIC_NAMES_H_


////////////////////////////////////////////////////////////////////////
//
//						standard game names
//
////////////////////////////////////////////////////////////////////////

////
//// varying
////
//static const char* BSSKIN_POSITION						= "position";						// float4
//static const char* BSSKIN_NORMAL						= "normal";							// float3
//static const char* BSSKIN_TANGENT						= "tangent";						// float3
//static const char* BSSKIN_BINORMAL						= "binormal";						// float3
//static const char* BSSKIN_TEXCOORD0						= "diffuse_texcoord0";				// float2 ( same for all textures )
//
////
//// uniform
////
//static const char* BSSKIN_FILL_SH						= "fill_sh";						// float4x4[3]			
//static const char* BSSKIN_VIEWPOS						= "view_position";					// float3
//static const char* BSSKIN_LIGHTDIR						= "key_dir";						// float3
//static const char* BSSKIN_LIGHTCOLOUR					= "key_dir_colour";					// float3
//static const char* BSSKIN_WORLDMATRIX					= "world_transform";
//static const char* BSSKIN_VIEWMATRIX					= "view_transform";
//static const char* BSSKIN_MODELVIEWPROJMATRIX			= "projection";					// float4x4
//
//


////////////////////////////////////////////////////////////////////////
//
//						bsskin specific
//	
////////////////////////////////////////////////////////////////////////

//
// uniform
//
static const char* BSSKIN_DIFFUSE_WRAP					= "diffuseWrap";					// float
static const char* BSSKIN_SPECULAR_FACING				= "specularFacing";					// float
static const char* BSSKIN_SPECULAR_GLANCING				= "specularGlancing";				// float
static const char* BSSKIN_FUZZ_COLOUR					= "fuzzColour";						// float3
static const char* BSSKIN_FUZZ_TIGHTNESS				= "fuzzTightness";					// float
static const char* BSSKIN_SUBCOLOUR						= "subColour";						// float3
static const char* BSSKIN_WRINKLE_REGIONS0_WEIGHTS		= "wrinkleRegionWeights0";			// float4
static const char* BSSKIN_WRINKLE_REGIONS1_WEIGHTS		= "wrinkleRegionWeights1";			// float4
static const char* BSSKIN_NORMAL_MAP_STRENGTH			= "normalMapStrength";				// float	
static const char* BSSKIN_SPECULAR_STRENGTH				= "specularStrength";				// float
//
// samplers
//
static const char* BSSKIN_DIFFUSE_SPECULAR_S			= "s_BaseDiffuseAndSpecular";		// sampler2D
static const char* BSSKIN_BASE_NORMAL_OCCLUSION_S		= "s_BaseNormalAndOcclusion";		// sampler2D
static const char* BSSKIN_WRINKLE_NORMAL_OCCLUSION_S	= "s_WrinkleNormalAndOcclusion";	// sampler2D
static const char* BSSKIN_WRINKLE_REGIONS0_S			= "s_WrinkleRegions0Mask";			// sampler2D
static const char* BSSKIN_WRINKLE_REGIONS1_S			= "s_WrinkleRegions1Mask";			// sampler2D
static const char* BSSKIN_SPECULAR_GAIN_S				= "s_SpecularGain";					// sampler2D ( should be 1D )
static const char* BSSKIN_DIFFUSE_GAIN_S				= "s_DiffuseGain";					// sampler2D ( should be 1D )




#endif // end of _BSSKIN_SEMANTIC_NAMES_H_
