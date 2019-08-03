//--------------------------------------------------
//!
//!	\file effecttrail_shared.cg
//!	Common parms for effectrails.
//!
//--------------------------------------------------

//--------------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------------
#include "effect_shared.h"

//--------------------------------------------------
//!
//!	IO structs
//!
//--------------------------------------------------
struct VS_OUTPUT
{
	float4 	pos				: POSITION0;
	float	ageN			: TEXCOORD0;
	float4	texcoord		: TEXCOORD1;
};

struct PS_OUTPUT
{
    float4	col	: COLOR0;
};

// texture mode
#define TTM_UNTEXTURED			0
#define TTM_SIMPLE_TEXTURED		1
#define TTM_ANIM_TEXTURED		2

// locals
uniform	float4x4	m_worldViewProj;

uniform	float		m_age;
uniform float		m_RCPfadetime;
uniform float4		m_TODModifier;
uniform float		m_TA_numTex;			// for texture animation only
uniform float		m_TA_TexWidth;			// for texture animation only
uniform float		m_Texfactor;

//--------------------------------------------------
//!
//!	trail_GetNormalisedAge
//!
//--------------------------------------------------
float trail_GetNormalisedAge( float birthTime )
{
	return clamp( (m_age - birthTime) * m_RCPfadetime, 0.0f, 1.0f );
}

//--------------------------------------------------
//!
//!	trail_TransformColour
//!
//--------------------------------------------------
float3 trail_TransformColour( float3 colour )
{
	return affine_mul_point( colour, g_ImageTransform );
}

//--------------------------------------------------
//!
//!	trail_GetAnimTexColour
//!
//--------------------------------------------------
float4 trail_GetAnimTexColour( uniform sampler2D s_diffuse0, float ageN, float2 texcoord )
{
	// get index of first texture to use.		
	float textureIndexR = ageN * m_TA_numTex;
	float textureIndexI = floor( textureIndexR );
	
	// rescale texcoords and offset to start tex
	texcoord.x = (texcoord.x + textureIndexI) * m_TA_TexWidth;
	
	// get second set of coords
	float2 texcoord2 = texcoord;
	texcoord2.x += m_TA_TexWidth;
	texcoord2.x = clamp( texcoord2.x, 0.0f, 1.0f );

	float4 col1 = tex2D( s_diffuse0, texcoord );
	float4 col2 = tex2D( s_diffuse0, texcoord2 );
			
	return lerp( col1, col2, textureIndexR - textureIndexI ); 	
}

//--------------------------------------------------
//!
//!	quad_ps_palette
//!
//--------------------------------------------------
PS_OUTPUT simple_ps( VS_OUTPUT input,
					uniform int eTexMode,
					uniform sampler2D s_diffuse0,
					uniform sampler1D s_fadePalette,
					uniform sampler1D s_crossPalette )
{
	PS_OUTPUT output;
	float4 fadecol = tex1D( s_fadePalette, input.ageN );
	float4 colModifier = m_TODModifier * tex1D( s_crossPalette, input.texcoord.x );
	
	float UTex = input.ageN * m_Texfactor;
	float2 texcoord = float2( UTex, input.texcoord.x );
	
	if (eTexMode == TTM_SIMPLE_TEXTURED)
	{
		output.col = fadecol * tex2D( s_diffuse0, texcoord ) * colModifier;
	}
	else if (eTexMode == TTM_ANIM_TEXTURED)
	{
		texcoord.x = texcoord.x - floor(texcoord.x);
		output.col = fadecol * trail_GetAnimTexColour( s_diffuse0, input.ageN, texcoord ) * colModifier;
	}
	else
	{
		output.col = fadecol * colModifier;
	}

	output.col.xyz = trail_TransformColour( output.col.xyz );
	return output;
}
