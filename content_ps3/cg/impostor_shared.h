//--------------------------------------------------
//!
//!	\file impostor_simple.fx
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

/*
// locals
uniform	float4x4	m_objectWorld;
uniform	float4x4	m_worldViewProj;

uniform	float		m_emitterAge;
uniform float		m_RCPlifetime;
uniform float		m_functionWidth;
uniform float		m_functionWidthRCP;
uniform	float3		m_force;
uniform float		m_rotAcc;
uniform float4		m_colourStart;
uniform float4		m_colourDiff;
uniform float4		m_TODModifier;
uniform float		m_TA_numTex;			// for texture animation only
uniform float		m_TA_TexWidth;			// for texture animation only
*/

uniform float		TA_numTex;
uniform float		TA_TexWidth;

//--------------------------------------------------
//!
//!	impostor_GetRandTexcoords
//!
//--------------------------------------------------
float2 impostor_GetRandTexcoords( float rand, float2 texcoord )
{
	// rand shoudl be in the range 0->1
	
	// get index of first texture to use.		
	float textureIndexR = rand * TA_numTex;
	float textureIndexI = floor( textureIndexR );
	
	// rescale texcoords and offset to start tex
	texcoord.x = (texcoord.x + textureIndexI) * TA_TexWidth;			
	return texcoord;
}

//--------------------------------------------------
//!
//!	impostor_GetAnimTexColour
//!
//--------------------------------------------------
float4 impostor_GetAnimTexColour( float age, float2 texcoord, uniform sampler2D tosample )
{
	// get index of first texture to use.		
	float textureIndexR = age * TA_numTex;
	float textureIndexI = floor( textureIndexR );
	
	// rescale texcoords and offset to start tex
	texcoord.x = (texcoord.x + textureIndexI) * TA_TexWidth;
	
	// get second set of coords
	float2 texcoord2 = texcoord;
	texcoord2.x += TA_TexWidth;
//	texcoord2.x = clamp( texcoord2.x, 0.0f, 1.0f );

	float4 col1 = tex2D( tosample, texcoord );
	float4 col2 = tex2D( tosample, texcoord2 );
			
	return lerp( col1, col2, textureIndexR - textureIndexI ); 	
}

/*
//--------------------------------------------------
//!
//!	functionSample_linearFilter
//!
//--------------------------------------------------
float functionSample_linearFilter(	sampler2D functionTable,
									float functionArgument,
									float functionIndex,
									float functionResolution,
									float functionResolutionRCP )
{
	// funciton argument must be in the range 0.0 to 1.0
	float lerpVal = frac( functionArgument * functionResolution );
	float sample1 = tex2Dlod( functionTable, float4( functionArgument, functionIndex, 0.0, 0.0f ) );
	float sample2 = tex2Dlod( functionTable, float4( functionArgument + functionResolutionRCP, functionIndex, 0.0, 0.0f ) );
	return lerp( sample1, sample2, lerpVal );
}

//--------------------------------------------------
//!
//!	psystem_GetNormalisedAge
//!
//--------------------------------------------------
float psystem_GetNormalisedAge( float birthTime )
{
	return clamp( (m_emitterAge - birthTime) * m_RCPlifetime, 0.0f, 1.0f );
}

//--------------------------------------------------
//!
//!	psystem_GetFunctionalSize
//!
//--------------------------------------------------
float psystem_GetFunctionalSize( float fNormalisedAge, float fSizeRange, float fSizeStart )
{
	float functionLerp = frac( fNormalisedAge * m_functionWidth );
	
	float sizeFunc1 = functionSample_linearFilter( s_functions, fNormalisedAge, SIZE_FUNCTION1_INDEX, m_functionWidth, m_functionWidthRCP );
	float sizeFunc2 = functionSample_linearFilter( s_functions, fNormalisedAge, SIZE_FUNCTION2_INDEX, m_functionWidth, m_functionWidthRCP );

	float sizeLerp = (fSizeRange * fNormalisedAge) + fSizeStart;
	return lerp( sizeFunc1, sizeFunc2, sizeLerp );
}

//--------------------------------------------------
//!
//!	psystem_GetPixelSize
//!
//--------------------------------------------------
float psystem_GetPixelSize( float4 posWorld, float sizeMeters )
{
	return ( 1.0f / dot( posWorld, g_cameraZ ) ) * sizeMeters * g_vpScalars.y;
}

//--------------------------------------------------
//!
//!	psystem_GetWorldPosition
//!
//--------------------------------------------------
float4 psystem_GetWorldPosition( float fNormalisedAge, float4 startPos, float3 velocity )
{
	float4 pos = startPos;	
	pos.xyz += velocity.xyz * fNormalisedAge.xxx;
	return mul( pos, m_objectWorld );
}

//--------------------------------------------------
//!
//!	psystem_GetOffsetWorldPosition
//!
//--------------------------------------------------
float3 psystem_GetOffsetWorldPosition( float3 start, float2 texcoord, float sizeMeters )
{
	float2 offset = (texcoord - 0.5f.xx) * -sizeMeters;
	start.xyz +=  offset.xxx * g_cameraUnitAxisX;
	start.xyz +=  offset.yyy * g_cameraUnitAxisY;
	return start;
}

float3 psystem_GetOffsetWorldPosition( float3 start, float2 texcoord, float sizeMeters, float3 Xaxis, float3 Yaxis )
{
	float2 offset = (texcoord - 0.5f.xx) * -sizeMeters;
	start.xyz +=  offset.xxx * Xaxis;
	start.xyz +=  offset.yyy * Yaxis;
	return start;
}

float3 psystem_GetOffsetWorldPosition( float3 start, float2 texcoord, float sizeMeters, float rotation )
{
	float2 offset = (texcoord - 0.5f.xx) * -sizeMeters;
		
	float2x2 rotation_matrix;
	sincos( rotation, rotation_matrix[0].y, rotation_matrix[0].x );
	rotation_matrix[1].xy = rotation_matrix[0].yx * float2( -1.0f, 1.0f );
	offset = mul( offset, rotation_matrix );
	
	start.xyz +=  offset.xxx * g_cameraUnitAxisX;
	start.xyz +=  offset.yyy * g_cameraUnitAxisY;
	return start;
}

//--------------------------------------------------
//!
//!	psystem_TransformColour
//!
//--------------------------------------------------
float3 psystem_TransformColour( float3 colour )
{
	return affine_mul_point( colour, g_ImageTransform );
}

//--------------------------------------------------
//!
//!	psystem_GetRotationTex
//!
//--------------------------------------------------
float2 psystem_GetRotationTex( float rot, float2 texcoord )
{
	float2x2 rotation_matrix;
	sincos( rot, rotation_matrix[0].y, rotation_matrix[0].x );
	rotation_matrix[1].xy = rotation_matrix[0].yx * float2( -1.0f, 1.0f );
	
	float2 rot_tex = (texcoord - 0.5f.xx);
	rot_tex = mul( rot_tex, rotation_matrix );
	return (rot_tex + 0.5f.xx);
}



//--------------------------------------------------
//!
//!	psystem_GetRandTexColour
//!
//--------------------------------------------------
float4 psystem_GetRandTexColour( float rand, float2 texcoord, uniform sampler2D tosample )
{
	// rand shoudl be in the range 0->1
	
	// get index of first texture to use.		
	float textureIndexR = rand * m_TA_numTex;
	float textureIndexI = floor( textureIndexR );
	
	// rescale texcoords and offset to start tex
	texcoord.x = (texcoord.x + textureIndexI) * m_TA_TexWidth;			
	return tex2D( tosample, texcoord );
}

//--------------------------------------------------
//!
//!	point_ps
//!
//--------------------------------------------------
PS_OUTPUT point_ps( POINT_VS_OUTPUT input, uniform int eTexMode )
{
	PS_OUTPUT output;
	
	if (eTexMode == PTM_SIMPLE_TEXTURED)
	{
		output.col = input.col * tex2D( s_diffuse0, input.texcoord ) * m_TODModifier;
	}
	else if (eTexMode == PTM_ANIM_TEXTURED)
	{
		output.col = input.col * psystem_GetAnimTexColour( input.age, input.texcoord, s_diffuse0 ) * m_TODModifier;
	}
	else if (eTexMode == PTM_RAND_TEXTURED)
	{
		output.col = input.col * psystem_GetAnimTexColour( input.age, input.texcoord, s_diffuse0 ) * m_TODModifier;
	}
	else // PTM_UNTEXTURED
	{
		output.col = input.col * m_TODModifier;
	}
	
	output.col.xyz = psystem_TransformColour( output.col.xyz );
	return output;
}

//--------------------------------------------------
//!
//!	point_ps_palette
//!
//--------------------------------------------------
PS_OUTPUT point_ps_palette( POINT_VS_OUTPUT_PALETTE input, uniform int eTexMode )
{
	PS_OUTPUT output;
	float4 col = tex1D( s_palette0, input.age );
	
	if (eTexMode == PTM_SIMPLE_TEXTURED)
	{
		output.col = col * tex2D( s_diffuse0, input.texcoord ) * m_TODModifier;
	}
	else if (eTexMode == PTM_ANIM_TEXTURED)
	{
		output.col = col * psystem_GetAnimTexColour( input.age, input.texcoord, s_diffuse0 ) * m_TODModifier;
	}
	else if (eTexMode == PTM_RAND_TEXTURED)
	{
		output.col = col * psystem_GetAnimTexColour( input.age, input.texcoord, s_diffuse0 ) * m_TODModifier;
	}
	else // PTM_UNTEXTURED
	{
		output.col = col * m_TODModifier;
	}
		
	output.col.xyz = psystem_TransformColour( output.col.xyz );			
	return output;
}

//--------------------------------------------------
//!
//!	quad_ps
//!
//--------------------------------------------------
PS_OUTPUT quad_ps( QUAD_VS_OUTPUT input,
					uniform int eTexMode )
{
	PS_OUTPUT output;
	
	if (eTexMode == PTM_SIMPLE_TEXTURED)
	{
		output.col = input.col * tex2D( s_diffuse0, input.texcoord.xy ) * m_TODModifier;
	}
	else if (eTexMode == PTM_ANIM_TEXTURED)
	{
		output.col = input.col * psystem_GetAnimTexColour( input.age, input.texcoord.xy, s_diffuse0 ) * m_TODModifier;
	}
	else if (eTexMode == PTM_RAND_TEXTURED)
	{
		output.col = input.col * psystem_GetRandTexColour( input.texcoord.z, input.texcoord.xy, s_diffuse0 ) * m_TODModifier;
	}
	else // PTM_UNTEXTURED
	{
		output.col = input.col * m_TODModifier;
	}
		
	output.col.xyz = psystem_TransformColour( output.col.xyz );			
	return output;
}

//--------------------------------------------------
//!
//!	quad_ps_palette
//!
//--------------------------------------------------
PS_OUTPUT quad_ps_palette( QUAD_VS_OUTPUT_PALETTE input,
							uniform int eTexMode )							
{
	PS_OUTPUT output;
	float4 col = tex1D( s_palette0, input.age );

	if (eTexMode == PTM_SIMPLE_TEXTURED)
	{
		output.col = col * tex2D( s_diffuse0, input.texcoord.xy ) * m_TODModifier;
	}
	else if (eTexMode == PTM_ANIM_TEXTURED)
	{
		output.col = col * psystem_GetAnimTexColour( input.age, input.texcoord.xy, s_diffuse0 ) * m_TODModifier;
	}
	else if (eTexMode == PTM_RAND_TEXTURED)
	{
		output.col = col * psystem_GetRandTexColour( input.texcoord.z, input.texcoord.xy, s_diffuse0 ) * m_TODModifier;
	}
	else // PTM_UNTEXTURED
	{
		output.col = col * m_TODModifier;
	}
			
	output.col.xyz = psystem_TransformColour( output.col.xyz );
	return output;
}
*/