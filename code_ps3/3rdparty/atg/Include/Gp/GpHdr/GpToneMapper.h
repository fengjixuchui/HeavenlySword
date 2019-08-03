//--------------------------------------------------------------------------------------------------
/**
	@file		GpToneMapper.h
	
	@brief		Helper class to perform tone mapping from HDR to 8-bit.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_TONEMAPPER_H
#define GP_TONEMAPPER_H

#include <Gc/GcShader.h>
#include <Gc/GcStreamBuffer.h>
#include <Gc/GcTexture.h>
#include <Gc/GcRenderBuffer.h>

//--------------------------------------------------------------------------------------------------
/**
	@class		GpToneMapper
	
	@brief		Provides tone-mapping from linear HDR colour to 8-bit sRGB colour.
**/
//--------------------------------------------------------------------------------------------------

class GpToneMapper
{
public:
	GpToneMapper();

	void SetExposureLevel( float exposureLevel );
	void SetLuminanceBurnLevel( float burnLevel );

	void Render( const GcTextureHandle& hLinearColours );

private:
	GcShaderHandle m_hVertexShader;					//!< The tone-mapping vertex shader.
	GcShaderHandle m_hFragmentShader;				//!< The tone-mapping fragment shader.
	
	GcStreamBufferHandle m_hVertexData;				//!< The vertex data.

	float m_exposureLevel;							//!< The current exposure level.
	float m_luminanceBurnLevel;						//!< The current luminance burn-out level.
};

#endif // ndef GP_TONEMAPPER_H
