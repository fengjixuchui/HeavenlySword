struct BranchVSOutput
{
    float4  vPosition               : POSITION;
    float2  vDiffuseTexCoords       : TEXCOORD0;
//    float2  vSelfShadowTexCoords    : TEXCOORD1;
    float2  vNormalTexCoords        : TEXCOORD1;
	float3  vTSLightDir				: TEXCOORD2;

//#ifdef BRANCH_DETAIL_LAYER
//    float2  vDetailTexCoords        : TEXCOORD3;
//#endif
//    float3  vNormal                 : TEXCOORD4;
//#ifdef USE_FOG
//    float	fFog                    : TEXCOORD5; // using FOG here causes a ps_2_0 compilation failure
//#endif
	float3	vExtinction				: TEXCOORD3;
	float3	vInscattering			: TEXCOORD4;
};
