//--------------------------------------------------
//!
//!	\file uber_material_variables.hlsl
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

//--------------------------------------------------------------------------------------
// defines
//--------------------------------------------------------------------------------------
// SHADOW_TYPE
#define ST_NOSHADOW		0
#define ST_GETSHADOWTERM	1

// VERTEX_TYPE
#define VT_STATIC		0
#define VT_SKINNED		1
#define VT_BATCHED		2

//--------------------------------------------------------------------------------------
// stream semantics
//--------------------------------------------------------------------------------------
#define FXSS_POSITION			POSITION0
#define FXSS_NORMAL				NORMAL0
#define FXSS_TANGENT			TANGENT0
#define FXSS_BINORMAL			BINORMAL0
#define FXSS_NORMALMAP_TEX		TEXCOORD0
#define FXSS_DIFFUSE0_TEX		TEXCOORD1
#define FXSS_DIFFUSE1_TEX		TEXCOORD2
#define FXSS_DIFFUSE2_TEX		TEXCOORD3
#define FXSS_INDICES			BLENDINDICES0
#define FXSS_WEIGHTS			BLENDWEIGHT0


//--------------------------------------------------------------------------------------
// structs
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4	RGBColor : COLOR0;
};

//--------------------------------------------------------------------------------------
// object invariant globals
//--------------------------------------------------------------------------------------

// lighting
float4x4	g_fillSHCoeffs[3];

float3	g_keyDirColour;
float3	g_keyDirReflect;

float3	g_reflectanceColour;

float4	g_shadowMapResolution;
float4	g_shadowPlane0;
float4	g_shadowPlane1;
float4	g_shadowPlane2;
float4	g_shadowPlane3;
float4	g_shadowPlane4;

float4	g_shadowRadii0;
float4	g_shadowRadii1;
float4	g_shadowRadii2;
float4	g_shadowRadii3;

float2	g_parallaxScaleAndBias;
float3	g_depthOfFieldParams;

float2	g_VPOStoUV;

// depth haze
float3 	g_DHConstsA;
float3 	g_DHConstsG;
float3 	g_DHB1plusB2;
float3 	g_DHBdash1;
float3 	g_DHBdash2;
float3 	g_DHRecipB1plusB2;
float4 	g_sunColour;

// textures
texture	g_stencilMap;
texture	g_shadowMap;
texture	g_shadowMap1;
texture	g_shadowMap2;
texture	g_shadowMap3;
texture	g_reflectanceMap;

//--------------------------------------------------------------------------------------
// object specific globals
//--------------------------------------------------------------------------------------

// matrices
float4x4	m_worldViewProj;
float4x4	m_worldView;
float4x4	m_world;
float4x4	m_reflectanceMat;
float4x4	m_shadowMapMat;
float4x4	m_shadowMapMat1;
float4x4	m_shadowMapMat2;
float4x4	m_shadowMapMat3;

// lighting
float3		m_keyLightDir_objectS;
float3	 	m_sunDir_objectS;
float3		m_viewPosition_objectS;
float3		m_rimLight_objectS;

// materials
uniform float	m_alphatestRef;
uniform float4	m_diffuseColour0;
uniform float4	m_diffuseColour1;
uniform float4	m_diffuseColour2;

uniform float	m_fresnelStrength;
uniform float3	m_reflectedColour;
uniform float3	m_specularColour;
uniform float	m_specularPower;

uniform float	m_hairSpecularShift;
uniform float	m_hairSpecularShift2;
uniform float3	m_specularColour2;
uniform float	m_specularPower2;

float4	m_blendMats[3*MAX_BONES];

// textures
uniform texture	m_normalMap;
uniform texture	m_diffuse0;
uniform texture	m_diffuse1;
uniform texture	m_diffuse2;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
uniform  sampler2D s_normalMap : register(s0);
uniform  sampler2D s_depthMap : register(s1);
uniform  sampler2D s_diffuse0 : register(s2);
uniform  sampler2D s_diffuse1 : register(s3);
uniform  sampler2D s_diffuse2 : register(s4);
uniform  sampler2D s_stencilMap : register(s5);
