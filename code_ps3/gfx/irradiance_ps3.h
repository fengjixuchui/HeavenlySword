//--------------------------------------------------
//!
//!	\file irradiance_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_IRRADIANCE_H
#define GFX_IRRADIANCE_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_SPHERICAL_HARMONICS_H
#include "gfx/sphericalharmonics.h"
#endif

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif


//--------------------------------------------------------------------------------------------------------
//!
//!	Irradiance Cache Manager
//! It generates a cube map per view per frame which stores 
//!
//--------------------------------------------------------------------------------------------------------
class IrradianceManager
{
public:
	IrradianceManager( void );
	void GenerateIrradianceCubeMap( const Texture::Ptr pCubeMap, const SHChannelMatrices& SHCoeffs );

private:
	SHChannelMatrices	m_cachedSHCoeffs;

	DebugShader*	m_pIrradianceVertexShader;
	DebugShader*	m_pIrradiancePixelShader;
	DebugShader*	m_pDownsampleVertexShader;
	DebugShader*	m_pDownsamplePixelShader;
	VBHandle		m_hSimpleQuadData[7];
};

#endif // GFX_IRRADIANCE_H
