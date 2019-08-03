//--------------------------------------------------
//!
//!	\file uber_material_macros.hlsl
//!	contains macros that generate uber shader bodies
//!
//--------------------------------------------------

//--------------------------------------------------
//!
//!	Vertex Shader Delimiters
//!
//--------------------------------------------------
#define FX_MACRO_START_STD_VERTEX_SHADER								\
	float3 diffuseLighting = 0.0f.xxx;									\
	float diffuseKeyScalar = 0.0f;

//--------------------------------------------------
#define FX_MACRO_STOP_STD_VERTEX_SHADER(outVars)						\
	outVars.position_screenS = position_projS;							\
	outVars.diffuseFillAndKeyScalar.xyz = diffuseLighting;				\
	outVars.diffuseFillAndKeyScalar.w = diffuseKeyScalar;

//--------------------------------------------------
#define FX_MACRO_STOP_STD_VERTEX_SHADER_DEPTH(outVars)					\
	FX_MACRO_STOP_STD_VERTEX_SHADER(outVars);							\
	outVars.depth = outVars.position_screenS.z;

//--------------------------------------------------
#define FX_MACRO_START_LAMBERT_PIXEL_SHADER(inVars)							\
	float3 diffuseLighting = inVars.diffuseFillAndKeyScalar.xyz;			\
	float diffuseKeyScalar = saturate( inVars.diffuseFillAndKeyScalar.w );

//--------------------------------------------------
#define FX_MACRO_STOP_LAMBERT_PIXEL_SHADER(inVars,outVars)															\
	outVars.RGBColor.xyz = surfaceColour.xyz * diffuseLighting;														\
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscattering );	\
	}																												

//--------------------------------------------------
#define FX_MACRO_STOP_LAMBERT_PIXEL_SHADER_ALPHA(inVars,outVars)													\
	outVars.RGBColor.xyz = surfaceColour.xyz * diffuseLighting;														\
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscattering );	\
	}

//--------------------------------------------------
#define FX_MACRO_START_SHINY_PIXEL_SHADER(inVars)							\
	float3 diffuseLighting = inVars.diffuseFillAndKeyScalar.xyz;			\
	float diffuseKeyScalar = inVars.diffuseFillAndKeyScalar.w;				\
	float3 specularLighting = 0.0f.xxx;

//--------------------------------------------------
#define FX_MACRO_STOP_SHINY_PIXEL_SHADER(inVars,outVars)															\
	outVars.RGBColor.xyz = (surfaceColour.xyz * diffuseLighting) + (surfaceColour.w * specularLighting);			\
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscattering );	\
	}																												
	
//--------------------------------------------------
#define FX_MACRO_STOP_SHINY_PIXEL_SHADER_ALPHA(inVars,outVars)														\
	outVars.RGBColor.xyz = (surfaceColour.xyz * diffuseLighting) + (specularResponse.w * specularLighting);			\
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscattering );	\
	}

//--------------------------------------------------
#define FX_MACRO_STOP_METALLIC_PIXEL_SHADER(inVars,outVars)															\
	outVars.RGBColor.xyz = surfaceColour.xyz * lerp( diffuseLighting, reflectanceLighting, inVars.inscatterAndFresnel.w * surfaceColour.w ) + (surfaceColour.w * specularLighting); \
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscatterAndFresnel );	\
	}																												

//--------------------------------------------------
#define FX_MACRO_STOP_METALLIC_PIXEL_SHADER_ALPHA(inVars,outVars)															\
	outVars.RGBColor.xyz = surfaceColour.xyz * lerp( diffuseLighting, reflectanceLighting, inVars.inscatterAndFresnel.w * specularResponse.w ) + (specularResponse.w * specularLighting); \
	outVars.RGBColor.w = surfaceColour.w;																			\
																													\
	if (bDepthHaze)																									\
	{																												\
		outVars.RGBColor.xyz = depth_haze_combine( outVars.RGBColor.xyz, inVars.extinction, inVars.inscatterAndFresnel );	\
	}

//--------------------------------------------------
//!
//!	Vertex info fetching 
//!
//--------------------------------------------------
#define FX_MACRO_FETCH_OBJECTSPACE_VERTEX_INFO(inVars)											\
	float4 position_objectS;																	\
	float3 normal_objectS;																		\
																								\
	if (eVertexType == VT_STATIC)																\
	{																							\
		position_objectS	= inVars.position_objectS;											\
		normal_objectS		= inVars.normal_objectS;											\
	}																							\
	else if (eVertexType == VT_SKINNED)															\
	{																							\
		float3x4 skinMat = skin_matrix( inVars.weights, inVars.indices, m_blendMats );			\
																								\
		position_objectS	= skin_point( skinMat, inVars.position_objectS );					\
		normal_objectS		= skin_direction( skinMat, inVars.normal_objectS );					\
	}																							\
																								\
	float4 position_projS = mul( position_objectS, m_worldViewProj );

//--------------------------------------------------
#define FX_MACRO_FETCH_OBJECTSPACE_VERTEX_INFO_NORMALMAPPED(inVars)								\
	float4 position_objectS;																	\
	float3 normal_objectS;																		\
	float3 tangent_objectS;																		\
	float3 binormal_objectS;																	\
																								\
	if (eVertexType == VT_STATIC)																\
	{																							\
		position_objectS	= inVars.position_objectS;											\
		normal_objectS		= inVars.normal_objectS;											\
		tangent_objectS		= inVars.tangent_objectS;											\
		binormal_objectS	= inVars.binormal_objectS;											\
	}																							\
	else if (eVertexType == VT_SKINNED)															\
	{																							\
		float3x4 skinMat = skin_matrix( inVars.weights, inVars.indices, m_blendMats );			\
																								\
		position_objectS	= skin_point( skinMat, inVars.position_objectS );					\
		normal_objectS		= skin_direction( skinMat, inVars.normal_objectS );					\
		tangent_objectS		= skin_direction( skinMat, inVars.tangent_objectS );				\
		binormal_objectS	= skin_direction( skinMat, inVars.binormal_objectS );				\
	}																							\
																								\
	float4 position_projS = mul( position_objectS, m_worldViewProj );

//--------------------------------------------------
//!
//!	Depth haze
//!
//--------------------------------------------------
#define FX_MACRO_CALCULATE_PER_VERTEX_HAZE_VARS(extinction,inscattering)	\
	if (bDepthHaze )														\
	{																		\
		depth_haze( position_objectS,										\
					extinction,												\
					inscattering.xyz,										\
					m_worldView,											\
					m_viewPosition_objectS,									\
					g_DHConstsA,											\
					g_DHConstsG,											\
					m_sunDir_objectS,										\
					g_DHB1plusB2,											\
					g_DHBdash1,												\
					g_DHBdash2,												\
					g_DHRecipB1plusB2,										\
					g_sunColour );											\
	}
	
//--------------------------------------------------
//!
//!	Shadows
//!
//--------------------------------------------------
//--------------------------------------------------
#define FX_MACRO_CALCULATE_SHADOW_TERM(inVars)														\
	float shadowTerm = 1.0f;																		\
																									\
	if (eShadowType == ST_GETSHADOWTERM)															\
	{																								\
		float2 texcoord = inVars.position_screenS * g_VPOStoUV;			\
		texcoord.y = 1.0f - texcoord.y;									\
		shadowTerm = tex2D( s_stencilMap, texcoord ).x;					\
	}																								

//--------------------------------------------------
//!
//!	Lighting
//!
//--------------------------------------------------
#define FX_MACRO_INCORPERATE_SH_COEFFS												\
	if (bSphericalHarmonics)														\
	{																				\
		float3 normal_worldS = affine_mul_direction( normal_objectS, m_world );		\
		diffuseLighting += max(sh_lookup( normal_worldS, g_fillSHCoeffs ),0 );		\
	}
	
//--------------------------------------------------
#define FX_MACRO_GENERATE_TSPACE_KEYLIGHT_DIR(outVars)									\
	outVars.keyLightDir_tangentS.x = dot( m_keyLightDir_objectS, tangent_objectS );		\
	outVars.keyLightDir_tangentS.y = dot( m_keyLightDir_objectS, binormal_objectS );	\
	outVars.keyLightDir_tangentS.z = dot( m_keyLightDir_objectS, normal_objectS );

//--------------------------------------------------
#define FX_MACRO_GENERATE_TSPACE_VIEW_DIR(outVars)								\
	float3 viewDir_objectS = position_objectS.xyz - m_viewPosition_objectS;		\
	outVars.viewDir_tangentS.x = dot( viewDir_objectS, tangent_objectS );		\
	outVars.viewDir_tangentS.y = dot( viewDir_objectS, binormal_objectS );		\
	outVars.viewDir_tangentS.z = dot( viewDir_objectS, normal_objectS );

//--------------------------------------------------
// shifted to world space in an attemp to move compiler bug resulting in negative fresnel
// compiler should optimise away normal_worldS, as this is used in SH
#define FX_MACRO_GENERATE_METALLIC_FRESNEL(outVars)											\
	float3 viewDir_objectS = normalize( position_objectS.xyz - m_viewPosition_objectS );	\
	float3 viewDir_worldS = affine_mul_direction( viewDir_objectS, m_world );				\
	float3 normal_worldS = affine_mul_direction( normal_objectS, m_world );					\
	float fresnel = 1.0f + 0.5f * min( dot( viewDir_worldS, normal_worldS ), 0.0f );		\
	fresnel = lerp( 1.0f, fresnel, m_fresnelStrength );										\
	outVars.inscatterAndFresnel.w = fresnel;												

#define FX_MACRO_GENERATE_METALLIC_FRESNEL_SHOULD_WORK_BUT_DOESNT(outVars)					\
	float3 viewDir_objectS = normalize( position_objectS.xyz - m_viewPosition_objectS );	\
	float fresnel = 1.0f + 0.5f * min( dot( viewDir_objectS, normal_objectS ), 0.0f );		\
	fresnel = lerp( 1.0f, fresnel, m_fresnelStrength );										\
	outVars.inscatterAndFresnel.w = fresnel;												

#define FX_MACRO_GENERATE_METALLIC_FRESNEL_PARALLAX(outVars)								\
	FX_MACRO_GENERATE_METALLIC_FRESNEL(outVars)												\
	outVars.viewDir_tangentS.x = dot( viewDir_objectS, tangent_objectS );					\
	outVars.viewDir_tangentS.y = dot( viewDir_objectS, binormal_objectS );					\
	outVars.viewDir_tangentS.z = dot( viewDir_objectS, normal_objectS );

//--------------------------------------------------
#define FX_MACRO_GENERATE_METALLIC_NMAP_INTERPOLANTS(outVars)										\
	outVars.tangent_reflectS.xyz	= affine_mul_direction( tangent_objectS, m_reflectanceMat );	\
	outVars.binormal_reflectS.xyz	= affine_mul_direction( binormal_objectS, m_reflectanceMat );	\
	outVars.normal_reflectS.xyz		= affine_mul_direction( normal_objectS, m_reflectanceMat );		\
																									\
	float3 viewDir_reflectS = affine_mul_direction( viewDir_objectS, m_reflectanceMat );			\
	outVars.tangent_reflectS.w	= viewDir_reflectS.x;												\
	outVars.binormal_reflectS.w	= viewDir_reflectS.y;												\
	outVars.normal_reflectS.w	= viewDir_reflectS.z;

//--------------------------------------------------
#define FX_MACRO_RETRIEVE_NORMAL_TS(normalMapTexcoord)												\
	float3 normal_tangentS = 2.0f.xxx*( tex2D( s_normalMap, normalMapTexcoord ).xyz - 0.5f.xxx );

//--------------------------------------------------
#define FX_MACRO_PERPIXEL_KEYLIGHT_LAMBERTIAN_RESPONSE(inVars)																	\
	diffuseLighting += max( dot( normal_tangentS, inVars.keyLightDir_tangentS ), 0.0f ).xxx * g_keyDirColour * shadowTerm;

//--------------------------------------------------
#define FX_MACRO_PERPIXEL_KEYLIGHT_PHONG_RESPONSE(inVars)																	\
	float3 reflectedResult = normalize( reflect( inVars.viewDir_tangentS, normal_tangentS ) );								\
	float3 litCoeffs = lit( dot( normal_tangentS, input.keyLightDir_tangentS ), dot( reflectedResult, input.keyLightDir_tangentS ), m_specularPower );	\
																														\
	diffuseLighting += litCoeffs.yyy * g_keyDirColour * shadowTerm;														\
	specularLighting = m_specularColour * litCoeffs.zzz * g_keyDirColour * shadowTerm;

//--------------------------------------------------
#define FX_MACRO_GENERATE_METALLIC_NMAP_REFLECTANCE(inVars)																	\
	float3 normal_reflectS =	inVars.tangent_reflectS.xyz		* normal_tangentS.xxx +										\
								inVars.binormal_reflectS.xyz	* normal_tangentS.yyy +										\
								inVars.normal_reflectS.xyz		* normal_tangentS.zzz;										\
																															\
	float3 viewDir_reflectS = float3( inVars.tangent_reflectS.w, inVars.binormal_reflectS.w, inVars.normal_reflectS.w );	\
	float3 reflectedResult = reflect( viewDir_reflectS, normal_reflectS );

//--------------------------------------------------
#define FX_MACRO_GENERATE_METALLIC_LIGHTING(reflectDir)																	\
	float3 reflectanceLighting = m_reflectedColour * g_reflectanceColour * texCUBE( s_reflectance, reflectDir ).xyz;	\
	float3 litCoeffs = lit( diffuseKeyScalar, dot( g_keyDirReflect, normalize( reflectDir ) ), m_specularPower );		\
																														\
	diffuseLighting += litCoeffs.yyy * g_keyDirColour * shadowTerm;														\
	specularLighting = m_specularColour * litCoeffs.zzz * g_keyDirColour * shadowTerm;

//--------------------------------------------------
// ATTN! really we should have an indipendant scale and bias for each texture channel,
// but for the moment we will share a global one.
	
// this is standard parallax mapping
#define FX_MACRO_PARALLAX_START(viewDir_tangentS,normalMapTexcoord)			\
	float3 eyeVec_tangentS = normalize( viewDir_tangentS );					\
	float pixelHeight = (tex2D( s_normalMap, normalMapTexcoord ).w - 0.5) * g_parallaxScaleAndBias.x;	\
	float2 parallaxOffset = (pixelHeight.xx * eyeVec_tangentS);

//--------------------------------------------------
#define FX_MACRO_PARALLAX_RETRIEVE_J1(normalMapTexcoord, diffuseTexcoord0)	\
	float2 normalMapTexcoord_Parallax = parallaxOffset + normalMapTexcoord;	\
	float2 diffuseTexcoord0_Parallax = parallaxOffset + diffuseTexcoord0;

//--------------------------------------------------
#define FX_MACRO_PARALLAX_RETRIEVE_J2(normalMapTexcoord, diffuseTexcoord0, diffuseTexcoord1 )		\
	float2 normalMapTexcoord_Parallax = parallaxOffset + normalMapTexcoord;	\
	float2 diffuseTexcoord0_Parallax = parallaxOffset + diffuseTexcoord0;	\
	float2 diffuseTexcoord1_Parallax = parallaxOffset + diffuseTexcoord1;

//--------------------------------------------------
#define FX_MACRO_PARALLAX_RETRIEVE_J3(normalMapTexcoord, diffuseTexcoord0, diffuseTexcoord1, diffuseTexcoord2)		\
	float2 normalMapTexcoord_Parallax = parallaxOffset + normalMapTexcoord;	\
	float2 diffuseTexcoord0_Parallax = parallaxOffset + diffuseTexcoord0;	\
	float2 diffuseTexcoord1_Parallax = parallaxOffset + diffuseTexcoord1;	\
	float2 diffuseTexcoord2_Parallax = parallaxOffset + diffuseTexcoord2;

//--------------------------------------------------
//!
//!	texture combiners
//! ive only written the ones ive used so far...
//!
//--------------------------------------------------
#define FX_MACRO_GETSURFACE_J2_MOD(surface)											\
	float3 texColour1 = diffuseResult0.xyz * diffuseResult1.xyz;					\
	surface.xyz = texColour1;														\
	surface.w = diffuseResult0.w;
	
#define FX_MACRO_GETSURFACE_J2_MOD_AB(surface)										\
	float3 texColour1 = diffuseResult0.xyz * diffuseResult1.xyz;					\
	texColour1 = lerp( diffuseResult0.xyz, texColour1, diffuseResult1.w );			\
	surface.xyz = texColour1;														\
	surface.w = diffuseResult0.w;

#define FX_MACRO_GETSURFACE_J2_ADD(surface)											\
	float3 texColour1 = diffuseResult0.xyz + diffuseResult1.xyz;					\
	surface.xyz = texColour1;														\
	surface.w = diffuseResult0.w;

#define FX_MACRO_GETSURFACE_J2_ADD_AB(surface)										\
	float3 texColour1 = diffuseResult0.xyz + diffuseResult1.xyz;					\
	texColour1 = lerp( diffuseResult0.xyz, texColour1, diffuseResult1.w );			\
	surface.xyz = texColour1;														\
	surface.w = diffuseResult0.w;

#define FX_MACRO_GETSURFACE_J3_ADD_ADD(surface)										\
	float3 texColour1 = diffuseResult0.xyz + diffuseResult1.xyz;					\
	float3 texColour2 = texColour1.xyz + diffuseResult2.xyz;						\
	surface.xyz = texColour2;														\
	surface.w = diffuseResult0.w;

#define FX_MACRO_GETSURFACE_J3_ADD_AB_MOD(surface)									\
	float3 texColour1 = diffuseResult0.xyz + diffuseResult1.xyz;					\
	texColour1 = lerp( diffuseResult0.xyz, texColour1, diffuseResult1.w );			\
	float3 texColour2 = texColour1.xyz * diffuseResult2.xyz;						\
	surface.xyz = texColour2;														\
	surface.w = diffuseResult0.w;
	
#define FX_MACRO_GETSURFACE_J3_OVER_AB_MOD(surface)									\
	float3 texColour1 = diffuseResult1.xyz;											\
	texColour1 = lerp( diffuseResult0.xyz, texColour1, diffuseResult1.w );			\
	float3 texColour2 = texColour1.xyz * diffuseResult2.xyz;						\
	surface.xyz = texColour2;														\
	surface.w = diffuseResult0.w;
	
//--------------------------------------------------
//!
//!	standard depth output (most shader can use these defaults)
//!
//--------------------------------------------------
struct STD_DEPTH_VS_INPUT
{
	float4	position_objectS			: FXSS_POSITION;
	int4	indices						: FXSS_INDICES;
	float4	weights						: FXSS_WEIGHTS;
};
struct STD_ALPHAS_DEPTH_VS_INPUT
{
	float4	position_objectS			: FXSS_POSITION;
	float2	diffuseTexcoord0			: FXSS_DIFFUSE0_TEX;	
	int4	indices						: FXSS_INDICES;
	float4	weights						: FXSS_WEIGHTS;
};
struct STD_DEPTH_VS_OUTPUT
{
	float4 	position_screenS			: POSITION0;
	float4 	pos_screenS					: TEXCOORD0;
	float4 	shadowCoord					: TEXCOORD1;
	float4 	shadowCoord1				: TEXCOORD2;
	float4 	shadowCoord2				: TEXCOORD3;
	float4 	shadowCoord3				: TEXCOORD4;
	float4 	pos_objectS					: TEXCOORD5;
};
struct STD_ALPHA_DEPTH_VS_OUTPUT
{
	float4 	position_screenS			: POSITION0;
	float4 	pos_screenS					: TEXCOORD0;
	float4 	shadowCoord					: TEXCOORD1;
	float4 	shadowCoord1				: TEXCOORD2;
	float4 	shadowCoord2				: TEXCOORD3;
	float4 	shadowCoord3				: TEXCOORD4;
	float4 	pos_objectS					: TEXCOORD5;
	float2	diffuseTexcoord0			: TEXCOORD6;
};

//--------------------------------------------------
//!
//!	standard depth vs and ps
//!
//--------------------------------------------------
STD_DEPTH_VS_OUTPUT std_depth_vs( STD_DEPTH_VS_INPUT inVars, uniform int eVertexType, uniform bool bShadow )
{
    STD_DEPTH_VS_OUTPUT outVars = (STD_DEPTH_VS_OUTPUT)0;

	float4 position_objectS;
	if (eVertexType == VT_STATIC)
	{
		position_objectS	= inVars.position_objectS;
	}
	else if (eVertexType == VT_SKINNED)
	{
		float3x4 skinMat = skin_matrix( inVars.weights, inVars.indices, m_blendMats );
		position_objectS	= skin_point( skinMat, inVars.position_objectS );
	}
	outVars.pos_screenS = mul( position_objectS, m_worldViewProj );
	outVars.position_screenS = outVars.pos_screenS;
	
	if( bShadow )
	{
		outVars.shadowCoord = mul( position_objectS, m_shadowMapMat );
		outVars.shadowCoord1 = mul( position_objectS, m_shadowMapMat1 );
		outVars.shadowCoord2 = mul( position_objectS, m_shadowMapMat2 );
		outVars.shadowCoord3 = mul( position_objectS, m_shadowMapMat3 );
	}
	
	return outVars;
}
STD_ALPHA_DEPTH_VS_OUTPUT std_depth_alpha_vs( STD_ALPHAS_DEPTH_VS_INPUT inVars, uniform int eVertexType, uniform bool bShadow )
{
    STD_ALPHA_DEPTH_VS_OUTPUT outVars = (STD_ALPHA_DEPTH_VS_OUTPUT)0;

	float4 position_objectS;
	if (eVertexType == VT_STATIC)
	{
		position_objectS	= inVars.position_objectS;
	}
	else if (eVertexType == VT_SKINNED)
	{
		float3x4 skinMat = skin_matrix( inVars.weights, inVars.indices, m_blendMats );
		position_objectS	= skin_point( skinMat, inVars.position_objectS );
	}
	outVars.pos_screenS = mul( position_objectS, m_worldViewProj );
	outVars.position_screenS = outVars.pos_screenS;
	outVars.diffuseTexcoord0 = inVars.diffuseTexcoord0;
	
	if( bShadow )
	{
		outVars.shadowCoord = mul( position_objectS, m_shadowMapMat );
		outVars.shadowCoord1 = mul( position_objectS, m_shadowMapMat1 );
		outVars.shadowCoord2 = mul( position_objectS, m_shadowMapMat2 );
		outVars.shadowCoord3 = mul( position_objectS, m_shadowMapMat3 );
	}
	
	return outVars;
}
float4 std_depth_ps( STD_DEPTH_VS_OUTPUT inVars )
{
	return inVars.pos_screenS.z;
}
float4 std_depth_alpha_ps( STD_ALPHA_DEPTH_VS_OUTPUT inVars )
{
	float4 texCol = tex2D( s_diffuse0, inVars.diffuseTexcoord0 ).xyzw;
	texCol.xyz = inVars.pos_screenS.zzz;
	return texCol;
}
