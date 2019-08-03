//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Floating-point render buffer clear.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_CLEAR_H
#define GP_CLEAR_H

#include <Gc/GcShader.h>
#include <Gc/GcStreamBuffer.h>
#include <Gc/GcRenderBuffer.h>

//--------------------------------------------------------------------------------------------------
/**
	@class		
	
	@brief		Clears a floating-point render buffer by rasterising a quad onto it.
**/
//--------------------------------------------------------------------------------------------------

class GpClear
{
public:
	GpClear();

    void SetColour( float red, float green, float blue, float alpha );
	
	void Clear( const GcRenderBufferHandle& hBuffer );

private:
	GcShaderHandle m_hVertexShader;				//!< The vertex shader.
	GcShaderHandle m_hFragmentShader;			//!< The fragment shader.
	GcStreamBufferHandle m_hVertexData;			//!< The vertex data.
};

#endif // ndef GP_CLEAR_H
