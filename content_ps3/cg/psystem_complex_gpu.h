//--------------------------------------------------
//!
//!	\file psystem_complex_gpu.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

uniform sampler2D s_functions : register(s0);

//--------------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------------
#include "psystem_shared.h"

//--------------------------------------------------
//!
//!	IO structs
//!
//--------------------------------------------------
struct POINT_VS_INPUT
{
	float	birthTime		: TEXCOORD0;
	float4	pos				: TEXCOORD1;
	float3	vel				: TEXCOORD2;
	float	sizeLerpStart	: TEXCOORD3;
	float	sizeLerpRange	: TEXCOORD4;
};

struct QUAD_VS_INPUT
{
	float	birthTime		: TEXCOORD0;
	float4	pos				: TEXCOORD1;
	float3	vel				: TEXCOORD2;
	float	sizeLerpStart	: TEXCOORD3;
	float	sizeLerpRange	: TEXCOORD4;
	float4	texcoord		: TEXCOORD5;	
};

//--------------------------------------------------
//!
//!	point_vs
//!
//--------------------------------------------------
POINT_VS_OUTPUT_PALETTE point_vs( POINT_VS_INPUT particle )
{
	POINT_VS_OUTPUT_PALETTE output;
	
	float particleAge_N = psystem_GetNormalisedAge( particle.birthTime );
	
	float sizeMeters = psystem_GetFunctionalSize( particleAge_N, particle.sizeLerpRange, particle.sizeLerpStart );
	float4 pos = psystem_GetWorldPosition( particleAge_N, particle.pos, particle.vel );
	
	float particleAgeSq = m_emitterAge - particle.birthTime;
	pos.xyz += m_force.xyz * particleAgeSq * particleAgeSq * 0.5f;
	
	output.pos = mul( pos, m_worldViewProj );
	
	if (particleAge_N < 1.0f)
	{	
		output.age = particleAge_N;	
		output.size = psystem_GetPixelSize( pos, sizeMeters );
	}
	else
	{
		output.pos = float4(-1.0f,-1.0f,-1.0f,-1.0f);
		output.age = 1.0f;	
		output.size = 0.0f;		
	}
	
	return output;
}

//--------------------------------------------------
//!
//!	quad_vs
//!
//--------------------------------------------------
QUAD_VS_OUTPUT_PALETTE quad_vs( QUAD_VS_INPUT particle )
{
	QUAD_VS_OUTPUT_PALETTE output = (QUAD_VS_OUTPUT_PALETTE)0;
	
	float particleAge_N = psystem_GetNormalisedAge( particle.birthTime );

	float sizeMeters = psystem_GetFunctionalSize( particleAge_N, particle.sizeLerpRange, particle.sizeLerpStart );
	float4 pos = psystem_GetWorldPosition( particleAge_N, particle.pos, particle.vel );

	float particleAgeSq = m_emitterAge - particle.birthTime;
	pos.xyz += m_force.xyz * particleAgeSq * particleAgeSq * 0.5f;
	pos.xyz = psystem_GetOffsetWorldPosition( pos.xyz, particle.texcoord, sizeMeters );
	
	output.pos = mul( pos, m_worldViewProj );

	if (particleAge_N < 1.0f)
	{
		output.age = particleAge_N;
		output.texcoord = particle.texcoord;
	}
	else
	{
		output.pos = float4(-1.0f,-1.0f,-1.0f,-1.0f);
		output.age = 1.0f;
		output.texcoord = float4(0.0f,0.0f,0.0f,0.0f);	
	}
	
	return output;
}
