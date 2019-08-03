#ifndef SPEEDTREE_LEAF_H
#define SPEEDTREE_LEAF_H

#include "speedtree.h"

#ifdef APPLY_DEPTHHAZE
#include "speedtree_depthhaze.h"
#endif

float4    g_mLeafUnitSquare[4] =                  // unit leaf card that's turned towards the camera and wind-rocked/rustled by the
            {                                    // vertex shader.  card is aligned on YZ plane and centered at (0.0f, 0.0f, 0.0f)
                float4(0.0f, 0.5f, 0.5f, 0.0f), 
                float4(0.0f, -0.5f, 0.5f, 0.0f), 
                float4(0.0f, -0.5f, -0.5f, 0.0f), 
                float4(0.0f, 0.5f, -0.5f, 0.0f)
            };

float4		g_vCameraAngles;

float4      g_avLeafAngles[MAX_NUM_LEAF_ANGLES]; // each element: .x = rock angle, .y = rustle angle
                                                 // each element is a float4, even though only a float2 is needed, to facilitate
                                                 // fast uploads on all platforms (one call to upload whole array)

float4      g_vLeafAngleScalars;                 // each tree model has unique scalar values: .x = rock scalar, .y = rustle scalar


#ifdef APPLY_DEPTHHAZE
float4 ComputeGeometry( int nCorner, float3 vPosition, float4 vWindParams, float4 vDimensions, float4 vAngles, out float3 vExtinction, out float3 vInscattering )
#else
float4 ComputeGeometry( int nCorner, float3 vPosition, float4 vWindParams, float4 vDimensions, float4 vAngles )
#endif
{
    float fAzimuth = g_vCameraAngles.x;                      // camera azimuth, but no effect if used for leaf meshes
    float fPitch = g_vCameraAngles.y;                        // camera pitch, but no effect if used for leaf meshes
	float fWindAngleIndex = vAngles.z;                 // which wind matrix this leaf card will follow

    // compute rock and rustle values
    float2 vLeafRockAndRustle = g_vLeafAngleScalars.xy * g_avLeafAngles[fWindAngleIndex].xy;;

	float3 vPivotedPoint = g_mLeafUnitSquare[nCorner].xzy; // * g_fTreeScale;

	float3 vCorner = vPivotedPoint * vDimensions.xyx;


	float3x3 matRotation = RotationMatrix_yAxis(fAzimuth + vAngles.y);
	matRotation = mul(matRotation, RotationMatrix_zAxis(fPitch + vAngles.x + vLeafRockAndRustle.y));
	matRotation = mul(matRotation, RotationMatrix_xAxis(-vLeafRockAndRustle.x));

	float4 outPosition = WindEffect_TwoWeights(float4(vPosition, 1.f), vWindParams);
	//float4 outPosition = float4(vPosition, 1.f);
    
	outPosition.xyz += mul(matRotation, vCorner);
	outPosition = float4(outPosition.xyz + g_vTreePos.xyz, 1.f);

	#ifdef APPLY_DEPTHHAZE
	DepthHaze(outPosition, vExtinction, vInscattering);
	#endif

	outPosition = mul(outPosition, g_mModelViewProj);

	return outPosition;																	  
}


#endif