/**
	@file radialmotionblur.h

	@author campf

	Class definition for radial motion blur effect.
*/

#ifndef RADIAL_MOTION_BLUR_H
#define RADIAL_MOTION_BLUR_H

#include "gfx/vertexdeclaration.h"
#include "gfx/debugshader_ps3.h"
#include "gfx/rendertarget.h"
#include "gfx/gaussian_ps3.h"

/**
	@class CRadialMotionBlur
*/
class CRadialMotionBlur
{
	public:

	enum
	{
		RENDER_TARGET_WIDTH = 320,
		RENDER_TARGET_HEIGHT = 180
	};

	CRadialMotionBlur(void);

	inline void Render(RenderTarget::Ptr &LDR720pBuffer )
	{
		if (m_bEnabled)
		{
			RenderInternal(LDR720pBuffer);
		}
	}
	inline void Enable(bool bEnable) { m_bEnabled = bEnable; }
	inline void SetMaskSize(float fSize) {m_fMaskSize = fSize;}

	private:

	void RenderInternal( RenderTarget::Ptr &LDR720pBuffer );

	VBHandle m_hDynamicQuadData;
	VBHandle m_hUnscaledQuadData;
	VBHandle m_hScaleUpQuadData[2];
	VBHandle m_hScaleDownQuadData[2];

	DebugShader *m_hQuadVertexShader;
	DebugShader *m_hQuadFragmentShader;
	DebugShader *m_hFilterDownsampleShader;

	NewGaussianBlur m_blurHelper;

	Texture::Ptr m_maskTexture;

	float m_fMaskSize;
	bool m_bEnabled;
};

#endif //RADIAL_MOTION_BLUR_H
