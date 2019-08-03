#ifndef SPEEDTREE_DEPTHHAZE_H
#define SPEEDTREE_DEPTHHAZE_H

#include "common.h"

float4x4 g_worldViewMatrix;
float3	 g_eyePosObjSpace;
float3   g_aConsts;				//   <1, log_2 e, termMultiplier >
float3	 g_gConsts;				//	 < ((1-g)^2), SunPower, SunMultipler>
float3	 g_sunDir;
float3   g_beta1PlusBeta2;
float3	 g_betaDash1;
float3	 g_betaDash2;
float3	 g_oneOverBeta1PlusBeta2;
float4	 g_sunColor;

void DepthHaze(in float4 objectPos, out float3 extinction, out float3 inscattering)
{
	depth_haze(objectPos, extinction, inscattering, g_worldViewMatrix, g_eyePosObjSpace, g_aConsts, g_gConsts, g_sunDir, g_beta1PlusBeta2, g_betaDash1, g_betaDash2, g_oneOverBeta1PlusBeta2, g_sunColor );
}

#endif