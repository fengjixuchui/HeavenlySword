/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_LENSEFFECTS_PS3_H
#define GFX_LENSEFFECTS_PS3_H

#ifndef GFX_EXPOSURE_H
#include "gfx/exposure.h"
#endif

#ifndef GFX_GAUSSIAN_H
#include "gfx/gaussian.h"
#endif

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

class Texture;

//--------------------------------------------------
//!
//!	LensArtifacts
//! generates ghosting and bloom in a PC/XENON friendly manner
//!
//--------------------------------------------------
class LensArtifacts
{
public:
	LensArtifacts( void );

	RenderTarget::Ptr Generate( const Texture::Ptr& pKeyLuminance, const Texture::Ptr& pSource );
	RenderTarget::Ptr Generate( float fKeyLuminance, const Texture::Ptr& pSource );

private:
	static float	GetMin()			{ return CStaticLensConfig::GetMin(); }
	static float	GetMax()			{ return CStaticLensConfig::GetMax(); }
	static float	GetEffectPower()	{ return CStaticLensConfig::GetEffectPower(); }
	static int		GetGausianLevels()	{ return CStaticLensConfig::GetGausianLevels(); }

	DebugShader *	m_captureVS;

	DebugShader	*	m_capturePS_normal;
	DebugShader	*	m_ghostPass1PS_normal;
	DebugShader	*	m_ghostPass2PS_normal;

	VBHandle		m_hSimpleQuadData;
	
	NewGaussianBlur	m_blurHelper;
	Texture::Ptr	m_pVignetteTexture;
};

#endif // ndef _LENSEFFECTS_H
