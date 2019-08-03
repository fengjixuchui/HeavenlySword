/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_LENSEFFECTS_PC_H
#define GFX_LENSEFFECTS_PC_H

#ifndef GFX_EXPOSURE_H
#include "gfx/exposure.h"
#endif

#ifndef GFX_GAUSSIAN_H
#include "gfx/gaussian.h"
#endif

class Shader;
class RenderTarget;

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
	~LensArtifacts( void );

	RenderTarget::Ptr Generate( const Texture::Ptr& pKeyLuminance, const RenderTarget::Ptr& pSource );
	RenderTarget::Ptr Generate( float fKeyLuminance, const RenderTarget::Ptr& pSource );

private:
	static float	GetMin()			{ return CStaticLensConfig::GetMin(); }
	static float	GetMax()			{ return CStaticLensConfig::GetMax(); }
	static float	GetEffectPower()	{ return CStaticLensConfig::GetEffectPower(); }
	static int		GetGausianLevels()	{ return CStaticLensConfig::GetGausianLevels(); }

	Shader* m_pCaptureVS;

	Shader* m_pCapturePS_normal;
	Shader* m_pGhostPass1PS_normal;
	Shader* m_pGhostPass2PS_normal;

	Shader* m_pCapturePS_cpuexp;
	Shader* m_pGhostPass1PS_cpuexp;
	Shader* m_pGhostPass2PS_cpuexp;

	CVertexDeclaration m_pCaptureDecl;
	
	NewGaussianBlur m_blurHelper;
	Texture::Ptr m_pVignetteTexture;
};

#endif // ndef _LENSEFFECTS_H
