#ifndef SPEEDTREE_BILLBOARD_H
#define SPEEDTREE_BILLBOARD_H

#ifdef APPLY_DEPTHHAZE
#include "speedtree_depthhaze.h"
#endif

float4x4    g_mBBUnitSquare =                    // unit billboard card that's turned towards the camera.  card is aligned on 
            {                                    // YZ plane and centered at (0.0f, 0.0f, 0.5f)
                float4(0.0f, 0.5f, 1.0f, 0.0f), 
                float4(0.0f, -0.5f, 1.0f, 0.0f), 
                float4(0.0f, -0.5f, 0.0f, 0.0f), 
                float4(0.0f, 0.5f, 0.0f, 0.0f) 
            };

float4      g_v360TexCoords[NUM_360_IMAGES];     // each element contains (s, t) texcoords - 4 elements define texcoords for one billboard
                                                 // each element is a float4, even though only a float2 is needed, to facilitate
                                                 // fast uploads on all platforms (one call to upload whole array)

float4		g_vCameraAngles;

float Modulate_Float(float x, float y)
{
    return x - (int(x / y) * y);
}

const float4	g_BillboardConsts = { 6.28318530f, 255.0f, 84.0f, 171.0f };
const float4	g_BillboardConsts2; 

#ifdef APPLY_DEPTHHAZE
float4  ComputeGeometry(float4 vPosition, int nCorner, float4 vGeom, float3 vMiscParams, out float2 texCoord, out float alpha, out float3 extinction, out float3 inscattering, out float3x3 rotationMatrix)
#else
float4  ComputeGeometry(float4 vPosition, int nCorner, float4 vGeom, float3 vMiscParams, out float2 texCoord, out float alpha)
#endif
{
    float fAzimuth = g_vCameraAngles.x;         // current camera azimuth
    float fPitch = g_vCameraAngles.y;           // current camera pitch
	float fSpecialAzimuth = g_vCameraAngles.z;
    float c_fTwoPi = g_BillboardConsts.x;               // 2 * pi
    int nNumImages = g_vCameraAngles.w; //input.vMiscParams.z;
    float c_fSliceDiameter = c_fTwoPi / float(nNumImages); // diameter = 360 / g_nNum360Im	ages
    float c_fLodFade = vGeom.w;           // computed on CPU - the amount the billboard as a whole is faded from 3D geometry
    float c_fClearAlpha = g_BillboardConsts.y;               // alpha testing, 255 means not visible
    float c_fOpaqueAlpha = g_BillboardConsts.z;               // alpha testing, 84 means fully visible
    float c_fAlphaSpread = g_BillboardConsts.w;              // 171 = 255 - 84
    int nTexCoordTableOffset = vMiscParams.y;

    float3x3 matRotation = RotationMatrix_yAxis(fAzimuth);

    // adjust the azimuth to appear in range [0,2*pi]
    float fAdjustedAzimuth = fSpecialAzimuth - vGeom.z;
    if (fAdjustedAzimuth < 0.0f)
        fAdjustedAzimuth += c_fTwoPi;
    else if (fAdjustedAzimuth > c_fTwoPi)
        fAdjustedAzimuth -= c_fTwoPi;

	//fAdjustedAzimuth = 0;
    // pick the billboard image index and access the extract texcoords from the table
#ifdef SPEEDTREE_BILLBOARD_PASS_2
	int nIndex0 = int(fAdjustedAzimuth / c_fSliceDiameter);
#else
	int nIndex0 = int(fAdjustedAzimuth / c_fSliceDiameter + 1);
#endif
	if (nIndex0 > (nNumImages - 1))
        nIndex0 = 0;

    //float4 vTexCoords = g_v360TexCoords[nIndex0 + nTexCoordTableOffset];
	float4 vTexCoords = g_v360TexCoords[nIndex0];
    if (nCorner == 0)
        texCoord = vTexCoords.zw;
    else if (nCorner == 1)
        texCoord = vTexCoords.xw;
    else if (nCorner == 2)
        texCoord = vTexCoords.xy;
    else
        texCoord = vTexCoords.zy;


#ifdef SPEEDTREE_BILLBOARD_PASS_2
	float fAlpha0 = (fAdjustedAzimuth - (nIndex0 * c_fSliceDiameter)) / c_fSliceDiameter;
#else
    float fAlpha0 = 1.0f - Modulate_Float(fAdjustedAzimuth, c_fSliceDiameter) / c_fSliceDiameter;
#endif

    float fFadePoint = lerp(c_fClearAlpha, c_fOpaqueAlpha, c_fLodFade);

    fAlpha0 = lerp(fFadePoint, c_fClearAlpha, pow(fAlpha0, g_BillboardConsts2.x));
    
    // each billboard may be faded at a distinct value, but it isn't efficient to change
    // the alpha test value per billboard.  instead we adjust the alpha value of the 
    // billboards's outgoing color to achieve the same effect against a static alpha test 
    // value (c_gOpaqueAlpha).
    //fAlpha0 = saturate(1.0f - saturate(fAlpha0 - c_fOpaqueAlpha) / c_fAlphaSpread);
	fAlpha0 = 1.0f - (fAlpha0 - c_fOpaqueAlpha) / c_fAlphaSpread;


	float3	vCorner = g_mBBUnitSquare[nCorner].xzy * vGeom.xyx;
    vPosition.xyz += mul(matRotation, vCorner);
	vPosition.w = 1.f;

#ifdef APPLY_DEPTHHAZE
	DepthHaze(vPosition, extinction, inscattering);
	rotationMatrix = matRotation;
#endif

	vPosition = mul(vPosition, g_mModelViewProj);
	alpha = fAlpha0;

	return vPosition;

}


#endif