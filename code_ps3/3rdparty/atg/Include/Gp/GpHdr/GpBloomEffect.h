//--------------------------------------------------------------------------------------------------
/**
	@file		GpBloomEffect.h
	
	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_BLOOMEFFECT_H
#define GP_BLOOMEFFECT_H

#include <Gc/GcShader.h>
#include <Gc/GcStreamBuffer.h>
#include <Gc/GcTexture.h>
#include <Gc/GcRenderBuffer.h>

//--------------------------------------------------------------------------------------------------
/**
	@class		GpBloomEffect
	
	@brief		Provides a simple camera bloom effect.
**/
//--------------------------------------------------------------------------------------------------

class GpBloomEffect
{
public:
	GpBloomEffect( u32 captureWidth, u32 captureHeight );

	void SetExposureLevel( float exposureLevel );
	void SetCaptureRange( float minLuminance, float maxLuminance );
	void SetStrength( float strength );

	void Capture( const GcTextureHandle& hTexture );
	void Render();

private:
	GcShaderHandle m_hVertexShader;
	GcShaderHandle m_hCaptureFragmentShader;
	GcShaderHandle m_hBlendFragmentShader;
	GcShaderHandle m_hBlurFragmentShader;

	uint m_sampleOffsetIndex;
	uint m_captureRangeIndex;
	uint m_bloomSizeIndex;
	uint m_strengthIndex;

	GcTextureHandle m_hCaptureTextures[2];
	GcRenderBufferHandle m_hCaptureBuffers[2];

	GcStreamBufferHandle m_hVertexData;				//!< The vertex data.

	float m_exposureLevel;							//!< The current exposure level.
	float m_minLuminance;							//!< The minimum luminance to capture before clamping.
	float m_maxLuminance;							//!< The maximum luminance to capture before clamping.
	float m_strength;								//!< The strength of the effect.
};

#endif // ndef GP_BLOOMEFFECT_H
