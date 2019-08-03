//--------------------------------------------------
//!
//!	\file psystem_simple_cpu.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

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
	float	ageN		: TEXCOORD0;
	float4	pos			: TEXCOORD1;
	float	size		: TEXCOORD2;
};

struct QUAD_VS_INPUT
{
	float	ageN		: TEXCOORD0;
	float4	pos			: TEXCOORD1;
	float	size		: TEXCOORD2;
	float4	texcoord	: TEXCOORD3;
};

//--------------------------------------------------
//!
//!	point_vs
//!
//--------------------------------------------------
POINT_VS_OUTPUT point_vs( POINT_VS_INPUT particle )
{
	POINT_VS_OUTPUT output;
		
	float4 pos = mul( particle.pos, m_objectWorld );
	output.pos = mul( pos, m_worldViewProj );
	output.age = particle.ageN;	
	output.col = m_colourStart + (m_colourDiff * particle.ageN.xxxx);
	output.size = psystem_GetPixelSize( particle.pos, particle.size );
	
	return output;
}

//--------------------------------------------------
//!
//!	quad_vs
//!
//--------------------------------------------------
QUAD_VS_OUTPUT quad_vs( QUAD_VS_INPUT particle )
{
	QUAD_VS_OUTPUT output = (QUAD_VS_OUTPUT)0;

	float4 pos = mul( particle.pos, m_objectWorld );
	pos.xyz = psystem_GetOffsetWorldPosition( pos.xyz, particle.texcoord, particle.size );
		
	output.pos = mul( pos, m_worldViewProj );
	output.age = particle.ageN;	
	output.col = m_colourStart + (m_colourDiff * particle.ageN.xxxx);
	output.texcoord = particle.texcoord;
	
	return output;
}
