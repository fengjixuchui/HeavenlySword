//--------------------------------------------------
//!
//!	\file weaponchains.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

//--------------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------------
#include "common.h"
#include "uber_material_variables.h"
#include "uber_material_macros.h"

//--------------------------------------------------
//!
//!	IO structs
//!
//--------------------------------------------------
struct VS_INPUT
{
	float4	position_objectS;//	: TEXCOORD0;
	float3	normal_objectS;//		: TEXCOORD1;
	float3	tangent_objectS;//		: TEXCOORD2;
	float3	binormal_objectS;//	: TEXCOORD3;
	float2	normalMapTexcoord;//	: TEXCOORD4;
	float2	diffuseTexcoord0;//	: TEXCOORD5;
}; 

struct VS_OUTPUT
{
	float4 	position_screenS			: POSITION0;
	float4 	diffuseFillAndKeyScalar		: TEXCOORD0;
	float2 	normalMapTexcoord			: TEXCOORD1;
	float3 	viewDir_tangentS			: TEXCOORD2;
	float3 	keyLightDir_tangentS		: TEXCOORD3;
	float2 	diffuseTexcoord0			: TEXCOORD4;
	float3 	extinction					: COLOR0;
	float3 	inscattering				: TEXCOORD5;
	float 	depth						: TEXCOORD7;
};

struct PS_INPUT
{
	float2 	position_screenS			: WPOS;
	float4 	diffuseFillAndKeyScalar		: TEXCOORD0;
	float2 	normalMapTexcoord			: TEXCOORD1;
	float3 	viewDir_tangentS			: TEXCOORD2;
	float3 	keyLightDir_tangentS		: TEXCOORD3;
	float2 	diffuseTexcoord0			: TEXCOORD4;
	float3 	extinction					: COLOR0;
	float3 	inscattering				: TEXCOORD5;
	float 	depth						: TEXCOORD7;
};

//--------------------------------------------------
//!
//!	based on phong 1n
//!
//--------------------------------------------------
VS_OUTPUT weaponchain_vs( VS_INPUT input )
{
	VS_OUTPUT output;

	FX_MACRO_START_STD_VERTEX_SHADER;
		
	float4 position_objectS	= input.position_objectS;
	float3 normal_objectS	= input.normal_objectS;
	float3 tangent_objectS	= input.tangent_objectS;
	float3 binormal_objectS	= input.binormal_objectS;
	float4 position_projS = mul( position_objectS, m_worldViewProj );
	
	const bool bSphericalHarmonics = true;
	FX_MACRO_INCORPERATE_SH_COEFFS;

	const bool bDepthHaze = true;
	FX_MACRO_CALCULATE_PER_VERTEX_HAZE_VARS(output.extinction,output.inscattering);

	output.normalMapTexcoord = input.normalMapTexcoord;
	output.diffuseTexcoord0 = input.diffuseTexcoord0;

	FX_MACRO_GENERATE_TSPACE_VIEW_DIR(output);
	
	FX_MACRO_GENERATE_TSPACE_KEYLIGHT_DIR(output);
	
	FX_MACRO_STOP_STD_VERTEX_SHADER_DEPTH(output);
	
	return output;
}

uniform sampler2D l_normalMap : register(s0);
uniform sampler2D l_diffuse0 : register(s2);

//--------------------------------------------------
//!
//!	based on lambert 1n
//!
//--------------------------------------------------
uniform float m_fAlphaRef;

PS_OUTPUT weaponchains_ps( PS_INPUT input )
{
	int eShadowType = ST_GETSHADOWTERM;
	PS_OUTPUT output;

	FX_MACRO_START_SHINY_PIXEL_SHADER(input);

	FX_MACRO_CALCULATE_SHADOW_TERM(input);
	
	// parallax texture offset calc
	FX_MACRO_PARALLAX_START(input.viewDir_tangentS,
							input.normalMapTexcoord);
	
	FX_MACRO_PARALLAX_RETRIEVE_J1(	input.normalMapTexcoord,
									input.diffuseTexcoord0);
	
	float3 normal_tangentS = 2.0f.xxx*( tex2D( l_normalMap, normalMapTexcoord_Parallax ).xyz - 0.5f.xxx );

	FX_MACRO_PERPIXEL_KEYLIGHT_PHONG_RESPONSE(input);
	
	float4 texCol = tex2D( l_diffuse0, diffuseTexcoord0_Parallax ).xyzw;
	float4 diffuseResult0 = texCol * m_diffuseColour0;
	float4 surfaceColour = diffuseResult0;

	const bool bDepthHaze = true;
	FX_MACRO_STOP_SHINY_PIXEL_SHADER(input,output);
	
	if (texCol.w < m_fAlphaRef)
		discard;

	output.RGBColor = _RGB_to_LogYUV( output.RGBColor );

    return output;
}
