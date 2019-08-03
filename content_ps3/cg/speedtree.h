#ifndef SPEEDTREE_H
#define SPEEDTREE_H

#include "SpeedTree_Defines.h"

float4x4    g_amWindMatrices[NUM_WIND_MATRICES];

float4x4    g_mModelViewProj;                       // composite modelview/projection matrix
float4      g_vTreePos;                             // each tree is in a unique position and rotation: .xyz = pos, .w = rotation
//float       g_fTreeScale;                           // each tree has a unique scale (1.0 is no scale)
float4		g_vTreeParams; // x - scale, y - g_fWindMatrixOffset

float3 g_vOSLightDir;

float4		g_VPOStoUV;
	
float CalculateShadow(float2 screenPos)
{
	float2 coord = screenPos * g_VPOStoUV.xy;
	coord.y = 1.f - coord.y;
	float shadowTerm = tex2D( shadowMap, coord ).x;

	return shadowTerm;
}


///////////////////////////////////////////////////////////////////////  
//  RotationMatrix_zAxis
//
//  Constructs a Z-axis rotation matrix

float3x3 RotationMatrix_zAxis(float fAngle)
{
    // compute sin/cos of fAngle
    float2 vSinCos;
    sincos(fAngle, vSinCos.x, vSinCos.y);
    
    return float3x3(vSinCos.y, -vSinCos.x, 0.0f, 
                    vSinCos.x, vSinCos.y, 0.0f, 
                    0.0f, 0.0f, 1.0f);
}


///////////////////////////////////////////////////////////////////////  
//  RotationMatrix_yAxis
//
//  Constructs a Y-axis rotation matrix

float3x3 RotationMatrix_yAxis(float fAngle)
{
    // compute sin/cos of fAngle
    float2 vSinCos;
    sincos(fAngle, vSinCos.x, vSinCos.y);
    
    return float3x3(vSinCos.y, 0.0f, vSinCos.x,
                    0.0f, 1.0f, 0.0f,
                    -vSinCos.x, 0.0f, vSinCos.y);
}


///////////////////////////////////////////////////////////////////////  
//  RotationMatrix_xAxis
//
//  Constructs a X-axis rotation matrix

float3x3 RotationMatrix_xAxis(float fAngle)
{
    // compute sin/cos of fAngle
    float2 vSinCos;
    sincos(fAngle, vSinCos.x, vSinCos.y);
    
    return float3x3(1.0f, 0.0f, 0.0f,
                    0.0f, vSinCos.y, -vSinCos.x,
                    0.0f, vSinCos.x, vSinCos.y);
}

float4 WindEffect_TwoWeights(float4 vPosition, float4 vWindInfo)
{
    // use the matrix offset to help keep indentical trees looking distinct - and
    // modulate back down to the range [0, NUM_WIND_MATRICES].
    //
    // note: the modulus operator has proven considerably difficult to use
    // on multiple platforms.  use care when modifying these two lines.

	float fWindMatrixOffset = g_vTreeParams.y;
    int nWindIndex1 = int(vWindInfo.x + fWindMatrixOffset) % (NUM_WIND_MATRICES - 1);
    int nWindIndex2 = int(vWindInfo.z + fWindMatrixOffset) % (NUM_WIND_MATRICES - 1);
    
    // extract the wind weights
    float fWindWeight1 = vWindInfo.y;
    float fWindWeight2 = vWindInfo.w;

    
    // interpolate between still and full wind effect #1 by fWindWeight1
    float4 vWindEffect = lerp(vPosition, mul(vPosition, g_amWindMatrices[nWindIndex1]), fWindWeight1);
	//float4 vWindEffect = lerp(vPosition, mul(g_amWindMatrices[nWindIndex1], vPosition), fWindWeight1);
    
    // interpolate again between previous position and full wind effect #2 by fWindWeight2
    vWindEffect = lerp(vWindEffect, mul(vWindEffect, g_amWindMatrices[nWindIndex2]), fWindWeight2);
    //vWindEffect = lerp(vWindEffect, mul(g_amWindMatrices[nWindIndex2], vWindEffect), fWindWeight2);
	vWindEffect*= g_vTreeParams.x;	// apply tree scale
    
    return vWindEffect;
}

#endif


