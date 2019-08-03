/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_GAUSSIAN_PC_H
#define GFX_GAUSSIAN_PC_H

#include "gfx/vertexdeclaration.h"
#include "gfx/texture.h"
#include "gfx/rendertarget.h"

class Shader;

/***************************************************************************************************
*
*	CLASS			CGaussianBlurHelper
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGaussianBlurHelper
{
public:
	CGaussianBlurHelper();

	void SeparableBlur( RenderTarget::Ptr const& pobSource, 
						RenderTarget::Ptr const& pobTemp, 
						CVector const& obWeight ) const;

	void RecursiveBlur( RenderTarget::Ptr& pobSource, 
						RenderTarget::Ptr& pobTemp, 
						CVector const& obWeight, 
						int iPassStart, 
						int iPassEnd ) const;

private:
	Shader* m_pobVertexShader;
	Shader* m_pobPixelShader;
	CVertexDeclaration m_pobDeclaration;
};

//--------------------------------------------------
//!
//!	NewGaussianBlur
//! PC/XENON friendly blur helper class
//!
//--------------------------------------------------
class NewGaussianBlur
{
public:
	NewGaussianBlur();
	void RecursiveBlur( RenderTarget::Ptr& pSource, 
						D3DFORMAT fmt,
						CVector const& weight, 
						int iPassStart, 
						int iPassEnd ) const;

private:
	Shader* m_pVertexShader;
	Shader* m_pPixelShader;
	CVertexDeclaration m_pDecl;
};

#endif // ndef GFX_GAUSSIAN_PC_H
