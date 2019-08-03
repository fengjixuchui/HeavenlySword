//--------------------------------------------------
//!
//!	\file effect_shared.h
//!	Common parameters shared by all effect shaders
//!
//--------------------------------------------------

//--------------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------------
#include "common.h"

// globals
float4		g_vpScalars;		// needed for point sprite psystems only
float4		g_cameraZ;			// needed for point sprite psystems only
float3		g_cameraUnitAxisX;	// needed for quad sprite psystems only
float3		g_cameraUnitAxisY;	// needed for quad sprite psystems only
float4x4	g_ImageTransform;
float3		g_eyePos;

