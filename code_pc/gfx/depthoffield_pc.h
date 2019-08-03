//--------------------------------------------------
//!
//!	\file depthoffield.h
//!	Depth of field functionality
//!
//--------------------------------------------------

#ifndef GFX_DEPTHOFFIELD_PC_H
#define GFX_DEPTHOFFIELD_PC_H

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_GAUSSIAN_H
#include "gfx/gaussian.h"
#endif

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
	~DepthOfField();

	void		DebugDisplayDepthOfField();
	void		ApplyDepthOfField( RenderTarget::Ptr& source );

private:
	Shader* m_pSimpleVS;
	Shader* m_pDebugCapturePS;
	Shader* m_pDebugRenderPS;
	Shader* m_pDownSamplePS;
	Shader* m_pDepthOfFieldPS;

	CVertexDeclaration m_pSimpleDecl;

	NewGaussianBlur m_blurHelper;

	RenderTarget::Ptr	GetDebugDepthOfField( RenderTarget::Ptr source );
	RenderTarget::Ptr	m_debugAlphaChannel;
	RenderTarget::Ptr	m_debugAlphaChannelBlur;
};

#endif _DEPTHOFFIELD_H
