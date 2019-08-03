//--------------------------------------------------
//!
//!	\file depthoffield.h
//!	Depth of field functionality
//!
//--------------------------------------------------

#ifndef GFX_DEPTHOFFIELD_PS3_H
#define GFX_DEPTHOFFIELD_PS3_H

#include "gfx/vertexdeclaration.h"
#include "gfx/gaussian.h"
#include "gfx/shader.h"
#include "gfx/pictureinpicture.h"

class DepthOfField;

//--------------------------------------------------
//!
//!	DepthOfField 
//! Our depth of field effect
//!
//--------------------------------------------------
class DepthOfField
{
public:
	DepthOfField();

	void		DebugDisplayDepthOfField();
	void		ApplyDepthOfField(	RenderTarget::Ptr& source,
									RenderTarget::Ptr& floatZ,
									RenderTarget::Ptr& scratchMem );

private:
	DebugShader *m_simpleVS;
	DebugShader *m_simple2VS;

	DebugShader *m_debugCapturePS;
	DebugShader *m_debugRenderPS;
	DebugShader *m_downSamplePS;
	DebugShader *m_depthOfFieldPS;
	DebugShader *m_pokeAlphaChannelPS;

	VBHandle		m_hSimpleQuadData;
	NewGaussianBlur m_blurHelper;

	RenderTarget::Ptr	GetDebugDepthOfField( RenderTarget::Ptr source );
	RenderTarget::Ptr	m_debugAlphaChannel;
	RenderTarget::Ptr	m_debugAlphaChannelBlur;
};

#endif // _DEPTHOFFIELD_H
